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

    Include files

******************************************************************************
*/
#include "icfg_api.h"
#include "vcl_api.h"
#include "ip2_utils.h" // For vtss_conv_prefix_to_ipv4mask
#include "misc_api.h"   //uport2iport(), iport2uport()
#include "topo_api.h"   //topo_usid2isid(), topo_isid2usid()
#include "vcl_icfg.h"
#include "vcl_trace.h"
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
static vtss_rc VCL_ICFG_global_conf(const vtss_icfg_query_request_t *req,
                                    vtss_icfg_query_result_t *result)
{
    vtss_rc                         rc = VTSS_OK;
    vcl_proto_vlan_proto_entry_t    entry;
    BOOL                            first_entry = TRUE, next = FALSE;

    while (vcl_proto_vlan_mgmt_proto_get(&entry, VCL_PROTO_VLAN_USER_STATIC, next, first_entry) == VTSS_RC_OK) {
        if (first_entry == TRUE) {
            first_entry = FALSE;
        }

        if (entry.proto_encap_type == VCL_PROTO_ENCAP_ETH2) {
            //
            // ETH type
            //
            i8  eth2_proto_string[15];

            switch (entry.proto.eth2_proto.eth_type) {
            case ETHERTYPE_ARP:
                strcpy(eth2_proto_string, "arp");
                break;

            case ETHERTYPE_IP:
                strcpy(eth2_proto_string, "ip");
                break;

            case ETHERTYPE_IPX:
                strcpy(eth2_proto_string, "ipx");
                break;

            case ETHERTYPE_AT:
                strcpy(eth2_proto_string, "at");
                break;
            default:
                sprintf(eth2_proto_string, "0x%X", entry.proto.eth2_proto.eth_type);
                break;
            }
            VTSS_RC(vtss_icfg_printf(result, "%s eth2 %s group %s\n", VCL_GLOBAL_MODE_PROTO_TEXT,
                                     eth2_proto_string, entry.group_id));
        } else if (entry.proto_encap_type == VCL_PROTO_ENCAP_LLC_SNAP) {
            //
            // SNAP
            //

            // Convert snap oui to string
            i8  snap_oui_string[15];

            if (entry.proto.llc_snap_proto.oui[0] == 0x0 &&
                entry.proto.llc_snap_proto.oui[1] == 0x0 &&
                entry.proto.llc_snap_proto.oui[2] == 0x0) {
                strcpy(snap_oui_string, "rfc-1042");
            } else if (entry.proto.llc_snap_proto.oui[0] == 0x0 &&
                       entry.proto.llc_snap_proto.oui[1] == 0x0 &&
                       entry.proto.llc_snap_proto.oui[2] == 0xF8) {
                strcpy(snap_oui_string, "snap-8021h");
            } else {
                sprintf(snap_oui_string, "0x%X", (entry.proto.llc_snap_proto.oui[0] << 16) |
                        (entry.proto.llc_snap_proto.oui[1] << 8) |
                        (entry.proto.llc_snap_proto.oui[2]));
            }

            VTSS_RC(vtss_icfg_printf(result, "%s snap %s 0x%X group %s\n", VCL_GLOBAL_MODE_PROTO_TEXT,                                snap_oui_string, entry.proto.llc_snap_proto.pid, entry.group_id));
        } else {
            //
            // llc
            //

            VTSS_RC(vtss_icfg_printf(result, "%s llc 0x%x 0x%x group %s\n", VCL_GLOBAL_MODE_PROTO_TEXT,
                                     entry.proto.llc_other_proto.dsap,
                                     entry.proto.llc_other_proto.ssap, entry.group_id));
        }
        next = TRUE;
    }

    return rc;
}

static vtss_rc VCL_ICFG_port_conf(const vtss_icfg_query_request_t *req,
                                  vtss_icfg_query_result_t *result)
{
    vtss_isid_t                         isid = topo_usid2isid(req->instance_id.port.usid);
    vtss_port_no_t                      iport = uport2iport(req->instance_id.port.begin_uport);
    vcl_proto_vlan_vlan_entry_t         entry;
    BOOL                                next, first;

    next = FALSE;
    first = TRUE;
    while (vcl_proto_vlan_mgmt_group_entry_get(isid, &entry, VCL_PROTO_VLAN_USER_STATIC, next, first) == VTSS_RC_OK) {
        if (entry.ports[iport]) {
            VTSS_RC(vtss_icfg_printf(result, " %s %s %s %u\n", VCL_PORT_MODE_PROTO_GROUP_TEXT, entry.group_id, "vlan", entry.vid));
        }
        next = TRUE;
        first = FALSE;
    }

    //
    // switchport vlan ip-subnet
    //

    BOOL default_list[VTSS_VCL_ARRAY_SIZE];
    i8 buf[100];
    i8 buf1[100];
    u16 i;

    for (i = 0; i < VCL_IP_VLAN_MAX_ENTRIES; i++) {
        default_list[i] = TRUE; // Initialize array
    }

    vcl_ip_vlan_mgmt_entry_t ip_vlan_entry;
    vtss_ipv4_t mask;
    next = FALSE;
    first = TRUE;

    while (vcl_ip_vlan_mgmt_ip_vlan_get(isid, &ip_vlan_entry, VCL_IP_VLAN_USER_STATIC, first, next) == VTSS_RC_OK) {
        next = TRUE;
        first = FALSE;

        if (ip_vlan_entry.ports[iport]) { // Only print if current port is part of IP subnet based VLAN membership
            VTSS_RC(vtss_conv_prefix_to_ipv4mask(ip_vlan_entry.mask_len, &mask));
            VTSS_RC(vtss_icfg_printf(result, " switchport vlan ip-subnet id %d %s/%s vlan %d\n",
                                     ip_vlan_entry.vce_id,
                                     misc_ipv4_txt(ip_vlan_entry.ip_addr, buf),
                                     misc_ipv4_txt(mask, buf1),
                                     ip_vlan_entry.vid));
            if (ip_vlan_entry.vce_id > 0) {
                default_list[ip_vlan_entry.vce_id - 1] = FALSE; // vce_id starts from 1, while the default list starts from 0, so subtract - 1
            } else {
                T_E("ip_vlan_entry.vce_id should never be less the 1 at this point - is %d", ip_vlan_entry.vce_id);
            }
        }
    }

    // Print the no command.
    vtss_icfg_conf_print_t conf_print;
    vtss_icfg_conf_print_init(&conf_print);
    conf_print.bool_list_max = VCL_IP_VLAN_MAX_ENTRIES - 1;
    conf_print.print_no_arguments = TRUE;
    conf_print.is_default = TRUE;
    conf_print.bool_list = &default_list[0];
    VTSS_RC(vtss_icfg_conf_print(req, result, conf_print, "switchport vlan ip-subnet id", ""));

    //
    // switchport vlan mac vlan
    //
    vcl_mac_vlan_mgmt_entry_get_cfg_t mac_vlan_entry;

    next = FALSE;
    first = TRUE;
    while (vcl_mac_vlan_mgmt_mac_vlan_get(isid, &mac_vlan_entry, VCL_MAC_VLAN_USER_STATIC, next, first) == VTSS_RC_OK) {
        next = TRUE;
        first = FALSE;
        T_N("vcl_mac_vlan_mgmt_mac_vlan_get, port:%d, isid:%d", iport, isid);
        if (mac_vlan_entry.ports[isid - 1][iport]) { // Only print if current port is part the entry
            VTSS_RC(vtss_icfg_printf(result, " switchport vlan mac %s vlan %d\n",
                                     misc_mac2str(mac_vlan_entry.smac.addr),
                                     mac_vlan_entry.vid));
        }
    }

    return VTSS_RC_OK;
}

/*
******************************************************************************

    Public functions

******************************************************************************
*/

/* Initialization function */
vtss_rc VCL_icfg_init(void)
{
    vtss_rc rc;

    /* Register callback functions to ICFG module.
       The configuration divided into two groups for this module.
       1. Global configuration
       2. Port configuration
    */
    if ((rc = vtss_icfg_query_register(VTSS_ICFG_VCL_GLOBAL_CONF, "vlan", VCL_ICFG_global_conf)) != VTSS_OK) {
        return rc;
    }
    rc = vtss_icfg_query_register(VTSS_ICFG_VCL_PORT_CONF, "vlan", VCL_ICFG_port_conf);

    return rc;
}
