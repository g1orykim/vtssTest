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

// Avoid "vtss_api.h not used in module vtss_sf4_api.c"
/*lint --e{766} */

#include "vtss_api.h"
#if defined(VTSS_FEATURE_SFI4)

#include "vtss_state.h"
#include "vtss_sfi4_api.h"

/* ================================================================= *
 *  Defects/Events
 * ================================================================= */

/* ================================================================= *
 *  Dynamic Config
 * ================================================================= */

vtss_rc vtss_sfi4_set_config(vtss_inst_t inst,
                          vtss_port_no_t port_no,
                          const vtss_sfi4_cfg_t *const cfg)
{
    vtss_rc rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, port_no)) == VTSS_RC_OK && cfg) {
        vtss_state->sfi4_state[port_no].sfi4_cfg = *cfg;
        rc = VTSS_FUNC_COLD(sfi4_set_config, port_no);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_sfi4_get_config(vtss_inst_t inst,
                                   vtss_port_no_t port_no,
                                   vtss_sfi4_cfg_t *const cfg)
{
	vtss_rc rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, port_no)) == VTSS_RC_OK && cfg) {
        *cfg = vtss_state->sfi4_state[port_no].sfi4_cfg;
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_sfi4_set_enable(const vtss_inst_t inst,
                          vtss_port_no_t port_no,
                          BOOL enable
                          )
{
    vtss_rc rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, port_no)) == VTSS_RC_OK) {
        vtss_state->sfi4_state[port_no].sfi4_cfg.enable = enable;
        rc = VTSS_FUNC_COLD(sfi4_set_enable, port_no);
    }
    VTSS_EXIT();
    return rc;	
}

vtss_rc vtss_sfi4_set_loopback(const vtss_inst_t inst,
                          vtss_port_no_t port_no,
                          BOOL enable,
                          vtss_sfi4_loopback_t dir
                          )
{
    vtss_rc rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, port_no)) == VTSS_RC_OK) {  

        (dir == sfi4_rx_to_tx) ? (vtss_state->sfi4_state[port_no].sfi4_cfg.rx_to_tx_loopback = enable):0;
        (dir == sfi4_tx_to_rx) ? (vtss_state->sfi4_state[port_no].sfi4_cfg.tx_to_rx_loopback = enable):0;

        rc = VTSS_FUNC_COLD(sfi4_set_loopback, port_no);
    }
    VTSS_EXIT();
    return rc;	
}





/* ================================================================= *
 *  State Reporting
 * ================================================================= */

vtss_rc vtss_sfi4_events_get(vtss_inst_t inst,
                          vtss_port_no_t port_no,
                          vtss_sfi4_events_t *const events)
{
	return VTSS_RC_OK;
}

vtss_rc vtss_sfi4_get_status(vtss_inst_t inst,
                          vtss_port_no_t port_no,
                          vtss_sfi4_status_t *const status)
{
	return VTSS_RC_OK;
}


/* ================================================================= *
 *  Performance Primitives
 * ================================================================= */



#endif /*VTSS_FEATURE_SFI4 */
