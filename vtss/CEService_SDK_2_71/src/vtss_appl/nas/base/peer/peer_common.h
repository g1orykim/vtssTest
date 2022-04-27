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

#ifndef PEER_EAP_COMMON_H__
#define PEER_EAP_COMMON_H__

#include <sys/types.h>            /* For size_t */
#include "../common/nas_types.h"

#define WPA_PUT_BE24(a, val)                    \
  do {                                          \
    /*lint -save -e572 */                       \
    (a)[0] = (u8)((((u32)(val)) >> 16) & 0xff); \
    (a)[1] = (u8)((((u32)(val)) >>  8) & 0xff); \
    (a)[2] = (u8)((((u32)(val)) >>  0) & 0xff); \
    /*lint -restore */                          \
  } while (0)

#define WPA_GET_BE24(a) ((((u32)(a)[0]) << 16) | (((u32)(a)[1]) << 8) | ((u32)(a)[2]))

#define WPA_PUT_BE32(a, val)                    \
  do {                                          \
    /*lint -save -e572 */                       \
    (a)[0] = (u8)((((u32)(val)) >> 24) & 0xff); \
    (a)[1] = (u8)((((u32)(val)) >> 16) & 0xff); \
    (a)[2] = (u8)((((u32)(val)) >>  8) & 0xff); \
    (a)[3] = (u8)((((u32)(val)) >>  0) & 0xff); \
    /*lint -restore */                          \
  } while (0)

#define WPA_GET_BE32(a) ((((u32)(a)[0]) << 24) | (((u32)(a)[1]) << 16) | (((u32) (a)[2]) << 8) | ((u32)(a)[3]))

struct nas_sm;
const  u8      *eap_hdr_validate(int vendor, EapType eap_type, const u8 *msg, size_t msglen, size_t *plen);
struct eap_hdr *eap_msg_alloc(struct nas_sm *sm, int vendor, EapType type, size_t *len, size_t payload_len, u8 code, u8 identifier, u8 **payload);

#endif /* PEER_EAP_COMMON_H__ */
