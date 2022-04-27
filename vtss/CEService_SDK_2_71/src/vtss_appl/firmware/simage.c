/*

 Vitesse Switch API software.

 Copyright (c) 2002-2011 Vitesse Semiconductor Corporation "Vitesse". All
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
#include "firmware.h"
#include "firmware_api.h"

#include <tomcrypt.h>

typedef struct {
    const u8 *buf_ptr;
    size_t    buf_len;
} bufptr_t;

static BOOL mac_check(int algo, const u8 *buf, size_t len, const u8 *digest)
{
    u8 calc_hash[SHA1_HASH_SIZE];
    unsigned long digest_length = SHA1_HASH_SIZE;
    if(hmac_memory(algo, (const u8*) SIMAGE_KEY, strlen(SIMAGE_KEY), 
                   buf, len, calc_hash, &digest_length) != CRYPT_OK ||
       digest_length != SHA1_HASH_SIZE ||
       digest_length > SIMAGE_DIGLEN) {
        T_E("MAC calculation error");
        return FALSE;
    }
    return (memcmp(digest, calc_hash, digest_length) == 0);
}

static const u8 *
bufsearch(const u8 *haystack, size_t length, const u8 *needle, size_t nlength)
{
    const u8 *end = haystack+length;
    while(haystack < end &&
          (haystack = memchr(haystack, needle[0], end - haystack))) {
        if(nlength <= (size_t)(end - haystack) &&
           memcmp(haystack, needle, nlength) == 0)
            return haystack;
        haystack++;
    }
    return NULL;
}

static u32 get_dword(const u8 *ptr)
{
    return (u32) ((ptr[0]) + 
                  (ptr[1] << 8) +
                  (ptr[2] << 16) +
                  (ptr[3] << 24));
}

static inline void buf_init(bufptr_t *b, const u8 *ptr, size_t len)
{
    b->buf_ptr = ptr;
    b->buf_len = len;
}

static inline void buf_skip(bufptr_t *b, size_t n)
{
    if(n <= b->buf_len) {
        b->buf_ptr += n;
        b->buf_len -= n;
    } else {
        b->buf_ptr += b->buf_len;
        b->buf_len = 0;
    }
}

static u32 get_dword_advance(bufptr_t *b)
{
    u32 val = get_dword(b->buf_ptr);
    buf_skip(b, sizeof(u32));
    return val;
}

static void simage_init(simage_t *simg, const u8 *image, size_t img_len)
{
    memset(simg, 0, sizeof(*simg));
    simg->img_ptr = image;
    simg->img_len = img_len;
}

static BOOL simage_validate_hash(simage_t *simg, const u8 *p)
{
    const u8 *ptrs = p;
    ptrs += SIMAGE_SIGLEN;      /* Skip Signature */
    simg->file_len = get_dword(ptrs); ptrs += 4;
    memcpy(simg->file_digest, ptrs, SIMAGE_DIGLEN); ptrs += SIMAGE_DIGLEN;
    simg->trailer_len = get_dword(ptrs); ptrs += 4;
    memcpy(simg->trailer_digest, ptrs, SIMAGE_DIGLEN); ptrs += SIMAGE_DIGLEN;
    simg->trailer_ptr = ptrs;
    if(simg->file_len < simg->img_len &&
       (simg->trailer_ptr + simg->trailer_len) < (simg->img_ptr + simg->img_len)) {
        int algo = register_hash (&sha1_desc);
        if(algo < 0) {
            T_E("Unable to register SHA1 hash function, configuration error");
            return FALSE;
        }
        if(!mac_check(algo, simg->img_ptr, simg->file_len, simg->file_digest)) {
            T_W("File MAC error");
            return FALSE;
        }
        if(!mac_check(algo, simg->trailer_ptr, simg->trailer_len, simg->trailer_digest)) {
            T_W("Trailer MAC error");
            return FALSE;
        }
        T_D("Image validated, now parsing");
        return TRUE;
    }
    T_D("Ptr consistency error: lengths (file %zu, img %zu), img %p, trailer(%p , %zu)",
        simg->file_len, simg->img_len, simg->img_ptr, simg->trailer_ptr, simg->trailer_len);
    return FALSE;
}

static const u8 *simage_search_trailer(simage_t *simg, bufptr_t *b)
{
    const u8 *hit;
    if(b->buf_len &&
       (hit = bufsearch(b->buf_ptr, b->buf_len, (const u8*)SIMAGE_SIG, SIMAGE_SIGLEN))) {
        buf_skip(b, hit - b->buf_ptr); /* Skip down */
        T_D("Found signature at %p, buffer left %zu", hit, b->buf_len);
        return b->buf_ptr;
    }
    T_D("No signature match");
    return NULL;
}

static vtss_rc simage_get_trailer(simage_t *simg, bufptr_t *b)
{
    u32 backp;
    const u8 *p;
    /* Try to use the backpointer, if found */
    p = (b->buf_ptr + b->buf_len); /* End of buffer */
    if(get_dword(p - 2*sizeof(u32)) == SIMAGE_TCOOKIE &&
       (backp = get_dword(p - 1*sizeof(u32))) < b->buf_len &&
       memcmp(p - backp, SIMAGE_SIG, SIMAGE_SIGLEN) == 0) {
        T_D("Signature found with backpointer: %d", backp);
        p -= backp;
        return simage_validate_hash(simg, p) ? VTSS_OK : FIRMWARE_ERROR_CRC;
    } else {
        /* Must search from start */
        while((p = simage_search_trailer(simg, b))) {
            if(simage_validate_hash(simg, p))
                return VTSS_OK; 
            else 
                /* Try for other matches */
                buf_skip(b, SIMAGE_SIGLEN);
        }
    }

    /* All failed */
    return FIRMWARE_ERROR_INVALID;
}

vtss_rc simage_parse_tlvs(simage_t *simg)
{
    bufptr_t buf;
    buf_init(&buf, simg->trailer_ptr, simg->trailer_len);
    while(buf.buf_len > 3*sizeof(u32)) {
        u32 type, id, dlen;
        type = get_dword_advance(&buf);
        id = get_dword_advance(&buf);
        dlen = get_dword_advance(&buf);
        if(id > 0 && id < SIMAGE_TLV_COUNT) {
            simage_tlv_t *tlv = &simg->tlv[id];
            T_N("type %d, id %d dlen %d", type, id, dlen);
            switch(type) {
            case SIMAGE_TLV_TYPE_DWORD:
                tlv->valid = TRUE;
                tlv->type = type;
                tlv->dw_value = get_dword(buf.buf_ptr);
                T_D("Dword id %d value %d", id, tlv->dw_value);
                break;
            case SIMAGE_TLV_TYPE_STRING:
                tlv->valid = TRUE;
                tlv->type = type;
                tlv->str_len = dlen;
                tlv->str_ptr = buf.buf_ptr;
                T_D("Str id %d value %s", id, buf.buf_ptr);
                break;
            case SIMAGE_TLV_TYPE_BINARY:
                tlv->valid = TRUE;
                tlv->type = type;
                tlv->str_len = dlen;
                tlv->str_ptr = buf.buf_ptr;
                T_D("Bin id %d len %u", id, dlen);
                break;
            default:
                T_I("Skip invalid type: type %d, id %d dlen %d", type, id, dlen);
            }
        } else {
            T_I("Skip invalid id: %d, id %d dlen %d", type, id, dlen);
        }
        buf_skip(&buf, dlen);
    }
    /* Always succeeds, even if skipped TLV's */
    return VTSS_OK;
}

vtss_rc simage_parse(simage_t *simg, const u8 *image, size_t img_len)
{
    bufptr_t buf;
    vtss_rc rc;
    simage_init(simg, image, img_len);
    buf_init(&buf, image, img_len);
    if((rc = simage_get_trailer(simg, &buf)) != VTSS_OK)
        return rc; 
    return simage_parse_tlvs(simg);
}

BOOL simage_get_tlv_dword(const simage_t *simg, int id, u32 *val)
{
    if(id > 0 && id < SIMAGE_TLV_COUNT) {
        const simage_tlv_t *tlv = &simg->tlv[id];
        if(tlv->valid && tlv->type == SIMAGE_TLV_TYPE_DWORD) {
            *val = tlv->dw_value;
            return TRUE;
        }
    } 
    return FALSE;
}

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
