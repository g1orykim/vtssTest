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


/*****************************************************************************
* This file contains code for handling received LLDP-MED frames. LLDP-MED is
* defined in TIA-1057. The Version used for delevoping this code is ANSI/TIA-1057-2006
* released at April 6, 2006.
******************************************************************************/

#include "lldp.h"
#include "vtss_lldp.h"
#include "lldp_tlv.h"
#include "lldp_os.h"
#include "lldp_private.h"
#include "lldpmed_shared.h"
#include "lldpmed_tx.h"
#include "lldpmed_rx.h"
#include "port_custom_api.h"
#include "vtss_api_if_api.h"

#ifdef VTSS_SW_OPTION_POE
#include "poe_custom_api.h"
#endif

//
// Converting a 64 bits number to a integer and fraction part.
//
// In : tude_val - The 64 bits number that shall be converted.
//      negative_number - Indicate that the number is negative ( The direction bit for the latitude and longitude ).
//      fraction_bits_cnt - Telling how many bits that are used for the fraction part.
//
// In/Out: Fraction - 32 bits number that is going to contain the fraction.
//         int_part - 32 bits number that is going to contain the integer part of the number.
//
void lldpmed_cal_fraction(lldp_64_t tude_val, lldp_u32_t *fraction, lldp_u32_t *int_part, lldp_bool_t negative_number, lldp_u32_t fraction_bits_cnt)
{
    lldp_64_t tude_val_local;

    if (tude_val < 0) {
        tude_val_local = ~tude_val;
        negative_number = TRUE;
    } else {
        tude_val_local = tude_val;
    }

    T_IG(TRACE_GRP_MED_TX, "tude_val:0x%llX, tude_val_local:0x%llX", tude_val, tude_val_local);
    *int_part  = tude_val_local / TUDE_MULTIPLIER; // The input from web is mulitplied with TUDE_MULTIPLIER to avoid using floating point variable.
    // Eaxmple: web input = 33.04, TUDE_MULTIPLIER = 100 gives tude_val_local = 3304. When dividing
    // 3304 with 100 we comes back to 33.

    lldp_u32_t tude_frac_part = (tude_val_local - ((lldp_64_t) * int_part * TUDE_MULTIPLIER)); // Example: tude_val_local = 3304, int_part just calculated to 33 then fraction becomes (3304 - 33*100 = 4).

    T_NG(TRACE_GRP_MED_TX, "int_part = %u, tude_frac_part = %u", *int_part, tude_frac_part);
    *fraction = 0;

    // Loop through all franction bits. 1st bit = 0.5, 2nd bit = 0.25, 3rd bit = 0.125 and so on.
    lldp_u32_t frac_cal = (lldp_u32_t)(0.5 * TUDE_MULTIPLIER);
    lldp_16_t  frac_bit;
    for (frac_bit = fraction_bits_cnt - 1; frac_bit >= 0; frac_bit --) {
        T_RG(TRACE_GRP_MED_TX, "frac_cal = %u,frac_bit = %d, tude_frac_part = %u, fraction = %u", frac_cal, frac_bit, tude_frac_part, *fraction);
        if (tude_frac_part == 0) {
            break;
        }
        if (tude_frac_part >=  frac_cal) {
            *fraction |= 1 << frac_bit;
            tude_frac_part -= frac_cal;
        }
        frac_cal /= 2;
    }

    // If it is a negative number the bits has to be inverted.
    if (negative_number == LLDP_TRUE) {
        *fraction = ~*fraction ;
        *int_part = ~*int_part;
    }
}

//
// Create LLDP-MED capabilities 16 bits word for this device
//
// Return: 16 bit bitmask for capabilities supported
lldp_u16_t lldpmed_get_capabilities_word(lldp_port_t port_idx)
{
    lldp_u16_t capa_bits = 0;
    lldp_u8_t poe_bit = 0;

#ifdef VTSS_SW_OPTION_POE
    // Get PoE hardware board configuration
    poe_custom_entry_t poe_hw_conf;
    poe_hw_conf = poe_custom_get_hw_config(port_idx, &poe_hw_conf);

    // Only set PoE if PoE is available for this port
    if (poe_hw_conf.available) {
        poe_bit = 1; // We are a PSE.
    }
#endif

    // See Table 10, TIA1057
    capa_bits  = 1 |  // LLDP-MED capbilities
#ifdef VTSS_SW_OPTION_LLDP_MED_NETWORK_POLICY
                 1 << 1 |
#endif
                 1 << 2 | // Location bit
                 poe_bit << 3 |
                 0 << 4 |
                 0 << 5;

    return capa_bits;
}

//
// Append LLDP-MED capabilities TLVs to the LLDP frame being build.
//
// In:
//
// In/Out: tlv - Pointer to buffer containing the LLDP frame
//
// Return: The length of the buffer
static lldp_u8_t lldpmed_capabilities_tlv_add(lldp_u8_t *buf, lldp_port_t port_idx )
{

    lldp_u8_t i = 0;
    lldp_u16_t capabilities_bits = lldpmed_get_capabilities_word(port_idx);

    buf[i++] = 0x01;   // Capabilities ID Subtype - Figure 6,TIA1057

    buf[i++] = (lldp_u8_t)((capabilities_bits >> 8) & 0xFF);
    buf[i++] = (lldp_u8_t)(capabilities_bits & 0xFF);


    buf[i++] = DEVICE_CLASS_TYPE; // We are always a Network Connectivity device, Table 11, TIA1057

    return i;
}

//
// Append LLDP-MED policies TLVs to the LLDP frame being build.
//
// In:
//
// In/Out: tlv - Pointer to buffer containing the LLDP frame
//
// Return: The length of the buffer
static lldp_u8_t lldpmed_network_policy_tlv_add(lldp_u8_t *buf, lldp_u8_t policy_idx)
{

    lldp_u8_t i = 0;


    lldp_struc_0_t conf;
    lldp_get_config(&conf, VTSS_ISID_LOCAL); // Get current configuration

    if (conf.policies_table[policy_idx].in_use == 0) {
        // Skip not in use policies
        T_R("Non active policy(%d) configured for port", policy_idx);
    } else {
        // Ok - Policy configured for this port. Generate TLV
        buf[i++] = 0x02;   // Policy ID Subtype - Figure 7,TIA1057
        buf[i++] = (lldp_u8_t) conf.policies_table[policy_idx].application_type; // Figure 7,TIA1057
        buf[i++] = (conf.policies_table[policy_idx].tagged_flag << 6) | ((conf.policies_table[policy_idx].vlan_id & 0xF80) >> 7 ); // Figure 7,TIA1057
        buf[i++] = (((conf.policies_table[policy_idx].vlan_id & 0x7F) << 1) |
                    ((conf.policies_table[policy_idx].l2_priority & 0x04) >> 2)); // Figure 7,TIA1057

        buf[i++] = (((conf.policies_table[policy_idx].l2_priority & 0x03) << 6)  |
                    conf.policies_table[policy_idx].dscp_value); // Figure 7,TIA1057
    }
    return i;
}

//
// Append LLDP-MED location TLVs to the LLDP frame being build. This is basically the build of TIA1057, Figure 9
//
// In:
//
// In/Out: tlv - Pointer to buffer containing the LLDP frame
//
// Return: The length of the buffer
lldp_u8_t lldpmed_coordinate_location_tlv_add(lldp_u8_t   *buf)
{
    lldp_u8_t i = 0;
    lldp_u32_t latitude_int, longitude_int;
    lldp_u32_t  altitude_int;
    lldp_u32_t fraction;

    lldp_struc_0_t conf;
    lldp_get_config(&conf, VTSS_ISID_LOCAL); // Get current configuration

    lldp_u8_t resolution = 34;// Fixed resolution for latitude and longitude to 34 bits (TIA1057, Figure 9)

    // Latitude
    lldpmed_cal_fraction(conf.location_info.latitude, &fraction, &latitude_int, (BOOL) conf.location_info.latitude_dir, 25 );
    buf[i++] = (resolution << 2) + ((latitude_int & 0x1FF) >> 7);
    buf[i++] = ((latitude_int & 0x7F) << 1) + ((fraction & 0x1000000) >> 24);
    buf[i++] = (fraction & 0xFF0000) >> 16;
    buf[i++] = (fraction & 0xFF00) >> 8;
    buf[i++] = fraction & 0xFF;

    T_DG(TRACE_GRP_MED_TX, "res:%d, latitude_int:%u, frac:%u, dir:%u", resolution, latitude_int, fraction, conf.location_info.latitude_dir);

    // Longitude
    lldpmed_cal_fraction(conf.location_info.longitude, &fraction, &longitude_int, (BOOL) conf.location_info.longitude_dir, 25 );

    buf[i++] = (resolution << 2) + ((longitude_int & 0x1FF) >> 7 );
    buf[i++] = ((longitude_int & 0x7F) << 1) + ((fraction & 0x1000000) >> 24);
    buf[i++] = (fraction & 0xFF0000) >> 16;
    buf[i++] = (fraction & 0xFF00) >> 8;
    buf[i++] = fraction & 0xFF;

    T_DG(TRACE_GRP_MED_TX, "res:%d, longitude_int:%u, frac:%u, dir:%u", resolution, longitude_int, fraction, conf.location_info.longitude_dir);

    // Altitude (The negative information is included in the altitude value, (Sign indication is not used, we set it to TRUE)
    lldpmed_cal_fraction(conf.location_info.altitude, &fraction, &altitude_int, FALSE, 8);

    resolution = 30;// Fixed resolution to 30 bits (TIA1057, Figure 9)
    buf[i++] = ((lldp_u8_t)(conf.location_info.altitude_type) << 4) + (resolution >> 2); // Figure 9, TIA1057
    buf[i++] = ((resolution & 0x3) << 6) + ((altitude_int & 0x3F0000) >> 16); // Figure 9, TIA1057
    buf[i++] = (altitude_int & 0xFF00) >> 8;
    buf[i++] = (altitude_int & 0xFF);
    buf[i++] = fraction & 0xFF;

    T_DG(TRACE_GRP_MED_TX, "res:%u, altitude_int:%u, frac:%u", resolution, altitude_int, fraction);

    buf[i++] = (lldp_u8_t) conf.location_info.datum;
    return i;
}

//
// Append LLDP-MED ECS ELIN location TLVs to the LLDP frame being build.
//
// In:
//
// In/Out: tlv - Pointer to buffer containing the LLDP frame
//
// Return: The length of the buffer
lldp_u8_t lldpmed_ecs_location_tlv_add(lldp_u8_t *buf)
{
    lldp_struc_0_t conf;
    lldp_get_config(&conf, VTSS_ISID_LOCAL); // Get current configuration

    lldp_u8_t len = strlen(conf.location_info.ecs);
    memcpy(buf, conf.location_info.ecs, len);
    T_DG(TRACE_GRP_MED_TX, "ecs = %s, len = %d", conf.location_info.ecs, len);
    return len;
}

//
// Append LLDP-MED ECS Civic location TLVs to the LLDP frame being build.
//
// In:
//
// In/Out: tlv - Pointer to buffer containing the LLDP frame
//
// Return: The length of the buffer
lldp_u8_t lldpmed_civic_location_tlv_add(lldp_u8_t   *buf)
{
    lldp_u8_t buf_index = 0;
    lldp_u8_t civic_string_index;
    int str_ptr = 0;

    lldp_u8_t ca_data_index = 0;
    lldp_8_t ca_data[CIVIC_CA_VALUE_LEN_MAX]; // Ca data. Max 250 octets, Fixgure 10, TIA1057
    lldp_u8_t ca_value_length;

    strcpy(ca_data, ""); // Initialize ca_data string

    lldp_struc_0_t conf;
    lldp_get_config(&conf, VTSS_ISID_LOCAL); // Get current configuration

    // Table for converting from index to ca type ( Table in Annex B, TIA1057)
    lldp_8_t catype_convert[LLDPMED_CATYPE_CNT] = {  (lldp_u8_t)LLDPMED_CATYPE_A1,
                                                     (lldp_u8_t)LLDPMED_CATYPE_A2,
                                                     (lldp_u8_t)LLDPMED_CATYPE_A3,
                                                     (lldp_u8_t)LLDPMED_CATYPE_A4,
                                                     (lldp_u8_t)LLDPMED_CATYPE_A5,
                                                     (lldp_u8_t)LLDPMED_CATYPE_A6,
                                                     (lldp_u8_t)LLDPMED_CATYPE_PRD,
                                                     (lldp_u8_t)LLDPMED_CATYPE_POD,
                                                     (lldp_u8_t)LLDPMED_CATYPE_STS,
                                                     (lldp_u8_t)LLDPMED_CATYPE_HNO,
                                                     (lldp_u8_t)LLDPMED_CATYPE_HNS,
                                                     (lldp_u8_t)LLDPMED_CATYPE_LMK,
                                                     (lldp_u8_t)LLDPMED_CATYPE_LOC,
                                                     (lldp_u8_t)LLDPMED_CATYPE_NAM,
                                                     (lldp_u8_t)LLDPMED_CATYPE_ZIP,
                                                     (lldp_u8_t)LLDPMED_CATYPE_BUILD,
                                                     (lldp_u8_t)LLDPMED_CATYPE_UNIT,
                                                     (lldp_u8_t)LLDPMED_CATYPE_FLR,
                                                     (lldp_u8_t)LLDPMED_CATYPE_ROOM,
                                                     (lldp_u8_t)LLDPMED_CATYPE_PLACE,
                                                     (lldp_u8_t)LLDPMED_CATYPE_PCN,
                                                     (lldp_u8_t)LLDPMED_CATYPE_POBOX,
                                                     (lldp_u8_t)LLDPMED_CATYPE_ADD_CODE
                                                  };

    for (civic_string_index = 0; civic_string_index < LLDPMED_CATYPE_CNT; civic_string_index++ ) {
        str_ptr = conf.location_info.civic.civic_str_ptr_array[civic_string_index];
        T_NG(TRACE_GRP_MED_TX, "civic_string_index =%d", civic_string_index);

        if (strlen(&conf.location_info.civic.ca_value[str_ptr])) {

            ca_value_length = strlen(&conf.location_info.civic.ca_value[str_ptr]);
            if (ca_data_index + ca_value_length + 2 > CIVIC_CA_VALUE_LEN_MAX) { // +2 = CAtype and CAlength

                // This shall never happen. Shall be cheeked in web/cli
                T_WG(TRACE_GRP_MED_TX, "Total CA Value information exceedds 250 bytes");
                return 0;
            } else {
                ca_data[ca_data_index++] = catype_convert[civic_string_index]; // Set CAtype, Section 3.3 in Annex B, TIA1057
                ca_data[ca_data_index++] = ca_value_length; // CAlength, Section 3.3 in Annex B, TIA1057
                strcpy(&ca_data[ca_data_index], &conf.location_info.civic.ca_value[str_ptr]); // CA Value, Section 3.3 in Annex B, TIA1057

                ca_data_index += ca_value_length; // Point to next string
                T_NG(TRACE_GRP_MED_TX, "conf.location_info.civic.civic_str_ptr_array[index] = %d,conf.location_info.ca_value[%d] = %s",
                     str_ptr, str_ptr, &conf.location_info.civic.ca_value[str_ptr] );
            }
        }
    }

    buf[buf_index++] = ca_data_index + 3; // LCI length, Figure 10, TIA1057 ( +3 for "WHAT" and country code )
    buf[buf_index++] = 2; // The "WHAT" value. Not quite sure what to set it to

    // Country code, Figure 10, TIA1057
    buf[buf_index++] = conf.location_info.ca_country_code[0];
    buf[buf_index++] = conf.location_info.ca_country_code[1];

    memcpy(&buf[buf_index], (lldp_u8_t *) &ca_data[0], ca_data_index);

    return buf_index + ca_data_index;
}

//
// Append LLDP-MED TLVs to the LLDP frame being build.
//
// In: p_idx - Mulitple functions - Can be a "port index", a "policy index" or "location data format" depending upon the subtype.
//
// In/Out: Buf - Pointer to buffer containing the LLDP frame
//
// Return: The length of the buffer
static lldp_u8_t lldpmed_update_tlv(lldp_u8_t *buf, lldp_port_t p_idx, lldpmed_tlv_subtype_t subtype)
{
    // TIA OUI - Figure 8,TIA1057
    lldp_u16_t i = 0;
    buf[i++] = 0x00;
    buf[i++] = 0x12;
    buf[i++] = 0xBB;

    switch (subtype) {
    case LLDPMED_TLV_SUBTYPE_LOCATION_ID:
        buf[i++] = 0x03;   // Location ID Subtype - Figure 8,TIA1057
        switch (p_idx) {
        case COORDINATE_BASED:
            T_NG(TRACE_GRP_MED_TX, "Adding COORDINATE_BASED tlv");
            buf[i++] = 0x01 ; // Location format, Table 14, TIA1057
            i += lldpmed_coordinate_location_tlv_add(&buf[i]) ;
            break;
        case CIVIC:
            buf[i++] = 0x02;   // Location data format  - Table 14,TIA1057
            i += lldpmed_civic_location_tlv_add(&buf[i]) ;
            break;
        case ECS:
            buf[i++] = 0x03;   // Location data format  - Table 14,TIA1057
            i += lldpmed_ecs_location_tlv_add(&buf[i]) ;
            T_DG(TRACE_GRP_MED_TX, "buf[%d]:0x%X, buf[%d]: 0x%X, buf[%d]:0x%X, buf[%d]:0x%X ", i - 3, buf[i - 3], i - 2, buf[i - 2], i - 1, buf[i - 1], i, buf[i]);

            break;
        default:
            T_W("Invalid location data format");
            i = 0; // Don't add the TLV;
        }
        break;

    case LLDPMED_TLV_SUBTYPE_NETWORK_POLICY:
        i += lldpmed_network_policy_tlv_add(&buf[i], p_idx) ;
        break;

    case LLDPMED_TLV_SUBTYPE_CAPABILITIIES:
        i += lldpmed_capabilities_tlv_add(&buf[i], p_idx) ;
        break;

    default :
        T_W("Unknown subtype: %d", subtype);
        i = 0; // Don't add the TLV;
        break;
    }

    return i;
}

//
// Append MAC/PHY LLDP-MED TLVs to the LLDP frame being build.
// This isn't a LLDP-MED TLV but is included here because
// it so far is the only Organizationally Specific TLV (Annex G in IEEE802.1AB) supported.
// If we are going to support more Organizationally Specific TLVs, it might be a idea to
// move them to a separate file.
// The MAC/PHY TLV is support because it is require by TIA1057 (Section 7.2).
//
// In: port_idx - The port in question
//
// In/Out: Buf - Pointer to buffer containing the LLDP frame
//
// Return: The length of the buffer
static lldp_u8_t lldp_mac_phy_update_tlv(lldp_u8_t *buf, lldp_port_t port_idx)
{
    port_cap_t cap;
    int i = 0;
    port_conf_t        port_conf;
    port_status_t      port_status;
    if (port_mgmt_conf_get(VTSS_ISID_LOCAL, port_idx, &port_conf) != VTSS_OK ||
        port_mgmt_status_get_all(VTSS_ISID_LOCAL, port_idx, &port_status) != VTSS_OK) {
        T_W("Didn't get port configuation");
    } else {
        // TIA OUI - Figure G-1, IEEE802.1AB
        buf[i++] = 0x00;
        buf[i++] = 0x12;
        buf[i++] = 0x0F;

        buf[i++] = 0x01; //Subtype - Figure G-1, IEEE802.1AB

        // Auto-negotiation
        if (port_custom_table[port_idx].cap & PORT_CAP_AUTONEG) {
            buf[i] =  0x1; // Auto-negotiation supported, Table G-2, IEEE802.1AB
        }
        buf[i++] |=  port_conf.autoneg << 1; // Auto-negotiation enabled, Table G-2, IEEE802.1AB

        // PDM auto-negotiation. I could not find the definetion in RFC3636 as specified in IEEE802.1AB
        // but have used the definition found at http://standards.ieee.org/reading/ieee/interp/802.1AB.html

        // MSB
        buf[i] = 0; // Setting all PDM auto-negotiation advertised capabilities to 0 as a start


        if (port_custom_table[port_idx].cap & PORT_CAP_100M_FDX) {
            buf[i] |= 0x04; // 100BASE-TX full duplex, http://standards.ieee.org/reading/ieee/interp/802.1AB.html
        }

        if (port_custom_table[port_idx].cap & PORT_CAP_100M_HDX) {
            buf[i] |= 0x08; // 100BASE-TX halfduplex , http://standards.ieee.org/reading/ieee/interp/802.1AB.html
        }

        if (port_custom_table[port_idx].cap & PORT_CAP_10M_FDX) {
            buf[i] |= 0x20; // 10BASE-TX full duplex, http://standards.ieee.org/reading/ieee/interp/802.1AB.html
        }

        if (port_custom_table[port_idx].cap & PORT_CAP_10M_HDX) {
            buf[i] |= 0x40; // 10BASE-TX halfduplex , http://standards.ieee.org/reading/ieee/interp/802.1AB.html
        }

        i++;

        // LSB
        buf[i] = 0x0; // Settingv all PDM auto-negotiation advertised capabilities to 0 as a start

        cap = port_custom_table[port_idx].cap;

        if (cap & PORT_CAP_1G_FDX) {
            if (cap & PORT_CAP_DUAL_COPPER ||
                cap & PORT_CAP_DUAL_FIBER) {
                buf[i] |= 0x04; // 1000BASE-X, http://standards.ieee.org/reading/ieee/interp/802.1AB.html
                buf[i] |= 0x01; // 1000BASE-T, http://standards.ieee.org/reading/ieee/interp/802.1AB.html
            } else if (port_custom_table[port_idx].cap & PORT_CAP_FIBER) {
                buf[i] |= 0x04; // 1000BASE-X, http://standards.ieee.org/reading/ieee/interp/802.1AB.html
            } else if (port_custom_table[port_idx].cap & PORT_CAP_COPPER   ) {
                buf[i] |= 0x01; // 1000BASE-T, http://standards.ieee.org/reading/ieee/interp/802.1AB.html
            }
        }

        if (port_conf.flow_control) {
            buf[i] |= 0x10; // Asymmetric and Symmetric PAUSE, http://standards.ieee.org/reading/ieee/interp/802.1AB.html
            buf[i] |= 0x80; // PAUSE, http://standards.ieee.org/reading/ieee/interp/802.1AB.html
        }

        i++;

        // Operation Mau type. Defined in RFC3636
        buf[i++] = 0x00; // Max defined dot3MauType so is 40, so we do never use the MSB byte.

        // dot3MauType vaues is defined in section 4 in RFC3636
        buf[i] = 0;   // Default to "no internal MAU"
        if (port_status.status.speed == VTSS_SPEED_10M) {
            if (port_status.status.fdx) {
                buf[i] = 11;   // 10M, fullduplex
            } else {
                buf[i] = 10;   // 10M, halfduplex
            }
        }

        if (port_status.status.speed == VTSS_SPEED_100M) {
            if (port_status.fiber == 0 && port_status.status.fdx) {
                buf[i] = 16;   // 100M, fullduplex
            } else if (port_status.fiber == 0 && port_status.status.fdx == 0) {
                buf[i] = 15;   // 100M, halfduplex
            } else if (port_status.fiber == 1 && port_status.status.fdx) {
                buf[i] = 18;   // 100M Full duplex + Fiber {
            } else {
                buf[i] = 17;   // 100M Half duplex+ Fiber
            }
        }

        if (port_status.status.speed == VTSS_SPEED_1G) {
            if (port_status.fiber) {
                buf[i] = 22;   // 1G Full duplex + Fiber
            } else {
                buf[i] = 30;   // 1G Full duplex + cobber
            }
        }

        i++;
    }
    return i;
}

lldp_u16_t lldpmed_tlv_add(lldp_u8_t *buf, lldp_port_t port_idx)
{
    lldp_u16_t tlv_info_len = 0;
    T_DG_PORT(TRACE_GRP_MED_TX, port_idx, "Entering lldpmed_tlv_add");

    // According to section 11.1, TIA1057 we shall only transmit LLDP-MED tlvs when
    // a LLDP-MED capabilities TLV has been received on the port.
    if (lldpmed_medTransmitEnabled_get(port_idx)) {
        lldp_struc_0_t conf;
        lldp_get_config(&conf, VTSS_ISID_LOCAL); // Get current configuration

        T_DG_PORT(TRACE_GRP_MED_TX, port_idx, "Make LLDP-MED TLVs, lldpmed_optional_tlv = 0x%X", conf.lldpmed_optional_tlv[port_idx]);
        // Capabilities MUST be the first lldp-med TLV in the frame. Section 10.2.2.3, TIA1057
        if (conf.lldpmed_optional_tlv[port_idx] & OPTIONAL_TLV_CAPABILITIES_BIT) {
            tlv_info_len +=  set_tlv_type_and_length_non_zero_len(buf + tlv_info_len, LLDP_TLV_ORG_TLV,
                                                                  lldpmed_update_tlv(buf + 2 + tlv_info_len, port_idx, LLDPMED_TLV_SUBTYPE_CAPABILITIIES));
        }

        // Location TLV
        if (conf.lldpmed_optional_tlv[port_idx] & OPTIONAL_TLV_LOCATION_BIT) {
            tlv_info_len +=  set_tlv_type_and_length_non_zero_len(buf + tlv_info_len, LLDP_TLV_ORG_TLV,
                                                                  lldpmed_update_tlv(buf + 2 + tlv_info_len, (lldp_port_t)COORDINATE_BASED, LLDPMED_TLV_SUBTYPE_LOCATION_ID));

            tlv_info_len +=  set_tlv_type_and_length_non_zero_len(buf + tlv_info_len, LLDP_TLV_ORG_TLV,
                                                                  lldpmed_update_tlv(buf + 2 + tlv_info_len, (lldp_port_t)CIVIC, LLDPMED_TLV_SUBTYPE_LOCATION_ID));

            tlv_info_len +=  set_tlv_type_and_length_non_zero_len(buf + tlv_info_len, LLDP_TLV_ORG_TLV,
                                                                  lldpmed_update_tlv(buf + 2 + tlv_info_len, (lldp_port_t)ECS, LLDPMED_TLV_SUBTYPE_LOCATION_ID));


        }
        // Organizationally TLV
        tlv_info_len +=  set_tlv_type_and_length_non_zero_len(buf + tlv_info_len, LLDP_TLV_ORG_TLV,
                                                              lldp_mac_phy_update_tlv(buf + 2 + tlv_info_len, port_idx));


        lldp_u8_t policy_idx;
        for (policy_idx = LLDPMED_POLICY_MIN; policy_idx <= LLDPMED_POLICY_MAX; policy_idx++) {
            if (conf.ports_policies[port_idx][policy_idx]) {
                if (conf.lldpmed_optional_tlv[port_idx] & OPTIONAL_TLV_POLICY_BIT) { // Bit 1 is the netwotkPolicy TLV, see lldpXMedPortConfigTLVsTxEnable MIB.
                    tlv_info_len +=  set_tlv_type_and_length_non_zero_len(buf + tlv_info_len, LLDP_TLV_ORG_TLV,
                                                                          lldpmed_update_tlv(buf + 2 + tlv_info_len, policy_idx, LLDPMED_TLV_SUBTYPE_NETWORK_POLICY));
                }

            }
        }
    }
    return tlv_info_len;
}
