/*

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


#ifndef LLDP_SM_H
#define LLDP_SM_H

/* import of lldp types */
#include "lldp_os.h"

typedef struct {
    /* Transmission counters */
    lldp_counter_t statsFramesOutTotal;

    /* Reception counters */
    lldp_counter_t statsAgeoutsTotal;
    lldp_counter_t statsFramesDiscardedTotal;
    lldp_counter_t statsFramesInErrorsTotal;
    lldp_counter_t statsFramesInTotal;
    lldp_counter_t statsTLVsDiscardedTotal;
    lldp_counter_t statsTLVsUnrecognizedTotal;

    /* Special counter for this implementation, counting unprocessed
    ** opganizationally defined TLVs */
    lldp_counter_t statsOrgTVLsDiscarded;
} lldp_statistics_t;

typedef struct {
    /* Transmit timers */
    lldp_timer_t txShutdownWhile;
    lldp_timer_t txDelayWhile;
    lldp_timer_t txTTR;
} lldp_timers_t;

typedef struct {

    enum {
        TX_INVALID_STATE,
        TX_LLDP_INITIALIZE,
        TX_IDLE,
        TX_SHUTDOWN_FRAME,
        TX_INFO_FRAME,
    } state;

    lldp_bool_t somethingChangedLocal;
    lldp_timer_t txTTL;

    lldp_bool_t re_evaluate_timers;
} lldp_tx_t;


typedef struct {

    enum {
        RX_INVALID_STATE,
        LLDP_WAIT_PORT_OPERATIONAL,
        DELETE_AGED_INFO,
        RX_LLDP_INITIALIZE,
        RX_WAIT_FOR_FRAME,
        RX_FRAME,
        DELETE_INFO,
        UPDATE_INFO,
    } state;

    lldp_bool_t badFrame;
    lldp_bool_t rcvFrame;
    lldp_bool_t rxChanges;
    lldp_bool_t rxInfoAge;
    lldp_timer_t rxTTL;
    lldp_bool_t somethingChangedRemote;
} lldp_rx_t;


typedef struct {
    lldp_tx_t tx;
    lldp_rx_t rx;
    lldp_timers_t timers;
    lldp_statistics_t stats;

    lldp_admin_state_t adminStatus;
    lldp_bool_t portEnabled;
    lldp_bool_t initialize;
    lldp_port_t port_no;
    lldp_port_t sid;
    vtss_glag_no_t glag_no;
} lldp_sm_t;



// See IEEE802.3at/D3 section 33.7.6.2
typedef enum {LOSS, NACK, ACK, DNULL} locAcknowledge_t;


// See IEEE802.3at/D3 section 33.7.6.2
typedef struct {
    enum {
        INITIALIZE,
        RUNNING,
        LOSS_OF_COMMUNICATIONS,
        REMOTE_REQUEST,
        REMOTE_ACK,
        REMOTE_NACK,
        WAIT_FOR_REMOTE,
        LOCAL_REQUEST,
        LOCAL_ACK,
        LOCAL_NACK,
    } state;
    locAcknowledge_t locAcknowledge;
    int remRequestedPowerValue;
    int remActualPowerValue;
    BOOL local_change;

} lldp_sm_pow_control_t;


void lldp_sm_step (lldp_sm_t *sm, BOOL rx_only);
void lldp_sm_init (lldp_sm_t *sm, lldp_port_t port);
void lldp_sm_timers_tick(lldp_sm_t  *sm);
#endif

