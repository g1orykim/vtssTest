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

 $Id$
 $Revision$

*/

#ifndef _VTSS_PTP_PACKET_CALLOUT_H_
#define _VTSS_PTP_PACKET_CALLOUT_H_

#include "vtss_ptp_types.h"

typedef struct {
    BOOL enable;        /* vlan tagging enabled */
    vtss_vid_t vid;     /* tag */
    u16 port;           /* port number */
} ptp_tag_conf_t;

typedef struct {
    vtss_etype_t tpid;
    vtss_vid_t vid;
} vtss_ptp_tag_t;    



/**
 * \file vtss_packet_callout.h
 * \brief PTP main API header file
 *
 * This file contain the definitions of API packet functions and associated
 * types for the 1588 packet interface
 * The functions are called from the ptp base implementation, and expected to be
 * implemented in the pladform specific part.
 *
 */


/**
 * Allocate a buffer and prepare for packet transmission.
 *
 * \param frame pointer to a frame buffer.
 * \param receiver pointer to a receiver structure (mac and IP address).
 * \param size length of data to transmit, excluding encapsulation.
 * \param header_size pointer to where the encapsulation header is stored.
 * \param tag_conf pointer to vlan tag configuration.
 * \return size if buffer allocated, 0 if no buffer alocated
 */
size_t
    vtss_1588_prepare_general_packet(u8 **frame, Protocol_adr_t *receiver, size_t size, size_t *header_size, ptp_tag_conf_t *tag_conf, int instance);

size_t
    vtss_1588_prepare_general_packet_2(u8 **frame, Protocol_adr_t *sender, Protocol_adr_t *receiver, size_t size, size_t *header_size, ptp_tag_conf_t *tag_conf, int instance);

void vtss_1588_release_general_packet(u8 **handle);

/**
 * Allocate a buffer for packet transmission.
 *
 * \param handle pointer to a buffer handle.
 * \param size length of data to transmit, including encapsulation.
 * \param frame pointer to where the frame pointer is stored.
 * \return size if buffer allocated, 0 if no buffer alocated
 */
size_t
    vtss_1588_packet_tx_alloc(void **handle, u8 **frame, size_t size);

/**
 * Free a buffer for packet transmission.
 *
 * \param handle pointer to a buffer handle.
 *
 */
void vtss_1588_packet_tx_free(void **handle);


/**
 * Allocate a buffer for packet transmission.
 *
 * \param buf pointer to frame buffer.
 * \param sender pointer to sender protocol adr (=0 if my local address is used).
 * \param receiver pointer to receiver protocol adr.
 * \param size of application data.
 * \param event TRUE if UDP PTP event port(319) is used, FALSE if UDP general port (320) is used
 * \param tag_conf pointer to vlan tag configuration.
 * \return size of encapsulation header
 */
size_t
    vtss_1588_pack_encap_header(uchar * buf, Protocol_adr_t *sender, Protocol_adr_t *receiver, u16 data_size, BOOL event, ptp_tag_conf_t *tag_conf, int instance);

void vtss_1588_pack_eth_header(uchar * buf, mac_addr_t sender, mac_addr_t receiver);

/**
 * Update encapsulation header, when responding to a request.
 *
 * \param buf   pointer to frame buffer.
 * \param uni   True indicates unicast operation, i.e the src and dst are swapped .
 * \param event True indicates event message type, i.e if UDP port 319 or 320 is used in IP/UDP encapsulation.
 * \param len   Length of application data
 * \return nothing
 */
void
    vtss_1588_update_encap_header(uchar *buf, BOOL uni, BOOL event, u16 len, int instance);

/**
 * Send a general PTP message.
 *
 * \param port_mask switch port mask.
 * \param frame  pointer to data to transmit, including encapsulation.
 * \param size length of data to transmit, including encapsulation.
 * \param context user defined context, user in tx_done callback.
 *
 */
size_t
    vtss_1588_tx_general(u64 port_mask,
                         u8 *frame,
                         size_t size,
                         vtss_ptp_tag_t *tag);

typedef void (*tx_done_cb_t)(void *context);
typedef void (*tx_timestamp_cb_t)(void *context, uint portnum, u32 tx_time);

typedef struct {
    tx_timestamp_cb_t cb_ts;            /* pointer to function called when tx timestamp is read from the hw */
    void *context;                      /* user defined context used as parameter to the timestamp callback */
} ptp_tx_timestamp_context_t;    

void vtss_1588_tag_get(ptp_tag_conf_t *tag_conf, int instance, vtss_ptp_tag_t *tag);

#define VTSS_PTP_MSG_TYPE_GENERAL       0
#define VTSS_PTP_MSG_TYPE_2_STEP        1
#define VTSS_PTP_MSG_TYPE_CORR_FIELD    2
#define VTSS_PTP_MSG_TYPE_ORG_TIME      3
typedef struct ptp_tx_buffer_handle_s { /* tx buffer handle for tx messages*/
    void *handle;                       /* handle for allocated tx buffer*/
    u8 *frame;                          /* pointer to allocated tx frame buffer */
    size_t size;                        /* length of data to transmit, including encapsulation. */
    u32 header_length;                  /* length of encapsulation header */
    u32 msg_type;                       /* 0 = general, 1 = 2-step, 2 = correctionfield update, 3 = org_time */
    u32   hw_time;                      /* internal HW time value used for correction field update. */
    BOOL buf_in_use;                    /* indicates if the packet is in use for transmission*/
    tx_done_cb_t cb;                    /* pointer to function called when the packet is transmitted, if this parameter
                                           is NULL then it is assumed that the handle is an rx_frame_dma buffer, and 
                                           it is released after the transmission, if the cb is != NULL it is assumed
                                           that the caller takes care of the buffer's life. I.e. this callback is only
                                           used for statically allocated buffers */
    ptp_tx_timestamp_context_t *context;/* user defined context used as parameter to the timestamp callback */
    vtss_ptp_tag_t tag;
} ptp_tx_buffer_handle_t;

void vtss_1588_tx_handle_init(ptp_tx_buffer_handle_t *ptp_buf_handle);

/**
 * Send a PTP message.
 *
 * \param port_mask switch port mask.
 * \param ptp_buf_handle ptp transmit buffer handle
 *
 */
size_t vtss_1588_tx_msg(u64 port_mask,
                          ptp_tx_buffer_handle_t *ptp_buf_handle);
/**
 * Send a Signalling PTP message, to an IP address, where the port and MAC
 * address is unknown.
 *
 * \param dest_ip  Destination IP address.
 *
 * \param buffer pointer to data to send.
 *
 * \param size length of data to transmit.
 *
 */
size_t
vtss_1588_tx_unicast_request(u32 dest_ip,
                     const void *buffer,
                     size_t size, 
                     int instance);

/**
 * \brief Set the ingress latency
 * \param portnum port number
 * \param ingress_latency ingress latency
 *
 */
void vtss_1588_ingress_latency_set(u16 portnum,
                                   vtss_timeinterval_t ingress_latency);

/**
 * \brief Set the egress latency
 * \param portnum port number
 * \param egress_latency egress latency
 *
 */
void vtss_1588_egress_latency_set(u16 portnum,
                                  vtss_timeinterval_t egress_latency);

/**
 * \brief Set the P2P path delay
 * \param portnum port number
 * \param p2p_delay P2P path delay
 *
 */
void vtss_1588_p2p_delay_set(u16 portnum,
                             vtss_timeinterval_t p2p_delay);

/**
 * \brief Set the link asymmetry
 * \param portnum port number
 * \param asymmetry link asymmetry
 *
 */
void vtss_1588_asymmetry_set(u16 portnum,
                             vtss_timeinterval_t asymmetry);

/**
 * \brief calculate difference between two tc counters.
 * \param r result = x-y
 * \param x tc counter
 * \param y tc counter
 */
void vtss_1588_ts_cnt_sub(u32 *r, u32 x, u32 y);

/**
 * \brief calculate sum of two tc counters.
 * \param r result = x+y
 * \param x tc counter
 * \param y tc counter
 */
void vtss_1588_ts_cnt_add(u32 *r, u32 x, u32 y);

void vtss_1588_timeinterval_to_ts_cnt(u32 *r, vtss_timeinterval_t x);

void vtss_1588_ts_cnt_to_timeinterval(vtss_timeinterval_t *r, u32 x);
                                  
/**
 * \brief Set/Get the clock mode for the HW clock
 *
 * \param mode FreeRun, Locking or Locked.
 *
 */
void vtss_local_clock_mode_set(vtss_ptp_clock_mode_t mode);
void vtss_local_clock_mode_get(vtss_ptp_clock_mode_t *mode);

BOOL vtss_1588_external_pdv(u32 clock_id);

#define VTSS_1588_PDV_INITIAL           0
#define VTSS_1588_PDV_FREQ_LOCKING      1
#define VTSS_1588_PDV_PHASE_LOCKING     2
#define VTSS_1588_PDV_FREQ_LOCKED       3
#define VTSS_1588_PDV_PHASE_LOCKED      4
#define VTSS_1588_PDV_PHASE_HOLDOVER    5
void vtss_1588_pdv_status_get(u32 *pdv_clock_state, i32 *freq_offset);

void vtss_1588_process_timestamp(const vtss_timestamp_t *send_time, 
                                 const vtss_timestamp_t *recv_time, 
                                 vtss_timeinterval_t correction,
                                 Integer8 logMsgIntv, BOOL fwd_path);

#endif /* _VTSS_PTP_PACKET_CALLOUT_H_ */
