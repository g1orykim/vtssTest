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
#include "misc_api.h"
#include "icli_api.h"
#include "icli_porting_util.h"
#include "topo_api.h"

#include "mac_api.h"
#include "mac_icfg.h"

#define VTSS_TRACE_MODULE_ID        VTSS_MODULE_ID_MAC

//******************************************************************************
// ICFG callback functions
//******************************************************************************
static vtss_rc mac_icfg_global_conf(const vtss_icfg_query_request_t *req, vtss_icfg_query_result_t *result)
{
    vtss_rc               rc = VTSS_OK;
    mac_age_conf_t        conf;
    switch_iter_t         sit;
    char                  buf[32];
    char                  dest_buf[600];
    mac_mgmt_addr_entry_t mac_entry;
    vtss_vid_mac_t        vid_mac;
    u32                   iport;

    if (!req || !result) {
        return VTSS_RC_ERROR;
    }

    /* Global level: mac address-table aging-time */
    (void)mac_mgmt_age_time_get(&conf);
    if (req->all_defaults || conf.mac_age_time != MAC_AGE_TIME_DEFAULT) {
        VTSS_RC(vtss_icfg_printf(result, "mac address-table aging-time %u\n", conf.mac_age_time));

    }

    /* Global level: mac address-table static, configurable switches */
    (void)icli_switch_iter_init(&sit);
    while (switch_iter_getnext(&sit)) {
        memset(&vid_mac, 0, sizeof(vid_mac));
        while (mac_mgmt_static_get_next(sit.isid, &vid_mac, &mac_entry, 1, 0) == VTSS_OK) {
            BOOL first = TRUE;
            vid_mac = mac_entry.vid_mac;
            dest_buf[0] = '\0';
            for (iport = VTSS_PORT_NO_START; iport < VTSS_PORTS; iport++) {
                if (mac_entry.destination[iport]) {
                    buf[0] = ' ';
                    (void)icli_port_info_txt(topo_isid2usid(sit.isid), iport2uport(iport), &buf[first ? 0 : 1]);
                    strcat(dest_buf, buf);
                    first = FALSE;
                }
            }
            VTSS_RC(vtss_icfg_printf(result, "mac address-table static %s vlan %u interface %s\n",
                                     icli_mac_txt(mac_entry.vid_mac.mac.addr, buf), mac_entry.vid_mac.vid, dest_buf));
        }
    }

    return rc;
}

static vtss_rc mac_icfg_intf_conf(const vtss_icfg_query_request_t *req, vtss_icfg_query_result_t *result)
{
    vtss_rc             rc = VTSS_OK;
    vtss_isid_t         isid;
    vtss_port_no_t      iport;
    vtss_learn_mode_t   mode;
    BOOL                chg_allowed;

    if (!req || !result) {
        return VTSS_RC_ERROR;
    }

    /* Interface level: mac address secure */
    /* Interface level: mac address learning */

    isid = topo_usid2isid(req->instance_id.port.usid);
    iport = uport2iport(req->instance_id.port.begin_uport);

    (void)mac_mgmt_learn_mode_get(isid, iport, &mode, &chg_allowed);

    if (req->all_defaults || !mode.automatic) {
        if (mode.discard) {
            VTSS_RC(vtss_icfg_printf(result, " mac address-table learning secure\n"));
        } else {
            VTSS_RC(vtss_icfg_printf(result, " %smac address-table learning\n", mode.automatic ? "" : "no "));
        }
    }

    return rc;
}

//******************************************************************************
//   Public functions
//******************************************************************************

vtss_rc mac_icfg_init(void)
{
    vtss_rc rc;

    /* Register callback functions to ICFG module. */
    if ((rc = vtss_icfg_query_register(VTSS_ICFG_MAC_INTERFACE_CONF, "mac", mac_icfg_intf_conf)) != VTSS_OK) {
        return rc;
    }

    if ((rc = vtss_icfg_query_register(VTSS_ICFG_MAC_GLOBAL_CONF, "mac", mac_icfg_global_conf)) != VTSS_OK) {
        return rc;
    }

    return VTSS_RC_OK;
}
