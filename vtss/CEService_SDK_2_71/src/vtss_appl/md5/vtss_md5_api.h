/*

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

#ifndef __VTSS_MD5_H__
#define __VTSS_MD5_H__

#ifndef md5_u8
#define md5_u8  unsigned char
#endif

#ifndef md5_u16
#define md5_u16  unsigned short
#endif

#ifndef md5_u32
#define md5_u32 unsigned int
#endif

/* Functions and structs are prefixed with 'vtss_' to avoid name-clashes. */
struct vtss_MD5Context {
  md5_u32 buf[4];
  md5_u32 bits[2];
  md5_u8  in[64];
};

#include <stddef.h> /* For size_t */
#include <string.h> /* For memcpy(), etc. */
#define MD5_MAC_LEN 16
void vtss_MD5Init(struct vtss_MD5Context *ctx);
void vtss_MD5Update(struct vtss_MD5Context *ctx, md5_u8 const *buf, md5_u16 len);
void vtss_MD5Final(md5_u8 digest[MD5_MAC_LEN], struct vtss_MD5Context *ctx);
void vtss_md5_vector(size_t num_elem, const md5_u8 *addr[], const size_t *len, md5_u8 *mac);
/* Currently unused functions: */
void vtss_hmac_md5_vector(const md5_u8 *key, size_t key_len, size_t num_elem, const md5_u8 *addr[], const size_t *len, md5_u8 *mac);
void vtss_hmac_md5(const md5_u8 *key, size_t key_len, const md5_u8 *data, size_t data_len, md5_u8 *mac);

/*
 * Use these two functions to speed up MD5 computations when the first
 * part of the MD5 always include the same shared secret.
 */
void vtss_md5_shared_key_init(struct vtss_MD5Context *ctx1, struct vtss_MD5Context *ctx2, const md5_u8 *secret, md5_u16 secret_len);
void vtss_md5_shared_key(struct vtss_MD5Context *ctx1, struct vtss_MD5Context *ctx2, md5_u8 * _data, md5_u16 data_len, md5_u8 *mac);

#endif /* __VTSS_MD5_H__ */

