/*
 * EAP peer state machines (RFC 4137)
 * Copyright (c) 2004-2007, Jouni Malinen <j@w1.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 *
 * This file implements the Peer State Machine as defined in RFC 4137. The used
 * states and state transitions match mostly with the RFC. However, there are
 * couple of additional transitions for working around small issues noticed
 * during testing. These exceptions are explained in comments within the
 * functions in this file. The method functions, m.func(), are similar to the
 * ones used in RFC 4137, but some small changes have used here to optimize
 * operations and to add functionality needed for fast re-authentication
 * (session resumption).
 */

// Original file comes from wpa_supplicant-0.6.1/src/eap_peer/eap.c

#include "vtss_md5_api.h"
#include "peer.h"
#include "peer_sm.h"
#include "peer_common.h"
#include <network.h>

#define setAuthorized()                                                \
  do {                                                                 \
    BOOL chgd = PEER_SM(sm)->authStatus != NAS_PORT_STATUS_AUTHORIZED; \
    PEER_SM(sm)->authStatus = NAS_PORT_STATUS_AUTHORIZED;              \
    nas_os_set_authorized(sm, TRUE, chgd);                             \
  } while (0);


#define setUnauthorized()                                                \
  do {                                                                   \
    BOOL chgd = PEER_SM(sm)->authStatus != NAS_PORT_STATUS_UNAUTHORIZED; \
    PEER_SM(sm)->authStatus = NAS_PORT_STATUS_UNAUTHORIZED;              \
    nas_os_set_authorized(sm, FALSE, chgd);                              \
  } while (0);

/**
 * SM_STATE - Declaration of a state machine function
 * @machine: State machine name
 * @state: State machine state
 *
 * This macro is used to declare a state machine function. It is used in place
 * of a C function definition to declare functions to be run when the state is
 * entered by calling SM_ENTER or SM_ENTER_GLOBAL.
 */
#define SM_STATE(machine, state) static void sm_ ## machine ## _ ## state ## _Enter(nas_sm_t *sm, int global)

/**
 * SM_ENTRY - State machine function entry point
 * @machine: State machine name
 * @state: State machine state
 *
 * This macro is used inside each state machine function declared with
 * SM_STATE. SM_ENTRY should be in the beginning of the function body, but
 * after declaration of possible local variables. This macro prints debug
 * information about state transition and update the state machine state.
 */
#define SM_ENTRY(machine, state)                                            \
  if (!global || PEER_SM(sm)->machine ## _state != machine ## _ ## state) { \
    PEER_SM(sm)->changed = TRUE;                                            \
    T_NG(TRACE_GRP_BASE, "EAP: " #machine " entering State " #state);       \
  }                                                                         \
  PEER_SM(sm)->machine ## _state = machine ## _ ## state;

/**
 * SM_ENTER - Enter a new state machine state
 * @machine: State machine name
 * @state: State machine state
 *
 * This macro expands to a function call to a state machine function defined
 * with SM_STATE macro. SM_ENTER is used in a state machine step function to
 * move the state machine to a new state.
 */
#define SM_ENTER(machine, state) sm_ ## machine ## _ ## state ## _Enter(sm, 0)

/**
 * SM_ENTER_GLOBAL - Enter a new state machine state based on global rule
 * @machine: State machine name
 * @state: State machine state
 *
 * This macro is like SM_ENTER, but this is used when entering a new state
 * based on a global (not specific to any particular state) rule. A separate
 * macro is used to avoid unwanted debug message floods when the same global
 * rule is forcing a state machine to remain in on state.
 */
#define SM_ENTER_GLOBAL(machine, state) sm_ ## machine ## _ ## state ## _Enter(sm, 1)

/**
 * SM_STEP - Declaration of a state machine step function
 * @machine: State machine name
 *
 * This macro is used to declare a state machine step function. It is used in
 * place of a C function definition to declare a function that is used to move
 * state machine to a new state based on state variables. This function uses
 * SM_ENTER and SM_ENTER_GLOBAL macros to enter new state.
 */
#define SM_STEP(machine) static void sm_ ## machine ## _Step(nas_sm_t *sm)

/**
 * SM_STEP_RUN - Call the state machine step function
 * @machine: State machine name
 *
 * This macro expands to a function call to a state machine step function
 * defined with SM_STEP macro.
 */
#define SM_STEP_RUN(machine) sm_ ## machine ## _Step(sm)

#define EAP_MAX_AUTH_ROUNDS 50

/******************************************************************************/
// get_eapReqData()
/******************************************************************************/
static u8 *get_eapReqData(nas_sm_t *sm, size_t *len)
{
    nas_eap_info_t *eap_info = &PEER_SM(sm)->eap_info;
    *len = eap_info->last_frame_len;
    return eap_info->last_frame;
}

static u8 *eap_sm_build_expanded_nak(nas_sm_t *sm, int id, size_t *len, const struct eap_method *methods, size_t count)
{
    struct eap_hdr *resp;
    u8 *pos;

    T_DG(TRACE_GRP_BASE, "%u: EAP: Building expanded EAP-Nak", sm->port_info->port_no);

    /* RFC 3748 - 5.3.2: Expanded Nak */
    *len = sizeof(struct eap_hdr) + 8;
    resp = (struct eap_hdr *)PEER_SM(sm)->eap_info.last_frame;
    if (resp == NULL) {
        return NULL;
    }

    resp->code = EAP_CODE_RESPONSE;
    resp->identifier = id;
    pos = (u8 *)(resp + 1);
    *(pos++) = (u8)EAP_TYPE_EXPANDED;
    WPA_PUT_BE24(pos, EAP_VENDOR_IETF);
    pos += 3;
    WPA_PUT_BE32(pos, EAP_TYPE_NAK);
    pos += 4;

    T_DG(TRACE_GRP_BASE, "%u: EAP: no more allowed methods", sm->port_info->port_no);
    *(pos++) = (u8)EAP_TYPE_EXPANDED;
    WPA_PUT_BE24(pos, EAP_VENDOR_IETF);
    pos += 3;
    WPA_PUT_BE32(pos, EAP_TYPE_NONE);

    (*len) += 8;

    resp->length = htons(*len);

    return (u8 *)resp;
}

static u8 *eap_sm_buildNak(nas_sm_t *sm, int id, size_t *len)
{
    struct eap_hdr *resp;
    u8 *pos;
    int found = 0, expanded_found = 0;
    size_t count;
    const struct eap_method *methods, *m;

    T_DG(TRACE_GRP_BASE, "%u: EAP: Building EAP-Nak (requested type %u vendor=%u method=%u not allowed)", sm->port_info->port_no, PEER_SM(sm)->reqMethod, PEER_SM(sm)->reqVendor, PEER_SM(sm)->reqVendorMethod);
    methods = eap_peer_get_methods(&count);
    if (methods == NULL) {
        return NULL;
    }
    if (PEER_SM(sm)->reqMethod == EAP_TYPE_EXPANDED) {
        return eap_sm_build_expanded_nak(sm, id, len, methods, count);
    }

    /* RFC 3748 - 5.3.1: Legacy Nak */
    *len = sizeof(struct eap_hdr) + 1;
    resp = (struct eap_hdr *)PEER_SM(sm)->eap_info.last_frame;
    if (resp == NULL) {
        return NULL;
    }

    resp->code = EAP_CODE_RESPONSE;
    resp->identifier = id;
    pos = (u8 *)(resp + 1);
    *(pos++) = (u8)EAP_TYPE_NAK;

    for (m = methods; m; m = m->next) {
        if (m->vendor == EAP_VENDOR_IETF && m->method == PEER_SM(sm)->reqMethod) {
            continue;    // Do not allow the current method again
        }
        if (m->vendor != EAP_VENDOR_IETF) {
            if (expanded_found) {
                continue;
            }
            expanded_found = 1;
            *(pos++) = (u8)EAP_TYPE_EXPANDED;
        } else {
            *(pos++) = (u8)m->method;
        }
        (*len)++;
        found++;
    }

    if (!found) {
        *pos = (u8)EAP_TYPE_NONE;
        (*len)++;
    }
    T_DG(TRACE_GRP_BASE, "%u: EAP: allowed methods:", sm->port_info->port_no);
    T_DG_HEX(TRACE_GRP_BASE, ((u8 *)(resp + 1)) + 1, found);

    resp->length = htons(*len);

    return (u8 *)resp;
}


static void eap_sm_processIdentity(nas_sm_t *sm, const u8 *req)
{
    const struct eap_hdr *hdr = (const struct eap_hdr *) req;
    const u8 *pos = (const u8 *)(hdr + 1);
    pos++;

    T_DG(TRACE_GRP_BASE, "%u: CTRL-EVENT-EAP-STARTED EAP authentication started", sm->port_info->port_no);

    /*
     * RFC 3748 - 5.1: Identity
     * Data field may contain a displayable message in UTF-8. If this
     * includes NUL-character, only the data before that should be
     * displayed. Some EAP implementasitons may piggy-back additional
     * options after the NUL.
     */
    /* TODO: could save displayable message so that it can be shown to the
     * user in case of interaction is required */
    T_DG(TRACE_GRP_BASE, "%u: EAP: EAP-Request Identity data:", sm->port_info->port_no);
    T_DG_HEX(TRACE_GRP_BASE, pos, ntohs(hdr->length) - 5);
}

/**
 * nas_sm_buildIdentity - Build EAP-Identity/Response for the current network
 * @sm: Pointer to EAP state machine allocated with peer_sm_init()
 * @id: EAP identifier for the packet
 * @len: Pointer to a variable that will be set to the length of the response
 * @encrypted: Whether the packet is for encrypted tunnel (EAP phase 2)
 * Returns: Pointer to the allocated EAP-Identity/Response packet or %NULL on
 * failure
 *
 * This function allocates and builds an EAP-Identity/Response packet for the
 * current network. The caller is responsible for freeing the returned data.
 */
static u8 *eap_sm_buildIdentity(nas_sm_t *sm, int id, size_t *len, int encrypted)
{
//  struct wpa_ssid *config = eap_get_config(sm);
    struct eap_hdr *resp;
    u8             *pos;
    size_t         identity_len;

    // Vitesse:
    // The identity is always given by the MAC address
    identity_len = strlen(sm->client_info.identity);

    // 4 + 1 (EAP Type) + user-name
    *len = sizeof(struct eap_hdr) + 1 + identity_len;

    // Allocate buffer possibly enabling more_work flag for immediate UDP transmission
    resp = (struct eap_hdr *)PEER_SM(sm)->eap_info.last_frame;
    if (resp == NULL) {
        return NULL;
    }

    resp->code = EAP_CODE_RESPONSE;
    resp->identifier = id;
    resp->length = htons(*len);
    pos = (u8 *) (resp + 1);
    *(pos++) = (u8)EAP_TYPE_IDENTITY;
    memcpy(pos, sm->client_info.identity, identity_len);
    return (u8 *) resp;
}

static void eap_sm_processNotify(nas_sm_t *sm, const u8 *req)
{
    const struct eap_hdr *hdr = (const struct eap_hdr *)req;
    const u8             *pos;
    size_t               msg_len;

    pos = (const u8 *)(hdr + 1);
    pos++;

    msg_len = ntohs(hdr->length);
    if (msg_len < 5) {
        return;
    }
    msg_len -= 5;
    T_DG(TRACE_GRP_BASE, "%u: EAP: EAP-Request Notification data", sm->port_info->port_no);
    T_DG_HEX(TRACE_GRP_BASE, pos, msg_len);
}

static u8 *eap_sm_buildNotify(nas_sm_t *sm, int id, size_t *len)
{
    struct eap_hdr *resp;
    u8             *pos;

    T_DG(TRACE_GRP_BASE, "%u: EAP: Generating EAP-Response Notification", sm->port_info->port_no);
    *len = sizeof(struct eap_hdr) + 1;
    resp = (struct eap_hdr *)PEER_SM(sm)->eap_info.last_frame;
    if (resp == NULL) {
        return NULL;
    }

    resp->code = EAP_CODE_RESPONSE;
    resp->identifier = id;
    resp->length = htons(*len);
    pos = (u8 *) (resp + 1);
    *pos = (u8)EAP_TYPE_NOTIFICATION;

    return (u8 *)resp;
}

static void eap_sm_parseEapReq(nas_sm_t *sm, const u8 *req, size_t len)
{
    const struct eap_hdr *hdr;
    size_t               plen;
    const u8             *pos;

    PEER_SM(sm)->rxReq = PEER_SM(sm)->rxResp = PEER_SM(sm)->rxSuccess = PEER_SM(sm)->rxFailure = FALSE;
    PEER_SM(sm)->reqId = 0;
    PEER_SM(sm)->reqMethod = EAP_TYPE_NONE;
    PEER_SM(sm)->reqVendor = EAP_VENDOR_IETF;
    PEER_SM(sm)->reqVendorMethod = (u32)EAP_TYPE_NONE;

    if (req == NULL || len < sizeof(*hdr)) {
        return;
    }

    hdr = (const struct eap_hdr *) req;
    plen = ntohs(hdr->length);
    if (plen > len) {
        T_DG(TRACE_GRP_BASE, "%u: EAP: Ignored truncated EAP-Packet (len=%zu plen=%zu)", sm->port_info->port_no, len, plen);
        return;
    }

    PEER_SM(sm)->reqId = hdr->identifier;

    if (PEER_SM(sm)->workaround) {
        vtss_md5_vector(1, (const u8 **)&req, &plen, PEER_SM(sm)->req_md5);
    }

    switch (hdr->code) {
    case EAP_CODE_REQUEST:
        if (plen < sizeof(*hdr) + 1) {
            T_DG(TRACE_GRP_BASE, "%u: EAP: Too short EAP-Request - no Type field", sm->port_info->port_no);
            return;
        }
        PEER_SM(sm)->rxReq = TRUE;
        pos = (const u8 *)(hdr + 1);
        PEER_SM(sm)->reqMethod = (EapType)(*(pos++));
        if (PEER_SM(sm)->reqMethod == EAP_TYPE_EXPANDED) {
            if (plen < sizeof(*hdr) + 8) {
                T_DG(TRACE_GRP_BASE, "%u: EAP: Ignored truncated expanded EAP-Packet (plen=%zu)", sm->port_info->port_no, plen);
                return;
            }
            PEER_SM(sm)->reqVendor = WPA_GET_BE24(pos);
            pos += 3;
            PEER_SM(sm)->reqVendorMethod = WPA_GET_BE32(pos);
        }
        T_DG(TRACE_GRP_BASE, "%u: EAP: Received EAP-Request id=%d method=%u vendor=%u vendorMethod=%u", sm->port_info->port_no, PEER_SM(sm)->reqId, PEER_SM(sm)->reqMethod, PEER_SM(sm)->reqVendor, PEER_SM(sm)->reqVendorMethod);
        break;
    case EAP_CODE_RESPONSE:
        if (PEER_SM(sm)->selectedMethod == EAP_TYPE_LEAP) {
            /*
             * LEAP differs from RFC 4137 by using reversed roles
             * for mutual authentication and because of this, we
             * need to accept EAP-Response frames if LEAP is used.
             */
            if (plen < sizeof(*hdr) + 1) {
                T_DG(TRACE_GRP_BASE, "%u: EAP: Too short EAP-Response - no Type field", sm->port_info->port_no);
                return;
            }
            PEER_SM(sm)->rxResp = TRUE;
            pos = (const u8 *)(hdr + 1);
            PEER_SM(sm)->reqMethod = (EapType)(*pos);
            T_DG(TRACE_GRP_BASE, "%u: EAP: Received EAP-Response for LEAP method=%d id=%d", sm->port_info->port_no, PEER_SM(sm)->reqMethod, PEER_SM(sm)->reqId);
            break;
        }
        T_DG(TRACE_GRP_BASE, "%u: EAP: Ignored EAP-Response", sm->port_info->port_no);
        break;
    case EAP_CODE_SUCCESS:
        T_DG(TRACE_GRP_BASE, "%u: EAP: Received EAP-Success", sm->port_info->port_no);
        PEER_SM(sm)->rxSuccess = TRUE;
        break;
    case EAP_CODE_FAILURE:
        T_DG(TRACE_GRP_BASE, "%u: EAP: Received EAP-Failure", sm->port_info->port_no);
        PEER_SM(sm)->rxFailure = TRUE;
        break;
    default:
        T_DG(TRACE_GRP_BASE, "%u: EAP: Ignored EAP-Packet with unknown code %d", sm->port_info->port_no, hdr->code);
        break;
    }
}

static void eap_deinit_prev_method(nas_sm_t *sm, const char *txt)
{
    if (PEER_SM(sm)->m == NULL || PEER_SM(sm)->eap_method_priv == NULL) {
        return;
    }

    T_DG(TRACE_GRP_BASE, "%u: EAP: deinitialize previously used EAP method (%d, %s) at %s", sm->port_info->port_no, PEER_SM(sm)->selectedMethod, PEER_SM(sm)->m->name, txt);
    PEER_SM(sm)->m->deinit(sm, PEER_SM(sm)->eap_method_priv);
    PEER_SM(sm)->eap_method_priv = NULL;
    PEER_SM(sm)->m = NULL;
}

static BOOL eap_sm_allowMethod(nas_sm_t *sm, int vendor, EapType method)
{
    if (eap_peer_get_eap_method(vendor, method)) {
        return TRUE;
    }
    T_DG(TRACE_GRP_BASE, "%u: EAP: not included in build: vendor %u method %u", sm->port_info->port_no, vendor, method);
    return FALSE;
}

/*
 * This state initializes state machine variables when the machine is
 * activated (EAPOL_portEnabled = TRUE). This is also used when re-starting
 * authentication (EAPOL_eapRestart == TRUE).
 */
SM_STATE(EAP, INITIALIZE)
{
    SM_ENTRY(EAP, INITIALIZE);
    if (PEER_SM(sm)->fast_reauth && PEER_SM(sm)->m && PEER_SM(sm)->m->has_reauth_data &&
        PEER_SM(sm)->m->has_reauth_data(sm, PEER_SM(sm)->eap_method_priv)) {
        T_DG(TRACE_GRP_BASE, "%u: EAP: maintaining EAP method data for fast reauthentication", sm->port_info->port_no);
        PEER_SM(sm)->m->deinit_for_reauth(sm, PEER_SM(sm)->eap_method_priv);
    } else {
        eap_deinit_prev_method(sm, "INITIALIZE");
    }
    PEER_SM(sm)->selectedMethod     = EAP_TYPE_NONE;
    PEER_SM(sm)->methodState        = METHOD_NONE;
    PEER_SM(sm)->allowNotifications = TRUE;
    PEER_SM(sm)->decision           = DECISION_FAIL;
    PEER_SM(sm)->EAPOL_idleWhile    = 0;
    if (PEER_SM(sm)->how_to_initialize != 2) {
        // In all cases but the Re-auth-case (i.e. first-time (just allocated) and re-init),
        // we reset these three variables.
        PEER_SM(sm)->EAPOL_eapSuccess = FALSE;
        PEER_SM(sm)->EAPOL_eapFail = FALSE;
        if (PEER_SM(sm)->how_to_initialize != 1) {
            // Don't change it here, since we need to update the chgd flag
            // in the call to setUnauthorized() below in the case where
            // we re-initialize.
            PEER_SM(sm)->authStatus = NAS_PORT_STATUS_UNAUTHORIZED;
        }
    }
    if (PEER_SM(sm)->how_to_initialize == 1) {
        // This is the re-init case (existing client getting re-initialized)
        PEER_SM(sm)->eap_info.stop_reason = NAS_STOP_REASON_INITIALIZING;
        setUnauthorized();
    }

    PEER_SM(sm)->EAPOL_eapRestart  = FALSE;
    PEER_SM(sm)->lastId            = -1; /* new session - make sure this does not match with the first EAP-Packet */

    /*
     * RFC 4137 does not reset EAPOL_eapResp and EAPOL_eapNoResp here. However, this
     * seemed to be able to trigger cases where both were set and if EAPOL
     * state machine uses EAPOL_eapNoResp first, it may end up not sending a real
     * reply correctly. This occurred when the workaround in FAIL state set
     * EAPOL_eapNoResp = TRUE. Maybe that workaround needs to be fixed to do
     * something else(?)
     */
    PEER_SM(sm)->EAPOL_eapResp   = FALSE;
    PEER_SM(sm)->EAPOL_eapNoResp = FALSE;
    PEER_SM(sm)->num_rounds      = 0;
}


/*
 * This state is reached whenever service from the lower layer is interrupted
 * or unavailable (EAPOL_portEnabled == FALSE). Immediate transition to INITIALIZE
 * occurs when the port becomes enabled.
 */
SM_STATE(EAP, DISABLED)
{
    SM_ENTRY(EAP, DISABLED);
    PEER_SM(sm)->num_rounds = 0;
}


/*
 * The state machine spends most of its time here, waiting for something to
 * happen. This state is entered unconditionally from INITIALIZE, DISCARD_IT, and
 * SEND_RESPONSE states.
 */
SM_STATE(EAP, IDLE)
{
    SM_ENTRY(EAP, IDLE);
}


/*
 * This state is entered when an EAP packet is received (EAPOL_eapReq == TRUE) to
 * parse the packet header.
 */
SM_STATE(EAP, RECEIVED)
{
    const u8 *eapReqData;
    size_t   eapReqDataLen;

    SM_ENTRY(EAP, RECEIVED);
    eapReqData = get_eapReqData(sm, &eapReqDataLen);
    /* parse rxReq, rxSuccess, rxFailure, reqId, reqMethod */
    eap_sm_parseEapReq(sm, eapReqData, eapReqDataLen);
    PEER_SM(sm)->num_rounds++;
}


/*
 * This state is entered when a request for a new type comes in. Either the
 * correct method is started, or a Nak response is built.
 */
SM_STATE(EAP, GET_METHOD)
{
    int reinit;
    EapType method;

    SM_ENTRY(EAP, GET_METHOD);

    if (PEER_SM(sm)->reqMethod == EAP_TYPE_EXPANDED) {
        method = (EapType)(PEER_SM(sm)->reqVendorMethod);
    } else {
        method = PEER_SM(sm)->reqMethod;
    }

    if (!eap_sm_allowMethod(sm, PEER_SM(sm)->reqVendor, method)) {
        T_DG(TRACE_GRP_BASE, "%u: EAP: vendor %u method %u not allowed", sm->port_info->port_no, PEER_SM(sm)->reqVendor, method);
        goto nak;
    }

    /*
     * RFC 4137 does not define specific operation for fast
     * re-authentication (session resumption). The design here is to allow
     * the previously used method data to be maintained for
     * re-authentication if the method support session resumption.
     * Otherwise, the previously used method data is freed and a new method
     * is allocated here.
     */
    if (PEER_SM(sm)->fast_reauth &&
        PEER_SM(sm)->m && PEER_SM(sm)->m->vendor == PEER_SM(sm)->reqVendor &&
        PEER_SM(sm)->m->method == method &&
        PEER_SM(sm)->m->has_reauth_data &&
        PEER_SM(sm)->m->has_reauth_data(sm, PEER_SM(sm)->eap_method_priv)) {
        T_DG(TRACE_GRP_BASE, "%u: EAP: Using previous method data for fast re-authentication", sm->port_info->port_no);
        reinit = 1;
    } else {
        eap_deinit_prev_method(sm, "GET_METHOD");
        reinit = 0;
    }

    PEER_SM(sm)->selectedMethod = PEER_SM(sm)->reqMethod;
    if (PEER_SM(sm)->m == NULL) {
        PEER_SM(sm)->m = eap_peer_get_eap_method(PEER_SM(sm)->reqVendor, method);
    }
    if (!PEER_SM(sm)->m) {
        T_DG(TRACE_GRP_BASE, "%u: EAP: Could not find selected method: vendor %d method %d", sm->port_info->port_no, PEER_SM(sm)->reqVendor, method);
        goto nak;
    }

    T_DG(TRACE_GRP_BASE, "%u: EAP: Initialize selected EAP method: vendor %u method %u (%s)", sm->port_info->port_no, PEER_SM(sm)->reqVendor, method, PEER_SM(sm)->m->name);
    if (reinit) {
        PEER_SM(sm)->eap_method_priv = PEER_SM(sm)->m->init_for_reauth(sm, PEER_SM(sm)->eap_method_priv);
    } else {
        PEER_SM(sm)->eap_method_priv = PEER_SM(sm)->m->init(sm);
    }

    if (PEER_SM(sm)->eap_method_priv == NULL) {
        T_DG(TRACE_GRP_BASE, "%u: EAP: Failed to initialize EAP method: vendor %u method %u (%s)", sm->port_info->port_no, PEER_SM(sm)->reqVendor, method, PEER_SM(sm)->m->name);
        PEER_SM(sm)->m = NULL;
        PEER_SM(sm)->methodState = METHOD_NONE;
        PEER_SM(sm)->selectedMethod = EAP_TYPE_NONE;
        goto nak;
    }

    PEER_SM(sm)->methodState = METHOD_INIT;
    T_DG(TRACE_GRP_BASE, "%u: EAP vendor %u method %u (%s) selected", sm->port_info->port_no, PEER_SM(sm)->reqVendor, method, PEER_SM(sm)->m->name);
    return;

nak:
    PEER_SM(sm)->eapRespData = eap_sm_buildNak(sm, PEER_SM(sm)->reqId, &PEER_SM(sm)->eapRespDataLen);
}


/*
 * The method processing happens here. The request from the authenticator is
 * processed, and an appropriate response packet is built.
 */
SM_STATE(EAP, METHOD)
{
    u8     *eapReqData;
    size_t eapReqDataLen;
    struct eap_method_ret ret;

    SM_ENTRY(EAP, METHOD);
    if (PEER_SM(sm)->m == NULL) {
        T_WG(TRACE_GRP_BASE, "%u: EAP::METHOD - method not selected", sm->port_info->port_no);
        return;
    }

    eapReqData = get_eapReqData(sm, &eapReqDataLen);

    /*
     * Get ignore, methodState, decision, allowNotifications, and
     * eapRespData. RFC 4137 uses three separate method procedure (check,
     * process, and buildResp) in this state. These have been combined into
     * a single function call to m->process() in order to optimize EAP
     * method implementation interface a bit. These procedures are only
     * used from within this METHOD state, so there is no need to keep
     * these as separate C functions.
     *
     * The RFC 4137 procedures return values as follows:
     * ignore = m.check(eapReqData)
     * (methodState, decision, allowNotifications) = m.process(eapReqData)
     * eapRespData = m.buildResp(reqId)
     */
    memset(&ret, 0, sizeof(ret));
    ret.ignore = PEER_SM(sm)->ignore;
    ret.methodState = PEER_SM(sm)->methodState;
    ret.decision = PEER_SM(sm)->decision;
    ret.allowNotifications = PEER_SM(sm)->allowNotifications;
    PEER_SM(sm)->eapRespData = PEER_SM(sm)->m->process(sm, PEER_SM(sm)->eap_method_priv, &ret, eapReqData, eapReqDataLen, &PEER_SM(sm)->eapRespDataLen);
    PEER_SM(sm)->ignore = ret.ignore;
    if (PEER_SM(sm)->ignore) {
        return;
    }
    PEER_SM(sm)->methodState = ret.methodState;
    PEER_SM(sm)->decision = ret.decision;
    PEER_SM(sm)->allowNotifications = ret.allowNotifications;
}

/*
 * This state signals the lower layer that a response packet is ready to be
 * sent.
 */
SM_STATE(EAP, SEND_RESPONSE)
{
    SM_ENTRY(EAP, SEND_RESPONSE);
    if (PEER_SM(sm)->eapRespData) {
        if (PEER_SM(sm)->workaround) {
            memcpy(PEER_SM(sm)->last_md5, PEER_SM(sm)->req_md5, 16);
        }
        PEER_SM(sm)->lastId = PEER_SM(sm)->reqId;
        PEER_SM(sm)->EAPOL_eapResp = TRUE;
        // In case of a retransmission, we may end up here without having
        // freed the previous backend server (e.g. RADIUS) resources. So we do it now.
        nas_os_backend_server_free_resources(sm);
        peer_backend_send_response(sm); // This functions sets the EAPOL_idleWhile member.
    }
    PEER_SM(sm)->EAPOL_eapReq = FALSE;
}

/*
 * This state signals the lower layer that the request was discarded, and no
 * response packet will be sent at this time.
 */
SM_STATE(EAP, DISCARD_IT)
{
    SM_ENTRY(EAP, DISCARD_IT);
    PEER_SM(sm)->EAPOL_eapReq = FALSE;
    PEER_SM(sm)->EAPOL_eapNoResp = TRUE;
}


/*
 * Handles requests for Identity method and builds a response.
 */
SM_STATE(EAP, IDENTITY)
{
    const u8 *eapReqData;
    size_t   eapReqDataLen;

    SM_ENTRY(EAP, IDENTITY);
    eapReqData = get_eapReqData(sm, &eapReqDataLen);
    eap_sm_processIdentity(sm, eapReqData);
    PEER_SM(sm)->eapRespData = eap_sm_buildIdentity(sm, PEER_SM(sm)->reqId, &PEER_SM(sm)->eapRespDataLen, 0);
}


/*
 * Handles requests for Notification method and builds a response.
 */
SM_STATE(EAP, NOTIFICATION)
{
    const u8 *eapReqData;
    size_t eapReqDataLen;

    SM_ENTRY(EAP, NOTIFICATION);
    eapReqData = get_eapReqData(sm, &eapReqDataLen);
    eap_sm_processNotify(sm, eapReqData);
    PEER_SM(sm)->eapRespData = eap_sm_buildNotify(sm, PEER_SM(sm)->reqId, &PEER_SM(sm)->eapRespDataLen);
}

/*
 * This state retransmits the previous response packet.
 */
SM_STATE(EAP, RETRANSMIT)
{
    SM_ENTRY(EAP, RETRANSMIT);
    if (PEER_SM(sm)->eap_info.last_frame_type != FRAME_TYPE_RADIUS) {
        // The buffer has been used for something else.
        PEER_SM(sm)->eapRespData = NULL;
    }
}

/*
 * This state is entered in case of a successful completion of authentication
 * and state machine waits here until port is disabled or EAP authentication is
 * restarted.
 */
SM_STATE(EAP, SUCCESS)
{
    SM_ENTRY(EAP, SUCCESS);
    PEER_SM(sm)->EAPOL_eapSuccess = TRUE;
    PEER_SM(sm)->reAuthWhen = nas_os_get_reauth_timer();
    setAuthorized();

    /*
     * RFC 4137 does not clear EAPOL_eapReq here, but this seems to be required
     * to avoid processing the same request twice when state machine is
     * initialized.
     */
    PEER_SM(sm)->EAPOL_eapReq = FALSE;

    /*
     * RFC 4137 does not set EAPOL_eapNoResp here, but this seems to be required
     * to get EAPOL Supplicant backend state machine into SUCCESS state. In
     * addition, either EAPOL_eapResp or EAPOL_eapNoResp is required to be set after
     * processing the received EAP frame.
     */
    PEER_SM(sm)->EAPOL_eapNoResp = TRUE;

    T_DG(TRACE_GRP_BASE, "%u: CTRL-EVENT-EAP-SUCCESS EAP authentication completed successfully", sm->port_info->port_no);
}


/*
 * This state is entered in case of a failure and state machine waits here
 * until port is disabled or EAP authentication is restarted.
 */
SM_STATE(EAP, FAILURE)
{
    SM_ENTRY(EAP, FAILURE);
    PEER_SM(sm)->EAPOL_eapFail = TRUE;
    setUnauthorized();

    /*
     * RFC 4137 does not clear EAPOL_eapReq here, but this seems to be required
     * to avoid processing the same request twice when state machine is
     * initialized.
     */
    PEER_SM(sm)->EAPOL_eapReq = FALSE;

    /*
     * RFC 4137 does not set EAPOL_eapNoResp here. However, either EAPOL_eapResp or
     * EAPOL_eapNoResp is required to be set after processing the received EAP
     * frame.
     */
    PEER_SM(sm)->EAPOL_eapNoResp = TRUE;

    T_DG(TRACE_GRP_BASE, "%u: CTRL-EVENT-EAP-FAILURE EAP authentication failed", sm->port_info->port_no);
}


static int eap_success_workaround(nas_sm_t *sm, int reqId, int lastId)
{
    /*
     * At least Microsoft IAS and Meetinghouse Aegis seem to be sending
     * EAP-Success/Failure with lastId + 1 even though RFC 3748 and
     * RFC 4137 require that reqId == lastId. In addition, it looks like
     * Ringmaster v2.1.2.0 would be using lastId + 2 in EAP-Success.
     *
     * Accept this kind of Id if EAP workarounds are enabled. These are
     * unauthenticated plaintext messages, so this should have minimal
     * security implications (bit easier to fake EAP-Success/Failure).
     */
    if (PEER_SM(sm)->workaround && (reqId == ((lastId + 1) & 0xff) || reqId == ((lastId + 2) & 0xff))) {
        T_DG(TRACE_GRP_BASE, "%u: EAP: Workaround for unexpected identifier field in EAP Success: reqId=%d lastId=%d (these are supposed to be same)", sm->port_info->port_no, reqId, lastId);
        return 1;
    }
    T_DG(TRACE_GRP_BASE, "%u: EAP: EAP-Success Id mismatch - reqId=%d lastId=%d", sm->port_info->port_no, reqId, lastId);
    return 0;
}


/*
 * RFC 4137 - Appendix A.1: EAP Peer State Machine - State transitions
 */

static void eap_peer_sm_step_idle(nas_sm_t *sm)
{
    /*
     * The first three transitions are from RFC 4137. The last two are
     * local additions to handle special cases with LEAP and PEAP server
     * not sending EAP-Success in some cases.
     */
    if (PEER_SM(sm)->EAPOL_eapReq) {
        SM_ENTER(EAP, RECEIVED);
    } else if ((PEER_SM(sm)->EAPOL_altAccept &&
                PEER_SM(sm)->decision != DECISION_FAIL) ||
               (PEER_SM(sm)->EAPOL_idleWhile == 0 &&
                PEER_SM(sm)->decision == DECISION_UNCOND_SUCC)) {
        SM_ENTER(EAP, SUCCESS);
    } else if (PEER_SM(sm)->EAPOL_altReject ||
               (PEER_SM(sm)->EAPOL_idleWhile == 0 &&
                PEER_SM(sm)->decision != DECISION_UNCOND_SUCC) ||
               (PEER_SM(sm)->EAPOL_altAccept &&
                PEER_SM(sm)->methodState != METHOD_CONT &&
                PEER_SM(sm)->decision == DECISION_FAIL)) {
        SM_ENTER(EAP, FAILURE);
    } else if (PEER_SM(sm)->selectedMethod == EAP_TYPE_LEAP &&
               PEER_SM(sm)->leap_done && PEER_SM(sm)->decision != DECISION_FAIL &&
               PEER_SM(sm)->methodState == METHOD_DONE) {
        SM_ENTER(EAP, SUCCESS);
    } else if (PEER_SM(sm)->selectedMethod == EAP_TYPE_PEAP &&
               PEER_SM(sm)->peap_done && PEER_SM(sm)->decision != DECISION_FAIL &&
               PEER_SM(sm)->methodState == METHOD_DONE) {
        SM_ENTER(EAP, SUCCESS);
    }
}

static int eap_peer_req_is_duplicate(nas_sm_t *sm)
{
    int duplicate;

    duplicate = (PEER_SM(sm)->reqId == PEER_SM(sm)->lastId) && PEER_SM(sm)->rxReq;
    if (PEER_SM(sm)->workaround && duplicate &&
        memcmp(PEER_SM(sm)->req_md5, PEER_SM(sm)->last_md5, 16) != 0) { // RBNTBD
        /*
         * RFC 4137 uses (reqId == lastId) as the only verification for
         * duplicate EAP requests. However, this misses cases where the
         * AS is incorrectly using the same id again; and
         * unfortunately, such implementations exist. Use MD5 hash as
         * an extra verification for the packets being duplicate to
         * workaround these issues.
         */
        T_DG(TRACE_GRP_BASE, "%u: EAP: AS used the same Id again, but EAP packets were not identical", sm->port_info->port_no);
        T_DG(TRACE_GRP_BASE, "%u: EAP: workaround - assume this is not a duplicate packet", sm->port_info->port_no);
        duplicate = 0;
    }

    return duplicate;
}

static void eap_peer_sm_step_received(nas_sm_t *sm)
{
    int duplicate = eap_peer_req_is_duplicate(sm);

    /*
     * Two special cases below for LEAP are local additions to work around
     * odd LEAP behavior (EAP-Success in the middle of authentication and
     * then swapped roles). Other transitions are based on RFC 4137.
     */
    if (PEER_SM(sm)->rxSuccess && PEER_SM(sm)->decision != DECISION_FAIL &&
        (PEER_SM(sm)->reqId == PEER_SM(sm)->lastId ||
         eap_success_workaround(sm, PEER_SM(sm)->reqId, PEER_SM(sm)->lastId))) {
        SM_ENTER(EAP, SUCCESS);
    } else if (PEER_SM(sm)->methodState != METHOD_CONT &&
               ((PEER_SM(sm)->rxFailure &&
                 PEER_SM(sm)->decision != DECISION_UNCOND_SUCC) ||
                (PEER_SM(sm)->rxSuccess && PEER_SM(sm)->decision == DECISION_FAIL &&
                 (PEER_SM(sm)->selectedMethod != EAP_TYPE_LEAP ||
                  PEER_SM(sm)->methodState != METHOD_MAY_CONT))) &&
               (PEER_SM(sm)->reqId == PEER_SM(sm)->lastId ||
                eap_success_workaround(sm, PEER_SM(sm)->reqId, PEER_SM(sm)->lastId))) {
        PEER_SM(sm)->eap_info.stop_reason = NAS_STOP_REASON_AUTH_FAILURE;
        SM_ENTER(EAP, FAILURE);
    } else if (PEER_SM(sm)->rxReq && duplicate) {
        SM_ENTER(EAP, RETRANSMIT);
    } else if (PEER_SM(sm)->rxReq && !duplicate &&
               PEER_SM(sm)->reqMethod == EAP_TYPE_NOTIFICATION &&
               PEER_SM(sm)->allowNotifications) {
        SM_ENTER(EAP, NOTIFICATION);
    } else if (PEER_SM(sm)->rxReq && !duplicate &&
               PEER_SM(sm)->selectedMethod == EAP_TYPE_NONE &&
               PEER_SM(sm)->reqMethod == EAP_TYPE_IDENTITY) {
        SM_ENTER(EAP, IDENTITY);
    } else if (PEER_SM(sm)->rxReq && !duplicate &&
               PEER_SM(sm)->selectedMethod == EAP_TYPE_NONE &&
               PEER_SM(sm)->reqMethod != EAP_TYPE_IDENTITY &&
               PEER_SM(sm)->reqMethod != EAP_TYPE_NOTIFICATION) {
        SM_ENTER(EAP, GET_METHOD);
    } else if (PEER_SM(sm)->rxReq && !duplicate &&
               PEER_SM(sm)->reqMethod == PEER_SM(sm)->selectedMethod &&
               PEER_SM(sm)->methodState != METHOD_DONE) {
        SM_ENTER(EAP, METHOD);
    } else if (PEER_SM(sm)->selectedMethod == EAP_TYPE_LEAP &&
               (PEER_SM(sm)->rxSuccess || PEER_SM(sm)->rxResp)) {
        SM_ENTER(EAP, METHOD);
    } else {
        SM_ENTER(EAP, DISCARD_IT);
    }
}


static void eap_peer_sm_step_local(nas_sm_t *sm)
{
    switch (PEER_SM(sm)->EAP_state) {
    case EAP_INITIALIZE:
        SM_ENTER(EAP, IDLE);
        break;
    case EAP_DISABLED:
        if (PEER_SM(sm)->EAPOL_portEnabled &&
            !PEER_SM(sm)->force_disabled) {
            SM_ENTER(EAP, INITIALIZE);
        }
        break;
    case EAP_IDLE:
        eap_peer_sm_step_idle(sm);
        break;
    case EAP_RECEIVED:
        eap_peer_sm_step_received(sm);
        break;
    case EAP_GET_METHOD:
        if (PEER_SM(sm)->selectedMethod == PEER_SM(sm)->reqMethod) {
            SM_ENTER(EAP, METHOD);
        } else {
            SM_ENTER(EAP, SEND_RESPONSE);
        }
        break;
    case EAP_METHOD:
        if (PEER_SM(sm)->ignore) {
            SM_ENTER(EAP, DISCARD_IT);
        } else {
            SM_ENTER(EAP, SEND_RESPONSE);
        }
        break;
    case EAP_SEND_RESPONSE:
        SM_ENTER(EAP, IDLE);
        break;
    case EAP_DISCARD_IT:
        SM_ENTER(EAP, IDLE);
        break;
    case EAP_IDENTITY:
        SM_ENTER(EAP, SEND_RESPONSE);
        break;
    case EAP_NOTIFICATION:
        SM_ENTER(EAP, SEND_RESPONSE);
        break;
    case EAP_RETRANSMIT:
        SM_ENTER(EAP, SEND_RESPONSE);
        break;
    case EAP_SUCCESS:
        break;
    case EAP_FAILURE:
        break;
    }
}

SM_STEP(EAP)
{
    /* Global transitions */
    if (PEER_SM(sm)->EAPOL_eapRestart &&
        PEER_SM(sm)->EAPOL_portEnabled) {
        SM_ENTER_GLOBAL(EAP, INITIALIZE);
    } else if (!PEER_SM(sm)->EAPOL_portEnabled || PEER_SM(sm)->force_disabled) {
        SM_ENTER_GLOBAL(EAP, DISABLED);
    } else if (PEER_SM(sm)->num_rounds > EAP_MAX_AUTH_ROUNDS) {
        /* RFC 4137 does not place any limit on number of EAP messages
         * in an authentication session. However, some error cases have
         * ended up in a state were EAP messages were sent between the
         * peer and server in a loop (e.g., TLS ACK frame in both
         * direction). Since this is quite undesired outcome, limit the
         * total number of EAP round-trips and abort authentication if
         * this limit is exceeded.
         */
        if (PEER_SM(sm)->num_rounds == EAP_MAX_AUTH_ROUNDS + 1) {
            T_DG(TRACE_GRP_BASE, "%u: EAP: more than %d authentication rounds - abort", sm->port_info->port_no, EAP_MAX_AUTH_ROUNDS);
            PEER_SM(sm)->num_rounds++;
            PEER_SM(sm)->eap_info.stop_reason = NAS_STOP_REASON_AUTH_TOO_MANY_ROUNDS;
            SM_ENTER_GLOBAL(EAP, FAILURE);
        }
    } else {
        /* Local transitions */
        eap_peer_sm_step_local(sm);
    }
}

/**
 */
void peer_sm_init(nas_sm_t *sm)
{
    PEER_SM(sm)->workaround        = 0;
    PEER_SM(sm)->force_disabled    = FALSE;
    PEER_SM(sm)->fast_reauth       = FALSE;
    PEER_SM(sm)->reAuthEnabled     = nas_os_get_reauth_enabled();
    PEER_SM(sm)->reAuthWhen        = 0;
    PEER_SM(sm)->EAPOL_altAccept   = FALSE;
    PEER_SM(sm)->EAPOL_altReject   = FALSE;
    PEER_SM(sm)->EAPOL_portEnabled = TRUE;
    PEER_SM(sm)->m                 = NULL;
    PEER_SM(sm)->eap_method_priv   = NULL;
    PEER_SM(sm)->how_to_initialize = 0;
}

/**
 * peer_sm_step - Step EAP peer state machine
 * @sm: Pointer to EAP state machine allocated with peer_sm_init()
 *
 * This function advances EAP state machine to a new state to match with the
 * current variables. This should be called whenever variables used by the EAP
 * state machine have changed.
 */
void peer_sm_step(nas_sm_t *sm)
{

    // Make sure that the RADIUS ID is freed whenever
    // it is not needed anymore. It is needed as long
    // as idleWhile > 0.
    // The function is fast to call, so it doesn't matter that
    // we call it more often than it's really needed to be called.
    if (PEER_SM(sm)->EAPOL_idleWhile == 0) {
        PEER_SM(sm)->eap_info.last_frame_type = FRAME_TYPE_NO_RETRANSMISSION; // Avoid retransmission from now on
        nas_os_backend_server_free_resources(sm);
    }

    do {
        PEER_SM(sm)->changed = FALSE;
        SM_STEP_RUN(EAP);
    } while (PEER_SM(sm)->changed);
}

/**
 * eap_set_workaround - Update EAP workarounds setting
 * @sm: Pointer to EAP state machine allocated with peer_sm_init()
 * @workaround: 1 = Enable EAP workarounds, 0 = Disable EAP workarounds
 */
void eap_set_workaround(nas_sm_t *sm, unsigned int workaround)
{
    PEER_SM(sm)->workaround = workaround;
}

/**
 * eap_get_eapRespData - Get EAP response data
 * @sm: Pointer to EAP state machine allocated with peer_sm_init()
 * @len: Pointer to variable that will be set to the length of the response
 * Returns: Pointer to the EAP response (eapRespData) or %NULL on failure
 *
 * Fetch EAP response (eapRespData) from the EAP state machine. This data is
 * available when EAP state machine has processed an incoming EAP request. The
 * EAP state machine does not maintain a reference to the response after this
 * function is called and the caller is responsible for freeing the data.
 */
u8 *eap_get_eapRespData(nas_sm_t *sm, size_t *len)
{
    u8 *resp;

    if (sm == NULL || PEER_SM(sm)->eapRespData == NULL) {
        *len = 0;
        return NULL;
    }

    resp = PEER_SM(sm)->eapRespData;
    *len = PEER_SM(sm)->eapRespDataLen;
    PEER_SM(sm)->eapRespData = NULL;
    PEER_SM(sm)->eapRespDataLen = 0;

    return resp;
}

/******************************************************************************/
// peer_force_reinit()
/******************************************************************************/
static void PEER_force_restart(nas_sm_t *sm, int how_to_initialize, BOOL step_sm)
{
    // Create a fake request identity packet to re-kickstart the peer state machine.
    peer_request_identity(sm);
    PEER_SM(sm)->EAPOL_eapReq      = TRUE;
    PEER_SM(sm)->EAPOL_eapRestart  = TRUE;
    PEER_SM(sm)->how_to_initialize = how_to_initialize;
    if (step_sm) {
        peer_sm_step(sm);
    }
}

/******************************************************************************/
// peer_force_reinit()
/******************************************************************************/
void peer_force_reinit(nas_sm_t *sm, BOOL this_is_an_existing_sm)
{
    PEER_force_restart(sm, this_is_an_existing_sm ? 1 : 0, TRUE);
}

/******************************************************************************/
// peer_force_reauth()
/******************************************************************************/
void peer_force_reauth(nas_sm_t *sm, BOOL step_sm)
{
    PEER_force_restart(sm, 2, step_sm);
}

/******************************************************************************/
// peer_timers_tick()
// Supposed to be called once a second.
/******************************************************************************/
void peer_timers_tick(nas_sm_t *sm)
{
    // Reauthentication timeout (corresponds to REAUTH_TIMER state machine in authenticator SM).
    if (PEER_SM(sm)->reAuthEnabled && PEER_SM(sm)->authStatus == NAS_PORT_STATUS_AUTHORIZED) {
        if (PEER_SM(sm)->reAuthWhen > 0) {
            if (--PEER_SM(sm)->reAuthWhen == 0) {
                peer_force_reauth(sm, FALSE);
                // reAuthWhen gets re-initialized when authentication succeeds.
            }
        }
    }

    // See comment in peer.c's MB_do_alloc_sm() where fake_request_identities_left is set.
    if (PEER_SM(sm)->EAPOL_idleWhile > 0 && PEER_SM(sm)->how_to_initialize == 0) {
        PEER_SM(sm)->EAPOL_idleWhile--;
        if (PEER_SM(sm)->EAPOL_idleWhile == 0) {
            // fake_request_identities_left is decreased in peer_backend_send_response()
            if (PEER_SM(sm)->fake_request_identities_left > 0) {
                peer_force_reinit(sm, FALSE);
            } else {
                PEER_SM(sm)->eap_info.stop_reason = NAS_STOP_REASON_AUTH_NOT_CONFIGURED;
            }
        }
    }

    peer_sm_step(sm);
}
