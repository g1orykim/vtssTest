/*

   Vitesse Switch API software.

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
#include "vtss_xxrp_mvrp.h"
#include "vtss_xxrp_types.h"
#include "vtss_xxrp_api.h"
#include "vtss_xxrp_os.h"
#include "vtss_xxrp_mad.h"
#include "vtss_xxrp_util.h"
#include "vtss_xxrp.h"

/* vtss_mvrp_tx is called only after LOCK has been taken. So, ignoring this lint error */
/*lint -esym(459, vtss_mvrp_tx) */
static const u8 mrp_mvrp_multicast_macaddr[] = {0x01, 0x80, 0xC2, 0x00, 0x00, 0x21};
static const u8 mrp_mvrp_eth_type[]          = {0x88, 0xF5};
#define MRP_MVRP_MULTICAST_MACADDR           mrp_mvrp_multicast_macaddr
#define VTSS_MVRP_ETH_TYPE                   mrp_mvrp_eth_type

static inline void mvrp_vid_to_mad_fsm_index(vtss_vid_t vid, u16 *mad_fsm_index)
{
    *mad_fsm_index = vid - 1;
}
void mvrp_mad_fsm_index_to_vid(u16 mad_fsm_index, u16 *vid)
{
    *vid = mad_fsm_index + 1;
}
BOOL vtss_mvrp_is_vlan_registered(u32 port_no, u16 vid)
{
    return XXRP_mvrp_is_vlan_present(port_no, vid);
}
BOOL is_mvrp_vlan_range_valid(u16 vid)
{
    return (vid >= VLAN_ID_START && vid <= VLAN_ID_END) ? TRUE : FALSE;
}
static u32 vtss_mvrp_process_vector(u32 port_no, vtss_mrp_attribute_type_t *attr_type, u16 first_value,
                                    u16 num_of_valid_vlans, u8 vector)
{
    u32 temp, rc = VTSS_XXRP_RC_OK;
    u16 vid = first_value, mad_fsm_indx;
    u8  event, divider = 36;

    /* Three-packed event parsing */
    for (temp = 0; temp < num_of_valid_vlans; temp++) {
#if 0
        if (!is_mvrp_vlan_range_valid(vid)) {
            rc = VTSS_XXRP_RC_INVALID_PARAMETER;
            break;
        }
#endif
        event = vector / divider;
        if (XXRP_mvrp_registrar_check_is_change_allowed(port_no, vid)) {
            mvrp_vid_to_mad_fsm_index(vid, &mad_fsm_indx);
            T_D("event = %u received for vid = %u\n", event, vid);
            switch (event) {
            case VTSS_XXRP_APPL_EVENT_NEW:
                T_D("New event received\n");
                (void)vtss_mrp_rx_statistics_set(VTSS_MRP_APPL_MVRP, port_no, VTSS_MRP_RX_NEW);
                rc = vtss_mrp_process_new_event(VTSS_MRP_APPL_MVRP, attr_type, port_no, mad_fsm_indx);
                break;
            case VTSS_XXRP_APPL_EVENT_JOININ:
                T_D("JoinIn event received\n");
                (void)vtss_mrp_rx_statistics_set(VTSS_MRP_APPL_MVRP, port_no, VTSS_MRP_RX_JOININ);
                rc = vtss_mrp_process_joinin_event(VTSS_MRP_APPL_MVRP, attr_type, port_no, mad_fsm_indx);
                break;
            case VTSS_XXRP_APPL_EVENT_IN:
                T_D("In event received\n");
                (void)vtss_mrp_rx_statistics_set(VTSS_MRP_APPL_MVRP, port_no, VTSS_MRP_RX_IN);
                rc = vtss_mrp_process_in_event(VTSS_MRP_APPL_MVRP, attr_type, port_no, mad_fsm_indx);
                break;
            case VTSS_XXRP_APPL_EVENT_JOINMT:
                T_D("JoinMt event received\n");
                (void)vtss_mrp_rx_statistics_set(VTSS_MRP_APPL_MVRP, port_no, VTSS_MRP_RX_JOINMT);
                rc = vtss_mrp_process_joinmt_event(VTSS_MRP_APPL_MVRP, attr_type, port_no, mad_fsm_indx);
                break;
            case VTSS_XXRP_APPL_EVENT_MT:
                T_D("Mt event received\n");
                (void)vtss_mrp_rx_statistics_set(VTSS_MRP_APPL_MVRP, port_no, VTSS_MRP_RX_MT);
                rc = vtss_mrp_process_mt_event(VTSS_MRP_APPL_MVRP, attr_type, port_no, mad_fsm_indx);
                break;
            case VTSS_XXRP_APPL_EVENT_LV:
                T_D("Lv event received\n");
                (void)vtss_mrp_rx_statistics_set(VTSS_MRP_APPL_MVRP, port_no, VTSS_MRP_RX_LV);
                rc = vtss_mrp_process_lv_event(VTSS_MRP_APPL_MVRP, attr_type, port_no, mad_fsm_indx);
                break;
            } /* switch (event) */
        } else {
            T_D("MVRP packet is not allowed on this port = %u for this vid = %u", port_no, vid);
        }
        vector -= (divider * event);
        divider = divider / 6;
        vid++;
    } /* for (temp = 0; temp < num_of_valid_vlans; temp++) */

    return rc;
}

static inline u16 vtss_mvrp_get_num_of_vals(u8 *bytes)
{
    bytes[0] &= VTSS_MVRP_NUMOFVALS_HIGHER_BYTE_MASK;
    return xxrp_ntohs(bytes);
}

static u32 vtss_mvrp_process_vector_attribute(u32 port_no, mvrp_pdu_vector_attr_t *vec_attr, u16 *size)
{
    i16                         number_of_values = 0;
    u16                         first_value, temp = 0, num_of_vec_events;
    u32                         rc = VTSS_XXRP_RC_OK;
    BOOL                        leaveall_event;
    vtss_mrp_attribute_type_t   attr_type;

    attr_type.mvrp_attr_type = VTSS_MVRP_VLAN_ATTRIBUTE;
    leaveall_event = VTSS_MVRP_EXTRACT_LA_EVENT(vec_attr->la_and_num_of_vals);
    T_D("la_and_num_of_vals[0] = 0x%x, la_and_num_of_vals[1] = 0x%x\n", vec_attr->la_and_num_of_vals[0],
        vec_attr->la_and_num_of_vals[1]);
    if (leaveall_event == TRUE) {
        T_D("LeaveAll event received from port = %u\n", port_no);
        (void)vtss_mrp_rx_statistics_set(VTSS_MRP_APPL_MVRP, port_no, VTSS_MRP_RX_LA);
        rc = vtss_mrp_process_leaveall(VTSS_MRP_APPL_MVRP, &attr_type, port_no);
    }
    number_of_values = (i16)vtss_mvrp_get_num_of_vals(vec_attr->la_and_num_of_vals);
    *size = (VTSS_MVRP_VECTOR_HDR_SIZE + ((number_of_values + 2) / 3));
    first_value = xxrp_ntohs(vec_attr->first_value);
    T_D("number_of_values = %u, first_value = %u\n", number_of_values, first_value);
    while (number_of_values > 0) {
        T_D("number_of_values = %u\n", number_of_values);
        if (number_of_values >= VTSS_MVRP_VECTOR_MAX_EVENTS) {
            num_of_vec_events = VTSS_MVRP_VECTOR_MAX_EVENTS;
        } else {
            num_of_vec_events = number_of_values;
        }
        rc += vtss_mvrp_process_vector(port_no, &attr_type, first_value, num_of_vec_events, vec_attr->vectors[temp++]);
        number_of_values -= VTSS_MVRP_VECTOR_MAX_EVENTS;
        first_value += VTSS_MVRP_VECTOR_MAX_EVENTS;
    }

    T_D("Exit Size = %u", *size);
    return rc;
}

BOOL vtss_mvrp_pdu_rx(u32 port_no, const u8 *pdu, u32 length)
{
    mvrp_pdu_fixed_flds_t       *mrpdu;
    mvrp_pdu_msg_t              *msg;
    mvrp_pdu_vector_attr_t      *vec_attr;
    BOOL                        status = FALSE;
    u32                         rc = VTSS_XXRP_RC_OK;
    u16                         msg_endmark, vec_attr_endmark, vec_attr_size, number_of_values = 0;
    u16                         number_of_vecs = 0;
    u8                          src_mac[VTSS_XXRP_MAC_ADDR_LEN];

    /* Check whether MVRP is globally enabled */
    if ((vtss_mrp_global_control_conf_get(VTSS_MRP_APPL_MVRP, &status)) != VTSS_XXRP_RC_OK) {
        T_E("MVRP global control configuration get failed\n");
        return FALSE;
    }
    if (!status) {
        T_N("MVRP is disabled globally\n");
        return FALSE;
    }
    /* Check if the MVRP is enabled on this port */
    if ((vtss_mrp_port_control_conf_get(VTSS_MRP_APPL_MVRP, port_no, &status)) != VTSS_XXRP_RC_OK) {
        T_E("MVRP port control configuration get failed\n");
        return FALSE;
    }
    if (!status) {
        T_E("MVRP is disabled on this port\n");
        return FALSE;
    }
    (void)vtss_mrp_rx_statistics_set(VTSS_MRP_APPL_MVRP, port_no, VTSS_MRP_TOTAL_RX_PKTS);
    if (length < VTSS_XXRP_MIN_PDU_SIZE) {
        T_E("Incorrect length of the mvrpdu\n");
        (void)vtss_mrp_rx_statistics_set(VTSS_MRP_APPL_MVRP, port_no, VTSS_MRP_DROPPED_PKTS);
        return FALSE;
    }
    /* TODO: Need to check NAS status etc */
    T_D("Initial checks are done length of pdu = %u\n", length);
    mrpdu = (mvrp_pdu_fixed_flds_t *) pdu;
    memcpy(src_mac, mrpdu->src_mac, VTSS_XXRP_MAC_ADDR_LEN);
    T_D("mrpdu->version = %u\n", mrpdu->version);
    if (mrpdu->version != VTSS_MVRP_VERSION) {
        T_E("MVRP version mismatch\n");
        (void)vtss_mrp_rx_statistics_set(VTSS_MRP_APPL_MVRP, port_no, VTSS_MRP_DROPPED_PKTS);
        return FALSE;
    }
    /* Move pdu pointer past the fixed fields */
    pdu += sizeof(mvrp_pdu_fixed_flds_t);
    length -= sizeof(mvrp_pdu_fixed_flds_t);
    msg = (mvrp_pdu_msg_t *)pdu;
    /* Check to see if there is a message or endmark */
    msg_endmark = xxrp_ntohs((u8 *)msg);
    T_D("msg_endmark = 0x%x\n", msg_endmark);

    /* Parse the messages till end mark is reached */
    while (msg_endmark != VTSS_MRP_ENDMARK) {

        if (length < VTSS_XXRP_MIN_MSG_SIZE) {
            T_E("Incorrect length of the mvrpdu\n");
            (void)vtss_mrp_rx_statistics_set(VTSS_MRP_APPL_MVRP, port_no, VTSS_MRP_DROPPED_PKTS);
            return FALSE;
        }

        T_D("msg->attr_type = %u, msg->attr_len = %u\n", msg->attr_type, msg->attr_len);
        if (msg->attr_type != VTSS_MVRP_ATTR_TYPE_VLAN) {
            T_E("MVRP wrong attr_type value\n");
            (void)vtss_mrp_rx_statistics_set(VTSS_MRP_APPL_MVRP, port_no, VTSS_MRP_DROPPED_PKTS);
            return FALSE;
        }

        if (msg->attr_len != VTSS_MVRP_ATTR_LEN_VLAN) {
            T_E("MVRP wrong attr_len value\n");
            (void)vtss_mrp_rx_statistics_set(VTSS_MRP_APPL_MVRP, port_no, VTSS_MRP_DROPPED_PKTS);
            return FALSE;
        }

        /* Move pdu pointer to vector attribute */
        pdu += sizeof(mvrp_pdu_msg_t);
        length -= sizeof(mvrp_pdu_msg_t);
        vec_attr_endmark = xxrp_ntohs(pdu);
        /* Process all the vector attributes until endmark is reached */
        while (vec_attr_endmark != VTSS_MRP_ENDMARK) {
            vec_attr = (mvrp_pdu_vector_attr_t *)pdu;
            T_D("length = %u", length);
            if (length < VTSS_XXRP_VEC_ATTR_HDR_SIZE) {
                T_E("Incorrect length of the mvrpdu\n");
                (void)vtss_mrp_rx_statistics_set(VTSS_MRP_APPL_MVRP, port_no, VTSS_MRP_DROPPED_PKTS);
                return FALSE;
            }
            number_of_values = (i16)vtss_mvrp_get_num_of_vals(vec_attr->la_and_num_of_vals);
            number_of_vecs = ((number_of_values + (VTSS_MVRP_VECTOR_MAX_EVENTS - 1)) / VTSS_MVRP_VECTOR_MAX_EVENTS);
            if (length < (number_of_vecs + VTSS_XXRP_VEC_ATTR_HDR_SIZE)) {
                T_E("Incorrect length of the mvrpdu\n");
                (void)vtss_mrp_rx_statistics_set(VTSS_MRP_APPL_MVRP, port_no, VTSS_MRP_DROPPED_PKTS);
                return FALSE;
            }
            length -= (number_of_vecs + VTSS_XXRP_VEC_ATTR_HDR_SIZE);
            rc = vtss_mvrp_process_vector_attribute(port_no, vec_attr, &vec_attr_size);
            /* Move pdu pointer to next vector attribute */
            pdu += vec_attr_size;
            vec_attr_endmark = xxrp_ntohs(pdu);
            T_D("vec_attr endmark = 0x%x\n", vec_attr_endmark);
        }
        pdu += sizeof(vec_attr_endmark);
        msg = (mvrp_pdu_msg_t *)pdu;
        msg_endmark = xxrp_ntohs((u8 *)msg);
        T_D("msg endmark = 0x%x\n", msg_endmark);
    } /* while (msg->attr_type != VTSS_MRP_ENDMARK) */

    rc += vtss_mrp_port_update_peer_mac_addr(VTSS_MRP_APPL_MVRP, port_no, src_mac);
    return ((rc == VTSS_XXRP_RC_OK) ? TRUE : FALSE);
}
void vtss_mvrp_join_indication(u32 port_no, vtss_mrp_attribute_type_t *attr_type, u16 joining_mad_index, BOOL new)
{
    u16 vid;

    mvrp_mad_fsm_index_to_vid(joining_mad_index, &vid);
    (void)XXRP_mvrp_vlan_port_membership_add(port_no, vid);
    if (new) {
        //Flush L2 fdb for this vid
    }
}
void vtss_mvrp_leave_indication(u32 port_no, vtss_mrp_attribute_type_t *attr_type, u32 leaving_mad_index)
{
    u16 vid;

    mvrp_mad_fsm_index_to_vid(leaving_mad_index, &vid);
    (void)XXRP_mvrp_vlan_port_membership_del(port_no, vid);
}
void vtss_mvrp_join_propagated(void *mvrp, void *my_port, u32 mad_index, BOOL new)
{
}
void vtss_mvrp_leave_propagated(void *mvrp, void *my_port, u32 mad_index)
{
}
u32 vtss_mvrp_tx(u32 port_no, u8 *all_attr_events, u32 total_events, BOOL la_flag)
{
    u8  temp;
    u8  event, first_event = 0, second_event = 0, third_event = 0;
    u8  *bufptr, *mvrp_pdu;
    u16 indx, start_vid = 1, num_of_vals = 0, vec_offset = VTSS_MVRP_VECTOR_HDR_SIZE;
    u32 num_of_events = 0;
    u16 pdu_size = VTSS_MVRP_PDU_SIZE_OF_FIXED_FLDS, tmp_len = 0;
    vtss_mrp_tx_context_t context;

    T_D("JUNK total_events = %u", total_events);
    /* TODO: fill context */
    mvrp_pdu = vtss_mrp_mrpdu_tx_alloc(port_no, VTSS_XXRP_MAX_PDU_SIZE, &context);
    if (!mvrp_pdu) {
        T_D("XXRP: No memory for tx buf allocation");
        return VTSS_XXRP_RC_NO_MEMORY;
    }
    bufptr = mvrp_pdu;
    xxrp_packet_dump(port_no, all_attr_events, TRUE);
    memset(bufptr, 0, VTSS_XXRP_MAX_PDU_SIZE);
    memcpy(bufptr, MRP_MVRP_MULTICAST_MACADDR, VTSS_XXRP_MAC_ADDR_LEN);
    bufptr += VTSS_XXRP_MAC_ADDR_LEN;
    bufptr += VTSS_XXRP_MAC_ADDR_LEN;
    memcpy(bufptr, mrp_mvrp_eth_type, VTSS_XXRP_ETH_TYPE_LEN);
    bufptr += VTSS_XXRP_ETH_TYPE_LEN;
    /* Version */
    *bufptr++ = VTSS_MVRP_VERSION;
    /* Message */
    *bufptr++ = VTSS_MVRP_ATTR_TYPE_VLAN;
    *bufptr++ = VTSS_MVRP_ATTR_LEN_VLAN;
    /* check for at least one valid event */
    if (total_events) {
        /* Check for continuous vectors */
        for (indx = 0, temp = 0; indx < XXRP_MAX_ATTRS; indx++) {
            if ((event = VTSS_XXRP_GET_EVENT(all_attr_events, indx)) == VTSS_XXRP_APPL_EVENT_INVALID) {
                if ((temp == 1) || (temp == 2)) {
                    T_N("first_event = %u, second_event = %u, third_event = %u\n", first_event, second_event, third_event);
                    bufptr[vec_offset++] = ((((first_event * 6) + second_event) * 6) + third_event);
                    tmp_len++;
                    temp = 0;
                    first_event = 0;
                    second_event = 0;
                    third_event = 0;
                } /* if ((temp == 1) || (temp == 2)) */
                /* Encode vector attribute */
                if (num_of_vals) { /* At least one event */
                    T_D("num_of_vals = %u, start vid = %u", num_of_vals, start_vid);
                    if (la_flag) {
                        la_flag = FALSE; /* Encode LA only once */
                        *bufptr++ = ((1 << VTSS_MVRP_LA_BIT_OFFSET) | ((num_of_vals >> 8) & VTSS_MVRP_NUMOFVALS_HIGHER_BYTE_MASK));
                        vtss_xxrp_update_tx_stats(VTSS_MRP_APPL_MVRP, port_no, VTSS_XXRP_APPL_EVENT_LA);
                    } else {
                        *bufptr++ = ((num_of_vals >> 8) & VTSS_MVRP_NUMOFVALS_HIGHER_BYTE_MASK);
                    } /* if (la_flag) */
                    *bufptr++ = (num_of_vals & 0xFF);
                    *bufptr++ = ((start_vid >> 8) & 0xFF);
                    *bufptr++ = (start_vid & 0xFF);
                    /* Move the pointer beyond vectors */
                    bufptr += (vec_offset - VTSS_MVRP_VECTOR_HDR_SIZE);
                    T_N("buf_ptr = %p", bufptr);
                    /* Initialize for next iteration */
                    vec_offset = VTSS_MVRP_VECTOR_HDR_SIZE;
                    num_of_vals = 0;
                    tmp_len += VTSS_MVRP_VECTOR_HDR_SIZE;
                } /* if (num_of_vals) */
                if (num_of_events == total_events) {
                    T_D("All events processed");
                    break;
                }
                start_vid = indx;
                continue;
            } /* if ((event = VTSS_XXRP_GET_EVENT(all_attr_events, indx)) == VTSS_XXRP_APPL_EVENT_INVALID) */
            vtss_xxrp_update_tx_stats(VTSS_MRP_APPL_MVRP, port_no, event);
            num_of_vals++;
            temp++;
            num_of_events++;
            if (num_of_vals == 1) {
                start_vid = indx + 1;
            } /* if (num_of_vals == 1) */
            if (temp == 1) {
                first_event = event;
            } /* if (temp == 1) */
            if (temp == 2) {
                second_event = event;
            } /* if (temp == 2) */
            if (temp == 3) {
                third_event = event;
                T_N("VEC: first_event = %u, second_event = %u, third_event = %u\n", first_event, second_event, third_event);
                bufptr[vec_offset++] = ((((first_event * 6) + second_event) * 6) + third_event);
                tmp_len++;
                temp = 0;
                first_event = 0;
                second_event = 0;
                third_event = 0;
            } /* if (temp == 3) */
        } /* for (indx = 0, temp = 0; indx < XXRP_MAX_ATTRS; indx++) */
        if (num_of_vals) { /* If reaching max attributes with valid event/s */
            if (la_flag) {
                la_flag = FALSE; /* Encode LA only once */
                *bufptr++ = ((1 << VTSS_MVRP_LA_BIT_OFFSET) | ((num_of_vals >> 8) & VTSS_MVRP_NUMOFVALS_HIGHER_BYTE_MASK));
                vtss_xxrp_update_tx_stats(VTSS_MRP_APPL_MVRP, port_no, VTSS_XXRP_APPL_EVENT_LA);
            } else {
                *bufptr++ = ((num_of_vals >> 8) & VTSS_MVRP_NUMOFVALS_HIGHER_BYTE_MASK);
            } /* if (la_flag) */
            *bufptr++ = (num_of_vals & 0xFF);
            *bufptr++ = ((start_vid >> 8) & 0xFF);
            *bufptr++ = (start_vid & 0xFF);
            tmp_len += VTSS_MVRP_VECTOR_HDR_SIZE;
        }
    } /* if (total_events) */
    pdu_size += tmp_len; /* Total frame size */
    if (la_flag) { /* This means there are no events to transmit except LA */
        mvrp_pdu[VTSS_MVRP_LA_BYTE_OFFSET - 1] |= (1 << VTSS_MVRP_LA_BIT_OFFSET);
        vtss_xxrp_update_tx_stats(VTSS_MRP_APPL_MVRP, port_no, VTSS_XXRP_APPL_EVENT_LA);
    }
    xxrp_packet_dump(port_no, mvrp_pdu, TRUE);
    if ((vtss_mrp_mrpdu_tx(port_no, mvrp_pdu, pdu_size, context)) == FALSE) {
        T_D("MVRP tx failed");
        return VTSS_XXRP_RC_UNKNOWN;
    } else {
        vtss_xxrp_update_tx_stats(VTSS_MRP_APPL_MVRP, port_no, VTSS_XXRP_APPL_EVENT_TX_PKTS);
    }
    T_D("CORRECT");
    return VTSS_XXRP_RC_OK;
}
void vtss_mvrp_added_port(void *mvrp, u32 port_no)
{
}
void vtss_mvrp_removed_port(void *mvrp, u32 port_no)
{
}
u32 vtss_mvrp_global_control_conf_create(vtss_mrp_t **mrp_app)
{
    vtss_mrp_t      *mvrp_app;
    u32             rc = VTSS_XXRP_RC_OK;

    /* Create mrp application structure and fill it. This will be freed in MRP on
       global disable of the MRP application  */
    mvrp_app = (vtss_mrp_t *) XXRP_SYS_MALLOC(sizeof(vtss_mrp_t));
    mvrp_app->appl_type = VTSS_MRP_APPL_MVRP;
    mvrp_app->mad = NULL;
    mvrp_app->map = NULL;
    mvrp_app->max_mad_index = VTSS_MVRP_VLAN_TAG_MAX_VALID - 1;
    mvrp_app->join_indication_fn = vtss_mvrp_join_indication;
    mvrp_app->leave_indication_fn = vtss_mvrp_leave_indication;
    mvrp_app->join_propagated_fn = vtss_mvrp_join_propagated;
    mvrp_app->leave_propagated_fn = vtss_mvrp_leave_propagated;
    mvrp_app->transmit_fn = vtss_mvrp_tx;
    mvrp_app->receive_fn = vtss_mvrp_pdu_rx;
    mvrp_app->added_port_fn = vtss_mvrp_added_port;
    mvrp_app->removed_port_fn = vtss_mvrp_removed_port;
    *mrp_app = mvrp_app;

    return rc;
}
void vtss_mrp_get_msti_from_mad_index(u16 mad_fsm_index, u8 *msti)
{
    /* Get the MSTI information from VLAN attribute */
    (void)mrp_mstp_index_msti_mapping_get(mad_fsm_index, msti);
}
u32 vtss_mrp_vlan_change_handler(vtss_vid_t vid, u32 port_no, BOOL is_add)
{
    u16 fsm_index;
    u32 rc = VTSS_XXRP_RC_OK;

    mvrp_vid_to_mad_fsm_index(vid, &fsm_index);
    rc = vtss_xxrp_vlan_change_handler(VTSS_MRP_APPL_MVRP, fsm_index, port_no, is_add);

    return rc;
}
