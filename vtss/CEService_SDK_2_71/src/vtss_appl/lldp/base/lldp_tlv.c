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


#include "lldp_sm.h"
#include "lldp.h"
#include "vtss_common_os.h"
#include "lldp_tlv.h"
#include "lldp_private.h"
#include "misc_api.h"

#ifdef VTSS_SW_OPTION_LLDP_MED
#include "lldpmed_tx.h"
#endif

#ifdef VTSS_SW_OPTION_EEE
#include "eee_tx.h"
#endif

#ifdef VTSS_SW_OPTION_POE
#include "poe_api.h"
#include "poe_custom_api.h"
#endif

/* ************************************************************************ **
 *
 *
 * Defines
 *
 *
 *
 ************************************************************************* */

/* ************************************************************************ **
 *
 *
 * Typedefs and enums
 *
 *
 *
 * ************************************************************************ */

/* ************************************************************************ **
 *
 *
 * Prototypes for local functions
 *
 *
 *
 * ************************************************************************ */
static lldp_u16_t append_chassis_id(lldp_u8_t *buf);
static lldp_u16_t append_port_id(lldp_u8_t *buf, lldp_port_t port);
static lldp_u16_t append_ttl(lldp_u8_t *buf, lldp_port_t port);
static lldp_u16_t append_end_of_pdu(void);
static lldp_16_t  append_port_descr(lldp_u8_t *buf, lldp_port_t port, lldp_port_t sid);
static lldp_u16_t append_system_name(lldp_u8_t *buf);
static lldp_u16_t append_system_descr(lldp_u8_t *buf);
static lldp_u16_t append_system_capabilities (lldp_u8_t *buf);
#ifdef VTSS_SW_OPTION_POE
static lldp_u16_t append_poe(lldp_u8_t *buf, lldp_port_t port);
#endif
static lldp_u16_t append_mgmt_address(lldp_u8_t *buf, lldp_port_t port);
/* ************************************************************************ **
  * Functions
  * ************************************************************************ */
lldp_u16_t lldp_tlv_add(lldp_u8_t *buf, lldp_u16_t cur_len, lldp_tlv_t tlv, lldp_port_t port_idx)
{
    lldp_u16_t tlv_info_len = 0;

    switch (tlv) {
    case LLDP_TLV_BASIC_MGMT_CHASSIS_ID:
        T_R("Getting LLDP_TLV_BASIC_MGMT_CHASSIS_ID");
        tlv_info_len = append_chassis_id(buf + 2);
        break;

    case LLDP_TLV_BASIC_MGMT_PORT_ID:
        T_R("Getting LLDP_TLV_BASIC_MGMT_PORT_ID");
        tlv_info_len = append_port_id(buf + 2, port_idx);
        break;

    case LLDP_TLV_BASIC_MGMT_TTL:
        tlv_info_len = append_ttl(buf + 2, port_idx);
        break;

    case LLDP_TLV_BASIC_MGMT_END_OF_LLDPDU:
        tlv_info_len = append_end_of_pdu();
        break;

    case LLDP_TLV_BASIC_MGMT_PORT_DESCR:
        T_NG_PORT(TRACE_GRP_TX, port_idx, "Getting LLDP_TLV_BASIC_MGMT_PORT_DESCR");
        tlv_info_len = append_port_descr(buf + 2, iport2uport(port_idx), lldp_os_get_usid());
        break;

    case LLDP_TLV_BASIC_MGMT_SYSTEM_NAME:
        T_R("Getting LLDP_TLV_BASIC_MGMT_SYSTEM_NAME");
        tlv_info_len = append_system_name(buf + 2);
        break;

    case LLDP_TLV_BASIC_MGMT_SYSTEM_DESCR:
        T_R("Getting LLDP_TLV_BASIC_MGMT_SYSTEM_DESCR");
        tlv_info_len = append_system_descr(buf + 2);
        break;

    case LLDP_TLV_BASIC_MGMT_SYSTEM_CAPA:
        T_R("Getting LLDP_TLV_BASIC_MGMT_SYSTEM_CAPA");
        tlv_info_len = append_system_capabilities(buf + 2);
        break;

    case LLDP_TLV_BASIC_MGMT_MGMT_ADDR:
        T_R("Getting LLDP_TLV_BASIC_MGMT_MGMT_ADDR");
        tlv_info_len = append_mgmt_address(buf + 2, port_idx);
        break;
    case LLDP_TLV_ORG_TLV:

#ifdef VTSS_SW_OPTION_LLDP_MED
        T_IG_PORT(TRACE_GRP_TX, port_idx, "Adding LLDP-MED TLV");
        tlv_info_len += lldpmed_tlv_add(buf + tlv_info_len, port_idx);
#endif

#ifdef VTSS_SW_OPTION_EEE
        tlv_info_len += eee_tlv_add(buf + tlv_info_len, port_idx);
#endif

#ifdef VTSS_SW_OPTION_LLDP_MED
        lldp_struc_0_t conf;
        lldp_get_config(&conf, VTSS_ISID_LOCAL); // Get current configuration
        if (conf.lldpmed_optional_tlv[port_idx] & 0x8) {  // Bit 3 is the extendedPSE TLV, see lldpXMedPortConfigTLVsTxEnable MIB.
#endif
#ifdef VTSS_SW_OPTION_POE
            T_DG(TRACE_GRP_POE, "Getting LLDP_TLV_ORG_TLV, tlv_info_len = %d", tlv_info_len);
            T_IG(TRACE_GRP_POE, "tlv_info_len = %d,cur_len = %d", tlv_info_len, cur_len);
            buf += tlv_info_len; // Point to last entry in the buffer
            tlv_info_len += set_tlv_type_and_length_non_zero_len (buf, tlv, append_poe(buf + 2, port_idx)); // Append PoE TLV
            T_IG(TRACE_GRP_POE, "tlv_info_len = %d,cur_len = %d", tlv_info_len, cur_len);
#endif // VTSS_SW_OPTION_POE
#ifdef VTSS_SW_OPTION_LLDP_MED
        }
#endif

        if (tlv_info_len == 0) {
            return cur_len; // Return cur_len if no organizationally TLV is supported
        } else  {
            return cur_len + tlv_info_len;
        }

    default:
        T_D("Unhandled TLV Type %u", (unsigned)tlv);
        return cur_len;
    }

    set_tlv_type_and_length (buf, tlv, tlv_info_len);

    T_NG(TRACE_GRP_TX, "cur_len = %d, tlv_info_len = %d, tlv = %d", cur_len, tlv_info_len, tlv);
    /* add additional 2 octets for header */
    return cur_len + 2 + tlv_info_len;
}

lldp_u16_t lldp_tlv_add_zero_ttl(lldp_u8_t *buf, lldp_u16_t cur_len)
{
    buf[2] = 0;
    buf[3] = 0;
    set_tlv_type_and_length (buf, LLDP_TLV_BASIC_MGMT_TTL, 2);
    return cur_len + 4;
}

lldp_u32_t lldp_tlv_mgmt_addr_len (void)
{
    return 5;
}

lldp_u8_t lldp_tlv_get_local_port_id (lldp_port_t port, lldp_8_t *port_str)
{
    sprintf(port_str, "%u", port);
    T_D("port_str = %s, len = %d", &port_str[0], strlen(port_str));
    return strlen(port_str);
}

lldp_u16_t set_tlv_type_and_length_non_zero_len (lldp_u8_t *buf, lldp_tlv_t tlv_type, lldp_u16_t tlv_info_string_len)
{
    T_IG(TRACE_GRP_POE, "tlv_info_string_len =%d", tlv_info_string_len);
    if (tlv_info_string_len != 0) {
        set_tlv_type_and_length (buf, tlv_type, tlv_info_string_len);
        return tlv_info_string_len + 2;
    }
    return 0;
}

void set_tlv_type_and_length (lldp_u8_t *buf, lldp_tlv_t tlv_type, lldp_u16_t tlv_info_string_len)
{
    buf[0] = (0xfe & ((lldp_8_t)tlv_type << 1)) | (tlv_info_string_len >> 8);
    buf[1] = tlv_info_string_len & 0xFF;
    T_NG(TRACE_GRP_TX, "TLV type : 0x%X, 0x%X, tlv_type = 0x%X, tlv_info_string_len = 0x%X, buf_addr = %p", buf[0], buf[1], tlv_type, tlv_info_string_len, buf);
}

static lldp_u16_t append_chassis_id (lldp_u8_t *buf)
{
    vtss_common_macaddr_t mac_addr;

    /*
    ** we append MAC address, which gives us length MAC_ADDRESS + Chassis id Subtype, hence
    ** information string length = 7
    */
    buf[0] = lldp_tlv_get_chassis_id_subtype(); /* chassis ID subtype */

    mac_addr = lldp_os_get_masters_mac();
    memcpy(&buf[1], mac_addr.macaddr, VTSS_COMMON_MACADDR_SIZE);
    return 7;
}

static lldp_u16_t append_port_id (lldp_u8_t *buf, lldp_port_t port)
{
    lldp_u8_t len;

    buf[0] = lldp_tlv_get_port_id_subtype(); /* Port ID subtype */
    len = lldp_tlv_get_local_port_id(port, (lldp_8_t *) &buf[1]);
    return 1 + len;
}

static lldp_u16_t append_ttl (lldp_u8_t *buf, lldp_port_t port)
{
    lldp_sm_t   *sm;
    sm = lldp_get_port_sm(port);

    buf[0] = HIGH_BYTE(sm->tx.txTTL);
    buf[1] = LOW_BYTE(sm->tx.txTTL);

    return 2;
}

static lldp_u16_t append_end_of_pdu (void)
{
    return 0;
}


static lldp_16_t append_port_descr (lldp_u8_t *buf, lldp_port_t port, lldp_port_t sid)
{
    T_DG_PORT(TRACE_GRP_TX, port, "Entering append_port_descr, sid = %d", sid);
    lldp_os_get_if_descr(sid, port, (lldp_8_t *) buf);
    return strlen((lldp_8_t *) buf);
}

static lldp_u16_t append_system_name (lldp_u8_t *buf)
{
    lldp_tlv_get_system_name((lldp_8_t *) buf);
    return strlen((lldp_8_t *) buf);
}

static lldp_u16_t append_system_descr (lldp_u8_t *buf)
{
    lldp_tlv_get_system_descr((lldp_8_t *) buf);
    return strlen((lldp_8_t *) buf);
}

int lldp_tlv_get_system_capabilities (void)
{
    /*
    ** The Vitesse implementation of LLDP always (at least at the time of writing)
    ** runs on a bridge (that has bridging enabled)
    */
    return 4;
}

int lldp_tlv_get_system_capabilities_ena (void)
{
    /*
    ** The Vitesse implementation of LLDP always (at least at the time of writing)
    ** runs on a bridge (that has bridging enabled)
    */
    return 4;
}

#ifdef VTSS_SW_OPTION_POE
// Appending Power Over Ethernet TLV
static lldp_u16_t append_poe(lldp_u8_t *buf, lldp_port_t port_index)
{
    // Variable containing the power information.
    lldp_u8_t power_conf = 0x0;   //

    // Get the PoE configuration
    poe_local_conf_t poe_local_conf;
    poe_mgmt_get_local_config(&poe_local_conf, VTSS_ISID_LOCAL);

    if (poe_local_conf.poe_mode[port_index] != POE_MODE_POE_DISABLED) { // Note 2, for table 10.2.1.1, TIA1057
        lldp_u8_t power_source = (lldp_u8_t) poe_custom_get_power_source();

        // Get the request power from any PD on the port
        lldp_u16_t requested_power = lldp_remote_get_requested_power(port_index, poe_local_conf.poe_mode[port_index]);

        poe_status_t          poe_status;
        poe_mgmt_get_status(VTSS_ISID_LOCAL, &poe_status); // Get the status fields.

        //ieee_draft_version = 0 means TIA1057
        lldp_u8_t ieee_draft_version = lldp_remote_get_ieee_draft_version(port_index);
        T_IG_PORT(TRACE_GRP_POE, port_index, "ieee_draft_version = %u", ieee_draft_version);
        if (ieee_draft_version != 0) {
            T_IG_PORT(TRACE_GRP_POE, port_index, "Generating IEEE TLV");
            // OUI - See table 33-18 in IEEE 803.2at/D1
            buf[0] = (lldp_8_t) 0x00;
            buf[1] = (lldp_8_t) 0x12;
            buf[2] = (lldp_8_t) 0x0F;

            // Power type = 0 - We are always a power source  entity ( PSE )
            power_conf |= power_source << 4;

            // Set the power source
            power_conf |= power_source << 4;
            T_DG_PORT(TRACE_GRP_POE, port_index, "power_source = %d, power_conf 1 = 0x%X",
                      power_source, power_conf);

            // Set power priority -- - See figure 33-26 in IEEE 803.2at/D3
            switch (poe_local_conf.priority[port_index]) {
            case LOW :
                power_conf |= 0x3;
                break;
            case HIGH:
                power_conf |= 0x2;
                break;
            case CRITICAL:
                power_conf |= 0x1;
                break;
            default:
                break;
                // 0x0 is Unknown
            }

            if (ieee_draft_version == 1 || ieee_draft_version == 3) {
                //
                //  figure 33-26 in IEEE 803.2at/D3 or table 33-18, IEEE 802.1at/D1
                //

                buf[3] = (lldp_u8_t) 0x5; // Subtype

                T_NG_PORT(TRACE_GRP_POE, port_index, "prio = %d, power_conf 2 =  0x%X", poe_local_conf.priority[port_index], power_conf);
                buf[4] = (lldp_u8_t) power_conf;

                // Power Value
                buf[5] = (lldp_8_t) (requested_power >> 8); // High part of the power
                buf[6] = (lldp_8_t) requested_power; // Low part of the power

                T_NG_PORT(TRACE_GRP_POE, port_index, "requested_power = %u", requested_power);

                if (ieee_draft_version == 1) {
                    buf[7] = 01; // Has to be updated
                    return 8; // Length always 8 - See table 33-18 in IEEE 803.2at/D1
                } else {
                    // We defaults to version IEEE802.3at/D3
                    buf[7] = (lldp_8_t) power_conf;
                    buf[8] = (lldp_8_t) (poe_status.power_allocated[port_index] >> 8); // High part of the power
                    buf[9] = (lldp_8_t) poe_status.power_allocated[port_index]; // Low part of the power

                    buf[10] = 01; // Acknowledge - See Table 33-26 in IEEE 803.2at/D3
                    return 11; // Length always 11 - See figure 33-26 in IEEE 803.2at/D3
                }
            } else {
                // Get hardware board configuration for this port
                poe_custom_entry_t board_poe_conf;
                board_poe_conf = poe_custom_get_hw_config(port_index, &board_poe_conf);

                buf[3] = (lldp_u8_t) 0x2; // Subtype, Figure 79-2, IEEE 803.2at/D4

                // MDI power support, Figure 79-2, IEEE 803.2at/D4
                buf[4] = (lldp_u8_t) 0x0; // Set to 0 just to make sure that we can set the bits individually (see below)
                buf[4] |= 1; // We are always a PSE - Table G-3, IEEE 802.1AB-2005
                buf[4] |= board_poe_conf.available << 1; // Table G-3, IEEE 802.1AB-2005
                if (lldp_os_get_admin_status(port_index) == LLDP_ENABLED_RX_TX ||
                    lldp_os_get_admin_status(port_index) == LLDP_ENABLED_TX_ONLY) {
                    buf[4] |= 1 << 2; // Table G-3, IEEE 802.1AB-2005
                }

                buf[4] |= board_poe_conf.pse_pairs_control_ability << 3; // Table G-3, IEEE 802.1AB-2005

                // PSE power Pair, Figure 79-2, IEEE 803.2at/D4
                buf[5] = board_poe_conf.pse_power_pair;

                // Power Class, Figure 79-2, IEEE 803.2at/D4
                buf[6] = poe_status.pd_class[port_index];

                // Type/source/priority , Figure 79-2, IEEE 803.2at/D4
                buf[7] = (lldp_8_t) power_conf;

                // PD requested power, , Figure 79-2, IEEE 803.2at/D4
                buf[8] = (lldp_8_t) ((requested_power >> 8) & 0xFF);
                buf[9] = (lldp_8_t) requested_power & 0xFF;

                // PSE allocated power value, , Figure 79-2, IEEE 803.2at/D4
                buf[10] = (lldp_8_t) ((poe_status.power_allocated[port_index] >> 8) & 0xFF);
                buf[11] = poe_status.power_allocated[port_index] & 0xFF;

                T_IG_PORT(TRACE_GRP_POE, port_index, "return 12");
                return 12; // Length always 12 for this TLV. Figure 79-2, IEEE 803.2at/D4
            }

        } else {
            T_IG_PORT(TRACE_GRP_POE, port_index, "Generating TIA TLV");
            // TIA OUI = 00-12-BB, See section 10.2.5, figure 12 in TIA-1057
            buf[0] = 0x00;
            buf[1] = 0x12;
            buf[2] = 0xBB;

            // Extended Power via MDI Subtype = 04, See section 10.2.5, figure 12 in TIA-1057
            buf[3] = (lldp_u8_t) 0x4;

            // Set the power source
            power_conf |= power_source << 4;

            // Set power priority -- See table 17 in TIA-1057
            switch (poe_local_conf.priority[port_index]) {
            case LOW :
                power_conf |= 0x3;
                break;
            case HIGH:
                power_conf |= 0x2;
                break;
            case CRITICAL:
                power_conf |= 0x1;
                break;
            default:
                break;
                // 0x0 is Unknown
            }
            T_NG_PORT(TRACE_GRP_POE, port_index, "prio = %d, power_conf 2 =  0x%X", poe_local_conf.priority[port_index], power_conf);
            buf[4] = (lldp_8_t) power_conf;

            // Power Value
            buf[5] = (lldp_8_t) ((poe_status.power_allocated[port_index] >> 8) & 0xFF);
            buf[6] = poe_status.power_allocated[port_index] & 0xFF;

            T_NG_PORT(TRACE_GRP_POE, port_index, "TIA alocated = %u", poe_status.power_allocated[port_index]);
            T_DG_PORT(TRACE_GRP_POE, port_index, "Appending PoE = 0x%X,0x%X,0x%X,0x%X,0x%X,0x%X,0x%X,", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6]);


            return 7; // Extended Power Via MDI String Length is always 7, See section 10.2.5, figure 12 in TIA-1057
        }
    } else {
        T_DG_PORT(TRACE_GRP_POE, port_index, "PoE is disabled");
        return 0; // This port has PoE disabled so we don't transmit any LLDP PoE information
    }

}
#endif // VTSS_SW_OPTION_POE

static lldp_u16_t append_system_capabilities (lldp_u8_t *buf)
{
    int capa = lldp_tlv_get_system_capabilities();
    int capa_ena = lldp_tlv_get_system_capabilities_ena();

    buf[0] = (lldp_8_t) (capa >> 8);
    buf[1] = (lldp_8_t) capa;
    buf[2] = (lldp_8_t)(capa_ena >> 8);
    buf[3] = (lldp_8_t) capa_ena;
    T_DG(TRACE_GRP_TX, "Appending system capabilities = %c %c", buf[0], buf[1]);
    T_DG(TRACE_GRP_TX, "Appending system capabilities ena = %c %c", buf[2], buf[3]);

    return 4;
}

char lldp_tlv_get_mgmt_addr_len (void)
{
    /* management address length = length(subtype + address) */
    return 5;
}

char lldp_tlv_get_mgmt_addr_subtype (void)
{
    /* management address subtype */
    return 1; /* IPv4 */
}

char lldp_tlv_get_mgmt_if_num_subtype (void)
{
    /* Interface Numbering subtype */
    return 2; /* ifIndex */
}

char lldp_tlv_get_mgmt_oid (void)
{
    return 0;
}

static lldp_u16_t append_mgmt_address (lldp_u8_t *buf, lldp_port_t port)
{
    lldp_u32_t mgmt_if_index = 0;
    /* we receive a port parameter even though we don't care about it here
    ** (more exotic future implementations might have management addresses
    ** per-vlan, so the port is included to support this in some sense.
    */
    /*lint --e{438} */
    port = port;

    /* management address length = length(subtype + address) */
    buf[0] = lldp_tlv_get_mgmt_addr_len();

    /* management address subtype */
    buf[1] = lldp_tlv_get_mgmt_addr_subtype();

    /* IPv4 Address */
    lldp_os_get_ip_address(&buf[2], port);

    /* Interface Numbering subtype */
    buf[6] = lldp_tlv_get_mgmt_if_num_subtype();

    /* Interface number */
    buf[7]  = (mgmt_if_index >> 24) & 0xFF;
    buf[8]  = (mgmt_if_index >> 16) & 0xFF;
    buf[9]  = (mgmt_if_index >>  8) & 0xFF;
    buf[10] = (mgmt_if_index >>  0) & 0xFF;

    /* OID Length */
    buf[11] = 0;

    /* if this function changes, make sure to update the lldp_tlv_mgmt_addr_len()
    ** function with the correct value: (from the MIB definition)
    ** "The total length of the management address subtype and the
    ** management address fields in LLDPDUs transmitted by the
    ** local LLDP agent."
    */
    return 12;
}
