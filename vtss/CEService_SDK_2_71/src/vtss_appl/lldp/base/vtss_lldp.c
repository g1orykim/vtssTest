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

#include "lldp.h"
#include "lldp_print.h"
#include "vtss_lldp.h"
#include "lldp_tlv.h"
#include "lldp_os.h"
#include "lldp_remote.h"
#include "lldp_private.h"
#include "misc_api.h" /* For iport2uport() */
#include "conf_api.h" // conf_mgmt_mac_addr_get
#ifdef VTSS_SW_OPTION_LLDP_MED
#include "lldpmed_rx.h"
#endif // VTSS_SW_OPTION_LLDP_MED

#ifdef VTSS_SW_OPTION_LLDP_ORG
#include "lldporg_spec_tlvs_rx.h"
#endif // VTSS_SW_OPTION_LLDP_ORG


#ifdef VTSS_SW_OPTION_EEE
#include "eee_rx.h"
#endif // VTSS_SW_OPTION_EEE

/* ************************************************************************ **
 *
 *
 * Local data
 *
 *
 * ************************************************************************ */
/* the state machines, one per switch port */
static lldp_sm_t  lldp_sm[LLDP_PORTS];
#ifdef VTSS_SW_OPTION_POE
#endif //VTSS_SW_OPTION_POE

/* frame_len is file-global as it is shared between 2 functions */
static lldp_u16_t frame_len;

static lldp_8_t   *lldp_rx_frame;
static lldp_u16_t lldp_rx_frame_len;

lldp_sm_t *lldp_get_port_sm(lldp_port_t port)
{
    return &lldp_sm[port];
}

void lldp_tx_initialize_lldp(lldp_sm_t *sm)
{
    sm->tx.somethingChangedLocal = LLDP_FALSE;
    sm->adminStatus = lldp_os_get_admin_status(sm->port_no);
    sm->tx.re_evaluate_timers = LLDP_FALSE;
#ifdef VTSS_SW_OPTION_LLDP_MED
    T_NG_PORT(TRACE_GRP_TX, (vtss_port_no_t)sm->port_no, "Setting medTransmitEnabled to FALSE");
    lldpmed_medTransmitEnabled_set(sm->port_no, LLDP_FALSE); // medTransmitEnabled must to be to FALSE, when TX is intialised. Section 11.2.1, bullet d), TIA1057
    lldpmed_medFastStart_timer_action(sm->port_no, 0, SET); // Set medFastStart timer to 0. Section 11.2.1,TIA1057, Bullet e)
#endif
}

void lldp_set_port_enabled(lldp_port_t port, lldp_u8_t enabled)
{
    lldp_sm_t   *sm;

    sm = lldp_get_port_sm(port);
    sm-> portEnabled = enabled;

    T_NG_PORT(TRACE_GRP_CONF, sm->port_no, "New state: %d", sm->portEnabled);
}

void lldp_set_timing_changed(void)
{
    lldp_port_t port;
    for (port = 0; port < LLDP_PORTS; port++) {
        lldp_sm[port].tx.re_evaluate_timers = LLDP_TRUE;
    }
}

/*
** Call this function on change to any of the following:
** ip address
** system name
*/
void lldp_something_changed_local(void)
{
    lldp_port_t port;
    for (port = 0; port < LLDP_PORTS; port++) {
        lldp_sm[port].tx.somethingChangedLocal = LLDP_TRUE;
    }
}

void sw_lldp_init(void)
{
    lldp_port_t port;
    for (port = 0; port < LLDP_PORTS; port++) {
        lldp_sm_init(&lldp_sm[port], port);
#ifdef VTSS_SW_OPTION_POE
        lldp_remote_set_lldp_info_valid(port, 0);
#endif
    }
}

void lldp_1sec_timer_tick(lldp_port_t port)
{
    lldp_sm_timers_tick(&lldp_sm[port]);
}

/* The transmission module has been split up into two logical functions
** to be consistent with 802.1AB
*/
void lldp_construct_info_lldpdu(lldp_port_t port_no, lldp_port_t sid)
{
    lldp_u8_t   *buf;
    lldp_tlv_t  optional_tlvs[] = { LLDP_TLV_BASIC_MGMT_PORT_DESCR,
                                    LLDP_TLV_BASIC_MGMT_SYSTEM_NAME,
                                    LLDP_TLV_BASIC_MGMT_SYSTEM_DESCR,
                                    LLDP_TLV_BASIC_MGMT_SYSTEM_CAPA,
                                    LLDP_TLV_BASIC_MGMT_MGMT_ADDR
                                  };
    lldp_u8_t tlv;
#define SHARED_BUFFER_SIZE 1500
//    uchar shared_buffer [SHARED_BUFFER_SIZE] = "0";

//    buf = &shared_buffer[0] ; //lldp_os_get_frame_storage();
    buf = lldp_os_get_frame_storage(1);

    /* reserve room for DA, SA and EthType */
    frame_len = 14;

    /* Append Mandatory TLVs */
    T_DG_PORT(TRACE_GRP_TX, port_no, "Frame length: %u", (unsigned)frame_len, port_no);
    frame_len = lldp_tlv_add(&buf[frame_len], frame_len, LLDP_TLV_BASIC_MGMT_CHASSIS_ID, port_no);
    T_DG_PORT(TRACE_GRP_TX, port_no, "Frame length: %u,", (unsigned)frame_len);
    lldp_u16_t unique_port_id = LLDP_GET_UNIQUE_PORT_ID(port_no);
    frame_len = lldp_tlv_add(&buf[frame_len], frame_len, LLDP_TLV_BASIC_MGMT_PORT_ID, unique_port_id);
    T_DG_PORT(TRACE_GRP_TX, port_no, "Frame length: %u", (unsigned)frame_len);

    frame_len = lldp_tlv_add(&buf[frame_len], frame_len, LLDP_TLV_BASIC_MGMT_TTL, port_no);
    T_DG_PORT(TRACE_GRP_TX, port_no, "Frame length: %u", (unsigned)frame_len);
    T_DG_PORT(TRACE_GRP_TX, port_no, "Construct Info PDU, length: %u", port_no, (unsigned)frame_len);
    /* Append enabled optional TLVs */
    for (tlv = 0; tlv < sizeof(optional_tlvs) / sizeof(optional_tlvs[0]); tlv++) {
        if (lldp_os_get_optional_tlv_enabled(optional_tlvs[tlv], port_no, VTSS_ISID_LOCAL)) {
            frame_len = lldp_tlv_add(&buf[frame_len], frame_len, optional_tlvs[tlv], port_no);
        }
    }

#if defined(VTSS_SW_OPTION_POE) || defined(VTSS_SW_OPTION_LLDP_MED) || defined(VTSS_SW_OPTION_EEE)
    frame_len = lldp_tlv_add(&buf[frame_len], frame_len, LLDP_TLV_ORG_TLV, port_no);
#endif


    T_DG_PORT(TRACE_GRP_TX, port_no, "Construct Info PDU, length: %u", (unsigned)frame_len);
    /* End of LLDPPDU is also mandatory */
    frame_len = lldp_tlv_add(&buf[frame_len], frame_len, LLDP_TLV_BASIC_MGMT_END_OF_LLDPDU, port_no);

    T_D("Port %u Construct Info PDU, length: %u", (unsigned)port_no, (unsigned)frame_len);
}

void lldp_pre_port_disabled(lldp_port_t port_no)
{
    if (((lldp_sm[port_no].adminStatus == LLDP_ENABLED_RX_TX) ||
         (lldp_sm[port_no].adminStatus == LLDP_ENABLED_TX_ONLY)) &&
        lldp_sm[port_no].portEnabled) {
        /* make sure we don't use shared buffer */
        lldp_os_set_tx_in_progress(LLDP_TRUE);

        /* construct & transmit shutdown LLDPDU */
        lldp_construct_shutdown_lldpdu(port_no);
        lldp_tx_frame(port_no);

        /* Now we can use shared buffer again */
        lldp_os_set_tx_in_progress(LLDP_FALSE);
    }
}

void lldp_construct_shutdown_lldpdu(lldp_port_t port_no)
{
    lldp_u8_t   *buf;

    buf = lldp_os_get_frame_storage(1);

    /* reserve room for DA, SA and EthType */
    frame_len = 14;

    /* Append Mandatory TLVs, now with zeroed TTL */
    frame_len = lldp_tlv_add(&buf[frame_len], frame_len, LLDP_TLV_BASIC_MGMT_CHASSIS_ID, port_no);
    lldp_u16_t unique_port_id = LLDP_GET_UNIQUE_PORT_ID(port_no);
    frame_len = lldp_tlv_add(&buf[frame_len], frame_len, LLDP_TLV_BASIC_MGMT_PORT_ID, unique_port_id);
    frame_len = lldp_tlv_add_zero_ttl(&buf[frame_len], frame_len);

    /* End of LLDPPDU is also mandatory */
    frame_len = lldp_tlv_add(&buf[frame_len], frame_len, LLDP_TLV_BASIC_MGMT_END_OF_LLDPDU, port_no);

    T_D("Port %u Construct Shutdown PDU, length: %u", (unsigned)port_no, (unsigned)frame_len);
}

void lldp_tx_frame(lldp_port_t port_no)
{
    lldp_u8_t   *buf;
    lldp_sm_t   *sm;

    sm = lldp_get_port_sm(port_no);
    sm->stats.statsFramesOutTotal++;



    buf = lldp_os_get_frame_storage(0);

    // Table 8-1 in IEEE 802.1AB
    buf[0] = 0x01;
    buf[1] = 0x80;
    buf[2] = 0xC2;
    buf[3] = 0x00;
    buf[4] = 0x00;
    buf[5] = 0x0E;

    // Section 8.2 in IEEE802.1AB. Get port mac address
    lldp_u8_t mac_address[6];
    if (conf_mgmt_mac_addr_get(mac_address, port_no + 1 - VTSS_PORT_NO_START)) {
        T_W("Problem getting port mac address");
        memset(&mac_address[0], 0, sizeof(mac_address));
    };
    memcpy(&buf[6], mac_address, sizeof(mac_address));

    // Table 8-2 in IEEE 802.1AB
    buf[12] = 0x88;
    buf[13] = 0xCC;

    T_DG_PORT(TRACE_GRP_TX, (vtss_port_no_t)port_no, "Tx Frame, frame_len = %d", frame_len);
    lldp_os_tx_frame(port_no, buf, frame_len);
}


//
// Function for debugging. Prints out the frame.
//
// IN : frame - Pointer to the frame to print.
//      len   - The length of the frame.
//
void lldp_printf_frame(lldp_8_t *frame, lldp_u16_t len)
{
    int i;
    char temp[10];
    for (i = 0; i < len; i++) {
        sprintf(temp, "%02X", frame[i]);
        printf("%s", temp);

        if (i + 1 < len ) {
            i++;
            sprintf(temp, "%02X ", frame[i]);
            printf("%s", temp);
        }
    }
    printf ("\n");
}

void lldp_frame_received(lldp_port_t port_no, const lldp_u8_t *const frame, lldp_u16_t len, vtss_glag_no_t glag_no)
{
    lldp_sm_t   *sm;
    T_DG_PORT(TRACE_GRP_RX, port_no, "Rx LLDP Frame, length=%u", (unsigned)len);
    /*
    ** it has already been verified that both the ethtype and the DSTMAC is as expected
    ** Now we can kick the state machine
    */
    lldp_rx_frame = (lldp_8_t *)frame;
    lldp_rx_frame_len = len;

//    lldp_printf_frame(lldp_rx_frame, len);

    sm = lldp_get_port_sm(port_no);
    sm->rx.rcvFrame = LLDP_TRUE;
    sm->glag_no = glag_no;
    lldp_sm_step(sm, TRUE); // Update the RX state machine
}

void lldp_rx_initialize_lldp(lldp_port_t port)
{
    /* Delete all information in remote systems mib with this local port */
    lldp_remote_delete_entries_for_local_port(port);
}

void lldp_statistics_clear(lldp_port_t port)
{
    memset(&lldp_sm[port].stats, 0, sizeof(lldp_statistics_t));
}

lldp_counter_t lldp_get_tx_frames(lldp_port_t port)
{
    return lldp_sm[port - 1].stats.statsFramesOutTotal;
}

lldp_counter_t lldp_get_rx_total_frames(lldp_port_t port)
{
    return lldp_sm[port - 1].stats.statsFramesInTotal;
}

lldp_counter_t lldp_get_rx_error_frames(lldp_port_t port)
{
    return lldp_sm[port - 1].stats.statsFramesInErrorsTotal;
}

lldp_counter_t lldp_get_rx_discarded_frames(lldp_port_t port)
{
    return lldp_sm[port - 1].stats.statsFramesDiscardedTotal;
}

lldp_counter_t lldp_get_TLVs_discarded(lldp_port_t port)
{
    return lldp_sm[port - 1].stats.statsTLVsDiscardedTotal;
}

lldp_counter_t lldp_get_TLVs_unrecognized(lldp_port_t port)
{
    return lldp_sm[port - 1].stats.statsTLVsUnrecognizedTotal;
}

lldp_counter_t lldp_get_TLVs_org_discarded(lldp_port_t port)
{
    return lldp_sm[port - 1].stats.statsOrgTVLsDiscarded;
}

lldp_counter_t lldp_get_ageouts(lldp_port_t port)
{
    return lldp_sm[port - 1].stats.statsAgeoutsTotal;
}

// Doing stuff that is needed when admin state is changed.
//
// In : current_admin_state - The admin state currently configured
//      new_admin_state     - The admin state that we are going to configure next.
void lldp_admin_state_changed(const admin_state_t current_admin_state, const admin_state_t new_admin_state)
{
    lldp_port_t port_idx;
    for (port_idx = 0; port_idx < LLDP_PORTS; port_idx++) {
        // The comment below is copied from IEEE802.1AB-2005, lldpStatsRxPortAgeoutsTotal MIB
        // When a port's admin status changes from 'disabled' to
        //'rxOnly', 'txOnly' or 'txAndRx', the counter associated with
        // the same port should reset to 0.
        if (current_admin_state[port_idx] == LLDP_DISABLED && new_admin_state[port_idx]  != LLDP_DISABLED) {
            lldp_sm[port_idx].stats.statsAgeoutsTotal = 0;
        }

        T_NG_PORT(TRACE_GRP_CONF, port_idx, "current admin=%d new admin=%d", current_admin_state[port_idx], new_admin_state[port_idx], port_idx);
    }
}

void lldp_get_stat_counters(lldp_counters_rec_t *cnt_array)
{
    lldp_port_t port = 0;
    int port_idx;
    for (port_idx = 0; port_idx < LLDP_PORTS; port_idx++) {
        port =  iport2uport(port_idx); // user port
        cnt_array[port_idx].tx_total            = lldp_get_tx_frames(port);
        cnt_array[port_idx].rx_total            = lldp_get_rx_total_frames(port);
        cnt_array[port_idx].rx_error            = lldp_get_rx_error_frames(port);
        cnt_array[port_idx].rx_discarded        = lldp_get_rx_discarded_frames(port);
        cnt_array[port_idx].TLVs_discarded      = lldp_get_TLVs_discarded(port);
        cnt_array[port_idx].TLVs_unrecognized   = lldp_get_TLVs_unrecognized(port);
        cnt_array[port_idx].TLVs_org_discarded  = lldp_get_TLVs_org_discarded(port);
        cnt_array[port_idx].ageouts             = lldp_get_ageouts(port);
    }
}

// Sets all counters to zero.
void lldp_clr_stat_counters(lldp_counters_rec_t *cnt_array)
{
    lldp_port_t port = 0;
    for (port = 0; port < LLDP_PORTS; port++) {
        cnt_array[port].tx_total            = 0;
        cnt_array[port].rx_total            = 0;
        cnt_array[port].rx_error            = 0;
        cnt_array[port].rx_discarded        = 0;
        cnt_array[port].TLVs_discarded      = 0;
        cnt_array[port].TLVs_unrecognized   = 0;
        cnt_array[port].TLVs_org_discarded  = 0;
        cnt_array[port].ageouts             = 0;
    }
}

static void bad_lldpdu(lldp_sm_t *sm)
{
    sm->stats.statsFramesDiscardedTotal++;
    sm->stats.statsFramesInErrorsTotal++;
    sm->rx.badFrame = LLDP_TRUE;
}

static lldp_tlv_t get_tlv_type(lldp_8_t *tlv)
{
    lldp_tlv_t type;

    type = (lldp_tlv_t) ((*tlv) >> 1) & 0x7F;
    return type;
}

static lldp_u16_t get_tlv_info_len (lldp_8_t *tlv)
{
    lldp_u16_t info_len;
    info_len = ((tlv[0] & 1) << 8) + (lldp_u8_t) tlv[1];
    return info_len;
}

static lldp_bool_t validate_tlv(lldp_8_t *tlv)
{
    lldp_tlv_t tlv_type;
    tlv_type = get_tlv_type(tlv);

    if ((tlv_type == LLDP_TLV_BASIC_MGMT_CHASSIS_ID) ||
        (tlv_type == LLDP_TLV_BASIC_MGMT_PORT_ID)    ||
        (tlv_type == LLDP_TLV_BASIC_MGMT_TTL)) {
        return LLDP_FALSE;
    }

    return LLDP_TRUE;
}

static lldp_bool_t validate_lldpdu(lldp_sm_t *sm, lldp_rx_remote_entry_t *rx_entry)
{
    u8         mgmt_addr_index = 0; // Support for multiple management addresses
    lldp_8_t   *tlv;
    lldp_8_t   *tmp_ptr;
    lldp_u16_t len;
    lldp_u8_t tlv_no;
    lldp_tlv_t tlv_type;
#ifdef VTSS_SW_OPTION_LLDP_MED
    lldp_bool_t first_lldpmed_tlv = LLDP_TRUE; // Signaling that this is the first lldp-med TLV in the LLDPDU
    rx_entry->lldpmed_capabilities_current = 0; // Reset current capabilities to none as default.
#endif

    T_NG(TRACE_GRP_RX, "Entering validate_lldpdu");
    /* do some basic validation that we have a somewhat reasonable LLDPDU to work with */
    if (lldp_rx_frame_len < 26) {
        bad_lldpdu(sm);
        return LLDP_FALSE;
    }

//    lldp_printf_frame(&lldp_rx_frame[0], lldp_rx_frame_len);

    rx_entry->smac = &lldp_rx_frame[6]; // Not part of the LLDP standard, but needed for voice vlan.

    /* the numbers below refers to sections in IEEE802.1AB */

    /* 10.3.2 - check three mandatory TLVs at beginning of LLDPDU */

    /* 10.3.2.a check CHASSIS ID TLV */
    tlv = &lldp_rx_frame[14];

    if (get_tlv_type(tlv) != LLDP_TLV_BASIC_MGMT_CHASSIS_ID) {
        T_DG(TRACE_GRP_RX, "TLV 1 not CHASSIS ID but %u", (unsigned)get_tlv_type(tlv));
        bad_lldpdu(sm);
        return LLDP_FALSE;
    }

    len = get_tlv_info_len(tlv);
    if ((256 < len) || (len < 2)) {
        T_DG(TRACE_GRP_RX, "TLV 1 length %u error", (unsigned)len);
        bad_lldpdu(sm);
        return LLDP_FALSE;
    }

    rx_entry->chassis_id_length = len - 1;
    rx_entry->chassis_id_subtype = tlv[2];

    // Make sure that chassis_id_subtype can be set to reserved.
    if (rx_entry->chassis_id_subtype == 0) {
        // Table 9-2, IEEE 802.1AB-2005
        T_I("Chassis Id subtype was reserved, reseting it to chassis component");
        rx_entry->chassis_id_subtype = 1;
    }

    rx_entry->chassis_id = &tlv[3];

    /* advance to next tlv */
    tlv += (2 + len);

    /* 10.3.2.b check PORT ID TLV */
    if (get_tlv_type(tlv) != LLDP_TLV_BASIC_MGMT_PORT_ID) {
        T_DG(TRACE_GRP_RX, "TLV 2 not PORT ID");
        bad_lldpdu(sm);
        return LLDP_FALSE;
    }

    len = get_tlv_info_len(tlv);
    if ((256 < len) || (len < 2)) {
        T_DG(TRACE_GRP_RX, "TLV 2 length %u error", (unsigned)len);
        bad_lldpdu(sm);
        return LLDP_FALSE;
    }

    rx_entry->port_id_length = len - 1;
    rx_entry->port_id_subtype = tlv[2];
    rx_entry->port_id = &tlv[3];

    /* advance to next tlv */
    tlv += (2 + len);

    /* 10.3.2.c check TIME TO LIVE TLV */
    if (get_tlv_type(tlv) != LLDP_TLV_BASIC_MGMT_TTL) {
        T_DG(TRACE_GRP_RX, "TLV 3 not TTL");
        bad_lldpdu(sm);
        return LLDP_FALSE;
    }

    len = get_tlv_info_len(tlv);
    if (len < 2) {
        T_DG(TRACE_GRP_RX, "TLV 3 length %u error", (unsigned)len);
        bad_lldpdu(sm);
        return LLDP_FALSE;
    }

    rx_entry->ttl = (tlv[2] << 8) | (tlv[3] & 0xFF);
    sm->rx.rxTTL = rx_entry->ttl;

    /* advance to next tlv */
    tlv += (2 + len);

    /* we have reached tlv # 4 */
    tlv_no = 4;


    /* iterate all other TLVs and save those we support */
    while (tlv < (lldp_rx_frame + lldp_rx_frame_len)) {
        if (!validate_tlv(tlv)) {
            T_DG(TRACE_GRP_RX, "TLV number  %u error, tlv type  = %d ", (unsigned)tlv_no, get_tlv_type(tlv));
            bad_lldpdu(sm);
            return LLDP_FALSE;
        }

        len = get_tlv_info_len(tlv);
        tlv_type = get_tlv_type(tlv);

        if (((tlv_type == LLDP_TLV_BASIC_MGMT_PORT_DESCR) ||
             (tlv_type == LLDP_TLV_BASIC_MGMT_SYSTEM_NAME) ||
             (tlv_type == LLDP_TLV_BASIC_MGMT_SYSTEM_DESCR)) &&
            (256 < len)) {
            T_DG(TRACE_GRP_RX, "TLV %u length %u error. tlv[0] = 0x%X, tlv[1] = 0x%X, ",
                 (unsigned)tlv_type, (unsigned)len, tlv[0], tlv[1]);
            bad_lldpdu(sm);
            return LLDP_FALSE;
        }

        /*
        ** we know for sure that this TLV is not:
        ** chassis id, port id or ttl
        */
        switch (tlv_type) {
        case LLDP_TLV_BASIC_MGMT_PORT_DESCR:
            rx_entry->port_description_length = len;
            rx_entry->port_description = &tlv[2];
            T_NG(TRACE_GRP_RX, "Setting port_description_length = %d, and port description to %s",
                 rx_entry->port_description_length,
                 rx_entry->port_description);
            break;

        case LLDP_TLV_BASIC_MGMT_SYSTEM_NAME:
            rx_entry->system_name_length = len;
            rx_entry->system_name = &tlv[2];
            break;

        case LLDP_TLV_BASIC_MGMT_SYSTEM_DESCR:
            rx_entry->system_description_length = len;
            rx_entry->system_description = &tlv[2];
            break;

        case LLDP_TLV_BASIC_MGMT_SYSTEM_CAPA:
            if (len == 4) {
                memcpy(rx_entry->system_capabilities, &tlv[2], 4);
            } else {
                T_DG(TRACE_GRP_RX, "Bad system capabilities, len = %d", len);
                bad_lldpdu(sm);
                return LLDP_FALSE;
            }
            break;

        case LLDP_TLV_BASIC_MGMT_MGMT_ADDR:
            if ((8 < len) && (len < 168)) {
                rx_entry->mgmt_addr[mgmt_addr_index].subtype = tlv[3];
                if ((1 < tlv[2]) && (tlv[2] < 34)) {
                    rx_entry->mgmt_addr[mgmt_addr_index].length = tlv[2] - 1;
                    rx_entry->mgmt_addr[mgmt_addr_index].mgmt_address = &tlv[4];
                } else {
                    T_DG(TRACE_GRP_RX, "Bad mgmt address");
                    bad_lldpdu(sm);
                    return LLDP_FALSE;
                }

                tmp_ptr = tlv + 3 + tlv[2];

                rx_entry->mgmt_addr[mgmt_addr_index].if_number_subtype = tmp_ptr[0];
                rx_entry->mgmt_addr[mgmt_addr_index].if_number = &tmp_ptr[1];

                rx_entry->mgmt_addr[mgmt_addr_index].oid_length = tmp_ptr[5];
                rx_entry->mgmt_addr[mgmt_addr_index].oid = &tmp_ptr[6];
                mgmt_addr_index++; // Next management address
            } else {
                T_DG(TRACE_GRP_RX, "Bad management address");
                bad_lldpdu(sm);
                return LLDP_FALSE;
            }
            break;

        case LLDP_TLV_BASIC_MGMT_END_OF_LLDPDU:
            T_NG(TRACE_GRP_RX, "LLDP_TLV_BASIC_MGMT_END_OF_LLDPDU");
            /* end validation thing immediately */
            return LLDP_TRUE;

        case LLDP_TLV_ORG_TLV:
            T_NG_PORT(TRACE_GRP_POE, (vtss_port_no_t) rx_entry->receive_port, "ORG TLV = 0x%X, 0x%X, 0x%X, 0x%X, 0x%X, 0x%X, 0x%X, 0x%X, 0x%X",
                      tlv[0], tlv[1], tlv[2], tlv[3], tlv[4], tlv[5], tlv[6], tlv[7], tlv[8]);


            lldp_bool_t org_tlv_supported = FALSE; // By default we don't support any org tlvs
            lldp_u8_t  org_oui[3];
            org_oui[0]    = tlv[2]; // Figure 9-12 in IEEE802.1-2005
            org_oui[1]    = tlv[3]; // Figure 9-12 in IEEE802.1-2005
            org_oui[2]    = tlv[4]; // Figure 9-12 in IEEE802.1-2005


            const lldp_u8_t LLDP_TLV_OUI_8023[]  = {0x00, 0x12, 0x0F}; //  Figure 33-26 in IEEE 802.3at/D3
            const lldp_u8_t LLDP_TLV_OUI_TIA[]  = {0x00, 0x12, 0xBB}; //  Figure 12 in TIA1057

            // Check if the OUI is of TIA-1057 type
            if (memcmp(org_oui, &LLDP_TLV_OUI_TIA[0], 3) == 0) {
                // We take special care of PoE because PoE is included in the some software
                // packages where LLDP-MED isn't.
                if (tlv[5] == 0x04) { // Subtype PoE - See Figure 12 in TIA1057
#ifdef VTSS_SW_OPTION_POE
                    if (len == 7) { // Length - See Figure 12 in TIA1057
                        T_NG(TRACE_GRP_POE, "PoE TIA ");
                        rx_entry->tia_info_valid = 1;
                        rx_entry->tia_type_source_prio = tlv[6]; // Figure 33-18 in IEEE801.3at/D1
                        rx_entry->tia_power            = (tlv[7] << 8) + tlv[8]; // Figure 33-18 in IEEE801.3at/D1

#ifdef VTSS_SW_OPTION_LLDP_MED
                        // Determine which LLDP-MED capabilities that is currently supported.
                        lldp_u8_t power_type = (rx_entry->tia_type_source_prio >> 6) & 0x3; // See Figure 12, TIA1057
                        if (power_type == 0x0) { // See Table 15, TIA1057
                            rx_entry->lldpmed_capabilities_current |= 0x8; // See TIA1057 MIBS LLDPXMEDREMCAPCURRENT
                        } else if (power_type == 0x1) { // See Table 15, TIA1057
                            rx_entry->lldpmed_capabilities_current |= 0x10; // See TIA1057 MIBS LLDPXMEDREMCAPCURRENT
                        }
#endif

                        org_tlv_supported = LLDP_TRUE; // OK - This is a valid PoE LLDP frame.
                        rx_entry->ieee_draft_version = 0;  //ieee_draft_version = 0 means TIA1057
                    }
#endif
                } else {

#ifdef VTSS_SW_OPTION_LLDP_MED
                    org_tlv_supported |= lldpmed_validate_lldpdu(tlv, rx_entry, len, first_lldpmed_tlv);
                    T_NG(TRACE_GRP_RX, "org_tlv_supported = 0x%X", org_tlv_supported);
                    first_lldpmed_tlv = LLDP_FALSE;
#endif // VTSS_SW_OPTION_LLDP_MED

                }

                // Check if the OUI is of IEEE 802.3 type - PoE.
            } else  if (memcmp(org_oui, &LLDP_TLV_OUI_8023[0], 3) == 0) {
#ifdef VTSS_SW_OPTION_POE
                // Subtype - See Figure 33-26 in IEEE801.3at/D3
                if (tlv[5] == 0x5) {
                    if (len == 8) { // IEEE802.3at/D1
                        T_NG(TRACE_GRP_POE, "PoE org_tlv_supported draft version 1");
                        rx_entry-> ieee_draft_version = 1;

                        rx_entry->poe_info_valid = 1;
                        rx_entry->requested_type_source_prio = tlv[6]; // Figure 33-18 in IEEE801.3at/D1
                        rx_entry->requested_power            = (tlv[7] << 8) + tlv[8]; // Figure 33-18 in IEEE801.3at/D1
                        T_DG(TRACE_GRP_POE, "Requested power = %u, %d,%d", rx_entry->requested_power, tlv[7], tlv[8]);
                        org_tlv_supported = LLDP_TRUE; // OK - This is a valid PoE LLDP frame.
                    } else if (len == 11) { //Figure 33-26 in IEEE802.3at/D3
                        T_NG(TRACE_GRP_POE, "PoE org_tlv_supported draft version 3");
                        rx_entry-> ieee_draft_version = 3;

                        rx_entry->poe_info_valid = 1;
                        rx_entry->requested_type_source_prio = tlv[6]; // Figure 33-26 in IEEE801.3at/D3
                        rx_entry->requested_power            = (tlv[7] << 8) + tlv[8]; // Figure 33-26 in IEEE801.3at/D3
                        rx_entry->actual_type_source_prio    = tlv[9]; // Figure 33-26 in IEEE801.3at/D3
                        rx_entry->actual_power               = (tlv[10] << 8) + tlv[11]; // Figure 33-26 in IEEE801.3at/D3
                        org_tlv_supported = LLDP_TRUE; // OK - This is a valid PoE LLDP frame.
                    } else {
                        // Note that EEE is using the same subtype as PoE drafts, so we don't want to give a warning.
                        T_I("Organizationally TLV - PoE but invalid length (%d bytes)", len);
                        org_tlv_supported = LLDP_FALSE;
                    }

                    // Subtype - See Figure 79-2 in IEEE802.3at/D4
                } else if (tlv[5] == 0x2) {
                    if (len == 12) { // Length must always be 12 // Figure 79-2,IEEE802.3at/D4
                        org_tlv_supported                    = LLDP_TRUE; // OK - This is a valid PoE LLDP frame.
                        rx_entry->poe_info_valid             = 1;
                        rx_entry->ieee_draft_version         = 4;   // Store that this is version 4.
                        rx_entry->poe_mdi_power_support      = tlv[6];  // Figure 79-2, IEEE802.3at/D4
                        rx_entry->poe_pse_power_pair         = tlv[7];  // Figure 79-2, IEEE802.3at/D4
                        rx_entry->poe_power_class            = tlv[8];  // Figure 79-2, IEEE802.3at/D4
                        rx_entry->requested_type_source_prio = tlv[9];  // Figure 79-2, IEEE802.3at/D4
                        rx_entry->requested_power            = (tlv[10] << 8) + tlv[11]; // Figure 79-2, IEEE802.3at/D4

                        // We reuse the name from the first drafts. The field name in draft 4 is "PSE Allocated power value"
                        rx_entry->actual_power               = (tlv[12] << 8) + tlv[13]; // Figure 79-2, IEEE802.3at/D4
                    }
                }
#endif

#ifdef VTSS_SW_OPTION_LLDP_ORG
                org_tlv_supported |= lldporg_validate_lldpdu(tlv, rx_entry, len);
                T_DG(TRACE_GRP_RX, "org_tlv_supported = 0x%X", org_tlv_supported);
#endif // VTSS_SW_OPTION_LLDP_ORG

#ifdef VTSS_SW_OPTION_EEE
                org_tlv_supported |= eee_validate_lldpdu((lldp_u8_t *)tlv, rx_entry, len);
#endif // VTSS_SW_OPTION_EEE
            }

            // Update discard counter in case that the organizationally TLV isn't supported
            if (!org_tlv_supported) {
                T_DG(TRACE_GRP_RX, "Ignoring Org. TLV no %u - type %u length %u",  (unsigned)tlv_no, (unsigned)tlv_type, (unsigned)len);
                sm->stats.statsOrgTVLsDiscarded++;
            }
            break;

        default:
            if ((9 <= (lldp_16_t) tlv_type) && ((lldp_16_t)tlv_type <= 126)) {
                sm->stats.statsTLVsUnrecognizedTotal++;
            }

            T_DG(TRACE_GRP_RX, "Ignoring TLV no %u - type %u length %u", (unsigned)tlv_no, (unsigned)tlv_type, (unsigned)len);
            break;
        }

        /* advance */
        tlv += (2 + len);

        tlv_no++;

        T_DG(TRACE_GRP_RX, "tlv_no = %d", tlv_no);
    }

    /* we didn't see any END OF LLDPDU, so we return false */
    T_DG(TRACE_GRP_RX, "Didn't see END TLV");
    bad_lldpdu(sm);
    return LLDP_FALSE;
}

void lldp_rx_process_frame(lldp_sm_t *sm)
{
    T_NG_PORT(TRACE_GRP_RX, sm->port_no, "Entering lldp_rx_process_frame");
    lldp_rx_remote_entry_t  received_entry;


    memset(&received_entry, 0, sizeof(received_entry));

    sm->stats.statsFramesInTotal++;

    received_entry.receive_port = sm->port_no;

    if (!validate_lldpdu(sm, &received_entry)) {
        T_D("Discarding bad frame");
        /* discard LLDPDU. badFrame has already been set true by validation function */
        return;
    }

    /* pass to remote entries handler */
    T_NG(TRACE_GRP_RX, "lldp_remote_handle_msap");
    if (lldp_remote_handle_msap(&received_entry)) {
        sm->rx.rxChanges = LLDP_TRUE;
    } else {
        sm->rx.rxChanges = LLDP_FALSE;
    }
}

