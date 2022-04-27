/*

   Vitesse VCL software.

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
#include "web_api.h"
#include "cli.h"
#include "vcl_api.h"
#include "ip2_legacy.h"

#ifdef VTSS_SW_OPTION_PRIV_LVL
#include "vtss_privilege_api.h"
#include "vtss_privilege_web_api.h"
#endif
/* =================
 * Trace definitions
 * -------------- */
#include <vtss_module_id.h>
#include <vtss_trace_lvl_api.h>
#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_WEB
#include <vtss_trace_api.h>
/* ============== */

#define VCL_WEB_BUF_LEN 512

/****************************************************************************/
/*  Web Handler  functions                                                  */
/****************************************************************************/
static cyg_int32 handler_config_vcl_mac_based_conf(CYG_HTTPD_STATE *p)
{
    vcl_mac_vlan_mgmt_entry_t         newconf;
    i8                                search_str[32];
    const i8                          *str;
    vtss_isid_t                       sid  = web_retrieve_request_sid(p); /* Includes USID = ISID */
    BOOL                              first_entry, next;
    size_t                            len;
    i8                                str_buff[24];
    vcl_mac_vlan_mgmt_entry_get_cfg_t entry;
    vtss_mac_t                        mac_addr;
    uint                              temp_mac[6];
    u32                               hiddenmac_cnt;
    int                               temp_vid;
    port_iter_t                       pit;
    u32                               mask_port = 0;
    i32                               vid_errors = 0;
    vtss_rc                           rc;
    char                              newstr[6] = {0};
    i32                               mac_errors = 0;

    if (redirectUnmanagedOrInvalid(p, sid)) { /* Redirect unmanaged/invalid access to handler */
        return -1;
    }
#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_VCL)) {
        return -1;
    }
#endif
    if (p->method == CYG_HTTPD_METHOD_POST) {
        int  new_vid, new_entry, i, k;
        // We need to know how many entries that is delete when that save button is pressed to
        // make sure that we do not loop through more entries than the number of valid entries shown
        // at the web page.
        i = 0; //index variable
        memset(search_str, 0, sizeof(search_str));
        while (i < VCL_MAC_VLAN_MAX_ENTRIES) {
            sprintf(search_str, "delete_%d", ++i);
            if (cyg_httpd_form_varable_find(p, search_str)) {//fine and delete
                sprintf(search_str, "hiddenmac_%d", i);
                if ((str = cyg_httpd_form_varable_string(p, search_str, &len)) != NULL) {
                    if (sscanf(str, "%2x-%2x-%2x-%2x-%2x-%2x", &temp_mac[0], &temp_mac[1], &temp_mac[2],
                               &temp_mac[3], &temp_mac[4], &temp_mac[5]) == 6) {
                        for (k = 0; k < 6; k++) {
                            mac_addr.addr[k] = temp_mac[k];
                        }
                        if (vcl_mac_vlan_mgmt_mac_vlan_del(sid, &mac_addr, VCL_MAC_VLAN_USER_STATIC) != VTSS_RC_OK) {
                            T_D("Deletion failed\n");
                        }
                    } /* if sscanf */
                }  /* if ((str = cyg_httpd_form_varable_string */
            }
        }
        /* For all the hidden mac; delete and add the entries to cater for changes in VID and port membership */
        for (hiddenmac_cnt = 1; hiddenmac_cnt <= VCL_MAC_VLAN_MAX_ENTRIES; hiddenmac_cnt++) {
            /* For deleted entries, don't check for updates */
            sprintf(search_str, "delete_%d", (int)hiddenmac_cnt);
            if (cyg_httpd_form_varable_find(p, search_str)) {
                continue;
            }
            sprintf(search_str, "hiddenmac_%d", hiddenmac_cnt);
            if ((str = cyg_httpd_form_varable_string(p, search_str, &len)) != NULL) {
                if (sscanf(str, "%2x-%2x-%2x-%2x-%2x-%2x", &temp_mac[0], &temp_mac[1], &temp_mac[2],
                           &temp_mac[3], &temp_mac[4], &temp_mac[5]) == 6) {
                    for (k = 0; k < 6; k++) {
                        mac_addr.addr[k] = temp_mac[k];
                    }
                }
                if (vcl_mac_vlan_mgmt_mac_vlan_del(sid, &mac_addr, VCL_MAC_VLAN_USER_STATIC)
                    == VTSS_RC_OK) {
                    newconf.smac = mac_addr;
                    sprintf(search_str, "vid_%d", hiddenmac_cnt);
                    if (cyg_httpd_form_varable_int(p, search_str, &temp_vid) &&
                        temp_vid > VTSS_VID_NULL &&
                        temp_vid < VTSS_VIDS) {
                        newconf.vid = temp_vid;
                        mask_port = 0;
                        (void) port_iter_init(&pit, NULL, sid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
                        while (port_iter_getnext(&pit)) {
                            mask_port++;
                            sprintf(search_str, "mask_%d_%d", hiddenmac_cnt, mask_port);
                            newconf.ports[pit.iport] = FALSE;
                            if (cyg_httpd_form_varable_find(p, search_str)) { /* "on" if checked */
                                newconf.ports[pit.iport] = TRUE;
                            } /* if(cyg_httpd_form_varable_find(p, search_str) */
                        } /* while (port_iter_getnext(&pit)) */
                        if ((rc = vcl_mac_vlan_mgmt_mac_vlan_add(sid, &newconf, VCL_MAC_VLAN_USER_STATIC)) != VTSS_RC_OK) {
                            if (rc == VCL_ERROR_ENTRY_WITH_DIFF_VLAN) {
                                vid_errors++;
                            }
                            T_D("MAC-based VLAN entry add Failed\n");
                        }
                    } /* if(cyg_httpd_form_varable_int(p, search_str, &new_conf.vid */
                } /* if (vcl_mac_vlan_mgmt_mac_vlan_del */
            } /* if(cyg_httpd_form_varable_string(p, search_str, &len) */
        } /* for (hiddenmac_cnt = 1 */
        new_entry = 0;//while loop index variable
        for (hiddenmac_cnt = 1; hiddenmac_cnt <= VCL_MAC_VLAN_MAX_ENTRIES; hiddenmac_cnt++) {
            new_entry++;
            /* Add new entry */
            new_vid = new_entry;
            sprintf(search_str, "vid_new_%d", new_vid);
            if (cyg_httpd_form_varable_int(p, search_str, &new_vid) &&
                new_vid > VTSS_VID_NULL &&
                new_vid < VTSS_VIDS) {
                newconf.vid = 0xfff & (uint)new_vid;
                (void) port_iter_init(&pit, NULL, sid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
                while (port_iter_getnext(&pit)) {
                    sprintf(search_str, "mask_new_%d_%d", new_entry, pit.uport);
                    newconf.ports[pit.iport] = FALSE;
                    if (cyg_httpd_form_varable_find(p, search_str)) { /* "on" if checked */
                        newconf.ports[pit.iport] = TRUE;
                    }
                }
                sprintf(search_str, "MACID_new_%d", new_entry);
                if ((str = cyg_httpd_form_varable_string(p, search_str, &len)) != NULL) {
                    strncpy(newstr, str, 3);
                    memset(temp_mac, 0, sizeof(temp_mac));
                    if (strchr(newstr, '-')) {
                        if (sscanf(str, "%02x-%02x-%02x-%02x-%02x-%02x", &temp_mac[0], &temp_mac[1], &temp_mac[2],
                                   &temp_mac[3], &temp_mac[4], &temp_mac[5]) == 6) {
                            for (i = 0; i < 6; i++) {
                                newconf.smac.addr[i] = (uchar) temp_mac[i];
                            }
                        }
                    } else if (strchr(newstr, '.')) {
                        if (sscanf(str, "%02x.%02x.%02x.%02x.%02x.%02x", &temp_mac[0], &temp_mac[1], &temp_mac[2],
                                   &temp_mac[3], &temp_mac[4], &temp_mac[5]) == 6) {
                            for (i = 0; i < 6; i++) {
                                newconf.smac.addr[i] = (uchar) temp_mac[i];
                            }
                        }
                    } else if (isxdigit(newstr[2])) {
                        if (sscanf(str, "%02x%02x%02x%02x%02x%02x", &temp_mac[0], &temp_mac[1], &temp_mac[2],
                                   &temp_mac[3], &temp_mac[4], &temp_mac[5]) == 6) {
                            for (i = 0; i < 6; i++) {
                                newconf.smac.addr[i] = (uchar) temp_mac[i];
                            }
                        }
                    } else {
                        mac_errors++;
                        redirect(p, mac_errors ? "/mac_based_vlan.htm?MAC_error=2" : "/mac_based_vlan.htm");
                        return -1;
                    }
                }

                T_D("Add vlan : %d", newconf.vid);
                rc = vcl_mac_vlan_mgmt_mac_vlan_add(sid, &newconf, VCL_MAC_VLAN_USER_STATIC);
                if (rc == VCL_ERROR_ENTRY_WITH_DIFF_VLAN) {
                    vid_errors++;
                }
                if (rc != VTSS_OK) {
                    T_D("vcl_mac_vlan_mgmt_mac_vlan_add(%d): failed", new_vid);
                }
            }
        }

        redirect(p, vid_errors ? "/mac_based_vlan.htm?MAC_error=1" : "/mac_based_vlan.htm");
        return -1;//
    } else {
        /* CYG_HTTPD_METHOD_GET (+HEAD)
               Format: [flag]/[date]
               <Del>         : 1/[mac_address]
               <Get>         : -1
            */
        int ct = 0;
        int vclFlag = -1;
        cyg_httpd_start_chunked("html");
        //insert stacking isid check code here
        first_entry = TRUE;
        next = FALSE;

        if (cyg_httpd_form_varable_int(p, "vclConfigFlag", &vclFlag)) {
        } else {
            vclFlag = -1;//default case
        }
        switch (vclFlag) {
        case 1: //delete entry
            if ((str = cyg_httpd_form_varable_string(p, "myMacAdrress", &len)) != NULL) {
                if (sscanf(str, "%2x-%2x-%2x-%2x-%2x-%2x", &temp_mac[0], &temp_mac[1], &temp_mac[2],
                           &temp_mac[3], &temp_mac[4], &temp_mac[5]) == 6) {
                    int i;//local used
                    for (i = 0; i < 6; i++) {
                        mac_addr.addr[i] = temp_mac[i];
                    }
                }
            }
            //call del api
            if (vcl_mac_vlan_mgmt_mac_vlan_del(sid, &mac_addr, VCL_MAC_VLAN_USER_STATIC) != VTSS_RC_OK) {
                T_E("Deletion failed\n");
            }
            break;
        default:
            while (vcl_mac_vlan_mgmt_mac_vlan_get(sid, &entry, VCL_MAC_VLAN_USER_STATIC, next, first_entry)
                   == VTSS_RC_OK) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/%s/%u/",
                              sid,
                              (misc_mac_txt(entry.smac.addr, str_buff)),
                              entry.vid);
                cyg_httpd_write_chunked(p->outbuffer, ct);
                (void) port_iter_init(&pit, NULL, sid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
                while (port_iter_getnext(&pit)) {
                    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u,",
                                  (entry.ports[sid - 1][pit.iport] == TRUE) ? 1 : 0);
                    cyg_httpd_write_chunked(p->outbuffer, ct);
                    //}
                }
                //show end of enty by ';' character
                cyg_httpd_write_chunked(";", 1);
                if (first_entry == TRUE) {
                    first_entry = FALSE;
                }

                next = TRUE;
            }
            break;
        } /* switch(vclFlag) */
        cyg_httpd_end_chunked();//end of http responseText
        return -1;
    } /* CYG_HTTPD_METHOD_GET */
}
static cyg_int32 handler_config_vcl_proto2grp_map(CYG_HTTPD_STATE *p)
{
    vtss_isid_t                       sid  = web_retrieve_request_sid(p); /* Includes USID = ISID */
    vcl_proto_vlan_proto_entry_t proto_grp;
    const i8                          *str;
    i8                                search_str[32];
    size_t                            len;

    if (redirectUnmanagedOrInvalid(p, sid)) { /* Redirect unmanaged/invalid access to handler */
        return -1;
    }
#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_VCL)) {
        return -1;
    }
#endif
    if (p->method == CYG_HTTPD_METHOD_POST) {/* POST Method */
        int  errors = 0, temp, temp_value, j;
        uint i;
        uint                     oui[OUI_SIZE];

        i = 0; /* index variable */
        memset(search_str, 0x0, sizeof(search_str));
        memset(&proto_grp, 0x0, sizeof(proto_grp));
        /* Delete operation */
        while (i < VCL_PROTO_VLAN_MAX_PROTOCOLS) {/* Loop for max entries time */
            sprintf(search_str, "delete_%d", i);
            if (cyg_httpd_form_varable_find(p, search_str)) {//find and delete
                sprintf(search_str, "hiddenvalue_%d", i);
                if ((str = cyg_httpd_form_varable_string(p, search_str, &len)) != NULL) {
                    if (len == 4) {/* Frame Type is Ethernet */
                        proto_grp.proto_encap_type = VCL_PROTO_ENCAP_ETH2;
                        sscanf(str, "%4x", &temp);
                        proto_grp.proto.eth2_proto.eth_type = temp;
                    } else if (len == 5) {/* Frame Type is LLC */
                        proto_grp.proto_encap_type = VCL_PROTO_ENCAP_LLC_OTHER;
                        if (sscanf(str, "%2x-%2x",
                                   &temp,
                                   &temp_value) == 2) {
                            proto_grp.proto.llc_other_proto.dsap = temp;
                            proto_grp.proto.llc_other_proto.ssap = temp_value;
                        }
                    } else if (len == 13) {/* Frame Type is SNAP */
                        proto_grp.proto_encap_type = VCL_PROTO_ENCAP_LLC_SNAP;
                        if (sscanf(str, "%2x-%2x-%2x-%4x",
                                   &oui[0],
                                   &oui[1],
                                   &oui[2],
                                   &temp_value) == 4) {
                            for (j = 0; j < 3; j++) {
                                proto_grp.proto.llc_snap_proto.oui[j] = oui[j];
                            }
                            proto_grp.proto.llc_snap_proto.pid = temp_value;
                        }
                    } else {/* Invalid Frame Type value length */
                        T_D("Invalid Frame Type detected!\n");
                    }
                }

                if ((vcl_proto_vlan_mgmt_proto_delete(proto_grp.proto_encap_type, &proto_grp.proto,
                                                      VCL_PROTO_VLAN_USER_STATIC)) != VTSS_RC_OK) {
                }
            }
            i++;/* increament while loop counter */
        }/* End of Delete operation */

        /* Add Operation: Add new entries */
        for (i = 1; i <= VCL_PROTO_VLAN_MAX_PROTOCOLS; i++) {
            if (cyg_httpd_form_variable_int_fmt(p, &temp, "Ftype_new_%u", i)) {//find and add
                proto_grp.proto_encap_type = temp;
                if (temp == VCL_PROTO_ENCAP_ETH2) {
                    sprintf(search_str, "value1_E_%d", i);
                    if (cyg_httpd_form_varable_hex(p, search_str, (ulong *)&temp_value)) {
                        proto_grp.proto.eth2_proto.eth_type = temp_value;
                    }
                } else if (temp == VCL_PROTO_ENCAP_LLC_SNAP) {
                    sprintf(search_str, "value1_S_%d", i);
                    if ((str = cyg_httpd_form_varable_string(p, search_str, &len)) != NULL) {
                        if (sscanf(str, "%2x-%2x-%2x", &oui[0], &oui[1], &oui[2]) == 3) {
                            for (j = 0; j < 3; j++) {
                                proto_grp.proto.llc_snap_proto.oui[j] = oui[j];
                            }
                        }
                    }
                    sprintf(search_str, "value2_S_%d", i);
                    if (cyg_httpd_form_varable_hex(p, search_str, (ulong *)&temp_value)) {
                        proto_grp.proto.llc_snap_proto.pid = temp_value;
                    }
                } else if (temp == VCL_PROTO_ENCAP_LLC_OTHER) {
                    sprintf(search_str, "value1_L_%d", i);
                    if (cyg_httpd_form_varable_hex(p, search_str, (ulong *)&temp_value)) {
                        proto_grp.proto.llc_other_proto.dsap = temp_value;
                    }
                    sprintf(search_str, "value2_L_%d", i);
                    if (cyg_httpd_form_varable_hex(p, search_str, (ulong *)&temp_value)) {
                        proto_grp.proto.llc_other_proto.ssap = temp_value;
                    }

                } else {
                    T_D("Invalid Frame Type detected!\n");
                }
                sprintf(search_str, "name_new_%d", i);
                if ((str = cyg_httpd_form_varable_string(p, search_str, &len)) != NULL) {
                    memcpy(proto_grp.group_id, str, len);
                    proto_grp.group_id[len] = '\0';
                }
                if ((vcl_proto_vlan_mgmt_proto_add(&proto_grp, VCL_PROTO_VLAN_USER_STATIC)) != VTSS_RC_OK) {
                }
            }
        }
        redirect(p, errors ? STACK_ERR_URL : "/vcl_protocol_grp_map.htm");
    } else {/* GET Method */
        /* Format: <Frame Type>/<value>/<Group Id>|... */
        BOOL                                first_entry, next;
        int ct;
        cyg_httpd_start_chunked("html");
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "Ethernet#SNAP#LLC,");
        cyg_httpd_write_chunked(p->outbuffer, ct);
        first_entry = TRUE;
        next = FALSE;
        while (vcl_proto_vlan_mgmt_proto_get(&proto_grp, VCL_MAC_VLAN_USER_STATIC, next, first_entry)
               == VTSS_RC_OK) {
            first_entry = FALSE;
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/",
                          proto_grp.proto_encap_type);
            cyg_httpd_write_chunked(p->outbuffer, ct);
            if (proto_grp.proto_encap_type == VCL_PROTO_ENCAP_ETH2) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%04x/",
                              proto_grp.proto.eth2_proto.eth_type);
                cyg_httpd_write_chunked(p->outbuffer, ct);
            } else if (proto_grp.proto_encap_type == VCL_PROTO_ENCAP_LLC_SNAP) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%02x-%02x-%02x-%04x/",
                              proto_grp.proto.llc_snap_proto.oui[0],
                              proto_grp.proto.llc_snap_proto.oui[1],
                              proto_grp.proto.llc_snap_proto.oui[2],
                              proto_grp.proto.llc_snap_proto.pid);
                cyg_httpd_write_chunked(p->outbuffer, ct);
            } else if (proto_grp.proto_encap_type == VCL_PROTO_ENCAP_LLC_OTHER) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%02x-%02x/",
                              proto_grp.proto.llc_other_proto.dsap,
                              proto_grp.proto.llc_other_proto.ssap);
                cyg_httpd_write_chunked(p->outbuffer, ct);
            } else {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "Invalid");
                cyg_httpd_write_chunked(p->outbuffer, ct);
            }
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s|",
                          proto_grp.group_id);
            cyg_httpd_write_chunked(p->outbuffer, ct);
            next = TRUE;
        }
        cyg_httpd_end_chunked();/* end of http responseText */
    }
    return -1;
}/* vcl protocol to group mapping configuration handler */

static cyg_int32 handler_config_vcl_grp2vlan_map(CYG_HTTPD_STATE *p)
{
    vtss_isid_t                         isid  = web_retrieve_request_sid(p); /* Includes USID = ISID */
    vcl_proto_vlan_vlan_entry_t         entry, existing_entry, add_members, delete_members;
    BOOL                                first_entry, next, add, delete;
    const i8                            *str;
    i8                                  search_str[32];
    size_t                              len;
    port_iter_t                         pit;

    add = FALSE;
    delete = FALSE;
    if (redirectUnmanagedOrInvalid(p, isid)) { /* Redirect unmanaged/invalid access to handler */
        return -1;
    }
#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_VCL)) {
        return -1;
    }
#endif
    if (p->method == CYG_HTTPD_METHOD_POST) {/* POST Method */
        int  errors = 0;
        int temp, i = 0;
        /* Operation: delete entry */
        while (i < VCL_PROTO_VLAN_MAX_GROUPS) {/* Loop for max entries time */
            memset(&entry, 0x0, sizeof(entry));
            memset(&existing_entry, 0x0, sizeof(existing_entry));
            memset(&delete_members, 0x0, sizeof(delete_members));
            delete = FALSE;

            sprintf(search_str, "hiddenGrp_%u", i);
            if ((str = cyg_httpd_form_varable_string(p, search_str, &len)) != NULL) {
                memcpy(entry.group_id, str, len);
                entry.group_id[len] = '\0';

                memcpy(existing_entry.group_id, str, len);
                existing_entry.group_id[len] = '\0';

                memcpy(delete_members.group_id, str, len);
                delete_members.group_id[len] = '\0';

                if (cyg_httpd_form_variable_int_fmt(p, &temp, "hiddenVID_%u", i)) {
                    entry.vid = temp;
                    existing_entry.vid = temp;
                    delete_members.vid = temp;
                }
                /* retrieve currently saved entry from switch */
                if ((vcl_proto_vlan_mgmt_group_entry_get_by_vlan(isid, &existing_entry, VCL_PROTO_VLAN_USER_STATIC, FALSE, FALSE))
                    != VTSS_RC_OK) {
                    errors++;
                }
                (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
                while (port_iter_getnext(&pit)) {
                    /* get the new list of member ports for current entry from web page */
                    sprintf(search_str, "mask_%u_%u", i, pit.uport);
                    entry.ports[pit.iport] = FALSE;
                    if (cyg_httpd_form_varable_find(p, search_str)) {//find
                        entry.ports[pit.iport] = TRUE;
                    }
                }
                sprintf(search_str, "delete_%u", i);
                if (cyg_httpd_form_varable_find(p, search_str)) {//find and delete complete entry
                    if ((vcl_proto_vlan_mgmt_group_entry_delete(isid, &existing_entry, VCL_PROTO_VLAN_USER_STATIC)) != VTSS_RC_OK) {
                        errors++;
                    }
                } else {/* delete member ports */
                    /* compare existing port list with new one and prepare port list to be deleted */
                    (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
                    while (port_iter_getnext(&pit)) {
                        if (entry.ports[pit.iport] != existing_entry.ports[pit.iport]) {
                            if (entry.ports[pit.iport] == TRUE) {/* add port members */
                                delete_members.ports[pit.iport] = FALSE;
                            } else {//delete port members
                                delete_members.ports[pit.iport] = TRUE;
                                delete = TRUE;/* i.e. few ports have been removed from current entry, so delete function need to called */
                            }
                        }
                    }
                    /* call delete function to delete removed ports from existing entry */
                    if (delete && ((vcl_proto_vlan_mgmt_group_entry_delete(isid, &delete_members, VCL_PROTO_VLAN_USER_STATIC))
                                   != VTSS_RC_OK)) {
                        if (delete == TRUE) {
                            errors++;
                        }
                    }
                }
            }
            i++;/* loop cntr incr */
        }

        /* Operation: add new port member(s) to existing entry */
        i = 0;/* initialize loop cntr */
        while (i < VCL_PROTO_VLAN_MAX_GROUPS) {/* Loop for max entries time */
            add = FALSE;
            memset(&entry, 0x0, sizeof(entry));
            memset(&existing_entry, 0x0, sizeof(existing_entry));
            memset(&add_members, 0x0, sizeof(add_members));
            sprintf(search_str, "hiddenGrp_%u", i);
            if ((str = cyg_httpd_form_varable_string(p, search_str, &len)) != NULL) {
                memcpy(entry.group_id, str, len);
                entry.group_id[len] = '\0';
                memcpy(add_members.group_id, str, len);
                add_members.group_id[len] = '\0';
                memcpy(existing_entry.group_id, str, len);
                existing_entry.group_id[len] = '\0';

                if (cyg_httpd_form_variable_int_fmt(p, &temp, "hiddenVID_%u", i)) {
                    entry.vid = temp;
                    existing_entry.vid = temp;
                    add_members.vid = temp;
                }
                /* get the new list of member ports for current entry from web page */
                (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
                while (port_iter_getnext(&pit)) {
                    sprintf(search_str, "mask_%u_%u", i, pit.uport);
                    entry.ports[pit.iport] = FALSE;
                    if (cyg_httpd_form_varable_find(p, search_str)) {//find
                        entry.ports[pit.iport] = TRUE;
                    }
                }
                sprintf(search_str, "delete_%u", i);
                if (!cyg_httpd_form_varable_find(p, search_str)) {//if port members is updated if current entry is not checked for deletion
                    /* retrieve currently saved entry from switch */
                    if ((vcl_proto_vlan_mgmt_group_entry_get_by_vlan(isid, &existing_entry, VCL_PROTO_VLAN_USER_STATIC, FALSE, FALSE))
                        != VTSS_RC_OK) {
                    }
                    (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
                    while (port_iter_getnext(&pit)) {
                        if (entry.ports[pit.iport] != existing_entry.ports[pit.iport]) {
                            if (entry.ports[pit.iport] == TRUE) {/* add port members */
                                add_members.ports[pit.iport] = TRUE;
                                add = TRUE;/* i.e current entry need to updated for new member ports being add in web-page */
                            } else {//delete port members
                                add_members.ports[pit.iport] = FALSE;
                            }
                        }
                    }
                    /* call add function to add new ports in existing entry */
                    if (add && ((vcl_proto_vlan_mgmt_group_entry_add(isid, &add_members, VCL_PROTO_VLAN_USER_STATIC))
                                != VTSS_RC_OK)) {
                        if (add == TRUE) {
                            redirect(p, errors ? STACK_ERR_URL : "/vcl_grp_2_vlan_mapping.htm?error=1");
                            return -1;
                        }
                    }
                }
            }
            i++;/* loop cntr */
        }

        /* Operation: Add new entry */
        memset(&entry, 0x0, sizeof(entry));
        for (i = 1; i <= VCL_PROTO_VLAN_MAX_GROUPS; i++) {
            if (cyg_httpd_form_variable_int_fmt(p, &temp, "vid_new_%u", i)) {//find and add
                entry.vid = temp;
                sprintf(search_str, "name_new_%u", i);
                if ((str = cyg_httpd_form_varable_string(p, search_str, &len)) != NULL) {
                    memcpy(entry.group_id, str, len);
                    entry.group_id[len] = '\0';
                }
                /* update member ports array */
                (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
                while (port_iter_getnext(&pit)) {
                    sprintf(search_str, "mask_new_%u_%u", i, pit.uport);
                    entry.ports[pit.iport] = FALSE;
                    if (cyg_httpd_form_varable_find(p, search_str)) {//find
                        entry.ports[pit.iport] = TRUE;
                    }
                }
                if ((vcl_proto_vlan_mgmt_group_entry_add(isid, &entry, VCL_PROTO_VLAN_USER_STATIC)) != VTSS_RC_OK) {
                    errors++;
                }
            }
        }
        redirect(p, errors ? STACK_ERR_URL : "/vcl_grp_2_vlan_mapping.htm");
    } else { /* GET Method */
        /* Format: <Group Name 1>#<Group Name 2>#...#<Group Name n>,<Group Name>/<VID>/<Member port 1 status>#...<Member port max status> */
        int ct;
        cyg_httpd_start_chunked("html");

        /* send list of group to VLAN mapping entries */
        first_entry = TRUE;
        next = FALSE;
        memset(&entry, 0x0, sizeof(entry));
        while ((vcl_proto_vlan_mgmt_group_entry_get(isid, &entry, VCL_PROTO_VLAN_USER_STATIC, next, first_entry))
               == VTSS_RC_OK) {
            first_entry = FALSE;
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s/%u/", entry.group_id, entry.vid);
            cyg_httpd_write_chunked(p->outbuffer, ct);
            /* Send member ports list for current entry */
            (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
            while (port_iter_getnext(&pit)) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u#", entry.ports[pit.iport]);
                cyg_httpd_write_chunked(p->outbuffer, ct);
            }
            cyg_httpd_write_chunked("|", 1);/* entry separator */
            next = TRUE;
        }
        cyg_httpd_end_chunked();/* end of http responseText */
    }
    return -1;
}/* vcl group to vlan mapping configuration web handler */

static cyg_int32 handler_config_vcl_ip_based_conf(CYG_HTTPD_STATE *p)
{
    vtss_isid_t                         sid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    BOOL                                first = TRUE, next = FALSE;
    vcl_ip_vlan_mgmt_entry_t            entry, dentry;
    i8                                  ip_str[20];
    port_iter_t                         pit;
    int                                 i, temp, vid, mask_port = 0, errors = 0, jj = 0;
    i8                                  search_str[32];
    BOOL                                ports[VTSS_PORT_ARRAY_SIZE];

    if (redirectUnmanagedOrInvalid(p, sid)) { /* Redirect unmanaged/invalid access to handler */
        return -1;
    }
#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_VCL)) {
        return -1;
    }
#endif
    if (p->method == CYG_HTTPD_METHOD_POST) {/* POST Method */
        memset(search_str, 0, sizeof(search_str));
        /* Delete the entries with delete check button checked */
        for (i = 1; i <= VCL_IP_VLAN_MAX_ENTRIES; i++) {
            sprintf(search_str, "delete_%d", i);
            if (cyg_httpd_form_varable_find(p, search_str)) {
                if (cyg_httpd_form_variable_int_fmt(p, &temp, "hiddenvce_%u", i)) {
                    memset(&entry, 0, sizeof(entry));
                    entry.vce_id = temp;
                    if (vcl_ip_vlan_mgmt_ip_vlan_del(sid, &entry, VCL_IP_VLAN_USER_STATIC) != VTSS_RC_OK) {
                        T_D("Deletion failed\n");
                    }
                }  /* if (cyg_httpd_form_variable_int_fmt(p, &temp, "hiddenvce_%u", i)) */
            } /* if (cyg_httpd_form_varable_find(p, search_str)) */
        } /* for (i = 0; i < VCL_IP_VLAN_MAX_ENTRIES; i++) */
        memset(search_str, 0, sizeof(search_str));
        /* Update the existing entries if VID or ports are changed */
        for (i = 1; i <= VCL_IP_VLAN_MAX_ENTRIES; i++) {
            if (cyg_httpd_form_variable_int_fmt(p, &temp, "hiddenvce_%u", i)) { /* Get VCE ID */
                memset(&entry, 0, sizeof(entry));
                memset(ports, 0, sizeof(ports));
                entry.vce_id = temp;
                if (!cyg_httpd_form_variable_int_fmt(p, &vid, "vid_%u", i)) { /* Get VLAN ID */
                    continue; /* This should never happen */
                }
                mask_port = 0;
                (void) port_iter_init(&pit, NULL, sid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
                while (port_iter_getnext(&pit)) {
                    mask_port++;
                    sprintf(search_str, "mask_%d_%d", i, mask_port);
                    if (cyg_httpd_form_varable_find(p, search_str)) { /* "on" if checked */
                        ports[pit.iport] = TRUE;
                    } /* if(cyg_httpd_form_varable_find(p, search_str) */
                } /* while (port_iter_getnext(&pit)) */
                if (vcl_ip_vlan_mgmt_ip_vlan_get(sid, &entry, VCL_IP_VLAN_USER_STATIC, FALSE, FALSE) == VTSS_RC_OK) {
                    /* If VID and Ports are not changed, we need not update the entry */
                    if ((entry.vid != vid) || (memcmp(ports, entry.ports, sizeof(ports)))) {
                        /* Special case: this will handle ports that are unchecked from checked in web */
                        for (jj = 0; jj < VTSS_PORT_ARRAY_SIZE; jj++) {
                            if (entry.ports[jj] && !ports[jj]) {
                                dentry.vce_id = temp;
                                if (vcl_ip_vlan_mgmt_ip_vlan_del(sid, &dentry, VCL_IP_VLAN_USER_STATIC) != VTSS_RC_OK) {
                                    T_D("vcl_ip_vlan_mgmt_ip_vlan_del failed\n");
                                }
                                break;
                            }
                        }
                        entry.vce_id = temp;
                        entry.vid = vid;
                        memcpy(entry.ports, ports, sizeof(ports));
                        /* IP address and mask will not change. So, need not populate them */
                        if (vcl_ip_vlan_mgmt_ip_vlan_add(sid, &entry, VCL_IP_VLAN_USER_STATIC) != VTSS_RC_OK) {
                            T_D("vcl_ip_vlan_mgmt_ip_vlan_add failed\n");
                        }
                    } /* if ((entry.vid != vid) || (memcmp(ports, entry.ports, sizeof(ports)))) */
                } /* if (vcl_ip_vlan_mgmt_ip_vlan_get(sid, &entry, VCL_IP_VLAN_USER_STATIC, FALSE, FALSE) == VTSS_RC_OK) */
            } /* if (cyg_httpd_form_variable_int_fmt(p, &temp, "hiddenvce_%u", i)) */
        } /* for (i = 0; i < VCL_IP_VLAN_MAX_ENTRIES; i++) */
        /* Add new entries */
        for (i = 1; i <= VCL_IP_VLAN_MAX_ENTRIES; i++) {
            if (cyg_httpd_form_variable_int_fmt(p, &temp, "vce_new_%u", i)) {
                memset(&entry, 0, sizeof(entry));
                entry.vce_id = temp;
                if (!cyg_httpd_form_variable_int_fmt(p, &vid, "vid_new_%u", i)) { /* Get VLAN ID */
                    continue; /* This should never happen */
                }
                entry.vid = vid;
                sprintf(search_str, "ipid_new_%d", i);
                (void)web_parse_ipv4(p, search_str, &entry.ip_addr); /* Get IP address */
                if (!cyg_httpd_form_variable_int_fmt(p, &temp, "mask_bits_new_%u", i)) { /* Get Mask length */
                    continue; /* This should never happen */
                }
                entry.mask_len = temp;
                mask_port = 0;
                (void) port_iter_init(&pit, NULL, sid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
                while (port_iter_getnext(&pit)) {
                    mask_port++;
                    sprintf(search_str, "mask_new_%d_%d", i, mask_port);
                    if (cyg_httpd_form_varable_find(p, search_str)) { /* "on" if checked */
                        entry.ports[pit.iport] = TRUE;
                    } /* if(cyg_httpd_form_varable_find(p, search_str) */
                } /* while (port_iter_getnext(&pit)) */
                if (vcl_ip_vlan_mgmt_ip_vlan_add(sid, &entry, VCL_IP_VLAN_USER_STATIC) != VTSS_RC_OK) {
                    T_D("vcl_ip_vlan_mgmt_ip_vlan_add failed\n");
                } /* if (vcl_ip_vlan_mgmt_ip_vlan_add(sid, &entry, VCL_IP_VLAN_USER_STATIC) != VTSS_RC_OK) */
            } /* if (cyg_httpd_form_variable_int_fmt(p, &temp, "vce_new_%u", i)) */
        } /* for (i = 1; i <= VCL_IP_VLAN_MAX_ENTRIES; i++) */
        redirect(p, errors ? STACK_ERR_URL : "/subnet_based_vlan.htm");
    } else { /* GET Method */
        i32 ct;
        cyg_httpd_start_chunked("html");
        while (vcl_ip_vlan_mgmt_ip_vlan_get(sid, &entry, VCL_IP_VLAN_USER_STATIC, first, next) == VTSS_RC_OK) {
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/%s/%u/%u/",
                          entry.vce_id, misc_ipv4_txt(entry.ip_addr, ip_str), entry.mask_len, entry.vid);
            cyg_httpd_write_chunked(p->outbuffer, ct);
            /* Send member ports list for current entry */
            (void) port_iter_init(&pit, NULL, sid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
            while (port_iter_getnext(&pit)) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u,", entry.ports[pit.iport]);
                cyg_httpd_write_chunked(p->outbuffer, ct);
            }
            first = FALSE;
            next = TRUE;
            cyg_httpd_write_chunked(";", 1);
        } /* while (vcl_ip_vlan_mgmt_ip_vlan_get(isid, &entry, VCL_IP_VLAN_USER_STATIC, first, next) */
        cyg_httpd_end_chunked();/* end of http responseText */
    }

    return -1;
} /* handler_config_vcl_ip_based_conf */

static cyg_int32 handler_stat_vcl_conf(CYG_HTTPD_STATE *p)
{
    BOOL                                first, next;
    vcl_mac_vlan_mgmt_entry_get_cfg_t   entry;
    vtss_isid_t                         sid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    vcl_mac_vlan_user_t                 user = VCL_MAC_VLAN_USER_STATIC;
    int                                 ct;
    char                                *temp_user;
    i8                                  str_buff[24];
    port_iter_t                         pit;

    if (redirectUnmanagedOrInvalid(p, sid)) { /* Redirect unmanaged/invalid access to handler */
        return -1;
    }
#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_STAT_TYPE, VTSS_MODULE_ID_VCL)) {
        return -1;
    }
#endif

    cyg_httpd_start_chunked("html");
    for (user = VCL_MAC_VLAN_USER_STATIC; user <= VCL_MAC_VLAN_USER_ALL; user++) {
        temp_user = vcl_mac_vlan_mgmt_vcl_user_to_txt(user);
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "#%s", temp_user);
        cyg_httpd_write_chunked(p->outbuffer, ct);
        next = FALSE;
        first = TRUE;
        if (vcl_mac_vlan_mgmt_mac_vlan_get(sid, &entry, VCL_MAC_VLAN_USER_ALL, next, first)
            == VTSS_RC_OK) {
            while ((vcl_mac_vlan_mgmt_mac_vlan_get(sid, &entry, user, next, first) == VTSS_RC_OK)) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "|%s/%u/",
                              (misc_mac_txt(entry.smac.addr, str_buff)),
                              entry.vid);
                cyg_httpd_write_chunked(p->outbuffer, ct);
                (void) port_iter_init(&pit, NULL, sid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
                while (port_iter_getnext(&pit)) {
                    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u,", (entry.ports[sid - 1][pit.iport] == TRUE) ? 1 : 0);
                    cyg_httpd_write_chunked(p->outbuffer, ct);
                }
                //show end of enty by ';' character
                cyg_httpd_write_chunked(";", 1);
                first = FALSE;
                next = TRUE;
            }
        }
    }
    cyg_httpd_end_chunked();
    return -1;
}

/****************************************************************************/
/*  Module JS lib config routine                                            */
/****************************************************************************/

static size_t vcl_lib_config_js(char **base_ptr, char **cur_ptr, size_t *length)
{
    char buff[VCL_WEB_BUF_LEN];
    (void) snprintf(buff, VCL_WEB_BUF_LEN,
                    "var configVCLMacIdMin = %d;\n"
                    "var configVCLMacIdMax = %d;\n"
                    "var configVCLIPIdMin = %d;\n"
                    "var configVCLIPIdMax = %d;\n"
                    "var configVCLProto2GrpMin = %d;\n"
                    "var configVCLProto2GrpMax = %d;\n"
                    "var configVCLGrp2VLANMin = %d;\n"
                    "var configVCLGrp2VLANMax = %d;\n",
                    1, /* First MAC-based VLAN ID */
                    VCL_MAC_VLAN_MAX_ENTRIES,   /* Last VCL MAC Entry (entry number, not absolute ID) */
                    1, /* First IP-based VLAN ID */
                    VCL_IP_VLAN_MAX_ENTRIES,    /* Last VCL IP Entry (entry number, not absolute ID) */
                    1, /* First protocol entry */
                    VCL_PROTO_VLAN_MAX_PROTOCOLS, /* Last VCL Protocol entry (entry number, not absolute ID) */
                    1, /* First protocol-based VLAN */
                    VCL_PROTO_VLAN_MAX_GROUPS   /* Last VCL Protocol VLAN entry (entry number, not absolute ID) */
                   );
    return webCommonBufferHandler(base_ptr, cur_ptr, length, buff);
}

/****************************************************************************/
/*  JS lib config table entry                                               */
/****************************************************************************/
web_lib_config_js_tab_entry(vcl_lib_config_js);

/****************************************************************************/
/*  HTTPD Table Entries                                                     */
/****************************************************************************/

/****************************************************************************/
/*  Common Luton28/JAGUAR/Luton26 table entries                             */
/****************************************************************************/
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_vcl_mac_based_handler, "/config/vcl_conf", handler_config_vcl_mac_based_conf);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_vcl_proto_2_grp_map_handler, "/config/vcl_proto_2_grp_map", handler_config_vcl_proto2grp_map);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_vcl_grp_2_vlan_map_handler, "/config/vcl_grp_2_vlan_map", handler_config_vcl_grp2vlan_map);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_vcl_ip_based_handler, "/config/vcl_ip_conf", handler_config_vcl_ip_based_conf);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_stat_vcl_handler, "/stat/vcl_conf", handler_stat_vcl_conf);
