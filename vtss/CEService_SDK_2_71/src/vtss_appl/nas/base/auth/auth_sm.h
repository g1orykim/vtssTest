/*

 Copyright (c) 2002-2009 Vitesse Semiconductor Corporation "Vitesse". All
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

/* This file contains portions of code from:
 * (and is redistributed under the terms of BSD license) */
/*
 * Host AP (software wireless LAN access point) user space daemon for
 * Host AP kernel driver / IEEE 802.1X Authenticator - EAPOL state machine
 * Copyright (c) 2002-2005, Jouni Malinen <jkmaline@cc.hut.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 */

#ifndef _AUTH_SM_H_
#define _AUTH_SM_H_

#include "../common/nas_types.h"   /* For nas_timer_t           */
#include "vtss_nas_platform_api.h" /* For nas_eap_info_t, et al */

/******************************************************************************/
// Global constants
/******************************************************************************/
#define AUTH_PAE_DEFAULT_QUIETPERIOD 60

/******************************************************************************/
/******************************************************************************/
#define _vtss_dot1x_bv_skipFirstEapolTx      0
#define _vtss_dot1x_bv_authAbort             1
#define _vtss_dot1x_bv_authFail              2
#define _vtss_dot1x_bv_authStart             3
#define _vtss_dot1x_bv_authTimeout           4
#define _vtss_dot1x_bv_authSuccess           5
#define _vtss_dot1x_bv_eapFail               6
#define _vtss_dot1x_bv_eapolEap              7
#define _vtss_dot1x_bv_eapTimeout            8
#define _vtss_dot1x_bv_eapSuccess            9
#define _vtss_dot1x_bv_initialize           10
#define _vtss_dot1x_bv_keyAvailable         11
#define _vtss_dot1x_bv_keyDone              12
#define _vtss_dot1x_bv_keyRun               13
#define _vtss_dot1x_bv_keyTxEnabled         14
#define _vtss_dot1x_bv_portEnabled          15
#define _vtss_dot1x_bv_portValid            16
#define _vtss_dot1x_bv_reAuthenticate       17
#define _vtss_dot1x_bv_eapResponseIdentity  18
#define _vtss_dot1x_bv_eapolLogoff          19
#define _vtss_dot1x_bv_eapolStart           20
#define _vtss_dot1x_bv_eapRestart           21
#define _vtss_dot1x_bv_rxKey                22
#define _vtss_dot1x_bv_reAuthEnabled        23
#define _vtss_dot1x_bv_eapNoReq             24
#define _vtss_dot1x_bv_eapReq               25
#define _vtss_dot1x_bv_eapResp              26
#define _vtss_dot1x_bv_operEdge             27
#define _vtss_dot1x_bv_isReqIdentitySM      28
#define _vtss_dot1x_bv_fakeForceAuthorized  29
#define AUTH_BV_BIT_CNT            (_vtss_dot1x_bv_operEdge + 1)

/******************************************************************************/
/******************************************************************************/
#define BVSET(FIELD)        ((sm->u.auth_sm))->admin.bbyt[(_vtss_dot1x_bv_ ## FIELD) / 8] |=   1U << ((_vtss_dot1x_bv_ ## FIELD) & 0x07)
#define BVCLR(FIELD)        ((sm->u.auth_sm))->admin.bbyt[(_vtss_dot1x_bv_ ## FIELD) / 8] &= ~(1U << ((_vtss_dot1x_bv_ ## FIELD) & 0x07))
#define BVTST(FIELD)      ((((sm->u.auth_sm))->admin.bbyt[(_vtss_dot1x_bv_ ## FIELD) / 8] &   (1U << ((_vtss_dot1x_bv_ ## FIELD) & 0x07))) != 0)
#define BVTSTNOT(FIELD)   ((((sm->u.auth_sm))->admin.bbyt[(_vtss_dot1x_bv_ ## FIELD) / 8] &   (1U << ((_vtss_dot1x_bv_ ## FIELD) & 0x07))) == 0)

#define AUTH_EAPOL_COUNTER_INCREMENT(counter) \
  do {                                        \
    AUTH_SM(sm)->eapol_counters.counter++;    \
    sm->port_info->eapol_counters->counter++; \
  } while(0)

/******************************************************************************/
/******************************************************************************/
typedef struct {
    u16 eapTimeoutTimer;
    u16 clientTimeoutTimer;
} auth_sm_timeouts_t;

/******************************************************************************/
// Admin
/******************************************************************************/
typedef struct {
    auth_sm_timeouts_t timeouts;
    u8                 bbyt[(AUTH_BV_BIT_CNT + 7) / 8];
} auth_sm_admin_t;

/****************************************************************************/
/****************************************************************************/
typedef enum {
    Both = 0,
    In   = 1
} nas_controlled_directions_t;

/******************************************************************************/
// Timers
/******************************************************************************/
typedef struct {
    // The usage of this is twofold:
    // 1) If the nas_os_backend_server_tx_request() call fails, then it is set to 15 seconds.
    //    The reason for the function fail is either that no RADIUS server is configured, or
    //    the IP stack is not yet up and running.
    //
    // 2) If the call to nas_os_backend_server_tx_request() succeeds, then the
    //    RADIUS module guarantees to call us back when a) a reply is there or
    //    b) a timeout occurred. In this case, we set the aWhile to 0xFFFFFFFF,
    //    so that it will never get all the way down to a timeout. The timeout
    //    is signalled directly from the RADIUS callback.
    u32 aWhile;

    //
    u32 quietWhile;

    //
    nas_timer_t reAuthWhen;
} auth_timers_t;

// Authenticator PAE states
enum {
    AUTH_PAE_INITIALIZE,
    AUTH_PAE_DISCONNECTED,
    AUTH_PAE_RESTART,
    AUTH_PAE_CONNECTING,
    AUTH_PAE_AUTHENTICATING,
    AUTH_PAE_AUTHENTICATED,
    AUTH_PAE_ABORTING,
    AUTH_PAE_HELD,
    AUTH_PAE_FORCE_AUTH,
    AUTH_PAE_FORCE_UNAUTH
};

/******************************************************************************/
// Authenticator PAE
/******************************************************************************/
typedef struct {
    // Variables
    u8                 state; // AUTH_PAE_xxx
    nas_port_control_t portMode;
    nas_counter_t      reAuthCount;
} auth_authenticator_pae_sm_t;

// Key Receive states
enum {
    KEY_RX_NO_KEY_RECEIVE,
    KEY_RX_KEY_RECEIVE
};

/******************************************************************************/
// Key receive
/******************************************************************************/
typedef struct {
    u8 state; // KEY_RX_xxx
} auth_key_receive_sm_t;

// Reauthentication Timer states
enum {
    REAUTH_TIMER_INITIALIZE,
    REAUTH_TIMER_REAUTHENTICATE
};

/******************************************************************************/
// Reauthentication Timer
/******************************************************************************/
typedef struct {
    u8 state; // REAUTH_TIMER_xxx
} auth_reauthenticate_timer_sm_t;

// Backend Authentication states
enum {
    BE_AUTH_INITIALIZE,
    BE_AUTH_IDLE,
    BE_AUTH_REQUEST,
    BE_AUTH_RESPONSE,
    BE_AUTH_IGNORE,
    BE_AUTH_SUCCESS,
    BE_AUTH_TIMEOUT,
    BE_AUTH_FAIL
};

/******************************************************************************/
// Backend Authentication
/******************************************************************************/
typedef struct {
    u8 state; // BE_AUTH_xxx
} auth_backend_auth_sm_t;

// Controlled Directions states
enum {
    CTRL_DIR_FORCE_BOTH,
    CTRL_DIR_IN_OR_BOTH
};

/******************************************************************************/
// Controlled Directions
/******************************************************************************/
typedef struct {
    u8 state; // CTRL_DIR_xxx

    // Variables
    nas_controlled_directions_t adminControlledDirections;
    nas_controlled_directions_t operControlledDirections;
} auth_controlled_dir_sm_t;

/******************************************************************************/
// Per-port state machine entities
/******************************************************************************/
typedef struct {
    // Top-level stuff
    nas_eap_info_t    eap_info;
    auth_sm_admin_t   admin;
    auth_timers_t     timers;
    nas_port_status_t authStatus;

    /* Port Timers state machine
       is implemented as simple "callback" */

    /* Authenticator PAE state machine */
    auth_authenticator_pae_sm_t auth_pae;

    // EAPOL Counters. Backend counters are located in @eap_info.
    nas_eapol_counters_t eapol_counters;

    /* Key receive state machine */
    auth_key_receive_sm_t key_rx;

    /* Reauthentication timer state machine */
    auth_reauthenticate_timer_sm_t reauth_timer;

    /* Backend authentication state machine */
    auth_backend_auth_sm_t be_auth;

    /* Controlled Directions state machine */
    auth_controlled_dir_sm_t ctrl_dir;
} auth_sm_t;

/******************************************************************************/
// Internal API
/******************************************************************************/
void auth_timers_tick(nas_sm_t *sm);
void auth_sm_init(nas_sm_t *sm);
void auth_sm_step(nas_sm_t *sm);

#endif /* _AUTH_SM_H_ */

