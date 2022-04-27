/*
 * EAP peer method: EAP-MD5 (RFC 3748 and RFC 1994)
 * Copyright (c) 2004-2006, Jouni Malinen <j@w1.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 */

/*lint -esym(459, eap_md5_process) */

#include "peer.h"
#include "peer_common.h"
#include "peer_methods.h"
#include "vtss_md5_api.h"
#include "dot1x_trace.h"

// Statically allocate the MD5 method
static struct eap_method md5_method;

static void *eap_md5_init(struct nas_sm *sm)
{
    /* No need for private data. However, must return non-NULL to indicate
     * success. */
    return (void *) 1;
}


static void eap_md5_deinit(struct nas_sm *sm, void *priv)
{
}

// Change this if you don't want extra data in the EAP response
// consisting of the user name.
#define EAP_MD5_ADD_USER_NAME_AS_EXTRA_DATA

static u8 *eap_md5_process(struct nas_sm *sm, void *priv,
                           struct eap_method_ret *ret,
                           const u8 *reqData, size_t reqDataLen,
                           size_t *respDataLen)
{

    // Vitesse: This function is not re-entrant, since we use a static, temporary buffer,
    // since we use the same buffer for reception and transmission.
    static u8 internal_buffer[100]; // Max expected EAP length.
    const struct eap_hdr *req;
    struct eap_hdr *resp;
    const u8 *pos, *challenge;
    const i8 *password;
    u8 *rpos;
    size_t len, challenge_len, password_len;
    const u8 *addr[3];
    size_t elen[3];
#ifdef EAP_MD5_ADD_USER_NAME_AS_EXTRA_DATA
    size_t supp_id_len = strlen(sm->client_info.identity);
#endif

    // Vitesse specific:
    if (reqDataLen > sizeof(internal_buffer)) {
        T_EG(TRACE_GRP_BASE, "%u: EAP-MD5: Input EAP frame (%zu bytes) bigger than %zu bytes", sm->port_info->port_no, reqDataLen, sizeof(internal_buffer));
        ret->ignore = TRUE;
        return NULL;
    }

    memcpy(internal_buffer, reqData, reqDataLen);

    // Vitesse specific:
    password     = sm->client_info.identity;
    password_len = strlen(password);

    pos = eap_hdr_validate(EAP_VENDOR_IETF, EAP_TYPE_MD5, internal_buffer, reqDataLen, &len);
    if (pos == NULL || len == 0) {
        T_WG(TRACE_GRP_BASE, "%u: EAP-MD5: Invalid frame (pos=%p len=%zu)", sm->port_info->port_no, pos, len);
        ret->ignore = TRUE;
        return NULL;
    }

    /*
     * CHAP Challenge:
     * Value-Size (1 octet) | Value(Challenge) | Name(optional)
     */
    req = (const struct eap_hdr *)internal_buffer;
    challenge_len = *pos++;
    if (challenge_len == 0 || challenge_len > len - 1) {
        T_WG(TRACE_GRP_BASE, "%u: EAP-MD5: Invalid challenge (challenge_len=%u len=%zu)", sm->port_info->port_no, challenge_len, len);
        ret->ignore = TRUE;
        return NULL;
    }
    ret->ignore = FALSE;
    challenge = pos;
    T_NG(TRACE_GRP_BASE, "%u: EAP-MD5: Challenge", sm->port_info->port_no);
    T_NG_HEX(TRACE_GRP_BASE, challenge, challenge_len);

    T_DG(TRACE_GRP_BASE, "%u: EAP-MD5: Generating Challenge Response", sm->port_info->port_no);
    ret->methodState = METHOD_DONE;
    // ret->decision = DECISION_UNCOND_SUCC;
    // Well, I don't understand why the original code had DECISION_UNCOND_SUCC here.
    // In that case, we'd think we were authenticated even if this last frame we're
    // about to send wasn't reacted upon by the server. I've taken the liberty to
    // change it to a conditional success, so that we have to wait for the EAP success
    // frame before opening the port for that MAC address.
    ret->decision = DECISION_COND_SUCC;
    ret->allowNotifications = TRUE;

#ifdef EAP_MD5_ADD_USER_NAME_AS_EXTRA_DATA
    resp = eap_msg_alloc(sm, EAP_VENDOR_IETF, EAP_TYPE_MD5, respDataLen, 1 + MD5_MAC_LEN + supp_id_len, EAP_CODE_RESPONSE, req->identifier, &rpos);
#else
    resp = eap_msg_alloc(sm, EAP_VENDOR_IETF, EAP_TYPE_MD5, respDataLen, 1 + MD5_MAC_LEN, EAP_CODE_RESPONSE, req->identifier, &rpos);
#endif
    if (resp == NULL) {
        return NULL;
    }

    /*
     * CHAP Response:
     * Value-Size (1 octet) | Value(Response) | Name(optional)
     */
    *rpos++ = MD5_MAC_LEN;

    addr[0] = &resp->identifier;
    elen[0] = 1;
    addr[1] = (u8 *)password;
    elen[1] = password_len;
    addr[2] = challenge;
    elen[2] = challenge_len;
    vtss_md5_vector(3, addr, elen, rpos);

    T_NG(TRACE_GRP_BASE, "%u: EAP-MD5: Response", sm->port_info->port_no);
    T_NG_HEX(TRACE_GRP_BASE, rpos, MD5_MAC_LEN);

#ifdef EAP_MD5_ADD_USER_NAME_AS_EXTRA_DATA
    // Vitesse: Optionally we can put extra data here.
    // We insert the user-name in clear text
    rpos += MD5_MAC_LEN;
    memcpy(rpos, sm->client_info.identity, supp_id_len);
#endif

    return (u8 *) resp;
}

int eap_peer_md5_register(void)
{
    struct eap_method *eap = &md5_method;
    int ret;

    // NULLify all unused callback functions like
    // isKeyAvailable(), getKey(), get_status(), has_reauth_data()
    // deinit_for_reauth(), init_for_reauth(), get_identity(),
    // free(), and get_emsk()
    memset(&md5_method, 0, sizeof(md5_method));

    // General stuff
    eap->version = EAP_PEER_METHOD_INTERFACE_VERSION;
    eap->vendor  = EAP_VENDOR_IETF;
    eap->method  = EAP_TYPE_MD5;
    eap->name    = "MD5";

    // Callbacks
    eap->init = eap_md5_init;
    eap->deinit = eap_md5_deinit;
    eap->process = eap_md5_process;

    ret = eap_peer_method_register(eap);
    return ret;
}
