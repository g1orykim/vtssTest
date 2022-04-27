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

#include "vtss_api.h"
#include "vtss_macsec_api.h"
#include "vtss_macsec_test_base.h"
#include <openssl/aes.h>

vtss_rc sak_update_hash_key(vtss_macsec_sak_t * sak)
{
    AES_KEY aes_key;
    char null_data[] = {0, 0, 0, 0, 0, 0, 0, 0,
                        0, 0, 0, 0, 0, 0, 0, 0};

    if (sak->len == 16) {
        AES_set_encrypt_key(sak->buf, 128, &aes_key);

    } else if (sak->len == 32) {
        AES_set_encrypt_key(sak->buf, 256, &aes_key);

    } else {
        return VTSS_RC_ERROR;

    }

    AES_ecb_encrypt((const unsigned char *)null_data,
                    (unsigned char *)sak->h_buf,
                    &aes_key,
                    AES_ENCRYPT);

    return VTSS_RC_OK;
}

