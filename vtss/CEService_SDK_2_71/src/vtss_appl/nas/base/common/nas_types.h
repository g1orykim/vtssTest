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

#ifndef _NAS_TYPES_H_
#define _NAS_TYPES_H_

#include "vtss_nas_api.h" /* For OS-specific types */

typedef u16 nas_timer_t;
typedef u8  nas_enum_t; /* When referencing an enum type */

/*
   EAP Packet Format

    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |     Code      |  Identifier   |            Length             |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |    Data ...
   +-+-+-+-+

   Code

      The Code field is one octet and identifies the Type of EAP packet.
      EAP Codes are assigned as follows:

         1       Request
         2       Response
         3       Success
         4       Failure


   Request and Response packet format

    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |     Code      |  Identifier   |            Length             |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |     Type      |  Type-Data ...
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

      1       Identity
      2       Notification
      3       Nak (Response only)
      4       MD5-Challenge
      5       One Time Password (OTP)
      6       Generic Token Card (GTC)
    254       Expanded Types
    255       Experimental use

*/

#ifdef __GNUC__
/* Including code and identifier; network byte order followed by length - 4 octets of data */
struct eap_hdr {
    u8  code;
    u8  identifier;
    u16 length;
} __attribute__ ((packed));
#else
#error "Don't know how to create a packed array for your compiler"
#endif

/* RFC 3748 - Extensible Authentication Protocol (EAP) */

enum {
    EAP_CODE_REQUEST  = 1,
    EAP_CODE_RESPONSE = 2,
    EAP_CODE_SUCCESS  = 3,
    EAP_CODE_FAILURE  = 4
};

/* EAP Request and Response data begins with one octet Type. Success and
 * Failure do not have additional data. */

typedef enum {
    EAP_TYPE_NONE         =   0,
    EAP_TYPE_IDENTITY     =   1, /* RFC 3748 */
    EAP_TYPE_NOTIFICATION =   2, /* RFC 3748 */
    EAP_TYPE_NAK          =   3, /* Response only, RFC 3748 */
    EAP_TYPE_MD5          =   4, /* RFC 3748 */
    EAP_TYPE_OTP          =   5, /* RFC 3748 */
    EAP_TYPE_GTC          =   6, /* RFC 3748 */
    EAP_TYPE_TLS          =  13, /* RFC 2716 */
    EAP_TYPE_LEAP         =  17, /* Cisco proprietary */
    EAP_TYPE_SIM          =  18, /* RFC 4186 */
    EAP_TYPE_TTLS         =  21, /* draft-ietf-pppext-eap-ttls-02.txt */
    EAP_TYPE_AKA          =  23, /* RFC 4187 */
    EAP_TYPE_PEAP         =  25, /* draft-josefsson-pppext-eap-tls-eap-06.txt */
    EAP_TYPE_MSCHAPV2     =  26, /* draft-kamath-pppext-eap-mschapv2-00.txt */
    EAP_TYPE_TLV          =  33, /* draft-josefsson-pppext-eap-tls-eap-07.txt */
    EAP_TYPE_TNC          =  38, /* TNC IF-T v1.0-r3; note: tentative assignment;
                                * type 38 has previously been allocated for
                                * EAP-HTTP Digest, (funk.com) */
    EAP_TYPE_FAST         =  43, /* RFC 4851 */
    EAP_TYPE_PAX          =  46, /* RFC 4746 */
    EAP_TYPE_PSK          =  47, /* RFC 4764 */
    EAP_TYPE_SAKE         =  48, /* RFC 4763 */
    EAP_TYPE_EXPANDED     = 254, /* RFC 3748 */
    EAP_TYPE_GPSK         = 255  /* EXPERIMENTAL - type not yet allocated draft-ietf-emu-eap-gpsk-01.txt */
} EapType;

/* SMI Network Management Private Enterprise Code for vendor specific types */
enum {
    EAP_VENDOR_IETF = 0
};

#endif /* _NAS_TYPES_H_ */

