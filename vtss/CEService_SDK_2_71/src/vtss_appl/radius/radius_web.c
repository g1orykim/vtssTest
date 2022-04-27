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

#include "main.h"
#include "web_api.h"
#include "vtss_radius_api.h"
#ifdef VTSS_SW_OPTION_PRIV_LVL
#include "vtss_privilege_api.h"
#include "vtss_privilege_web_api.h"
#endif

#ifndef VTSS_SW_OPTION_AUTH
#define VTSS_AUTH_NUMBER_OF_SERVERS 5
#endif

/****************************************************************************/
/*  Web Handler  functions                                                  */
/****************************************************************************/

cyg_int32 handler_stat_auth_radius_overview(CYG_HTTPD_STATE *p)
{
    vtss_radius_all_server_status_s server_status;
    int                             server, cnt;
    char                            ip_buf[16];

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_STAT_TYPE, VTSS_MODULE_ID_SECURITY)) {
        return -1;
    }
#endif

    cyg_httpd_start_chunked("html");

    if (vtss_radius_auth_server_status_get(&server_status) == VTSS_OK) {
        // Format: <server_state_1>#<server_state_2>#...#<server_state_N>
        // where <server_state_X> == ip_addr/port/state/dead_time_left_secs
        for (server = 0; server < ARRSZ(server_status.status); server++) {
            cnt = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s/%u/%d/%u",
                           misc_ipv4_txt(server_status.status[server].ip_addr, ip_buf),
                           server_status.status[server].port,
                           server_status.status[server].state,
                           server_status.status[server].dead_time_left_secs);
            cyg_httpd_write_chunked(p->outbuffer, cnt);
            if (server < ARRSZ(server_status.status) - 1) {
                cyg_httpd_write_chunked("#", 1);
            }
        }
    }
    cyg_httpd_write_chunked("|", 1);

#ifdef VTSS_SW_OPTION_DOT1X_ACCT
    if (vtss_radius_acct_server_status_get(&server_status) == VTSS_OK) {
        // Format: <server_state_1>#<server_state_2>#...#<server_state_N>
        // where <server_state_X> == ip_addr/port/state/dead_time_left_secs
        for (server = 0; server < ARRSZ(server_status.status); server++) {
            cnt = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s/%u/%d/%u",
                           misc_ipv4_txt(server_status.status[server].ip_addr, ip_buf),
                           server_status.status[server].port,
                           server_status.status[server].state,
                           server_status.status[server].dead_time_left_secs);
            cyg_httpd_write_chunked(p->outbuffer, cnt);
            if (server < ARRSZ(server_status.status) - 1) {
                cyg_httpd_write_chunked("#", 1);
            }
        }
    }
#endif

    cyg_httpd_end_chunked();

    return -1; // Do not further search the file system.
}

/*lint -sem(handler_stat_auth_radius_details, thread_protected) ... There is only one httpd thread */
cyg_int32 handler_stat_auth_radius_details(CYG_HTTPD_STATE *p)
{
    vtss_radius_auth_client_server_mib_s mib_auth = {0};
#ifdef VTSS_SW_OPTION_DOT1X_ACCT
    vtss_radius_acct_client_server_mib_s mib_acct = {0};
#endif
    int                                  cnt, server, errors = 0;
    char                                 ip_buf[16];

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_STAT_TYPE, VTSS_MODULE_ID_SECURITY)) {
        return -1;
    }
#endif

    cyg_httpd_start_chunked("html");

    // server = %d (1-based)
    server = atoi(var_server);
    if (server < 1 || server > VTSS_AUTH_NUMBER_OF_SERVERS) {
        errors++;
    }

    server--; // 1-based in I/F. 0-based in code.

    // This function is also used to clear the counters, when the URL contains the string "clear=1"
    if (!errors && strcmp(var_clear, "1") == 0) {
        // If the port is MAC-based, this clears the summed up counters and all MAC-based state-machine counters.
        if (vtss_radius_auth_client_mib_clr(server) != VTSS_OK) {
            errors++;
        }
#ifdef VTSS_SW_OPTION_DOT1X_ACCT
        if (vtss_radius_acct_client_mib_clr(server) != VTSS_OK) {
            errors++;
        }
#endif
    }

    // Get the authentication statistics for this server
    if (!errors && vtss_radius_auth_client_mib_get(server, &mib_auth) != VTSS_OK) {
        errors++;
    }

    if (!errors) {
        // Format: server#ip_addr#udp_port#state#dead_time_left_secs#roundtriptime#cnt1#cnt2#...#cntN
        cnt = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d#%s#%u#%d#%u#%u ms#%u#%u#%u#%u#%u#%u#%u#%u#%u#%u#%u#%u",
                       server + 1, // 1-based
                       misc_ipv4_txt(mib_auth.radiusAuthServerInetAddress, ip_buf),
                       mib_auth.radiusAuthClientServerInetPortNumber,
                       mib_auth.state,
                       mib_auth.dead_time_left_secs,
                       mib_auth.radiusAuthClientExtRoundTripTime * ECOS_MSECS_PER_HWTICK, // We return it in milliseconds.
                       mib_auth.radiusAuthClientExtAccessRequests,
                       mib_auth.radiusAuthClientExtAccessRetransmissions,
                       mib_auth.radiusAuthClientExtAccessAccepts,
                       mib_auth.radiusAuthClientExtAccessRejects,
                       mib_auth.radiusAuthClientExtAccessChallenges,
                       mib_auth.radiusAuthClientExtMalformedAccessResponses,
                       mib_auth.radiusAuthClientExtBadAuthenticators,
                       mib_auth.radiusAuthClientExtPendingRequests,
                       mib_auth.radiusAuthClientExtTimeouts,
                       mib_auth.radiusAuthClientExtUnknownTypes,
                       mib_auth.radiusAuthClientExtPacketsDropped,
                       mib_auth.radiusAuthClientCounterDiscontinuity);
        cyg_httpd_write_chunked(p->outbuffer, cnt);
    }
    cyg_httpd_write_chunked("|", 1);

#ifdef VTSS_SW_OPTION_DOT1X_ACCT
    // Get the accounting statistics for this server
    if (!errors && vtss_radius_acct_client_mib_get(server, &mib_acct) != VTSS_OK) {
        errors++;
    }

    if (!errors) {
        // Format: server#ip_addr#udp_port#state#dead_time_left_secs#roundtriptime#cnt1#cnt2#...#cntN
        cnt = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d#%s#%u#%d#%u#%u ms#%u#%u#%u#%u#%u#%u#%u#%u#%u#%u",
                       server + 1, // 1-based
                       misc_ipv4_txt(mib_acct.radiusAccServerInetAddress, ip_buf),
                       mib_acct.radiusAccClientServerInetPortNumber,
                       mib_acct.state,
                       mib_acct.dead_time_left_secs,
                       mib_acct.radiusAccClientExtRoundTripTime * ECOS_MSECS_PER_HWTICK, // We return it in milliseconds.
                       mib_acct.radiusAccClientExtRequests,
                       mib_acct.radiusAccClientExtRetransmissions,
                       mib_acct.radiusAccClientExtResponses,
                       mib_acct.radiusAccClientExtMalformedResponses,
                       mib_acct.radiusAccClientExtBadAuthenticators,
                       mib_acct.radiusAccClientExtPendingRequests,
                       mib_acct.radiusAccClientExtTimeouts,
                       mib_acct.radiusAccClientExtUnknownTypes,
                       mib_acct.radiusAccClientExtPacketsDropped,
                       mib_acct.radiusAccClientCounterDiscontinuity);
        cyg_httpd_write_chunked(p->outbuffer, cnt);
    }
#endif

    cyg_httpd_end_chunked();

    return -1; // Do not further search the file system.
}

/****************************************************************************/
/*  HTTPD Table Entries                                                     */
/****************************************************************************/

CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_stat_auth_radius_overview, "/stat/auth_status_radius_overview", handler_stat_auth_radius_overview);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_stat_auth_radius_details, "/stat/auth_status_radius_details", handler_stat_auth_radius_details);

/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
