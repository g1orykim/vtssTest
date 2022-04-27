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
#include "lldp_os.h"
#include "lldp_private.h"
#include "lldpmed_shared.h"
#include "misc_api.h"


lldp_bool_t medTransmitEnabled[LLDP_PORTS];
//
// Gets the "global" variable medTransmitEnabled specified in TIA1057
//
// In: port_idx - Index for the port in question
//     tx_enable - New value for medTransmitEnabled
//     action : action to done, TX_EN_SET or TX_EN_GET
//
// Return: Current vlaue of medTransmitEnabled

lldp_bool_t lldpmed_medTransmitEnabled_get(lldp_port_t port_idx)
{
    return medTransmitEnabled[port_idx];
}

//
// Sets/Gets the "global" variable medTransmitEnabled specified in TIA1057
//
// In: port_idx - Index for the port in question
//     tx_enable - New value for medTransmitEnabled
//     action : action to done, TX_EN_SET or TX_EN_GET
//
// Return: Current vlaue of medTransmitEnabled

void lldpmed_medTransmitEnabled_set(lldp_port_t port_idx, lldp_bool_t tx_enable)
{
    medTransmitEnabled[port_idx] = tx_enable;
}




lldp_bool_t medFastStart[LLDP_PORTS]; // "global" variable medFastStart specified in TIA1057
//
// Gets the "global" variable medFastStart specified in TIA1057
//
// In: port_idx - Index for the port in question
//
// Return: Current value of medTransmitEnabled

lldp_u8_t lldpmed_medFastStart_timer_get(lldp_port_t port_idx)
{
    T_RG_PORT(TRACE_GRP_MED_RX, (vtss_port_no_t)port_idx, "lldpmed_medFastStart_timer_get");
    return medFastStart[port_idx];
}

//
// Performs an "action" to the "global" variable medFastStart specified in TIA1057
//
// In: port_idx - Index for the port in question
//     medFastStart_value - New value for medFastStart
//     action : Set = set medTransmitEnabled to medFastStart_value, GET = get medTransmitEnabled, DECREMENT = sumtract one from medFastStart
//

void lldpmed_medFastStart_timer_action(lldp_port_t port_idx, lldp_u8_t medFastStart_value, lldpmed_fast_start_repeat_count_t action)
{
    if (action == SET) {
        medFastStart[port_idx] = medFastStart_value;
    }

    if (action == DECREMENT) {
        if (medFastStart[port_idx] != 0) {
            medFastStart[port_idx]--;
        }
    }
    T_RG_PORT(TRACE_GRP_MED_RX, (vtss_port_no_t)port_idx, "lldpmed_medFastStart_timer_action - action = %d, Value = %d", action, medFastStart[port_idx]);
}


//
// Validate a LLDP_TLV_ORG_TLV TLV, and updates the LLDP entry table with the
// information
//
// In: tlv - Pointer to the TLV that shall be validated
//         len - The length of the TLV
//
// Out: rx_entry - Pointer to the entry that shall have information updated
//
// Return: TRUE is the TLV was accepted.
lldp_bool_t lldpmed_validate_lldpdu(lldp_8_t   *tlv, lldp_rx_remote_entry_t   *rx_entry, lldp_u16_t len, lldp_bool_t first_lldpmed_tlv)
{
    T_NG_PORT(TRACE_GRP_MED_RX, rx_entry->receive_port, "ORG TLV = 0x%X, 0x%X, 0x%X, 0x%X, 0x%X, 0x%X, 0x%X, 0x%X, 0x%X",
              tlv[0], tlv[1], tlv[2], tlv[3], tlv[4], tlv[5], tlv[6], tlv[7], tlv[8]);

    lldp_bool_t org_tlv_supported = FALSE; // By default we don't support any org tlvs
    lldp_u8_t  org_oui[3];
    lldp_u8_t  p_index = 0;

    // Figure 6, TIA1057
    org_oui[0]    = tlv[2];
    org_oui[1]    = tlv[3];
    org_oui[2]    = tlv[4];

    if (first_lldpmed_tlv) {
        for (p_index = 0; p_index < MAX_LLDPMED_POLICY_APPLICATIONS_CNT; p_index ++) {
            rx_entry->lldpmed_policy_vld[p_index] = 0; //
        }
        for (p_index = 0; p_index < MAX_LLDPMED_LOCATION_CNT; p_index ++) {
            rx_entry->lldpmed_location_vld[p_index] = 0; // Store that this location field isn't used.
        }
    }



    const lldp_u8_t LLDP_TLV_OUI_TIA[]  = {0x00, 0x12, 0xBB}; //  Figure 12 in TIA1057

    // Check if the OUI is of TIA-1057 type
    if (memcmp(org_oui, &LLDP_TLV_OUI_TIA[0], 3) == 0) {
        switch (tlv[5]) {
        case 0x01: // Subtype Capabilities - See Figure 6 in TIA1057.
            if (len == 7) { // Length Must always be 7 for this TLV.
                rx_entry->lldpmed_capabilities_vld = TRUE;
                rx_entry->lldpmed_capabilities = (tlv[6] << 8) + tlv[7];
                rx_entry->lldpmed_device_type  = tlv[8];
                org_tlv_supported = TRUE;
                lldpmed_medTransmitEnabled_set(rx_entry->receive_port, LLDP_TRUE); // Set medTransmitEnabled according to section 11.2.1 bullet a), TIA1057
                rx_entry->lldpmed_capabilities_current |= 0x1; // See TIA1057 MIBS LLDPXMEDREMCAPCURRENT

                T_DG_PORT(TRACE_GRP_MED_RX, rx_entry->receive_port, "Capa = 0x%X", rx_entry->lldpmed_capabilities );
            }
            break;

        case 0x02: // Subtype Policy - See Figure 7 in TIA1057.
            if (len == 8) { // Length Must always be 8 for this TLV.
                rx_entry->lldpmed_capabilities_current |= 0x2; // See TIA1057 MIBS LLDPXMEDREMCAPCURRENT

                // Multiple Policy TLVs allowed within one LLDPDU - Section 10.2.3.1 in TIA1057
                for (p_index = 0; p_index < MAX_LLDPMED_POLICY_APPLICATIONS_CNT; p_index ++) {
                    T_DG_PORT(TRACE_GRP_MED_RX, rx_entry->receive_port, "rx_entry->lldpmed_policy_vld[p_index] = %d, p_index = %d", rx_entry->lldpmed_policy_vld[p_index], p_index);
                    if (rx_entry->lldpmed_policy_vld[p_index] == 1) {
                        // policy field is already used, continue to the next field.
                    } else {
                        T_DG_PORT(TRACE_GRP_MED_RX, rx_entry->receive_port, "LLDP-MED Policy subtype ");
                        rx_entry->lldpmed_policy[p_index] = (tlv[6] << 24) + (tlv[7] << 16) + (tlv[8] << 8)  + tlv[9] ;
                        rx_entry->lldpmed_policy_vld[p_index] = 1; // Store that this policy field is used.

                        org_tlv_supported = TRUE;
                        T_DG_PORT(TRACE_GRP_MED_RX, rx_entry->receive_port, "rx_entry->lldpmed_policy = %u", rx_entry->lldpmed_policy[p_index]);
                        break; // Policy field updated, continue to next TLV.
                    }
                }
            }
            break;

        case 0x03: // Subtype Location - See Figure 8 in TIA1057.
            rx_entry->lldpmed_capabilities_current |= 0x4; // See TIA1057 MIBS LLDPXMEDREMCAPCURRENT
            // As I understand it then according  to section 10.2.4 in TIA1057 we shall support multiple location TLVs ( For now 3, See table 14, TIA1057)
            for (p_index = 0; p_index < MAX_LLDPMED_LOCATION_CNT; p_index ++) {
                T_NG_PORT(TRACE_GRP_MED_RX, rx_entry->receive_port, "rx_entry->lldpmed_location_vld[p_index] = %d, p_index = %d", rx_entry->lldpmed_location_vld[p_index], p_index);
                if (rx_entry->lldpmed_location_vld[p_index] == 1) {
                    // location field is already used, continue to the next field.
                } else {
                    rx_entry->lldpmed_location_length[p_index] = ((tlv[0] & 0x1) << 8) + tlv[1] - 5; // Figure 8 + Section 10.2.4.1, TIA1057
                    rx_entry->lldpmed_location_format[p_index] = tlv[6];  // Figure 8, TIA1057
                    rx_entry->lldpmed_location[p_index] = &tlv[7] ; // Figure 8, TIA1057

                    T_DG_PORT(TRACE_GRP_MED_RX, rx_entry->receive_port, "LLDP-MED Location subtype format = %d, length = %d",
                              rx_entry->lldpmed_location_format[p_index],
                              rx_entry->lldpmed_location_length[p_index]);

                    if (rx_entry->lldpmed_location_format[p_index] == 1 &&
                        rx_entry->lldpmed_location_length[p_index] != 16) { // Fixed length for coordinate-based LCI - Table 14, TIA1057
                        T_DG_PORT(TRACE_GRP_MED_RX, rx_entry->receive_port, "Invalid LLDP location length / format, length = %u ", rx_entry->lldpmed_location_length[p_index]);
                        rx_entry->lldpmed_location_vld[p_index] = 0; // Store that this location field isn't used.
                        org_tlv_supported = FALSE;

                    } else {
                        org_tlv_supported = TRUE;
                        rx_entry->lldpmed_location_vld[p_index] = 1; // Store that this location field is used.
                    }

                    break; // Location field updated, continue to next TLV.
                }
            }
            break;



        case 0x04: // Subtype PoE - See Figure 12 in TIA1057
            // Handled specially and the line below should never be reached - see vtss_lldp.c file
            T_DG_PORT(TRACE_GRP_MED_RX, rx_entry->receive_port, "Unexpected PoE TLV");
            org_tlv_supported = FALSE;
            break;


        // Hardware revision TLV - TIA1057, Figure 13
        case 0x5:
            rx_entry->lldpmed_capabilities_current |= 0x10; // See TIA1057 MIBS LLDPXMEDREMCAPCURRENT

            // See section 10.2.6.1.1 in TIA1057
            if (len >= 4) {
                org_tlv_supported = TRUE;
                rx_entry->lldpmed_hw_rev_length = MIN(len - 4, MAX_LLDPMED_INVENTORY_LENGTH);
            } else {
                // Shall never happen
                rx_entry->lldpmed_hw_rev_length = 0;
                T_DG_PORT(TRACE_GRP_MED_RX, rx_entry->receive_port, "Hardware revision TLV length too short");
            }

            rx_entry->lldpmed_hw_rev =  &tlv[6];
            break;

        // Firmware revision TLV - TIA1057, Figure 14
        case 0x6:
            rx_entry->lldpmed_capabilities_current |= 0x10; // See TIA1057 MIBS LLDPXMEDREMCAPCURRENT

            // See section 10.2.6.2.1 in TIA1057
            if (len >= 4) {
                rx_entry->lldpmed_firm_rev_length = MIN(len - 4, MAX_LLDPMED_INVENTORY_LENGTH);
                org_tlv_supported = TRUE;
            } else {
                // Shall never happen
                rx_entry->lldpmed_firm_rev_length = 0;
                T_DG_PORT(TRACE_GRP_MED_RX, rx_entry->receive_port, "Firmware revision TLV length too short");
            }

            rx_entry->lldpmed_firm_rev = &tlv[6];
            break;

        // Software revision TLV - TIA1057, Figure 15
        case 0x7:
            rx_entry->lldpmed_capabilities_current |= 0x10; // See TIA1057 MIBS LLDPXMEDREMCAPCURRENT

            // See section 10.2.6.3.1 in TIA1057
            if (len >= 4) {
                rx_entry->lldpmed_sw_rev_length = MIN(len - 4, MAX_LLDPMED_INVENTORY_LENGTH);
                org_tlv_supported = TRUE;
            } else {
                // Shall never happen
                rx_entry->lldpmed_sw_rev_length = 0;
                T_DG_PORT(TRACE_GRP_MED_RX, rx_entry->receive_port, "Software revision TLV length too short");
            }

            rx_entry->lldpmed_sw_rev = &tlv[6];
            break;

        // Serial number TLV - TIA1057, Figure 16
        case 0x8:
            rx_entry->lldpmed_capabilities_current |= 0x10; // See TIA1057 MIBS LLDPXMEDREMCAPCURRENT

            // See section 10.2.6.4.1 in TIA1057
            if (len >= 4) {
                rx_entry->lldpmed_serial_no_length = MIN(len - 4, MAX_LLDPMED_INVENTORY_LENGTH);
                org_tlv_supported = TRUE;
            } else {
                // Shall never happen
                rx_entry->lldpmed_serial_no_length = 0;
                T_DG_PORT(TRACE_GRP_MED_RX, rx_entry->receive_port, "Serial number TLV length too short");
            }

            rx_entry->lldpmed_serial_no = &tlv[6];
            break;

        // Manufacturer name TLV - TIA1057, Figure 17
        case 0x9:
            rx_entry->lldpmed_capabilities_current |= 0x10; // See TIA1057 MIBS LLDPXMEDREMCAPCURRENT

            // See section 10.2.6.5.1 in TIA1057
            if (len >= 4) {
                rx_entry->lldpmed_manufacturer_name_length = MIN(len - 4, MAX_LLDPMED_INVENTORY_LENGTH);
                org_tlv_supported = TRUE;
            } else {
                // Shall never happen
                rx_entry->lldpmed_manufacturer_name_length = 0;
                T_DG_PORT(TRACE_GRP_MED_RX, rx_entry->receive_port, "Manufacturer name TLV length too short");
            }

            rx_entry->lldpmed_manufacturer_name = &tlv[6];
            break;

        // Model name TLV - TIA1057, Figure 18
        case 0xA:
            rx_entry->lldpmed_capabilities_current |= 0x10; // See TIA1057 MIBS LLDPXMEDREMCAPCURRENT


            // See section 10.2.6.6.1 in TIA1057
            if (len >= 4) {
                rx_entry->lldpmed_model_name_length = MIN(len - 4, MAX_LLDPMED_INVENTORY_LENGTH);
                org_tlv_supported = TRUE;
            } else {
                // Shall never happen
                rx_entry->lldpmed_model_name_length = 0;
                T_DG_PORT(TRACE_GRP_MED_RX, rx_entry->receive_port, "Model name TLV length too short");
            }

            rx_entry->lldpmed_model_name = &tlv[6];
            break;

        // Assert ID - TIA1057, Figure 19
        case 0xB:
            rx_entry->lldpmed_capabilities_current |= 0x10; // See TIA1057 MIBS LLDPXMEDREMCAPCURRENT

            // See section 10.2.6.7.1 in TIA1057
            if (len >= 4) {
                rx_entry->lldpmed_assert_id_length = MIN(len - 4, MAX_LLDPMED_INVENTORY_LENGTH);
                org_tlv_supported = TRUE;
            } else {
                // Shall never happen
                rx_entry->lldpmed_assert_id_length = 0;
                T_DG_PORT(TRACE_GRP_MED_RX, rx_entry->receive_port, "Assert ID TLV length too short");
            }

            rx_entry->lldpmed_assert_id = &tlv[6];
            break;

        default:
            org_tlv_supported = FALSE;
        }
    }

    rx_entry->lldpmed_info_vld = org_tlv_supported; // Signaling that a least LLDP-MED entry field is valid
    T_DG_PORT(TRACE_GRP_MED_RX, rx_entry->receive_port, "org_tlv_supported = 0x%X", org_tlv_supported);
    return org_tlv_supported;
}


//
// Getting the number of policies within an entry
//
// In : Pointer to the Entry containing the information
//
// Return : Number of policies
//


lldp_u8_t lldpmed_get_policies_cnt (lldp_remote_entry_t   *entry)
{

    lldp_u8_t policy_cnt = 0;
    lldp_u8_t p_index;

    for (p_index = 0; p_index < MAX_LLDPMED_POLICY_APPLICATIONS_CNT; p_index ++) {
        if (entry->lldpmed_policy_vld[p_index] == 1) {
            policy_cnt++;
        }
    }

    return policy_cnt;
}

//
// Getting the number of locations within an entry
//
// In : Pointer to the Entry containing the information
//
// Return : Number of locations
//


lldp_u8_t lldpmed_get_locations_cnt (lldp_remote_entry_t   *entry)
{

    lldp_u8_t location_cnt = 0;
    lldp_u8_t p_index;

    for (p_index = 0; p_index < MAX_LLDPMED_LOCATION_CNT; p_index ++) {
        if (entry->lldpmed_location_vld[p_index] == 1) {
            location_cnt++;
        }
    }

    return location_cnt;
}

//
// Converting a device type to a printable string
//
// In : Pointer to the Entry containing the information
//
// In/Out: Pointer to the string.
//
void lldpmed_device_type2str (lldp_remote_entry_t   *entry, lldp_8_t *string_ptr )
{
    T_NG(TRACE_GRP_MED_RX, "entry->lldpmed_device_type = %d", entry->lldpmed_device_type);
    switch (entry->lldpmed_device_type) {
    case LLDPMED_DEVICE_TYPE_NOT_DEFINED:
        sprintf(string_ptr, "%s", "Type Not Defined");
        break;

    case LLDPMED_DEVICE_TYPE_ENDPOINT_CLASS_I:
        sprintf(string_ptr, "%s", "Endpoint Class I");
        break;

    case LLDPMED_DEVICE_TYPE_ENDPOINT_CLASS_II:
        sprintf(string_ptr, "%s", "Endpoint Class II");
        break;

    case LLDPMED_DEVICE_TYPE_ENDPOINT_CLASS_III:
        sprintf(string_ptr, "%s", "Endpoint Class III");
        break;

    case LLDPMED_DEVICE_TYPE_NETWORK_CONNECTIVITY:
        sprintf(string_ptr, "%s", "Network Connectivity");
        break;

    default: /* reserved */
        sprintf(string_ptr, "%s", "Reserved");
        break;

    }
}


//
// Converting a LLDP-MED capabilities to a printable string
//
// In : Pointer to the Entry containing the information
//
// In/Out: Pointer to the string.
//
void lldpmed_capabilities2str (lldp_remote_entry_t   *entry, lldp_8_t *string_ptr )
{

    T_IG_PORT(TRACE_GRP_MED_RX, entry->receive_port, "Entering lldpmed_capabilities2str, capa = %d", entry->lldpmed_capabilities);

    // See Table 10, TIA1057 for capabilities.
    lldp_u8_t bit_no;
    lldp_bool_t print_comma = LLDP_FALSE;

    // Define the capabilities in Table 10 in TIA1057
    const lldp_8_t *sys_capa[] = {"LLDP-MED Capabilities",
                                  "Network Policy",
                                  "Location Identification",
                                  "Extended Power via MDI - PSE",
                                  "Extended Power via MDI - PD",
                                  "Inventory",
                                  "Reserved"
                                 };

    strcpy(string_ptr, "");
    // Make the string with comma between each capability.
    for (bit_no = 0; bit_no < 7; bit_no++) {
        if (entry->lldpmed_capabilities & (1 << bit_no)) {
            if (print_comma) {
                strncat(string_ptr, ", ", 254);
            }
            strncat(string_ptr, sys_capa[bit_no], 254);
            print_comma = LLDP_TRUE;
        }
    }
}


//
// Converting a LLDP-MED application type to a printable string
//
// In : appl_type_value - The application type
//
// In/Out: string_ptr - Pointer to the string.
//
void lldpmed_appl_type2str (lldpmed_application_type_t appl_type_value, lldp_8_t *string_ptr)
{

    // Define the apllication types in Table 12 in TIA1057
    const lldp_8_t *appl_type_str[] = {"Reserved",
                                       "Voice",
                                       "Voice Signaling",
                                       "Guest Voice",
                                       "Guest Voice signaling",
                                       "Softphone Voice",
                                       "Video Conferencing",
                                       "Steaming Video",
                                       "Video Signaling",
                                       "Reserved"
                                      };

    T_NG(TRACE_GRP_MED_RX, "Appl_type_value = %d, appl_type = %s", appl_type_value, appl_type_str[appl_type_value]);

    // Make the string
    if ((lldp_u16_t)appl_type_value < MAX_LLDPMED_POLICY_APPLICATIONS_CNT) { // Make sure that we do point outside the array.
        sprintf(string_ptr, "%s", appl_type_str[appl_type_value]);
    } else {
        sprintf(string_ptr, "%s", "Reserved");
    }
}




//
// Converting a LLDP-MED policy application type (from the entry table) to a printable string
//
// In : lldpmed_policy - the policy found in the entry table
//
// In/Out: string_ptr - Pointer to the string.
//
void lldpmed_policy_appl_type2str (lldp_u32_t lldpmed_policy, lldp_8_t *string_ptr)
{

    // The lldmed_policy contains the whole policy information. Application is bits 31 downto 24, See figure 7, TIA1057
    lldp_u8_t appl_type_value = lldpmed_policy >> 24 & 0xFF;

    lldpmed_appl_type2str((lldpmed_application_type_t) appl_type_value, string_ptr);
}




//
// Converting a 64 bits number of a integer and fraction part(where it is possible to specify
// the number of bits used for both the integer and fraction part) to at printable string.
//
// In : res - Resolution. How many of the tude bits that are valid.
//      tude - The 64 bits number that should be converted. ( Comes from latitude, logitude and altitude)
//      int_bits - Number of bits used for the integer part.
//      frac_bits - Number of bits used for the fraction part.
// In/Out: string_ptr - Pointer to the string.
//
lldp_bool_t lldpmed_tude2decimal_str (lldp_u8_t res,  lldp_u64_t tude, lldp_u8_t int_bits, lldp_u8_t frac_bits, lldp_8_t *string_ptr)
{
    lldp_u8_t p_index;
    lldp_u8_t eraseBits = int_bits + frac_bits - res;
    lldp_u64_t mask = 0;
    lldp_bool_t neg_result = LLDP_FALSE;


    if (eraseBits > 0) while (eraseBits--) {
            mask = (mask << 1) | 1;
        }
    tude &= ~mask;

    // Creat a mask matching the number of bits used for the integer and fraction parts.
    int int_bits_mask = ((1 << int_bits) - 1);
    int frac_bits_mask = ((1 << frac_bits) - 1);

    lldp_u32_t int_part = (tude >> (frac_bits)) & int_bits_mask;  // Example - Integer part is the upper 9 bits, Section 2.1 RFC3825,July2004
    lldp_u32_t fraction = tude & frac_bits_mask; // Example - Fraction is the lower 25 bits, Section 2.1 RFC3825,July2004

    // Check if MSB if the interger part is set ( negative number )
    if (int_part & (1 << (int_bits - 1))) {
        int_part = ~int_part & int_bits_mask;
        fraction = ~fraction & frac_bits_mask;
        neg_result = LLDP_TRUE;
    }

    T_IG(TRACE_GRP_MED_RX, "tude:0x%llX, int_bits_mask:0x%X, frac_bits_mask:0x%X, int_part:%u, fraction:%u, neg_result:%d, res:0x%X, mask:%llX",
         tude, int_bits_mask, frac_bits_mask, int_part, fraction, neg_result, res, mask);


    // Calulate the fraction value;
    lldp_u32_t fraction_val = 0;
    lldp_u32_t digi_mulitiplier = 1000;
    lldp_u32_t value = (lldp_u32_t)(0.5 * 10 * digi_mulitiplier);

    for (p_index = 0; p_index < res - int_bits; p_index ++ ) {
        if (fraction & (1 << (frac_bits - 1 - p_index))) {
            fraction_val += value;
        }
        value /= 2;
    }

    // pad the zeroes needed to the fraction
    lldp_8_t frac_string_ptr[10] ;

    strcpy(frac_string_ptr, "");
    if (fraction_val != 0) {
        for (p_index =  0; p_index < 9; p_index++) {
            if (fraction_val / digi_mulitiplier > 0) {
                break;
            }
            strcat(frac_string_ptr, "0");
            digi_mulitiplier /= 10;
        }
    }
    // Genereate printable string containing "integer.fraction"
    sprintf(&string_ptr[0], "%u.%s%.3u", int_part, frac_string_ptr, fraction_val);
    return neg_result;
}



//
// Converting altitude type to a printable string
//
// In : at - altitude type
//
// In/Out: string_ptr - Pointer to the string.
//
void lldpmed_at2str(lldpmed_at_type_t at, lldp_8_t *string_ptr)
{
    // meters or floor, RFC3825,July2004 section 2.1
    switch (at) {
    case METERS:
        strcpy(string_ptr, " meter(s)");
        break;
    case FLOOR:
        strcpy(string_ptr, " floor");
        break;
    default:
        strcpy(string_ptr, "");
        // undefined at the moment.
        break;
    }
}


//
// Converting map datum type to a printable string
//
// In : datume - The datum
//
// In/Out: string_ptr - Pointer to the string.
//
void lldpmed_datum2str(lldpmed_datum_t datum, lldp_8_t *string_ptr)
{
    // meters or floor, RFC3825,July2004 section 2.1
    switch (datum) {
    case WGS84:
        strcpy(string_ptr, "WGS84");
        break;
    case NAD83_NAVD88:
        strcpy(string_ptr, "NAD83/NAVD88");
        break;
    case NAD83_MLLW         :
        strcpy(string_ptr, "NAD83/MLLW");
        break;
    default:
        // undefined at the moment.
        strcpy(string_ptr, "");
        break;
    }
}

//
// Converting a coordinate TLV to a printable string
//
// In : location_info - Containing the TLV information
//
// In/Out: string_ptr - Pointer to the string.
//

static void lldpmed_coordinate2str (lldp_u8_t *location_info, lldp_8_t *string_ptr)
{

    T_NG(TRACE_GRP_MED_RX, "Entering lldpmed_coordinate2str");

    lldp_u8_t resolution;
    lldp_u64_t tude;
    lldp_8_t tude_str[255];

    // Calculate Latitude, Figure 9, TIA1057
    resolution = location_info[0] >> 2;

    /*lint --e{571} */
    tude = ((lldp_u64_t)location_info[0] << 32) +
           (lldp_u32_t)(location_info[1] << 24) +
           (location_info[2] << 16) +
           (location_info[3] << 8) +
           location_info[4];

    strcpy(string_ptr, "Latitude:");
    //Integer part is the upper 9 bits, fraction 25 bits , Section 2.1 RFC3825,July2004
    if (lldpmed_tude2decimal_str(resolution, tude, 9, 25, tude_str)) {
        strcat(string_ptr, tude_str);
        strcat(string_ptr, " South, ");
    } else {
        strcat(string_ptr, tude_str);
        strcat(string_ptr, " North, ");
    }



    // Calculate Longitude, Figure 9, TIA1057
    strcat(string_ptr, "Longitude:");
    resolution = location_info[5] >> 2;

    tude = (((lldp_u64_t)location_info[5] & 0x3) << 32 ) +
           (lldp_u32_t)(location_info[6] << 24) +
           (location_info[7] << 16) +
           (location_info[8] << 8) +
           location_info[9];

    //Integer part is the upper 9 bits, fraction 25 bits , Section 2.1 RFC3825,July2004
    if (lldpmed_tude2decimal_str(resolution, tude, 9, 25, tude_str)) {
        strcat(string_ptr, tude_str);
        strcat(string_ptr, " West, ");
    } else {
        strcat(string_ptr, tude_str);
        strcat(string_ptr, " East, ");
    }


    // Find Altitude, Figure 9, TIA1057
    strcat(string_ptr, "Altitude:");
    lldpmed_at_type_t at = (lldpmed_at_type_t) (location_info[10] >> 4); // Altitude type is the 4 upper bits, Figure 9, TIA1057
    resolution = ((location_info[10] & 0x0F) << 2) + (location_info[11] >> 6); // resolution bits, Figure 9, TIA1057

    T_DG(TRACE_GRP_MED_RX, "resolution:%d, 0x%X, 0x%X", resolution, location_info[10], location_info[11]);

    // Altitude, Figure 9,TIA1057
    tude = ((location_info[11] & 0x3F) << 24) +
           (location_info[12]  << 16) +
           (location_info[13]  << 8) +
           location_info[14];


    // RFC3825,July2004 section 2.1 - 22 bits integer and 8 bits fraction.
    if (lldpmed_tude2decimal_str(resolution, tude, 22, 8, tude_str)) {
        strcat(string_ptr, "-");
    }
    strcat(string_ptr, tude_str);

    // Add altitude
    lldp_8_t altitude_type_str[25];
    lldpmed_at2str(at, altitude_type_str); // Get altitude type as string
    strcat(string_ptr, altitude_type_str); // Add altitude type to the total string.


    // Add Datum
    lldpmed_datum_t datum = (lldpmed_datum_t)location_info[15]; // Datum, Figure 9, TIA1057
    lldp_8_t datum_str[25];
    lldpmed_datum2str(datum, datum_str); // Get datum as string
    strcat(string_ptr, ", Map datum:");
    strcat(string_ptr, datum_str); // Add datum to the total string.

}

// Function for converting from the index used when storing the catype in flash configuration and
// the lldpmed_catype_t type.
//
// In : Index - Index for how the CA TYPE is stored in flash configuration.
//
// Return: lldpmed_catype_t corresponding to the index.
//

lldpmed_catype_t lldpmed_index2catype(lldp_u8_t ca_index)
{
    // See LLDP-MED catype for civic location, Section 3.4, Annex B, TIA1057
    if (ca_index < 6) {
        return (lldpmed_catype_t) (ca_index + 1);
    } else {
        return (lldpmed_catype_t) (ca_index + 10);
    }
}

// Function for converting from the ca_type to index used when storing the catype in flash configuration and
// the lldpmed_catype_t type.
//
// In : ca_type - The CA TYPE.
//
// Return: Index for how the CA TYPE is stored in flash configuration.
//
lldp_u8_t lldpmed_catype2index(lldpmed_catype_t ca_type)
{
    // See LLDP-MED catype for civic location, Section 3.4, Annex B, TIA1057
    if (ca_type < 7) {
        return (lldpmed_catype_t) (ca_type - 1);
    } else {
        return (lldpmed_catype_t) (ca_type - 10);
    }
}


// Getting Ca type as printable string
//
// In : ca_type - CA TYPE as integer
//
// In/Out: string_ptr - Pointer to the string.
//
void lldpmed_catype2str(lldpmed_catype_t ca_type, lldp_8_t *string_ptr)
{
    // Table in ANNEX B, TIA1057
    strcpy(string_ptr, "");
    switch (ca_type) {
    case LLDPMED_CATYPE_A1:
        strcat(string_ptr, "National subdivision");
        break;
    case LLDPMED_CATYPE_A2:
        strcat(string_ptr, "County");
        break;

    case LLDPMED_CATYPE_A3:
        strcat(string_ptr, "City");
        break;
    case LLDPMED_CATYPE_A4:
        strcat(string_ptr, "City district");
        break;
    case LLDPMED_CATYPE_A5:
        strcat(string_ptr, "Block (Neighborhood)");
        break;
    case LLDPMED_CATYPE_A6:
        strcat(string_ptr, "Street");
        break;
    case LLDPMED_CATYPE_PRD:
        strcat(string_ptr, "Street Dir");
        break;
    case LLDPMED_CATYPE_POD:
        strcat(string_ptr, "Trailling Street");
        break;
    case LLDPMED_CATYPE_STS:
        strcat(string_ptr, "Street Suffix");
        break;
    case LLDPMED_CATYPE_HNO:
        strcat(string_ptr, "House No.");
        break;
    case LLDPMED_CATYPE_HNS:
        strcat(string_ptr, "House No. Suffix");
        break;
    case LLDPMED_CATYPE_LMK:
        strcat(string_ptr, "Landmark");
        break;
    case LLDPMED_CATYPE_LOC:
        strcat(string_ptr, "Additional Location Info");
        break;
    case LLDPMED_CATYPE_NAM:
        strcat(string_ptr, "Name");
        break;

    case LLDPMED_CATYPE_ZIP:
        strcat(string_ptr, "Zip");
        break;
    case LLDPMED_CATYPE_BUILD:
        strcat(string_ptr, "Building");
        break;
    case LLDPMED_CATYPE_UNIT:
        strcat(string_ptr, "Unit");
        break;
    case LLDPMED_CATYPE_FLR:
        strcat(string_ptr, "Floor");
        break;
    case LLDPMED_CATYPE_ROOM:
        strcat(string_ptr, "Room No.");
        break;
    case LLDPMED_CATYPE_PLACE:
        strcat(string_ptr, "Placetype");
        break;
    case LLDPMED_CATYPE_PCN:
        strcat(string_ptr, "Postal Community Name");
        break;
    case LLDPMED_CATYPE_POBOX:
        strcat(string_ptr, "P.O. Box");
        break;
    case LLDPMED_CATYPE_ADD_CODE:
        strcat(string_ptr, "Addination Code");
        break;

    default:
        break;
    }
}



//
//
//

void lldpmed_update_civic_info(lldpmed_location_civic_info_t *civic_info, lldpmed_catype_t new_ca_type, lldp_8_t *new_ca_value)
{
    lldp_16_t civic_index;
    lldpmed_location_civic_info_t current_civic_info;
    lldp_u8_t ca_cnt = 0;
    lldp_16_t str_ptr = 0;
    char tmp_str[CIVIC_CA_VALUE_LEN_MAX];
    lldp_u8_t len;

    memcpy(&current_civic_info, civic_info, sizeof (current_civic_info));

    for (civic_index = 0; civic_index < LLDPMED_CATYPE_CNT; civic_index++ ) {
        T_NG(TRACE_GRP_MED_RX, "new_ca_type = %d, current ca_type =%d, civic_index = %d", new_ca_type, current_civic_info.civic_ca_type_array[civic_index], civic_index);
        if (new_ca_type == current_civic_info.civic_ca_type_array[civic_index]) {
            T_NG(TRACE_GRP_MED_RX, "Inserting new_ca_value : %s", new_ca_value);
            misc_strncpyz(tmp_str, new_ca_value, CIVIC_CA_VALUE_LEN_MAX);
        } else {
            if (strlen(&current_civic_info.ca_value[current_civic_info.civic_str_ptr_array[civic_index]]) > CIVIC_CA_VALUE_LEN_MAX) {
                T_W("Internal string length error. String Len = %d, String = %s",
                    strlen(&current_civic_info.ca_value[current_civic_info.civic_str_ptr_array[civic_index]]),
                    &current_civic_info.ca_value[current_civic_info.civic_str_ptr_array[civic_index]]);
                strcpy(tmp_str, "");
            } else {
                strcpy(tmp_str, &current_civic_info.ca_value[current_civic_info.civic_str_ptr_array[civic_index]]);
            }
        }

        len = strlen(tmp_str);

        if ((len + str_ptr + ca_cnt * 2) > CIVIC_CA_VALUE_LEN_MAX) { // Each CA takes 2 bytes (one for CA type and one for CA length), Figure 10, TIA1057
            T_WG(TRACE_GRP_MED_RX, "Total infomation for Civic Address Location exceeds maximum allowed charaters");
            return;
        }

        civic_info->civic_str_ptr_array[civic_index] = str_ptr;

        T_NG(TRACE_GRP_MED_RX, "new_ca_type =%d, tmp_str =%s, len = %d, str_ptr = %d", new_ca_type, tmp_str, len, str_ptr);

        if (len > 0) {
            strcpy(&civic_info->ca_value[str_ptr], tmp_str);
            str_ptr += strlen(tmp_str) + 1;
            ca_cnt++;
        } else {
            civic_info->ca_value[str_ptr] = '\0';
            str_ptr ++;
        }
        civic_info->civic_ca_type_array[civic_index] = lldpmed_index2catype(civic_index); // Store the CAType
    }
}
//
// Converting a civic TLV to a printable string
//
// In : civic_info - Containing the TLV civic information
//
// In/Out: string_ptr - Pointer to the string.
//

static void lldpmed_civic2str (lldp_u8_t *location_info, lldp_8_t *string_ptr)
{


    lldp_u8_t lci_length = location_info[0] + 1; // Figure 10, TIA1057 + Section 10.2.4.3.2 (adding 1 to the LCI length)
//  lldp_u8_t what = location_info[1]; // Figure 10, TIA1057
    lldp_u8_t country_code[3] = {location_info[2], location_info[3], '\0'}; // Figure 10, TIA1057
    lldpmed_catype_t ca_type; // See Figure 10, TIA1057
    lldp_u8_t ca_length; // See Figure 10, TIA1057


    lldp_u8_t tlv_index = 4; // First CAtype is byte 4 in the TLV. Figure 10, TIA1057
    lldp_8_t str_buf[255]; // Temp. string buffer



    sprintf(string_ptr, "Country code:%s ", country_code);

    while (tlv_index < lci_length) {
        ca_type = (lldpmed_catype_t) location_info[tlv_index];
        ca_length = location_info[tlv_index + 1];


        if (tlv_index +  2 + ca_length  > lci_length) {
            T_WG(TRACE_GRP_MED_RX, "Invalid CA length, tlv_index = %d, ca_length = %d, lci_length = %d",
                 tlv_index, ca_length, lci_length);
            break;
        }
        // Insert -- between each CA value
        strcat(string_ptr, " -- ");

        // Add the ca type (as string) to the string
        lldpmed_catype2str(ca_type, str_buf);
        strcat(string_ptr, str_buf);
        strcat(string_ptr, ":");

        //Add CA value,  Figure 10, TIA1057
        memcpy(&str_buf[0], &location_info[tlv_index + 2], ca_length);
        str_buf[ca_length] = '\0';
        strcat(string_ptr, str_buf);


        tlv_index += 2 + ca_length; // Select next CA
        T_RG(TRACE_GRP_MED_RX, "string_ptr = %s, str_buf = %s", string_ptr, str_buf);
    }
}

//
// Converting a ECS ELIN TLV to a printable string
//
// In : civic_info - Containing the TLV civic information
//
// In/Out: string_ptr - Pointer to the string.
//

static void lldpmed_elin2str (lldp_u8_t *location_info, lldp_u8_t length,  lldp_8_t *string_ptr)
{
    // Figure 11, TIA1057
    strcpy(string_ptr, "Emergency Call Service:");
    strncat(string_ptr, (lldp_8_t *) location_info, length);
}


//
// Converting a LLDP-MED location to a printable string
//
// In : Entry containing the location
//
// In/Out: string_ptr - Pointer to the string.
//
void lldpmed_location2str (lldp_remote_entry_t *entry, lldp_8_t *string_ptr, lldp_u8_t p_index)
{
    if (entry->lldpmed_location_vld[p_index] == LLDP_FALSE) {
        sprintf(string_ptr, "%s", "No location information available");
    } else {

        switch (entry->lldpmed_location_format[p_index])  {

        case LLDPMED_LOCATION_INVALID:
            sprintf(string_ptr, "%s", "Invalid location ID");
            break;
        case LLDPMED_LOCATION_COORDINATE:
            lldpmed_coordinate2str(entry->lldpmed_location[p_index], string_ptr);
            break;
        case LLDPMED_LOCATION_CIVIC:
            lldpmed_civic2str(entry->lldpmed_location[p_index], string_ptr);
            break;
        case LLDPMED_LOCATION_ECS:
            lldpmed_elin2str(entry->lldpmed_location[p_index], entry->lldpmed_location_length[p_index], string_ptr);
            break;
        default:
            sprintf(string_ptr, "%s", "Reserved for future expansion");
        }

    }
}



//
// Getting the tagged flag from the lldpmed_policy
//
// In : Pointer to the Entry containing the information
//
// Return: True if tagged flag is set
//
BOOL lldpmed_get_tagged_flag(lldp_u32_t lldpmed_policy)
{

    // The lldmed_policy contains the whole policy information. TAG flag is bit 22, See figure 7, TIA1057
    lldp_u8_t tagged_flag = lldpmed_policy >> 22 & 0x1;

    return tagged_flag;
}

//
// Getting the unknown policy flag from the lldpmed_policy
//
// In : lldpmed_policy - The policy
//
// Return: True if unknown policy flag is set
//
BOOL lldpmed_get_unknown_policy_flag (lldp_u32_t lldpmed_policy)
{

    // The lldmed_policy contains the whole policy information. Policy flag is bit 23, See figure 7, TIA1057
    lldp_u8_t policy_flag = lldpmed_policy >> 23 & 0x1;
    return  policy_flag;
}


//
// Converting a LLDP-MED policy Policy Flag to a printable string
//
// In : The policy
//
// In/Out: Pointer to the string.
//
void lldpmed_policy_flag_type2str (lldp_u32_t lldpmed_policy, lldp_8_t *string_ptr )
{
    // Make the string
    if (lldpmed_get_unknown_policy_flag(lldpmed_policy)) {
        sprintf(string_ptr, "%s", "Unknown");
    } else {
        sprintf(string_ptr, "%s", "Defined");
    }
}

//
// Converting a LLDP-MED policy TAG to a printable string
//
// In : The policy
//
// In/Out: Pointer to the string.
//
void lldpmed_policy_tag2str (lldp_u32_t policy, lldp_8_t *string_ptr )
{
    // Make the string
    if (lldpmed_get_tagged_flag(policy)) {
        sprintf(string_ptr, "%s", "Tagged");
    } else {
        sprintf(string_ptr, "%s", "Untagged");
    }
}

//
// Converting a LLDP-MED policy VLAN ID to a printable string
//
// In : The policy
//
// In/Out: Pointer to the string.
//
void lldpmed_policy_vlan_id2str (lldp_u32_t policy, lldp_8_t *string_ptr )
{
    // The lldmed_policy contains the whole policy information. VLAN ID is bits 20 downto 9, See figure 7, TIA1057
    lldp_u16_t vlan_id = (policy >> 9) & 0xFFF;


    // Make the string
    if (lldpmed_get_tagged_flag(policy) == 0 || lldpmed_get_unknown_policy_flag(policy))  { // See Section 10.2.3.2 and 10.2.3.3 in TIA1057
        sprintf(string_ptr, "%s", "-");
    } else {
        sprintf(string_ptr, "%u", vlan_id);
    }
}


// Converting a LLDP-MED policy priority to a printable string
//
// In : The policy
//
// In/Out: Pointer to the string.
//
void lldpmed_policy_prio2str (lldp_u32_t policy, lldp_8_t *string_ptr )
{
    // The policy contains the whole policy information. Priority is bits 8 downto 6, See figure 7, TIA1057
    lldp_u8_t prio = policy >> 6 & 0x7;

    // Make the string
    if (lldpmed_get_tagged_flag(policy) == 0 || lldpmed_get_unknown_policy_flag(policy))  { // See Section 10.2.3.2 and 10.2.3.3 in TIA1057
        sprintf(string_ptr, "%s", "-");
    } else {
        sprintf(string_ptr, "%u", prio);
    }

}

// Converting LLDP-MED policy dscp to a printable string
//
// In : The policy
//
// In/Out: Pointer to the string.
//
void lldpmed_policy_dscp2str (lldp_u32_t policy, lldp_8_t *string_ptr )
{
    // The lldmed_policy contains the whole policy information. DSCP is bits 5 downto 0, See figure 7, TIA1057
    lldp_u8_t dscp = policy & 0x3F;

    // Make the string
    if (lldpmed_get_unknown_policy_flag(policy))  { // See Section 10.2.3.2 in TIA1057
        sprintf(string_ptr, "%s", "-");
    } else {
        sprintf(string_ptr, "%u", dscp);
    }
}


//
// Returns true if the LLDP entry table needs to be updated with the information in the received LLDP frame.
//

lldp_bool_t lldpmed_update_neccessary(lldp_rx_remote_entry_t   *rx_entry, lldp_remote_entry_t   *entry)
{

    T_RG(TRACE_GRP_MED_RX, "%d, %d,%d,%d,%d,%d", (rx_entry->lldpmed_capabilities != entry->lldpmed_capabilities),
         (rx_entry->lldpmed_device_type != entry->lldpmed_device_type),
         (memcmp(rx_entry->lldpmed_policy, entry->lldpmed_policy, sizeof(entry->lldpmed_policy)) != 0),
         (memcmp(rx_entry->lldpmed_policy_vld, entry->lldpmed_policy_vld, sizeof(entry->lldpmed_policy_vld)) != 0),
         (memcmp(rx_entry->lldpmed_location_format, entry->lldpmed_location_format, sizeof(entry->lldpmed_location_format)) != 0 ),
         (memcmp(rx_entry->lldpmed_location_vld, entry->lldpmed_location_vld, sizeof(entry->lldpmed_location_vld)) != 0) );


    if (rx_entry->lldpmed_info_vld != entry->lldpmed_info_vld ||
        rx_entry->lldpmed_capabilities_vld != entry->lldpmed_capabilities_vld ||
        rx_entry->lldpmed_capabilities != entry->lldpmed_capabilities ||
        rx_entry->lldpmed_device_type != entry->lldpmed_device_type ||
        memcmp(rx_entry->lldpmed_policy, entry->lldpmed_policy, sizeof(entry->lldpmed_policy)) != 0 ||
        memcmp(rx_entry->lldpmed_policy_vld, entry->lldpmed_policy_vld, sizeof(entry->lldpmed_policy_vld)) != 0 ||
        memcmp(rx_entry->lldpmed_location_format, entry->lldpmed_location_format, sizeof(entry->lldpmed_location_format)) != 0 ||
        memcmp(rx_entry->lldpmed_location_vld, entry->lldpmed_location_vld, sizeof(entry->lldpmed_location_vld)) != 0 ||
        memcmp(rx_entry->lldpmed_hw_rev, entry->lldpmed_hw_rev, rx_entry->lldpmed_hw_rev_length) != 0 ||
        memcmp(rx_entry->lldpmed_firm_rev, entry->lldpmed_firm_rev, rx_entry->lldpmed_firm_rev_length) != 0 ||
        memcmp(rx_entry->lldpmed_sw_rev, entry->lldpmed_sw_rev, rx_entry->lldpmed_sw_rev_length) != 0 ||
        memcmp(rx_entry->lldpmed_serial_no, entry->lldpmed_serial_no, rx_entry->lldpmed_serial_no_length) != 0 ||
        memcmp(rx_entry->lldpmed_manufacturer_name, entry->lldpmed_manufacturer_name, rx_entry->lldpmed_manufacturer_name_length) != 0 ||
        memcmp(rx_entry->lldpmed_model_name, entry->lldpmed_model_name, rx_entry->lldpmed_model_name_length) != 0 ||
        memcmp(rx_entry->lldpmed_assert_id, entry->lldpmed_assert_id, rx_entry->lldpmed_assert_id_length) != 0) {
        return LLDP_TRUE;

    }
    lldp_u8_t loc_index ;
    for (loc_index = 0; loc_index < MAX_LLDPMED_LOCATION_CNT; loc_index++) {
        T_RG(TRACE_GRP_MED_RX, "rx_entry->lldpmed_location_vld[loc_index] = %d", rx_entry->lldpmed_location_vld[loc_index]);
        if (rx_entry->lldpmed_location_vld[loc_index]) {
            if (memcmp(entry->lldpmed_location[loc_index], rx_entry->lldpmed_location[loc_index], rx_entry->lldpmed_location_length[loc_index]) != 0) {
                return LLDP_TRUE;
            }
        }
    }

    return LLDP_FALSE;
}

//
// Copies the information from the received packet (rx_enty) to the entry table.
//
void lldpmed_update_entry(lldp_rx_remote_entry_t   *rx_entry, lldp_remote_entry_t   *entry)
{
    T_DG_PORT(TRACE_GRP_MED_RX, (vtss_port_no_t)entry->receive_port, "lldpmed_update_entry LLDP-MEM update");

    // Section 11.2.4, TIA1057 specifies that medFastStart timer must be set if
    // the LLDP-MED capabilities TLV has changed
    lldp_struc_0_t conf;
    lldp_get_config(&conf, VTSS_ISID_LOCAL); // Get current configuration
    if ((rx_entry->lldpmed_capabilities != entry->lldpmed_capabilities) ||
        (rx_entry->lldpmed_info_vld &&  !entry->lldpmed_info_vld)) {
        T_NG_PORT(TRACE_GRP_MED_RX, (vtss_port_no_t)entry->receive_port,
                  "rx.lldpmed_capabilities:%lu, entry.lldpmed_capabilities:%d, rx.lldpmed_info_vld:%d, entry.lldpmed_info_vld:%d",
                  rx_entry->lldpmed_capabilities, entry->lldpmed_capabilities, rx_entry->lldpmed_info_vld, entry->lldpmed_info_vld);
        lldpmed_medFastStart_timer_action(rx_entry->receive_port, conf.medFastStartRepeatCount, SET); // Section 11.2.4, TIA1057
    }


    // Do the update
    entry->lldpmed_info_vld           = rx_entry->lldpmed_info_vld;
    entry->lldpmed_capabilities_vld   = rx_entry->lldpmed_capabilities_vld;
    entry->lldpmed_capabilities       = rx_entry->lldpmed_capabilities;
    entry->lldpmed_capabilities_current = rx_entry->lldpmed_capabilities_current;
    entry->lldpmed_device_type        = rx_entry->lldpmed_device_type;
    memcpy(entry->lldpmed_policy, rx_entry->lldpmed_policy, sizeof(entry->lldpmed_policy));
    memcpy(entry->lldpmed_policy_vld, rx_entry->lldpmed_policy_vld, sizeof(entry->lldpmed_policy_vld));



    lldp_u8_t loc_index;
    for (loc_index = 0; loc_index < MAX_LLDPMED_LOCATION_CNT; loc_index++) {
        if (rx_entry->lldpmed_location_vld[loc_index]) {
            memcpy(entry->lldpmed_location[loc_index], rx_entry->lldpmed_location[loc_index], rx_entry->lldpmed_location_length[loc_index]);
        }
    }
    memcpy(entry->lldpmed_location_length, rx_entry->lldpmed_location_length, sizeof(rx_entry->lldpmed_location_length));
    memcpy(entry->lldpmed_location_vld, rx_entry->lldpmed_location_vld, sizeof(entry->lldpmed_location_vld));
    memcpy(entry->lldpmed_location_format, rx_entry->lldpmed_location_format, sizeof(entry->lldpmed_location_format));

    entry->lldpmed_hw_rev_length = rx_entry->lldpmed_hw_rev_length;
    memcpy(entry->lldpmed_hw_rev, rx_entry->lldpmed_hw_rev, rx_entry->lldpmed_hw_rev_length);

    entry->lldpmed_firm_rev_length = rx_entry->lldpmed_firm_rev_length;
    memcpy(entry->lldpmed_firm_rev, rx_entry->lldpmed_firm_rev, rx_entry->lldpmed_firm_rev_length);

    entry->lldpmed_sw_rev_length = rx_entry->lldpmed_sw_rev_length;
    memcpy(entry->lldpmed_sw_rev, rx_entry->lldpmed_sw_rev, rx_entry->lldpmed_sw_rev_length);

    entry->lldpmed_serial_no_length = rx_entry->lldpmed_serial_no_length;
    memcpy(entry->lldpmed_serial_no, rx_entry->lldpmed_serial_no, rx_entry->lldpmed_serial_no_length);

    entry->lldpmed_manufacturer_name_length = rx_entry->lldpmed_manufacturer_name_length;
    memcpy(entry->lldpmed_manufacturer_name, rx_entry->lldpmed_manufacturer_name, rx_entry->lldpmed_manufacturer_name_length);

    entry->lldpmed_model_name_length = rx_entry->lldpmed_model_name_length;
    memcpy(entry->lldpmed_model_name, rx_entry->lldpmed_model_name, rx_entry->lldpmed_model_name_length);

    entry->lldpmed_assert_id_length  = rx_entry->lldpmed_assert_id_length;
    memcpy(entry->lldpmed_assert_id, rx_entry->lldpmed_assert_id, rx_entry->lldpmed_assert_id_length);
}

