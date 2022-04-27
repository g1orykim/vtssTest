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

// Avoid "vtss_api.h not used in module vtss_otn_api.c"
/*lint --e{766} */

#include "vtss_api.h"
#if defined(VTSS_FEATURE_OTN)

#include "vtss_state.h"
#include "vtss_otn_api.h"

/* ================================================================= *
 *  Dynamic Config
 * ================================================================= */

/* ================================================================= *
 *  OCH Config
 * ================================================================= */
vtss_rc vtss_otn_och_fec_set(const vtss_inst_t          inst,
                             const vtss_port_no_t       port_no,
                             const vtss_otn_och_fec_t   *const cfg)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && cfg) {
        rc = VTSS_RC_ERROR;
        if ((u16)cfg->type < (u16)VTSS_OTN_OCH_FEC_MAX) {
            rc = VTSS_FUNC_COLD(cil.otn_och_fec_set, port_no, cfg);
        }
    }
    VTSS_EXIT();
#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
    /* Inform 1588 about the FEC change */
    if (rc == VTSS_RC_OK) {
        (void)vtss_phy_ts_fec_mode_set(inst, port_no, cfg);
    }
#endif
    return rc;
}


vtss_rc vtss_otn_och_fec_get(const vtss_inst_t      inst,
                             const vtss_port_no_t   port_no,
                             vtss_otn_och_fec_t     *const cfg)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && cfg) {
        *cfg = vtss_state->otn_state[port_no].och_fec;
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_otn_och_ber_threshold_set(const vtss_inst_t     inst,
                                        const vtss_port_no_t port_no,
                                        const vtss_otn_och_fec_t   *const cfg)
{
    vtss_state_t *vtss_state;
    vtss_otn_och_fec_t fec_state;
    vtss_rc      rc;        

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && cfg) {
        fec_state = vtss_state->otn_state[port_no].och_fec;  
        if ((fec_state.type != cfg->type) || ((cfg->type != VTSS_OTN_OCH_FEC_RS) && (cfg->type != VTSS_OTN_OCH_FEC_I7))) {
            VTSS_EXIT();
            return VTSS_RC_ERROR;
        }

        if (cfg->type == VTSS_OTN_OCH_FEC_RS)
            if ((cfg->ber_threshold !=0) && (cfg->ber_threshold != 1)){
                VTSS_EXIT();
                return VTSS_RC_ERROR;
            }    

        rc = VTSS_FUNC_COLD(cil.otn_och_ber_threshold_set, port_no, cfg);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_otn_och_ber_threshold_get(const vtss_inst_t     inst,
                                        const vtss_port_no_t port_no,
                                        vtss_otn_och_fec_t   *const cfg)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;
                            
    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && cfg) {
        *cfg = vtss_state->otn_state[port_no].och_fec;
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_otn_och_loopback_set(const vtss_inst_t         inst,
                                  const vtss_port_no_t      port_no,
                                  const vtss_otn_och_loop_t *const cfg)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    do {
        if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && cfg) {
            if (!(vtss_state->otn_state[port_no].och_loop.line_side_equipment_loopback == cfg->line_side_equipment_loopback) || !(vtss_state->otn_state[port_no].och_loop.line_side_line_loopback == cfg->line_side_line_loopback) || !(vtss_state->otn_state[port_no].och_loop.system_side_line_loopback == cfg->system_side_line_loopback) || !(vtss_state->otn_state[port_no].och_loop.system_side_equipment_loopback == cfg->system_side_equipment_loopback)) {
                if ((vtss_state->otn_state[port_no].och_loop.line_side_equipment_loopback) && (vtss_state->otn_state[port_no].och_loop.system_side_line_loopback)) {
                    rc = VTSS_RC_ERROR;
                    break; 
                }
                if ((vtss_state->otn_state[port_no].och_loop.line_side_line_loopback) && (vtss_state->otn_state[port_no].och_loop.system_side_equipment_loopback)) {
                    rc = VTSS_RC_ERROR;
                    break; 
                }
                vtss_state->otn_state[port_no].och_loop = *cfg;
                rc = VTSS_FUNC_COLD(cil.otn_och_loopback_set, port_no);
            }
        }
    } while(0);
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_och_loopback_get(const vtss_inst_t    inst,
                                  const vtss_port_no_t port_no,
                                  vtss_otn_och_loop_t  *const cfg)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && cfg) {
        *cfg = vtss_state->otn_state[port_no].och_loop;
    }
    VTSS_EXIT();
    return rc;
}


/* ================================================================= *
 *  OTU Config
 * ================================================================= */
vtss_rc vtss_otn_otu_tti_set(const vtss_inst_t          inst,
                             const vtss_port_no_t       port_no,
                             const vtss_otn_otu_tti_t   *const cfg)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && cfg) {
        vtss_state->otn_state[port_no].otu_tti = *cfg;
        rc = VTSS_FUNC_COLD(cil.otn_otu_tti_set, port_no);
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_otu_tti_get(const vtss_inst_t      inst,
                             const vtss_port_no_t   port_no,
                             vtss_otn_otu_tti_t     *const cfg)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && cfg) {
        *cfg = vtss_state->otn_state[port_no].otu_tti;
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_otu_acc_tti_get(const vtss_inst_t         inst,
                                 const vtss_port_no_t      port_no,
                                 vtss_otn_otu_acc_tti_t    *const tti)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && tti) {
        rc = VTSS_FUNC_COLD(cil.otn_otu_acc_tti_get, port_no, tti);
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_otu_ais_set(const vtss_inst_t          inst,
                             const vtss_port_no_t       port_no,
                             const vtss_otn_otu_ais_t   *const cfg)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && cfg) {
        vtss_state->otn_state[port_no].otu_ais = *cfg;
        rc = VTSS_FUNC_COLD(cil.otn_otu_ais_set, port_no);
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_otu_ais_get(const vtss_inst_t       inst,
                             const vtss_port_no_t    port_no,
                             vtss_otn_otu_ais_t      *const cfg)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && cfg) {
        *cfg = vtss_state->otn_state[port_no].otu_ais;
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_otu_consequent_actions_set(const vtss_inst_t               inst,
                                            const vtss_port_no_t            port_no,
                                            const vtss_otn_otu_cons_act_t   *const cfg)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && cfg) {
        vtss_state->otn_state[port_no].otu_cons = *cfg;
        rc = VTSS_FUNC_COLD(cil.otn_otu_consequent_actions_set, port_no);
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_otu_consequent_actions_get(const vtss_inst_t         inst,
                                            const vtss_port_no_t      port_no,
                                            vtss_otn_otu_cons_act_t   *const cfg)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && cfg) {
        *cfg = vtss_state->otn_state[port_no].otu_cons;
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_otu_oh_insertion_set(const vtss_inst_t             inst,
                                      const vtss_port_no_t          port_no,
                                      const vtss_otn_otu_oh_ins_t   *const cfg)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && cfg) {
        vtss_state->otn_state[port_no].otu_oh = *cfg;
        rc = VTSS_FUNC_COLD(cil.otn_otu_oh_insertion_set, port_no);
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_otu_oh_insertion_get(const vtss_inst_t       inst,
                                      const vtss_port_no_t    port_no,
                                      vtss_otn_otu_oh_ins_t   *const cfg)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && cfg) {
        *cfg = vtss_state->otn_state[port_no].otu_oh;
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_otu_tx_res_set(const vtss_inst_t             inst,
                                const vtss_port_no_t          port_no,
                                const vtss_otn_otu_tx_res_t   *const cfg)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && cfg) {
        vtss_state->otn_state[port_no].otu_res = *cfg;
        rc = VTSS_FUNC_COLD(cil.otn_otu_tx_res_set, port_no);
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_otu_tx_res_get(const vtss_inst_t       inst,
                                const vtss_port_no_t    port_no,
                                vtss_otn_otu_tx_res_t   *const cfg)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && cfg) {
        *cfg = vtss_state->otn_state[port_no].otu_res;
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_otu_acc_res_get(const vtss_inst_t        inst,
                                 const vtss_port_no_t     port_no,
                                 vtss_otn_otu_acc_res_t   *const res)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && res) {
        rc = VTSS_FUNC_COLD(cil.otn_otu_acc_res_get, port_no, res);
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_otu_tx_smres_set(const vtss_inst_t               inst,
                                  const vtss_port_no_t            port_no,
                                  const vtss_otn_otu_tx_smres_t   *const cfg)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && cfg) {
        vtss_state->otn_state[port_no].otu_smres = *cfg;
        rc = VTSS_FUNC_COLD(cil.otn_otu_tx_smres_set, port_no);
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_otu_tx_smres_get(const vtss_inst_t         inst,
                                  const vtss_port_no_t      port_no,
                                  vtss_otn_otu_tx_smres_t   *const cfg)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && cfg) {
        *cfg = vtss_state->otn_state[port_no].otu_smres;
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_otu_acc_smres_get(const vtss_inst_t          inst,
                                   const vtss_port_no_t       port_no,
                                   vtss_otn_otu_acc_smres_t   *const smres)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && smres) {
        rc = VTSS_FUNC_COLD(cil.otn_otu_acc_smres_get, port_no, smres);
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_otu_tx_gcc0_set(const vtss_inst_t              inst,
                                 const vtss_port_no_t           port_no,
                                 const vtss_otn_otu_tx_gcc0_t   *const cfg)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && cfg) {
        vtss_state->otn_state[port_no].otu_gcc0 = *cfg;
        rc = VTSS_FUNC_COLD(cil.otn_otu_tx_gcc0_set, port_no);
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_otu_tx_gcc0_get(const vtss_inst_t        inst,
                                 const vtss_port_no_t     port_no,
                                 vtss_otn_otu_tx_gcc0_t   *const cfg)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && cfg) {
        *cfg = vtss_state->otn_state[port_no].otu_gcc0;
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_otu_acc_gcc0_get(const vtss_inst_t         inst,
                                  const vtss_port_no_t      port_no,
                                  vtss_otn_otu_acc_gcc0_t   *const gcc0)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && gcc0) {
        rc = VTSS_FUNC_COLD(cil.otn_otu_acc_gcc0_get, port_no, gcc0);
    }
    VTSS_EXIT();
    return rc;
}



/* ================================================================= *
 *  ODU Config
 * ================================================================= */
vtss_rc vtss_otn_odu_mode_set(const vtss_inst_t             inst,
                              const vtss_port_no_t          port_no,
                              const vtss_otn_odu_mode_t     *const cfg)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && cfg) {
        vtss_state->otn_state[port_no].odu_mode = *cfg;
        rc = VTSS_FUNC_COLD(cil.otn_odu_mode_set, port_no);
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_odu_mode_get(const vtss_inst_t      inst,
                              const vtss_port_no_t   port_no,
                              vtss_otn_odu_mode_t    *const cfg)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && cfg) {
        *cfg = vtss_state->otn_state[port_no].odu_mode;
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_odu_tti_set(const vtss_inst_t          inst,
                             const vtss_port_no_t       port_no,
                             const vtss_otn_odu_tti_t   *const cfg)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && cfg) {
        vtss_state->otn_state[port_no].odu_tti = *cfg;
        rc = VTSS_FUNC_COLD(cil.otn_odu_tti_set, port_no);
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_odu_tti_get(const vtss_inst_t      inst,
                             const vtss_port_no_t   port_no,
                             vtss_otn_odu_tti_t     *const cfg)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && cfg) {
        *cfg = vtss_state->otn_state[port_no].odu_tti;
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_odu_acc_tti_get(const vtss_inst_t         inst,
                                 const vtss_port_no_t      port_no,
                                 vtss_otn_odu_acc_tti_t    *const tti)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && tti) {
        rc = VTSS_FUNC_COLD(cil.otn_odu_acc_tti_get, port_no, tti);
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_odu_ais_set(const vtss_inst_t          inst,
                             const vtss_port_no_t       port_no,
                             const vtss_otn_odu_ais_t   *const cfg)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && cfg) {
        vtss_state->otn_state[port_no].odu_ais = *cfg;
        rc = VTSS_FUNC_COLD(cil.otn_odu_ais_set, port_no);
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_odu_ais_get(const vtss_inst_t       inst,
                             const vtss_port_no_t    port_no,
                             vtss_otn_odu_ais_t      *const cfg)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && cfg) {
        *cfg = vtss_state->otn_state[port_no].odu_ais;
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_odu_consequent_actions_set(const vtss_inst_t               inst,
                                            const vtss_port_no_t            port_no,
                                            const vtss_otn_odu_cons_act_t   *const cfg)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && cfg) {
        vtss_state->otn_state[port_no].odu_cons = *cfg;
        rc = VTSS_FUNC_COLD(cil.otn_odu_consequent_actions_set, port_no);
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_odu_consequent_actions_get(const vtss_inst_t         inst,
                                            const vtss_port_no_t      port_no,
                                            vtss_otn_odu_cons_act_t   *const cfg)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && cfg) {
        *cfg = vtss_state->otn_state[port_no].odu_cons;
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_odu_oh_insertion_set(const vtss_inst_t             inst,
                                      const vtss_port_no_t          port_no,
                                      const vtss_otn_odu_oh_ins_t   *const cfg)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && cfg) {
        vtss_state->otn_state[port_no].odu_oh = *cfg;
        rc = VTSS_FUNC_COLD(cil.otn_odu_oh_insertion_set, port_no);
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_odu_oh_insertion_get(const vtss_inst_t       inst,
                                      const vtss_port_no_t    port_no,
                                      vtss_otn_odu_oh_ins_t   *const cfg)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && cfg) {
        *cfg = vtss_state->otn_state[port_no].odu_oh;
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_odu_tx_res_set(const vtss_inst_t              inst,
                                const vtss_port_no_t           port_no,
                                const vtss_otn_odu_tx_res_t    *const cfg)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && cfg) {
        vtss_state->otn_state[port_no].odu_res = *cfg;
        rc = VTSS_FUNC_COLD(cil.otn_odu_tx_res_set, port_no);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_otn_odu_tx_res_get(const vtss_inst_t       inst,
                                const vtss_port_no_t    port_no,
                                vtss_otn_odu_tx_res_t   *const cfg)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && cfg) {
        *cfg = vtss_state->otn_state[port_no].odu_res;
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_otn_odu_acc_res_get(const vtss_inst_t        inst,
                                 const vtss_port_no_t     port_no,
                                 vtss_otn_odu_acc_res_t   *const res)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && res) {
        rc = VTSS_FUNC_COLD(cil.otn_odu_acc_res_get, port_no, res);
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_odu_tx_exp_set(const vtss_inst_t             inst,
                                const vtss_port_no_t          port_no,
                                const vtss_otn_odu_tx_exp_t   *const cfg)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && cfg) {
        vtss_state->otn_state[port_no].odu_exp = *cfg;
        rc = VTSS_FUNC_COLD(cil.otn_odu_tx_exp_set, port_no);
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_odu_tx_exp_get(const vtss_inst_t       inst,
                                const vtss_port_no_t    port_no,
                                vtss_otn_odu_tx_exp_t   *const cfg)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && cfg) {
        *cfg = vtss_state->otn_state[port_no].odu_exp;
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_odu_acc_exp_get(const vtss_inst_t        inst,
                                 const vtss_port_no_t     port_no,
                                 vtss_otn_odu_acc_exp_t   *const exp)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && exp) {
        rc = VTSS_FUNC_COLD(cil.otn_odu_acc_exp_get, port_no, exp);
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_odu_tx_ftfl_set(const vtss_inst_t              inst,
                                 const vtss_port_no_t           port_no,
                                 const vtss_otn_odu_tx_ftfl_t   *const cfg)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && cfg) {
        vtss_state->otn_state[port_no].odu_ftfl = *cfg;
        rc = VTSS_FUNC_COLD(cil.otn_odu_tx_ftfl_set, port_no);
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_odu_tx_ftfl_get(const vtss_inst_t        inst,
                                 const vtss_port_no_t     port_no,
                                 vtss_otn_odu_tx_ftfl_t   *const cfg)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && cfg) {
        *cfg = vtss_state->otn_state[port_no].odu_ftfl;
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_odu_acc_ftfl_get(const vtss_inst_t         inst,
                                  const vtss_port_no_t      port_no,
                                  vtss_otn_odu_acc_ftfl_t   *const ftfl)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && ftfl) {
        rc = VTSS_FUNC_COLD(cil.otn_odu_acc_ftfl_get, port_no, ftfl);
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_odu_tx_aps_set(const vtss_inst_t             inst,
                                const vtss_port_no_t          port_no,
                                const vtss_otn_odu_tx_aps_t   *const cfg)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && cfg) {
        vtss_state->otn_state[port_no].odu_aps = *cfg;
        rc = VTSS_FUNC_COLD(cil.otn_odu_tx_aps_set, port_no);
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_odu_tx_aps_get(const vtss_inst_t       inst,
                                const vtss_port_no_t    port_no,
                                vtss_otn_odu_tx_aps_t   *const cfg)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && cfg) {
        *cfg = vtss_state->otn_state[port_no].odu_aps;
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_odu_acc_aps_get(const vtss_inst_t        inst,
                                 const vtss_port_no_t     port_no,
                                 vtss_otn_odu_acc_aps_t   *const aps)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && aps) {
        rc = VTSS_FUNC_COLD(cil.otn_odu_acc_aps_get, port_no, aps);
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_odu_pt_set(const vtss_inst_t         inst,
                            const vtss_port_no_t      port_no,
                            const vtss_otn_odu_pt_t   *const cfg)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && cfg) {
        vtss_state->otn_state[port_no].odu_pt = *cfg;
        rc = VTSS_FUNC_COLD(cil.otn_odu_pt_set, port_no);
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_odu_pt_get(const vtss_inst_t      inst,
                            const vtss_port_no_t   port_no,
                            vtss_otn_odu_pt_t      *const cfg)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && cfg) {
        *cfg = vtss_state->otn_state[port_no].odu_pt;
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_odu_acc_pt_get(const vtss_inst_t        inst,
                                const vtss_port_no_t     port_no,
                                vtss_otn_odu_acc_pt_t    *const pt)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && pt) {
        rc = VTSS_FUNC_COLD(cil.otn_odu_acc_pt_get, port_no, pt);
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_odu_opu_oh_insertion_set(const vtss_inst_t                 inst,
                                          const vtss_port_no_t              port_no,
                                          const vtss_otn_odu_opu_oh_ins_t   *const cfg)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && cfg) {
        vtss_state->otn_state[port_no].odu_opu_oh = *cfg;
        rc = VTSS_FUNC_COLD(cil.otn_odu_opu_oh_insertion_set, port_no);
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_odu_opu_oh_insertion_get(const vtss_inst_t           inst,
                                          const vtss_port_no_t        port_no,
                                          vtss_otn_odu_opu_oh_ins_t   *const cfg)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && cfg) {
        *cfg = vtss_state->otn_state[port_no].odu_opu_oh;
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_odu_opu_test_insertion_set(const vtss_inst_t                   inst,
                                            const vtss_port_no_t                port_no,
                                            const vtss_otn_odu_opu_test_ins_t   *const cfg)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && cfg) {
        vtss_state->otn_state[port_no].odu_opu_test = *cfg;
        rc = VTSS_FUNC_COLD(cil.otn_odu_opu_test_insertion_set, port_no);
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_odu_opu_test_insertion_get(const vtss_inst_t             inst,
                                            const vtss_port_no_t          port_no,
                                            vtss_otn_odu_opu_test_ins_t   *const cfg)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && cfg) {
        *cfg = vtss_state->otn_state[port_no].odu_opu_test;
    }
    VTSS_EXIT();
    return rc;
}



/* ================================================================= *
 *  ODUT Config
 * ================================================================= */
vtss_rc vtss_otn_odut_mode_set(const vtss_inst_t              inst,
                               const vtss_port_no_t           port_no,
                               const u32                      level,
                               const vtss_otn_odut_mode_t     *const cfg)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && cfg) {
        rc = VTSS_RC_ERROR;
        if (level < VTSS_OTN_ODUT_LEVEL_NUMBER) {
            vtss_state->otn_state[port_no].odut_mode[level] = *cfg;
            rc = VTSS_FUNC_COLD(cil.otn_odut_mode_set, port_no, level);
        }
        
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_odut_mode_get(const vtss_inst_t      inst,
                               const vtss_port_no_t   port_no,
                               const u32              level,
                               vtss_otn_odut_mode_t   *const cfg)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && cfg) {
        if (level < VTSS_OTN_ODUT_LEVEL_NUMBER)
            *cfg = vtss_state->otn_state[port_no].odut_mode[level];
        else
            rc = VTSS_RC_ERROR;
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_odut_tti_set(const vtss_inst_t           inst,
                              const vtss_port_no_t        port_no,
                              const u32                   level,
                              const vtss_otn_odut_tti_t   *const cfg)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && cfg) {
        rc = VTSS_RC_ERROR;
        if (level < VTSS_OTN_ODUT_LEVEL_NUMBER) {
            vtss_state->otn_state[port_no].odut_tti[level] = *cfg;
            rc = VTSS_FUNC_COLD(cil.otn_odut_tti_set, port_no, level);
        }
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_odut_tti_get(const vtss_inst_t       inst,
                              const vtss_port_no_t    port_no,
                              const u32               level,
                              vtss_otn_odut_tti_t     *const cfg)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && cfg) {
        if (level < VTSS_OTN_ODUT_LEVEL_NUMBER)
            *cfg = vtss_state->otn_state[port_no].odut_tti[level];
        else
            rc = VTSS_RC_ERROR;
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_odut_acc_tti_get(const vtss_inst_t          inst,
                                  const vtss_port_no_t       port_no,
                                  const u32                  level,
                                  vtss_otn_odut_acc_tti_t    *const tti)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && tti) {
        rc = VTSS_RC_ERROR;
        if (level < VTSS_OTN_ODUT_LEVEL_NUMBER)
            rc = VTSS_FUNC_COLD(cil.otn_odut_acc_tti_get, port_no, level, tti);
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_odut_consequent_actions_set(const vtss_inst_t                inst,
                                             const vtss_port_no_t             port_no,
                                             const u32                        level,
                                             const vtss_otn_odut_cons_act_t   *const cfg)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && cfg) {
        rc = VTSS_RC_ERROR;
        if (level < VTSS_OTN_ODUT_LEVEL_NUMBER) {
            vtss_state->otn_state[port_no].odut_cons[level] = *cfg;
            rc = VTSS_FUNC_COLD(cil.otn_odut_consequent_actions_set, port_no, level);
        }
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_odut_consequent_actions_get(const vtss_inst_t          inst,
                                             const vtss_port_no_t       port_no,
                                             const u32                  level,
                                             vtss_otn_odut_cons_act_t   *const cfg)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && cfg) {
        if (level < VTSS_OTN_ODUT_LEVEL_NUMBER)
            *cfg = vtss_state->otn_state[port_no].odut_cons[level];
        else
            rc = VTSS_RC_ERROR;
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_odut_bdi_set(const vtss_inst_t             inst,
                              const vtss_port_no_t          port_no,
                              const u32                     level,
                              const vtss_otn_odut_bdi_t     *const cfg)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && cfg) {
        rc = VTSS_RC_ERROR;
        if (level < VTSS_OTN_ODUT_LEVEL_NUMBER) {
            vtss_state->otn_state[port_no].odut_bdi[level] = *cfg;
            rc = VTSS_FUNC_COLD(cil.otn_odut_bdi_set, port_no, level);
        }
        
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_odut_bdi_get(const vtss_inst_t      inst,
                              const vtss_port_no_t   port_no,
                              const u32              level,
                              vtss_otn_odut_bdi_t   *const cfg)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && cfg) {
        if (level < VTSS_OTN_ODUT_LEVEL_NUMBER)
            *cfg = vtss_state->otn_state[port_no].odut_bdi[level];
        else
            rc = VTSS_RC_ERROR;
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_odut_tx_stat_set(const vtss_inst_t               inst,
                                  const vtss_port_no_t            port_no,
                                  const u32                       level,
                                  const vtss_otn_odut_tx_stat_t   *const cfg)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && cfg) {
        rc = VTSS_RC_ERROR;
        if (level < VTSS_OTN_ODUT_LEVEL_NUMBER) {
            vtss_state->otn_state[port_no].odut_stat[level] = *cfg;
            rc = VTSS_FUNC_COLD(cil.otn_odut_tx_stat_set, port_no, level);
        }
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_odut_tx_stat_get(const vtss_inst_t         inst,
                                  const vtss_port_no_t      port_no,
                                  const u32                 level,
                                  vtss_otn_odut_tx_stat_t   *const cfg)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && cfg) {
        if (level < VTSS_OTN_ODUT_LEVEL_NUMBER)
            *cfg = vtss_state->otn_state[port_no].odut_stat[level];
        else
            rc = VTSS_RC_ERROR;
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_odut_acc_stat_get(const vtss_inst_t          inst,
                                   const vtss_port_no_t       port_no,
                                   const u32                  level,
                                   vtss_otn_odut_acc_stat_t   *const stat)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && stat) {
        if (level < VTSS_OTN_ODUT_LEVEL_NUMBER)
            rc = VTSS_FUNC_COLD(cil.otn_odut_acc_stat_get, port_no, level, stat);
        else
            rc = VTSS_RC_ERROR;
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_odut_tx_aps_set(const vtss_inst_t              inst,
                                 const vtss_port_no_t           port_no,
                                 const u32                      level,
                                 const vtss_otn_odut_tx_aps_t   *const cfg)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && cfg) {
        rc = VTSS_RC_ERROR;
        if (level < VTSS_OTN_ODUT_LEVEL_NUMBER) {
            vtss_state->otn_state[port_no].odut_aps[level] = *cfg;
            rc = VTSS_FUNC_COLD(cil.otn_odut_tx_aps_set, port_no, level);
        }
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_odut_tx_aps_get(const vtss_inst_t        inst,
                                 const vtss_port_no_t     port_no,
                                 const u32                level,
                                 vtss_otn_odut_tx_aps_t   *const cfg)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && cfg) {
        if (level < VTSS_OTN_ODUT_LEVEL_NUMBER)
            *cfg = vtss_state->otn_state[port_no].odut_aps[level];
        else
            rc = VTSS_RC_ERROR;
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_odut_acc_aps_get(const vtss_inst_t         inst,
                                  const vtss_port_no_t      port_no,
                                  const u32                 level,
                                  vtss_otn_odut_acc_aps_t   *const aps)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && aps) {
        if (level < VTSS_OTN_ODUT_LEVEL_NUMBER)
            rc = VTSS_FUNC_COLD(cil.otn_odut_acc_aps_get, port_no, level, aps);
        else
            rc = VTSS_RC_ERROR;
    }
    VTSS_EXIT();
    return rc;
}



/* ================================================================= *
 *  State Reporting
 * ================================================================= */
vtss_rc vtss_otn_och_defects_get(const vtss_inst_t       inst,
                                 const vtss_port_no_t    port_no,
                                 vtss_otn_och_defects_t  *const defects)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && defects) {
        rc = VTSS_FUNC_COLD(cil.otn_och_defects_get, port_no, defects);
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_otu_defects_get(const vtss_inst_t       inst,
                                 const vtss_port_no_t    port_no,
                                 vtss_otn_otu_defects_t  *const defects)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && defects) {
        rc = VTSS_FUNC_COLD(cil.otn_otu_defects_get, port_no, defects);
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_odu_defects_get(const vtss_inst_t       inst,
                                 const vtss_port_no_t    port_no,
                                 vtss_otn_odu_defects_t  *const defects)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && defects) {
        rc = VTSS_FUNC_COLD(cil.otn_odu_defects_get, port_no, defects);
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_odut_defects_get(const vtss_inst_t        inst,
                                  const vtss_port_no_t     port_no,
                                  const u32                level,
                                  vtss_otn_odut_defects_t  *const defects)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && defects) {
        rc = VTSS_RC_ERROR;
        if (level < VTSS_OTN_ODUT_LEVEL_NUMBER)
            rc = VTSS_FUNC_COLD(cil.otn_odut_defects_get, port_no, level, defects);
    }
    VTSS_EXIT();
    return rc;
}



/* ================================================================= *
 *  Performance Primitives
 * ================================================================= */
vtss_rc vtss_otn_och_perf_get(const vtss_inst_t      inst,
                              const vtss_port_no_t   port_no,
                              vtss_otn_och_perf_t    *const perf)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && perf) {
        rc = VTSS_FUNC_COLD(cil.otn_och_perf_get, port_no, perf);
    }
    VTSS_EXIT();
    return rc;
}



vtss_rc vtss_otn_otu_perf_get(const vtss_inst_t      inst,
                              const vtss_port_no_t   port_no,
                              vtss_otn_otu_perf_t    *const perf)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && perf) {
        rc = VTSS_FUNC_COLD(cil.otn_otu_perf_get, port_no, perf);
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_odu_perf_get(const vtss_inst_t      inst,
                              const vtss_port_no_t   port_no,
                              vtss_otn_odu_perf_t    *const perf)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && perf) {
        rc = VTSS_FUNC_COLD(cil.otn_odu_perf_get, port_no, perf);
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_odut_perf_get(const vtss_inst_t       inst,
                               const vtss_port_no_t    port_no,
                               const u32               level,
                               vtss_otn_odut_perf_t    *const perf)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && perf) {
        rc = VTSS_RC_ERROR;
        if (level < VTSS_OTN_ODUT_LEVEL_NUMBER)
            rc = VTSS_FUNC_COLD(cil.otn_odut_perf_get, port_no, level, perf);
    }
    VTSS_EXIT();
    return rc;
}



/* ================================================================= *
 *  Defects/Events
 * ================================================================= */
vtss_rc vtss_otn_och_event_enable(const vtss_inst_t            inst,
                                  const vtss_port_no_t         port_no,
                                  const BOOL                   enable,
                                  const vtss_otn_och_event_t   ev_mask)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        rc = VTSS_FUNC_COLD(cil.otn_och_event_enable, port_no, enable, ev_mask);
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_och_event_poll(const vtss_inst_t        inst,
                                const vtss_port_no_t     port_no,
                                vtss_otn_och_event_t     *const status)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && status) {
        rc = VTSS_FUNC_COLD(cil.otn_och_event_poll, port_no, status);
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_och_event_poll_without_mask(const vtss_inst_t        inst,
                                             const vtss_port_no_t     port_no,
                                             vtss_otn_och_event_t     *const status)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && status) {
        rc = VTSS_FUNC_COLD(cil.otn_och_event_poll_without_mask, port_no, status);
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_otu_event_enable(const vtss_inst_t            inst,
                                  const vtss_port_no_t         port_no,
                                  const BOOL                   enable,
                                  const vtss_otn_otu_event_t   ev_mask)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        rc = VTSS_FUNC_COLD(cil.otn_otu_event_enable, port_no, enable, ev_mask);
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_otu_event_poll(const vtss_inst_t        inst,
                                const vtss_port_no_t     port_no,
                                vtss_otn_otu_event_t     *const status)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && status) {
        rc = VTSS_FUNC_COLD(cil.otn_otu_event_poll, port_no, status);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_otn_otu_event_poll_without_mask(const vtss_inst_t        inst,
                                const vtss_port_no_t     port_no,
                                vtss_otn_otu_event_t     *const status)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && status) {
        rc = VTSS_FUNC_COLD(cil.otn_otu_event_poll_without_mask, port_no, status);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_otn_odu_event_enable(const vtss_inst_t            inst,
                                  const vtss_port_no_t         port_no,
                                  const BOOL                   enable,
                                  const vtss_otn_odu_event_t   ev_mask)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        rc = VTSS_FUNC_COLD(cil.otn_odu_event_enable, port_no, enable, ev_mask);
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_odu_event_poll(const vtss_inst_t        inst,
                                const vtss_port_no_t     port_no,
                                vtss_otn_odu_event_t     *const status)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && status) {
        rc = VTSS_FUNC_COLD(cil.otn_odu_event_poll, port_no, status);
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_odu_event_poll_without_mask(const vtss_inst_t        inst,
                                const vtss_port_no_t     port_no,
                                vtss_otn_odu_event_t     *const status)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && status) {
        rc = VTSS_FUNC_COLD(cil.otn_odu_event_poll_without_mask, port_no, status);
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_odut_event_enable(const vtss_inst_t             inst,
                                   const vtss_port_no_t          port_no,
                                   const u32                     level,
                                   const BOOL                    enable,
                                   const vtss_otn_odut_event_t   ev_mask)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        rc = VTSS_RC_ERROR;
        if (level < VTSS_OTN_ODUT_LEVEL_NUMBER)
            rc = VTSS_FUNC_COLD(cil.otn_odut_event_enable, port_no, level, enable, ev_mask);
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_otn_odut_event_poll(const vtss_inst_t         inst,
                                 const vtss_port_no_t      port_no,
                                 const u32                 level,
                                 vtss_otn_odut_event_t     *const status)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && status) {
        rc = VTSS_RC_ERROR;
        if (level < VTSS_OTN_ODUT_LEVEL_NUMBER)
            rc = VTSS_FUNC_COLD(cil.otn_odut_event_poll, port_no, level, status);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_otn_odut_event_poll_without_mask(const vtss_inst_t         inst,
                                              const vtss_port_no_t      port_no,
                                              const u32                 level,
                                              vtss_otn_odut_event_t     *const status)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && status) {
        rc = VTSS_RC_ERROR;
        if (level < VTSS_OTN_ODUT_LEVEL_NUMBER)
            rc = VTSS_FUNC_COLD(cil.otn_odut_event_poll_without_mask, port_no, level, status);
    }
    VTSS_EXIT();
    return rc;
}


/* ================================================================= *
 *  EFEC events
 * ================================================================= */
/*
vtss_rc vtss_efec_event_enable(const vtss_inst_t            inst,
                                  const vtss_port_no_t         port_no,
                                  const BOOL                   enable,
                                  const vtss_efec_event_t   ev_mask)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        rc = VTSS_FUNC_COLD(cil.efec_event_enable, port_no, enable, ev_mask);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_efec_event_poll(const vtss_inst_t        inst,
                                const vtss_port_no_t     port_no,
                                vtss_efec_event_t     *const status)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && status) {
        rc = VTSS_FUNC_COLD(cil.efec_event_poll, port_no, status);
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_efec_event_poll_without_mask(const vtss_inst_t        inst,
                                             const vtss_port_no_t     port_no,
                                             vtss_efec_event_t     *const status)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && status) {
        rc = VTSS_FUNC_COLD(cil.efec_event_poll_without_mask, port_no, status);
    }
    VTSS_EXIT();
    return rc;
}
*/
/* ================================================================= *
 *  Test/Debug Config
 * ================================================================= */


#endif /*VTSS_FEATURE_xfi */
