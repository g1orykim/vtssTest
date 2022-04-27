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
* This file contains code for handling received LLDP EEE frames. EEE is
* defined in IEEE802.3az. The version used for delevoping this code is 2010.
******************************************************************************/
#ifdef VTSS_SW_OPTION_EEE

#include "lldp.h"
#include "lldp_print.h"
#include "vtss_lldp.h"
#include "lldp_os.h"
#include "lldp_private.h"
#include "eee_shared.h" /* For EEE_TLV_SUBTYPE */
#include "eee_api.h"    /* For eee_mgmt_remote_state_change() */

// Validates an EEE TLV and updates the LLDP entry table with the information.
//
// In: tlv - Pointer to the TLV that shall be validated
//     len - The length of the TLV
//
// Out: rx_entry - Pointer to the entry that shall have information updated
//
// Return: TRUE if the TLV was accepted.
lldp_bool_t eee_validate_lldpdu(lldp_u8_t *tlv, lldp_rx_remote_entry_t *rx_entry, lldp_u16_t len)
{
    lldp_bool_t org_tlv_supported = LLDP_FALSE; // By default we don't support any org tlvs

    // Switch on subtype
    switch (tlv[5]) {
    case EEE_TLV_SUBTYPE :
        if (len == 14) { // Length Must always be 14 for this TLV, Figure 79-5a,IEEE802.3az
            rx_entry->eee.valid          = LLDP_TRUE;
            rx_entry->eee.RemTxTwSys     = (tlv[6]  << 8) + tlv[7];  // Figure 79-5a,IEEE802.3az
            rx_entry->eee.RemRxTwSys     = (tlv[8]  << 8) + tlv[9];  // Figure 79-5a,IEEE802.3az
            rx_entry->eee.RemFbTwSys     = (tlv[10] << 8) + tlv[11]; // Figure 79-5a,IEEE802.3az
            rx_entry->eee.RemTxTwSysEcho = (tlv[12] << 8) + tlv[13]; // Figure 79-5a,IEEE802.3az
            rx_entry->eee.RemRxTwSysEcho = (tlv[14] << 8) + tlv[15]; // Figure 79-5a,IEEE802.3az
            org_tlv_supported = TRUE;
            T_DG_PORT(TRACE_GRP_EEE, rx_entry->receive_port, "RemTxTwSys = %u, tlv[6] = %d, tlv[7] = %d", rx_entry->eee.RemTxTwSys, tlv[6], tlv[7]);
        }
        break;

    default:
        org_tlv_supported = LLDP_FALSE;
    }

    T_DG(TRACE_GRP_EEE, "Validating LLDP for EEE %d, type = %d", org_tlv_supported, tlv[5]);
    return org_tlv_supported;
}

// Returns true if the LLDP entry table needs to be updated with the information in the received LLDP frame.
//
// In : rx_entry - Pointer to the last received lldp infomation
//      entry    - Pointer to the current lldp information stored in the entry table.
//
// Return: TRUE if current entry information isn't equal to the last received lldp information
lldp_bool_t eee_update_necessary(lldp_rx_remote_entry_t *rx_entry, lldp_remote_entry_t *entry)
{
    if (rx_entry->eee.valid) {
        // Always send it to the EEE module, whether it has changed or not, because the info already stored
        // in the LLDP database may be from a previous link partner.
        // This shouldn't have been called from within the base module, and the base module should always
        // have parsed EEE TLVs even if EEE was not part of a build, i.e. this file should not have
        // been guarded by VTSS_SW_OPTION_EEE.
        (void)eee_mgmt_remote_state_change(rx_entry->receive_port, rx_entry->eee.RemRxTwSys, rx_entry->eee.RemTxTwSys, rx_entry->eee.RemRxTwSysEcho, rx_entry->eee.RemTxTwSysEcho);
    }
    return memcmp(&rx_entry->eee, &entry->eee, sizeof(lldp_eee_t)) == 0 ? LLDP_FALSE : LLDP_TRUE;
}

/// Copies the information from the received packet (rx_enty) to the entry table.
//
//  In : rx_entry - Pointer to the last received lldp infomation.
//       entry    - Pointer to the entry that shall be updated
void eee_update_entry(lldp_rx_remote_entry_t *rx_entry, lldp_remote_entry_t *entry)
{
    entry->eee = rx_entry->eee;
}

#endif
