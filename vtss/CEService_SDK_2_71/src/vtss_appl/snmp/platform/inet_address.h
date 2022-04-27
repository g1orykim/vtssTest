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

/*  This header file is based on IANAifType-MIB to define the interface type which is used in ifType  */
#ifndef INET_ADDRESS_H
#define INET_ADDRESS_H

typedef enum {
    INET_ADDRESS_UNKNOWN = 0,
    INET_ADDRESS_IPV4,
    INET_ADDRESS_IPV6,
    INET_ADDRESS_IPV4Z,
    INET_ADDRESS_IPV6Z,
    INET_ADDRESS_DNS = 16,
} inet_address_type;

typedef enum {
    INET_VERSION_UNKNOWN = 0,
    INET_VERSION_IPV4,
    INET_VERSION_IPV6,
} inet_version;

#define INET_ADDRESS_IPV4_LEN 4
#define INET_ADDRESS_IPV6_LEN 16

BOOL prepare_get_next_inetAddress(long *type, char *addr, size_t addr_max_len, size_t *addr_len );
vtss_rc get_pre_inetAddress(char *addr, size_t *addr_len );

#endif

