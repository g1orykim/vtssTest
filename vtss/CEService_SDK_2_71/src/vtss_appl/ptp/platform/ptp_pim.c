/*

 Vitesse Switch API software.

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

#define PTP_PIM_MESSAGE_TYPE_NAMES_DECLARE
#define PTP_PIM_COMMAND_NAMES_DECLARE

#include "main.h"
#include "conf_api.h"
#include "packet_api.h"
#include "misc_api.h"
#include "acl_api.h"            /* set up access rule */
#include "ptp_pim_api.h"
#include "ptp_pim.h"
#include "critd_api.h"
#include "vtss_ptp_os.h"
#include "port_api.h"


/******************************************************************************/
// Semaphore stuff.
/******************************************************************************/
static critd_t PTP_PIM_crit;

// Macros for accessing semaphore functions
// -----------------------------------------
#if VTSS_TRACE_ENABLED
#define PTP_PIM_CRIT_ENTER()         critd_enter(        &PTP_PIM_crit, VTSS_TRACE_GRP_DEFAULT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define PTP_PIM_CRIT_EXIT()          critd_exit(         &PTP_PIM_crit, VTSS_TRACE_GRP_DEFAULT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define PTP_PIM_CRIT_ASSERT_LOCKED() critd_assert_locked(&PTP_PIM_crit, VTSS_TRACE_GRP_DEFAULT,                       __FILE__, __LINE__)
#else
// Leave out function and line arguments
#define PTP_PIM_CRIT_ENTER()         critd_enter(        &PTP_PIM_crit)
#define PTP_PIM_CRIT_EXIT()          critd_exit(         &PTP_PIM_crit)
#define PTP_PIM_CRIT_ASSERT_LOCKED() critd_assert_locked(&PTP_PIM_crit)
#endif



/****************************************************************************/
// Protocol configuration Data
/****************************************************************************/
static ptp_pim_init_t   my_config;
static BOOL             my_pim_active = FALSE;  /* true is the PIM protocol is active */
static BOOL             my_init_done  = FALSE;  /* true is the PIM initialization has been called once */
static vtss_ace_id_t    my_id;                  /* allocated ACL id if the PIM protocol is active */

/****************************************************************************/
// Common constants
/****************************************************************************/
static const vtss_mac_t        ptp_pim_dst_mac  = {{0x01,0x01,0xc1,0x00,0x00,0x03}};
static const vtss_mac_t        ptp_pim_dst_mask = {{0xff,0xff,0xff,0xff,0xff,0xff}};

/****************************************************************************/
// Statistics
/****************************************************************************/
static ptp_pim_frame_statistics_t ptp_pim_statistics[VTSS_PORTS];

// Declare and initialize static 2d array for easy lookup of length for any
// combination of command and message type.
// Invalid combinations are marked with a 0.
// TxTimestampGet.Reply and TxTimestampSpi.Event needs special care - see below.
static const u16 PIM_message_length[PTP_PIM_COMMAND_CNT][PTP_PIM_MESSAGE_TYPE_CNT] = {
//  Req, Rep, Event
    {  0,  0,  0},  // Command 0 not used
    { 16,  0,  0},  // 1PPS
    { 14,  0,  0},  // ModemDelay
    {  6,  0,  0},  // ModemDelay pre notification
};

/******************************************************************************/
// ptp_pim_validate_header()
// Validate the header .
/******************************************************************************/
static vtss_rc ptp_pim_validate_header(const ptp_pim_header_t *header, vtss_port_no_t port)
{
    BOOL           errors = 0;
    u16            msg_len = 0;

    // Validate header
    if (header->version != 0) {
        T_WG(my_config.tg, "Invalid version: %d", header->version);
        errors++;
    }
    if (header->command >= PTP_PIM_COMMAND_CNT) {
        T_WG(my_config.tg, "Invalid command: %d", header->command);
        errors++;
    } else {
        msg_len = PIM_message_length[header->command][header->message_type];
        if ((msg_len == 0) || (msg_len != header->length)) {
            T_WG(my_config.tg, "Invalid length: %d (expected %d)", header->length, msg_len);
            errors++;
        }
    }
    if (header->message_type >= PTP_PIM_MESSAGE_TYPE_CNT) {
        T_WG(my_config.tg, "Invalid message type: %d", header->message_type);
        errors++;
    }

    if (errors) {
        ptp_pim_statistics[port].errors++;
        return VTSS_INVALID_PARAMETER;
    }

    // Update message type statistics
    switch (header->message_type) {
    case PTP_PIM_MESSAGE_TYPE_REQUEST:
        ptp_pim_statistics[port].request++;
        break;
    case PTP_PIM_MESSAGE_TYPE_REPLY:
        ptp_pim_statistics[port].reply++;
        break;
    case PTP_PIM_MESSAGE_TYPE_EVENT:
        ptp_pim_statistics[port].event++;
        break;
    }
    return VTSS_OK;
}

/******************************************************************************/
// ptp_pim_dump()
// Dump PIM message on the console.
// NOTE: Validate the message first. No check is done here.
/******************************************************************************/
static void ptp_pim_dump(const char *context, const ptp_pim_header_t *header, const u8 *body, size_t body_len)
{
    T_IG(my_config.tg, "Dump of %s PIM message header:\n"
         "Version     : %u\n"
         "MessageId   : %u\n"
         "Command     : %s (%u)\n"
         "MessageType : %s (%u)\n"
         "Length      : %u\n",
         context,
         header->version,
         header->message_id,
         PTP_PIM_command_names[header->command], header->command,
         PTP_PIM_message_type_names[header->message_type], header->message_type,
         header->length);
    if (body_len) {
        T_IG(my_config.tg, "Dump of %s PIM message body:", context);
        T_IG_HEX(my_config.tg, body, body_len);
    }
}

/******************************************************************************/
// ptp_pim_tx()
// Send an in-mand management message.
// dst_ports must contain at least one port
// The switch MAC address is inserted as src MAC
// The body (if any) is copied into the frame after the header
/******************************************************************************/
static vtss_rc ptp_pim_tx(vtss_port_no_t dst_port, const ptp_pim_header_t *header, const u8 *body, size_t body_len)
{
    u8                *frm;
    size_t            tx_len;
    packet_tx_props_t tx_props;
    vtss_mac_t        src_mac;
    vtss_etype_t      etype = PTP_PIM_DEFAULT_ETHERTYPE;

    // Allocate a buffer for the frame.
    tx_len = PTP_PIM_MAC_HEADER_LENGTH + PTP_PIM_EPID_LENGTH + PTP_PIM_HEADER_LENGTH + body_len;
    frm = packet_tx_alloc(tx_len);
    if (frm == NULL) {
        T_WG(my_config.tg, "Unable to allocate %zu bytes", tx_len);
        return VTSS_UNSPECIFIED_ERROR;
    }

    // Dump the message
    ptp_pim_dump("transmitted", header, body, body_len);

    // Initialize tx_props
    packet_tx_props_init(&tx_props);
    tx_props.packet_info.modid     = VTSS_MODULE_ID_PHY_1588_SIM;
    tx_props.packet_info.frm[0]    = frm;
    tx_props.packet_info.len[0]    = tx_len;
    tx_props.tx_info.dst_port_mask = 1LL<<dst_port;

    // Insert DMAC, DMAC and EtherType
    (void)conf_mgmt_mac_addr_get(src_mac.addr, iport2uport(dst_port)); // Insert the MAC address for the port as src
    (void)PTP_PIM_pack_mac_header(frm, &ptp_pim_dst_mac, &src_mac, etype);
    // Insert EPID
    PTP_PIM_pack16(PTP_PIM_EPID, frm + PTP_PIM_MAC_HEADER_LENGTH);

    // Insert header
    (void)PTP_PIM_pack_header(frm + PTP_PIM_MAC_HEADER_LENGTH + PTP_PIM_EPID_LENGTH, header);

    // Insert body - if any
    if (body_len) {
        memcpy(frm + PTP_PIM_MAC_HEADER_LENGTH + PTP_PIM_EPID_LENGTH + PTP_PIM_HEADER_LENGTH, body, body_len);
    }

    if (packet_tx(&tx_props) == VTSS_RC_OK) {
        return VTSS_OK;
    } else {
        T_WG(my_config.tg, "Unable to transmit %zu bytes", tx_len);
        return VTSS_UNSPECIFIED_ERROR;
    }
}

/******************************************************************************/
// ptp_pim_rx()
// Handle in-band management packets.
/******************************************************************************/
static void ptp_pim_rx(vtss_port_no_t src_port, const u8 *inbm_hdr, size_t inbm_len)
{
    ptp_pim_header_t header;
    vtss_timestamp_t ts;
    vtss_timeinterval_t delay;

    PTP_PIM_CRIT_ENTER(); // Protect statics.
    T_DG(my_config.tg, "port:%d, len:%d", iport2uport(src_port), inbm_len);

    (void)PTP_PIM_unpack_header(inbm_hdr, &header);
    // Validate the header
    if (ptp_pim_validate_header(&header, src_port) == VTSS_OK) {
        const u8 *body     = inbm_hdr + PTP_PIM_HEADER_LENGTH;
        size_t    body_len = header.length - PTP_PIM_HEADER_LENGTH;
        ptp_pim_dump("received", &header, body, body_len);
        if (header.message_type == PTP_PIM_MESSAGE_TYPE_REQUEST) {
            // Execute request (no reply are currently implemented)
            switch (header.command) {
                case PTP_PIM_COMMAND_1PPS:
                    (void)PTP_PIM_unpack_timestamp(body, &ts);
                    my_config.co_1pps(src_port, &ts);
                    break;
                case PTP_PIM_COMMAND_MODEM_DELAY:
                    delay = PTP_PIM_unpack64(body);
                    my_config.co_delay(src_port, &delay);
                    break;
                case PTP_PIM_COMMAND_MODEM_PRE_DELAY:
                    my_config.co_pre_delay(src_port);
                    break;
                default:
                    T_WG(my_config.tg, "invalid command, port %d", iport2uport(src_port));
                    break;
            }
        }
    } else {
        ptp_pim_statistics[src_port].dropped++;
    }
    PTP_PIM_CRIT_EXIT();
}


/******************************************************************************/
// Public functions
/******************************************************************************/

/******************************************************************************/
// ptp_pim_packet_rx()
/******************************************************************************/
static BOOL ptp_pim_packet_rx(void *contxt,
                                   const u8 *const frm,
                                   const vtss_packet_rx_info_t *const rx_info)
{
    size_t                          header_offset;
    vtss_mac_t                      dst_mac;
    vtss_mac_t                      src_mac;
    u16                             etype;
    u16                             epid;

    T_NG_HEX(my_config.tg, frm, rx_info->length);
    header_offset = PTP_PIM_unpack_mac_header(frm, &dst_mac, &src_mac, &etype);
    if (etype == PTP_PIM_DEFAULT_ETHERTYPE) {
        epid = PTP_PIM_unpack16(frm + header_offset);
        header_offset += 2;
        if (epid == PTP_PIM_EPID) {
            ptp_pim_rx(rx_info->port_no, frm + header_offset, rx_info->length - header_offset);
            return TRUE;
        }
    }
    return FALSE;
}

/******************************************************************************/
// ptp_pim_init()
/******************************************************************************/
void ptp_pim_init(const ptp_pim_init_t *ini, BOOL pim_active)
{
    packet_rx_filter_t rx_filter;
    void               *filter_id;
    vtss_rc            rc;
    acl_entry_conf_t   acl_conf;
    my_config = *ini;
    static const u16   etype = PTP_PIM_DEFAULT_ETHERTYPE;
#if defined(VTSS_FEATURE_ACL_V2)
    vtss_port_no_t     port_no;
    port_iter_t       pit;
#endif /* defined(VTSS_FEATURE_ACL_V2) */
    
    if (!my_init_done) {
        my_init_done = TRUE;
        critd_init(&PTP_PIM_crit, "PTP_PIM_crit", VTSS_MODULE_ID_PTP, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
        PTP_PIM_CRIT_EXIT();
        /* Ethernet in-band management frames registration */
        memset(&rx_filter, 0, sizeof(rx_filter));
        rx_filter.modid            = VTSS_MODULE_ID_PTP;
        rx_filter.match            = PACKET_RX_FILTER_MATCH_ETYPE;
        rx_filter.prio             = PACKET_RX_FILTER_PRIO_NORMAL;
        rx_filter.contxt           = NULL;
        rx_filter.cb               = ptp_pim_packet_rx;
        rx_filter.etype            = PTP_PIM_DEFAULT_ETHERTYPE;
        if ((rc = packet_rx_filter_register(&rx_filter, &filter_id)) != VTSS_OK) {
            T_EG(my_config.tg,"Unable to register for in-band management ethertype packets. (%s)", error_txt(rc));
            return;
        }
    }
    PTP_PIM_CRIT_ENTER();
    if (pim_active && !my_pim_active) {
        if ((rc = acl_mgmt_ace_init(VTSS_ACE_TYPE_ETYPE, &acl_conf)) != VTSS_OK) {
            T_EG(my_config.tg,"Unable to init ACL rule. (%s)", error_txt(rc));
        } else {
            acl_conf.action.cpu_queue = PACKET_XTR_QU_BPDU;  /* increase extraction priority to the same as bpdu packets */
            VTSS_BF_SET(acl_conf.flags.value, ACE_FLAG_IP_FRAGMENT, 0);  /* ignore IPV4 fragmented packets */
            VTSS_BF_SET(acl_conf.flags.mask, ACE_FLAG_IP_FRAGMENT, 1);
#if defined(VTSS_FEATURE_ACL_V2)
            acl_conf.action.port_action = VTSS_ACL_PORT_ACTION_FILTER;
            port_iter_init_local(&pit);
            while (port_iter_getnext(&pit)) {
                port_no = pit.iport;
                acl_conf.port_list[port_no] = TRUE;
            }
            memset(acl_conf.action.port_list, 0, sizeof(acl_conf.action.port_list));
#else
            acl_conf.action.port_no = VTSS_PORT_NO_NONE;
            acl_conf.action.permit = FALSE;
#endif /* VTSS_FEATURE_ACL_V2 */
            acl_conf.action.logging = FALSE;
            acl_conf.action.shutdown = FALSE;
            acl_conf.action.force_cpu = TRUE;
            acl_conf.action.cpu_once = FALSE;
            acl_conf.isid = VTSS_ISID_LOCAL;
            acl_conf.frame.etype.etype.value[0] = (etype>>8) & 0xff;
            acl_conf.frame.etype.etype.value[1] = etype & 0xff;
            acl_conf.frame.etype.etype.mask[0] = 0xff;
            acl_conf.frame.etype.etype.mask[1] = 0xff;
            memcpy(acl_conf.frame.etype.dmac.value, &ptp_pim_dst_mac, sizeof(ptp_pim_dst_mac));
            memcpy(acl_conf.frame.etype.dmac.mask, &ptp_pim_dst_mask, sizeof(ptp_pim_dst_mask));

            if ((rc = acl_mgmt_ace_add(ACL_USER_PTP, ACE_ID_NONE, &acl_conf)) != VTSS_OK) {
                T_EG(my_config.tg,"Unable to add ACL rule. (%s)", error_txt(rc));
            } else {
                if (acl_conf.conflict) {
                    T_WG(my_config.tg,"acl_mgmt_ace_add conflict");
                } else {
                    T_DG(my_config.tg,"ACL conf->id %d", acl_conf.id);
                    my_id = acl_conf.id;
                    my_pim_active = TRUE;
                }
            }
        }
    }    
    if (!pim_active && my_pim_active) {
        my_pim_active = FALSE;
        if ((rc = acl_mgmt_ace_del(ACL_USER_PTP, my_id)) != VTSS_OK) {
            T_EG(my_config.tg,"Unable to delete ACL rule. (%s)", error_txt(rc));
        }
        T_DG(my_config.tg,"ACL conf->id %d", my_id);
    }
    PTP_PIM_CRIT_EXIT();
}

/******************************************************************************/
// ptp_pim_1pps_msg_send()
// Transmit the time of next 1pps pulse.
/******************************************************************************/
void ptp_pim_1pps_msg_send(vtss_port_no_t port_no, const vtss_timestamp_t *ts)
{
    u8                           body[64] = {0};
    size_t                       body_len = 0;
    static u8     message_id = 0; // Static: message_id is remembered and incremented for each request.
    ptp_pim_header_t             header;

    PTP_PIM_CRIT_ENTER(); // Protect statics.
    T_DG(my_config.tg, "transmit 1pps event");

    body_len  = PTP_PIM_pack_timestamp(body, ts);

    // Create the header...
    memset(&header, 0, sizeof(header));
    header.message_id   = message_id++;
    header.command      = PTP_PIM_COMMAND_1PPS;
    header.message_type = PTP_PIM_MESSAGE_TYPE_REQUEST;
    header.length       = PTP_PIM_HEADER_LENGTH + body_len;

    // Transmit message...
    if (ptp_pim_tx(port_no, &header, body, body_len) != VTSS_OK) {
        ptp_pim_statistics[port_no].tx_dropped++; // Something went wery wrong
    }
    PTP_PIM_CRIT_EXIT();
}

/******************************************************************************/
// ptp_pim_modem_delay_msg_send()
// Send a request to a specific port.
/******************************************************************************/
void ptp_pim_modem_delay_msg_send(vtss_port_no_t port_no, const vtss_timeinterval_t *delay)
{
    u8            body[64] = {0};
    size_t        body_len = 0;
    ptp_pim_header_t header;
    static u8     message_id = 0; // Static: message_id is remembered and incremented for each request.

    PTP_PIM_CRIT_ENTER(); // Protect statics.
    T_DG(my_config.tg, "transmit delay event");

    PTP_PIM_pack64(*delay, body);
    body_len = sizeof(vtss_timeinterval_t);
    // Create the header...
    memset(&header, 0, sizeof(header));
    header.message_id   = message_id++;
    header.command      = PTP_PIM_COMMAND_MODEM_DELAY;
    header.message_type = PTP_PIM_MESSAGE_TYPE_REQUEST;
    header.length       = PTP_PIM_HEADER_LENGTH + body_len;

    // Transmit message...
    if (ptp_pim_tx(port_no, &header, body, body_len) != VTSS_OK) {
        ptp_pim_statistics[port_no].tx_dropped++; // Something went wery wrong
    }
    PTP_PIM_CRIT_EXIT();
}

/******************************************************************************/
// ptp_pim_modem_pre_delay_msg_send()
// Send a request to a specific port.
/******************************************************************************/
void ptp_pim_modem_pre_delay_msg_send(vtss_port_no_t port_no)
{
    u8            body[64] = {0};
    size_t        body_len = 0;
    ptp_pim_header_t header;
    static u8     message_id = 0; // Static: message_id is remembered and incremented for each request.

    PTP_PIM_CRIT_ENTER(); // Protect statics.
    T_DG(my_config.tg, "transmit delay event");

    body_len = 0;
    // Create the header...
    memset(&header, 0, sizeof(header));
    header.message_id   = message_id++;
    header.command      = PTP_PIM_COMMAND_MODEM_PRE_DELAY;
    header.message_type = PTP_PIM_MESSAGE_TYPE_REQUEST;
    header.length       = PTP_PIM_HEADER_LENGTH + body_len;

    // Transmit message...
    if (ptp_pim_tx(port_no, &header, body, body_len) != VTSS_OK) {
        ptp_pim_statistics[port_no].tx_dropped++; // Something went wery wrong
    }
    PTP_PIM_CRIT_EXIT();
}

/******************************************************************************/
// ptp_pim_statistics_get()
// Read the protocol statisticcs, optionally clear the statistics.
/******************************************************************************/
void ptp_pim_statistics_get(vtss_port_no_t port_no, ptp_pim_frame_statistics_t *stati, BOOL clear)
{
    PTP_PIM_CRIT_ENTER(); // Protect statics.
    *stati = ptp_pim_statistics[port_no];
    PTP_PIM_CRIT_EXIT();
}


/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
