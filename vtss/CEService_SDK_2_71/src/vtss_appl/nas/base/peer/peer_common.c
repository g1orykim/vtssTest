/*
 * EAP common peer/server definitions
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

#include "../common/nas_types.h"
#include "peer_common.h"
#include "peer_sm.h"
#include "peer.h"
#include <network.h> /* for ntohs() and htons() */

/**
 * eap_hdr_validate - Validate EAP header
 * @vendor: Expected EAP Vendor-Id (0 = IETF)
 * @eap_type: Expected EAP type number
 * @msg: EAP frame (starting with EAP header)
 * @msglen: Length of msg
 * @plen: Pointer to variable to contain the returned payload length
 * Returns: Pointer to EAP payload (after type field), or %NULL on failure
 *
 * This is a helper function for EAP method implementations. This is usually
 * called in the beginning of struct eap_method::process() function to verify
 * that the received EAP request packet has a valid header. This function is
 * able to process both legacy and expanded EAP headers and in most cases, the
 * caller can just use the returned payload pointer (into *plen) for processing
 * the payload regardless of whether the packet used the expanded EAP header or
 * not.
 */
const u8 *eap_hdr_validate(int vendor, EapType eap_type, const u8 *msg, size_t msglen, size_t *plen)
{
    const struct eap_hdr *hdr;
    const u8             *pos;
    size_t               len;

    hdr = (const struct eap_hdr *) msg;

    if (msglen < sizeof(*hdr)) {
        T_DG(TRACE_GRP_BASE, "EAP: Too short EAP frame");
        return NULL;
    }

    len = ntohs(hdr->length);
    if (len < sizeof(*hdr) + 1 || len > msglen) {
        T_DG(TRACE_GRP_BASE, "EAP: Invalid EAP length");
        return NULL;
    }

    pos = (const u8 *) (hdr + 1);

    if (*pos == (u8)EAP_TYPE_EXPANDED) {
        int exp_vendor;
        u32 exp_type;
        if (len < sizeof(*hdr) + 8) {
            T_DG(TRACE_GRP_BASE, "EAP: Invalid expanded EAP length");
            return NULL;
        }
        pos++;
        exp_vendor = WPA_GET_BE24(pos);
        pos += 3;
        exp_type = WPA_GET_BE32(pos);
        pos += 4;
        if (exp_vendor != vendor || exp_type != (u32)eap_type) {
            T_DG(TRACE_GRP_BASE, "EAP: Invalid expanded frame type");
            return NULL;
        }

        *plen = len - sizeof(*hdr) - 8;
        return pos;
    } else {
        if (vendor != EAP_VENDOR_IETF || *pos != (u8)eap_type) {
            T_DG(TRACE_GRP_BASE, "EAP: Invalid frame type");
            return NULL;
        }
        *plen = len - sizeof(*hdr) - 1;
        return pos + 1;
    }
}

/**
 * eap_msg_alloc - Allocate a buffer for an EAP message
 * @vendor: Vendor-Id (0 = IETF)
 * @type: EAP type
 * @len: Buffer for returning message length
 * @payload_len: Payload length in bytes (data after Type)
 * @code: Message Code (EAP_CODE_*)
 * @identifier: Identifier
 * @payload: Pointer to payload pointer that will be set to point to the
 * beginning of the payload or %NULL if payload pointer is not needed
 * Returns: Pointer to the allocated message buffer or %NULL on error
 *
 * This function can be used to allocate a buffer for an EAP message and fill
 * in the EAP header. This function is automatically using expanded EAP header
 * if the selected Vendor-Id is not IETF. In other words, most EAP methods do
 * not need to separately select which header type to use when using this
 * function to allocate the message buffers.
 */
struct eap_hdr *eap_msg_alloc(nas_sm_t *sm, int vendor, EapType type, size_t *len,
                              size_t payload_len, u8 code, u8 identifier, u8 **payload)
{
    struct eap_hdr *hdr;
    u8             *pos;

    *len = sizeof(struct eap_hdr) + (vendor == EAP_VENDOR_IETF ? 1 : 8) +
           payload_len;
    hdr = (struct eap_hdr *)PEER_SM(sm)->eap_info.last_frame;
    if (hdr) {
        hdr->code = code;
        hdr->identifier = identifier;
        hdr->length = htons(*len);
        pos = (u8 *)(hdr + 1);
        if (vendor == EAP_VENDOR_IETF) {
            *(pos++) = (u8)type;
        } else {
            *(pos++) = (u8)EAP_TYPE_EXPANDED;
            WPA_PUT_BE24(pos, vendor);
            pos += 3;
            WPA_PUT_BE32(pos, type);
            pos += 4;
        }
        if (payload) {
            *payload = pos;
        }
    }

    return hdr;
}
