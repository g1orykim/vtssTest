/*

 Vitesse Switch API software.

 Copyright (c) 2002-2012 Vitesse Semiconductor Corporation "Vitesse". All
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

    Include files

******************************************************************************
*/
#include "icfg_api.h"
#include "vtss_snmp_api.h"
#include "snmp_icfg.h"
#include "misc_api.h"   //uport2iport(), iport2uport()
#include "cli.h"
#include "topo_api.h"

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
static vtss_rc TRAP_ICFG_global_conf(const vtss_icfg_query_request_t *req,
                                     vtss_icfg_query_result_t *result)
{
    vtss_rc     rc = VTSS_OK;
    vtss_trap_sys_conf_t conf, def_conf;

    trap_mgmt_sys_default_get( &def_conf );
    (void) trap_mgmt_mode_get( &conf.trap_mode );

    /* command: snmp-server trap
                no snmp-server trap
    */

    if (req->all_defaults ||
        (conf.trap_mode != def_conf.trap_mode)) {
        rc = vtss_icfg_printf(result, "%s%s\n",
                              conf.trap_mode == FALSE ? "no " : "", "snmp-server trap");
    }
    return rc;
}

static vtss_rc TRAP_ICFG_host_conf(const vtss_icfg_query_request_t *req,
                                   vtss_icfg_query_result_t *result)
{
    vtss_rc     rc = VTSS_OK;
    i8         cmd[256];

#ifdef VTSS_SW_OPTION_IPV6
    i8         ipStr[64];
#endif
    vtss_trap_entry_t trap_entry, def_entry;
    vtss_trap_conf_t  *conf = &trap_entry.trap_conf, *def_conf = &def_entry.trap_conf;
    vtss_trap_event_t  *event = &trap_entry.trap_event, *def_event = &def_entry.trap_event;

    /*
        SUB-MODE: snmp-server host <word32>
        COMMAND =
        COMMAND = shutdown
        COMMAND = host { <ipv4_ucast> | <ipv6_ucast> | <hostname> } [ <1-65535> ] [ traps | informs ]
        COMMAND = version { v1 [ <word127> ] | v2 [ <word127> ] | { v3 [ probe | engineID <word10-32> ] [ <word32> ] } }
        COMMAND = informs retries <0-255> timeout <0-2147>
        COMMAND = traps [ aaa authentication ] [ system [ coldstart ] [ warmstart ] ] [ switch [ stp ] [ rmon ] ]
    */

    trap_mgmt_conf_default_get(&def_entry);
    strncpy( trap_entry.trap_conf_name, req->instance_id.string, TRAP_MAX_NAME_LEN );
    trap_entry.trap_conf_name[TRAP_MAX_NAME_LEN] = 0;
    if ( VTSS_RC_OK  == trap_mgmt_conf_get(&trap_entry)) {
        if (req->all_defaults ||
            (conf->enable != def_conf->enable)) {
            rc = vtss_icfg_printf(result, " %s%s\n",
                                  conf->enable == TRUE ? "no " : "", "shutdown");
        }

#ifdef VTSS_SW_OPTION_IPV6
        if (req->all_defaults ||
            (conf->dip.ipv6_flag != def_conf->dip.ipv6_flag) ||
            (conf->dip.ipv6_flag && memcmp(&conf->dip.addr.ipv6, &def_conf->dip.addr.ipv6, sizeof(def_conf->dip.addr.ipv6))) ||
            (!conf->dip.ipv6_flag && strcmp(conf->dip.addr.ipv4_str, def_conf->dip.addr.ipv4_str) && conf->dip.addr.ipv4_str[0] != 0 ) ||
            (conf->trap_port != def_conf->trap_port) ||
            (conf->trap_inform_mode != def_conf->trap_inform_mode)) {
            rc = vtss_icfg_printf( result, " host %s %d %s\n", conf->dip.ipv6_flag ? misc_ipv6_txt(&conf->dip.addr.ipv6, ipStr) : conf->dip.addr.ipv4_str[0] ? conf->dip.addr.ipv4_str : "0.0.0.0",
                                   conf->trap_port, conf->trap_inform_mode ? "informs" : "traps");

#else
        if (req->all_defaults ||
            (!conf->dip.ipv6_flag && strcmp(conf->dip.addr.ipv4_str, def_conf->dip.addr.ipv4_str) && conf->dip.addr.ipv4_str[0] != 0 ) ||
            (conf->trap_port != def_conf->trap_port) ||
            (conf->trap_inform_mode != def_conf->trap_inform_mode)) {
            rc = vtss_icfg_printf( result, " host %s %d %s\n", conf->dip.addr.ipv4_str[0] ? conf->dip.addr.ipv4_str : "0.0.0.0",
                                   conf->trap_port, conf->trap_inform_mode ? "informs" : "traps");

#endif
        }


        if (req->all_defaults ||
            (conf->trap_version != def_conf->trap_version) ||
            (conf->trap_version != SNMP_SUPPORT_V3 && strcmp(conf->trap_community, def_conf->trap_community)) ||
            (conf->trap_version == SNMP_SUPPORT_V3 && ((conf->trap_probe_engineid != def_conf->trap_probe_engineid) ||
                                                       (!conf->trap_probe_engineid && ((conf->trap_engineid_len != def_conf->trap_engineid_len) ||
                                                                                       memcmp(conf->trap_engineid, def_conf->trap_engineid, conf->trap_engineid_len) || strcmp(conf->trap_security_name, def_conf->trap_security_name)))))) {
            if (conf->trap_version != SNMP_SUPPORT_V3) {
                rc = vtss_icfg_printf( result, " version %s %s\n", conf->trap_version == SNMP_SUPPORT_V1  ? "v1" : "v2", conf->trap_community);
            } else if (conf->trap_probe_engineid) {
                rc = vtss_icfg_printf( result, " version v3 probe %s\n", conf->trap_security_name);
            } else {
                rc = vtss_icfg_printf( result, " version v3 engineID %s %s\n", misc_engineid2str(conf->trap_engineid, conf->trap_engineid_len), conf->trap_security_name);
            }
        }

        if (req->all_defaults ||
            conf->trap_inform_retries != def_conf->trap_inform_retries || conf->trap_inform_timeout != def_conf->trap_inform_timeout ) {
            rc = vtss_icfg_printf( result, " informs retries %u timeout %u\n", conf->trap_inform_retries, conf->trap_inform_timeout);
        }

        if (req->all_defaults ||
            event->aaa.trap_authen_fail != def_event->aaa.trap_authen_fail || memcmp(&event->system, &def_event->system, sizeof(def_event->system)) ||
            memcmp(&event->sw, &def_event->sw, sizeof(def_event->sw))) {
            sprintf(cmd, " traps ");
            if ( event->aaa.trap_authen_fail) {
                sprintf(cmd, "%s%s", cmd, "aaa authentication ");
            }

            if ( event->sw.stp || event->sw.rmon) {
                sprintf(cmd, "%s%s%s%s", cmd, "switch ", event->system.warm_start ? "stp " : "", event->system.cold_start ? "rmon " : "" );
                rc = vtss_icfg_printf(result, "%s\n", cmd);
            }

            if ( event->system.warm_start || event->system.cold_start) {
                sprintf(cmd, "%s%s%s%s", cmd, "system ", event->system.warm_start ? "warmstart " : "", event->system.cold_start ? "coldstart " : "" );
                rc = vtss_icfg_printf(result, "%s\n", cmd);
            }
        }

    } else {
        return VTSS_RC_ERROR;
    }

    return rc;
}

static vtss_rc TRAP_ICFG_port_conf(const vtss_icfg_query_request_t *req,
                                   vtss_icfg_query_result_t *result)
{
    vtss_rc             rc = VTSS_OK;
    vtss_trap_entry_t   trap_entry, def_entry;
    vtss_trap_event_t    *event = &trap_entry.trap_event, *def_event = &def_entry.trap_event;
    vtss_isid_t         isid = topo_usid2isid(req->instance_id.port.usid);
    vtss_port_no_t      iport = uport2iport(req->instance_id.port.begin_uport);

    /*
       COMMAND = snmp-server host <word32> traps [ linkup ] [ linkdown ] [ lldp ]
       */

    trap_mgmt_conf_default_get(&def_entry);

    memset(trap_entry.trap_conf_name, 0, sizeof(trap_entry.trap_conf_name));
    while ( VTSS_RC_OK == trap_mgmt_conf_get_next(&trap_entry)) {
        if ( req->all_defaults ||
             event->interface.trap_linkup[isid][iport] != def_event->interface.trap_linkup[isid][iport] ||
             event->interface.trap_linkdown[isid][iport] != def_event->interface.trap_linkdown[isid][iport] ||
             event->interface.trap_lldp[isid][iport] != def_event->interface.trap_lldp[isid][iport] ) {
            rc = vtss_icfg_printf(result, " snmp-server host %s traps %s%s%s\n", trap_entry.trap_conf_name,
                                  event->interface.trap_linkup[isid][iport] ? "linkup " : "",
                                  event->interface.trap_linkdown[isid][iport] ? "linkdown " : "",
                                  event->interface.trap_lldp[isid][iport] ? "lldp " : "");
        }

    }

    return rc;
}
/*
******************************************************************************

    Public functions

******************************************************************************
*/

/* Initialization function */
vtss_rc trap_icfg_init(void)
{
    vtss_rc rc;

    /* Register callback functions to ICFG module.
       The configuration divided into two groups for this module.
       1. Global configuration
       2. Port configuration
    */
    if ((rc = vtss_icfg_query_register(VTSS_ICFG_TRAP_GLOBAL_CONF, "snmp", TRAP_ICFG_global_conf)) != VTSS_OK) {
        return rc;
    }

    if ((rc = vtss_icfg_query_register(VTSS_ICFG_TRAP_HOST_CONF, "snmp", TRAP_ICFG_host_conf)) != VTSS_OK) {
        return rc;
    }

    if ((rc = vtss_icfg_query_register(VTSS_ICFG_TRAP_PORT_CONF, "snmp", TRAP_ICFG_port_conf)) != VTSS_OK) {
        return rc;
    }

    return rc;
}
