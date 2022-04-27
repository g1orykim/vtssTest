/*

 Vitesse Switch Software.

 Copyright (c) 2002-2012 Vitesse Semiconductor Corporation "Vitesse". All
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

#include <main.h>
#include "inet_address.h"

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_SNMP

BOOL prepare_get_next_inetAddress(long *type, char *addr, size_t addr_max_len, size_t *addr_len )
{
    if (addr_max_len < INET_ADDRESS_IPV6_LEN) {
        return FALSE;
    }

    switch (*type) {
    case INET_ADDRESS_UNKNOWN:
        *type = INET_ADDRESS_IPV4;
        *addr_len = INET_ADDRESS_IPV4_LEN;
        memset( addr, 0, *addr_len);
        break;
    case INET_ADDRESS_IPV4:
        if (*addr_len < INET_ADDRESS_IPV4_LEN) {
            *addr_len = INET_ADDRESS_IPV4_LEN;
        } else if (*addr_len > INET_ADDRESS_IPV4_LEN) {
            *type = INET_ADDRESS_IPV6;
            *addr_len = INET_ADDRESS_IPV6_LEN;
        } else {
            break;
        }
        memset( addr, 0, *addr_len);
        break;
    case INET_ADDRESS_IPV6:
        if (*addr_len < INET_ADDRESS_IPV6_LEN) {
            *addr_len = INET_ADDRESS_IPV6_LEN;
        } else if (*addr_len > INET_ADDRESS_IPV6_LEN) {
            return FALSE;
        } else {
            break;
        }
        memset( addr, 0, *addr_len);
        break;
    default:
        return FALSE;
    }

    return TRUE;
}


vtss_rc get_pre_inetAddress(char *addr, size_t *addr_len )
{
    int i = 0;
    BOOL overflow = TRUE;
    u8 *tmp = NULL;
    u8 *ptr = tmp;

    tmp = VTSS_MALLOC(sizeof(char) * (*addr_len));

    if ( NULL == tmp) {
        return VTSS_RC_INCOMPLETE;
    }

    memcpy(tmp, addr, *addr_len);
    for (i = *addr_len - 1, ptr = (u8 *)(tmp + *addr_len - 1); i >= 0; i--, ptr-- ) {
        if (*ptr != 0x0 ) {
            --*ptr;
            overflow = FALSE;
            break;
        } else {
            *ptr = 0xff;
        }
    }

    if ( FALSE == overflow) {
        memcpy(addr, tmp, *addr_len);
    }
    VTSS_FREE(tmp);
    return overflow ? VTSS_RC_ERROR : VTSS_RC_OK;
}

