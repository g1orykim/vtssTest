/*

  Vitesse Switch API software.

 Copyright (c) 2002-2014 Vitesse Semiconductor Corporation "Vitesse". All
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
#include "os_file_api.h"
#include <dirent.h>
#include <unistd.h>

#include "../base/src/vtss_gvrp.h"
#include "xxrp_api.h"
#include "vtss_xxrp_callout.h"

#ifdef VTSS_SW_OPTION_PRIV_LVL
#include "vtss_privilege_api.h"
#include "vtss_privilege_web_api.h"
#endif

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_XXRP
#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_XXRP

static const
struct {
    char *name;
    enum timer_context tc;
} elem_name[3] = {
    {"jointime", GARP_TC__transmitPDU},
    {"leavetime", GARP_TC__leavetimer},
    {"leavealltime", GARP_TC__leavealltimer}
};



static cyg_int32 gvrp_handler_conf_status(CYG_HTTPD_STATE *httpd_state)
{
    T_D("entry");

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(httpd_state, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_XXRP)) {
        goto early_out;
    }
#endif

    switch (httpd_state->method) {
    case CYG_HTTPD_METHOD_GET: {
        int cnt;
        char str[32]; // 2+1+2+1+3+1+5+1= 16, so plus some
        char encoded_string[32]; // 2+1+2+1+3+1+5+1= 16, so plus some

        u32 jointtime = vtss_gvrp_get_timer(GARP_TC__transmitPDU);
        u32 leavetime = vtss_gvrp_get_timer(GARP_TC__leavetimer);
        u32 leavealltime = vtss_gvrp_get_timer(GARP_TC__leavealltimer);
        u32 maxvlans = vtss_gvrp_max_vlans();
        int enabled = vtss_gvrp_is_enabled();

        sprintf(str, "OK*%d*%d*%d*%d*%d", jointtime, leavetime, leavealltime, maxvlans, enabled);

        (void)cyg_httpd_start_chunked("html");
        cnt = cgi_escape(str, encoded_string);
        (void)cyg_httpd_write_chunked(encoded_string, cnt);
        (void)cyg_httpd_end_chunked();
    }
    break;

    case CYG_HTTPD_METHOD_POST: {
        const char  *var_p;
        size_t var_len, var_len2;
        char str[10];
        int i;

        str[9] = 0;

        (void)cyg_httpd_form_varable_string(httpd_state, "gvrp_enable", &var_len2);

        var_p = cyg_httpd_form_varable_string(httpd_state, "maxvlans", &var_len);
        str[0] = 0;
        if (var_p && var_len) {
            strncpy(str, var_p, MIN(sizeof(str) - 1, var_len));
            str[MIN(sizeof(str) - 1, var_len)] = 0;
        }

        // (1) --- Get global GVRP settings.
        if (0 == var_len2) {
            // --- Disable GVRP
            (void)xxrp_mgmt_global_enabled_set(VTSS_GARP_APPL_GVRP, 0);
            GVRP_CRIT_ENTER();
            vtss_gvrp_destruct(FALSE);
            GVRP_CRIT_EXIT();
            vtss_gvrp_max_vlans_set(atoi(str));

        } else {
            // --- Enable GVRP. Get how many VLANs shall be possible.

            (void)vtss_gvrp_construct(-1, atoi(str));
            (void)xxrp_mgmt_global_enabled_set(VTSS_GARP_APPL_GVRP, 1);
        }


        // --- Get  join-time, leave-time, leaveall-time.
        for (i = 0; i < 3; ++i) {

            var_p = cyg_httpd_form_varable_string(httpd_state, elem_name[i].name, &var_len);
            str[0] = 0;
            if (var_p && var_len) {
                strncpy(str, var_p, MIN(sizeof(str) - 1, var_len));
                str[MIN(sizeof(str) - 1, var_len)] = 0;
            }

            (void)vtss_gvrp_set_timer(elem_name[i].tc, atoi(str));
        }
        redirect(httpd_state, "/gvrp_config.htm");
    }
    break;

    default:
        break;
    }

early_out:

    return -1;
}



static cyg_int32 gvrp_handler_port_enable(CYG_HTTPD_STATE *httpd_state)
{
    vtss_isid_t    isid  = web_retrieve_request_sid(httpd_state); /* Includes USID = ISID */
    port_iter_t  pit;

    T_D("entry");

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(httpd_state, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_XXRP)) {
        goto early_out2;
    }
#endif

    switch (httpd_state->method) {
    case CYG_HTTPD_METHOD_GET: {
        static const char *const F[2] = {"%d.%d", "*%d.%d"};
        int cnt;
        char str[32];
        char encoded_string[32];
        BOOL enable;
        int first = 0;

        (void)cyg_httpd_start_chunked("html");

        (void)port_iter_init(&pit, 0, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
        while (port_iter_getnext(&pit)) {

            (void)xxrp_mgmt_enabled_get(isid, pit.iport, VTSS_GARP_APPL_GVRP, &enable);
            sprintf(str, F[first], pit.uport, enable ? 1 : 0);
            cnt = cgi_escape(str, encoded_string);
            (void)cyg_httpd_write_chunked(encoded_string, cnt);

            first = 1;
        }

        (void)cyg_httpd_end_chunked();
    }
    break;

    case CYG_HTTPD_METHOD_POST: {
        const char *var_p;
        size_t var_len;
        char str[10];

        str[9] = 0;

        (void)port_iter_init(&pit, 0, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
        while (port_iter_getnext(&pit)) {

            sprintf(str, "mode_%d", (int)pit.uport);
            var_p = cyg_httpd_form_varable_string(httpd_state, str, &var_len);

            if (var_len) {
                strncpy(str, var_p, MIN(sizeof(str) - 1, var_len));
                str[MIN(sizeof(str) - 1, var_len)] = 0;
                (void)xxrp_mgmt_enabled_set(isid, pit.iport, VTSS_GARP_APPL_GVRP, str[0] == '1');
            }
        }
        redirect(httpd_state, "/gvrp_port.htm");

    }
    break;

    default:
        break;
    }

early_out2:

    return -1;
}


/****************************************************************************/
/*  HTTPD Table Entries                                                     */
/****************************************************************************/

CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_gvrp_conf,       "/config/gvrp_conf_status",      gvrp_handler_conf_status);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_gvrp_conf_port,  "/config/gvrp_conf_port_enable", gvrp_handler_port_enable);

/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
