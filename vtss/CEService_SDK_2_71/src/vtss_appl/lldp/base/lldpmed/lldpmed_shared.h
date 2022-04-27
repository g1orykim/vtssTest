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


/******************************************************************************
* Types / functions defintions for LLDP-MED shared by RX and TX
******************************************************************************/

#ifndef LLDPMED_SHARED_H
#define LLDPMED_SHARED_H
#include "lldp_basic_types.h"

// Table 11, TIA1057
typedef enum {
    LLDPMED_DEVICE_TYPE_NOT_DEFINED,
    LLDPMED_DEVICE_TYPE_ENDPOINT_CLASS_I,
    LLDPMED_DEVICE_TYPE_ENDPOINT_CLASS_II,
    LLDPMED_DEVICE_TYPE_ENDPOINT_CLASS_III,
    LLDPMED_DEVICE_TYPE_NETWORK_CONNECTIVITY,
    LLDPMED_DEVICE_TYPE_RESERVED
} lldpmed_device_type_t;


// Table 12, TIA1057
typedef enum {
    LLDPMED_LOCATION_INVALID,
    LLDPMED_LOCATION_COORDINATE,
    LLDPMED_LOCATION_CIVIC,
    LLDPMED_LOCATION_ECS
} lldpmed_location_type_t;



// Location Data Format, Table 14, TIA1057
typedef enum {
    COORDINATE_BASED = 1,
    CIVIC = 2,
    ECS = 3
} lldpmed_location_data_format_t;



// LLDP-MED TLV subtype for a network connectivity, Table 5, TIA1057
typedef enum {
    LLDPMED_TLV_SUBTYPE_CAPABILITIIES  = 1,
    LLDPMED_TLV_SUBTYPE_NETWORK_POLICY = 2,
    LLDPMED_TLV_SUBTYPE_LOCATION_ID    = 3,
    LLDPMED_TLV_SUBTYPE_POE            = 4
} lldpmed_tlv_subtype_t;

// LLDP-MED TLV application types, Table 12,TIA1057
typedef enum {
    VOICE  = 1,
    VOICE_SIGNALING = 2,
    GUEST_VOICE    = 3,
    GUEST_VOICE_SIGNALING = 4,
    SOFTPHONE_VOICE = 5,
    VIDEO_CONFERENCING = 6,
    STREAMING_VIDEO = 7,
    VIDEO_SIGNALING = 8
} lldpmed_application_type_t;


// LLDP-MED catype for civic location, Section 3.4, Annex B, TIA1057
typedef enum {
    LLDPMED_CATYPE_A1  = 1,
    LLDPMED_CATYPE_A2  = 2,
    LLDPMED_CATYPE_A3  = 3,
    LLDPMED_CATYPE_A4  = 4,
    LLDPMED_CATYPE_A5  = 5,
    LLDPMED_CATYPE_A6  = 6,
    LLDPMED_CATYPE_PRD  = 16,
    LLDPMED_CATYPE_POD  = 17,
    LLDPMED_CATYPE_STS  = 18,
    LLDPMED_CATYPE_HNO  = 19,
    LLDPMED_CATYPE_HNS  = 20,
    LLDPMED_CATYPE_LMK  = 21,
    LLDPMED_CATYPE_LOC  = 22,
    LLDPMED_CATYPE_NAM  = 23,
    LLDPMED_CATYPE_ZIP  = 24,
    LLDPMED_CATYPE_BUILD = 25,
    LLDPMED_CATYPE_UNIT = 26,
    LLDPMED_CATYPE_FLR  = 27,
    LLDPMED_CATYPE_ROOM = 28,
    LLDPMED_CATYPE_PLACE = 29,
    LLDPMED_CATYPE_PCN   = 30,
    LLDPMED_CATYPE_POBOX = 31,
    LLDPMED_CATYPE_ADD_CODE = 32
} lldpmed_catype_t;
#define LLDPMED_CATYPE_CNT 23 // Number of elements in lldpmed_catype_t. Keep this in sync with the lldpmed_catype_t


// See section 10.2.3, TIA1057
typedef struct {
    lldpmed_application_type_t application_type;
    lldp_bool_t   tagged_flag;
    lldp_u16_t    vlan_id;
    lldp_u8_t     l2_priority;
    lldp_u8_t     dscp_value ;
    lldp_bool_t   in_use; // Signaling if this policy is use.
} lldpmed_policy_t;

// Section 10.2.3.6, TIA1057
#define LLDPMED_L2_PRIORITY_MIN 0
#define LLDPMED_L2_PRIORITY_MAX 7

// Section 10.2.3.7, TIA1057
#define LLDPMED_DSCP_MIN 0
#define LLDPMED_DSCP_MAX 63



// Table 13, TIA1057
#define LLDPMED_VID_MIN 1
#define LLDPMED_VID_MAX 4095

// Define Policy min and max
#define LLDPMED_POLICY_NONE -1 // Defines that no policy is defined
#define LLDPMED_POLICY_MIN 0 // Defines the lowest policy number
#define LLDPMED_POLICY_MAX 31 // Defines the highest policy number.  (I have not found 
// a number specified anywhere, so I have choosen 31, and the
//code only support lower numbers = NEVER SET ABOVE 31).

#define LLDPMED_POLICIES_CNT LLDPMED_POLICY_MAX - LLDPMED_POLICY_MIN + 1 // Defines how many different policies the switch supports.

typedef lldpmed_policy_t lldpmed_policies_table_t[LLDPMED_POLICIES_CNT]; // Table containing all policies

typedef lldp_bool_t lldpmed_port_policies_t[LLDPMED_POLICIES_CNT]; // Array containing which policies a port has "enabled"
typedef lldpmed_port_policies_t lldpmed_ports_policies_t[LLDP_PORTS]; // Array with policies for all ports (Becomes a two diminsion array).


#define DEVICE_CLASS_TYPE 0x4; // We are always a network connectivity device, Table 11, TIA1057

typedef  lldp_16_t civic_str_ptr_t[LLDPMED_CATYPE_CNT]; // Array of pointers to point within the ca_value string list
typedef  lldpmed_catype_t civic_ca_type_t[LLDPMED_CATYPE_CNT]; // Array the give the ca type for the corresponding pointer in civic_str_ptr_t


#define CIVIC_CA_VALUE_LEN_MAX 250 // Figure 10, TIA1057
#define ECS_VALUE_LEN_MAX 25 // Emergency call service, Figure 11, TIA1057

typedef struct {
    // The civic location consists of multiple stings. In principle they can all be of up to 250 bytes long, but all together they
    // must also be within 250 bytes (Figure 10, TIA1057)
    // In order not to reserved a lot of space in flash, all the individual strings are all concatenated within one single string, and we use
    // an array of pointers to point at each individual string.

    lldp_8_t ca_value[CIVIC_CA_VALUE_LEN_MAX]; // Single string containg all the civic strings (250 bytes, Figure 10, TIA1057)
    civic_str_ptr_t civic_str_ptr_array; // Array of pointer to point at each individual string
    civic_ca_type_t civic_ca_type_array; // Array the give the ca type for the corresponding pointer in civic_str_ptr_t
} lldpmed_location_civic_info_t;

// Types for alititude,  meters or floor, RFC3825,July2004 section 2.1
typedef enum {
    METERS = 1,
    FLOOR  = 2,
} lldpmed_at_type_t;

typedef enum {
    NORTH = 0,
    SOUTH = 1,
} lldpmed_latitude_dir_t;

typedef enum {
    EAST = 0,
    WEST = 1,
} lldpmed_longitude_dir_t;


// RFC3825,July 2004, section 2.1
typedef enum {
    WGS84 = 1,
    NAD83_NAVD88 = 2,
    NAD83_MLLW = 3,
} lldpmed_datum_t;

#define CA_COUNTRY_CODE_LEN 3

// See section 10.2.3, TIA1057
typedef struct {
    // Coordinated based location, Figure 9, TIA1057
    lldp_32_t latitude;
    lldpmed_latitude_dir_t latitude_dir; // Latitude direction, 0 = North, 1 = South
    lldp_32_t longitude;
    lldpmed_longitude_dir_t longitude_dir; // Latitude direction, 0 = East, 1 = West
    lldp_32_t altitude;
    lldpmed_at_type_t altitude_type;// Coordinate based location - RFC3825, 1 = meters, 2 =floors
    lldpmed_datum_t datum;

    lldpmed_location_civic_info_t civic; // Civic Address Location infomation
    lldp_8_t ca_country_code[CA_COUNTRY_CODE_LEN]; // Figure 10, TIA1057
    lldp_8_t ecs[ECS_VALUE_LEN_MAX + 1]; // Emergency call service, Figure 11, TIA1057. Adding 1 for making space for "\0"
} lldpmed_location_info_t;

// Defining the LLDPMED optional TLVs as the bit the lldpmed_optional_tlv_t bit vector.
typedef enum {
    OPTIONAL_TLV_CAPABILITIES_BIT = 1 << 0, // Bit 0 is the capabilities TLV, See TIA-1057, MIB  lldpXMedPortConfigTLVsTxEnable
    OPTIONAL_TLV_POLICY_BIT = 1 << 1,       // Bit 1 is the network Policy TLV, see lldpXMedPortConfigTLVsTxEnable MIB.
    OPTIONAL_TLV_LOCATION_BIT = 1 << 2,     // Bit 2 is the location TLV, see lldpXMedPortConfigTLVsTxEnable MIB.
} lldpmed_optional_tlv_bits_t;

typedef lldp_u8_t lldpmed_optional_tlv_t[LLDP_PORTS]; // Enable/disable of the LLDP-MED optional TLVs (See TIA-1057, MIB LLDPXMEDPORTCONFIGTLVSTXENABLE).
typedef lldp_u8_t lldpmed_notification_ena_t[LLDP_PORTS]; // Enable/Disable of SNMP trap notification - See MIBs

// Defines possible actions for "global" medFastStart timer
typedef enum {
    SET,
    DECREMENT,
} lldpmed_fast_start_repeat_count_t;



#define TUDE_MULTIPLIER 10000 // Defines the number of digits for latitude, longitude and altitude ( 10 = 1 digit, 100 = 2 digits and so on ).
#define TUDE_DIGIT 4  // Keep this in sync with TUDE_MULTIPLIER


#define LLDPMED_LATITUDE_VALUE_MIN 0 // Defines lowest allowed value for latitude
#define LLDPMED_LATITUDE_VALUE_MAX 90 * TUDE_MULTIPLIER // Defines highest allowed value for latitude
#define LLDPMED_LONGITUDE_VALUE_MIN 0 // Defines lowest allowed value for longitude
#define LLDPMED_LONGITUDE_VALUE_MAX 180 * TUDE_MULTIPLIER // Defines highest allowed value for longitude
#define LLDPMED_ALTITUDE_VALUE_MIN -32767 * TUDE_MULTIPLIER // Defines lowest allowed value for altitude
#define LLDPMED_ALTITUDE_VALUE_MAX 32767 * TUDE_MULTIPLIER // Defines highest allowed value for altitude

#endif



