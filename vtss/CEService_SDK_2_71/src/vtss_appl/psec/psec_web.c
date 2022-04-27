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
#ifdef VTSS_SW_OPTION_PSEC_LIMIT
#include "psec_limit_api.h" /* For psec_limit_mgmt_XXX_cfg_get()               */
#endif
#include "psec_api.h"         /* Interface to the module that this file supports */
#include "psec.h"             /* For semi-public PSEC functions                  */
#include "msg_api.h"          /* For msg_abstime_get()                           */
#include "misc_api.h"         /* For iport2uport()                               */
#ifdef VTSS_SW_OPTION_PRIV_LVL
#include "vtss_privilege_api.h"
#include "vtss_privilege_web_api.h"
#endif

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_PSEC

#define PSEC_WEB_BUF_LEN 512

/* =================
 * Trace definitions
 * -------------- */
#include <vtss_module_id.h>
#include <vtss_trace_lvl_api.h>
#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_WEB
#include <vtss_trace_api.h>
/* ============== */

/*lint -sem(handler_stat_psec_status_port, thread_protected) */

/******************************************************************************/
// psec_web_switch_status()
/******************************************************************************/
static vtss_rc psec_web_switch_status(vtss_isid_t isid, CYG_HTTPD_STATE *p)
{
    vtss_rc                 rc = VTSS_RC_OK;
    psec_users_t            user;
    int                     cnt;
    i8                      delim;
    psec_port_state_t       port_state;
    port_iter_t             pit;
#ifdef VTSS_SW_OPTION_PSEC_LIMIT
    psec_limit_switch_cfg_t switch_cfg;
#endif

    (void)cyg_httpd_start_chunked("html");

    // Format [Sys]#[Users]#[PortStatus1]#[PortStatus2]#...#[PortStatusN]

    // [Sys]: psec_limit_supported
    cnt = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d#",
#ifdef VTSS_SW_OPTION_PSEC_LIMIT
                   1
#else
                   0
#endif
                  );

    // [Users]: user_name_1/user_abbr_1/user_name_2/user_abbr_2/.../user_name_N/user_abbr_N
    for (user = (psec_users_t)0; user < PSEC_USER_CNT; user++) {
        if (user == (psec_users_t)((int)PSEC_USER_CNT - 1)) {
            delim = '#';
        } else {
            delim = '/';
        }
        cnt += snprintf(p->outbuffer + cnt, sizeof(p->outbuffer) - cnt, "%s/%c%c", psec_user_name(user), psec_user_abbr(user), delim);
    }

    (void)cyg_httpd_write_chunked(p->outbuffer, cnt);

#ifdef VTSS_SW_OPTION_PSEC_LIMIT
    // As a service, also provide the current limit, if PSEC LIMIT is enabled.
    if ((rc = psec_limit_mgmt_switch_cfg_get(isid, &switch_cfg)) != VTSS_RC_OK) {
        goto do_exit;
    }
#endif

    (void)port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_UPORT, PORT_ITER_FLAGS_NORMAL);
    while (port_iter_getnext(&pit)) {
        u32 limit = 0, state = 0;

        if (!pit.first) {
            (void)cyg_httpd_write_chunked("#", 1);
        }

        // The FALSE argument tells the psec_mgmt_port_state_get() function
        // that we haven't obtained the critical section, so that the function
        // should do it itself.
        if ((rc = psec_mgmt_port_state_get(isid, pit.iport, &port_state, FALSE)) != VTSS_RC_OK) {
            goto do_exit;
        }

#ifdef VTSS_SW_OPTION_PSEC_LIMIT
        if (PSEC_USER_ENA_GET(&port_state, PSEC_USER_PSEC_LIMIT)) {
            // The Limit Control is enabled on this port. Get the limit.
            limit = switch_cfg.port_cfg[pit.iport - VTSS_PORT_NO_START].limit;
        }
        if (port_state.ena_mask != 0) {
            if (port_state.flags & PSEC_PORT_STATE_FLAGS_SHUT_DOWN) {
                state = 3; // Shutdown
            } else if (port_state.flags & PSEC_PORT_STATE_FLAGS_LIMIT_REACHED) {
                state = 2; // Limit reached
            } else {
                state = 1; // Ready
            }
        }
#endif

        // [PortStatus]: uport/ena_user_mask/state/cur_mac_cnt/limit.
        cnt = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d/%u/%u/%u/%u", pit.uport, port_state.ena_mask, state, port_state.mac_cnt, limit);
        (void)cyg_httpd_write_chunked(p->outbuffer, cnt);
    }

do_exit:
    cyg_httpd_end_chunked();
    return rc;
}

/******************************************************************************/
// psec_web_port_status()
/******************************************************************************/
static vtss_rc psec_web_port_status(vtss_isid_t isid, vtss_port_no_t iport, CYG_HTTPD_STATE *p)
{
    vtss_rc                 rc;
    int                     cnt;
    psec_port_state_t       port_state;
    psec_mac_state_t        *mac_state;

    (void)cyg_httpd_start_chunked("html");

    // Format: port#[MACs]
    // [MACs]: [MAC_1]#[MAC_2]#...#[MAC_N]
    // [MAC_x]: mac_address_x/vid_x/state_x/added_time_x/age_hold_time_left_x

    // The TRUE in the call to psec_mgmt_port_state_get() means that we have already
    // taken the critical section, so that we can iterate over this port.
    if ((rc = psec_mgmt_port_state_get(isid, iport, &port_state, TRUE)) != VTSS_RC_OK) {
        goto do_exit;
    }

    cnt = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d", iport2uport(iport));
    if (port_state.macs) {
        cnt += snprintf(p->outbuffer + cnt, sizeof(p->outbuffer) - cnt, "#");
    }
    (void)cyg_httpd_write_chunked(p->outbuffer, cnt);

    mac_state = port_state.macs;
    while (mac_state) {
        if (mac_state->flags & PSEC_MAC_STATE_FLAGS_IN_MAC_MODULE) {
            cnt = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s/%u/%d/%s/%u",
                           misc_mac2str(mac_state->vid_mac.mac.addr),
                           mac_state->vid_mac.vid,
                           (mac_state->flags & PSEC_MAC_STATE_FLAGS_BLOCKED) ? 0 : 1,
                           misc_time2str(msg_abstime_get(VTSS_ISID_LOCAL, mac_state->creation_time_secs)),
                           mac_state->age_or_hold_time_secs
                          );
        }
        (void)cyg_httpd_write_chunked(p->outbuffer, cnt);

        mac_state = mac_state->next;
        if (mac_state) {
            (void)cyg_httpd_write_chunked("#", 1);
        }
    }

    // Must free the MAC address "array".
    if (port_state.macs) {
        VTSS_FREE(port_state.macs);
    }

do_exit:
    cyg_httpd_end_chunked();
    return rc;
}

/****************************************************************************/
// handler_stat_psec_status_switch()
/****************************************************************************/
static cyg_int32 handler_stat_psec_status_switch(CYG_HTTPD_STATE *p)
{
    vtss_rc     rc;
    vtss_isid_t isid = web_retrieve_request_sid(p);
    if (redirectUnmanagedOrInvalid(p, isid)) {
        /* Redirect unmanaged/invalid access to handler */
        return -1;
    }

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_STAT_TYPE, VTSS_MODULE_ID_SECURITY)) {
        return -1;
    }
#endif

    // Since the psec_web_switch_status() uses semi-public functions to get the
    // current status (and I don't want to make them fully public), I've
    // moved all of it to psec_web.c
    if ((rc = psec_web_switch_status(isid, p)) != VTSS_RC_OK) {
        T_D("%s", error_txt(rc));
    }

    return -1; // Do not further search the file system.
}

/****************************************************************************/
// handler_stat_psec_status_port()
/****************************************************************************/
static cyg_int32 handler_stat_psec_status_port(CYG_HTTPD_STATE *p)
{
    vtss_isid_t    isid = web_retrieve_request_sid(p);
    vtss_port_no_t iport;
    vtss_rc        rc;

    if (redirectUnmanagedOrInvalid(p, isid)) {
        /* Redirect unmanaged/invalid access to handler */
        return -1;
    }

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_STAT_TYPE, VTSS_MODULE_ID_SECURITY)) {
        return -1;
    }
#endif

    iport = uport2iport(atoi(var_uport));
    if (iport >= port_isid_port_count(isid) + VTSS_PORT_NO_START || port_isid_port_no_is_stack(isid, iport)) {
        return -1;
    }

    // Since the psec_web_switch_status() uses semi-public functions to get the
    // current status (and I don't want to make them fully public), I've
    // moved all of it to psec_web.c.
    if ((rc = psec_web_port_status(isid, iport, p)) != VTSS_RC_OK) {
        T_D("%s", error_txt(rc));
    }

    return -1; // Do not further search the file system.
}

/****************************************************************************/
/*  Module Filter CSS routine                                               */
/****************************************************************************/
static size_t psec_lib_filter_css(char **base_ptr, char **cur_ptr, size_t *length)
{
    char buff[PSEC_WEB_BUF_LEN];
    (void)snprintf(buff, PSEC_WEB_BUF_LEN,
#ifdef VTSS_SW_OPTION_PSEC_LIMIT
                   ".PSEC_LIMIT_DIS { display: none; }\r\n"
#else
                   ".PSEC_LIMIT_ENA { display: none; }\r\n"
#endif /* VTSS_SW_OPTION_PSEC_LIMIT */
                  );
    return webCommonBufferHandler(base_ptr, cur_ptr, length, buff);
}

/****************************************************************************/
/*  Filter CSS table entry                                                  */
/****************************************************************************/
web_lib_filter_css_tab_entry(psec_lib_filter_css);

/****************************************************************************/
/*  HTTPD Table Entries                                                     */
/****************************************************************************/
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_stat_psec_status_switch, "/stat/psec_status_switch", handler_stat_psec_status_switch);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_stat_psec_status_port,   "/stat/psec_status_port",   handler_stat_psec_status_port);

/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
