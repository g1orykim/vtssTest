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

#ifndef LLDP_H
#define LLDP_H

#include "lldp_sm.h"



#define LLDP_ETHTYPE 0x88CC
typedef int (*lldp_printf_t)(const char *fmt, ...) __attribute__ ((format (__printf__, 1, 2)));


void lldp_set_port_enabled (lldp_port_t port, lldp_u8_t enabled);
void lldp_something_changed_local (void);
void sw_lldp_init(void);
void lldp_1sec_timer_tick (lldp_port_t port);
void lldp_frame_received(lldp_port_t port_no, const lldp_u8_t *const frame, lldp_u16_t len, vtss_glag_no_t glag_no);
void lldp_pre_port_disabled (lldp_port_t port_no);
void lldp_set_timing_changed (void);
void lldp_rx_process_frame (lldp_sm_t *sm);



// Stat counters for one port
typedef struct {
    lldp_counter_t tx_total;
    lldp_counter_t rx_total;
    lldp_counter_t rx_error;
    lldp_counter_t rx_discarded;
    lldp_counter_t TLVs_discarded;
    lldp_counter_t TLVs_unrecognized;
    lldp_counter_t TLVs_org_discarded;
    lldp_counter_t ageouts;
} lldp_counters_rec_t;


void lldp_get_stat_counters (lldp_counters_rec_t *cnt_array) ;
void lldp_clr_stat_counters (lldp_counters_rec_t *cnt_array);

lldp_counter_t lldp_get_tx_frames (lldp_port_t port);
lldp_counter_t lldp_get_rx_total_frames (lldp_port_t port);
lldp_counter_t lldp_get_rx_error_frames (lldp_port_t port);
lldp_counter_t lldp_get_rx_discarded_frames (lldp_port_t port);
lldp_counter_t lldp_get_TLVs_discarded (lldp_port_t port);
lldp_counter_t lldp_get_TLVs_unrecognized (lldp_port_t port);
lldp_counter_t lldp_get_TLVs_org_discarded (lldp_port_t port);
lldp_counter_t lldp_get_ageouts (lldp_port_t port);
void lldp_admin_state_changed(const admin_state_t current_admin_state, const admin_state_t new_admin_state) ;
void lldp_printf_frame(lldp_8_t *frame, lldp_u16_t len);
void lldp_statistics_clear (lldp_port_t port);
#endif



