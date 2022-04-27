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

/**
 * \file vtss_pcs_10gbase_r_api.c
 * \brief  PCS10G base R  API
 */

#ifndef _VTSS_PCS_10GBASE_R_API_
#define _VTSS_PCS_10GBASE_R_API_

#include <vtss_options.h>
#include <vtss_types.h>
#include <vtss_api.h>
#include <vtss_port_api.h>
#include "vtss_state.h"
#include <vtss_pcs_10gbase_r_api.h>

#if defined(VTSS_FEATURE_PCS_10GBASE_R)

/* ================================================================= *
 *  Defects/Events
 * ================================================================= */

vtss_rc vtss_pcs_10gbase_r_event_enable(const vtss_inst_t                   inst,
                                        const vtss_port_no_t                port_no,
                                        const BOOL                          enable,
                                        const vtss_pcs_10gbase_r_event_t    ev_mask)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK ) {
        rc = VTSS_FUNC_COLD(cil.pcs_10gbase_r_event_enable, port_no, enable, ev_mask);
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_pcs_10gbase_r_event_poll(const vtss_inst_t            inst,
                                      const vtss_port_no_t         port_no,
                                      vtss_pcs_10gbase_r_event_t   *const status)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK ) {
        rc = VTSS_FUNC_COLD(cil.pcs_10gbase_r_event_poll, port_no, status);
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_pcs_10gbase_r_event_poll_without_mask(const vtss_inst_t            inst,
                                                   const vtss_port_no_t         port_no,
                                                   vtss_pcs_10gbase_r_event_t   *const status)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        rc = VTSS_FUNC_COLD(cil.pcs_10gbase_r_event_poll_without_mask, port_no, status);
    }
    VTSS_EXIT();
    return rc;
}


/* ================================================================= *
 *  Dynamic Config
 * ================================================================= */
vtss_rc vtss_pcs_10gbase_r_config_set(const vtss_inst_t inst,
                                      const vtss_port_no_t port_no,
                                      const vtss_pcs_10gbase_r_cfg_t *const cfg)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && cfg) {
        vtss_state->pcs_10gbase_r_state[port_no].pcs_10gbase_r_cfg = *cfg;
        rc = VTSS_FUNC_COLD(cil.pcs_10gbase_r_config_set, port_no);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_pcs_10gbase_r_config_get(const vtss_inst_t inst,
                                      const vtss_port_no_t port_no,
                                      vtss_pcs_10gbase_r_cfg_t * const cfg)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && cfg) {
        *cfg = vtss_state->pcs_10gbase_r_state[port_no].pcs_10gbase_r_cfg;
    }
    VTSS_EXIT();
    return rc;
}

/* ================================================================= *
 *  State Reporting
 * ================================================================= */
vtss_rc vtss_pcs_10gbase_r_status_get(const vtss_inst_t inst,
                                      const vtss_port_no_t port_no,
                                      vtss_pcs_10gbase_r_status_t * const status)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && status) {
        rc = VTSS_FUNC_COLD(cil.pcs_10gbase_r_status_get, port_no, status);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_pcs_10gbase_r_counters_update(const vtss_inst_t inst,
                                           const vtss_port_no_t port_no)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        rc = VTSS_FUNC_COLD(cil.pcs_10gbase_r_counters_update, port_no);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_pcs_10gbase_r_counters_clear(const vtss_inst_t inst,
                                          const vtss_port_no_t port_no)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        rc = VTSS_FUNC_COLD(cil.pcs_10gbase_r_counters_clear, port_no);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_pcs_10gbase_r_counters_get(const vtss_inst_t inst,
                                        const vtss_port_no_t port_no,
                                        vtss_pcs_10gbase_r_cnt_t * const counters)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && counters) {
        rc = VTSS_FUNC_COLD(cil.pcs_10gbase_r_counters_get, port_no, counters);
    }
    VTSS_EXIT();
    return rc;
}



/* ================================================================= *
 *  Performance Primitives
 * ================================================================= */


#endif /* VTSS_FEATURE_PCS_10GBASE_R */
#endif /* _VTSS_PCS_10GBASE_R_API_ */
