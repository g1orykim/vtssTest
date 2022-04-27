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

#if 0
#include "vtss_xxrp_types.h"
#include "vtss_xxrp_types.h"
#include "vtss_xxrp_os.h"
#include "vtss_xxrp_api.h"
#include "l2proto_api.h"
#endif



/*
 * Add stubs for unimplemented API functions here.
 * This file will be removed when code is complete.
 */

/***************************************************************************************************
 * Stubs (removed when implemented in base part)
 * Note that they will only work with one MRP application!
 **************************************************************************************************/
#if 0
static BOOL stub_global_enable;
u32 vtss_mrp_global_control_conf_set(vtss_mrp_appl_t application,
                                     BOOL            enable)
{
    if (stub_global_enable == enable) {
        if (enable) {
            T_W("already enabled");
            return VTSS_XXRP_RC_ALREADY_CONFIGURED;
        } else {
            T_W("already disabled");
            return VTSS_XXRP_RC_NOT_ENABLED;
        }
    }
    stub_global_enable = enable;
    T_I("global %s", enable ? "enabled" : "disabled");
    return VTSS_XXRP_RC_OK;
}

u32 vtss_mrp_global_control_conf_get(vtss_mrp_appl_t application,
                                     BOOL  *const    status)
{
    *status = stub_global_enable;
    return VTSS_XXRP_RC_OK;
}

static BOOL stub_port_enable[L2_MAX_PORTS];
u32 vtss_mrp_port_control_conf_set(vtss_mrp_appl_t application,
                                   u32             port_no,
                                   BOOL            enable)
{
    if (port_no >=  L2_MAX_PORTS) {
        T_E("port_no >=  L2_MAX_PORTS");
        return VTSS_XXRP_RC_INVALID_PARAMETER;
    }

    if (stub_port_enable[port_no] == enable) {
        if (enable) {
            T_W("already enabled");
            return VTSS_XXRP_RC_ALREADY_CONFIGURED;
        } else {
            T_W("already disabled");
            return VTSS_XXRP_RC_NOT_ENABLED;
        }
    }
    stub_port_enable[port_no] = enable;
    T_I("port %lu %s", port_no, enable ? "enabled" : "disabled");
    return VTSS_XXRP_RC_OK;
}

u32 vtss_mrp_port_control_conf_get(vtss_mrp_appl_t application,
                                   u32             port_no,
                                   BOOL  *const    status)
{
    if (port_no >=  L2_MAX_PORTS) {
        T_E("port_no >=  L2_MAX_PORTS");
        return VTSS_XXRP_RC_INVALID_PARAMETER;
    }

//    *status = stub_port_enable[port_no];
    *status = FALSE; // terrible hack until base module is ready
    return VTSS_XXRP_RC_OK;
}

static BOOL stub_periodic_enable[L2_MAX_PORTS];
u32 vtss_mrp_periodic_transmission_control_conf_set(vtss_mrp_appl_t application,
                                                    u32             port_no,
                                                    BOOL            enable)
{
    if (port_no >=  L2_MAX_PORTS) {
        T_E("port_no >=  L2_MAX_PORTS");
        return VTSS_XXRP_RC_INVALID_PARAMETER;
    }

    stub_periodic_enable[port_no] = enable;
    T_I("periodic port:%lu %s", port_no, enable ? "enabled" : "disabled");
    return VTSS_XXRP_RC_OK;
}

u32 vtss_mrp_periodic_transmission_control_conf_get(vtss_mrp_appl_t application,
                                                    u32             port_no,
                                                    BOOL  *const    status)
{
    if (port_no >=  L2_MAX_PORTS) {
        T_E("port_no >=  L2_MAX_PORTS");
        return VTSS_XXRP_RC_INVALID_PARAMETER;
    }

    *status = stub_periodic_enable[port_no];
    return VTSS_XXRP_RC_OK;
}

static vtss_mrp_timer_conf_t stub_timers[L2_MAX_PORTS];
u32 vtss_mrp_timers_conf_set(vtss_mrp_appl_t              application,
                             u32                          port_no,
                             vtss_mrp_timer_conf_t *const timers)
{
    if (port_no >=  L2_MAX_PORTS) {
        T_E("port_no >=  L2_MAX_PORTS");
        return VTSS_XXRP_RC_INVALID_PARAMETER;
    }

    stub_timers[port_no] = *timers;
    T_I("timers port:%lu j:%lu l:%lu la:%lu", port_no, timers->join_timer, timers->leave_timer, timers->leave_all_timer);
    return VTSS_XXRP_RC_OK;
}

u32 vtss_mrp_timers_conf_get(vtss_mrp_appl_t        application,
                             u32                    port_no,
                             vtss_mrp_timer_conf_t  *timers)
{
    if (port_no >=  L2_MAX_PORTS) {
        T_E("port_no >=  L2_MAX_PORTS");
        return VTSS_XXRP_RC_INVALID_PARAMETER;
    }

    *timers = stub_timers[port_no];
    return VTSS_XXRP_RC_OK;
}

u32 vtss_mrp_applicant_admin_control_conf_set(vtss_mrp_appl_t             application,
                                              u32                         port_no,
                                              vtss_mrp_attribute_type_t   attr_type,
                                              BOOL                        participant)
{
    if (port_no >=  L2_MAX_PORTS) {
        T_E("port_no >=  L2_MAX_PORTS");
        return VTSS_XXRP_RC_INVALID_PARAMETER;
    }

    T_I("applicant adm port:%lu attr_type:%lu participant:%s", port_no, attr_type.dummy, participant ? "normal-participant" : "non-participant");
    return VTSS_XXRP_RC_OK;
}

u32 vtss_mrp_applicant_admin_control_conf_get(vtss_mrp_appl_t             application,
                                              u32                         port_no,
                                              vtss_mrp_attribute_type_t   attr_type,
                                              BOOL  *const                status)
{
    if (port_no >=  L2_MAX_PORTS) {
        T_E("port_no >=  L2_MAX_PORTS");
        return VTSS_XXRP_RC_INVALID_PARAMETER;
    }

    T_W("Not implemented!");
    return VTSS_XXRP_RC_OK;
}

u32 vtss_mrp_mstp_port_state_change_handler(u32                                     port_no,
                                            u8                                      msti,
                                            vtss_mrp_mstp_port_state_change_type_t  port_state_type)
{
    if (port_no >=  L2_MAX_PORTS) {
        T_E("port_no >=  L2_MAX_PORTS");
        return VTSS_XXRP_RC_INVALID_PARAMETER;
    }

    //T_I("mstp state port:%lu msti:%u state:%s", port_no, msti, (port_state_type == VTSS_MRP_MSTP_PORT_ADD) ? "add" : "delete");
    return VTSS_XXRP_RC_OK;
}


u32 vtss_mrp_timer_tick(u32 delay)
{
    static int state = 0;
    u32        ret;

    switch (state) {
    case 0:
        state = 1;
        ret = 100;
        break;
    case 1:
        state = 2;
        ret = 500;
        break;
    case 2:
        state = 3;
        ret = 1000;
        break;
    default:
        state = 0;
        ret = 0;
        break;
    }
    T_I("Tick delay %lu, ret %lu", delay, ret);

    return ret;
}

BOOL vtss_mrp_mrpdu_rx(u32 port_no, const u8 *mrpdu, u32 length)
{
    T_N("MRPDU rx, port %lu, len %lu", port_no, length);
    return TRUE;
}
#endif

