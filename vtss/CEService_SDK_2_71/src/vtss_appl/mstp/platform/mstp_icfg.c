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

#include "mstp_api.h"
#include "icfg_api.h"
#include "misc_api.h"
#include "icli_api.h"
#include "topo_api.h"

#define VTSS_TRACE_MODULE_ID        VTSS_MODULE_ID_RSTP
#define VTSS_ALLOC_MODULE_ID        VTSS_MODULE_ID_RSTP

//******************************************************************************
// ICFG callback functions 
//******************************************************************************

static vtss_rc mstp_icfg_global_conf(const vtss_icfg_query_request_t *req,
                                     vtss_icfg_query_result_t *result)
{
    vtss_rc               rc = VTSS_OK;
    mstp_bridge_param_t   sc;
    u32 def_ver = MSTP_PROTOCOL_VERSION_MSTP;
    u8 msti,mstimax = N_MSTI_MAX;
    u32 prio;
#if !defined(VTSS_MSTP_FULL)
    def_ver = MSTP_PROTOCOL_VERSION_RSTP;
#endif /* VTSS_MSTP_FULL */

    if (!req || !result) {
        return VTSS_RC_ERROR;
    }

    if (!mstp_get_system_config(&sc)) {
        return VTSS_RC_ERROR;
    }

    /* Global level */
    if (req->all_defaults || sc.forceVersion != def_ver) {
        VTSS_RC(vtss_icfg_printf(result,"spanning-tree mode %s\n",
                                 sc.forceVersion == MSTP_PROTOCOL_VERSION_MSTP ? "mstp" :
                                 sc.forceVersion == MSTP_PROTOCOL_VERSION_RSTP ? "rstp" : "stp"));
    }
    if (req->all_defaults || sc.bridgeMaxAge != 20 || sc.bridgeForwardDelay != 15)  {
        VTSS_RC(vtss_icfg_printf(result,"spanning-tree mst max-age %d forward-time %d\n",sc.bridgeMaxAge,sc.bridgeForwardDelay));
    }
    if (req->all_defaults || sc.txHoldCount != 6) {
        VTSS_RC(vtss_icfg_printf(result,"spanning-tree transmit hold-count %d\n",sc.txHoldCount));
    }
    if (req->all_defaults || sc.MaxHops != 20) {
        VTSS_RC(vtss_icfg_printf(result,"spanning-tree mst max-hops %d\n",sc.MaxHops));
    }
#ifdef VTSS_SW_OPT_MSTP_BPDU_ENH
    if (req->all_defaults || sc.bpduFiltering) {
        VTSS_RC(vtss_icfg_printf(result,"%sspanning-tree edge bpdu-filter\n",sc.bpduFiltering?"":"no "));
    }
    if (req->all_defaults || sc.bpduGuard) {
        VTSS_RC(vtss_icfg_printf(result,"%sspanning-tree edge bpdu-guard\n",sc.bpduGuard?"":"no "));
    }
    if (req->all_defaults || sc.errorRecoveryDelay != 0) {
        if (sc.errorRecoveryDelay == 0) {
            VTSS_RC(vtss_icfg_printf(result,"no spanning-tree recovery interval\n"));
        } else {
            VTSS_RC(vtss_icfg_printf(result,"spanning-tree recovery interval %d\n",sc.errorRecoveryDelay));
        }
    }
#endif /* VTSS_SW_OPT_MSTP_BPDU_ENH */

    if(mstp_get_system_config(&sc) && sc.forceVersion < MSTP_PROTOCOL_VERSION_MSTP)
        mstimax = 1;
    for(msti = 0; msti < mstimax; msti++) {
        prio = mstp_get_msti_priority(msti) << 8;
        if (req->all_defaults || prio != 32768) {
            VTSS_RC(vtss_icfg_printf(result,"spanning-tree mst %d priority %d\n",msti, prio));        
        }
    }

#if defined(VTSS_MSTP_FULL)
    const int vlanrange_size = 10/2*2 + (100-10)/2*3 + (1000-100)/2*4 + (4096-1000)/2*5; // Worst-case: Every odd-numbered VLAN, comma-separated
    mstp_msti_config_t *conf = (mstp_msti_config_t *)VTSS_MALLOC(sizeof(*conf));
    char *vlanrange = (char *)VTSS_MALLOC(vlanrange_size);

    if (!conf || !vlanrange) {
        T_E("Out of heap memory");
        VTSS_FREE(conf);
        VTSS_FREE(vlanrange);
        return VTSS_RC_ERROR;
    }

    if (mstp_get_msti_config(conf, NULL)) {
        if (req->all_defaults || strlen(conf->configname) != 0) {
            if (strlen(conf->configname) == 0) {
                rc = vtss_icfg_printf(result,"no spanning-tree mst name\n");
            } else {            
                rc = vtss_icfg_printf(result, "spanning-tree mst name %.*s revision %d\n",
                                         (int)sizeof(conf->configname), conf->configname, conf->revision);
            }
        }

        for (msti = 1; rc == VTSS_RC_OK && msti < N_MSTI_MAX; msti++) {
            vlanrange[0] = vlanrange[vlanrange_size-1] = '\0';
            (void) mstp_mstimap2str(conf, msti, vlanrange, vlanrange_size);
            if (req->all_defaults || strlen(vlanrange) != 0) {
                if (strlen(vlanrange) == 0) {
                    rc = vtss_icfg_printf(result, "no spanning-tree mst %d vlan\n", msti);
                } else {
                    rc = vtss_icfg_printf(result, "spanning-tree mst %d vlan %s\n", msti, vlanrange);
                }
            }
        }
    }

    VTSS_FREE(conf);
    VTSS_FREE(vlanrange);
#endif /* VTSS_MSTP_FULL */

    return rc;
}

static vtss_rc mstp_icfg_intf_conf(const vtss_icfg_query_request_t *req,
                                   vtss_icfg_query_result_t *result)
{
    vtss_rc                  rc = VTSS_OK;
    vtss_isid_t              isid;
    vtss_port_no_t           iport;
    mstp_port_param_t        conf;
    BOOL                     enable;

    if (!req || !result) {
        return VTSS_RC_ERROR;
    }

    /* Interface level */
    if (req->cmd_mode == ICLI_CMD_MODE_STP_AGGR) {
        isid = VTSS_ISID_LOCAL;
        iport = VTSS_PORT_NO_NONE;
    } else {
        isid = topo_usid2isid(req->instance_id.port.usid);
        iport = uport2iport(req->instance_id.port.begin_uport);
    }

    if(!mstp_get_port_config(isid, iport, &enable, &conf)) {
        return VTSS_RC_ERROR;
    }
    if (req->all_defaults || !enable) {
        VTSS_RC(vtss_icfg_printf(result," %sspanning-tree\n",enable?"":"no "));
    }
    if (req->all_defaults || conf.adminEdgePort) {
        VTSS_RC(vtss_icfg_printf(result," %sspanning-tree edge\n",conf.adminEdgePort?"":"no "));
    }
    if (req->all_defaults || !conf.adminAutoEdgePort) {
        VTSS_RC(vtss_icfg_printf(result," %sspanning-tree auto-edge\n",conf.adminAutoEdgePort?"":"no "));
    }
    if (req->all_defaults || (conf.adminPointToPointMAC != P2P_AUTO)) {
        VTSS_RC(vtss_icfg_printf(result," spanning-tree link-type %s\n",
                                 conf.adminPointToPointMAC == P2P_AUTO?"auto":
                                 conf.adminPointToPointMAC == P2P_FORCETRUE?"point-to-point":"shared"));
    }
    if (req->all_defaults || conf.restrictedRole) {
        VTSS_RC(vtss_icfg_printf(result," %sspanning-tree restricted-role\n",conf.restrictedRole?"":"no "));
    }
    if (req->all_defaults || conf.restrictedTcn) {
        VTSS_RC(vtss_icfg_printf(result," %sspanning-tree restricted-tcn\n",conf.restrictedTcn?"":"no "));
    }
#ifdef VTSS_SW_OPT_MSTP_BPDU_ENH
    if (req->all_defaults || conf.bpduGuard) {
        VTSS_RC(vtss_icfg_printf(result," %sspanning-tree bpdu-guard\n",conf.bpduGuard?"":"no "));
    }
#endif /* VTSS_SW_OPT_MSTP_BPDU_ENH */

#if defined(VTSS_MSTP_FULL)    
    mstp_msti_port_param_t    msti_conf;
    u8 msti;
    for(msti = 0; msti < N_MSTI_MAX; msti++) {
        if (mstp_get_msti_port_config(isid, msti, iport, &msti_conf)) {
            if (req->all_defaults || msti_conf.adminPathCost != 0) {
                if (msti_conf.adminPathCost != 0) {
                    VTSS_RC(vtss_icfg_printf(result," spanning-tree mst %d cost %d\n",msti,msti_conf.adminPathCost));
                } else {
                    VTSS_RC(vtss_icfg_printf(result," spanning-tree mst %d cost %s\n",msti,"auto"));
                }
            }
            if (req->all_defaults || msti_conf.adminPortPriority != 128) {
                VTSS_RC(vtss_icfg_printf(result," spanning-tree mst %d port-priority %d\n",msti,msti_conf.adminPortPriority));
            }
        }
    }      
#endif /* VTSS_MSTP_FULL */

    return rc;
}

//******************************************************************************
//   Public functions
//******************************************************************************

vtss_rc mstp_icfg_init(void)
{
    vtss_rc rc;

    /* Interface - Register callback functions to ICFG module */
    if ((rc = vtss_icfg_query_register(VTSS_ICFG_MSTP_INTERFACE_CONF, "mstp", mstp_icfg_intf_conf)) != VTSS_OK) {
        return rc;
    }

    /* Aggregation - Register callback functions to ICFG module */
    if ((rc = vtss_icfg_query_register(VTSS_ICFG_MSTP_AGGR_CONF, "mstp", mstp_icfg_intf_conf)) != VTSS_OK) {
        return rc;
    }

    /* Global - Register callback functions to ICFG module */
    if ((rc = vtss_icfg_query_register(VTSS_ICFG_MSTP_GLOBAL_CONF, "mstp", mstp_icfg_global_conf)) != VTSS_OK) {
        return rc;
    }


    return VTSS_RC_OK;
}
