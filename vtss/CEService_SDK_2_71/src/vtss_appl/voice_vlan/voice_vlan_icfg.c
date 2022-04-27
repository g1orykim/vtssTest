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
    > CP.Wang, 2012/10/18 10:10
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
#include "voice_vlan_api.h"
#include "topo_api.h"
#include "misc_api.h"
#include "mgmt_api.h"

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
static vtss_rc _voice_vlan_icfg(
    IN const vtss_icfg_query_request_t  *req,
    IN vtss_icfg_query_result_t         *result
)
{
    vtss_rc                 rc;
    voice_vlan_conf_t       conf;
    voice_vlan_oui_entry_t  oui_entry;
    vtss_isid_t             isid;
    vtss_port_no_t          iport;
    voice_vlan_port_conf_t  port_conf;

    if ( req == NULL ) {
        //T_E("req == NULL\n");
        return VTSS_RC_ERROR;
    }

    if ( result == NULL ) {
        //T_E("result == NULL\n");
        return VTSS_RC_ERROR;
    }

    switch ( req->cmd_mode ) {
    case ICLI_CMD_MODE_GLOBAL_CONFIG:
        rc = voice_vlan_mgmt_conf_get( &conf );
        if ( rc == VTSS_OK ) {
            // voice vlan
            if ( conf.mode ) {
                (void)vtss_icfg_printf(result, "voice vlan\n");
            } else if ( req->all_defaults ) {
                (void)vtss_icfg_printf(result, "no voice vlan\n");
            }

            // voice vlan vid <vid:1-4094>
            if ( conf.vid != VOICE_VLAN_MGMT_DEFAULT_VID || req->all_defaults ) {
                (void)vtss_icfg_printf(result, "voice vlan vid %d\n", conf.vid);
            }

            // voice vlan aging-time <aging_time:10-10000000>
            if ( conf.age_time != VOICE_VLAN_MGMT_DEFAULT_AGE_TIME || req->all_defaults ) {
                (void)vtss_icfg_printf(result, "voice vlan aging-time %u\n", conf.age_time);
            }

            // voice vlan class <cos:0-7>
            if ( conf.traffic_class != VOICE_VLAN_MGMT_DEFAULT_TRAFFIC_CLASS || req->all_defaults ) {
                (void)vtss_icfg_printf(result, "voice vlan class %u\n", conf.traffic_class);
            }
        } else {
            //T_E("voice_vlan_mgmt_conf_get()\n");
        }

        // voice vlan oui <oui> [description <line>]
        memset(&oui_entry, 0, sizeof(oui_entry));
        while (voice_vlan_oui_entry_get(&oui_entry, TRUE) == VTSS_OK) {
            (void)vtss_icfg_printf(result, "voice vlan oui %02X-%02X-%02X",
                                   oui_entry.oui_addr[0], oui_entry.oui_addr[1], oui_entry.oui_addr[2]);

            if ( oui_entry.description[0] ) {
                (void)vtss_icfg_printf(result, " description %s", oui_entry.description);
            }
            (void)vtss_icfg_printf(result, "\n");
        };
        break;

    case ICLI_CMD_MODE_INTERFACE_PORT_LIST:
        /* get isid and iport */
        isid = topo_usid2isid(req->instance_id.port.usid);
        iport = uport2iport(req->instance_id.port.begin_uport);

        rc = voice_vlan_mgmt_port_conf_get(isid, &port_conf);
        if ( rc == VTSS_OK ) {
            // switchport voice vlan mode {auto | force | disable}
            // no switchport voice vlan mode
            switch ( port_conf.port_mode[iport] ) {
            case VOICE_VLAN_PORT_MODE_AUTO:
                (void)vtss_icfg_printf(result, " switchport voice vlan mode auto\n");
                break;

            case VOICE_VLAN_PORT_MODE_FORCED:
                (void)vtss_icfg_printf(result, " switchport voice vlan mode force\n");
                break;

            case VOICE_VLAN_PORT_MODE_DISABLED:
                if ( req->all_defaults ) {
                    (void)vtss_icfg_printf(result, " switchport voice vlan mode disable\n");
                }
                break;

            default:
                //T_E("unknown port mode %d\n", port_conf.port_mode[iport]);
                break;
            }

            // switchport voice vlan security
            if ( port_conf.security[iport] ) {
                (void)vtss_icfg_printf(result, " switchport voice vlan security\n");
            } else if ( req->all_defaults ) {
                (void)vtss_icfg_printf(result, " no switchport voice vlan security\n");
            }

#if defined (VTSS_SW_OPTION_LLDP)
            // switchport voice vlan discovery-protocol {oui | lldp | both}
            switch ( port_conf.discovery_protocol[iport] ) {
            case VOICE_VLAN_DISCOVERY_PROTOCOL_OUI:
                if ( req->all_defaults ) {
                    (void)vtss_icfg_printf(result, " switchport voice vlan discovery-protocol oui\n");
                }
                break;

            case VOICE_VLAN_DISCOVERY_PROTOCOL_LLDP:
                (void)vtss_icfg_printf(result, " switchport voice vlan discovery-protocol lldp\n");
                break;

            case VOICE_VLAN_DISCOVERY_PROTOCOL_BOTH:
                (void)vtss_icfg_printf(result, " switchport voice vlan discovery-protocol both\n");
                break;

            default:
                //T_E("unknown discovery protocol %d\n", port_conf.discovery_protocol[iport]);
                break;
            }
#endif /* VTSS_SW_OPTION_LLDP */
        } else {
            //T_E("voice_vlan_mgmt_port_conf_get()\n");
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
vtss_rc voice_vlan_icfg_init(void)
{
    vtss_rc rc;

    /*
        Register Global config callback functions to ICFG module.
    */
    rc = vtss_icfg_query_register(VTSS_ICFG_GLOBAL_VOICE_VLAN, "voice-vlan", _voice_vlan_icfg);
    if ( rc != VTSS_OK ) {
        return rc;
    }

    /*
        Register Interface port list config callback functions to ICFG module.
    */
    rc = vtss_icfg_query_register(VTSS_ICFG_INTERFACE_ETHERNET_VOICE_VLAN, "voice-vlan", _voice_vlan_icfg);
    return rc;
}
