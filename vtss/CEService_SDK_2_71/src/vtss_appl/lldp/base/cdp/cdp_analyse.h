/*

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

// This file is the header file for the corresponding cdp_analyse.c file

#include <network.h> // For htons

// CDP TLV types.
#define CDP_TYPE_DEVICE_ID         0x0001
#define CDP_TYPE_ADDRESS           0x0002
#define CDP_TYPE_PORT_ID           0x0003
#define CDP_TYPE_CAPABILITIES      0x0004
#define CDP_TYPE_IOS_VERSION       0x0005
#define CDP_TYPE_PLATFORM          0x0006
#define CDP_TYPE_IP_PREFIX         0x0007
#define CDP_TYPE_PROTOCOL_HELLO    0x0008
#define CDP_TYPE_VTP_MGMT_DOMAIN   0x0009
#define CDP_TYPE_NATIVE_VLAN       0x000a
#define CDP_TYPE_DUPLEX            0x000b
#define CDP_TYPE_UNKNOWN_0x000c    0x000c
#define CDP_TYPE_UNKNOWN_0x000d    0x000d
#define CDP_TYPE_APPLIANCE_REPLY   0x000e
#define CDP_TYPE_APPLIANCE_QUERY   0x000f
#define CDP_TYPE_POWER_CONSUMPTION 0x0010
#define CDP_TYPE_MTU               0x0011
#define CDP_TYPE_EXTENDED_TRUST    0x0012
#define CDP_TYPE_UNTRUSTED_COS     0x0013
#define CDP_TYPE_SYSTEM_NAME       0x0014
#define CDP_TYPE_SYSTEM_OID        0x0015
#define CDP_TYPE_MGMT_ADDRESS      0x0016
#define CDP_TYPE_LOCATION          0x0017
#define CDP_TYPE_UNKNOWN_0x001A    0x001A

#define MAX_CDP_FRAME_SIZE 1518



// stucture that can contain the CDP information that we supports.
typedef struct  {
    uint len;
    uint8_t  version;
    uint8_t  ttl;
    uint16_t checksum;
    uint8_t  device_id_len;
    BOOL     device_id_vld;
    uint8_t  *device_id;
    uint8_t  address_protocol_type; // We only supports IPv4 ( 0x1), others = 0x0
    uint8_t  address_len ;
    BOOL     address_vld;
    uint8_t  *address; // Pointer to the address
    uint8_t  port_id_len;
    BOOL     port_id_vld;
    uint8_t  *port_id;
    uint32_t capabilities;
    BOOL     ios_version_vld;
    uint8_t  ios_version_len;
    uint8_t  *ios_version;
    uint8_t  platform_len;
    BOOL     platform_vld;
    uint8_t  *platform;
} cdp_packet_t ;



// Function that checks a CDP ( checksum verification ) and passon the CDP information to the LLDP entry table.
vtss_rc cdp_frame_decode(uint8_t port_no, const uint8_t  *const frame, uint16_t len);

uint16_t cdp_checksum(uchar *data, uint length);


