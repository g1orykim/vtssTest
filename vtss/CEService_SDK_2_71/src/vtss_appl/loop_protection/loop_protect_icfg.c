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
/*
******************************************************************************

    Revision history
    > CP.Wang, 2013/01/02 11:34
        - create

******************************************************************************
*/

/*
******************************************************************************

    Include files

******************************************************************************
*/
#include <stdlib.h>
#include "icfg_api.h"
#include "loop_protect_api.h"
#include "topo_api.h"
#include "misc_api.h"

/*
******************************************************************************

    Constant and Macro definition

******************************************************************************
*/

/*
******************************************************************************

    Data structure type definition

******************************************************************************
*/

/*
******************************************************************************

    Static Function

******************************************************************************
*/
/* ICFG callback functions */
static vtss_rc _loop_protect_icfg(
    IN const vtss_icfg_query_request_t  *req,
    IN vtss_icfg_query_result_t         *result
)
{
    vtss_rc                     rc;
    vtss_isid_t                 isid;
    vtss_port_no_t              iport;
    loop_protect_conf_t         sc;
    loop_protect_port_conf_t    conf;

    if ( req == NULL ) {
        return VTSS_RC_ERROR;
    }

    if ( result == NULL ) {
        return VTSS_RC_ERROR;
    }

    switch ( req->cmd_mode ) {
    case ICLI_CMD_MODE_GLOBAL_CONFIG:
        rc = loop_protect_conf_get( &sc );
        if ( rc != VTSS_OK ) {
            return rc;
        }
    
        if ( sc.enabled != LOOP_PROTECT_DEFAULT_GLOBAL_ENABLED ) {
            (void)vtss_icfg_printf(result, "%sloop-protect\n", sc.enabled ? "" : "no ");
        } else if ( req->all_defaults ) {
            (void)vtss_icfg_printf(result, "%sloop-protect\n", sc.enabled ? "" : "no ");
        }

        if ( sc.transmission_time != LOOP_PROTECT_DEFAULT_GLOBAL_TX_TIME ) {
            (void)vtss_icfg_printf(result, "loop-protect transmit-time %d\n", sc.transmission_time);
        } else if ( req->all_defaults ) {
            (void)vtss_icfg_printf(result, "loop-protect transmit-time %u\n", LOOP_PROTECT_DEFAULT_GLOBAL_TX_TIME);
        }

        if ( sc.shutdown_time != LOOP_PROTECT_DEFAULT_GLOBAL_SHUTDOWN_TIME ) {
            (void)vtss_icfg_printf(result, "loop-protect shutdown-time %d\n", sc.shutdown_time);
        } else if ( req->all_defaults ) {
            (void)vtss_icfg_printf(result, "loop-protect shutdown-time %u\n", LOOP_PROTECT_DEFAULT_GLOBAL_SHUTDOWN_TIME);
        }
        break;

    case ICLI_CMD_MODE_INTERFACE_PORT_LIST:
        /* get isid and iport */
        isid = topo_usid2isid(req->instance_id.port.usid);
        iport = uport2iport(req->instance_id.port.begin_uport);

        rc = loop_protect_conf_port_get(isid, iport, &conf);
        if ( rc != VTSS_OK ) {
            return rc;
        }

        if ( conf.enabled != LOOP_PROTECT_DEFAULT_PORT_ENABLED ) {
            (void)vtss_icfg_printf(result, " %sloop-protect\n", conf.enabled ? "" : "no ");
        } else if ( req->all_defaults ) {
            (void)vtss_icfg_printf(result, " %sloop-protect\n", conf.enabled ? "" : "no ");
        }

        if ( conf.action != LOOP_PROTECT_DEFAULT_PORT_ACTION ) {
            switch ( conf.action ) {
            case LOOP_PROTECT_ACTION_SHUTDOWN:
                (void)vtss_icfg_printf(result, " loop-protect action shutdown\n");
                break;
            case LOOP_PROTECT_ACTION_SHUT_LOG:
                (void)vtss_icfg_printf(result, " loop-protect action shutdown log\n");
                break;
            case LOOP_PROTECT_ACTION_LOG_ONLY:
                (void)vtss_icfg_printf(result, " loop-protect action log\n");
                break;
            default:
                break;
            }
        } else if ( req->all_defaults ) {
            (void)vtss_icfg_printf(result, " no loop-protect action\n");
        }

        if ( conf.transmit != LOOP_PROTECT_DEFAULT_PORT_TX_MODE ) {
            (void)vtss_icfg_printf(result, " %sloop-protect tx-mode\n", conf.transmit ? "" : "no ");
        } else if ( req->all_defaults ) {
            (void)vtss_icfg_printf(result, " %sloop-protect tx-mode\n", conf.transmit ? "" : "no ");
        }
        break;

    default:
        /* no config in other modes */
        break;
    }
    return VTSS_OK;
}

/*
******************************************************************************

    Public functions

******************************************************************************
*/

/* Initialization function */
vtss_rc loop_protect_icfg_init(void)
{
    vtss_rc     rc;

    /*
        Register Global config callback functions to ICFG module.
    */
    rc = vtss_icfg_query_register(VTSS_ICFG_GLOBAL_LOOP_PROTECT, "loop-protect", _loop_protect_icfg);
    if ( rc != VTSS_OK ) {
        return rc;
    }

    /*
        Register Interface port list config callback functions to ICFG module.
    */
    rc = vtss_icfg_query_register(VTSS_ICFG_INTERFACE_ETHERNET_LOOP_PROTECT, "loop-protect", _loop_protect_icfg);
    return rc;
}
