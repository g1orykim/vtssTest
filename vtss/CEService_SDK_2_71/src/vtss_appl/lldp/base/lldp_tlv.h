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


#ifndef LLDP_TLV_H
#define LLDP_TLV_H

#define LLDP_CHASSIS_ID_SUBTYPE_MAC_ADDRESS 4
#define LLDP_PORT_ID_SUBTYPE_LOCAL 7

typedef enum {
    LLDP_TLV_BASIC_MGMT_END_OF_LLDPDU      = 0,
    LLDP_TLV_BASIC_MGMT_CHASSIS_ID         = 1,
    LLDP_TLV_BASIC_MGMT_PORT_ID            = 2,
    LLDP_TLV_BASIC_MGMT_TTL                = 3,
    LLDP_TLV_BASIC_MGMT_PORT_DESCR         = 4,
    LLDP_TLV_BASIC_MGMT_SYSTEM_NAME        = 5,
    LLDP_TLV_BASIC_MGMT_SYSTEM_DESCR       = 6,
    LLDP_TLV_BASIC_MGMT_SYSTEM_CAPA        = 7,
    LLDP_TLV_BASIC_MGMT_MGMT_ADDR          = 8,
    LLDP_TLV_ORG_TLV                       = 127
} lldp_tlv_t;


// Organizationally TLVs have some unique identifier(OUI) to indentify the type of TLVs.
#define  LLDP_TLV_OUI_MED   = {0x00,0x12,0xBB}; //  Section 10.1 in TIA-1057
// #define  LLDP_TLV_OUI_8023  = {0x00,0x12,0x0F}; //  Figure 33-26 in IEEE802.3at/D3







lldp_u16_t lldp_tlv_add(lldp_u8_t *buf, lldp_u16_t cur_len, lldp_tlv_t tlv, lldp_port_t port);
lldp_u16_t lldp_tlv_add_zero_ttl (lldp_u8_t *buf, lldp_u16_t cur_len);
lldp_u32_t lldp_tlv_mgmt_addr_len (void);

#define lldp_tlv_get_port_id_subtype() LLDP_PORT_ID_SUBTYPE_LOCAL
#define lldp_tlv_get_chassis_id_subtype() LLDP_CHASSIS_ID_SUBTYPE_MAC_ADDRESS
#define lldp_tlv_get_system_name(b) lldp_os_get_system_name(b)
#define lldp_tlv_get_system_descr(b) lldp_os_get_system_descr(b)
lldp_u8_t lldp_tlv_get_local_port_id (lldp_port_t port, lldp_8_t *port_str);
char lldp_tlv_get_mgmt_addr_len (void);
char lldp_tlv_get_mgmt_addr_subtype (void);
char lldp_tlv_get_mgmt_if_num_subtype (void);
char lldp_tlv_get_mgmt_oid (void);
int lldp_tlv_get_system_capabilities (void);
int lldp_tlv_get_system_capabilities_ena (void);
void set_tlv_type_and_length (lldp_u8_t *buf, lldp_tlv_t tlv_type, lldp_u16_t tlv_info_string_len);
lldp_u16_t set_tlv_type_and_length_non_zero_len (lldp_u8_t *buf, lldp_tlv_t tlv_type, lldp_u16_t tlv_info_string_len);
#endif

