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
#include "mgmt_api.h"
#include "dhcp_snooping_api.h"
#include "port_api.h"
#include "msg_api.h"
#ifdef VTSS_SW_OPTION_PRIV_LVL
#include "vtss_privilege_api.h"
#include "vtss_privilege_web_api.h"
#endif
#include "misc_api.h"   //uport2iport(), iport2uport()
#include "topo_api.h"   //topo_usid2isid(), topo_isid2usid()

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

static cyg_int32 handler_config_dhcp_snooping(CYG_HTTPD_STATE *p)
{
    vtss_isid_t                 sid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    port_iter_t                 pit;
    int                         ct;
    dhcp_snooping_conf_t        conf, newconf;
    dhcp_snooping_port_conf_t   port_conf, port_newconf;
    int                         var_value;
    char                        var_name[32];

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_DHCP)) {
        return -1;
    }
#endif

    (void) port_iter_init(&pit, NULL, sid, PORT_ITER_SORT_ORDER_UPORT, PORT_ITER_FLAGS_NORMAL);

    if (p->method == CYG_HTTPD_METHOD_POST) {
        /* store form data */
        if (dhcp_snooping_mgmt_conf_get(&conf) != VTSS_OK ||
            dhcp_snooping_mgmt_port_conf_get(sid, &port_conf) != VTSS_OK) {
            redirect(p, "/dhcp_snooping.htm");
            return -1;
        }
        newconf = conf;

        /* snooping_mode */
        if (cyg_httpd_form_varable_int(p, "snooping_mode", &var_value)) {
            newconf.snooping_mode = var_value;
        }

        if (memcmp(&newconf, &conf, sizeof(newconf)) != 0) {
            T_D("Calling dhcp_snooping_mgmt_conf_set()");
            if (dhcp_snooping_mgmt_conf_set(&newconf) < 0) {
                T_E("dhcp_snooping_mgmt_conf_set(): failed");
            }
        }

        /* store form data */
        port_newconf = port_conf;

        while (port_iter_getnext(&pit)) {
            /* port_mode */
            sprintf(var_name, "port_mode_%u", pit.uport);
            if (cyg_httpd_form_varable_int(p, var_name, &var_value)) {
                port_newconf.port_mode[pit.iport] = var_value;
            }
        }

        if (memcmp(&port_newconf, &port_conf, sizeof(port_newconf)) != 0) {
            T_D("Calling dhcp_snooping_mgmt_port_conf_set()");
            if (dhcp_snooping_mgmt_port_conf_set(sid, &port_newconf) < 0) {
                T_E("dhcp_snooping_mgmt_port_conf_set(): failed");
            }
        }
        redirect(p, "/dhcp_snooping.htm");
    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        (void)cyg_httpd_start_chunked("html");

        /* get form data
           Format: [snooping_mode],[uport]/[port_mode]|...
        */
        if (dhcp_snooping_mgmt_conf_get(&conf) == VTSS_OK &&
            dhcp_snooping_mgmt_port_conf_get(sid, &port_conf) == VTSS_OK) {
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u,", conf.snooping_mode);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            while (port_iter_getnext(&pit)) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/%d|", pit.uport, port_conf.port_mode[pit.iport]);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            }
        }
        (void)cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}

#if 0 // use new DHCP Helper comment "show ip dhcp detailed statistics snooping"
static cyg_int32 handler_stat_dhcp_snooping_statistics(CYG_HTTPD_STATE *p)
{
    vtss_isid_t             sid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    vtss_port_no_t          iport;
    dhcp_snooping_stats_t   stats;
    int                     ct;
    const char              *var_string;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_STAT_TYPE, VTSS_MODULE_ID_DHCP)) {
        return -1;
    }
#endif

    (void)cyg_httpd_start_chunked("html");

    if ((var_string = cyg_httpd_form_varable_find(p, "port")) != NULL) {
        iport = uport2iport(atoi(var_string));
    } else {
        goto out;
    }

    if (!VTSS_ISID_LEGAL(sid) || !msg_switch_exists(sid)) {
        goto out;    /* Most likely stack error - bail out */
    }

    if ((cyg_httpd_form_varable_find(p, "clear") != NULL)) { /* Clear? */
        if (dhcp_snooping_stats_clear(sid, iport)) {
            goto out;
        }
    }

    if (dhcp_snooping_stats_get(sid, iport, &stats)) {
        goto out;
    }

    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u,%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u",
                  iport2uport(iport),
                  stats.rx_stats.discover_rx,        stats.tx_stats.discover_tx,
                  stats.rx_stats.offer_rx,           stats.tx_stats.offer_tx,
                  stats.rx_stats.request_rx,         stats.tx_stats.request_tx,
                  stats.rx_stats.decline_rx,         stats.tx_stats.decline_tx,
                  stats.rx_stats.ack_rx,             stats.tx_stats.ack_tx,
                  stats.rx_stats.nak_rx,             stats.tx_stats.nak_tx,
                  stats.rx_stats.release_rx,         stats.tx_stats.release_tx,
                  stats.rx_stats.inform_rx,          stats.tx_stats.inform_tx,
                  stats.rx_stats.leasequery_rx,      stats.tx_stats.leasequery_tx,
                  stats.rx_stats.leaseunassigned_rx, stats.tx_stats.leaseunassigned_tx,
                  stats.rx_stats.leaseunknown_rx,    stats.tx_stats.leaseunknown_tx,
                  stats.rx_stats.leaseactive_rx,     stats.tx_stats.leaseactive_tx,
                  stats.rx_stats.discard_untrust_rx, stats.rx_stats.discard_chksum_err_rx);
    (void)cyg_httpd_write_chunked(p->outbuffer, ct);

out:
    (void)cyg_httpd_end_chunked();
    return -1; // Do not further search the file system.
}
#endif

static cyg_int32 handler_stat_dynamic_dhcp_snooping(CYG_HTTPD_STATE *p)
{
    int                                 ct;
    dhcp_snooping_ip_assigned_info_t    entry, search_entry;
    char                                buf1[32], buf2[16], buf3[16], buf4[16];
    uint                                mac_addr[6] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
    int                                 i, entry_cnt = 0;
    int                                 num_of_entries = 0;
    int                                 dyn_get_next_entry = 0;
    const char                          *var_string;
    BOOL                                rc;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_STAT_TYPE, VTSS_MODULE_ID_DHCP)) {
        return -1;
    }
#endif

    if (p->method == CYG_HTTPD_METHOD_POST) {
        redirect(p, "/dyna_dhcp_snooping.htm");
    } else { /* CYG_HTTPD_METHOD_GET (+HEAD) */
        cyg_httpd_start_chunked("html");

        memset(&entry, 0x0, sizeof(entry));
        entry.vid = 0;

        // Get number of entries per page
        if ((var_string = cyg_httpd_form_varable_find(p, "DynNumberOfEntries")) != NULL) {
            num_of_entries = atoi(var_string);
        }
        if (num_of_entries <= 0 || num_of_entries > 99) {
            num_of_entries = 20;
        }

        // Get start vid
        if ((var_string = cyg_httpd_form_varable_find(p, "DynGetNextVid")) != NULL) {
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

        // Get or GetNext
        if ((var_string = cyg_httpd_form_varable_find(p, "GetNextEntry")) != NULL) {
            dyn_get_next_entry = atoi(var_string);
        }
        if (entry.vid != 0) {
            entry.vid--;
        }

        /* get form data
           Format: <start_mac_addr>/<start_vid>/<num_of_entries>|<mac_addr>/<vid>/<sid>/<port_no>/<ip_addr>/<ip_mask>/<server_ip>/<server_type>|...
        */
        search_entry = entry;
        do {
            if ((rc = dhcp_snooping_ip_assigned_info_getnext(search_entry.mac, search_entry.vid, &search_entry))) {
                if (++entry_cnt > num_of_entries) {
                    break;
                }
                if (entry_cnt == 1) {  /* just for get next button */
                    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s/%d/%d|",
                                  misc_mac_txt(search_entry.mac, buf1),
                                  search_entry.vid,
                                  num_of_entries);
                    cyg_httpd_write_chunked(p->outbuffer, ct);
                }
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s/%d/%d/%d/%s/%s/%s%s|",
                              misc_mac_txt(search_entry.mac, buf1),
                              search_entry.vid,
                              topo_usid2isid(search_entry.isid),
                              iport2uport(search_entry.port_no),
                              misc_ipv4_txt(search_entry.assigned_ip, buf2),
                              misc_ipv4_txt(search_entry.assigned_mask, buf3),
                              misc_ipv4_txt(search_entry.dhcp_server_ip, buf4),
#if defined(VTSS_SW_OPTION_DHCP_SERVER)
                              search_entry.local_dhcp_server ? " (Local)" : " (Remote)"
#else
                              ""
#endif /* VTSS_SW_OPTION_DHCP_SERVER */
                             );
                cyg_httpd_write_chunked(p->outbuffer, ct);
            }
        } while (rc);

        if (entry_cnt == 0) { /* No entry existing */
            if (dyn_get_next_entry) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s/%d/%d|NoEntries",
                              misc_mac_txt(entry.mac, buf1),
                              entry.vid,
                              num_of_entries);
                cyg_httpd_write_chunked(p->outbuffer, ct);
            }
        }

        cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}

/****************************************************************************/
/*  HTTPD Table Entries                                                     */
/****************************************************************************/

CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_dhcp_snooping, "/config/dhcp_snooping", handler_config_dhcp_snooping);
//CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_stat_dhcp_snooping, "/stat/dhcp_snooping_statistics", handler_stat_dhcp_snooping_statistics);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_dynamic_dhcp_snooping, "/stat/dynamic_dhcp_snooping", handler_stat_dynamic_dhcp_snooping);

/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
