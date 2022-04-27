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
#include "dhcp_helper_api.h"
#include "port_api.h"
#include "msg_api.h"
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

/****************************************************************************/
/*  Web Handler  functions                                                  */
/****************************************************************************/

static cyg_int32 handler_stat_dhcp_helper_statistics(CYG_HTTPD_STATE *p)
{
    vtss_isid_t         sid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    dhcp_helper_user_t  user = DHCP_HELPER_USER_CNT, user_idx;
    vtss_port_no_t      iport;
    dhcp_helper_stats_t stats;
    int                 ct, var_int;
    const char          *var_string;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_STAT_TYPE, VTSS_MODULE_ID_DHCP)) {
        return -1;
    }
#endif

    (void)cyg_httpd_start_chunked("html");

    if ((var_string = cyg_httpd_form_varable_find(p, "user")) != NULL) {
        var_int = atoi(var_string);
        if (var_int >= 0) {
            user = var_int;
        }
    } else {
        goto out;
    }

    if ((var_string = cyg_httpd_form_varable_find(p, "port")) != NULL) {
        iport = uport2iport(atoi(var_string));
    } else {
        goto out;
    }

    if (!VTSS_ISID_LEGAL(sid) || !msg_switch_configurable(sid)) {
        goto out;    /* Most likely stack error - bail out */
    }

    if ((cyg_httpd_form_varable_find(p, "clear") != NULL)) { /* Clear? */
        if (dhcp_helper_stats_clear(user, sid, iport)) {
            goto out;
        }
    }

    if (dhcp_helper_stats_get(user, sid, iport, &stats)) {
        goto out;
    }

    // Format: [select_user_id]/[user_name1]/[user_name2]/...,[port_no],[counter_1]/[counter_2]/.../[counter_n]
    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d/", user);
    (void)cyg_httpd_write_chunked(p->outbuffer, ct);
    for (user_idx = DHCP_HELPER_USER_HELPER; user_idx < DHCP_HELPER_USER_CNT; user_idx++) {
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s/", dhcp_helper_user_names[user_idx]);
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);
    }
    (void)cyg_httpd_write_chunked(",", 1);

    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u,%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u",
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
                  stats.rx_stats.discard_chksum_err_rx);
    (void)cyg_httpd_write_chunked(p->outbuffer, ct);

#if defined(VTSS_SW_OPTION_DHCP_SNOOPING)
    if (user == DHCP_HELPER_USER_SNOOPING || user == DHCP_HELPER_USER_CNT) {
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u", stats.rx_stats.discard_untrust_rx);
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);
    }
#endif /* VTSS_SW_OPTION_DHCP_SNOOPING */
out:
    (void)cyg_httpd_end_chunked();
    return -1; // Do not further search the file system.
}

/****************************************************************************/
/*  HTTPD Table Entries                                                     */
/****************************************************************************/

CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_stat_dhcp_helper, "/stat/dhcp_helper_statistics", handler_stat_dhcp_helper_statistics);

/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
