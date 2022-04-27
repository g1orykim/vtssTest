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
#include "icli_api.h"
#include "icli_porting_util.h"
#include "misc_api.h" // For e.g. vtss_uport_no_t
#include "msg_api.h"
#include "mgmt_api.h" // For mgmt_prio2txt
#ifdef VTSS_SW_OPTION_ICFG
#include "icfg_api.h"
#endif
#include "vcl_api.h" // For vcl_mac_vlan_mgmt_entry_get_cfg_t
#include <vcl_trace.h>

/***************************************************************************/
/*  Functions called by iCLI                                                */
/****************************************************************************/
//  see vcl_icli_functions.h
vtss_rc vcl_icli_show_vlan(const i32 session_id, const BOOL has_address, const vtss_mac_t *mac_addr)
{
    vcl_mac_vlan_mgmt_entry_get_cfg_t entry;
    BOOL                              first_entry = TRUE, next, entry_found = FALSE;
    switch_iter_t                     sit;
    i8                                str_buf[ICLI_PORTING_STR_BUF_SIZE * VTSS_PORT_ARRAY_SIZE];

    VTSS_RC(switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_USID));
    while (switch_iter_getnext(&sit)) {
        first_entry = TRUE;
        next = FALSE;
        T_IG(TRACE_GRP_CLI, "isid:%d", sit.isid);
        while (vcl_mac_vlan_mgmt_mac_vlan_get(sit.isid, &entry, VCL_MAC_VLAN_USER_STATIC, next, first_entry) == VTSS_RC_OK) {
            next = TRUE;
            first_entry = FALSE;

            // user has ask for a specific MAC address, skip all others.
            if (has_address) {
                if (memcmp(mac_addr, &entry.smac, sizeof(entry.smac)) != 0) {
                    T_IG(TRACE_GRP_CLI, "Skipping MAC address, isid:%d", sit.isid);
                    continue;
                }
            }

            if (!entry_found) { // Printing header if this is the first entry found
                icli_parm_header(session_id, "MAC Address        VID   Interfaces");
            }

            ICLI_PRINTF("%02x-%02x-%02x-%02x-%02x-%02x  %-4u  %s\n", entry.smac.addr[0], entry.smac.addr[1],
                        entry.smac.addr[2], entry.smac.addr[3], entry.smac.addr[4], entry.smac.addr[5],
                        entry.vid, icli_port_list_info_txt(sit.isid, &entry.ports[sit.isid - 1][0], str_buf, FALSE));

            entry_found = TRUE;
        }
        T_IG(TRACE_GRP_CLI, "No more for isid:%d", sit.isid);
    }

    if (!entry_found && has_address) {
        ICLI_PRINTF("Entry with MAC address %02x-%02x-%02x-%02x-%02x-%02x not found\n", mac_addr->addr[0], mac_addr->addr[1],
                    mac_addr->addr[2], mac_addr->addr[3], mac_addr->addr[4], mac_addr->addr[5]);
    }

    return VTSS_RC_OK;
}

//  see vcl_icli_functions.h
vtss_rc vcl_icli_show_ipsubnet(const i32 session_id, const BOOL has_id, const u8 subnet_id)
{
    vcl_ip_vlan_mgmt_entry_t entry;
    BOOL                     first = TRUE, next = FALSE;
    i8                       ip_str[100];
    i8                       str_buf[ICLI_PORTING_STR_BUF_SIZE * VTSS_PORT_ARRAY_SIZE];
    switch_iter_t            sit;
    VTSS_RC(switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_USID));
    while (switch_iter_getnext(&sit)) {
        if (has_id) {
            entry.vce_id = subnet_id;
            if (vcl_ip_vlan_mgmt_ip_vlan_get(sit.isid, &entry, VCL_IP_VLAN_USER_STATIC, FALSE, FALSE) != VTSS_RC_OK) {
                ICLI_PRINTF("Entry with VCE ID %u not found\n", entry.vce_id);
            } else {
                icli_parm_header(session_id, "VCE ID  IP Address       Mask Length  VID   Interfaces");
                ICLI_PRINTF("%-6u  %-15s  %-11u  %-4u  %s\n", entry.vce_id, misc_ipv4_txt(entry.ip_addr, ip_str),
                            entry.mask_len, entry.vid, icli_port_list_info_txt(sit.isid, &entry.ports[0], str_buf, FALSE));

            } /* if (vcl_ip_vlan_mgmt_ip_vlan_get(isid, &entry, VCL_IP_VLAN_USER_STATIC, FALSE, FALSE) != VTSS_RC_OK) */
        } else {
            first = TRUE;
            next = FALSE;
            while (vcl_ip_vlan_mgmt_ip_vlan_get(sit.isid, &entry, VCL_IP_VLAN_USER_STATIC, first, next)
                   == VTSS_RC_OK) {
                if (first == TRUE) {
                    ICLI_PRINTF("VCE ID  IP Address       Mask Length  VID   Interfaces\n");
                    ICLI_PRINTF("------  ---------------  -----------  ----  ----------\n");
                    first = FALSE;
                }
                ICLI_PRINTF("%-6u  %-15s  %-11u  %-4u  %s\n", entry.vce_id, misc_ipv4_txt(entry.ip_addr, ip_str),
                            entry.mask_len, entry.vid, icli_port_list_info_txt(sit.isid, &entry.ports[0], str_buf, FALSE));
                next = TRUE;
            } /* while (vcl_ip_vlan_mgmt_ip_vlan_get(isid, &entry, VCL_IP_VLAN_USER_STATIC, first, next) */
        } /* if (vcl_req->vce_spec == TRUE) */
    } /* for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) */
    return VTSS_RC_OK;
}
/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
