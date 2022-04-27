/*

 Vitesse Switch API software.

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

*/

#include "mpls.h"
#include "mpls_api.h"
#include "msg_api.h"

#define VTSS_RC(expr)   { vtss_rc __rc__ = (expr); if (__rc__ < VTSS_RC_OK) return __rc__; }

// Critical region
critd_t vtss_mpls_crit;

#if (VTSS_TRACE_ENABLED)
static vtss_trace_reg_t trace_reg = {
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "MPLS",
    .descr     = "MPLS/MPLS-TP"
};

static vtss_trace_grp_t trace_grps[TRACE_GRP_CNT] = {
    [VTSS_TRACE_GRP_DEFAULT] = {
        .name      = "default",
        .descr     = "Default",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [TRACE_GRP_CRIT] = {
        .name      = "crit",
        .descr     = "Critical regions ",
        .lvl       = VTSS_TRACE_LVL_ERROR,
        .timestamp = 1,
    }
};
#endif /* VTSS_TRACE_ENABLED */



/* Since the MPLS application is still under development, the restore-to-default
 * code isn't production-level either. We utilize a function in the MPLS ICLI code
 * for that purpose.
 */
extern void mpls_load_defaults_non_production(void);  // in mpls.icli
static void mpls_load_defaults(void)
{
    T_D("MPLS load defaults begin");
    mpls_load_defaults_non_production();
    T_D("MPLS load defaults done");
}

vtss_rc mpls_init(vtss_init_data_t *data)
{
    switch (data->cmd) {
    case INIT_CMD_INIT:
        /* Initialize and register trace resources */
        VTSS_TRACE_REG_INIT(&trace_reg, trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&trace_reg);
        critd_init(&vtss_mpls_crit, "mpls_crit", VTSS_MODULE_ID_MPLS, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
        MPLS_CRIT_EXIT();
        break;

    case INIT_CMD_CONF_DEF:
        if (data->isid == VTSS_ISID_GLOBAL) {
            mpls_load_defaults();
        }
        break;

    case INIT_CMD_MASTER_UP:
        break;

    case INIT_CMD_MASTER_DOWN:
        break;

    default:
        break;
    }

    return VTSS_OK;
}

