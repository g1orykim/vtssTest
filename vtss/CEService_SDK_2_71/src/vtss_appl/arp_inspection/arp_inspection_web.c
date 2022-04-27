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

#include "web_api.h"
#include "arp_inspection_api.h"
#include "port_api.h"
#include "vlan_api.h"
#ifdef VTSS_SW_OPTION_PRIV_LVL
#include "vtss_privilege_api.h"
#include "vtss_privilege_web_api.h"
#endif

#ifdef VTSS_SW_OPTION_ACL
#include "mgmt_api.h"
#endif /* VTSS_SW_OPTION_ACL */

/* =================
 * Trace definitions
 * -------------- */
#include <vtss_module_id.h>
#include <vtss_trace_lvl_api.h>
#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_WEB
#include <vtss_trace_api.h>
/* ============== */

#define ARP_INSPECTION_MAX_VLAN_CNT VLAN_ENTRY_CNT

/****************************************************************************/
/*  local API  functions                                                    */
/****************************************************************************/

static arp_inspection_log_type_t parse_log_type_for_vlan(u8 flags)
{
    if ((flags & ARP_INSPECTION_VLAN_LOG_DENY) && (flags & ARP_INSPECTION_VLAN_LOG_PERMIT)) {
        return ARP_INSPECTION_LOG_ALL;
    } else if (flags & ARP_INSPECTION_VLAN_LOG_DENY) {
        return ARP_INSPECTION_LOG_DENY;
    } else if (flags & ARP_INSPECTION_VLAN_LOG_PERMIT) {
        return ARP_INSPECTION_LOG_PERMIT;
    } else {
        return ARP_INSPECTION_LOG_NONE;
    }
}


/****************************************************************************/
/*  Web Handler  functions                                                  */
/****************************************************************************/

static cyg_int32 handler_config_arp_inspection(CYG_HTTPD_STATE *p)
{
    vtss_isid_t                     sid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    int                             ct;
    vtss_uport_no_t                 uport;
    char                            search_str[64];
    u32                             mode = 0;
    arp_inspection_port_mode_conf_t port_mode_conf;
    int                             first = 1;
    const char                      *var_string;
    int                             var_value = 0;
    port_iter_t                     pit;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_SECURITY)) {
        return -1;
    }
#endif

    (void) arp_inspection_mgmt_conf_port_mode_get(sid , &port_mode_conf);

    if (p->method == CYG_HTTPD_METHOD_POST) {
        if (cyg_httpd_form_varable_long_int(p, "arp_inspection_mode", &mode)) {
            if (arp_inspection_mgmt_conf_mode_set(&mode) != VTSS_OK) {
                T_W("arp_inspection_mgmt_conf_mode_set() failed");
            }
        }

        (void) port_iter_init(&pit, NULL, sid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT);
        while (port_iter_getnext(&pit)) {
            uport = iport2uport(pit.iport);

            // port mode
            sprintf(search_str, "port_mode_%d", uport);
            (void) cyg_httpd_form_varable_long_int(p, search_str, &mode);
            port_mode_conf.mode[pit.iport] = mode;

            // vlan mode
            sprintf(search_str, "vlan_mode_%d", uport);
            (void) cyg_httpd_form_varable_long_int(p, search_str, &mode);
            port_mode_conf.check_VLAN[pit.iport] = mode;

            // log type
            sprintf(search_str, "log_mode_%d", uport);
            (void) cyg_httpd_form_varable_long_int(p, search_str, &mode);
            port_mode_conf.log_type[pit.iport] = mode;


        }
        if (arp_inspection_mgmt_conf_port_mode_set(sid, &port_mode_conf) != VTSS_OK) {
            T_W("arp_inspection_mgmt_conf_port_mode_set() failed");
        }

        redirect(p, "/arp_inspection.htm");
    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        (void)cyg_httpd_start_chunked("html");

        /* translate dynamic entries into static entries */
        if ((var_string = cyg_httpd_form_varable_find(p, "translateFlag")) != NULL) {
            var_value = atoi(var_string);
            if (var_value == 1) {
                if (arp_inspection_mgmt_conf_translate_dynamic_into_static() < VTSS_OK) {
                    T_W("arp inspection translation failed");
                }
            }
        }

        /* get form data
           Format: <mode>,<port_no>/<port_mode>/[vlan_mode]/[log_mode]|...
        */
        if (arp_inspection_mgmt_conf_mode_get(&mode) == VTSS_OK) {
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d,", mode);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);

            if (arp_inspection_mgmt_conf_port_mode_get(sid , &port_mode_conf) == VTSS_OK) {

                (void) port_iter_init(&pit, NULL, sid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT);
                while (port_iter_getnext(&pit)) {
                    if (!first) {
                        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "|");
                        (void)cyg_httpd_write_chunked(p->outbuffer, ct);
                    }
                    first = 0;
                    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d/%d/%d/%d",
                                  iport2uport(pit.iport), port_mode_conf.mode[pit.iport],
                                  port_mode_conf.check_VLAN[pit.iport], port_mode_conf.log_type[pit.iport]);
                    (void)cyg_httpd_write_chunked(p->outbuffer, ct);
                }

            }
        }
        (void)cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}

static cyg_int32 handler_config_arp_inspection_static_table(CYG_HTTPD_STATE *p)
{
    vtss_isid_t             sid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    int                     ct, entry_index, vid_temp;
    int                     entry_cnt, max_entries, remained_entries;
    arp_inspection_entry_t  entry;
    char                    search_str[64];
    char                    search_str2[64];
    char                    search_str3[64];
    char                    search_str4[64];
    char                    buf1[80], buf2[80];
    port_iter_t             pit;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_SECURITY)) {
        return -1;
    }
#endif

    if (p->method == CYG_HTTPD_METHOD_POST) {

        (void) port_iter_init(&pit, NULL, sid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT);
        while (port_iter_getnext(&pit)) {
            if (arp_inspection_mgmt_conf_static_entry_get(&entry, FALSE) == VTSS_OK) {
                if ((entry.isid == sid) && (entry.port_no == pit.iport)) {
                    //CPRINTF("%4ld  %4ld  %17s  %-15s\n", iport2uport(entry.port_no), entry.vid, misc_mac_txt(entry.mac, buf1), misc_ipv4_txt(entry.assigned_ip, buf2));
                    sprintf(search_str, "delete_%d_%d_%s_%s",
                            iport2uport(pit.iport),
                            entry.vid,
                            misc_mac_txt(entry.mac, buf1),
                            misc_ipv4_txt(entry.assigned_ip, buf2));
                    if (cyg_httpd_form_varable_find(p, search_str)) { /* "delete" if checked */
                        (void) arp_inspection_mgmt_conf_static_entry_del(&entry);
                    }
                }
                while (arp_inspection_mgmt_conf_static_entry_get(&entry, TRUE) == VTSS_OK) {
                    if ((entry.isid == sid) && (entry.port_no == pit.iport)) {
                        sprintf(search_str, "delete_%d_%d_%s_%s",
                                iport2uport(pit.iport),
                                entry.vid,
                                misc_mac_txt(entry.mac, buf1),
                                misc_ipv4_txt(entry.assigned_ip, buf2));
                        if (cyg_httpd_form_varable_find(p, search_str)) { /* "delete" if checked */
                            (void) arp_inspection_mgmt_conf_static_entry_del(&entry);
                        }
                    }
                }
            }
        }
        for (entry_index = 1; entry_index <= ARP_INSPECTION_MAX_ENTRY_CNT; entry_index++) {
            sprintf(search_str, "new_port_no_%d", entry_index);
            sprintf(search_str2, "new_vid_%d", entry_index);
            sprintf(search_str3, "new_mac_addr_%d", entry_index);
            sprintf(search_str4, "new_ip_addr_%d", entry_index);
            if (cyg_httpd_form_varable_long_int(p, search_str, &entry.port_no) &&
                cyg_httpd_form_varable_int(p, search_str2, &vid_temp) &&
                cyg_httpd_form_varable_mac(p, search_str3, entry.mac) &&
                cyg_httpd_form_varable_ipv4(p, search_str4, &entry.assigned_ip)) {
                entry.isid = sid;
                entry.type = ARP_INSPECTION_STATIC_TYPE;
                entry.valid = TRUE;
                entry.vid = vid_temp;
                entry.port_no = uport2iport(entry.port_no);
                if (arp_inspection_mgmt_conf_static_entry_set(&entry) != VTSS_OK) {
                    T_D("arp_inspection_mgmt_conf_static_entry_set() failed");
                }
            }
        }

        redirect(p, "/arp_inspection_static_table.htm");
    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        (void)cyg_httpd_start_chunked("html");

        /* get form data
           Format: <max_entries_num>,<port_no>/<vid>/<mac_addr>/<ip_addr>|...,<remained_entries_in_sid>
        */

        /* max entries num */
        max_entries = ARP_INSPECTION_MAX_ENTRY_CNT;
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d,", max_entries);
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);

        /* all entries which are already inserted into static table */
        entry_cnt = 0;
        if (arp_inspection_mgmt_conf_static_entry_get(&entry, FALSE) == VTSS_OK) {
            if (entry.isid == sid) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d/%d/%s/%s",
                              iport2uport(entry.port_no),
                              entry.vid,
                              misc_mac_txt(entry.mac, buf1),
                              misc_ipv4_txt(entry.assigned_ip, buf2));
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            } else {
                entry_cnt++;
            }
            while (arp_inspection_mgmt_conf_static_entry_get(&entry, TRUE) == VTSS_OK) {
                if (entry.isid == sid) {
                    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "|%d/%d/%s/%s",
                                  iport2uport(entry.port_no),
                                  entry.vid,
                                  misc_mac_txt(entry.mac, buf1),
                                  misc_ipv4_txt(entry.assigned_ip, buf2));
                    (void)cyg_httpd_write_chunked(p->outbuffer, ct);
                } else {
                    entry_cnt++;
                }
            }
        }

        /* remained entries in sid switch */
        remained_entries = ARP_INSPECTION_MAX_ENTRY_CNT - entry_cnt;
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), ",%d", remained_entries);
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);

        /* end */
        (void)cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}

static cyg_int32 handler_stat_dynamic_arp_inspection(CYG_HTTPD_STATE *p)
{
    vtss_isid_t                     sid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    int                             ct;
    arp_inspection_entry_t          entry, search_entry;
    arp_inspection_port_mode_conf_t port_mode_conf;
    char                            buf1[80], buf2[80];
    uint                            mac_addr[6];
    int                             i, entry_cnt = 0;
    int                             num_of_entries_arp = 0;
    int                             dyn_get_next_entry = 0;
    const char                      *var_string;
    ulong                           dummy;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_STAT_TYPE, VTSS_MODULE_ID_SECURITY)) {
        return -1;
    }
#endif

    if (p->method == CYG_HTTPD_METHOD_POST) {
        redirect(p, "/dyna_arp_inspection.htm");
    } else { /* CYG_HTTPD_METHOD_GET (+HEAD) */
        (void)cyg_httpd_start_chunked("html");

        memset(&entry, 0x0, sizeof(entry));
        entry.isid = sid;
        entry.vid = 1;

        // Get number of entries per page
        if ((var_string = cyg_httpd_form_varable_find(p, "DynNumberOfEntries")) != NULL) {
            num_of_entries_arp = atoi(var_string);
        }
        if (num_of_entries_arp <= 0 || num_of_entries_arp > 99) {
            num_of_entries_arp = 20;
        }

        // Get start port
        if ((var_string = cyg_httpd_form_varable_find(p, "port")) != NULL) {
            entry.port_no = uport2iport(atoi(var_string));
        }

        // Get start vid
        if ((var_string = cyg_httpd_form_varable_find(p, "DynStartVid")) != NULL) {
            entry.vid = atoi(var_string);
        }

        // Get start MAC address
        if ((var_string = cyg_httpd_form_varable_find(p, "DynGetNextAddr")) != NULL) {
            if  (sscanf((char *)var_string, "%2x-%2x-%2x-%2x-%2x-%2x",
                        &mac_addr[0], &mac_addr[1], &mac_addr[2],
                        &mac_addr[3], &mac_addr[4], &mac_addr[5]) == 6) {
                for (i = 0; i < 6; i++) {
                    entry.mac[i] = (u8) mac_addr[i];
                }
            }
        }

        // Get start IP address
        if ((var_string = cyg_httpd_form_varable_find(p, "DynGetNextIPAddr")) != NULL) {
            (void)mgmt_txt2ipv4((char *)var_string, &entry.assigned_ip, &dummy, 0);
        }

        // Get or GetNext
        if ((var_string = cyg_httpd_form_varable_find(p, "GetNextEntry")) != NULL) {
            dyn_get_next_entry = atoi(var_string);
        }
        if (!dyn_get_next_entry && entry.assigned_ip != 0) {
            entry.assigned_ip--;
        }

        /* get form data
           Format: <start_port_no>/<start_vid>/<start_mac_addr>/<start_ip_addr>/<num_of_entries>|<port_no>/<vid>/<mac_addr>/<ip_addr>|...
        */
        if (!dyn_get_next_entry) { /* not get next button */
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d/%d/%s/%s/%d|",
                          iport2uport(entry.port_no),
                          entry.vid,
                          misc_mac_txt(entry.mac, buf1),
                          misc_ipv4_txt(entry.assigned_ip, buf2),
                          num_of_entries_arp);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        }

        search_entry = entry;
        if (arp_inspection_mgmt_conf_port_mode_get(sid, &port_mode_conf) == VTSS_OK) {
            while (arp_inspection_mgmt_conf_dynamic_entry_get(&search_entry, TRUE) == VTSS_OK) {
                if (search_entry.isid != sid) {
                    continue;
                }
                if (++entry_cnt > num_of_entries_arp) {
                    break;
                }
                if (dyn_get_next_entry && entry_cnt == 1) {  /* just for get next button */
                    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d/%d/%s/%s/%d|",
                                  iport2uport(search_entry.port_no),
                                  search_entry.vid,
                                  misc_mac_txt(search_entry.mac, buf1),
                                  misc_ipv4_txt(search_entry.assigned_ip, buf2),
                                  num_of_entries_arp);
                    (void)cyg_httpd_write_chunked(p->outbuffer, ct);
                }
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d/%d/%s/%s|",
                              iport2uport(search_entry.port_no),
                              search_entry.vid,
                              misc_mac_txt(search_entry.mac, buf1),
                              misc_ipv4_txt(search_entry.assigned_ip, buf2));
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            }
        }

        if (entry_cnt == 0) { /* No entry existing */
            if (dyn_get_next_entry) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d/%d/%s/%s/%d|",
                              iport2uport(entry.port_no),
                              entry.vid,
                              misc_mac_txt(entry.mac, buf1),
                              misc_ipv4_txt(entry.assigned_ip == 0 ? entry.assigned_ip : entry.assigned_ip + 1, buf2),
                              num_of_entries_arp);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            }
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "NoEntries");
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        }

        (void)cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}

static cyg_int32 handler_config_arp_inspection_vlan(CYG_HTTPD_STATE *p)
{
    arp_inspection_vlan_mode_conf_t vlan_mode_conf;
    int                             ct, entry_index, vid_temp, log_type;
    int                             entry_cnt, max_entries, remained_entries, idx;
    char                            search_str[64];
    char                            search_str2[64];
    BOOL                            change_flag = FALSE;

    int                             prev_start_vid = 1;
    int                             prev_number_of_entries = 20;
    int                             number_of_entries = 0;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_SECURITY)) {
        return -1;
    }
#endif

    if (p->method == CYG_HTTPD_METHOD_POST) {

#if 0
        (void) port_iter_init(&pit, NULL, sid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT);
        while (port_iter_getnext(&pit)) {
            if (arp_inspection_mgmt_conf_static_entry_get(&entry, FALSE) == VTSS_OK) {
                if ((entry.isid == sid) && (entry.port_no == pit.iport)) {
                    //CPRINTF("%4ld  %4ld  %17s  %-15s\n", iport2uport(entry.port_no), entry.vid, misc_mac_txt(entry.mac, buf1), misc_ipv4_txt(entry.assigned_ip, buf2));
                    sprintf(search_str, "delete_%ld_%d_%s_%s",
                            iport2uport(pit.iport),
                            entry.vid,
                            misc_mac_txt(entry.mac, buf1),
                            misc_ipv4_txt(entry.assigned_ip, buf2));
                    if (cyg_httpd_form_varable_find(p, search_str)) { /* "delete" if checked */
                        (void) arp_inspection_mgmt_conf_static_entry_del(&entry);
                    }
                }
                while (arp_inspection_mgmt_conf_static_entry_get(&entry, TRUE) == VTSS_OK) {
                    if ((entry.isid == sid) && (entry.port_no == pit.iport)) {
                        sprintf(search_str, "delete_%ld_%d_%s_%s",
                                iport2uport(pit.iport),
                                entry.vid,
                                misc_mac_txt(entry.mac, buf1),
                                misc_ipv4_txt(entry.assigned_ip, buf2));
                        if (cyg_httpd_form_varable_find(p, search_str)) { /* "delete" if checked */
                            (void) arp_inspection_mgmt_conf_static_entry_del(&entry);
                        }
                    }
                }
            }
        }
#endif

        // delete the entries if the user checked
        for (entry_index = 1; entry_index <= ARP_INSPECTION_MAX_VLAN_CNT; entry_index++) {
            sprintf(search_str, "delete_%d", entry_index);

            if (cyg_httpd_form_varable_find(p, search_str)) { /* "delete" if checked */
                vlan_mode_conf.flags = 0;
                (void) arp_inspection_mgmt_conf_vlan_mode_set(entry_index, &vlan_mode_conf);
                change_flag = TRUE;
            }
        }

        // insert new entries
        for (entry_index = 1; entry_index <= ARP_INSPECTION_MAX_VLAN_CNT; entry_index++) {
            sprintf(search_str, "new_vid_%d", entry_index);
            sprintf(search_str2, "new_log_%d", entry_index);

            // init the value
            vlan_mode_conf.flags = 0;

            if (cyg_httpd_form_varable_int(p, search_str2, &log_type) &&
                cyg_httpd_form_varable_int(p, search_str, &vid_temp)) {

                vlan_mode_conf.flags |= ARP_INSPECTION_VLAN_MODE;

                switch (log_type) {
                case ARP_INSPECTION_LOG_NONE:
                    vlan_mode_conf.flags &= ~ARP_INSPECTION_VLAN_LOG_DENY;
                    vlan_mode_conf.flags &= ~ARP_INSPECTION_VLAN_LOG_PERMIT;
                    break;
                case ARP_INSPECTION_LOG_DENY:
                    vlan_mode_conf.flags |= ARP_INSPECTION_VLAN_LOG_DENY;
                    vlan_mode_conf.flags &= ~ARP_INSPECTION_VLAN_LOG_PERMIT;
                    break;
                case ARP_INSPECTION_LOG_PERMIT:
                    vlan_mode_conf.flags &= ~ARP_INSPECTION_VLAN_LOG_DENY;
                    vlan_mode_conf.flags |= ARP_INSPECTION_VLAN_LOG_PERMIT;
                    break;
                case ARP_INSPECTION_LOG_ALL:
                    vlan_mode_conf.flags |= ARP_INSPECTION_VLAN_LOG_DENY;
                    vlan_mode_conf.flags |= ARP_INSPECTION_VLAN_LOG_PERMIT;
                    break;
                default:
                    vlan_mode_conf.flags &= ~ARP_INSPECTION_VLAN_LOG_DENY;
                    vlan_mode_conf.flags &= ~ARP_INSPECTION_VLAN_LOG_PERMIT;
                    break;
                }

                (void) arp_inspection_mgmt_conf_vlan_mode_set(vid_temp, &vlan_mode_conf);
                change_flag = TRUE;
            }
        }

        if (change_flag) {
            (void) arp_inspection_mgmt_conf_vlan_mode_save();
        }

        redirect(p, "/arp_inspection_vlan.htm");
    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        (void)cyg_httpd_start_chunked("html");

        /* get form data
           Format: <max_entries_num>,<vid>/<log_type>|...,<remained_entries>
        */

        // Get the start vid from the web page.
        prev_start_vid = httpd_form_get_value_int(p, "DynStartVid", 1, 4095);
        // Get the number of number of entries shown per page. Here we don't care about how many
        // that can de shown in per page. This is controlled in the .htm code. We simply set the
        // max value to a "high" number (9999).
        prev_number_of_entries = httpd_form_get_value_int(p, "DynNumberOfEntries", 1, 9999);

        /* max entries num */
        max_entries = ARP_INSPECTION_MAX_VLAN_CNT;
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d,", max_entries);
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);

        /* all entries which are already inserted into static table */
        entry_cnt = 0;

        for (idx = VLAN_ID_MIN; idx <= VLAN_ID_MAX; idx++) {
            // get configuration
            if (arp_inspection_mgmt_conf_vlan_mode_get(idx, &vlan_mode_conf, FALSE) != VTSS_OK) {
                continue;
            }

            if (vlan_mode_conf.flags & ARP_INSPECTION_VLAN_MODE) {

                if (idx >= prev_start_vid && number_of_entries < prev_number_of_entries) {
                    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d/%d|",
                                  idx, parse_log_type_for_vlan(vlan_mode_conf.flags));
                    (void)cyg_httpd_write_chunked(p->outbuffer, ct);
                    number_of_entries++;
                }
                entry_cnt++;
            }
        }

        /* remained entries */
        remained_entries = ARP_INSPECTION_MAX_VLAN_CNT - entry_cnt;
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), ",%d", remained_entries);
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);

        /* end */
        (void)cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}

static cyg_int32 handler_config_arp_inspection_dynamic_table(CYG_HTTPD_STATE *p)
{
    vtss_isid_t             sid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    int                     ct;
    int                     entry_cnt = 0;
    arp_inspection_entry_t  entry;
    char                    search_str[64];
    char                    buf1[80], buf2[80];
    port_iter_t             pit;


    arp_inspection_entry_t          search_entry;
    arp_inspection_port_mode_conf_t port_mode_conf;
    uint                            mac_addr[6];
    int                             i;
    int                             num_of_entries_arp = 0;
    int                             dyn_get_next_entry = 0;
    const char                      *var_string;
    ulong                           dummy;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_SECURITY)) {
        return -1;
    }
#endif

    if (p->method == CYG_HTTPD_METHOD_POST) {

        (void) port_iter_init(&pit, NULL, sid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT);
        while (port_iter_getnext(&pit)) {
            if (arp_inspection_mgmt_conf_dynamic_entry_get(&entry, FALSE) == VTSS_OK) {
                if ((entry.isid == sid) && (entry.port_no == pit.iport)) {
                    //CPRINTF("%4ld  %4d  %17s  %-15s\n", iport2uport(entry.port_no), entry.vid, misc_mac_txt(entry.mac, buf1), misc_ipv4_txt(entry.assigned_ip, buf2));
                    sprintf(search_str, "translate_%d_%d_%s_%s",
                            iport2uport(pit.iport),
                            entry.vid,
                            misc_mac_txt(entry.mac, buf1),
                            misc_ipv4_txt(entry.assigned_ip, buf2));
                    if (cyg_httpd_form_varable_find(p, search_str)) { /* "delete" if checked */
                        (void) arp_inspection_mgmt_conf_translate_dynamic_entry_into_static_entry(&entry);
                    }
                }
                while (arp_inspection_mgmt_conf_dynamic_entry_get(&entry, TRUE) == VTSS_OK) {
                    if ((entry.isid == sid) && (entry.port_no == pit.iport)) {
                        //CPRINTF("%4ld  %4d  %17s  %-15s\n", iport2uport(entry.port_no), entry.vid, misc_mac_txt(entry.mac, buf1), misc_ipv4_txt(entry.assigned_ip, buf2));
                        sprintf(search_str, "translate_%d_%d_%s_%s",
                                iport2uport(pit.iport),
                                entry.vid,
                                misc_mac_txt(entry.mac, buf1),
                                misc_ipv4_txt(entry.assigned_ip, buf2));
                        if (cyg_httpd_form_varable_find(p, search_str)) { /* "delete" if checked */
                            (void) arp_inspection_mgmt_conf_translate_dynamic_entry_into_static_entry(&entry);
                        }
                    }
                }
            }
        }

        redirect(p, "/dynamic_arp_inspection.htm");
    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        (void)cyg_httpd_start_chunked("html");

        memset(&entry, 0x0, sizeof(entry));
        entry.isid = sid;
        entry.vid = 1;

        // Get number of entries per page
        if ((var_string = cyg_httpd_form_varable_find(p, "DynNumberOfEntries")) != NULL) {
            num_of_entries_arp = atoi(var_string);
        }
        if (num_of_entries_arp <= 0 || num_of_entries_arp > 99) {
            num_of_entries_arp = 20;
        }

        // Get start port
        if ((var_string = cyg_httpd_form_varable_find(p, "port")) != NULL) {
            entry.port_no = uport2iport(atoi(var_string));
        }

        // Get start vid
        if ((var_string = cyg_httpd_form_varable_find(p, "DynStartVid")) != NULL) {
            entry.vid = atoi(var_string);
        }

        // Get start MAC address
        if ((var_string = cyg_httpd_form_varable_find(p, "DynGetNextAddr")) != NULL) {
            if  (sscanf((char *)var_string, "%2x-%2x-%2x-%2x-%2x-%2x",
                        &mac_addr[0], &mac_addr[1], &mac_addr[2],
                        &mac_addr[3], &mac_addr[4], &mac_addr[5]) == 6) {
                for (i = 0; i < 6; i++) {
                    entry.mac[i] = (u8) mac_addr[i];
                }
            }
        }

        // Get start IP address
        if ((var_string = cyg_httpd_form_varable_find(p, "DynGetNextIPAddr")) != NULL) {
            (void)mgmt_txt2ipv4((char *)var_string, &entry.assigned_ip, &dummy, 0);
        }

        // Get or GetNext
        if ((var_string = cyg_httpd_form_varable_find(p, "GetNextEntry")) != NULL) {
            dyn_get_next_entry = atoi(var_string);
        }
        if (!dyn_get_next_entry && entry.assigned_ip != 0) {
            entry.assigned_ip--;
        }

        /* get form data
           Format: <start_port_no>/<start_vid>/<start_mac_addr>/<start_ip_addr>/<num_of_entries>|<port_no>/<vid>/<mac_addr>/<ip_addr>|...
        */
        if (!dyn_get_next_entry) { /* not get next button */
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d/%d/%s/%s/%d|",
                          iport2uport(entry.port_no),
                          entry.vid,
                          misc_mac_txt(entry.mac, buf1),
                          misc_ipv4_txt(entry.assigned_ip, buf2),
                          num_of_entries_arp);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        }

        search_entry = entry;
        if (arp_inspection_mgmt_conf_port_mode_get(sid, &port_mode_conf) == VTSS_OK) {
            while (arp_inspection_mgmt_conf_dynamic_entry_get(&search_entry, TRUE) == VTSS_OK) {
                if (search_entry.isid != sid) {
                    continue;
                }
                if (++entry_cnt > num_of_entries_arp) {
                    break;
                }
                if (dyn_get_next_entry && entry_cnt == 1) {  /* just for get next button */
                    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d/%d/%s/%s/%d|",
                                  iport2uport(search_entry.port_no),
                                  search_entry.vid,
                                  misc_mac_txt(search_entry.mac, buf1),
                                  misc_ipv4_txt(search_entry.assigned_ip, buf2),
                                  num_of_entries_arp);
                    (void)cyg_httpd_write_chunked(p->outbuffer, ct);
                }
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d/%d/%s/%s|",
                              iport2uport(search_entry.port_no),
                              search_entry.vid,
                              misc_mac_txt(search_entry.mac, buf1),
                              misc_ipv4_txt(search_entry.assigned_ip, buf2));
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            }
        }

        if (entry_cnt == 0) { /* No entry existing */
            if (dyn_get_next_entry) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d/%d/%s/%s/%d|",
                              iport2uport(entry.port_no),
                              entry.vid,
                              misc_mac_txt(entry.mac, buf1),
                              misc_ipv4_txt(entry.assigned_ip == 0 ? entry.assigned_ip : entry.assigned_ip + 1, buf2),
                              num_of_entries_arp);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            }
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "NoEntries");
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        }

        (void)cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}


/****************************************************************************/
/*  HTTPD Table Entries                                                     */
/****************************************************************************/

CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_arp_inspection, "/config/arp_inspection", handler_config_arp_inspection);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_arp_inspection_static_table, "/config/arp_inspection_static_table", handler_config_arp_inspection_static_table);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_stat_arp_inspection, "/stat/dynamic_arp_inspection", handler_stat_dynamic_arp_inspection);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_arp_inspection_vlan, "/config/arp_inspection_vlan", handler_config_arp_inspection_vlan);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_arp_inspection_dynamic_table, "/config/arp_inspection_dynamic_table", handler_config_arp_inspection_dynamic_table);

/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
