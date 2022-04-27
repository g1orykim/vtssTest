/*

 Copyright (c) 2002-2009 Vitesse Semiconductor Corporation "Vitesse". All
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

// This file corresponds to a merging of
//   wpa_supplicant-0.6.1/wpa_supplicant/wpa_supplicant.c and
//   wpa_supplicant-0.6.1/src/eap_peer/eap_methods.c
// and is the top-level mac-based authentication supplicant file.

// In wpa_supplicant, all eap methods are allocated and added dynamically,
// but we want to add the supported methods (currently only MD5) statically.
// The methods themselves do not contain state/per-port data, so only one
// instance of each is needed. In order to re-use as much of the original
// source as possible, we still use the eap_methods array of methods, despite
// the fact than only one method is needed.

// All Base-lib functions and base-lib callout functions are thread safe, as they are called
// with DOT1X_CRIT() taken, but Lint cannot see that in its final wrap-up (thread walk)
/*lint -esym(459, MB_backend_frame_received)          */
/*lint -esym(459, MB_backend_server_timeout_occurred) */
/*lint -esym(459, MB_clear_statistics)                */
/*lint -esym(459, MB_get_backend_counters)            */
/*lint -esym(459, MB_init)                            */
/*lint -esym(459, MB_reauthenticate)                  */
/*lint -esym(459, MB_reinitialize)                    */
/*lint -esym(459, MB_timer_tick)                      */
/*lint -esym(459, MB_init_sm)                         */
/*lint -esym(459, MB_uninit_sm)                       */

// For definition of eap_peer_register_methods().
#include "peer_methods.h"
#include "peer_sm.h"
#include "peer.h"
#include "../common/nas.h"

#define PEER_BACKEND_COUNTER_INCREMENT(counter)       \
  do {                                                \
    PEER_SM(sm)->eap_info.backend_counters.counter++; \
    sm->port_info->backend_counters->counter++;       \
  } while(0)

/******************************************************************************/
// peer_backend_send_response()
/******************************************************************************/
void peer_backend_send_response(nas_sm_t *sm)
{
    nas_eap_info_t *eap_info = &PEER_SM(sm)->eap_info;
    u16            len       = PEER_SM(sm)->eapRespDataLen;

    if (nas_os_backend_server_tx_request(sm,
                                         &eap_info->radius_handle,
                                         eap_info->last_frame,
                                         len,
                                         eap_info->radius_state_attribute,
                                         eap_info->radius_state_attribute_len,
                                         sm->client_info.identity,
                                         sm->port_info->port_no,
                                         sm->client_info.vid_mac.mac.addr)) {

        // Call to the RADIUS module succeeded. Since that
        // module is guaranteed to call us back with either
        // a response or a timeout, we shouldn't timeout
        // ourselves, but simply wait for that event.
        // Therefore we set the idleWhile to a number so big
        // that it will never reach 0 before the RADIUS server
        // has called us back.
        PEER_SM(sm)->EAPOL_idleWhile = 0xFFFFFFFFUL;
    } else {
        // The RADIUS module is not ready yet, or no
        // RADIUS servers are enabled/configured.
        // Try again in 15 seconds, but give up when fake_request_identities_left reaches 0.
        // This is handled in peer_eap.c's EAPOL_idleWhile countdown code.
        if (PEER_SM(sm)->fake_request_identities_left > 0) {
            PEER_SM(sm)->fake_request_identities_left--;
        }
        PEER_SM(sm)->EAPOL_idleWhile = 15;
    }

    PEER_SM(sm)->EAPOL_eapResp = FALSE;

    // Count the number of responses both in this SM and the port SM
    PEER_BACKEND_COUNTER_INCREMENT(backendResponses);
}

/******************************************************************************/
// MB_get_next_eap_identifier()
/******************************************************************************/
static u8 MB_get_next_eap_identifier(nas_sm_t *sm, BOOL creating_canned_eap)
{
    u8 result;
    if (creating_canned_eap) {
        // For artificially created success and failure packets, the ID of the packet must be the
        // same as the ID of the previous response, which happens to be the state machine's lastId member.
        result = PEER_SM(sm)->lastId;
    } else {
        result = sm->last_eap_tx_identifier + 1;
    }
    sm->last_eap_tx_identifier = result;
    return result;
}

/******************************************************************************/
// MB_create_canned_eap()
/******************************************************************************/
static inline void MB_create_canned_eap(nas_sm_t *sm, BOOL success)
{
    nas_eap_info_t *eap_info = &PEER_SM(sm)->eap_info;
    u8             *eap;

    eap = eap_info->last_frame;
    *eap++ = success ? EAP_CODE_SUCCESS : EAP_CODE_FAILURE;
    *eap++ = MB_get_next_eap_identifier(sm, TRUE);

    // Length is 4 bytes for header
    *eap++ = 0;
    *eap++ = 4;

    eap_info->last_frame_len = 4;

    // Register this frame type
    eap_info->last_frame_type = FRAME_TYPE_NO_RETRANSMISSION;
}

/******************************************************************************/
// MB_do_clear_statistics()
/******************************************************************************/
static void MB_do_clear_statistics(nas_sm_t *sm)
{
    memset(&PEER_SM(sm)->eap_info.backend_counters, 0, sizeof(PEER_SM(sm)->eap_info.backend_counters));
}

/******************************************************************************/
// MB_clear_statistics()
// If called with a top-level SM, clear all statistics including global.
// If called with a sub-level SM, clear only this SM's statistics.
/******************************************************************************/
static void MB_clear_statistics(nas_sm_t *top_or_sub_sm)
{
    if (top_or_sub_sm->sm_type == NAS_SM_TYPE_TOP) {
        nas_sm_t *sm;
        // Clear top-level counters
        memset(top_or_sub_sm->port_info->backend_counters, 0, sizeof(*top_or_sub_sm->port_info->backend_counters));
        for (sm = top_or_sub_sm->next; sm; sm = sm->next) {
            MB_do_clear_statistics(sm);
        }
    } else {
        // Only clear for this SM.
        MB_do_clear_statistics(top_or_sub_sm);
    }
}

/******************************************************************************/
// MB_init()
/******************************************************************************/
static void MB_init(void)
{
    // Initialize the EAP methods we support (MD5, that is).
    if (eap_peer_register_methods() != 0) {
        T_EG(TRACE_GRP_BASE, "Register Methods failed");
    }
}

/******************************************************************************/
// MB_init_sm()
// An SM has just been allocated. Initialize it.
/******************************************************************************/
static void MB_init_sm(nas_sm_t *sm, vtss_vid_mac_t *vid_mac, BOOL is_req_identity_sm)
{
    nas_sm_t *top_sm = sm->port_info->top_sm;

    sm->last_eap_tx_identifier = top_sm->last_eap_tx_identifier;

    sm->client_info.vid_mac = top_sm->client_info.vid_mac = *vid_mac;

    // Create the identity, which for MAC-based authentication is a string representing the MAC address
    nas_os_mac2str(vid_mac->mac.addr, sm->client_info.mac_addr_str);
    strcpy(sm->client_info.identity,  sm->client_info.mac_addr_str);

    // Also update the  "last client" info.
    strcpy(top_sm->client_info.identity,     sm->client_info.identity);
    strcpy(top_sm->client_info.mac_addr_str, sm->client_info.mac_addr_str);

    // We start the EAP SM by sending a fake request identity EAPOL packet. The EAP SM
    // calls the nas_os_backend_server_tx_request() function to send a RADIUS frame. At boot time
    // (and if the RADIUS module is not configured (properly)), the RADIUS module may not
    // be up and running yet (e.g. IP stack not up and running). This means that if
    // nas_os_backend_ser_tx_request() returns FALSE, we should retry in a short while, counted
    // by EAPOL_idleWhile. In order to get the EAP SM to retry, and not directly enter
    // the unauthorized state, we only let EAPOL_idleWhile reach 0 when fake_request_identities_left
    // is 0.
#define MB_FAKE_REQUEST_IDENTITY_CNT 2
    PEER_SM(sm)->fake_request_identities_left     = MB_FAKE_REQUEST_IDENTITY_CNT;
    PEER_SM(sm)->eap_info.radius_handle           = -1;
    PEER_SM(sm)->eap_info.stop_reason             = NAS_STOP_REASON_UNKNOWN;
    PEER_SM(sm)->authStatus                       = NAS_PORT_STATUS_UNAUTHORIZED;

    // Clear statistics
    MB_do_clear_statistics(sm);

    peer_sm_init(sm);

#ifdef VTSS_SW_OPTION_DOT1X_ACCT
    nas_os_acct_sm_init(sm);
#endif

    peer_force_reinit(sm, FALSE);
}

/******************************************************************************/
// MB_uninit_sm()
// Pre-warning that this SM is going to be freed in a split-second.
/******************************************************************************/
static void MB_uninit_sm(nas_sm_t *sm)
{
    // Remove it from the list of used RADIUS IDs
    nas_os_backend_server_free_resources(sm);

#ifdef VTSS_SW_OPTION_DOT1X_ACCT
    nas_os_acct_sm_release(sm);
#endif

    nas_os_freeing_sm(sm);
}

/******************************************************************************/
// MB_timer_tick()
/******************************************************************************/
static nas_stop_reason_t MB_timer_tick(nas_sm_t *sm)
{
#ifdef VTSS_SW_OPTION_DOT1X_ACCT
    nas_os_acct_timer_tick(sm);
#endif

    peer_timers_tick(sm);

    return NAS_STOP_REASON_NONE; // Don't delete me.
}

/******************************************************************************/
// MB_reauth_param_changed()
/******************************************************************************/
static void MB_reauth_param_changed(nas_sm_t *top_sm, BOOL ena, u16 reauth_period)
{
    nas_sm_t *sm;
    for (sm = top_sm->next; sm; sm = sm->next) {
        PEER_SM(sm)->reAuthEnabled = ena;
        PEER_SM(sm)->reAuthWhen    = ena ? reauth_period : 0;
    }
}

/******************************************************************************/
// MB_reauthenticate()
/******************************************************************************/
static void MB_reauthenticate(nas_sm_t *top_sm)
{
    nas_sm_t *sm;

    // Reauthenticate all clients on the port
    for (sm = top_sm->next; sm; sm = sm->next) {
        // Only reauthenticate authorized ports.
        if (PEER_SM(sm)->authStatus == NAS_PORT_STATUS_AUTHORIZED) {
            peer_force_reauth(sm, TRUE);
        }
    }
}

/******************************************************************************/
// MB_reinitialize()
/******************************************************************************/
static void MB_reinitialize(nas_sm_t *top_sm)
{
    nas_sm_t *sm;

    // Brute force restart all clients
    for (sm = top_sm->next; sm; sm = sm->next) {
        peer_force_reinit(sm, TRUE);
    }
}

/******************************************************************************/
// MB_get_sm_from_radius_handle()
/******************************************************************************/
static nas_sm_t *MB_get_sm_from_radius_handle(nas_sm_t *top_sm, int radius_handle)
{
    nas_sm_t *sm;
    for (sm = top_sm->next; sm; sm = sm->next) {
        if (PEER_SM(sm)->eap_info.radius_handle == radius_handle) {
            return sm;
        }
    }
    return NULL;
}

/******************************************************************************/
// MB_update_backend_counters()
/******************************************************************************/
static inline void MB_update_backend_counters(nas_sm_t *sm, nas_backend_code_t code)
{
    switch (code) {
    case NAS_BACKEND_CODE_ACCESS_CHALLENGE:
        PEER_BACKEND_COUNTER_INCREMENT(backendAccessChallenges);
        break;

    case NAS_BACKEND_CODE_ACCESS_ACCEPT:
        PEER_BACKEND_COUNTER_INCREMENT(backendAuthSuccesses);
        break;

    case NAS_BACKEND_CODE_ACCESS_REJECT:
        PEER_BACKEND_COUNTER_INCREMENT(backendAuthFails);
        break;
    }
}

/******************************************************************************/
// MB_backend_frame_received()
/******************************************************************************/
static void MB_backend_frame_received(nas_sm_t *sm, nas_backend_code_t code)
{
    // Gotta count the events here and not in the state machine
    // if we're running MAC-based, because the statemachine doesn't
    // know if the packet was artificially created or not.
    // We also count on the port state-machine, which holds a grand total.
    MB_update_backend_counters(sm, code);
    PEER_SM(sm)->EAPOL_eapReq = TRUE;
    if (PEER_SM(sm)->eap_info.last_frame_len == 0) {
        if (code == NAS_BACKEND_CODE_ACCESS_ACCEPT || code == NAS_BACKEND_CODE_ACCESS_REJECT) {
            // Under some circumstances the RADIUS server doesn't send an EAP attribute with the RADIUS frame.
            // In these cases we need to create an artificial sucecss or failure packet and pass it on to the peer state machine,
            // i.e. this is only needed for MAC-based authentication.
            MB_create_canned_eap(sm, code == NAS_BACKEND_CODE_ACCESS_ACCEPT ? TRUE : FALSE);
        } else {
            // It didn't contain EAP data. Nothing to signal to the peer SM.
            PEER_SM(sm)->EAPOL_eapReq = FALSE;
        }
    }

    // Step State machine
    peer_sm_step(sm);
}

/******************************************************************************/
// MB_backend_server_timeout_occurred()
/******************************************************************************/
static void MB_backend_server_timeout_occurred(nas_sm_t *sm)
{
    PEER_SM(sm)->eap_info.stop_reason = NAS_STOP_REASON_AUTH_TIMEOUT;
    PEER_SM(sm)->EAPOL_idleWhile = 0;
    peer_sm_step(sm);
}

/******************************************************************************/
// MB_new_auth_session()
/******************************************************************************/
static void MB_new_auth_session(nas_sm_t *sm)
{
    nas_eap_info_t *eap_info = &PEER_SM(sm)->eap_info;

    PEER_SM(sm)->EAPOL_eapSuccess = FALSE;
    PEER_SM(sm)->EAPOL_eapFail    = FALSE;

    // No state for initial request/identity packet
    eap_info->radius_state_attribute_len = 0;
}

/******************************************************************************/
// MB_get_port_status()
/******************************************************************************/
static nas_port_status_t MB_get_port_status(nas_sm_t *top_sm)
{
    nas_sm_t *sm;
    u32 auth_cnt = 0, unauth_cnt = 0;

    // We don't have one single port-status, so we loop through all of our
    // clients and return ((auth_cnt + 1) << 16) | (unauth_cnt + 1)
    for (sm = top_sm->next; sm; sm = sm->next) {
        if (PEER_SM(sm)->authStatus == NAS_PORT_STATUS_AUTHORIZED) {
            auth_cnt++;
        } else {
            unauth_cnt++;
        }
    }

    return nas_os_encode_auth_unauth(auth_cnt, unauth_cnt);
}

/******************************************************************************/
// MB_get_status()
/******************************************************************************/
static nas_port_status_t MB_get_status(nas_sm_t *sub_sm)
{
    return PEER_SM(sub_sm)->authStatus;
}

/******************************************************************************/
// MB_get_backend_counters()
// If SM is top_sm, get overall counters for that port, otherwise get for
// that SM.
/******************************************************************************/
static nas_backend_counters_t *MB_get_backend_counters(nas_sm_t *top_or_sub_sm)
{
    if (top_or_sub_sm->sm_type == NAS_SM_TYPE_TOP) {
        // Get summed backend counters.
        return top_or_sub_sm->port_info->backend_counters;
    } else {
        return &PEER_SM(top_or_sub_sm)->eap_info.backend_counters;
    }
}

/******************************************************************************/
// MB_get_eap_info()
// sm is never a top_sm.
/******************************************************************************/
static nas_eap_info_t *MB_get_eap_info(nas_sm_t *sub_sm)
{
    return &PEER_SM(sub_sm)->eap_info;
}

/******************************************************************************/
// peer_request_identity()
/******************************************************************************/
void peer_request_identity(nas_sm_t *sm)
{
    nas_eap_info_t *eap_info = &PEER_SM(sm)->eap_info;
    u8             *eap;

    eap = eap_info->last_frame;

    MB_new_auth_session(sm);

#ifdef VTSS_SW_OPTION_DOT1X_ACCT
    if (eap_info->acct.session_id == 0) {
        eap_info->acct.session_id = NAS_new_session_id();
    }
#endif

    *eap++ = EAP_CODE_REQUEST;
    *eap++ = MB_get_next_eap_identifier(sm, FALSE);

    // Length is 4 bytes for header + 1 byte for code
    *eap++ = 0;
    *eap++ = 5;

    *eap = (u8)EAP_TYPE_IDENTITY;

    eap_info->last_frame_len = 5;

    // Register this frame type
    eap_info->last_frame_type = FRAME_TYPE_EAPOL;
}

/******************************************************************************/
// Function pointers passed to the NAS umbrella.
// We really don't have to fill in all the unsupported functions with NULL.
// The reason I've done it is to check that I've implemented all.
/******************************************************************************/
static const nas_funcs_t MB_funcs = {
    // All SMs:
    .init                            = MB_init,
    .init_sm                         = MB_init_sm,
    .uninit_sm                       = MB_uninit_sm,
    .clear_statistics                = MB_clear_statistics,
    .timer_tick                      = MB_timer_tick,
    .get_port_status                 = MB_get_port_status,
    .get_status                      = MB_get_status,
    .set_status                      = NULL,

    // IEEE8021X SMs:
    .get_eapol_counters              = NULL,
    .ieee8021x_eapol_frame_received  = NULL,
    .fake_force_authorized           = NULL,
    .reset_reauth_cnt                = NULL,

    // Authentication SMs:
    .reauth_param_changed            = MB_reauth_param_changed,
    .reauthenticate                  = MB_reauthenticate,
    .reinitialize                    = MB_reinitialize,
    .get_sm_from_radius_handle       = MB_get_sm_from_radius_handle,
    .backend_frame_received          = MB_backend_frame_received,
    .backend_server_timeout_occurred = MB_backend_server_timeout_occurred,
    .get_backend_counters            = MB_get_backend_counters,
    .get_eap_info                    = MB_get_eap_info,
};

/******************************************************************************/
// mac_based_get_funcs()
// Called by the NAS umbrella to get our function pointers.
/******************************************************************************/
nas_funcs_t const *mac_based_get_funcs(void)
{
    return &MB_funcs;
}

