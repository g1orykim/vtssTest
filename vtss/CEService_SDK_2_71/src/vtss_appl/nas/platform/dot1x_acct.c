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

/*lint -esym(459, DOT1X_ACCT_radius_rx_callback) */
/*lint -esym(459, DOT1X_ACCT_reset_callback)     */

#include "vtss_nas_platform_api.h" /* For NAS base-lib types               */
#include "dot1x.h"                 /* For semi-public functions and macros */
#include "dot1x_api.h"             /* For dot1x_port_control_to_str()      */
#include "dot1x_trace.h"           /* For T_xG(TRACE_GRP_ACCT)             */
#include "msg_api.h"               /* For msg_switch_exists()              */

#include "control_api.h"           /* For control_system_reset_register()  */
#include "vtss_common_os.h"        /* For vtss_common_macaddr_t            */
#include <network.h>               /* For ntohl()                          */

/******************************************************************************/
// DOT1X Accounting status type
// Note: These values maps directly to the radius values
/******************************************************************************/
enum {
    DOT1X_ACCT_STATUS_TYPE_START = 1,
    DOT1X_ACCT_STATUS_TYPE_STOP,
    DOT1X_ACCT_STATUS_TYPE_INTERIM_UPDATE,
};

/******************************************************************************/
// DOT1X Accounting states
/******************************************************************************/
#define DOT1X_ACCT_STATE_INIT            0  // Initial state.
#define DOT1X_ACCT_STATE_START_PENDING   1  // We have sent an Accounting start frame, but not yet received an acknowledge from the RADIUS server.
#define DOT1X_ACCT_STATE_STARTED         2  // We have received an acknowledge frame and now we are allowed to send a stop frame or interim updates.
#define DOT1X_ACCT_STATE_LAST            3

/******************************************************************************/
// DOT1X Accounting events
/******************************************************************************/
#define DOT1X_ACCT_EVENT_UP              0  // Fired when session is authorized.
#define DOT1X_ACCT_EVENT_DOWN            1  // Fired when Session is unauthorized (or released e.g. due to session timeouts).
#define DOT1X_ACCT_EVENT_START_ACK_RCV   2  // Fired when we receive an ack from the RADIUS server.
#define DOT1X_ACCT_EVENT_INTERIM_EXPIRED 3  // Fired when the interim timer expires.
#define DOT1X_ACCT_EVENT_LAST            4

// Forward declarations
static void DOT1X_ACCT_state_machine(nas_sm_t *sm, u32 event);

/****************************************************************************/
// DOT1X_ACCT_radius_rx_callback()
/****************************************************************************/
static void DOT1X_ACCT_radius_rx_callback(u8 handle, void *ctx, vtss_radius_access_codes_e code, vtss_radius_rx_callback_result_e res)
{
    nas_sm_t       *sm = ctx;
    nas_eap_info_t *eap_info;
    acct_sm_t      *acct;

    if (sm == NULL) { // This is a response to a stop frame - nothing to do
        return;
    }

    if ((eap_info = nas_get_eap_info(sm)) == NULL) {
        return;
    }

    acct = &eap_info->acct;

    dot1x_crit_enter();

    if (acct->handle != handle) {
        T_E("Received unknown radius response (exp = %d, got %u)", acct->handle, handle);
        goto do_exit;
    }

    acct->handle = -1;

    if (!dot1x_glbl_enabled()) {
        goto do_exit;
    }

    if (res == VTSS_RADIUS_RX_CALLBACK_OK) {
        switch (code) {
        case VTSS_RADIUS_CODE_ACCOUNTING_RESPONSE: {
            if (acct->status_type == DOT1X_ACCT_STATUS_TYPE_START) {
                DOT1X_ACCT_state_machine(sm, DOT1X_ACCT_EVENT_START_ACK_RCV);
            }
            break;
        }
        default:
            T_EG(TRACE_GRP_ACCT, "Unknown RADIUS type received: %d", code);
            break;
        }
    }

do_exit:
    dot1x_crit_exit();
}

/******************************************************************************/
// DOT1X_ACCT_radius_tx_request()
/******************************************************************************/
static void DOT1X_ACCT_radius_tx_request(struct nas_sm *sm, u8 status_type)
{
    vtss_rc               res;
    u8                    handle;
    i8                    temp_str[20];
    vtss_common_macaddr_t sys_mac;
    nas_eap_info_t        *eap_info    = nas_get_eap_info(sm);
    nas_client_info_t     *client_info = nas_get_client_info(sm);
    nas_port_info_t       *port_info   = nas_get_port_info(sm);
    acct_sm_t             *acct;
    vtss_isid_t           isid;
    vtss_port_no_t        iport;

    if (eap_info == NULL) {
        T_EG(TRACE_GRP_ACCT, "No eap_info");
        return;
    }

    if (client_info == NULL) {
        T_EG(TRACE_GRP_ACCT, "No client_info");
        return;
    }

    if (port_info == NULL) {
        T_EG(TRACE_GRP_ACCT, "No port_info");
        return;
    }

    T_NG(TRACE_GRP_ACCT, "enter, mode = %s, port = %u", dot1x_port_control_to_str(port_info->port_control, FALSE), port_info->port_no);

    acct  = &eap_info->acct;
    isid  = DOT1X_NAS_PORT_2_ISID(port_info->port_no);
    iport = DOT1X_NAS_PORT_2_SWITCH_API_PORT(port_info->port_no);

    if (acct->handle != -1) { // There is a pending request
        (void)vtss_radius_free(acct->handle); // Cancel it
        acct->handle = -1;
    }

    // Allocate a RADIUS handle (i.e. a RADIUS ID).
    if ((res = vtss_radius_alloc(&handle, VTSS_RADIUS_CODE_ACCOUNTING_REQUEST)) != VTSS_RC_OK) {
        T_EG(TRACE_GRP_ACCT, "Got \"%s\" from vtss_radius_alloc()", error_txt(res));
        return;
    }

    // Add the required attributes
    // Acct-Status-Type
    temp_str[0] = temp_str[1] = temp_str[2] = 0;
    temp_str[3] = status_type; // NOTE: Direct mapping
    if ((res = vtss_radius_tlv_set(handle, VTSS_RADIUS_ATTRIBUTE_ACCT_STATUS_TYPE, 4, (u8 *)temp_str, TRUE)) != VTSS_RC_OK) {
        T_EG(TRACE_GRP_ACCT, "Got \"%s\" from vtss_radius_tlv_set() on Acct-Status_Type", error_txt(res));
        return;
    }

    // Acct-Session-Id
    (void)snprintf(temp_str, sizeof(temp_str) - 1, "%08X", acct->session_id);
    if ((res = vtss_radius_tlv_set(handle, VTSS_RADIUS_ATTRIBUTE_ACCT_SESSION_ID, strlen(temp_str), (u8 *)temp_str, TRUE)) != VTSS_RC_OK) {
        T_EG(TRACE_GRP_ACCT, "Got \"%s\" from vtss_radius_tlv_set() on Acct-Session_Id attribute", error_txt(res));
        return;
    }

    // Add the optional attributes.
#define DOT1X_ADD_OPTIONAL_RADIUS_ATTRIB(t, l, v)                                                             \
  res = vtss_radius_tlv_set(handle, t, l, v, FALSE);                                                          \
  if (res != VTSS_RC_OK && res != VTSS_RADIUS_ERROR_NOT_ROOM_FOR_TLV) {                                          \
    T_EG(TRACE_GRP_ACCT, "Got \"%s\" from vtss_tlv_set() on optional attribute", vtss_radius_error_txt(res)); \
    return;                                                                                                   \
  }

    // NAS-Port-Type
    temp_str[0] = temp_str[1] = temp_str[2] = 0;
    temp_str[3] = VTSS_RADIUS_NAS_PORT_TYPE_ETHERNET;
    DOT1X_ADD_OPTIONAL_RADIUS_ATTRIB(VTSS_RADIUS_ATTRIBUTE_NAS_PORT_TYPE, 4, (u8 *)temp_str);

    // User name (supplicant identity)
    DOT1X_ADD_OPTIONAL_RADIUS_ATTRIB(VTSS_RADIUS_ATTRIBUTE_USER_NAME, strlen(client_info->identity), (u8 *)client_info->identity);

    // NAS-Port-Id
    temp_str[0] = temp_str[1] = temp_str[2] = 0;
    temp_str[3] = port_info->port_no;
    DOT1X_ADD_OPTIONAL_RADIUS_ATTRIB(VTSS_RADIUS_ATTRIBUTE_NAS_PORT, 4, (u8 *)temp_str);

    (void)snprintf(temp_str, sizeof(temp_str) - 1, "Port %u", port_info->port_no);
    DOT1X_ADD_OPTIONAL_RADIUS_ATTRIB(VTSS_RADIUS_ATTRIBUTE_NAS_PORT_ID, strlen(temp_str), (u8 *)temp_str);

    // Calling-Station-Id
    DOT1X_ADD_OPTIONAL_RADIUS_ATTRIB(VTSS_RADIUS_ATTRIBUTE_CALLING_STATION_ID, strlen(client_info->mac_addr_str), (u8 *)client_info->mac_addr_str);

    // Called-Station-Id
    vtss_os_get_systemmac(&sys_mac);
    nas_os_mac2str(sys_mac.macaddr, temp_str);
    DOT1X_ADD_OPTIONAL_RADIUS_ATTRIB(VTSS_RADIUS_ATTRIBUTE_CALLED_STATION_ID, strlen(temp_str), (u8 *)temp_str);

    if ((status_type == DOT1X_ACCT_STATUS_TYPE_STOP) || (status_type == DOT1X_ACCT_STATUS_TYPE_INTERIM_UPDATE)) {
        // Acct-Session-Time
        u32 session_time = HOST2NETL((cyg_current_time() / CYGNUM_HAL_RTC_DENOMINATOR) - acct->session_time);
        DOT1X_ADD_OPTIONAL_RADIUS_ATTRIB(VTSS_RADIUS_ATTRIBUTE_ACCT_SESSION_TIME, 4, (u8 *)&session_time);

        if (acct->counters) {
            vtss_rc              rc;
            vtss_port_counters_t counters;
            if (VTSS_ISID_LEGAL(isid) && msg_switch_exists(isid)) {
                if ((rc = port_mgmt_counters_get(isid, iport, &counters)) == VTSS_RC_OK) {
                    u32 cnt;
                    // Acct-Input-Packets
                    cnt = HOST2NETL((counters.rmon.rx_etherStatsPkts - acct->input_packets) & 0xffffffff);
                    DOT1X_ADD_OPTIONAL_RADIUS_ATTRIB(VTSS_RADIUS_ATTRIBUTE_ACCT_INPUT_PACKETS, 4, (u8 *)&cnt);
                    // Acct-Output-Packets
                    cnt = HOST2NETL((counters.rmon.tx_etherStatsPkts - acct->output_packets) & 0xffffffff);
                    DOT1X_ADD_OPTIONAL_RADIUS_ATTRIB(VTSS_RADIUS_ATTRIBUTE_ACCT_OUTPUT_PACKETS, 4, (u8 *)&cnt);
                    // Acct-Input-Octets
                    cnt = HOST2NETL((counters.rmon.rx_etherStatsOctets - acct->input_octets) & 0xffffffff);
                    DOT1X_ADD_OPTIONAL_RADIUS_ATTRIB(VTSS_RADIUS_ATTRIBUTE_ACCT_INPUT_OCTETS, 4, (u8 *)&cnt);
                    // Acct-Output-Octets
                    cnt = HOST2NETL((counters.rmon.tx_etherStatsOctets - acct->output_octets) & 0xffffffff);
                    DOT1X_ADD_OPTIONAL_RADIUS_ATTRIB(VTSS_RADIUS_ATTRIBUTE_ACCT_OUTPUT_OCTETS, 4, (u8 *)&cnt);
                    // Acct-Input-Gigawords
                    cnt = HOST2NETL(((counters.rmon.rx_etherStatsOctets - acct->input_octets) >> 32) & 0xffffffff);
                    DOT1X_ADD_OPTIONAL_RADIUS_ATTRIB(VTSS_RADIUS_ATTRIBUTE_ACCT_INPUT_GIGAWORDS, 4, (u8 *)&cnt);
                    // Acct-Output-Gigawords
                    cnt = HOST2NETL(((counters.rmon.tx_etherStatsOctets - acct->output_octets) >> 32) & 0xffffffff);
                    DOT1X_ADD_OPTIONAL_RADIUS_ATTRIB(VTSS_RADIUS_ATTRIBUTE_ACCT_OUTPUT_GIGAWORDS, 4, (u8 *)&cnt);
                } else {
                    T_EG(TRACE_GRP_ACCT, "Unable to read counters (code = %d, str=%s)", rc, error_txt(rc));
                }
            }
        }
    }

    if (status_type == DOT1X_ACCT_STATUS_TYPE_STOP) {
        port_conf_t        conf;
        // Acct-Terminate-Cause
        temp_str[0] = temp_str[1] = temp_str[2] = 0;
        // Don't change the sequence of these tests unless you know what you are doing!
        if (eap_info->stop_reason == NAS_STOP_REASON_SWITCH_REBOOT) {
            temp_str[3] = VTSS_RADIUS_ACCT_TERMINATE_CAUSE_ADMIN_REBOOT;
        } else if ((port_mgmt_conf_get(isid, iport, &conf) == VTSS_RC_OK) && (conf.enable == FALSE)) {
            temp_str[3] = VTSS_RADIUS_ACCT_TERMINATE_CAUSE_ADMIN_RESET;
        } else if ((eap_info->stop_reason == NAS_STOP_REASON_AUTH_FAILURE) ||
                   (eap_info->stop_reason == NAS_STOP_REASON_AUTH_TIMEOUT) ||
                   (eap_info->stop_reason == NAS_STOP_REASON_REAUTH_COUNT_EXCEEDED)) {
            temp_str[3] = VTSS_RADIUS_ACCT_TERMINATE_CAUSE_USER_ERROR;
        } else if ((acct->releasing) ||
                   (eap_info->stop_reason == NAS_STOP_REASON_UNKNOWN) ||
                   (eap_info->stop_reason == NAS_STOP_REASON_INITIALIZING) ||
                   (eap_info->stop_reason == NAS_STOP_REASON_AUTH_NOT_CONFIGURED) ||
                   (eap_info->stop_reason == NAS_STOP_REASON_FORCED_UNAUTHORIZED) ||
                   (eap_info->stop_reason == NAS_STOP_REASON_MAC_TABLE_ERROR) ||
                   (eap_info->stop_reason == NAS_STOP_REASON_SWITCH_DOWN) ||
                   (eap_info->stop_reason == NAS_STOP_REASON_PORT_LINK_DOWN) ||
                   (eap_info->stop_reason == NAS_STOP_REASON_STATION_MOVED) ||
                   (eap_info->stop_reason == NAS_STOP_REASON_AGED_OUT) ||
                   (eap_info->stop_reason == NAS_STOP_REASON_HOLD_TIME_EXPIRED) ||
                   (eap_info->stop_reason == NAS_STOP_REASON_PORT_MODE_CHANGED) ||
                   (eap_info->stop_reason == NAS_STOP_REASON_PORT_SHUT_DOWN)) {
            temp_str[3] = VTSS_RADIUS_ACCT_TERMINATE_CAUSE_NAS_REQUEST;
        } else {
            temp_str[3] = VTSS_RADIUS_ACCT_TERMINATE_CAUSE_USER_REQUEST;
        }
        DOT1X_ADD_OPTIONAL_RADIUS_ATTRIB(VTSS_RADIUS_ATTRIBUTE_ACCT_TERMINATE_CAUSE, 4, (u8 *)temp_str);
        T_DG(TRACE_GRP_ACCT, "Sending stop frame, term cause: %d, port: %u, stop_reason: %d, releasing: %s", temp_str[3], port_info->port_no, eap_info->stop_reason, acct->releasing ? "TRUE" : "FALSE");
    }
#undef DOT1X_ADD_OPTIONAL_RADIUS_ATTRIB

    acct->status_type = status_type; // Store the status_type so we ran react on it when we receive the response message

    // Transmit the RADIUS frame and ask to be called back whenever a response arrives.
    // The RADIUS module takes care of retransmitting, changing server, etc.
    // The sm is not needed if it is a stop frame (and is sometimes not valid), so use a NULL pointer in this case
    if ((res = vtss_radius_tx(handle, (status_type != DOT1X_ACCT_STATUS_TYPE_STOP) ? sm : NULL, DOT1X_ACCT_radius_rx_callback)) != VTSS_RC_OK) {
        T_EG(TRACE_GRP_ACCT, "vtss_radius_tx() returned \"%s\"", vtss_radius_error_txt(res));
        return;
    }
    if (status_type != DOT1X_ACCT_STATUS_TYPE_STOP) {
        acct->handle = handle;
    }
}

/******************************************************************************/
// Initialize the accounting part of the shared state.
// If we reinitialize a session, we must let the authorized_enable flag survive
/******************************************************************************/
static void DOT1X_ACCT_sm_init(struct nas_sm *sm, BOOL keep_authorize_enabled)
{
    nas_eap_info_t  *eap_info         = nas_get_eap_info(sm);
    nas_port_info_t *port_info        = nas_get_port_info(sm);
    acct_sm_t       *acct;
    BOOL            authorize_enabled;

    if (eap_info == NULL) {
        T_EG(TRACE_GRP_ACCT, "No eap_info");
        return;
    }

    if (port_info == NULL) {
        T_EG(TRACE_GRP_ACCT, "No port_info");
        return;
    }

    acct              = &eap_info->acct;
    authorize_enabled = acct->authorize_enabled;

    T_NG(TRACE_GRP_ACCT, "enter, mode = %s, port = %u", dot1x_port_control_to_str(port_info->port_control, FALSE), port_info->port_no);
    memset(acct, 0, sizeof(acct_sm_t));
    acct->handle = -1;
    if (keep_authorize_enabled) {
        acct->authorize_enabled = authorize_enabled;
    }
}

/******************************************************************************/
// DOT1X_ACCT_state_machine()
// Implements the following state/event machine
/******************************************************************************/
//
// |=================|====================================|====================================|====================================|
// |           State | INIT                               | START_PENDING                      | STARTED                            |
// | Event           |                                    |                                    |                                    |
// |=================|====================================|====================================|====================================|
// | UP              | Action: Save counters & time       | Action: -                          | Action: -                          |
// |                 |         Send start message         |                                    |                                    |
// |                 |                                    |                                    |                                    |
// |                 | New state: START_PENDING           | New state: -                       | New state: -                       |
// |-----------------|------------------------------------|------------------------------------|------------------------------------|
// | DOWN            | Action: -                          | Action: Cancel start message       | Action: Send stop message          |
// |                 |                                    |         Initialize shared sm       |         Initialize ahared sm       |
// |                 |                                    |                                    |                                    |
// |                 | New state: -                       | New state: INIT                    | New state: INIT                    |
// |-----------------|------------------------------------|------------------------------------|------------------------------------|
// | START_ACK_RCV   | Action: -                          | Action: Start interim timer        | Action: -                          |
// |                 |                                    |                                    |                                    |
// |                 | New state: -                       | New state: STARTED                 | New state: -                       |
// |-----------------|------------------------------------|------------------------------------|------------------------------------|
// | INTERIM_EXPIRED | Action: -                          | Action: -                          | Action: Send interim message       |
// |                 |                                    |                                    |                                    |
// |                 | New state: -                       | New state: -                       | New state: -                       |
// |-----------------|------------------------------------|------------------------------------|------------------------------------|
//
static void DOT1X_ACCT_state_machine(struct nas_sm *sm, u32 event)
{
    nas_eap_info_t  *eap_info  = nas_get_eap_info(sm);
    nas_port_info_t *port_info = nas_get_port_info(sm);
    acct_sm_t       *acct;
    u32             curr_state;
    u32             next_state;

    if (eap_info == NULL) {
        T_EG(TRACE_GRP_ACCT, "No eap_info");
        return;
    }

    if (port_info == NULL) {
        T_EG(TRACE_GRP_ACCT, "No port_info");
        return;
    }

    acct       = &eap_info->acct;
    curr_state = acct->state;
    next_state = acct->state;

    T_NG(TRACE_GRP_ACCT, "enter, mode = %s, port = %u", dot1x_port_control_to_str(port_info->port_control, FALSE), port_info->port_no);
    if (event >= DOT1X_ACCT_EVENT_LAST) {
        T_EG(TRACE_GRP_ACCT, "Invalid event (%u)", event);
        return;
    }

    switch (curr_state) {
    case DOT1X_ACCT_STATE_INIT:
        switch (event) {
        case DOT1X_ACCT_EVENT_UP:
            if (vtss_radius_acct_ready()) {
                if (NAS_PORT_CONTROL_IS_SINGLE_CLIENT(port_info->port_control)) { // Save counters
                    vtss_isid_t          isid     = DOT1X_NAS_PORT_2_ISID(port_info->port_no);
                    vtss_port_no_t       api_port = DOT1X_NAS_PORT_2_SWITCH_API_PORT(port_info->port_no);
                    vtss_rc              rc;
                    vtss_port_counters_t counters;
                    acct->counters = FALSE;
                    if (VTSS_ISID_LEGAL(isid) && msg_switch_exists(isid)) {
                        if ((rc = port_mgmt_counters_get(isid, api_port, &counters)) == VTSS_RC_OK) {
                            acct->input_packets  = counters.rmon.rx_etherStatsPkts;
                            acct->output_packets = counters.rmon.tx_etherStatsPkts;
                            acct->input_octets   = counters.rmon.rx_etherStatsOctets;
                            acct->output_octets  = counters.rmon.tx_etherStatsOctets;
                            acct->counters = TRUE;
                        } else {
                            T_EG(TRACE_GRP_ACCT, "Unable to read counters (code = %d, str=%s)", rc, error_txt(rc));
                        }
                    } else {
                        T_EG(TRACE_GRP_ACCT, "Invalid isid (%d) or non-existing switch", isid);
                    }
                }
                acct->session_time = cyg_current_time() / CYGNUM_HAL_RTC_DENOMINATOR; /* Save uptime in seconds */
                DOT1X_ACCT_radius_tx_request(sm, DOT1X_ACCT_STATUS_TYPE_START);
                next_state = DOT1X_ACCT_STATE_START_PENDING;
            }
            break;
        }
        break;
    case DOT1X_ACCT_STATE_START_PENDING:
        switch (event) {
        case DOT1X_ACCT_EVENT_DOWN:
            if (acct->handle != -1) {
                (void)vtss_radius_free(acct->handle); // Cancel the pending start message
            }
            DOT1X_ACCT_sm_init(sm, TRUE);
            next_state = DOT1X_ACCT_STATE_INIT;
            break;
        case DOT1X_ACCT_EVENT_START_ACK_RCV:
            if (acct->interim_interval) {
                acct->interim_timer = acct->interim_interval; // Start interim timer
            }
            next_state = DOT1X_ACCT_STATE_STARTED;
            break;
        }
        break;
    case DOT1X_ACCT_STATE_STARTED:
        switch (event) {
        case DOT1X_ACCT_EVENT_DOWN:
            if (vtss_radius_acct_ready()) {
                DOT1X_ACCT_radius_tx_request(sm, DOT1X_ACCT_STATUS_TYPE_STOP);
            }
            DOT1X_ACCT_sm_init(sm, TRUE);
            next_state = DOT1X_ACCT_STATE_INIT;
            break;
        case DOT1X_ACCT_EVENT_INTERIM_EXPIRED:
            if (vtss_radius_acct_ready()) {
                DOT1X_ACCT_radius_tx_request(sm, DOT1X_ACCT_STATUS_TYPE_INTERIM_UPDATE);
            }
            break;
        }
        break;
    default:
        T_EG(TRACE_GRP_ACCT, "Invalid state (%u)", curr_state);
    }
    T_DG(TRACE_GRP_ACCT, "event = %u, state = %u, next_state = %u", event, curr_state, next_state);
    acct->state = next_state;
}

/******************************************************************************/
// DOT1X_ACCT_reset_callback()
// Called when system is reset.
// Fake a global disable of dot1x in order to send out accounting stop frames
/******************************************************************************/
static void DOT1X_ACCT_reset_callback(vtss_restart_t restart)
{
    dot1x_disable_due_to_soon_boot();
}

/******************************************************************************/
//
// nas_os_XXX functions. Callbacks from the nas/base/ files
//
/******************************************************************************/

/******************************************************************************/
// Callback from the core dot1x code when the 1sec timer tics.
// Part of the accounting code that cannot be called from the RADIUS thread
// (e.g.code that sends a RADIUS frame) must be called from here instead.
/******************************************************************************/
void nas_os_acct_timer_tick(struct nas_sm *sm)
{
    nas_eap_info_t  *eap_info  = nas_get_eap_info(sm);
    nas_port_info_t *port_info = nas_get_port_info(sm);
    acct_sm_t       *acct;

    if (eap_info == NULL) {
        T_EG(TRACE_GRP_ACCT, "No eap_info");
        return;
    }

    if (port_info == NULL) {
        T_EG(TRACE_GRP_ACCT, "No port_info");
        return;
    }

    acct = &eap_info->acct;

    T_RG(TRACE_GRP_ACCT, "enter, mode = %s, port = %u", dot1x_port_control_to_str(port_info->port_control, FALSE), port_info->port_no);
    dot1x_crit_assert_locked();

    // If the session is reinitialized, we can receive a very fast unauthorized -> autorized transition.
    // If we get a disabled event, we will wait to look for the authorized event at the next timer tick
    if (acct->authorize_disabled) {
        T_DG(TRACE_GRP_ACCT, "Authorized p %u \"Down\", e: %d, d: %d", port_info->port_no, acct->authorize_enabled, acct->authorize_disabled);
        DOT1X_ACCT_state_machine(sm, DOT1X_ACCT_EVENT_DOWN);
        acct->authorize_disabled = FALSE;
    } else if (acct->authorize_enabled) {
        T_DG(TRACE_GRP_ACCT, "Authorized p %u \"Up\", e: %d, d: %d", port_info->port_no, acct->authorize_enabled, acct->authorize_disabled);
        DOT1X_ACCT_state_machine(sm, DOT1X_ACCT_EVENT_UP);
        acct->authorize_enabled = FALSE;
    }

    if (acct->interim_timer) {
        if (--acct->interim_timer == 0) { // interim timer is expired
            T_DG(TRACE_GRP_ACCT, "Interim timer expired on port %u. Resetting it to %u seconds", port_info->port_no, acct->interim_interval);
            DOT1X_ACCT_state_machine(sm, DOT1X_ACCT_EVENT_INTERIM_EXPIRED);
            acct->interim_timer = acct->interim_interval; // restart interim timer
        }
    }
}

/******************************************************************************/
// Callback from the core dot1x code when a state machine is initialized.
/******************************************************************************/
void nas_os_acct_sm_init(struct nas_sm *sm)
{
    nas_port_info_t *port_info = nas_get_port_info(sm);

    if (port_info == NULL) {
        T_EG(TRACE_GRP_ACCT, "No port_info");
        return;
    }

    T_NG(TRACE_GRP_ACCT, "enter, mode = %s, port = %u", dot1x_port_control_to_str(port_info->port_control, FALSE), port_info->port_no);
    dot1x_crit_assert_locked();
    DOT1X_ACCT_sm_init(sm, FALSE);
}

/******************************************************************************/
// nas_os_acct_sm_release()
// Callback from the core code when a state machine is released.
// Only relevant for MacBased
/******************************************************************************/
void nas_os_acct_sm_release(struct nas_sm *sm)
{
    nas_eap_info_t  *eap_info  = nas_get_eap_info(sm);
    nas_port_info_t *port_info = nas_get_port_info(sm);
    acct_sm_t       *acct;

    if (eap_info == NULL) {
        T_EG(TRACE_GRP_ACCT, "No eap_info");
        return;
    }

    if (port_info == NULL) {
        T_EG(TRACE_GRP_ACCT, "No port_info");
        return;
    }

    acct = &eap_info->acct;

    T_NG(TRACE_GRP_ACCT, "enter, mode = %s, port = %u", dot1x_port_control_to_str(port_info->port_control, FALSE), port_info->port_no);
    dot1x_crit_assert_locked();

    acct->releasing = TRUE;
    DOT1X_ACCT_state_machine(sm, DOT1X_ACCT_EVENT_DOWN);
}

/******************************************************************************/
//
// dot1x_acct functions. Callbacks from the nas/platform files
//
/******************************************************************************/

/****************************************************************************/
// dot1x_acct_authorized_changed()
/****************************************************************************/
void dot1x_acct_authorized_changed(nas_port_control_t admin_state, struct nas_sm *sm, BOOL authorized)
{
    nas_eap_info_t  *eap_info  = nas_get_eap_info(sm);
    nas_port_info_t *port_info = nas_get_port_info(sm);

    if (eap_info == NULL) {
        T_EG(TRACE_GRP_ACCT, "No eap_info");
        return;
    }

    if (port_info == NULL) {
        T_EG(TRACE_GRP_ACCT, "No port_info");
        return;
    }

    T_NG(TRACE_GRP_ACCT, "enter, admin_state = %s, port = %u, authorized = %s", dot1x_port_control_to_str(admin_state, FALSE), port_info->port_no, authorized ? "Up" : "Down");

    if (!NAS_PORT_CONTROL_IS_ACCOUNTABLE(admin_state)) {
        return;
    }

    // Notify accounting that the authorized state has changed
    // If the session is reinitialized, we can receive a very fast unauthorized -> authorized transition.
    // and this is why we will need to save both events
    if (authorized) {
        eap_info->acct.authorize_enabled = TRUE;
        T_DG(TRACE_GRP_ACCT, "Authorized p %u \"Up\", e: %d, d: %d", port_info->port_no, eap_info->acct.authorize_enabled, eap_info->acct.authorize_disabled);
    } else {
        eap_info->acct.authorize_disabled = TRUE;
        eap_info->acct.authorize_enabled = FALSE;
        T_DG(TRACE_GRP_ACCT, "Authorized p %u \"Down\", e: %d, d: %d", port_info->port_no, eap_info->acct.authorize_enabled, eap_info->acct.authorize_disabled);
    }
}

/****************************************************************************/
// dot1x_acct_radius_rx()
/****************************************************************************/
void dot1x_acct_radius_rx(u8 radius_handle, nas_eap_info_t *eap_info)
{
    u32 interim_interval;
    u16 tlv_len;

    tlv_len = sizeof(interim_interval);
    if (vtss_radius_tlv_get(radius_handle, VTSS_RADIUS_ATTRIBUTE_ACCT_INTERIM_INTERVAL, &tlv_len, (u8 *)&interim_interval) == VTSS_RC_OK) {
        eap_info->acct.interim_interval = NET2HOSTL(interim_interval);
        T_DG(TRACE_GRP_ACCT, "Interim interval set to %u seconds", eap_info->acct.interim_interval);
    }
}

/****************************************************************************/
// dot1x_acct_append_radius_tlv()
/****************************************************************************/
BOOL dot1x_acct_append_radius_tlv(u8 radius_handle, nas_eap_info_t *eap_info)
{
    vtss_rc rc;
    i8      temp_str[20];

    // Acct-Session-Id
    (void)snprintf(temp_str, sizeof(temp_str) - 1, "%08X",   eap_info->acct.session_id);
    rc = vtss_radius_tlv_set(radius_handle, VTSS_RADIUS_ATTRIBUTE_ACCT_SESSION_ID, strlen(temp_str), (u8 *)temp_str, FALSE);
    if (rc != VTSS_RC_OK && rc != VTSS_RADIUS_ERROR_NOT_ROOM_FOR_TLV) {
        T_E("Got \"%s\" from vtss_tlv_set() on optional attribute", vtss_error_txt(rc));
        return FALSE;
    }
    return TRUE;
}

/****************************************************************************/
// dot1x_acct_init()
/****************************************************************************/
void dot1x_acct_init(void)
{
    // Register system reset callback
    control_system_reset_register(DOT1X_ACCT_reset_callback);
}
