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
// ************************************************************************
// This file contains function for for analysing CDP (Cisco Discovery Protocol )
// frames. The design specification for this module is included in DS0138.
// ************************************************************************


/* ************************************************************************
 * Includes
 * ************************************************************************ */

#include "lldp.h"
#include "lldp_tlv.h"
#include "main.h"
#include "cdp_analyse.h"
#include "vtss_trace_api.h"
#include <network.h> // For htons



/* ************************************************************************
 * Defines
 * ************************************************************************ */
// Conversion from network formart to out host format.
#define CDP_NTOHS(data_ptr)  ((*data_ptr << 8) + (*(data_ptr+1)))
#define CDP_NTOHL(data_ptr)  ((*data_ptr << 24) + (*(data_ptr+1) << 16) + (*(data_ptr+2) << 8) + (*(data_ptr+3)))

/* ************************************************************************
 * Typedefs and enums
 * ************************************************************************ */

/* ************************************************************************
 * Prototypes for local functions
 * ************************************************************************ */

/* ************************************************************************
 * Public data
 * ************************************************************************ */

/* ************************************************************************
 * Local data
 * ************************************************************************ */
static uint8_t cdp_frame_buffer[MAX_CDP_FRAME_SIZE]; // Buffer for the frame currently being analysed.
static lldp_u8_t lldp_frame_buffer[MAX_CDP_FRAME_SIZE];// Buffer for containing the LLDP frame being constructed.
static cdp_packet_t cdp_packet;        // Stucture containing the CDP information we supports





/* ************************************************************************
 * Functions
 * ************************************************************************ */

// Function for making sure that we don't exceed the lldp_frame_buffer array
static uint16_t incr_byte_pos(uint16_t *byte_pos, uint16_t increment_value)
{
    if (*byte_pos + increment_value > MAX_CDP_FRAME_SIZE) {
        T_E("CDP frame size error, byte_pos = %d, increment_value = %d", *byte_pos, increment_value);
    } else  {
        *byte_pos += increment_value;
    }
    return *byte_pos;
}

// Function for converting CDP information to a LLDP frame. When the the
// LLDP frame is constructed the frame is passed on to the LLDP module. (See DS0138)
static void cdp2lldp (const cdp_packet_t cdp_pkt, uint8_t port_no)
{
    uint16_t byte_pos = 0; // The currect byte in the constructed LLDP frame
    T_RG(TRACE_GRP_CDP, "Entering cdp2lldp");

    static char empty_string[1] = " "; // We need to be able to point to a empty string for because
    // the CDP frame doesn't contain informartion about port description.

    uint8_t tlv_len ;
    lldp_u8_t *lldp_frame_ptr  = &lldp_frame_buffer[0];  // Point to the buffer

    byte_pos = 14; // The TLV information start at byte 14 in a LLDP frame


    //
    // CDP device id converted into LLDP chassis ID


    if (cdp_pkt.device_id_vld) {
        // Add TLV to the constructed LLDP frame                                         // Len + 1 to include the subtype
        set_tlv_type_and_length(&lldp_frame_ptr[byte_pos], LLDP_TLV_BASIC_MGMT_CHASSIS_ID, cdp_pkt.device_id_len + 1);
        (void) incr_byte_pos(&byte_pos, 2);

        lldp_frame_ptr[byte_pos] = 7; // Subtype
        (void) incr_byte_pos(&byte_pos, 1);
        T_IG(TRACE_GRP_CDP, "Subtype Add for CDP Frame, byte_pos = %d", byte_pos);


        // Make sure that we doesn't exceeds the allowed TLV length.
        tlv_len = cdp_pkt.device_id_len;
        if (cdp_pkt.device_id_len >= MAX_CHASSIS_ID_LENGTH) {
            tlv_len = MAX_CHASSIS_ID_LENGTH;
        }

        memcpy(lldp_frame_ptr + byte_pos, cdp_pkt.device_id, tlv_len);
        (void) incr_byte_pos(&byte_pos, tlv_len);
    }


    //
    // Port ID
    //
    if (cdp_pkt.port_id_vld) {
        tlv_len = cdp_pkt.port_id_len;
        if (tlv_len >= MAX_PORT_ID_LENGTH) {
            tlv_len = MAX_PORT_ID_LENGTH;
        }
        T_NG(TRACE_GRP_CDP, "LLDP_TLV_BASIC_MGMT_PORT_ID,tlv_len = %d", tlv_len);
        // Len + 1 to include the subtype
        set_tlv_type_and_length(&lldp_frame_ptr[byte_pos], LLDP_TLV_BASIC_MGMT_PORT_ID, tlv_len + 1);
        (void) incr_byte_pos(&byte_pos, 2);



        lldp_frame_ptr[byte_pos] = 7; // subtype
        (void) incr_byte_pos(&byte_pos, 1);

        memcpy(lldp_frame_ptr + byte_pos, cdp_pkt.port_id, tlv_len);
        (void) incr_byte_pos(&byte_pos, tlv_len);
    }

    //
    // TTL
    //

    set_tlv_type_and_length(&lldp_frame_ptr[byte_pos], LLDP_TLV_BASIC_MGMT_TTL, 2);
    (void) incr_byte_pos(&byte_pos, 2);

    lldp_frame_ptr[byte_pos] = 0; // CDP uses 1 byte for TTL while LLDP uses 2 bytes. Set LLDP MSB byte to 0
    (void) incr_byte_pos(&byte_pos, 1);
    lldp_frame_ptr[byte_pos] = cdp_pkt.ttl;
    (void) incr_byte_pos(&byte_pos, 1);


    //
    // Port Description and system name is not part of the CDP frame, but must be included in the LLDP frame.
    // Add a <SPACE> as string
    //
    tlv_len = (uint8_t) sizeof(empty_string);
    set_tlv_type_and_length(&lldp_frame_ptr[byte_pos], LLDP_TLV_BASIC_MGMT_PORT_DESCR, tlv_len);
    (void) incr_byte_pos(&byte_pos, 2);

    memcpy(lldp_frame_ptr + byte_pos, &empty_string[0], tlv_len);
    (void) incr_byte_pos(&byte_pos, tlv_len);

    set_tlv_type_and_length(&lldp_frame_ptr[byte_pos], LLDP_TLV_BASIC_MGMT_SYSTEM_NAME, tlv_len);
    (void) incr_byte_pos(&byte_pos, 2);

    memcpy(lldp_frame_ptr + byte_pos, &empty_string[0], tlv_len);
    (void) incr_byte_pos(&byte_pos, tlv_len);

    //
    // CDP  Version & Platform is converted into LLDP system decription
    //

    uint8_t platform_tlv_len = 0;
    uint8_t version_tlv_len = 0;

    // Make sure that we don't get a memory overrun. Subtract 1 because we wnat to add a new-line between version and Platform
    if (cdp_pkt.ios_version_vld) {
        version_tlv_len = cdp_pkt.ios_version_len;
        if (version_tlv_len > MAX_SYSTEM_NAME_LENGTH - 1) {
            version_tlv_len = MAX_SYSTEM_NAME_LENGTH - 1;
        } else if (version_tlv_len == 0) {
            version_tlv_len = 1;
        }
        T_NG(TRACE_GRP_CDP, "version_tlv_len = %d", version_tlv_len);

        cdp_pkt.ios_version[version_tlv_len++] = 0x0A; // Add newline between version and plarform strings
        T_NG(TRACE_GRP_CDP, "version_tlv_len = %d", version_tlv_len);
    }

    if (cdp_pkt.platform_vld) {
        // Make sure that we don't get a memory overrun. Truncate platform string in case that version and platform
        // exceeds MAX_SYSTEM_NAME_LENGTH
        platform_tlv_len = cdp_pkt.platform_len ;
        if (platform_tlv_len + version_tlv_len >= MAX_SYSTEM_NAME_LENGTH) {
            platform_tlv_len = MAX_SYSTEM_NAME_LENGTH - version_tlv_len;
        }

    }

    if (cdp_pkt.ios_version_vld || cdp_pkt.platform_vld) {
        tlv_len = platform_tlv_len + version_tlv_len;
        T_DG(TRACE_GRP_CDP, "tlv_len = %u, platform_tlv_len = %u, version_tlv_len = %u", tlv_len, platform_tlv_len, version_tlv_len);

        // Set type length to system description
        set_tlv_type_and_length(&lldp_frame_ptr[byte_pos], LLDP_TLV_BASIC_MGMT_SYSTEM_DESCR, tlv_len);

        (void) incr_byte_pos(&byte_pos, 2);
    }

    if (cdp_pkt.ios_version_vld) {
        // Add version and platform strings
        T_NG(TRACE_GRP_CDP, "tlv_len = %d", version_tlv_len);
        memcpy(lldp_frame_ptr + byte_pos, cdp_pkt.ios_version, version_tlv_len);
        (void) incr_byte_pos(&byte_pos, version_tlv_len);
    }


    if (cdp_pkt.platform_vld) {
        T_NG(TRACE_GRP_CDP, "tlv_len = %d", platform_tlv_len);
        memcpy(lldp_frame_ptr + byte_pos, cdp_pkt.platform, platform_tlv_len);
        (void) incr_byte_pos(&byte_pos, platform_tlv_len);
    }


    //
    // System capabilities - Capbilites is not the same for CDP and LLDP. Conversion must be made.
    //
    set_tlv_type_and_length(&lldp_frame_ptr[byte_pos], LLDP_TLV_BASIC_MGMT_SYSTEM_CAPA, 4);
    (void) incr_byte_pos(&byte_pos, 2);


    int8_t capa_bit ;
    uint8_t system_capabilities = 0;

    for (capa_bit = 0 ; capa_bit < 7; capa_bit ++) {
        T_NG(TRACE_GRP_CDP, " 0x%X, capa_bit = %u, cdp_pkt.capabilities = 0x%X " , cdp_pkt.capabilities & (1 << capa_bit), capa_bit, cdp_pkt.capabilities);
        if (cdp_pkt.capabilities & (1 << capa_bit)) {
            switch (capa_bit) {
            case 0 :
                system_capabilities |= 1 << 4;
                break;
            case 1 :
            case 2 :
                system_capabilities |= 1 << 2;
                break;

            case 6 :
                system_capabilities |= 1 << 1;
                break;
            default:
                break;
            }
        }
    }
    if (system_capabilities == 0) {
        system_capabilities = 1;
    };

    T_DG(TRACE_GRP_CDP, "system_capabilities = 0x%X", system_capabilities);
    lldp_frame_ptr[byte_pos] = 0;
    (void) incr_byte_pos(&byte_pos, 1);
    lldp_frame_ptr[byte_pos] = system_capabilities;  // Set Enable bits
    (void) incr_byte_pos(&byte_pos, 1);
    lldp_frame_ptr[byte_pos] = 0;
    (void) incr_byte_pos(&byte_pos, 1);
    lldp_frame_ptr[byte_pos] = system_capabilities;  // Set capabilities bits
    (void) incr_byte_pos(&byte_pos, 1);

    //
    // Management address
    //
    if (cdp_pkt.address_vld) {
        tlv_len = cdp_pkt.address_len;
        if (tlv_len > 31) {
            tlv_len = 31;
        }

        T_DG(TRACE_GRP_CDP, "address_len = %u", cdp_pkt.address_len);
        set_tlv_type_and_length(&lldp_frame_ptr[byte_pos], LLDP_TLV_BASIC_MGMT_MGMT_ADDR, tlv_len + 2 + 5);
        (void) incr_byte_pos(&byte_pos, 2);

        lldp_frame_ptr[byte_pos] = cdp_pkt.address_len + 1;
        (void) incr_byte_pos(&byte_pos, 1);
        lldp_frame_ptr[byte_pos] = cdp_pkt.address_protocol_type;
        (void) incr_byte_pos(&byte_pos, 1);


        memcpy(lldp_frame_ptr + byte_pos, cdp_pkt.address, cdp_pkt.address_len);

        (void) incr_byte_pos(&byte_pos, cdp_pkt.address_len);


        lldp_frame_ptr[incr_byte_pos(&byte_pos, 1)] = 2; /* ifIndex */

        /* Interface number */
        lldp_u32_t mgmt_if_index = 0;
        lldp_frame_ptr[byte_pos] = (mgmt_if_index >> 24) & 0xFF;
        (void) incr_byte_pos(&byte_pos, 1);
        lldp_frame_ptr[byte_pos] = (mgmt_if_index >> 16) & 0xFF;
        (void) incr_byte_pos(&byte_pos, 1);
        lldp_frame_ptr[byte_pos] = (mgmt_if_index >>  8) & 0xFF;
        (void) incr_byte_pos(&byte_pos, 1);
        lldp_frame_ptr[byte_pos] = (mgmt_if_index >>  0) & 0xFF;
        (void) incr_byte_pos(&byte_pos, 1);
    }

    //
    // END TLV
    //

    lldp_frame_ptr[byte_pos] = 0;
    (void) incr_byte_pos(&byte_pos, 1);
    lldp_frame_ptr[byte_pos] = 0;
    (void) incr_byte_pos(&byte_pos, 1);

    //
    // Update LLDP entry
    //
    lldp_frame_received(port_no, lldp_frame_ptr, byte_pos, 0);
    T_DG(TRACE_GRP_CDP, "Existing cdp2lldp");



    return;
}


//
// Function for calucating checksum for CDP data
//
uint16_t cdp_basic_checksum(uchar *data, uint length)
{
    long sum = 0;
    const uint16_t *d = (const uint16_t *)data; // Convert to 16 bits

    while (length > 1) {
        T_RG(TRACE_GRP_CDP, "*d = 0x%X, sum = %lu, len = %u", *d, sum, length);
        sum += *d++;
        length -= 2;
    }

    if (length) {
        sum += htons(*(const uint8_t *)d);
        T_RG(TRACE_GRP_CDP, "*d = 0x%X, sum = %lu, len = %u", *d, sum, length);
    }

    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);

    T_NG(TRACE_GRP_CDP, "checksum = 0x%X", (uint16_t) sum);

    return (uint16_t)sum;
}


uint16_t cdp_checksum(uchar *data, uint length)
{
    return (0xFFFF - cdp_basic_checksum(data, length));
}


//
// Function that returns VTSS_OK if the checksum is correct for the given CDP frame
//
// IN : data - Pointer to the CDP frame
//      length - The length of the CDP frame
//
// Return : VTSS_OK if check-sum is correct else VTSS_UNSPECIFIED_ERROR

vtss_rc is_checksum_correct (uchar *data, uint length)
{
    uint16_t checksum = ~cdp_basic_checksum(data, length);
    if (checksum == 0) {
        T_NG(TRACE_GRP_CDP, "CDP checksum correct");
        return VTSS_OK;
    } else {
        T_IG(TRACE_GRP_CDP, "CDP checksum failure, checksum = 0x%X", checksum);
        return VTSS_UNSPECIFIED_ERROR;
    }
}

//
// Decode CDP TLVs
//
vtss_rc cdp_tlv_decode (uint8_t *cdp_data_ptr, uint16_t cdp_data_len)
{
    int16_t cdp_byte_analysed = 0; // Contains the byte currently being analysed.
    uint16_t tlv_type; // The TLV type
    uint16_t tlv_len = 0; // The length of the TLV
    uint8_t *address_ptr = cdp_data_ptr;

    T_NG(TRACE_GRP_CDP, "cdp_data_len = %u", cdp_data_len);
    memcpy(&cdp_frame_buffer[0], cdp_data_ptr, cdp_data_len);

    // Default none of the TLVs are valid
    cdp_packet.device_id_vld = FALSE;
    cdp_packet.address_vld = FALSE;
    cdp_packet.port_id_vld = FALSE;
    cdp_packet.ios_version_vld = FALSE;
    cdp_packet.platform_vld = FALSE;

    while (cdp_byte_analysed < cdp_data_len) {
        tlv_type = CDP_NTOHS(cdp_data_ptr); // Convert to 16 bits
        tlv_len  = CDP_NTOHS((cdp_data_ptr + 2)); // Convert to 16 bits
        T_NG(TRACE_GRP_CDP, "tlv = 0x%X, tlv type = 0x%X", tlv_len, tlv_type);

        // Make sure that we don't run forever.
        if (tlv_len == 0 || tlv_len > cdp_data_len ) {
            T_W("CDP frame contained invalid TLV length, tlv_len = %u, cdp_data_len = %u", tlv_len, cdp_data_len);
            return VTSS_RC_ERROR;
        }


        switch (tlv_type) {

        case CDP_TYPE_DEVICE_ID:
            cdp_packet.device_id_vld = TRUE;
            cdp_packet.device_id = &cdp_frame_buffer[4 + cdp_byte_analysed] ;
            cdp_packet.device_id_len = tlv_len - 4;
            T_NG(TRACE_GRP_CDP, "TLV = CDP_TYPE_DEVICE_ID, device_id = %s, device_id_len = %d", cdp_packet.device_id, cdp_packet.device_id_len);
            break;

        case CDP_TYPE_ADDRESS:
            cdp_packet.address_vld = TRUE;
            address_ptr = cdp_frame_buffer + 4 + cdp_byte_analysed;
            int32_t num_of_addresses = CDP_NTOHL (address_ptr) ;
            T_RG(TRACE_GRP_CDP, "Address : 0x%X 0x%X 0x%X 0x%X", *address_ptr, *(address_ptr + 1), *(address_ptr + 2), *(address_ptr + 3));
            T_NG(TRACE_GRP_CDP, "Address : num_of_addresses = %d", num_of_addresses);
            address_ptr += 4;


            uint8_t address_protocol_type;
            uint8_t address_protocol_type_len;
            uint8_t address_len ;
            uint8_t *protocol ;

            // I know that the CDP can contain mulitple addresses. We only support one for the moment
//      for (i = num_of_addresses ; i > 0; i--) {
            address_protocol_type = *address_ptr++;
            T_NG(TRACE_GRP_CDP, "address_protocol_type = %d", address_protocol_type);

            address_protocol_type_len = *address_ptr++;
            T_NG(TRACE_GRP_CDP, "address_protocol_type_len = %d", address_protocol_type_len);

            protocol = address_ptr;
            address_ptr += address_protocol_type_len;

            address_len = CDP_NTOHS(address_ptr);
            address_ptr += 2;

            cdp_packet.address_protocol_type = address_protocol_type;
            cdp_packet.address_len = address_len;
            cdp_packet.address     = address_ptr;

            T_NG(TRACE_GRP_CDP, "protocol = 0x%X, address_len = 0x%X, address = %s", *protocol, address_len, address_ptr);

            //    }
            T_NG(TRACE_GRP_CDP, "TLV = CDP_TYPE_ADDRESS");
            break;

        case CDP_TYPE_PORT_ID:
            cdp_packet.port_id_vld = TRUE;
            cdp_packet.port_id = &cdp_frame_buffer[4 + cdp_byte_analysed] ;
            cdp_packet.port_id_len = tlv_len - 4;
            T_NG(TRACE_GRP_CDP, "TLV = CDP_TYPE_PORT_ID, ID = %s, len = %d", cdp_packet.port_id, cdp_packet.port_id_len);
            break;

        case CDP_TYPE_CAPABILITIES:
            address_ptr = cdp_frame_buffer + 4 + cdp_byte_analysed ;
            cdp_packet.capabilities = CDP_NTOHL(address_ptr) ;
            T_DG(TRACE_GRP_CDP, "TLV = CDP_TYPE_CAPABILITIES, capabilities = 0x%X", cdp_packet.capabilities);
            break;

        case CDP_TYPE_IOS_VERSION:
            cdp_packet.ios_version_vld = TRUE;
            cdp_packet.ios_version = &cdp_frame_buffer[4 + cdp_byte_analysed] ;
            cdp_packet.ios_version_len = tlv_len - 4;
            T_NG(TRACE_GRP_CDP, "TLV = CDP_TYPE_IOS_VERSION, ios_version = %s, len = %d", cdp_packet.ios_version, cdp_packet.ios_version_len);
            break;

        case CDP_TYPE_PLATFORM:
            cdp_packet.platform_vld = TRUE;
            cdp_packet.platform = &cdp_frame_buffer[4 + cdp_byte_analysed] ;
            cdp_packet.platform_len = tlv_len - 4;
            T_RG(TRACE_GRP_CDP, "TLV = CDP_TYPE_PLATFORM, platform = %s", cdp_packet.platform);
            break;

        case CDP_TYPE_IP_PREFIX:
        case CDP_TYPE_PROTOCOL_HELLO:
        case CDP_TYPE_VTP_MGMT_DOMAIN:
        case CDP_TYPE_NATIVE_VLAN:
        case CDP_TYPE_DUPLEX:
        case CDP_TYPE_UNKNOWN_0x000c:
        case CDP_TYPE_UNKNOWN_0x000d:
        case CDP_TYPE_APPLIANCE_REPLY:
        case CDP_TYPE_APPLIANCE_QUERY :
        case CDP_TYPE_POWER_CONSUMPTION:
        case CDP_TYPE_MTU:
        case CDP_TYPE_EXTENDED_TRUST:
        case CDP_TYPE_UNTRUSTED_COS:
        case CDP_TYPE_SYSTEM_NAME:
        case CDP_TYPE_SYSTEM_OID:
        case CDP_TYPE_MGMT_ADDRESS:
        case CDP_TYPE_LOCATION:
        case CDP_TYPE_UNKNOWN_0x001A:
            T_NG(TRACE_GRP_CDP, "Action for TLV not defined (0x%X) ", tlv_type);
            break;
        default :
            T_DG(TRACE_GRP_CDP, "CDP frame contained a unknown TLV (0x%X) ", tlv_type);
        }


        cdp_data_ptr += tlv_len; // Point to next TLV
        cdp_byte_analysed += tlv_len;
    }

    return VTSS_OK;
}

//
// Function that makes sure that the CDP frame contains correct information and passes
// on the information to the LLDP module.
//
// IN : port_no : The port at which the CDP frame was received
//      frame   : Pointer to the CDP frame.
//      frm_len : The length of the CDP frame.
//
vtss_rc cdp_frame_decode(uint8_t port_no, const uint8_t  *const frame, uint16_t frm_len)
{
    const uint8_t cdp_data_start = 22; // CDP data starts at the 22th byte into the frame.

    uint8_t *cdp_data_ptr = (uint8_t *)frame + cdp_data_start;  // Pick out the cdp data
    cdp_packet.len = frm_len - cdp_data_start; // Calulate the length of the CDP packet frame


    if (frm_len > MAX_CDP_FRAME_SIZE) {
        // Making sure that we don't gets a buffer overrun.
        return VTSS_UNSPECIFIED_ERROR;
    } else {

        //Verify checksum
        (void) is_checksum_correct(cdp_data_ptr, cdp_packet.len);
        cdp_packet.version       = *(cdp_data_ptr); // Pick out CDP version
        cdp_packet.ttl           = *(cdp_data_ptr + 1); // pick out the time to live value
        cdp_packet.checksum = CDP_NTOHS(cdp_data_ptr); // Convert to 16 bits
        T_NG(TRACE_GRP_CDP, "Version = %d, ttl = %d, checksum = 0x%X", cdp_packet.version, cdp_packet.ttl, cdp_packet.checksum);

        cdp_data_ptr += 4; // Point to first TLV
        VTSS_RC(cdp_tlv_decode(cdp_data_ptr, cdp_packet.len - 4)); // Call decoding of the TLVs


        cdp2lldp(cdp_packet, port_no);
        return VTSS_OK;
    }
}


