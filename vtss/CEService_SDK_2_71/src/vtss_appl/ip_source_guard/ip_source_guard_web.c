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
#include "ip_source_guard_api.h"
#include "port_api.h"
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

/****************************************************************************/
/*  Web Handler  functions                                                  */
/****************************************************************************/

static cyg_int32 handler_config_ip_source_guard(CYG_HTTPD_STATE *p)
{
    vtss_isid_t                                 sid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    int                                         ct;
    vtss_uport_no_t                             uport;
    ulong                                       mode = 0;
    ip_source_guard_port_mode_conf_t            port_mode_conf;
    ip_source_guard_port_dynamic_entry_conf_t   dynamic_entry_conf;
    const char                                  *var_string;
    port_iter_t                                 pit;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_SECURITY)) {
        return -1;
    }
#endif

    (void) ip_source_guard_mgmt_conf_get_port_mode(sid, &port_mode_conf);
    (void) ip_source_guard_mgmt_conf_get_port_dynamic_entry_cnt(sid, &dynamic_entry_conf);

    if (p->method == CYG_HTTPD_METHOD_POST) {
        if (cyg_httpd_form_varable_long_int(p, "ip_source_guard_mode", &mode)) {
            if (ip_source_guard_mgmt_conf_set_mode(mode) != VTSS_OK) {
                T_E("ip_source_guard_mgmt_conf_set_mode() failed");
            }
        }
        (void) port_iter_init(&pit, NULL, sid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT);
        while (port_iter_getnext(&pit)) {
            uport = iport2uport(pit.iport);
            if (!cyg_httpd_form_variable_long_int_fmt(p, &port_mode_conf.mode[pit.iport], "port_mode_%d", uport) ||
                !cyg_httpd_form_variable_long_int_fmt(p, &dynamic_entry_conf.entry_cnt[pit.iport], "max_dynamic_clients_%d", uport)) {
                continue;
            }
        }
        if (ip_source_guard_mgmt_conf_set_port_mode(sid, &port_mode_conf) != VTSS_OK) {
            T_E("ip_source_guard_mgmt_conf_set_port_mode() failed");
        }
        if (ip_source_guard_mgmt_conf_set_port_dynamic_entry_cnt(sid, &dynamic_entry_conf) != VTSS_OK) {
            T_E("ip_source_guard_mgmt_conf_set_port_dynamic_entry_cnt() failed");
        }

        redirect(p, "/ip_source_guard.htm");
    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        (void)cyg_httpd_start_chunked("html");

        /* translate dynamic entries into static entries */
        if ((var_string = cyg_httpd_form_varable_find(p, "translateFlag")) != NULL && atoi(var_string) == 1 ) {
            if (ip_source_guard_mgmt_conf_translate_dynamic_into_static() < VTSS_OK) {
                T_W("ip source guard translation failed");
            }
        }

        /* get form data
           Format: <mode>,[port_no]/[port_mode]/[max_dynamic_clients]|...
        */
        if (ip_source_guard_mgmt_conf_get_mode(&mode) == VTSS_OK) {
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d,", mode);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);

            (void) port_iter_init(&pit, NULL, sid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT);
            while (port_iter_getnext(&pit)) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d/%d/%d|",
                              iport2uport(pit.iport),
                              port_mode_conf.mode[pit.iport],
                              dynamic_entry_conf.entry_cnt[pit.iport]);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            }
        }
        (void)cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}

static cyg_int32 handler_config_ip_source_guard_static_table(CYG_HTTPD_STATE *p)
{
    vtss_isid_t             sid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    int                     ct, entry_index;
    int                     entry_cnt, max_entries, remained_entries;
    ip_source_guard_entry_t entry;
    char                    search_str[64];
    char                    search_str2[64];
    char                    search_str3[64];
    char                    search_str4[64];
    char                    buf1[80], buf2[80], buf3[80];
    int                     vid_temp;
    port_iter_t             pit;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_SECURITY)) {
        return -1;
    }
#endif

    if (p->method == CYG_HTTPD_METHOD_POST) {

        (void) port_iter_init(&pit, NULL, sid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT);
        while (port_iter_getnext(&pit)) {
            if (ip_source_guard_mgmt_conf_get_first_static_entry(&entry) == VTSS_OK) {
                if (entry.isid == sid && entry.port_no == pit.iport) {
                    sprintf(search_str, "delete_%d_%d_%s_%s",
                            iport2uport(pit.iport),
                            entry.vid,
                            misc_ipv4_txt(entry.assigned_ip, buf1),
#if defined(VTSS_FEATURE_ACL_V2)
                            misc_mac_txt(entry.assigned_mac, buf2)
#else
                            misc_ipv4_txt(entry.ip_mask, buf2)
#endif /* VTSS_FEATURE_ACL_V2 */
                           );
                    if (cyg_httpd_form_varable_find(p, search_str)) {   /* "delete" if checked */
                        (void) ip_source_guard_mgmt_conf_del_static_entry(&entry);
                    }
                }

                while (ip_source_guard_mgmt_conf_get_next_static_entry(&entry) == VTSS_OK) {
                    if (entry.isid == sid && entry.port_no == pit.iport) {
                        sprintf(search_str, "delete_%d_%d_%s_%s",
                                iport2uport(pit.iport),
                                entry.vid,
                                misc_ipv4_txt(entry.assigned_ip, buf1),
#if defined(VTSS_FEATURE_ACL_V2)
                                misc_mac_txt(entry.assigned_mac, buf2)
#else
                                misc_ipv4_txt(entry.ip_mask, buf2)
#endif /* VTSS_FEATURE_ACL_V2 */
                               );
                        if (cyg_httpd_form_varable_find(p, search_str)) {   /* "delete" if checked */
                            (void) ip_source_guard_mgmt_conf_del_static_entry(&entry);
                        }
                    }
                }
            }
        }
        // Set new entry
        for (entry_index = 1; entry_index <= IP_SOURCE_GUARD_MAX_ENTRY_CNT; entry_index++) {
            memset(&entry, 0, sizeof(entry));
            sprintf(search_str, "new_port_no_%d", entry_index);
            sprintf(search_str2, "new_vid_%d", entry_index);
            sprintf(search_str3, "new_ip_addr_%d", entry_index);
#if defined(VTSS_FEATURE_ACL_V2)
            sprintf(search_str4, "new_mac_addr_%d", entry_index);
#else
            sprintf(search_str4, "new_ip_mask_%d", entry_index);
#endif /* VTSS_FEATURE_ACL_V2 */

            if (cyg_httpd_form_varable_long_int(p, search_str, &entry.port_no) &&
                cyg_httpd_form_varable_int(p, search_str2, &vid_temp) &&
                cyg_httpd_form_varable_ipv4(p, search_str3, &entry.assigned_ip) &&
#if defined(VTSS_FEATURE_ACL_V2)
                cyg_httpd_form_varable_mac(p, search_str4, entry.assigned_mac)
#else
                cyg_httpd_form_varable_ipv4(p, search_str4, &entry.ip_mask)
#endif /* VTSS_FEATURE_ACL_V2 */
               ) {
                entry.isid = sid;
                entry.type = IP_SOURCE_GUARD_STATIC_TYPE;
                entry.valid = 1;
                entry.vid = vid_temp;
                entry.port_no = uport2iport(entry.port_no);
#if defined(VTSS_FEATURE_ACL_V2)
                entry.ip_mask = 0xFFFFFFFF;
#endif /* VTSS_FEATURE_ACL_V2 */
                if (ip_source_guard_mgmt_conf_set_static_entry(&entry) != VTSS_OK) {
                    T_D("ip_source_guard_mgmt_conf_set_static_entry() failed");
                }
            }
        }

        redirect(p, "/ip_source_guard_static_table.htm");
    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        (void)cyg_httpd_start_chunked("html");

        /* get form data
           Format: <total_max_entries_num>,<port_no>/<vid>/<ip_addr>/<ip_mask>/<mac_addr>|...,<remained_entries_in_sid>
        */

        /* max entries num */
        max_entries = IP_SOURCE_GUARD_MAX_ENTRY_CNT;
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d,", max_entries);
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);

        /* all entries which are already inserted into static table */
        entry_cnt = 0;
        if (ip_source_guard_mgmt_conf_get_first_static_entry(&entry) == VTSS_OK) {
            if (entry.isid == sid) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d/%d/%s/%s/%s|",
                              iport2uport(entry.port_no),
                              entry.vid,
                              misc_ipv4_txt(entry.assigned_ip, buf1),
                              misc_ipv4_txt(entry.ip_mask, buf2),
                              misc_mac_txt(entry.assigned_mac, buf3));
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            } else {
                entry_cnt++;
            }

            while (ip_source_guard_mgmt_conf_get_next_static_entry(&entry) == VTSS_OK) {
                if (entry.isid != sid) {
                    entry_cnt++;
                    continue;
                }
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d/%d/%s/%s/%s|",
                              iport2uport(entry.port_no),
                              entry.vid,
                              misc_ipv4_txt(entry.assigned_ip, buf1),
                              misc_ipv4_txt(entry.ip_mask, buf2),
                              misc_mac_txt(entry.assigned_mac, buf3));
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            }
        }

        /* remained entries in sid switch */
        remained_entries = IP_SOURCE_GUARD_MAX_ENTRY_CNT - entry_cnt;
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), ",%d", remained_entries);
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);

        /* end */
        (void)cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}

static cyg_int32 handler_stat_dynamic_ip_source_guard(CYG_HTTPD_STATE *p)
{
    vtss_isid_t             sid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    int                     ct;
    ip_source_guard_entry_t entry, search_entry;
    char                    buf1[80], buf2[80];
    int                     entry_cnt = 0;
    int                     num_of_entries_ip_source_guard = 0;
    int                     dyn_get_next_entry = 0;
    const char              *var_string;
    vtss_ipv4_t             dummy;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_STAT_TYPE, VTSS_MODULE_ID_SECURITY)) {
        return -1;
    }
#endif

    if (p->method == CYG_HTTPD_METHOD_POST) {
        redirect(p, "/dyna_ip_source_guard.htm");
    } else { /* CYG_HTTPD_METHOD_GET (+HEAD) */
        (void)cyg_httpd_start_chunked("html");

        memset(&entry, 0x0, sizeof(entry));
        entry.isid = sid;
        entry.vid = 1;

        // Get number of entries per page
        if ((var_string = cyg_httpd_form_varable_find(p, "DynNumberOfEntries")) != NULL) {
            num_of_entries_ip_source_guard = atoi(var_string);
        }
        if (num_of_entries_ip_source_guard <= 0 || num_of_entries_ip_source_guard > 99) {
            num_of_entries_ip_source_guard = 20;
        }

        // Get start port
        if ((var_string = cyg_httpd_form_varable_find(p, "port")) != NULL) {
            entry.port_no = uport2iport(atoi(var_string));
        }

        // Get start vid
        if ((var_string = cyg_httpd_form_varable_find(p, "DynStartVid")) != NULL) {
            entry.vid = atoi(var_string);
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
           Format: <start_port_no>/<start_vid>/<start_ip_addr>/<num_of_entries>|<port_no>/<vid>/<ip_addr>/<mac_addr>|...
        */
        if (!dyn_get_next_entry) { /* not get next button */
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d/%d/%s/%d|",
                          iport2uport(entry.port_no),
                          entry.vid,
                          misc_ipv4_txt(entry.assigned_ip, buf1),
                          num_of_entries_ip_source_guard);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        }

        search_entry = entry;
        while (ip_source_guard_mgmt_conf_get_next_dynamic_entry(&search_entry) == VTSS_OK) {
            if (search_entry.isid != sid) {
                continue;
            }
            if (++entry_cnt > num_of_entries_ip_source_guard) {
                break;
            }
            if (dyn_get_next_entry && (entry_cnt == 1)) { /* Only for GetNext button */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d/%d/%s/%d|",
                              iport2uport(search_entry.port_no),
                              search_entry.vid,
                              misc_ipv4_txt(search_entry.assigned_ip, buf1),
                              num_of_entries_ip_source_guard);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            }
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d/%d/%s/%s|",
                          iport2uport(search_entry.port_no),
                          search_entry.vid,
                          misc_ipv4_txt(search_entry.assigned_ip, buf1),
                          misc_mac_txt(search_entry.assigned_mac, buf2));
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        }

        if (entry_cnt == 0) { /* No entry existing */
            if (dyn_get_next_entry) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d/%d/%s/%d|",
                              iport2uport(entry.port_no),
                              entry.vid,
                              misc_ipv4_txt(entry.assigned_ip == 0 ? entry.assigned_ip : entry.assigned_ip + 1, buf1),
                              num_of_entries_ip_source_guard);
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

CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_ip_source_guard, "/config/ip_source_guard", handler_config_ip_source_guard);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_ip_source_guard_static_table, "/config/ip_source_guard_static_table", handler_config_ip_source_guard_static_table);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_stat_ip_source_guard, "/stat/dynamic_ip_source_guard", handler_stat_dynamic_ip_source_guard);

/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
