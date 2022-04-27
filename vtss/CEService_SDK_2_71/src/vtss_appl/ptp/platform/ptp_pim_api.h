/*

 Vitesse Switch API software.

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


#ifndef _PTP_PIM_API_H_
#define _PTP_PIM_API_H_


typedef struct {
    u32 request;        // Number of received PIM request messages.
    u32 reply;          // Number of received PIM reply messages.
    u32 event;          // Number of received PIM event messages.
    u32 dropped;        // Number of dropped received PIM messages.
    u32 tx_dropped;     // Number of dropped transmitted PIM messages.
    u32 errors;         // Number of received PIM messages containing errors.
} ptp_pim_frame_statistics_t;




typedef struct {
    // Callout function called when a 1pps message is received.
    // Arguments:
    //   #port_no : Rx port.
    //   #ts      : Points to the received timestamp.
    void (*co_1pps)(vtss_port_no_t port_no, const vtss_timestamp_t *ts);
    // Callout function called when a delay message is received.
    // Arguments:
    //   #port_no : Rx port.
    //   #delay   : Points to the received delay value (the value is a scaled ns value).
    void (*co_delay)(vtss_port_no_t port_no, const vtss_timeinterval_t *ts);
    // Callout function called when a pre delay message is received.
    // Arguments:
    //   #port_no : Rx port.
    void (*co_pre_delay)(vtss_port_no_t port_no);
    // trace group
    u32 tg;
} ptp_pim_init_t;

/******************************************************************************/
// ptp_pim_init()
/******************************************************************************/
void ptp_pim_init(const ptp_pim_init_t *ini, BOOL pim_active);

/******************************************************************************/
// ptp_pim_1pps_msg_send()
// Transmit the time of next 1pps pulse.
/******************************************************************************/
void ptp_pim_1pps_msg_send(vtss_port_no_t port_no, const vtss_timestamp_t *ts);

/******************************************************************************/
// ptp_pim_modem_delay_msg_send()
// Send a request to a specific port.
/******************************************************************************/
void ptp_pim_modem_delay_msg_send(vtss_port_no_t port_no, const vtss_timeinterval_t *delay);

/******************************************************************************/
// ptp_pim_modem_pre_delay_msg_send()
// Send a request to a specific port.
/******************************************************************************/
void ptp_pim_modem_pre_delay_msg_send(vtss_port_no_t port_no);

/******************************************************************************/
// ptp_pim_statistics_get()
// Read the protocol statisticcs, optionally clear the statistics.
/******************************************************************************/
void ptp_pim_statistics_get(vtss_port_no_t port_no, ptp_pim_frame_statistics_t *stati, BOOL clear);





#endif /* _PTP_PIM_API_H_ */

/******************************************************************************/
//  End of file.
/******************************************************************************/
