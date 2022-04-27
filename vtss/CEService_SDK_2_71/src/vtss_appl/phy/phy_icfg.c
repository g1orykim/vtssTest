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

#include "icfg_api.h"
#include "icli_api.h"
#include "misc_api.h"
#include "icli_porting_util.h"
#include "phy_api.h"
#include "phy_icfg.h"
#include "topo_api.h"

#define VTSS_TRACE_MODULE_ID        VTSS_MODULE_ID_PHY

#define VTSS_PHY_DEFAULT_INST PHY_INST_NONE

//******************************************************************************
// ICFG callback functions
//******************************************************************************
static vtss_rc phy_icfg_global_conf(const vtss_icfg_query_request_t *req, vtss_icfg_query_result_t *result)
{
    T_D("Enter function: %s\n", __FUNCTION__);
    phy_inst_start_t    start_inst = VTSS_PHY_DEFAULT_INST;

    if (!req || !result) {
        return VTSS_RC_ERROR;
    }

    start_inst = phy_mgmt_start_inst_get();
    if (req->all_defaults || start_inst != VTSS_PHY_DEFAULT_INST) {
        if (start_inst != VTSS_PHY_DEFAULT_INST) {
            VTSS_RC(vtss_icfg_printf(result, "platform phy instance %s\n", start_inst == PHY_INST_1G_PHY ? "1g" : "10g"));
        }
    }
    if (req->all_defaults && start_inst == VTSS_PHY_DEFAULT_INST) {
        VTSS_RC(vtss_icfg_printf(result, "%s platform phy instance\n", "no"));
    }

    T_D("Exit function: %s\n", __FUNCTION__);
    return VTSS_OK;
}

#if defined(VTSS_CHIP_10G_PHY) && defined(VTSS_CHIP_10G_PHY_SAVE_FAILOVER_IN_CFG)
/* returns TRUE, if failover is enabled otherwise returns FALSE
*
* TBD>>>  In vcli failover configuration was not saved, should we save it in icli?
*
*/
static BOOL phy_intf_failover_get(vtss_isid_t isid, vtss_port_no_t iport)
{
    vtss_phy_10g_failover_mode_t   phy_failover, api_failover;
    BOOL                           active, next, bc;
    vtss_phy_10g_id_t              phy_id;

    vtss_phy_10g_failover_get(PHY_INST, iport, &api_failover);

    phy_mgmt_failover_get(iport, &phy_failover);

    active = 0;
    next = 0;
    bc = 0;
    if ((phy_id.channel_id == 0) && (api_failover == VTSS_PHY_10G_PMA_TO_FROM_XAUI_NORMAL)) {
        active = 1;
    }

    if (((phy_id.channel_id == 0) && (api_failover == VTSS_PHY_10G_PMA_0_TO_FROM_XAUI_0_TO_XAUI_1)) ||
        ((phy_id.channel_id == 1) && (api_failover == VTSS_PHY_10G_PMA_0_TO_FROM_XAUI_1_TO_XAUI_0))) {
        active = 1;
        bc = 1;
    }
    if (((phy_id.channel_id == 0) && (phy_failover == VTSS_PHY_10G_PMA_0_TO_FROM_XAUI_0_TO_XAUI_1)) ||
        (((phy_id.channel_id == 1) && (phy_failover == VTSS_PHY_10G_PMA_0_TO_FROM_XAUI_1_TO_XAUI_0)))) {
        next = 1;
        bc = 1;
    }
    return active;
}

static vtss_rc phy_icfg_intf_conf ( const vtss_icfg_query_request_t *req,
                                    vtss_icfg_query_result_t *result
                                  )
{
    T_D("Enter function: %s\n", __FUNCTION__);
    vtss_isid_t                    isid;
    vtss_port_no_t                 iport;
    BOOL                           mode;

    if (!req || !result) {
        return VTSS_RC_ERROR;
    }

    isid = topo_usid2isid(req->instance_id.port.usid);
    iport = uport2iport(req->instance_id.port.begin_uport);

    if (!vtss_phy_10G_is_valid(PHY_INST, iport)) {
        return VTSS_OK; /* failover is only support on 10G PHYs */
    }

    mode = phy_intf_failover_get(isid, iport);

    VTSS_RC(vtss_icfg_printf(result, " %splatform phy failover\n", mode ? "no " : ""));

    T_D("Exit function: %s\n", __FUNCTION__);
    return VTSS_OK;
}
#endif

//******************************************************************************
//   Public functions
//******************************************************************************

vtss_rc phy_icfg_init(void)
{
    T_I("Enter proc phy_icfg_init\n");
    vtss_rc rc = VTSS_OK;
#if defined(VTSS_CHIP_10G_PHY) && defined(VTSS_CHIP_10G_PHY_SAVE_FAILOVER_IN_CFG)
    if ((rc = vtss_icfg_query_register(VTSS_ICFG_PHY_INTERFACE_CONF, "phy", phy_icfg_intf_conf )) != VTSS_OK) {
        return rc;
    }
#endif

    if ((rc = vtss_icfg_query_register(VTSS_ICFG_PHY_GLOBAL_CONF, "phy", phy_icfg_global_conf)) != VTSS_OK) {
        return rc;
    }
    T_I("Exit proc phy_icfg_init\n");
    return rc;
}
