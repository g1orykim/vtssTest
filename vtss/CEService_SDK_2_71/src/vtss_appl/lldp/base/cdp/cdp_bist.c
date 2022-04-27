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
// ************************************************************************
// This file contains function for doing CDP (Cisco Discovery Protocol ) build in test.
// The design specification for this module is included in DS0138.
// ************************************************************************


/* ************************************************************************
 * Includes
 * ************************************************************************ */

#include "lldp.h"
#include "main.h"
#include "cdp_analyse.h"
#include "vtss_trace_api.h"
#include <network.h> // For htons
#include "lldp_print.h"
#ifdef VTSS_SW_OPTION_PACKET
#include "packet_api.h"
#endif

/* ************************************************************************
 * Defines
 * ************************************************************************ */

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

/* ************************************************************************
 * Functions
 * ************************************************************************ */

#define AGEOUT_TOLERANCE 2 // When measuring ageout we expected it to be with 2 second.

// The Chassis ID we are using for our BIST test.
void cdp_get_chassis_id_str (lldp_8_t *string_ptr)
{
    // We uses our MAC address as device/chassis ID
    vtss_common_macaddr_t mac_addr;
    vtss_os_get_systemmac(&mac_addr);
    /*lint --e{64} */
    mac_addr2str(&mac_addr.macaddr[0], string_ptr);
    strcat(string_ptr, "-BIST");
}

// Appending CDP device ID TLV (LLDP chassis TLV) to the CDP frame being generated.
static uint16_t cdp_append_device_id_tlv (uint8_t *cdp_frame)
{
    uint8_t frame_len = 0;
    lldp_8_t string_ptr[100];

    cdp_frame[frame_len++] = 0x00;
    cdp_frame[frame_len++] = CDP_TYPE_DEVICE_ID;

    cdp_get_chassis_id_str(string_ptr);

    cdp_frame[frame_len++] = 0x00;
    cdp_frame[frame_len++] = strlen(string_ptr) + 4; // TLV 1 Length.

    memcpy(&cdp_frame[frame_len], string_ptr, strlen(string_ptr));
    frame_len += strlen(string_ptr);

    return frame_len;
}

// Appending invalid CDP  ID to the CDP frame being generated.
static uint16_t cdp_append_invalid_tlv (uint8_t *cdp_frame)
{
    uint8_t frame_len = 0;

    // Not used TLV number
    cdp_frame[frame_len++] = 0x00;
    cdp_frame[frame_len++] = 0x44; // TLV number
    cdp_frame[frame_len++] = 0x00;
    cdp_frame[frame_len++] = 4; // TLV (valid) Length.


    // Invalid TLV number
    cdp_frame[frame_len++] = 0x00;
    cdp_frame[frame_len++] = 0x00; // TLV number
    cdp_frame[frame_len++] = 0x00;
    cdp_frame[frame_len++] = 4; // TLV (valid) Length.

    // Invalid length ( Must be the LAST TLV - Exceeding the CDP frame ).
    cdp_frame[frame_len++] = 0x00;
    cdp_frame[frame_len++] = 0x44; // TLV number
    cdp_frame[frame_len++] = 0x00;
    cdp_frame[frame_len++] = 7; // TLV (invalid) Length.


    return frame_len;
}



// Appending CDP port ID TLV to the CDP frame being generated.
static uint16_t cdp_append_port_id_tlv (uint8_t *cdp_frame, uint8_t port_no)
{
    uint8_t frame_len = 0;
    lldp_8_t string_ptr[100];

    cdp_frame[frame_len++] = 0x00;
    cdp_frame[frame_len++] = CDP_TYPE_PORT_ID;

    sprintf(string_ptr, "%u", port_no);

    cdp_frame[frame_len++] = 0x00;
    cdp_frame[frame_len++] = strlen(string_ptr) + 4; // TLV 1 Length.

    memcpy(&cdp_frame[frame_len], string_ptr, strlen(string_ptr));
    frame_len += strlen(string_ptr);

    return frame_len;
}


// Appending CDP address to the CDP frame being generated.
static uint16_t cdp_append_address_tlv (uint8_t *cdp_frame)
{

    uint8_t frame_len = 0;
    uint8_t our_ip[4];

    cdp_frame[frame_len++] = 0x00;
    cdp_frame[frame_len++] = CDP_TYPE_ADDRESS;


    cdp_frame[frame_len++] = 0x00; // Length
    cdp_frame[frame_len++] = 0x11; // Length

    cdp_frame[frame_len++] = 0x00; // Number of Addresses
    cdp_frame[frame_len++] = 0x00; // Number of Addresses
    cdp_frame[frame_len++] = 0x00; // Number of Addresses
    cdp_frame[frame_len++] = 0x01; // Number of Addresses

    cdp_frame[frame_len++] = 0x01; // Protocol type
    cdp_frame[frame_len++] = 0x01; // Length

    cdp_frame[frame_len++] = 0xCC; // Protocol = IP

    cdp_frame[frame_len++] = 0x00; // Address length
    cdp_frame[frame_len++] = 0x04; // Address length

    /*lint --e{64} */
    lldp_os_get_ip_address(&our_ip[0], 0);
    cdp_frame[frame_len++] = our_ip[0]; // Ip address.
    cdp_frame[frame_len++] = our_ip[1]; // Ip address.
    cdp_frame[frame_len++] = our_ip[2]; // Ip address.
    cdp_frame[frame_len++] = our_ip[3]; // Ip address.

    T_NG(TRACE_GRP_CDP, "Our IP = %d.%d.%d.%d", our_ip[0], our_ip[1], our_ip[2], our_ip[3]);
    return frame_len;
}

// Appending CDP version and platform to the CDP frame being generated.
static uint16_t cdp_append_version_platform_tlv (uint8_t *cdp_frame)
{
    uint8_t frame_len = 0;
    lldp_8_t string_ptr[100];

    cdp_frame[frame_len++] = 0x00;
    cdp_frame[frame_len++] = CDP_TYPE_IOS_VERSION;

    strcpy(string_ptr, "CDP_TYPE_IOS_VERSION");

    cdp_frame[frame_len++] = 0x00;
    cdp_frame[frame_len++] = strlen(string_ptr) + 4; // TLV Length.

    memcpy(&cdp_frame[frame_len], string_ptr, strlen(string_ptr));
    frame_len += strlen(string_ptr);

    //
    // TLV type CDP_TYPE_PLATFORM
    //

    cdp_frame[frame_len++] = 0x00;
    cdp_frame[frame_len++] = CDP_TYPE_PLATFORM;

    strcpy(string_ptr, "CDP_TYPE_PLATFORM");

    cdp_frame[frame_len++] = 0x00;
    cdp_frame[frame_len++] = strlen(string_ptr) + 4; // TLV Length.

    memcpy(&cdp_frame[frame_len], string_ptr, strlen(string_ptr));
    frame_len += strlen(string_ptr);

    return frame_len;
}

// Appending CDP capabilities TLV to the CDP frame being generated.
static uint16_t cdp_append_capa_tlv (uint8_t *cdp_frame)
{
    uint8_t frame_len = 0;
    cdp_frame[frame_len++] = 0x00;
    cdp_frame[frame_len++] = CDP_TYPE_CAPABILITIES;


    cdp_frame[frame_len++] = 0x00;
    cdp_frame[frame_len++] = 8; // TLV Length.

    cdp_frame[frame_len++] = 0x00;
    cdp_frame[frame_len++] = 0x00;
    cdp_frame[frame_len++] = 0x00;
    cdp_frame[frame_len++] = 0x0F; // 0xF = Brigde(+),Router(+)

    return frame_len;
}

// Generate a "test" CDP frame which we transmits out to a port that the user has loopbacked to the next port.
static uint16_t cdp_generate_cdp_frame (uint8_t *cdp_frame, uint8_t port_no, BOOL include_invalid_tlv, uint8_t ttl_value)
{
    vtss_common_macaddr_t mac_addr;

    vtss_os_get_systemmac(&mac_addr);


    uint16_t frame_len = 0;
    uint16_t check_sum = 0;

    /* fill in SA, DA + eth Type */
    cdp_frame[frame_len++] = 0x01;
    cdp_frame[frame_len++] = 0x00;
    cdp_frame[frame_len++] = 0x0C;
    cdp_frame[frame_len++] = 0xCC;
    cdp_frame[frame_len++] = 0xCC;
    cdp_frame[frame_len++] = 0xCC;

    memcpy(&cdp_frame[6], mac_addr.macaddr, VTSS_COMMON_MACADDR_SIZE);
    frame_len += 6;

    // Frame length set to 0 for now. Will be updated later on.
    cdp_frame[frame_len++] = 0x00;
    cdp_frame[frame_len++] = 0x00;


    cdp_frame[frame_len++] = 0xAA; // DSAP
    cdp_frame[frame_len++] = 0xAA; // SSAP
    cdp_frame[frame_len++] = 0x03; // Control Feild
    cdp_frame[frame_len++] = 0x00; // Organization code
    cdp_frame[frame_len++] = 0x00; // Organization code
    cdp_frame[frame_len++] = 0x0C; // Organization code
    cdp_frame[frame_len++] = 0x20; // PID
    cdp_frame[frame_len++] = 0x00; // PID


    cdp_frame[frame_len++] = 0x02; // Protocol Version
    cdp_frame[frame_len++] = ttl_value;

    // CDP Checksum - Set to 0x0 for now - Will be updated later
    cdp_frame[frame_len++] = 0x00;
    cdp_frame[frame_len++] = 0x00;



    // Append TLVs
    frame_len += cdp_append_device_id_tlv(&cdp_frame[frame_len]);
    frame_len += cdp_append_port_id_tlv(&cdp_frame[frame_len], port_no);
    frame_len += cdp_append_address_tlv(&cdp_frame[frame_len]);
    frame_len += cdp_append_capa_tlv(&cdp_frame[frame_len]);
    frame_len += cdp_append_version_platform_tlv(&cdp_frame[frame_len]);


    // Appending invalid TLV
    if (include_invalid_tlv) {
        frame_len += cdp_append_invalid_tlv(&cdp_frame[frame_len]);
    }

    // Update length field
    cdp_frame[12] = (frame_len >> 8) & 0xFF;
    cdp_frame[13] = frame_len & 0xFF;

    // Update checksum
    check_sum = cdp_checksum(&cdp_frame[22], frame_len - 22);
    cdp_frame[24] = check_sum & 0xFF;
    cdp_frame[25] = (check_sum >> 8) & 0xFF;

    return frame_len;
}

void cdp_transmit_frame (uint8_t port_no, BOOL include_invalid_tlv, uint8_t ttl_value)
{
    uint8_t *frm_p;
    uint8_t frame[1518];
    vtss_port_no_t port_index = port_no - VTSS_PORT_NO_START;
    packet_tx_props_t tx_props;

    T_DG_PORT(TRACE_GRP_CDP, port_index, "Trying to transmit CDP frame");

    uint16_t frame_len = cdp_generate_cdp_frame(&frame[0], port_no, include_invalid_tlv, ttl_value);

    // Allocate frame buffer. Exclude room for FCS.
    if ((frm_p = packet_tx_alloc(frame_len)) == NULL) {
        T_W("CDP packet_tx_alloc failed.");
        return;
    } else {
        memset(frm_p, 0, 64); // Do manual padding since FDMA isn't able to do it, and the FDMA requires minimum 64 bytes.
        memcpy(frm_p, frame, frame_len);
    }

#ifdef VTSS_SW_OPTION_PACKET
    packet_tx_props_init(&tx_props);
    tx_props.packet_info.modid     = VTSS_MODULE_ID_LLDP; // In lack of a CDP module ID.
    tx_props.packet_info.frm[0]    = frm_p;
    tx_props.packet_info.len[0]    = frame_len;
    tx_props.tx_info.dst_port_mask = VTSS_BIT64(port_no);
    if (packet_tx(&tx_props) != VTSS_RC_OK) {
        T_E("CDP frame was not transmitted correctly");
    } else {
        T_DG_PORT(TRACE_GRP_CDP, port_index, "CDP frame transmitted");
    }
#endif
    VTSS_OS_MSLEEP(400); // Make sure that the frame is transmitted
}


// Function for compariong chassis id in a LLDP entry.
BOOL is_chassis_correct (char *expected_chassis_name, lldp_remote_entry_t *entry)
{
    lldp_8_t chassis_string[MAX_CHASSIS_ID_LENGTH];
    lldp_remote_chassis_id_to_string(entry, &chassis_string[0]);
    T_NG(TRACE_GRP_CDP, "expected_chassis_name = %s, chassis_string =%s", expected_chassis_name, chassis_string);
    return ! strcmp(expected_chassis_name, chassis_string);
}


//
// Function for checking that the TLV in the LLDP entry matches the information
// send by with the CDP bist frame.
//
BOOL is_tlvs_correct (lldp_remote_entry_t *entry, uint8_t tx_port)
{
    lldp_8_t string_ptr[MAX_MGMT_LENGTH];
    lldp_8_t compare_string[MAX_MGMT_LENGTH];
    lldp_u8_t our_ip[4];

    // Check chassis Id.
    cdp_get_chassis_id_str(string_ptr); // Get the chassis id we uses for bist.
    if (!is_chassis_correct(&string_ptr[0], entry)) {
        T_EG(TRACE_GRP_CDP, "Chassis doesn't match - %s", string_ptr );
        return FALSE;
    }

    // Check system description
    if (strcmp(entry->system_description, "CDP_TYPE_IOS_VERSION\nCDP_TYPE_PLATFORM")) {
        T_EG(TRACE_GRP_CDP, "System Description doesn't match %s, %d", entry->system_description, strcmp(entry->system_description, "CDP_TYPE_IOS_VERSION CDP_TYPE_PLATFORM"));
        return FALSE;
    }

    // Check IP address
    lldp_os_get_ip_address(&our_ip[0], 0);
    ip_addr2str((lldp_8_t *)  &our_ip[0], &string_ptr[0]);
    lldp_remote_mgmt_addr_to_string(entry, &compare_string[0], FALSE, 0);
    strcat(string_ptr, " (IPv4)");
    if (strcmp(compare_string, string_ptr)) {
        T_EG(TRACE_GRP_CDP, "IP doesn't match got = %s , expected = %s", compare_string, string_ptr);
        return FALSE;
    }

    // Check Port ID
    sprintf(string_ptr, "%u", tx_port);
    lldp_remote_port_id_to_string(entry, &compare_string[0]);

    if (strcmp(compare_string, string_ptr)) {
        T_EG(TRACE_GRP_CDP, "Port ID doesn't match got = %s , expected = %s", compare_string, string_ptr);
        return FALSE;
    }

    // Check capabilities
    strcpy(string_ptr, "Bridge(+), Router(+)");
    strcpy(compare_string, "");
    lldp_remote_system_capa_to_string(entry, &compare_string[0]);
    if (strcmp(compare_string, string_ptr)) {
        T_EG(TRACE_GRP_CDP, "Capabilities doesn't match %s , %s", compare_string, string_ptr);
        return FALSE;
    }

    return TRUE; // OK - No error found
}

//
// Function for checking statistic counters
//
BOOL is_stat_counter_correct(uint8_t rx_port, vtss_isid_t isid, uint16_t expected_value)
{
    lldp_counters_rec_t stat_table[LLDP_PORTS], *stat;

    // Get statistic counters
    lldp_mgmt_get_lock();
    (void)lldp_mgmt_stat_get(isid, &stat_table[0], NULL, NULL);
    lldp_mgmt_get_unlock();

    // Pick the counters for the port in question.
    stat = &stat_table[rx_port - VTSS_PORT_NO_START];

    T_RG(TRACE_GRP_CDP, "Statistic counters = %u", stat->rx_total);
    if (stat->rx_total != expected_value ||
        stat->rx_error != 0) {
        T_EG(TRACE_GRP_CDP, "Statistic counters error, expected %u, got %u", expected_value, stat->rx_total);
        return FALSE;
    }
    return TRUE; // OK - Statistic counters are as expected
}


//
// Function that checks the LLDP neighbors table contains the expected into fro a certian port.
// It is possible to check that the neighbors table doesn't contain a entry by setting the expect_entry BOOL
//
BOOL is_entry_corretly_in_table(BOOL expect_entry, uint16_t rx_port, vtss_isid_t isid, uint8_t tx_port)
{
    lldp_remote_entry_t *table = NULL, *entry;
    BOOL entry_not_found = true; // Signaling if an entry corresponding to the port being was found in the LLDP table.

    // Get the entries in the LLDP neighbors table
    lldp_mgmt_get_lock();
    table = lldp_mgmt_get_entries(isid);
    lldp_mgmt_get_unlock();
    uint8_t i;
    for (i = 0, entry = table; i < lldp_remote_get_max_entries(); i++, entry++) {
        if (entry->in_use == 0 || entry->receive_port != (rx_port - VTSS_PORT_NO_START)) {
            continue;
        }

        entry_not_found = false; // OK  - A Valid entry was found.

        // Check TLVs
        if (!is_tlvs_correct(entry, tx_port) && expect_entry) {
            return FALSE;
        }
    }

    // Check that a entry was found (if expected)
    if (entry_not_found && expect_entry) {
        T_EG(TRACE_GRP_CDP, "No entry found");
        return FALSE;
    }

    // Check that no entry was found (if that is what is expected)
    if (!entry_not_found && !expect_entry) {
        T_EG(TRACE_GRP_CDP, "Entry unexpected found");
        return FALSE;
    }

    return TRUE;
}


//
// Function for checking TTL ( age out )
//
BOOL is_aged_out(uint8_t rx_port, vtss_isid_t isid, uint8_t tx_port, uint8_t ttl_value)
{

    uint8_t ageout_counter;

    for (ageout_counter = 0; ageout_counter < (ttl_value - AGEOUT_TOLERANCE) ; ageout_counter++ ) {
        // Check that the LLDP neighbors table doesn't contain the CDP bist entry
        if (!is_entry_corretly_in_table(TRUE, rx_port, isid, tx_port)) {
            T_EG(TRACE_GRP_CDP, "Entry removed to early from the LLDP neighbors table ( removed after %d sec. shouldn't be removed before %d sec.)",
                 ageout_counter, ttl_value);
            return FALSE;
        }
        VTSS_OS_MSLEEP(1000);
    }

    VTSS_OS_MSLEEP((AGEOUT_TOLERANCE * 1000)); // We allow a 1 second "slip" because we can not measure that precisely

    // Check that the LLDP neighbors table doesn't contain the CDP bist entry
    if (!is_entry_corretly_in_table(FALSE, rx_port, isid, tx_port)) {
        T_EG(TRACE_GRP_CDP, "Entry not removed from the LLDP neighbors table");
        return FALSE;
    }

    // Check Age out counter


    return TRUE; // OK - Age out worked
}

//
// CDP Build In Self Test
//
lldp_8_t *cdp_bist(const BOOL port_list[VTSS_PORT_ARRAY_SIZE], vtss_isid_t isid)
{
    uint8_t tx_port, rx_port = 0;
    lldp_16_t port_idx;
    uint8_t ttl_value = 0;
    uint16_t test_cnt = 0;

    // For doing BIST we must set LLDP to TX only to avoid being "confushed" by the LLDP frames.
    lldp_struc_0_t      conf;
    lldp_mgmt_get_config(&conf, isid);
    for (port_idx = 0; port_idx < LLDP_PORTS; port_idx++) {
        conf.admin_state[port_idx] = LLDP_ENABLED_RX_ONLY;
    }

    // Get admin state
    vtss_rc rc = lldp_mgmt_set_admin_state(conf.admin_state, isid);
    if (rc != VTSS_OK) {
        T_W("%s \n", lldp_error_txt(rc));
    }


    lldp_mgmt_stat_clr(isid); // Clear statistic counter for starting with known values.
    VTSS_OS_MSLEEP(100); // Make sure that cousters are cleared, before the CDP frame is transmitted.


    for (tx_port = VTSS_PORT_NO_START; tx_port < VTSS_PORT_NO_END; tx_port++) {
        if (port_list[tx_port] == 0 || PORT_NO_IS_STACK(tx_port)) {
            continue;
        }

        T_IG(TRACE_GRP_CDP, "Starting test for tx_port %d", tx_port);

        // Do the test a random number of times
        for (test_cnt = 1; test_cnt < (rand() % 200); test_cnt ++ ) {
            rx_port = tx_port + 1 ; // User must make physical loopback between the port being test and the next port
            ttl_value = (rand() % 10) + AGEOUT_TOLERANCE;
            cdp_transmit_frame(tx_port, TRUE, ttl_value); // Transmit a BIST CDP frame

            // Check entry in LLDP neighbors table
            if (!is_entry_corretly_in_table(TRUE, rx_port, isid, tx_port)) {
                goto bist_error;
            }

            // Check statistic counters
            if (!is_stat_counter_correct(rx_port, isid, test_cnt)) {
                goto bist_error;
            }
        }

        if (!is_aged_out(rx_port, isid, tx_port, ttl_value)) {
            goto bist_error;
        }
    }


    return "Bist PASSED";
bist_error:
    return "Bist FAILED";
}



