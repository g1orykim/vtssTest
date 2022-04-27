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


/*****************************************************************************
* This file contains code for handling received LLDP frames containing Organizationally Specific TLVs
* ( Annex G and Annex F in IEEE802.1AB )
* The Version used for delevoping this code is 802.1AB - released at 6 May, 2005.
******************************************************************************/


#include "lldporg_spec_tlvs_rx.h"
#include "lldp.h"
//
// Validate a TLV, and updates the LLDP entry table with the
// information
//
// In: tlv - Pointer to the TLV that shall be validated
//         len - The length of the TLV
//
// Out: rx_entry - Pointer to the entry that shall have information updated
//
// Return: TRUE if the TLV was accepted.
lldp_bool_t lldporg_validate_lldpdu(lldp_8_t   *tlv, lldp_rx_remote_entry_t   *rx_entry, lldp_u16_t len)
{
    lldp_bool_t org_tlv_supported = FALSE; // By default we don't support any org tlvs

    // Switch upon subtype
    switch (tlv[5]) {
    case 0x01 : // Subtype MAC/PHY  - Figure G-1,IEEE802.1AB
        if (len == 9) { // Length Must always be 9 for this TLV, Figure G-1, IEEE802.1AB.
            rx_entry->lldporg_autoneg_vld = TRUE;
            rx_entry->lldporg_autoneg = tlv[6]; // Figure G-1,IEEE802.1AB
            rx_entry->lldporg_autoneg_advertised_capa = (tlv[7] << 8) + tlv[8]; // Figure G-1,IEEE802.1AB
            rx_entry->lldporg_operational_mau_type = (tlv[9] << 8) + tlv[10]; // Figure G-1,IEEE802.1AB
            org_tlv_supported = TRUE;
        }
        break;

    default:
        org_tlv_supported = FALSE;
    }


    return org_tlv_supported;
}




//
// Converting a LLDP-MED autonegotiation support to a printable string
//
// In : entry - Entry containg the information
//
// In/Out: string_ptr - Pointer to the string.
//
void lldporg_autoneg_support2str (lldp_remote_entry_t   *entry, char *string_ptr)
{

    //  Table G-2, IEEE802.1AB
    if (entry->lldporg_autoneg & 0x1) {
        strcpy(string_ptr, "Supported");
    } else {
        strcpy(string_ptr, "Not Supported");
    }
}

//
// Converting a LLDP-MED autonegotiation status to a printable string
//
// In : entry - Entry containg the information
//
// In/Out: string_ptr - Pointer to the string.
//
void lldporg_autoneg_status2str (lldp_remote_entry_t   *entry, char *string_ptr)
{

    // Table G-2, IEEE802.1AB
    if (entry->lldporg_autoneg & 0x2) {
        strcpy(string_ptr, "Enabled");
    } else {
        strcpy(string_ptr, "Disabled");
    }
}


//
// Converting a LLDP-MED autonegotiation status to a printable string
//
// In : entry - Entry containg the information
//
// In/Out: string_ptr - Pointer to the string.
//
void lldporg_operational_mau_type2str (lldp_remote_entry_t   *entry, char *string_ptr)
{
    // String text found at rfc3636
    const lldp_8_t *operational_mau_type_strings[] = {"Invalid MAU Type",
                                                      "No internal MAU, view from AUI",
                                                      "10Base5 - Thick coax MAU",
                                                      "FOIRL MAU",
                                                      "10Base2 - Thin coax MAU",
                                                      "10BaseT - UTP MAU",
                                                      "10BaseFP - Passive fiber MAU",
                                                      "10BaseFB - Sync fiber MAU",
                                                      "10BaseFL - Async fiber MAU",
                                                      "Broad36 - Broadband DTE MAU",
                                                      "10BaseTHD - UTP MAU, half duplex mode",
                                                      "10BaseTFD - UTP MAU, full duplex mode",
                                                      "10BaseFLHD - Async fiber MAU, half duplex mode",
                                                      "10BaseFLFD - Async fiber MAU, full duplex mode",
                                                      "100BaseT4 - 4 pair category 3 UTP",
                                                      "100BaseTXHD - 2 pair category 5 UTP, half duplex mode",
                                                      "100BaseTXFD - 2 pair category 5 UTP, full duplex mode",
                                                      "100BaseFXHD - X fiber over PMT, half duplex mode",
                                                      "100BaseFXFD - X fiber over PMT, full duplex mode",
                                                      "100BaseT2HD - 2 pair category 3 UTP, half duplex mode",
                                                      "100BaseT2FD - 2 pair category 3 UTP, full duplex mode",
                                                      "1000BaseXHD - PCS/PMA, unknown PMD, half duplex mode",
                                                      "1000BaseXFD - PCS/PMA, unknown PMD, full duplex mode",
                                                      "1000BaseLXHD- Fiber over long-wavelength laser, half duplex mode",
                                                      "1000BaseLXFD - Fiber over long-wavelength laser, full duplex mode",
                                                      "1000BaseSXHD - Fiber over short-wavelength laser, half duplex mode",
                                                      "1000BaseSXFD - Fiber over short-wavelength laser, full duplex mode",
                                                      "1000BaseCXHD - Copper over 150-Ohm balanced cable, half duplex mode",
                                                      "1000BaseCXFD - Copper over 150-Ohm balanced cable, fill duplex mode",
                                                      "1000BaseTHD - Four-pair Category 5 UTP, half duplex mode",
                                                      "1000BaseTFD - Four-pair Category 5 UTP, full duplex mode",
                                                      "10GigBaseX - X PCS/PMA, unknown PMD.",
                                                      "10GigBaseLX4 - X fiber over WWDM optics",
                                                      "10GigBaseR - R PCS/PMA, unknown PMD.",
                                                      "10GigBaseER - R fiber over 1550 nm optics",
                                                      "10GigBaseLR - R fiber over 1310 nm optics",
                                                      "10GigBaseSR - R fiber over 850 nm optics",
                                                      "10GigBaseW - W PCS/PMA, unknown PMD.",
                                                      "10GigBaseEW - W fiber over 1550 nm optics",
                                                      "10GigBaseLW - W fiber over 1310 nm optics",
                                                      "10GigBaseSW - W fiber over 850 nm optics"
                                                     };


    // So far only the first 40 types is defined in RFC 3636
    if (entry->lldporg_operational_mau_type > 40) {
        strcpy(string_ptr, operational_mau_type_strings[0]); // Set text to invalid
    } else {
        strcpy(string_ptr, operational_mau_type_strings[entry->lldporg_operational_mau_type]);
    }
}


//
// Converting a LLDP-MED autonegotiation status to a printable string
//
// In : entry - Entry containg the information
//
// In/Out: string_ptr - Pointer to the string.
//
void lldporg_autoneg_capa2str (lldp_remote_entry_t   *entry, char *string_ptr)
{
    strcpy(string_ptr, "");
    lldp_bool_t print_comma = LLDP_FALSE;
    lldp_u8_t bit_no;
    // String text found at - http://standards.ieee.org/reading/ieee/interp/802.1AB.html
    const lldp_8_t *advertised_types_strings[] = {"1000BASE-T full duplex mode",
                                                  "1000BASE-T half duplex mode",
                                                  "1000BASE-X, -LX, -SX, -CX full duplex mode ",
                                                  "1000BASE-X, -LX, -SX, -CX half duplex mode",
                                                  "Asymmetric and Symmetric PAUSE for full-duplex inks",
                                                  "Symmetric PAUSE for full-duplex links",
                                                  "Asymmetric PAUSE for full-duplex links",
                                                  "PAUSE for full-duplex links",
                                                  "100BASE-T2 full duplex mode",
                                                  "100BASE-T2 half duplex mode",
                                                  "100BASE-TX full duplex mode",
                                                  "100BASE-TX half duplex mode",
                                                  "100BASE-T4",
                                                  "100BASE-T full duplex mode",
                                                  "10BASE-T half duplex mode",
                                                  "other or unknown"
                                                 };

    for (bit_no = 0; bit_no <= 15; bit_no++) {
        if (entry->lldporg_autoneg_advertised_capa & (1 << bit_no)) {
            if (print_comma) {
                strncat(string_ptr, ", ", 254);
            }
            strncat(string_ptr, advertised_types_strings[bit_no], 254);
            /* print enabled/disabled indicaion */
            print_comma = LLDP_TRUE;
        }
    }
}




//
// Returns true if the LLDP entry table needs to be updated with the information in the received LLDP frame.
//

lldp_bool_t lldporg_update_neccessary(lldp_rx_remote_entry_t   *rx_entry, lldp_remote_entry_t   *entry)
{

    if (rx_entry->lldporg_operational_mau_type != entry->lldporg_operational_mau_type ||
        rx_entry->lldporg_autoneg_vld != entry->lldporg_autoneg_vld ||
        rx_entry->lldporg_autoneg != entry->lldporg_autoneg ||
        rx_entry->lldporg_autoneg_advertised_capa != entry->lldporg_autoneg_advertised_capa) {
        return LLDP_TRUE;

    }
    return LLDP_FALSE;
}

//
// Copies the information from the received packet (rx_enty) to the entry table.
//
void lldporg_update_entry(lldp_rx_remote_entry_t   *rx_entry, lldp_remote_entry_t   *entry)
{
    entry->lldporg_autoneg_vld = rx_entry->lldporg_autoneg_vld;
    entry->lldporg_operational_mau_type = rx_entry->lldporg_operational_mau_type;
    entry->lldporg_autoneg = rx_entry->lldporg_autoneg;
    entry->lldporg_autoneg_advertised_capa = rx_entry->lldporg_autoneg_advertised_capa;
}


