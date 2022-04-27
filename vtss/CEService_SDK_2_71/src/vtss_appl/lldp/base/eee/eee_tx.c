/*

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

/*****************************************************************************
* This file contains code for handling received LLDP EEE frames. EEE is
* defined in IEEE802.3az. The Version used for delevoping this code is 2010.
******************************************************************************/

#ifdef VTSS_SW_OPTION_EEE

#include "lldp.h"
#include "vtss_lldp.h"
#include "lldp_tlv.h"
#include "lldp_os.h"
#include "eee_api.h"
#include "eee_shared.h" // For EEE_TLV_SUBTYPE

//
// Append EEE to the LLDP frame being build.
//
// In: port_idx - The port in question
//
// In/Out: Buf - Pointer to buffer containing the LLDP frame
//
// Return: The length of the buffer
static lldp_u8_t eee_update_tlv(lldp_u8_t *buf, lldp_port_t port_idx)
{
    int               i = 0;
    eee_switch_conf_t eee_switch_conf;
    eee_port_state_t  eee_port_state;

    if (eee_mgmt_switch_conf_get(VTSS_ISID_LOCAL,           &eee_switch_conf) != VTSS_RC_OK ||
        eee_mgmt_port_state_get (VTSS_ISID_LOCAL, port_idx, &eee_port_state)  != VTSS_RC_OK) {
        return 0;
    }

    T_DG_PORT(TRACE_GRP_EEE, port_idx, "eee_update_tlv");
    if (eee_switch_conf.port[port_idx].eee_ena && eee_port_state.link_partner_eee_capable) {
        T_DG_PORT(TRACE_GRP_EEE, port_idx, "Port enabled");
        // OUI - Figure 79-5a, IEEE802.3az
        buf[i++] = 0x00;
        buf[i++] = 0x12;
        buf[i++] = 0x0F;

        buf[i++] = EEE_TLV_SUBTYPE; // Subtype - TBD  Figure 79-5a, IEEE802.3az

        // Transmit Tw
        u16 tx_tw = eee_port_state.LocTxSystemValue;
        buf[i++] =  (tx_tw >> 8) & 0xFF;
        buf[i++] =  tx_tw & 0xFF;

        // Receive Tw
        u16 rx_tw = eee_port_state.LocRxSystemValue;
        buf[i++] =  (rx_tw >> 8) & 0xFF; // eee_api_conf.rx_wakeup_time[port_idx] >> 8 & 0xFF;
        buf[i++] =  rx_tw & 0xFF; //eee_api_conf.rx_wakeup_time[port_idx] & 0xFF;

        // Fallback Tw. Not supported - Set to Receive Tw, IEEE 802.3az section 79.3.0.2
        u16 fall_back = eee_port_state.LocFbSystemValue;
        buf[i++] =  (fall_back >> 8) & 0xFF ; // eee_api_conf.rx_wakeup_time[port_idx] >> 8 & 0xFF;
        buf[i++] =  fall_back  & 0xFF; //eee_api_conf.rx_wakeup_time[port_idx] & 0xFF;

        // Echo Transmit Tw
        u16 echo_tx_tw = eee_port_state.LocTxSystemValueEcho;
        buf[i++] =  (echo_tx_tw >> 8) & 0xFF;
        buf[i++] =  echo_tx_tw & 0xFF;

        // Echo Receive Tw
        u16 echo_rx_tw = eee_port_state.LocRxSystemValueEcho;
        buf[i++] =  (echo_rx_tw >> 8) & 0xFF;
        buf[i++] =  echo_rx_tw & 0xFF;

        T_DG_PORT(TRACE_GRP_EEE, port_idx, "rx_tw =%d, tx_tw = %d", rx_tw, tx_tw);
    }
    return i;
}

// Function for adding EEE TLV information
//
// In: port_idx - The port in question
//
// In/Out: Buf - Pointer to buffer containing the LLDP frame
//
// Return the length of the TLV.
lldp_u16_t eee_tlv_add(lldp_u8_t *buf, lldp_port_t port_idx)
{
    lldp_u16_t tlv_info_len = 0;

    // Organizationally TLV
    tlv_info_len +=  set_tlv_type_and_length_non_zero_len(buf + tlv_info_len, LLDP_TLV_ORG_TLV, eee_update_tlv(buf + 2 + tlv_info_len, port_idx));

    T_DG_PORT(TRACE_GRP_EEE, port_idx, "tlv_info_len = %d", tlv_info_len);
    return tlv_info_len;
}

#endif

