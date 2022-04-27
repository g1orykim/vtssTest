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
#include "dot1x_api.h"
#include "msg_api.h"
#include "port_api.h" /* For port_iter_t     */
#include "vlan_api.h" /* For VLAN_ID_MIN/MAX */

#ifdef VTSS_SW_OPTION_PRIV_LVL
#include "vtss_privilege_api.h"
#include "vtss_privilege_web_api.h"
#endif

#define DOT1X_WEB_BUF_LEN 512

/* =================
 * Trace definitions
 * -------------- */
#include <vtss_module_id.h>
#include <vtss_trace_lvl_api.h>
#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_WEB
#include <vtss_trace_api.h>
/* ============== */

/******************************************************************************/
// DOT1X_web_compose_supported_protocols()
/******************************************************************************/
static u32 DOT1X_web_compose_supported_protocols(void)
{
    u32 result  = 0;
#ifdef VTSS_SW_OPTION_DOT1X
    result |= 1; // Port-based 802.1X - currently always
#endif
#ifdef VTSS_SW_OPTION_NAS_MAC_BASED
    result |= 2; // MAC-based Authentication
#endif
#ifdef VTSS_SW_OPTION_NAS_DOT1X_SINGLE
    result |= 4; // Single 802.1X
#endif
#ifdef VTSS_SW_OPTION_NAS_DOT1X_MULTI
    result |= 8; // Multi 802.1X
#endif
    return result;
}

/******************************************************************************/
// DOT1X_web_compose_supported_options()
/******************************************************************************/
static u32 DOT1X_web_compose_supported_options(void)
{
    u32 result  = 0;
#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS
    result |= 1; // RADIUS-assigned QoS
#endif
#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_VLAN
    result |= 2; // RADIUS-assigned VLAN
#endif
#ifdef VTSS_SW_OPTION_NAS_GUEST_VLAN
    result |= 4; // Guest VLAN
#endif
    return result;
}

/******************************************************************************/
// DOT1X_web_handler_nas_reset()
/******************************************************************************/
static cyg_int32 DOT1X_web_handler_nas_reset(CYG_HTTPD_STATE *p)
{
    vtss_isid_t     isid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    int             uport; /* Must be an int */
    vtss_port_no_t  iport;
    int             errors = 0, now = FALSE;
    const char      *var_string;
    size_t          len;

    if (redirectUnmanagedOrInvalid(p, isid)) { /* Redirect unmanaged/invalid access to handler */
        return -1;
    }

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_SECURITY)) {
        return -1;
    }
#endif

    if (p->method == CYG_HTTPD_METHOD_GET) {
        (void)cyg_httpd_start_chunked("html");
        // The Re-authenticate [now] option is implemented as a CYG_HTTPD_METHOD_GET method.
        // The URL-encoded properties are: port=<uport>&bool=<true|false>.

        // port=%d
        iport = VTSS_PORT_NO_NONE; // Something illegal
        if (cyg_httpd_form_varable_int(p, "port", &uport)) {
            iport = uport2iport(uport);
        }

        // In case someone changes VTSS_PORT_NO_START to a value > 0, we need to have the code
        // look like it does, which will cause a Lint warning when VTSS_PORT_NO_START == 0
        /*lint -e{685,568} */
        if (iport < VTSS_PORT_NO_START || iport >= VTSS_PORT_NO_START + VTSS_PORTS) {
            errors++;
        }

        // bool="true" || "false". Corresponds to [now] in CLI.
        if ((var_string = cyg_httpd_form_varable_string(p, "bool", &len)) != NULL && len >= 4) {
            if (strncasecmp(var_string, "true", 4) == 0) {
                now = TRUE;
            } else if (strncasecmp(var_string, "false", 5) == 0) {
                now = FALSE;
            } else {
                errors++;
            }
        } else {
            errors++;
        }

        // Do the reauthentication.
        if (!errors && dot1x_mgmt_reauth(isid, iport, now) != VTSS_RC_OK) {
            errors++;
        }

        if (errors) {
            T_W("Error in URL");
        }

        (void)cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}

/******************************************************************************/
// DOT1X_web_handler_nas_config()
/******************************************************************************/
// Avoid Lint warning: Variable 'switch_status' (line 150) may not have been initialized)
// Lint can't see it, but it really *is* initialized.
/*lint -e{644} */
static cyg_int32 DOT1X_web_handler_nas_config(CYG_HTTPD_STATE *p)
{
    vtss_isid_t           isid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    dot1x_glbl_cfg_t      glbl_cfg;
    dot1x_switch_cfg_t    switch_cfg;
    dot1x_switch_status_t switch_status;
    vtss_rc               rc;
    int                   cnt, an_integer, errors = 0;
    const char            *err_buf_ptr;
    port_iter_t           pit;

    if (redirectUnmanagedOrInvalid(p, isid)) { /* Redirect unmanaged/invalid access to handler */
        return -1;
    }

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_SECURITY)) {
        return -1;
    }
#endif

    if ((rc = dot1x_mgmt_glbl_cfg_get(&glbl_cfg))     != VTSS_RC_OK ||
        (rc = dot1x_mgmt_switch_cfg_get(isid, &switch_cfg)) != VTSS_RC_OK) {
        errors++;
    }

    if (p->method == CYG_HTTPD_METHOD_POST) {
        if (!errors) {
            // enabled: select
            if (cyg_httpd_form_varable_int(p, "nas_enabled", &an_integer) && (an_integer == FALSE || an_integer == TRUE)) {
                glbl_cfg.enabled = an_integer;
            } else {
                errors++;
            }

            // reauth_enable: Checkbox
            glbl_cfg.reauth_enabled = cyg_httpd_form_varable_find(p, "reauth_enable") ? TRUE : FALSE; // Returns non-NULL if checked

            // reauth_period: Integer. Only overwrite if enabled.
            if (glbl_cfg.reauth_enabled) {
                if (cyg_httpd_form_varable_int(p, "reauth_period", &an_integer)) {
                    glbl_cfg.reauth_period_secs = an_integer;
                } else {
                    errors++;
                }
            }

            // eapol_timeout: Integer
            if (cyg_httpd_form_varable_int(p, "eapol_timeout", &an_integer)) {
                glbl_cfg.eapol_timeout_secs = an_integer;
            } else {
                errors++;
            }

#ifdef NAS_USES_PSEC
            if (cyg_httpd_form_varable_int(p, "age_period", &an_integer)) {
                glbl_cfg.psec_aging_enabled = an_integer != 0;
                if (an_integer) {
                    // Only overwrite if enabled. No support for 0 on the Web-page
                    glbl_cfg.psec_aging_period_secs = an_integer;
                }
            } else {
                errors++;
            }

            if (cyg_httpd_form_varable_int(p, "hold_time", &an_integer)) {
                glbl_cfg.psec_hold_enabled = an_integer != 0;
                if (an_integer) {
                    // Only overwrite if enabled. No support for 0 on the Web-page
                    glbl_cfg.psec_hold_time_secs = an_integer;
                }
            } else {
                errors++;
            }
#endif

#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS
            // RADIUS-assigned QoS: Checkbox
            glbl_cfg.qos_backend_assignment_enabled = cyg_httpd_form_varable_find(p, "_backend_qos_glbl") ? TRUE : FALSE; // Returns non-NULL if checked
#endif

#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_VLAN
            // RADIUS-assigned VLAN: Checkbox
            glbl_cfg.vlan_backend_assignment_enabled = cyg_httpd_form_varable_find(p, "_backend_vlan_glbl") ? TRUE : FALSE; // Returns non-NULL if checked
#endif

#ifdef VTSS_SW_OPTION_NAS_GUEST_VLAN
            // Guest VLAN: Checkbox
            glbl_cfg.guest_vlan_enabled = cyg_httpd_form_varable_find(p, "_guest_vlan_glbl") ? TRUE : FALSE; // Returns non-NULL if checked

            if (glbl_cfg.guest_vlan_enabled) {

                // Guest VID: Integer
                if (cyg_httpd_form_varable_int(p, "guest_vid", &an_integer)) {
                    glbl_cfg.guest_vid = an_integer;
                } else {
                    errors++;
                }

                // Max. Reauth Count: Integer
                if (cyg_httpd_form_varable_int(p, "reauth_max", &an_integer)) {
                    glbl_cfg.reauth_max = an_integer;
                } else {
                    errors++;
                }

                // Allow entering Guest VLAN if EAPOL frame seen: Checkbox
                glbl_cfg.guest_vlan_allow_eapols = cyg_httpd_form_varable_find(p, "allow_eapol_frm") ? TRUE : FALSE; // Returns non-NULL if checked
            }
#endif

            if (!errors) {
                if ((rc = dot1x_mgmt_glbl_cfg_set(&glbl_cfg)) != VTSS_RC_OK) {
                    T_D("dot1x_mgmt_glbl_cfg_set() failed");
                    errors++;
                }
            }

            // Switch config.
            if ((rc = port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL)) != VTSS_RC_OK) {
                errors++;
            }
            while (!errors && port_iter_getnext(&pit)) {
                char var_name[16];

                // admin_%d: Int
                sprintf(var_name, "admin_%u", pit.uport);
                if (cyg_httpd_form_varable_int(p, var_name, &an_integer)) {
                    switch_cfg.port_cfg[pit.iport - VTSS_PORT_NO_START].admin_state = an_integer;
                } else {
                    errors++;
                }

#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS
                // RADIUS-assigned QoS: Checkbox
                sprintf(var_name, "backend_qos_%u", pit.uport);
                switch_cfg.port_cfg[pit.iport - VTSS_PORT_NO_START].qos_backend_assignment_enabled = cyg_httpd_form_varable_find(p, var_name) ? TRUE : FALSE; // Returns non-NULL if checked
#endif

#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_VLAN
                // RADIUS-assigned VLAN: Checkbox
                sprintf(var_name, "backend_vlan_%u", pit.uport);
                switch_cfg.port_cfg[pit.iport - VTSS_PORT_NO_START].vlan_backend_assignment_enabled = cyg_httpd_form_varable_find(p, var_name) ? TRUE : FALSE; // Returns non-NULL if checked
#endif

#ifdef VTSS_SW_OPTION_NAS_GUEST_VLAN
                // Guest VLAN: Checkbox
                sprintf(var_name, "guest_vlan_%u", pit.uport);
                switch_cfg.port_cfg[pit.iport - VTSS_PORT_NO_START].guest_vlan_enabled = cyg_httpd_form_varable_find(p, var_name) ? TRUE : FALSE; // Returns non-NULL if checked
#endif
            }

            if (!errors) {
                if ((rc = dot1x_mgmt_switch_cfg_set(isid, &switch_cfg)) != VTSS_RC_OK) {
                    T_D("dot1x_mgmt_switch_cfg_set() failed");
                    errors++;
                }
            }

            if (errors) {
                // There are two types of errors: Those where a form variable was invalid,
                // and those where a dot1x_mgmt_XXX() function failed.
                // In the first case, we redirect to the STACK_ERR_URL page, and in the
                // second, we redirect to a custom error page.
                if (rc == VTSS_RC_OK) {
                    redirect(p, STACK_ERR_URL);
                } else {
                    err_buf_ptr = error_txt(rc);
                    send_custom_error(p, "NAS Error", err_buf_ptr, strlen(err_buf_ptr));
                }
            } else {
                // No errors. Update with the current settings.
                redirect(p, "/nas.htm");
            }
        }
    } else { /* CYG_HTTPD_METHOD_GET (+HEAD) */
        (void)cyg_httpd_start_chunked("html");

        if (dot1x_mgmt_switch_status_get(isid, &switch_status) != VTSS_RC_OK) {
            errors++;
        }

        if (!errors) {
            // Protocols/Options/Enabled/ReauthEna/ReauthPeriod/EAPTimeout/AgePeriod/HoldTime/BackendQosGloballyEnabled/BackendVLANGloballyEnabled/GuestVLANGloballyEnabled/GuestVID/ReauthMax/AllowEapolFrms#[PortConfigs]
            // [PortConfigs] = [PortConfig1]#[PortConfig2]#...#[PortConfigN]
            // [PortConfig]  = PortNumber/AdminState/PortState/BackendQoS/BackendVLAN/GuestVLAN
            cnt = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/%u/%d/%d/%d/%d/%u/%u/%d/%d/%d/%d/%u/%d#",
                           DOT1X_web_compose_supported_protocols(),
                           DOT1X_web_compose_supported_options(),
                           glbl_cfg.enabled,
                           glbl_cfg.reauth_enabled,
                           glbl_cfg.reauth_period_secs,
                           glbl_cfg.eapol_timeout_secs,
#ifdef NAS_USES_PSEC
                           glbl_cfg.psec_aging_enabled ? glbl_cfg.psec_aging_period_secs : 0,
                           glbl_cfg.psec_hold_enabled  ? glbl_cfg.psec_hold_time_secs    : 0,
#else
                           0,
                           0,
#endif
#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS
                           glbl_cfg.qos_backend_assignment_enabled,
#else
                           0,
#endif
#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_VLAN
                           glbl_cfg.vlan_backend_assignment_enabled,
#else
                           0,
#endif
#ifdef VTSS_SW_OPTION_NAS_GUEST_VLAN
                           glbl_cfg.guest_vlan_enabled,
                           glbl_cfg.guest_vid,
                           glbl_cfg.reauth_max,
                           glbl_cfg.guest_vlan_allow_eapols
#else
                           0,
                           0,
                           0,
                           0
#endif
                          );
            (void)cyg_httpd_write_chunked(p->outbuffer, cnt);

            (void)port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
            while (port_iter_getnext(&pit)) {

                // [PortConfigs] = [PortConfig1]#[PortConfig2]#...#[PortConfigN]
                // [PortConfig]  = PortNumber/AdminState/PortState/BackendQoS/BackendVLAN/GuestVLAN
                cnt = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d/%d/%d/%d/%d/%d",
                               pit.uport,
                               switch_cfg.port_cfg[pit.iport - VTSS_PORT_NO_START].admin_state,
                               switch_status.status[pit.iport - VTSS_PORT_NO_START],
#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS
                               switch_cfg.port_cfg[pit.iport - VTSS_PORT_NO_START].qos_backend_assignment_enabled,
#else
                               0,
#endif
#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_VLAN
                               switch_cfg.port_cfg[pit.iport - VTSS_PORT_NO_START].vlan_backend_assignment_enabled,
#else
                               0,
#endif
#ifdef VTSS_SW_OPTION_NAS_GUEST_VLAN
                               switch_cfg.port_cfg[pit.iport - VTSS_PORT_NO_START].guest_vlan_enabled
#else
                               0
#endif
                              );
                (void)cyg_httpd_write_chunked(p->outbuffer, cnt);
                if (!pit.last) {
                    (void)cyg_httpd_write_chunked("#", 1);
                }
            }
        }
        (void)cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}

/******************************************************************************/
// DOT1X_web_handler_nas_switch_status()
/******************************************************************************/
static cyg_int32 DOT1X_web_handler_nas_switch_status(CYG_HTTPD_STATE *p)
{
    vtss_isid_t           isid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    dot1x_switch_status_t switch_status;
    nas_client_info_t     last_supplicant_info;
    char                  encoded_supplicant_identity[3 * NAS_SUPPLICANT_ID_MAX_LENGTH];
    int                   cnt;
    port_iter_t           pit;

    if (redirectUnmanagedOrInvalid(p, isid)) { /* Redirect unmanaged/invalid access to handler */
        return -1;
    }

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_STAT_TYPE, VTSS_MODULE_ID_SECURITY)) {
        return -1;
    }
#endif

    (void)cyg_httpd_start_chunked("html");

    if (dot1x_mgmt_switch_status_get(isid, &switch_status) == VTSS_RC_OK) {
        // Format: Protocols/Options#[PortStates]
        // [PortStates] = [PortState1]#[PortState2]#...#[PortStateN]
        // [PortState]  = PortNumber/AdminState/PortState/LastSource/LastIdentity/QosClass/VLAN
        cnt = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/%u#", DOT1X_web_compose_supported_protocols(), DOT1X_web_compose_supported_options());
        (void)cyg_httpd_write_chunked(p->outbuffer, cnt);
        (void)port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
        while (port_iter_getnext(&pit)) {
            if (dot1x_mgmt_port_last_supplicant_info_get(isid, pit.iport, &last_supplicant_info) == VTSS_RC_OK) {
                char qos_str[20], vlan_str[30];
                qos_str[0]  = '\0';
                vlan_str[0] = '\0';
#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS
                dot1x_qos_class_to_str(switch_status.qos_class[pit.iport - VTSS_PORT_NO_START], qos_str);
#endif
#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_VLAN
                if (switch_status.vlan_type[pit.iport - VTSS_PORT_NO_START] == NAS_VLAN_TYPE_BACKEND_ASSIGNED) {
                    sprintf(vlan_str, "%d (RADIUS-assigned)", switch_status.vid[pit.iport - VTSS_PORT_NO_START]);
                }
#endif
#ifdef VTSS_SW_OPTION_NAS_GUEST_VLAN
                if (switch_status.vlan_type[pit.iport - VTSS_PORT_NO_START] == NAS_VLAN_TYPE_GUEST_VLAN) {
                    sprintf(vlan_str, "%d (Guest)", switch_status.vid[pit.iport - VTSS_PORT_NO_START]);
                }
#endif
                (void)cgi_escape(last_supplicant_info.identity, encoded_supplicant_identity);
                // Format: PortNumber/AdminState_1/PortState_1/LastSource_1/LastIdentity_1/QosClass_1/VLAN
                cnt = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d/%d/%d/%s/%s/%s/%s",
                               pit.uport,
                               switch_status.admin_state[pit.iport - VTSS_PORT_NO_START],
                               switch_status.status[pit.iport - VTSS_PORT_NO_START],
                               last_supplicant_info.mac_addr_str,
                               encoded_supplicant_identity,
                               qos_str,
                               vlan_str);
                (void)cyg_httpd_write_chunked(p->outbuffer, cnt);
                if (!pit.last) {
                    (void)cyg_httpd_write_chunked("#", 1);
                }
            } else {
                break;
            }
        }
    }

    (void)cyg_httpd_end_chunked();

    return -1; // Do not further search the file system.
}

/******************************************************************************/
// DOT1X_web_handler_nas_port_status()
/******************************************************************************/
// Avoid Lint warning: Variable 'statistics' (line 480) may not have been initialized)
// Lint can't see it, but it really *is* initialized.
/*lint -e{644} */
static cyg_int32 DOT1X_web_handler_nas_port_status(CYG_HTTPD_STATE *p)
{
    vtss_isid_t        isid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    int                uport, clear;
    vtss_port_no_t     iport;
    dot1x_statistics_t statistics;
    char               encoded_identity[3 * NAS_SUPPLICANT_ID_MAX_LENGTH];
    int                cnt, errors = 0, i;
    vtss_vid_mac_t     vid_mac;

#ifdef NAS_MULTI_CLIENT
    const char         *var_string;
    size_t             len;
    int                an_integer;
    uint               mac_addr_uint[6];
#endif

    if (redirectUnmanagedOrInvalid(p, isid)) { /* Redirect unmanaged/invalid access to handler */
        return -1;
    }

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_STAT_TYPE, VTSS_MODULE_ID_SECURITY)) {
        return -1;
    }
#endif

    (void)cyg_httpd_start_chunked("html");
    memset(&vid_mac, 0, sizeof(vid_mac));

    // port=%d
    iport = VTSS_PORT_NO_NONE; // Something illegal
    if (cyg_httpd_form_varable_int(p, "port", &uport)) {
        iport = uport2iport(uport);
    }

    // In case someone changes VTSS_PORT_NO_START to a value > 0, we need to have the code
    // look like it does, which will cause a Lint warning when VTSS_PORT_NO_START == 0
    /*lint -e{685, 568} */
    if (iport < VTSS_PORT_NO_START || iport >= VTSS_PORT_NO_START + VTSS_PORTS) {
        errors++;
    }

#ifdef NAS_MULTI_CLIENT
    // mac=%s
    if ((var_string = cyg_httpd_form_varable_string(p, "mac", &len)) != NULL && len >= 17) {
        if (sscanf(var_string, "%2x-%2x-%2x-%2x-%2x-%2x", &mac_addr_uint[0], &mac_addr_uint[1], &mac_addr_uint[2], &mac_addr_uint[3], &mac_addr_uint[4], &mac_addr_uint[5]) == 6) {
            // The sscanf() cannot scan hex chars.
            for (i = 0; i < 6; i++) {
                vid_mac.mac.addr[i] = (uchar)mac_addr_uint[i];
            }
        }
    }

    // vid=%s
    if (cyg_httpd_form_varable_int(p, "vid", &an_integer) && an_integer >= VLAN_ID_MIN && an_integer <= VLAN_ID_MAX) {
        vid_mac.vid = an_integer;
    }
#endif

    // This function is also used to clear the counters, when the URL contains the string "clear=1"
    if (!errors && cyg_httpd_form_varable_int(p, "clear", &clear) && clear == 1) {
        // If @vid_mac.vid == 0 (as set by the memset() above), by convention, all counters on the port are cleared.
        if (dot1x_mgmt_statistics_clear(isid, iport, &vid_mac) != VTSS_RC_OK) {
            if (vid_mac.vid == 0) {
                errors++; // Don't count errors on specific <MAC, VID> (cases where vid_mac.vid != 0), since the <MAC, VID> may have been removed in the meanwhile
            }
        }
    }

    // Whether counters are cleared or not, return statistics.
    // The statistics is a pretty complicated thing, that to a high
    // degree depends on the port mode (admin state).
    // Format:
    //   [port_cfg]#[summed]#[specific]#[attached_clients]
    // Where
    //   [port_cfg]         = port_number/protocols/options/admin_state/qos_class/vlan_type/vid (always non-empty)
    //   [client]           = identity/mac_addr/vid/state/last_auth_time
    //   [eapol_counters]   = eapol_cnt1/eapol_cnt2/.../eapol_cnt22                             (non-empty for BPDU-based protocols)
    //   [backend_counters] = backend_cnt1/backend_cnt2/.../backend_cnt5                        (non-empty for backend-based protocols)
    //   [client_data]      = [client]|[eapol_counters]|[backend_counters]
    //   [summed]           = [client_data]                                                     (always non-empty)
    //   [specific]         = [client_data]                                                     (may be non-empty for multi-clients)
    //   [attached_clients] = [client1]|[client2]|...|[clientN]                                 (may be non-empty for multi-clients)
    // admin_state is never NAS_PORT_CONTROL_DISABLED. It should always be what the user has configured,
    // whether NAS is globally disabled or not. The summed.client.state indicates the actual port state, which
    // may take one of the following values:
    // 0 = Link down, 1 = Authorized, 2 = Unauthorized, 3 = NAS globally disabled, > 3 = multi-client auth/unauth count.

    if (!errors) {
        // The first iteration is about the port counters, and the second is about the possibly selected <VID, MAC>
        BOOL skip;
        for (i = 0; i < 2; i++) {
            vtss_vid_mac_t *used_vid_mac;

            if (i == 0) {
                used_vid_mac = NULL; // Asking for port statistics
                skip = FALSE;
            } else if (vid_mac.vid == 0) {
                // Doesn't want specific MAC.
                used_vid_mac = NULL;
                skip = TRUE;
            } else {
                used_vid_mac = &vid_mac; // Asking for specific statistics if vid_mac.vid != 0.
                skip = FALSE;
            }

            if (!skip) {
                if (!errors && dot1x_mgmt_statistics_get(isid, iport, used_vid_mac, &statistics) != VTSS_RC_OK) {
                    if (i == 0) {
                        // Don't count errors for specific <MAC, VID>, because it might be that
                        // it doesn't exist anymore
                        errors++;
                    }
                    skip = TRUE;
                }
            }

            if (i == 0) { // Only part of port-statistics (first iteration)
                // [port_cfg] = port_number/protocols/options/admin_state/qos_class/vlan_type/vid
                char qos_str[20];
                char vlan_str[30];
                qos_str[0]  = '\0';
                vlan_str[0] = '\0';
#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS
                dot1x_qos_class_to_str(statistics.qos_class, qos_str);
#endif
#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_VLAN
                if (statistics.vlan_type == NAS_VLAN_TYPE_BACKEND_ASSIGNED) {
                    sprintf(vlan_str, "%d (RADIUS-assigned)", statistics.vid);
                }
#endif
#ifdef VTSS_SW_OPTION_NAS_GUEST_VLAN
                if (statistics.vlan_type == NAS_VLAN_TYPE_GUEST_VLAN) {
                    sprintf(vlan_str, "%d (Guest)", statistics.vid);
                }
#endif
                cnt = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/%u/%u/%d/%s/%d/%s#",
                               iport2uport(iport),
                               DOT1X_web_compose_supported_protocols(),
                               DOT1X_web_compose_supported_options(),
                               skip ? NAS_PORT_CONTROL_FORCE_UNAUTHORIZED : statistics.admin_state, // If an error occurred, provide the web-page with some more or less useful info.
                               qos_str,
#ifdef NAS_USES_VLAN
                               statistics.vlan_type,
#else
                               0,
#endif
                               vlan_str);
                (void)cyg_httpd_write_chunked(p->outbuffer, cnt);
            }

            if (!skip) {
                // [summed_or_specific.client] = identity/mac_addr/vid/state/last_auth_time
                (void)cgi_escape(statistics.client_info.identity, encoded_identity);
                cnt = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s/%s/%d/%d/%s|",
                               encoded_identity,
                               statistics.client_info.mac_addr_str,
                               statistics.client_info.vid_mac.vid,
                               statistics.status,
                               statistics.client_info.rel_auth_time_secs == 0 ? "" : misc_time2str(msg_abstime_get(VTSS_ISID_LOCAL, statistics.client_info.rel_auth_time_secs)));
                (void)cyg_httpd_write_chunked(p->outbuffer, cnt);

                // [summed_or_specific.eapol_counters] = eapol_cnt1/eapol_cnt2/.../eapol_cnt22
                cnt = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u|",
                               statistics.eapol_counters.authEntersConnecting,
                               statistics.eapol_counters.authEapLogoffsWhileConnecting,
                               statistics.eapol_counters.authEntersAuthenticating,
                               statistics.eapol_counters.authAuthSuccessesWhileAuthenticating,
                               statistics.eapol_counters.authAuthTimeoutsWhileAuthenticating,
                               statistics.eapol_counters.authAuthFailWhileAuthenticating,
                               statistics.eapol_counters.authAuthEapStartsWhileAuthenticating,
                               statistics.eapol_counters.authAuthEapLogoffWhileAuthenticating,
                               statistics.eapol_counters.authAuthReauthsWhileAuthenticated,
                               statistics.eapol_counters.authAuthEapStartsWhileAuthenticated,
                               statistics.eapol_counters.authAuthEapLogoffWhileAuthenticated,
                               statistics.eapol_counters.dot1xAuthEapolFramesRx,
                               statistics.eapol_counters.dot1xAuthEapolFramesTx,
                               statistics.eapol_counters.dot1xAuthEapolStartFramesRx,
                               statistics.eapol_counters.dot1xAuthEapolLogoffFramesRx,
                               statistics.eapol_counters.dot1xAuthEapolRespIdFramesRx,
                               statistics.eapol_counters.dot1xAuthEapolRespFramesRx,
                               statistics.eapol_counters.dot1xAuthEapolReqIdFramesTx,
                               statistics.eapol_counters.dot1xAuthEapolReqFramesTx,
                               statistics.eapol_counters.dot1xAuthInvalidEapolFramesRx,
                               statistics.eapol_counters.dot1xAuthEapLengthErrorFramesRx,
                               statistics.eapol_counters.dot1xAuthLastEapolFrameVersion);
                (void)cyg_httpd_write_chunked(p->outbuffer, cnt);

                // [summed_or_specific.backend_counters] = backend_cnt1/backend_cnt2/.../backend_cnt5
                cnt = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/%u/%u/%u/%u#",
                               statistics.backend_counters.backendResponses,
                               statistics.backend_counters.backendAccessChallenges,
                               statistics.backend_counters.backendOtherRequestsToSupplicant,
                               statistics.backend_counters.backendAuthSuccesses,
                               statistics.backend_counters.backendAuthFails);
                (void)cyg_httpd_write_chunked(p->outbuffer, cnt);
            } else {
                // This is quite normal (at least when i == 1). Print the missing major separator without filling in data.
                (void)cyg_httpd_write_chunked("#", 1);
            }
        } // for (i = 0; i < 2; i++) */

        // And now for the attached clients. This only exists for multi-modes (mac-based & multi 802.1X).
#ifdef NAS_MULTI_CLIENT
        // [attached_clients] = [client1]|[client2]|...|[clientN]
        // where
        // [clientX] = identity/mac_addr/vid/state/last_auth_time
        //
        // Here @state can only be Authorized (1) or Unauthorized (2).
        //
        // Since we cannot get the client_cnt together with all the MAC addresses in one atomic
        // operation, the format doesn't include a total count.
        // Since we potentically can have more MAC addresses than what the
        // p->outbuffer can hold, we write_chunked() for every new MAC address we get.
        dot1x_multi_client_status_t client_status;
        BOOL client_found = TRUE, client_first = TRUE;
        int flush_when_this_limit_is_reached = 0; // Used to optimize the calls to cyg_httpd_write_chunked()
        cnt = 0;
        do {
            // This one updates client_found whether an error occurred or not.
            if (dot1x_mgmt_multi_client_status_get(isid, iport, &client_status, &client_found, client_first) == VTSS_RC_OK && client_found) {
                (void)cgi_escape(client_status.client_info.identity, encoded_identity);
                cnt += snprintf(p->outbuffer + cnt, sizeof(p->outbuffer) - cnt, "%s%s/%s/%d/%d/%s",
                                client_first ? "" : "|",
                                encoded_identity,
                                client_status.client_info.mac_addr_str,
                                client_status.client_info.vid_mac.vid,
                                client_status.status,
                                client_status.client_info.rel_auth_time_secs == 0 ? "" : misc_time2str(msg_abstime_get(VTSS_ISID_LOCAL, client_status.client_info.rel_auth_time_secs)));

                if (client_first) {
                    // To optimize calls to cyg_httpd_write_chunked(), we only flush when
                    // we've gathered so much data, that there's only room for four
                    // more clients (which is to be on the safe side). At this point,
                    // we know how much one client takes (which is cnt).
                    flush_when_this_limit_is_reached = sizeof(p->outbuffer) - 4 * cnt;
                }
                client_first = FALSE;
                if (cnt > flush_when_this_limit_is_reached) {
                    (void)cyg_httpd_write_chunked(p->outbuffer, cnt);
                    cnt = 0;
                }
            }
        } while (client_found);
        if (cnt) {
            // Write what is left.
            (void)cyg_httpd_write_chunked(p->outbuffer, cnt);
        }
#endif
    } /* if (!errors) */
    (void)cyg_httpd_end_chunked();

    return -1; // Do not further search the file system.
}

/****************************************************************************/
/*  Module Filter CSS routine                                               */
/****************************************************************************/

static size_t DOT1X_lib_filter_css(char **base_ptr, char **cur_ptr, size_t *length)
{
    char buff[DOT1X_WEB_BUF_LEN];
    (void) snprintf(buff, DOT1X_WEB_BUF_LEN,
#ifdef VTSS_SW_OPTION_NAS_DOT1X_SINGLE
                    ".NAS_SINGLE_DIS {display:none;}\r\n"
#endif
#ifdef NAS_DOT1X_SINGLE_OR_MULTI
                    ".NAS_NO_SINGLE_NO_MULTI {display:none;}\r\n"
#else
                    ".NAS_SINGLE_OR_MULTI {display:none;}\r\n"
#endif
#ifndef NAS_MULTI_CLIENT
                    ".NAS_MULTI_OR_MAC_BASED {display:none;}\r\n"
#endif

#ifndef NAS_USES_PSEC
                    ".NAS_USES_PSEC {display:none;}\r\n"
#endif
#ifndef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS
                    ".NAS_BACKEND_QOS {display:none;}\r\n"
#endif
#ifndef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS_CUSTOM
                    ".NAS_BACKEND_QOS_CUSTOM {display:none;}\r\n"
#endif
#ifndef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS_RFC4675
                    ".NAS_BACKEND_QOS_RFC4675 {display:none;}\r\n"
#endif

#ifndef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_VLAN
                    ".NAS_BACKEND_VLAN {display:none;}\r\n"
#endif
#ifndef VTSS_SW_OPTION_NAS_GUEST_VLAN
                    ".NAS_GUEST_VLAN {display:none;}\r\n"
#endif
#ifndef NAS_USES_VLAN
                    ".NAS_USES_VLAN {display:none;}\r\n"
#endif
                   );
    return webCommonBufferHandler(base_ptr, cur_ptr, length, buff);
}
/****************************************************************************/
/*  Filter CSS table entry                                                  */
/****************************************************************************/
web_lib_filter_css_tab_entry(DOT1X_lib_filter_css);

/****************************************************************************/
/*  HTTPD Table Entries                                                     */
/****************************************************************************/
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_nas_reset,       "/config/nas_reset",       DOT1X_web_handler_nas_reset);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_nas,             "/config/nas",             DOT1X_web_handler_nas_config);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_stat_nas_status_switch, "/stat/nas_status_switch", DOT1X_web_handler_nas_switch_status);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_stat_nas_status_port,   "/stat/nas_status_port",   DOT1X_web_handler_nas_port_status);

/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
