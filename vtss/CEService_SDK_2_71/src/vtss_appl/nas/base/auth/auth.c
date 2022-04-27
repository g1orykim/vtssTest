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

// All Base-lib functions and base-lib callout functions are thread safe, as they are called
// with DOT1X_CRIT() taken, but Lint cannot see that in its final wrap-up (thread walk)
/*lint -esym(459, IEEE8021X_timer_tick)                      */
/*lint -esym(459, IEEE8021X_backend_frame_received)          */
/*lint -esym(459, IEEE8021X_fake_force_authorized)           */
/*lint -esym(459, IEEE8021X_backend_server_timeout_occurred) */
/*lint -esym(459, IEEE8021X_eapol_frame_received)            */
/*lint -esym(459, IEEE8021X_init_sm)                         */
/*lint -esym(459, IEEE8021X_uninit_sm)                       */
/*lint -esym(459, IEEE8021X_reinitialize)                    */

#include <string.h>
#include "auth.h"
#include "auth_sm.h"
#include <network.h>

#define AUTH_EAPOL_COUNTER_SET(counter, val)      \
  do {                                            \
    AUTH_SM(sm)->eapol_counters.counter = val;    \
    sm->port_info->eapol_counters->counter = val; \
  } while(0)

/******************************************************************************/
// Naming conventions:
//   Public functions: auth_xxx()
//   Private functions: AUTH_xxx()
/******************************************************************************/

/******************************************************************************/
//
//  Local functions
//
/******************************************************************************/

/******************************************************************************/
// AUTH_print_eapol_packet()
/******************************************************************************/
static inline void AUTH_print_eapol_packet(nas_ieee8021x_eapol_packet_t *p, nas_port_t port)
{
    static const char *packet_types[] = {"EAP-Packet", "EAPOL-Start", "EAPOL-Logoff", "EAPOL-Key", "EAPOL-Enc.-ASF-ALERT"};

    if (p->packet_type != NAS_IEEE8021X_TYPE_EAPOL_START &&
        p->packet_type != NAS_IEEE8021X_TYPE_EAPOL_LOGOFF) {
        T_NG(TRACE_GRP_BASE, "%u: Packet type: %s, body length: %d", port, packet_types[p->packet_type], p->body_length);
    } else {
        T_NG(TRACE_GRP_BASE, "%u: Packet type: %s", port, packet_types[p->packet_type]);
    }
}

/******************************************************************************/
// AUTH_send()
/******************************************************************************/
static void AUTH_send(nas_sm_t *sm)
{
    static       u8 EAPOL_tx_buffer[NAS_MAX_FRAME_SIZE];
    static const u8 eap_mac_addr[6] = NAS_IEEE8021X_MAC_ADDR;
    nas_eap_info_t  *eap_info       = &AUTH_SM(sm)->eap_info;

    if (BVTSTNOT(skipFirstEapolTx)) {
        // type is not correct if code == 3 (success) or code == 4 (failure)
        T_DG(TRACE_GRP_BASE, "%u: Tx EAPOL (code = %u, type = %u)", sm->port_info->port_no, eap_info->last_frame[0], eap_info->last_frame[4]);

#ifdef VTSS_SW_OPTION_NAS_DOT1X_MULTI
        if (sm->port_info->port_control == NAS_PORT_CONTROL_DOT1X_MULTI && BVTSTNOT(isReqIdentitySM)) {
            if (sm->client_info.vid_mac.vid == 0) {
                T_EG(TRACE_GRP_BASE, "%u: Invalid internal state. Found VID = 0 on non-ReqIdSM", sm->port_info->port_no);
            }

            // If we're in Multi 802.1X mode, we always unicast EAPOL frames to the supplicant's
            // MAC address. The exception is when this is a request identity SM, in which case
            // no supplicants are attached (so to wake up any supplicants, we multicast it).
            memcpy(EAPOL_tx_buffer, sm->client_info.vid_mac.mac.addr, 6);
        } else
#endif
        {
            // Set PAE group address as DMAC
            memcpy(EAPOL_tx_buffer, eap_mac_addr, 6);
        }

        // SMAC
        // Fill in port MAC address
        nas_os_get_port_mac(sm->port_info->port_no, &EAPOL_tx_buffer[6]);

        // ETH Type
        EAPOL_tx_buffer[12] = NAS_IEEE8021X_ETH_TYPE >> 8;
        EAPOL_tx_buffer[13] = NAS_IEEE8021X_ETH_TYPE & 0xFF;

        // EAPOL header
        EAPOL_tx_buffer[14] = NAS_IEEE8021X_EAPOL_VERSION;
        EAPOL_tx_buffer[15] = NAS_IEEE8021X_TYPE_EAP_PACKET;
        EAPOL_tx_buffer[16] = (eap_info->last_frame_len >> 8);
        EAPOL_tx_buffer[17] = (eap_info->last_frame_len & 0xFF);

        // EAP Data
        memcpy(&EAPOL_tx_buffer[18], eap_info->last_frame, eap_info->last_frame_len);

        // Transmit frame
        // Register TX'ed identifier
        // See IEEE8021X_get_next_eap_identifier() for details.
        if (BVTST(isReqIdentitySM)) {
            sm->port_info->top_sm->last_eap_tx_identifier = eap_info->last_frame[1];
        } else {
            sm->last_eap_tx_identifier = eap_info->last_frame[1];
        }

        nas_os_ieee8021x_send_eapol_frame(sm->port_info->port_no, EAPOL_tx_buffer, 14 + 4 + eap_info->last_frame_len);

        AUTH_EAPOL_COUNTER_INCREMENT(dot1xAuthEapolFramesTx);

        if (EAPOL_tx_buffer[18] == EAP_CODE_REQUEST) {
            // Register this frame type
            eap_info->last_frame_type = FRAME_TYPE_EAPOL;

            if (EAPOL_tx_buffer[22] == (u8)EAP_TYPE_IDENTITY) {
                AUTH_EAPOL_COUNTER_INCREMENT(dot1xAuthEapolReqIdFramesTx);
            } else {
                AUTH_EAPOL_COUNTER_INCREMENT(dot1xAuthEapolReqFramesTx);

                // Start supplicant timeout
                auth_start_timeout(sm, NAS_IEEE8021X_TIMEOUT_SUPPLICANT_RESPONSE_TIMEOUT);
            }
        } else {
            // Register this frame type
            eap_info->last_frame_type = FRAME_TYPE_NO_RETRANSMISSION;
        }
    } else {
        T_DG(TRACE_GRP_BASE, "%u: Skipping first EAPOL (code = %u, type = %u)", sm->port_info->port_no, eap_info->last_frame[0], eap_info->last_frame[4]);
        BVCLR(skipFirstEapolTx);
        eap_info->last_frame_type = FRAME_TYPE_NO_RETRANSMISSION;
        // If running port-based 802.1X and Guest VLAN is enabled and we've
        // just exited Guest VLAN because an EAPOL frame was received, then
        // the same SM is used for receiving the EAPOL, and reAuthCount might
        // be >= MaxReauthCnt, causing Guest VLAN to be entered immediately.
        // Therefore, we clear the current reAuthCount. In some situations
        // this might cause the SM to have one more run before actually
        // entering the Guest VLAN, but I think that's OK.
        AUTH_SM(sm)->auth_pae.reAuthCount = 0;
    }
}

/******************************************************************************/
// AUTH_handle_eap_response()
/******************************************************************************/
static void AUTH_handle_eap_response(nas_sm_t *sm, u8 *eap_hdr, u16 len)
{
    u8             *dat;
    u8             id_length = len - 5;
    nas_eap_info_t *eap_info = &AUTH_SM(sm)->eap_info;

    // When we get here, the eap response has been validated for size

    // Check for eapIdentifier
    dat = (u8 *)(eap_hdr + 1);
    // Here we cannot be a Request Identity SM, so use the SM's own last_eap_tx_identifier.
    // See also comment in IEEE8021X_get_next_eap_identifier().
    if (*dat != sm->last_eap_tx_identifier) {
        T_DG(TRACE_GRP_BASE, "%u: Discarding packet due to incorrect EAP identifier", sm->port_info->port_no);
        return;
    }

    // Cancel timeout timer for state machine ...
    auth_cancel_timeout(sm, NAS_IEEE8021X_TIMEOUT_EAPOL_TIMEOUT);
    auth_cancel_timeout(sm, NAS_IEEE8021X_TIMEOUT_SUPPLICANT_RESPONSE_TIMEOUT);

    // Check for response/identity
    dat = (u8 *)(eap_hdr + 4);

    if (*dat == (u8)EAP_TYPE_IDENTITY) {
        AUTH_EAPOL_COUNTER_INCREMENT(dot1xAuthEapolRespIdFramesRx);

        BVSET(eapResponseIdentity);

        // Remember identity for radius usage, section 2.1 of RFC 3579
        if (id_length >= NAS_SUPPLICANT_ID_MAX_LENGTH) {
            // No room for supplicant ID. We then supply user's MAC address instead of identity
            strcpy(sm->client_info.identity, sm->client_info.mac_addr_str);
        } else {
            // Plenty of room for identity
            memcpy(sm->client_info.identity, (dat + 1), id_length);
            sm->client_info.identity[id_length] = 0;
        }
        strcpy(sm->port_info->top_sm->client_info.identity, sm->client_info.identity);
    } else {
        AUTH_EAPOL_COUNTER_INCREMENT(dot1xAuthEapolRespFramesRx);
    }

    // Register this frame
    // Copy EAP packet
    memcpy(eap_info->last_frame, (u8 *)eap_hdr, len);
    eap_info->last_frame_len = len;

    // Register this frame type
    eap_info->last_frame_type = FRAME_TYPE_NO_RETRANSMISSION;

    // Signal to auth PAE state machine that frame is available
    BVSET(eapolEap);
}

/******************************************************************************/
// AUTH_handle_eap()
// Process incoming EAP packet from Supplicant
/******************************************************************************/
static BOOL AUTH_handle_eap(nas_sm_t *sm, nas_ieee8021x_eap_packet_hdr_t *eap_hdr, u16 len)
{
    u16 eap_len;

    // Check that header is present - if not, discard packet
    if (len < sizeof(nas_ieee8021x_eap_packet_hdr_t)) {
        return FALSE;
    }

    // Read len parameter
    eap_len = ntohs(eap_hdr->length);

    // Check supplied length
    if (eap_len < sizeof(nas_ieee8021x_eap_packet_hdr_t)) {
        T_DG(TRACE_GRP_BASE, "%u: Too short frame", sm->port_info->port_no);
        return FALSE;
    } else if (eap_len > len) {
        T_DG(TRACE_GRP_BASE, "%u: Too short frame to contain this EAP packet", sm->port_info->port_no);
        return FALSE;
    } else if (eap_len < len) {
        // Debugging only.
        T_DG(TRACE_GRP_BASE, "%u: Ignoring extra bytes after EAP packet", sm->port_info->port_no);
    }

    T_DG(TRACE_GRP_BASE, "%u: EAP packet ID (%u)", sm->port_info->port_no, eap_hdr->identifier);

    switch (eap_hdr->eap_code) {
    case EAP_CODE_REQUEST:
        T_DG(TRACE_GRP_BASE, "%u: EAP Request", sm->port_info->port_no);
        return TRUE;

    case EAP_CODE_RESPONSE:
        T_DG(TRACE_GRP_BASE, "%u: EAP Response", sm->port_info->port_no);
        AUTH_handle_eap_response(sm, (u8 *)eap_hdr, eap_len);
        return TRUE;

    case EAP_CODE_SUCCESS:
        T_DG(TRACE_GRP_BASE, "%u: EAP Success", sm->port_info->port_no);
        return TRUE;

    case EAP_CODE_FAILURE:
        T_DG(TRACE_GRP_BASE, "%u: EAP Failure", sm->port_info->port_no);
        return TRUE;

    default:
        T_DG(TRACE_GRP_BASE, "%u: Unknown code", sm->port_info->port_no);
        return TRUE;
    }
}

/******************************************************************************/
// IEEE8021X_init()
/******************************************************************************/
static void IEEE8021X_init(void)
{
}

/******************************************************************************/
// IEEE8021X_do_clear_statistics()
/******************************************************************************/
static void IEEE8021X_do_clear_statistics(nas_sm_t *sm)
{
    memset(&AUTH_SM(sm)->eapol_counters,            0, sizeof(AUTH_SM(sm)->eapol_counters));
    memset(&AUTH_SM(sm)->eap_info.backend_counters, 0, sizeof(AUTH_SM(sm)->eap_info.backend_counters));
}

/******************************************************************************/
// IEEE8021X_clear_statistics()
// If called with a top-level SM, clear all statistics including global.
// If called with a sub-level SM, clear only this SM's statistics.
/******************************************************************************/
static void IEEE8021X_clear_statistics(nas_sm_t *top_or_sub_sm)
{
    // If we're not a multi-client SM, we need to clear both the top and
    // the (only) SM attached.
    if (!NAS_PORT_CONTROL_IS_MULTI_CLIENT(top_or_sub_sm->port_info->port_control)) {
        // Well-well. It might be we shouldn't clear the top-level statistics
        // for Single 802.1X SMs. I don't know. But for now, we do.
        top_or_sub_sm = top_or_sub_sm->port_info->top_sm;
    }

    if (top_or_sub_sm->sm_type == NAS_SM_TYPE_TOP) {
        nas_sm_t *sm;
        // Clear top-level counters and all attached sub-SM counters
        memset(top_or_sub_sm->port_info->eapol_counters,   0, sizeof(*top_or_sub_sm->port_info->eapol_counters));
        memset(top_or_sub_sm->port_info->backend_counters, 0, sizeof(*top_or_sub_sm->port_info->backend_counters));
        for (sm = top_or_sub_sm->next; sm; sm = sm->next) {
            IEEE8021X_do_clear_statistics(sm);
        }
    } else {
        // Clear this SM's counters.
        IEEE8021X_do_clear_statistics(top_or_sub_sm);
    }
}

/******************************************************************************/
// IEEE8021X_timer_tick()
/******************************************************************************/
static nas_stop_reason_t IEEE8021X_timer_tick(nas_sm_t *sm)
{
#ifdef VTSS_SW_OPTION_DOT1X_ACCT
    nas_os_acct_timer_tick(sm);
#endif

    auth_timers_tick(sm);

    return AUTH_SM(sm)->eap_info.delete_me ? AUTH_SM(sm)->eap_info.stop_reason : NAS_STOP_REASON_NONE;
}

/******************************************************************************/
// IEEE8021X_do_update_client_info()
/******************************************************************************/
static void IEEE8021X_do_update_client_info(nas_sm_t *any_sm, vtss_vid_mac_t *vid_mac)
{
    any_sm->client_info.vid_mac = *vid_mac;
    nas_os_mac2str(any_sm->client_info.vid_mac.mac.addr, any_sm->client_info.mac_addr_str);
}

/******************************************************************************/
// IEEE8021X_update_client_info()
/******************************************************************************/
static void IEEE8021X_update_client_info(nas_sm_t *sub_sm, vtss_vid_mac_t *vid_mac, BOOL called_from_init)
{
    nas_sm_t *top_sm = sub_sm->port_info->top_sm;
    BOOL set_top = FALSE;
    BOOL set_sub = FALSE;

    if (called_from_init) {
        sub_sm->client_info.identity[0] = '\0';
        set_sub = TRUE;

        if (vid_mac->vid != 0) {
            // Only update the top SM's last client info if it's a valid MAC address.
            // It may not be a valid MAC address if it's a Request Identity SM or if it's a port-based 802.1X SM.
            set_top = TRUE;
            top_sm->client_info.identity[0] = '\0';
        }
    } else {
        // Never touch the VID part of the <VID, MAC> of this SM if it's MAC-table based. This is solely handled by the platform code.
        // Always overwrite the Top SM's last-client-info, since this is not used for anything, but statistics.
        set_top = TRUE;

        if (!NAS_PORT_CONTROL_IS_MAC_TABLE_AND_BPDU_BASED(top_sm->port_info->port_control)) {
            // Port-based. Also update the sub-SM's client info.
            set_sub = TRUE;
        }
    }

    if (set_top) {
        IEEE8021X_do_update_client_info(top_sm, vid_mac);
    }

    if (set_sub) {
        IEEE8021X_do_update_client_info(sub_sm, vid_mac);
    }
}

/******************************************************************************/
// IEEE8021X_init_sm()
// The SM has just been allocated. Initialize it.
/******************************************************************************/
static void IEEE8021X_init_sm(nas_sm_t *sm, vtss_vid_mac_t *vid_mac, BOOL is_req_identity_sm)
{
    sm->last_eap_tx_identifier = sm->port_info->top_sm->last_eap_tx_identifier;
    if (is_req_identity_sm) {
        BVSET(isReqIdentitySM);
    } else {
        BVCLR(isReqIdentitySM);
    }

    IEEE8021X_update_client_info(sm, vid_mac, TRUE);

    AUTH_SM(sm)->authStatus             = NAS_PORT_STATUS_UNAUTHORIZED;
    AUTH_SM(sm)->eap_info.radius_handle = -1; // == not in use
    // Clear statistics
    IEEE8021X_do_clear_statistics(sm);

#ifdef VTSS_SW_OPTION_DOT1X_ACCT
    nas_os_acct_sm_init(sm);
#endif

    auth_sm_init(sm);

    // If we're in Single- or Multi- 802.1X, we need to skip transmission of the first
    // EAPOL frame, since that's another Request Identity, which was already sent by the
    // dedicated Request Identity SM.
    if (NAS_PORT_CONTROL_IS_MAC_TABLE_AND_BPDU_BASED(sm->port_info->port_control) && !is_req_identity_sm) {
        BVSET(skipFirstEapolTx);
        // Gotta step the SM until it reaches a stable state so that it's ready to accept the up-coming
        // EAPOL frame from the supplicant.
        auth_sm_step(sm);
    } else {
        BVCLR(skipFirstEapolTx);
    }
}

/******************************************************************************/
// IEEE8021X_uninit_sm()
// Pre-warning that this SM is going to be freed in a split second.
/******************************************************************************/
static void IEEE8021X_uninit_sm(nas_sm_t *sm)
{
    if (NAS_PORT_CONTROL_IS_BPDU_BASED(sm->port_info->port_control)) {
        nas_os_backend_server_free_resources(sm);

#ifdef VTSS_SW_OPTION_DOT1X_ACCT
        nas_os_acct_sm_release(sm);
#endif

        nas_os_freeing_sm(sm);
    }
}

/******************************************************************************/
/******************************************************************************/
static void IEEE8021X_reauth_param_changed(nas_sm_t *top_sm, BOOL ena, u16 reauth_period)
{
    nas_sm_t *sm;

    for (sm = top_sm->next; sm; sm = sm->next) {
        if (ena) {
            BVSET(reAuthEnabled);
            AUTH_SM(sm)->timers.reAuthWhen = reauth_period;
        } else {
            BVCLR(reAuthEnabled);
            AUTH_SM(sm)->timers.reAuthWhen = 0;
        }
    }
}

/******************************************************************************/
/******************************************************************************/
static void IEEE8021X_reauthenticate(nas_sm_t *top_sm)
{
    nas_sm_t *sm;

    // Reauthenticate all supplicants on the port
    for (sm = top_sm->next; sm; sm = sm->next) {
        BVSET(reAuthenticate);
    }
}

/******************************************************************************/
// IEEE8021X_reinitialize()
// Causes a reauthentication NOW.
/******************************************************************************/
static void IEEE8021X_reinitialize(nas_sm_t *top_sm)
{
    nas_sm_t *sm;

    // Initialize the state machines by asserting initialize and then
    // deasserting it after one step
    // Prior to stepping the SM, change the portMode to something different from
    // the current port_control, or it may go wrong in the SM start-up phase.
    for (sm = top_sm->next; sm; sm = sm->next) {
        AUTH_SM(sm)->auth_pae.portMode = NAS_PORT_CONTROL_DISABLED;
        BVSET(initialize);
        auth_sm_step(sm);
        BVCLR(initialize);
        auth_sm_step(sm);
    }
}

/******************************************************************************/
/******************************************************************************/
static nas_sm_t *IEEE8021X_get_sm_from_radius_handle(nas_sm_t *top_sm, int radius_handle)
{
    nas_sm_t *sm;
    for (sm = top_sm->next; sm; sm = sm->next) {
        if (AUTH_SM(sm)->eap_info.radius_handle == radius_handle) {
            return sm;
        }
    }
    return NULL;
}

/******************************************************************************/
// IEEE8021X_backend_server_timeout_occurred()
/******************************************************************************/
static void IEEE8021X_backend_server_timeout_occurred(nas_sm_t *sm)
{
    AUTH_SM(sm)->timers.aWhile = 0;
    auth_sm_step(sm);
}

/******************************************************************************/
// IEEE8021X_get_next_eap_identifier()
/******************************************************************************/
static inline u8 IEEE8021X_get_next_eap_identifier(nas_sm_t *sm)
{
    if (BVTST(isReqIdentitySM)) {
        // When we're a request identity SM, the last_eap_tx_identifier
        // must be taken from the Top SM, so that it can be reused
        // by newly allocated sub SMs (this is a requirement for
        // Multi 802.1X modes). If we didn't re-use it, then the second
        // supplicant's response would be rejected because the identifier
        // in the Response Identity frame didn't match the identifier
        // from the Request Identity frame we sent out while being a
        // Request Identity SM.
        return sm->port_info->top_sm->last_eap_tx_identifier + 1;
    } else {
        return sm->last_eap_tx_identifier + 1;
    }
}

/******************************************************************************/
// IEEE8021X_new_auth_session()
/******************************************************************************/
static void IEEE8021X_new_auth_session(nas_sm_t *sm)
{
    nas_eap_info_t *eap_info = &AUTH_SM(sm)->eap_info;

    BVCLR(eapSuccess);
    BVCLR(eapFail);

    // No state for initial request/identity packet
    eap_info->radius_state_attribute_len = 0;
}

/******************************************************************************/
// IEEE8021X_get_port_status()
/******************************************************************************/
static nas_port_status_t IEEE8021X_get_port_status(nas_sm_t *top_sm)
{
#ifdef VTSS_SW_OPTION_NAS_DOT1X_MULTI
    nas_sm_t *sm;
    if (NAS_PORT_CONTROL_IS_MULTI_CLIENT(top_sm->port_info->port_control)) {
        u32 auth_cnt = 0, unauth_cnt = 0;

        // We don't have one single port-status, so we loop through all of our
        // clients and return ((auth_cnt + 1) << 16) | (unauth_cnt + 1)
        for (sm = top_sm->next; sm; sm = sm->next) {
            if (AUTH_SM(sm)->authStatus == NAS_PORT_STATUS_AUTHORIZED) {
                auth_cnt++;
            } else {
                unauth_cnt++;
            }
        }
        return nas_os_encode_auth_unauth(auth_cnt, unauth_cnt);
    } else
#endif
    {
        if (top_sm->next == NULL) {
            // Could be a possibility for Single 802.1X SMs.
            return NAS_PORT_STATUS_UNAUTHORIZED;
        } else {
            return AUTH_SM(top_sm->next)->authStatus;
        }
    }
}

/******************************************************************************/
// IEEE8021X_get_status()
/******************************************************************************/
static nas_port_status_t IEEE8021X_get_status(nas_sm_t *sub_sm)
{
    return AUTH_SM(sub_sm)->authStatus;
}

/******************************************************************************/
// IEEE8021X_get_eapol_counters()
/******************************************************************************/
static nas_eapol_counters_t *IEEE8021X_get_eapol_counters(nas_sm_t *top_or_sub_sm)
{
    if (top_or_sub_sm->sm_type == NAS_SM_TYPE_TOP) {
        // Get summed EAPOL counters
        return top_or_sub_sm->port_info->eapol_counters;
    } else {
        return &AUTH_SM(top_or_sub_sm)->eapol_counters;
    }
}

/******************************************************************************/
// IEEE8021X_get_backend_counters()
// If SM is top_sm, get overall counters for that port, otherwise get for
// that SM.
/******************************************************************************/
static nas_backend_counters_t *IEEE8021X_get_backend_counters(nas_sm_t *top_or_sub_sm)
{
    if (top_or_sub_sm->sm_type == NAS_SM_TYPE_TOP) {
        // Get summed backend counters
        return top_or_sub_sm->port_info->backend_counters;
    } else {
        return &AUTH_SM(top_or_sub_sm)->eap_info.backend_counters;
    }
}

/******************************************************************************/
// IEEE8021X_eapol_frame_received()
/******************************************************************************/
static void IEEE8021X_eapol_frame_received(nas_sm_t *top_or_sub_sm, u8 *frame, vtss_vid_t vid, size_t len)
{
    nas_sm_t                     *sm;
    nas_ieee8021x_eapol_packet_t packet = {0};
    u8                           i;
    vtss_vid_mac_t               vid_mac;

    if (top_or_sub_sm->sm_type == NAS_SM_TYPE_TOP) {
        sm = top_or_sub_sm->next;
        if (!sm) {
            T_EG(TRACE_GRP_BASE, "%u: next_ptr is NULL", top_or_sub_sm->port_info->port_no);
            return;
        }
    } else {
        sm = top_or_sub_sm;
    }

    // Check frame size
    if (len < 18) {
        AUTH_EAPOL_COUNTER_INCREMENT(dot1xAuthEapLengthErrorFramesRx);
        return;
    }

    AUTH_EAPOL_COUNTER_INCREMENT(dot1xAuthEapolFramesRx);

    // Point to SRC MAC
    frame = &frame[6];

    for (i = 0; i < 6 ; i++) {
        vid_mac.mac.addr[i] = *(frame++);
    }
    vid_mac.vid = vid;

    IEEE8021X_update_client_info(sm, &vid_mac, FALSE);

    // Skip past eth type field (as it had already been checked) and point to EAP packet
    frame += 2;

    // Fill in packet structure
    packet.protocol_version = *frame++;
    packet.packet_type      = *frame++;

    AUTH_EAPOL_COUNTER_SET(dot1xAuthLastEapolFrameVersion, packet.protocol_version);

    // Only fill in body & length if not start/logoff packet
    if (packet.packet_type != NAS_IEEE8021X_TYPE_EAPOL_START &&
        packet.packet_type != NAS_IEEE8021X_TYPE_EAPOL_LOGOFF) {
        packet.body_length = ((u16)frame[0]) << 8 | frame[1];
        packet.body        = (u8 *)&frame[2];

        if (packet.body_length > len - 18) {
            // The EAPOL frame indicates that the length of the embedded EAP packet
            // is longer than the length of the frame on the wire.
            AUTH_EAPOL_COUNTER_INCREMENT(dot1xAuthEapLengthErrorFramesRx);
            return;
        }
    }

    // As version 2 and version 1 of EAPOL PDU is essentially the same, we can ignore the
    // version check, see note 2 in 7.5.5 in IEEE-802.1X-Rev-D11 */

    T_DG(TRACE_GRP_BASE, "%u: EAPOL frame, len %zu, src=%s", sm->port_info->port_no, len, sm->client_info.mac_addr_str);
    AUTH_print_eapol_packet(&packet, sm->port_info->port_no);

    // Process packet
    switch (packet.packet_type) {
    case NAS_IEEE8021X_TYPE_EAP_PACKET:
        if (AUTH_handle_eap(sm, (nas_ieee8021x_eap_packet_hdr_t *)packet.body, packet.body_length) == FALSE) {
            AUTH_EAPOL_COUNTER_INCREMENT(dot1xAuthEapLengthErrorFramesRx);
        }
        auth_sm_step(sm);
        break;

    case NAS_IEEE8021X_TYPE_EAPOL_START:
        T_DG(TRACE_GRP_BASE, "%u: EAPOL Start received", sm->port_info->port_no);
        AUTH_EAPOL_COUNTER_INCREMENT(dot1xAuthEapolStartFramesRx);
        BVSET(eapolStart);
        auth_sm_step(sm);
        break;

    case NAS_IEEE8021X_TYPE_EAPOL_LOGOFF:
        T_DG(TRACE_GRP_BASE, "%u: EAPOL Logoff received", sm->port_info->port_no);
        AUTH_EAPOL_COUNTER_INCREMENT(dot1xAuthEapolLogoffFramesRx);
        BVSET(eapolLogoff);
        auth_sm_step(sm);
        break;

    case NAS_IEEE8021X_TYPE_EAPOL_KEY:
        // EAPOL Keys are ignored
        break;

    case NAS_IEEE8021X_TYPE_EAPOL_ENCAPSULATED_ASF_ALERT:
        // EAPOL Encapsulated ASF alerts are ignored
        break;

    default:
        AUTH_EAPOL_COUNTER_INCREMENT(dot1xAuthInvalidEapolFramesRx);
        // Debug only
        T_DG(TRACE_GRP_BASE, "%u: Invalid EAPOL packet received", sm->port_info->port_no);
        break;
    }
}

/******************************************************************************/
// IEEE8021X_fake_force_authorized()
/******************************************************************************/
static void IEEE8021X_fake_force_authorized(nas_sm_t *sm, BOOL ena)
{
#ifdef VTSS_SW_OPTION_NAS_GUEST_VLAN
    if (ena) {
        BVSET(fakeForceAuthorized);
        auth_cancel_timeout(sm, NAS_IEEE8021X_TIMEOUT_ALL_TIMEOUTS);
    } else {
        BVCLR(fakeForceAuthorized);
        // When disabling, we should skip the first EAPOL request identity SM
        // because the reason for going out of fakeForceAuthorized mode is
        // that we've just received an EAPOL frame, which we will be invoked
        // with in a split-second.
        // Another reason for calling this function is because the port control
        // changes, so that this SM will get replaced by another SM in just a second,
        // so in that case we also shouldn't send the first Request Identity SM.
        // The final reason for calling this function is that Guest VLAN gets
        // disabled on this port. In this case, we live with the fact that we
        // don't send EAPOL Request Identity frames the first X seconds.
        BVSET(skipFirstEapolTx);

        // Take it out of the fakeForceAuthorized mode by initializing the SM
        BVSET(initialize);
        auth_sm_step(sm);
        BVCLR(initialize);
    }

    // Prepare for next time.
    AUTH_SM(sm)->auth_pae.reAuthCount = 0;

    // This will cause the SM to enter the correct mode (FAKE_FORCE_AUTH or RESTART).
    auth_sm_step(sm);
#endif
}

/******************************************************************************/
// IEEE8021X_reset_reauth_cnt()
/******************************************************************************/
static void IEEE8021X_reset_reauth_cnt(nas_sm_t *sm)
{
    AUTH_SM(sm)->auth_pae.reAuthCount = 0;
}

/******************************************************************************/
// IEEE8021X_get_eap_info()
// sm is never a top_sm.
/******************************************************************************/
static nas_eap_info_t *IEEE8021X_get_eap_info(nas_sm_t *sub_sm)
{
    return &AUTH_SM(sub_sm)->eap_info;
}

/******************************************************************************/
// AUTH_create_canned_eap()
/******************************************************************************/
static inline void AUTH_create_canned_eap(nas_sm_t *sm, BOOL success)
{
    nas_eap_info_t *eap_info = &AUTH_SM(sm)->eap_info;
    u8             *eap;

    eap      = eap_info->last_frame;
    *(eap++) = success ? EAP_CODE_SUCCESS : EAP_CODE_FAILURE;
    *(eap++) = IEEE8021X_get_next_eap_identifier(sm);

    // Length is 4 bytes for header
    *(eap++) = 0;
    *(eap++) = 4;

    eap_info->last_frame_len = 4;

    // Register this frame type
    eap_info->last_frame_type = FRAME_TYPE_NO_RETRANSMISSION;
}

/******************************************************************************/
//
//  Public functions
//
/******************************************************************************/

/******************************************************************************/
// auth_request_identity()
/******************************************************************************/
void auth_request_identity(nas_sm_t *sm)
{
    nas_eap_info_t *eap_info = &AUTH_SM(sm)->eap_info;
    u8             *eap;

    // Clear any flags from last time around (if reauthentication).
    BVCLR(eapResponseIdentity);

    eap = eap_info->last_frame;

    IEEE8021X_new_auth_session(sm);

#ifdef VTSS_SW_OPTION_DOT1X_ACCT
    if (eap_info->acct.session_id == 0) {
        eap_info->acct.session_id = NAS_new_session_id();
    }
#endif

    *eap++ = EAP_CODE_REQUEST;
    *eap++ = IEEE8021X_get_next_eap_identifier(sm);

    // Length is 4 bytes for header + 1 byte for code
    *eap++ = 0;
    *eap++ = 5;

    *eap = (u8)EAP_TYPE_IDENTITY;

    eap_info->last_frame_len = 5;

    // Register this frame type
    eap_info->last_frame_type = FRAME_TYPE_EAPOL;

    BVSET(eapReq);
    BVCLR(eapRestart);

    // Set eapTimeoutTimer timeout
    auth_start_timeout(sm, NAS_IEEE8021X_TIMEOUT_EAPOL_TIMEOUT);
}

/******************************************************************************/
// auth_tx_canned_eap()
// Called by auth_sm.c
/******************************************************************************/
void auth_tx_canned_eap(nas_sm_t *sm, BOOL success)
{
    AUTH_create_canned_eap(sm, success);
    AUTH_send(sm);
}

/******************************************************************************/
// auth_tx_req()
// Called by auth_sm.c
/******************************************************************************/
void auth_tx_req(nas_sm_t *sm)
{
    AUTH_send(sm);
}

/******************************************************************************/
// auth_start_timeout()
/******************************************************************************/
void auth_start_timeout(nas_sm_t *sm, nas_ieee8021x_timeouts_t timeout)
{
    switch (timeout) {
    case NAS_IEEE8021X_TIMEOUT_EAPOL_TIMEOUT:
        BVCLR(eapTimeout);
        AUTH_SM(sm)->admin.timeouts.eapTimeoutTimer    = nas_os_get_eapol_request_identity_timeout();
        break;

    case NAS_IEEE8021X_TIMEOUT_SUPPLICANT_RESPONSE_TIMEOUT:
        AUTH_SM(sm)->admin.timeouts.clientTimeoutTimer = nas_os_get_eapol_challenge_timeout();
        break;

    default:
        break;
    }
}

/******************************************************************************/
// auth_cancel_timeout()
/******************************************************************************/
void auth_cancel_timeout(nas_sm_t *sm, nas_ieee8021x_timeouts_t timeout)
{
    switch (timeout) {
    case NAS_IEEE8021X_TIMEOUT_EAPOL_TIMEOUT:
        BVCLR(eapTimeout);
        AUTH_SM(sm)->admin.timeouts.eapTimeoutTimer = 0;
        break;

    case NAS_IEEE8021X_TIMEOUT_SUPPLICANT_RESPONSE_TIMEOUT:
        AUTH_SM(sm)->admin.timeouts.clientTimeoutTimer = 0;
        break;

    case NAS_IEEE8021X_TIMEOUT_ALL_TIMEOUTS:
        BVCLR(eapTimeout);
        AUTH_SM(sm)->admin.timeouts.eapTimeoutTimer = 0;
        AUTH_SM(sm)->admin.timeouts.clientTimeoutTimer = 0;
        break;

    default:
        break;
    }
}

/******************************************************************************/
// auth_backend_send_response()
/******************************************************************************/
void auth_backend_send_response(nas_sm_t *sm)
{
    nas_eap_info_t    *eap_info    = &AUTH_SM(sm)->eap_info;
    nas_client_info_t *client_info = &sm->client_info;
    u16               len          = eap_info->last_frame_len;

    if (nas_os_backend_server_tx_request(sm,
                                         &eap_info->radius_handle,
                                         eap_info->last_frame,
                                         len,
                                         eap_info->radius_state_attribute,
                                         eap_info->radius_state_attribute_len,
                                         client_info->identity,
                                         sm->port_info->port_no,
                                         client_info->vid_mac.mac.addr)) {
        // Call to the RADIUS module succeeded. Since that
        // module is guaranteed to call us back with either
        // a response or a timeout, we shouldn't timeout
        // ourselves, but simply wait for that event.
        // Therefore we set the aWhile to a number so big
        // that it will never reach 0 before the RADIUS server
        // has called us back.
        AUTH_SM(sm)->timers.aWhile = 0xFFFFFFFFUL;
    } else {
        // The RADIUS module is not ready yet, or no
        // RADIUS servers are enabled/configured.
        // Try again in 15 seconds.
        AUTH_SM(sm)->timers.aWhile = 15;
    }

    BVCLR(eapResp);
}

/******************************************************************************/
// IEEE8021X_backend_frame_received()
/******************************************************************************/
static void IEEE8021X_backend_frame_received(nas_sm_t *sm, nas_backend_code_t code)
{
    if (code == NAS_BACKEND_CODE_ACCESS_CHALLENGE) {
        auth_start_timeout(sm, NAS_IEEE8021X_TIMEOUT_EAPOL_TIMEOUT);
        if (AUTH_SM(sm)->eap_info.last_frame_len != 0) {
            BVSET(eapReq);
        } else {
            BVSET(eapNoReq);
        }
    } else {
        auth_cancel_timeout(sm, NAS_IEEE8021X_TIMEOUT_EAPOL_TIMEOUT);
        if (code == NAS_BACKEND_CODE_ACCESS_ACCEPT) {
            BVSET(eapSuccess);
        } else {
            /* code == NAS_BACKEND_ACCESS_REJECT */
            BVSET(eapFail);
        }
    }

    // Step State machine
    auth_sm_step(sm);
}

/******************************************************************************/
// Function pointers passed to the NAS umbrella.
// We really don't have to fill in all the unsupported functions with NULL.
// The reason I've done it is to check that I've implemented all.
/******************************************************************************/
static const nas_funcs_t IEEE8021X_funcs = {
    // All SMs:
    .init                            = IEEE8021X_init,
    .init_sm                         = IEEE8021X_init_sm,
    .uninit_sm                       = IEEE8021X_uninit_sm,
    .clear_statistics                = IEEE8021X_clear_statistics,
    .timer_tick                      = IEEE8021X_timer_tick,
    .get_port_status                 = IEEE8021X_get_port_status,
    .get_status                      = IEEE8021X_get_status,
    .set_status                      = NULL,

    // IEEE8021X SMs:
    .get_eapol_counters              = IEEE8021X_get_eapol_counters,
    .ieee8021x_eapol_frame_received  = IEEE8021X_eapol_frame_received,
    .fake_force_authorized           = IEEE8021X_fake_force_authorized,
    .reset_reauth_cnt                = IEEE8021X_reset_reauth_cnt,

    // Authentication SMs:
    .reauth_param_changed            = IEEE8021X_reauth_param_changed,
    .reauthenticate                  = IEEE8021X_reauthenticate,
    .reinitialize                    = IEEE8021X_reinitialize,
    .get_sm_from_radius_handle       = IEEE8021X_get_sm_from_radius_handle,
    .backend_frame_received          = IEEE8021X_backend_frame_received,
    .backend_server_timeout_occurred = IEEE8021X_backend_server_timeout_occurred,
    .get_backend_counters            = IEEE8021X_get_backend_counters,
    .get_eap_info                    = IEEE8021X_get_eap_info,
};

/******************************************************************************/
// ieee8021x_get_funcs()
// Called by the NAS umbrella to get our function pointers.
/******************************************************************************/
nas_funcs_t const *ieee8021x_get_funcs(void)
{
    return &IEEE8021X_funcs;
}

