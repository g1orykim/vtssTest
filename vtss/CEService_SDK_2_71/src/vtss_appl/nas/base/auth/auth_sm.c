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

/* Based on hostapd-0.5.9/eapol_sm.c */

#include <string.h>
#include "auth_sm.h"
#include "auth.h"

/* ************************************************************************ **
 *
 * Defines
 *
 * ************************************************************************ */

/* Definitions for easing state machine implementation */
/* --------------------------------------------------- */
#define SM_STATE(machine, state) \
  static void sm_ ## machine ## _ ## state ## _Enter(nas_sm_t *sm)

#define SM_ENTRY(machine, _state, _data)                                                  \
  if (AUTH_SM(sm)->_data.state != machine ## _ ## _state){                                \
    T_NG(TRACE_GRP_BASE, "%u: - %s", sm->port_info->port_no, (#machine " -> " #_state)); \
  }                                                                                       \
  AUTH_SM(sm)->_data.state = machine ## _ ## _state;

#define SM_ENTER(machine, state) sm_ ## machine ## _ ## state ## _Enter(sm)

#define SM_STEP(machine) \
  static void sm_ ## machine ## _Step(nas_sm_t *sm)

#define SM_STEP_RUN(machine) sm_ ## machine ## _Step(sm)

#define AUTH_BACKEND_COUNTER_INCREMENT(counter)       \
  do {                                                \
    AUTH_SM(sm)->eap_info.backend_counters.counter++; \
    sm->port_info->backend_counters->counter++;       \
  } while(0)

/* Action functions                                    */
/* --------------------------------------------------- */

#define setPortAuthorized()                                            \
  do {                                                                 \
    BOOL chgd = AUTH_SM(sm)->authStatus != NAS_PORT_STATUS_AUTHORIZED; \
    AUTH_SM(sm)->authStatus = NAS_PORT_STATUS_AUTHORIZED;              \
    nas_os_set_authorized(sm, TRUE, chgd);                             \
  } while (0);


#define setPortUnauthorized()                                            \
  do {                                                                   \
    BOOL chgd = AUTH_SM(sm)->authStatus != NAS_PORT_STATUS_UNAUTHORIZED; \
    AUTH_SM(sm)->authStatus = NAS_PORT_STATUS_UNAUTHORIZED;              \
    nas_os_set_authorized(sm, FALSE, chgd);                              \
  } while (0);

#define txCannedFail()     do{ T_DG(TRACE_GRP_BASE, "%u: action txCannedFail()",    sm->port_info->port_no); auth_tx_canned_eap(sm, 0); } while (0)
#define txCannedSuccess( ) do{ T_DG(TRACE_GRP_BASE, "%u: action txCannedSuccess()", sm->port_info->port_no); auth_tx_canned_eap(sm, 1); } while (0)
#define txReq()            auth_tx_req(sm);
#define abortAuth()        do{nas_os_backend_server_free_resources(sm);} while (0)
#define txKey()            do{ } while (0)
#define processKey()       do{ } while (0)

/* ************************************************************************ **
 *
 * Private functions
 *
 * ************************************************************************ */

/* All chapter references are to 802.1X/D11 */

/* Port Timers state machine - implemented as a single function intented to be called
   once a second */
static void check_timeouts(nas_sm_t *sm)
{
    /* eapTimeout Timer */
    if (AUTH_SM(sm)->admin.timeouts.eapTimeoutTimer) {
        if (--AUTH_SM(sm)->admin.timeouts.eapTimeoutTimer == 0) {
            T_DG(TRACE_GRP_BASE, "%u: eapTimeout occurred", sm->port_info->port_no);

            /* eap timeout occured */
            BVSET(eapTimeout);
        }
    }

    /* Client timeout, i.e. the period to wait before retransmitting an access-request
     * frame from the RADIUS server to the supplicant */
    if (AUTH_SM(sm)->admin.timeouts.clientTimeoutTimer) {
        if (--AUTH_SM(sm)->admin.timeouts.clientTimeoutTimer == 0) {
            T_DG(TRACE_GRP_BASE, "%u: No response from supplicant", sm->port_info->port_no);

            /* Only retransmit if frametype is correct */
            if (AUTH_SM(sm)->eap_info.last_frame_type != FRAME_TYPE_EAPOL) {
                return;
            }

            /* retransmit the "old" frame */
            txReq();
        }
    }
}

/* ------------------------------- */
/* Authenticator PAE state machine */
/* ------------------------------- */

SM_STATE(AUTH_PAE, INITIALIZE)
{
    SM_ENTRY(AUTH_PAE, INITIALIZE, auth_pae);
    AUTH_SM(sm)->eap_info.stop_reason = NAS_STOP_REASON_INITIALIZING;
    if (NAS_PORT_CONTROL_IS_BPDU_BASED(sm->port_info->port_control)) {
        // Only change the portMode if we're in Single-, Multi-, or Port-based 802.1X
        // ForceAuth and ForceUnauth must not be set from here, because that would cause
        // failure within the SM_STEP(AUTH_PAE) to reach the desired state.
        AUTH_SM(sm)->auth_pae.portMode = sm->port_info->port_control;
    }
}

SM_STATE(AUTH_PAE, DISCONNECTED)
{
    if (BVTST(eapolLogoff)) {
        AUTH_SM(sm)->eap_info.stop_reason = NAS_STOP_REASON_EAPOL_LOGOFF;
        if (AUTH_SM(sm)->auth_pae.state == AUTH_PAE_CONNECTING) {
            /* 8.2.4.2.2 */
            AUTH_EAPOL_COUNTER_INCREMENT(authEapLogoffsWhileConnecting);
        } else if (AUTH_SM(sm)->auth_pae.state == AUTH_PAE_AUTHENTICATED) {
            /* 8.2.4.2.11 */
            AUTH_EAPOL_COUNTER_INCREMENT(authAuthEapLogoffWhileAuthenticated);
        }
    } else if (AUTH_SM(sm)->auth_pae.reAuthCount > nas_os_get_reauth_max()) {
        AUTH_SM(sm)->eap_info.stop_reason = NAS_STOP_REASON_REAUTH_COUNT_EXCEEDED;
    }

    SM_ENTRY(AUTH_PAE, DISCONNECTED, auth_pae);

    setPortUnauthorized();

    AUTH_SM(sm)->auth_pae.reAuthCount = 0;
    BVCLR(eapolLogoff);
}

SM_STATE(AUTH_PAE, RESTART)
{
    if (AUTH_SM(sm)->auth_pae.state == AUTH_PAE_AUTHENTICATED) {
        if (BVTST(reAuthenticate)) {
            /* 8.2.4.2.9 */
            AUTH_EAPOL_COUNTER_INCREMENT(authAuthReauthsWhileAuthenticated);
        }
        if (BVTST(eapolStart)) {
            /* 8.2.4.2.10 */
            AUTH_EAPOL_COUNTER_INCREMENT(authAuthEapStartsWhileAuthenticated);
        }
        if (BVTST(eapolLogoff)) {
            /* 8.2.4.2.11 */
            AUTH_EAPOL_COUNTER_INCREMENT(authAuthEapLogoffWhileAuthenticated);
        }
    }

    SM_ENTRY(AUTH_PAE, RESTART, auth_pae);
    BVSET(eapRestart);
    auth_request_identity(sm);
}

SM_STATE(AUTH_PAE, CONNECTING)
{
    if (AUTH_SM(sm)->auth_pae.state != AUTH_PAE_CONNECTING) {
        /* 8.2.4.2.1 */
        AUTH_EAPOL_COUNTER_INCREMENT(authEntersConnecting);
    }

    SM_ENTRY(AUTH_PAE, CONNECTING, auth_pae);

    BVCLR(reAuthenticate);
    AUTH_SM(sm)->auth_pae.reAuthCount++;
}

SM_STATE(AUTH_PAE, HELD)
{
    if (AUTH_SM(sm)->auth_pae.state == AUTH_PAE_AUTHENTICATING && BVTST(authFail)) {
        AUTH_EAPOL_COUNTER_INCREMENT(authAuthFailWhileAuthenticating);
    }

    SM_ENTRY(AUTH_PAE, HELD, auth_pae);

    AUTH_SM(sm)->eap_info.stop_reason = NAS_STOP_REASON_AUTH_FAILURE;
    setPortUnauthorized();
    // In Single- and Multi 802.1X modes, the only way to get out of the HELD
    // state is to let the PSEC module remove the entry after a hold-timeout.
    // Therefore, we set the quietWhile to the highest possible value in those
    // states.
    if (NAS_PORT_CONTROL_IS_MAC_TABLE_AND_BPDU_BASED(sm->port_info->port_control)) {
        AUTH_SM(sm)->timers.quietWhile = 0xFFFFFFFF;
    } else {
        AUTH_SM(sm)->timers.quietWhile = AUTH_PAE_DEFAULT_QUIETPERIOD;
    }
    BVCLR(eapolLogoff);
}

SM_STATE(AUTH_PAE, AUTHENTICATED)
{
    if (AUTH_SM(sm)->auth_pae.state == AUTH_PAE_AUTHENTICATING && BVTST(authSuccess)) {
        /* 8.2.4.2.4 */
        AUTH_EAPOL_COUNTER_INCREMENT(authAuthSuccessesWhileAuthenticating);
    }

    SM_ENTRY(AUTH_PAE, AUTHENTICATED, auth_pae);

    setPortAuthorized();
    AUTH_SM(sm)->auth_pae.reAuthCount = 0;
}

SM_STATE(AUTH_PAE, AUTHENTICATING)
{
    if (AUTH_SM(sm)->auth_pae.state == AUTH_PAE_CONNECTING && BVTST(eapResponseIdentity)) {
        /* 8.2.4.2.3 */
        AUTH_EAPOL_COUNTER_INCREMENT(authEntersAuthenticating);
        BVCLR(eapResponseIdentity);
    }

    SM_ENTRY(AUTH_PAE, AUTHENTICATING, auth_pae);

    BVCLR(eapolStart);
    BVCLR(authSuccess);
    BVCLR(authFail);
    BVCLR(authTimeout);
    BVSET(authStart);
    BVCLR(keyRun);
    BVCLR(keyDone);
}

SM_STATE(AUTH_PAE, ABORTING)
{
    if (AUTH_SM(sm)->auth_pae.state == AUTH_PAE_AUTHENTICATING) {
        if (BVTST(authTimeout)) {
            /* 8.2.4.2.5 */
            AUTH_SM(sm)->eap_info.stop_reason = NAS_STOP_REASON_AUTH_TIMEOUT;
            AUTH_EAPOL_COUNTER_INCREMENT(authAuthTimeoutsWhileAuthenticating);
        }
        if (BVTST(eapolStart)) {
            /* 8.2.4.2.7 */
            AUTH_SM(sm)->eap_info.stop_reason = NAS_STOP_REASON_EAPOL_START;
            AUTH_EAPOL_COUNTER_INCREMENT(authAuthEapStartsWhileAuthenticating);
        }
        if (BVTST(eapolLogoff)) {
            /* 8.2.4.2.8 */
            AUTH_SM(sm)->eap_info.stop_reason = NAS_STOP_REASON_EAPOL_LOGOFF;
            AUTH_EAPOL_COUNTER_INCREMENT(authAuthEapLogoffWhileAuthenticating);
        }
    }

    SM_ENTRY(AUTH_PAE, ABORTING, auth_pae);

    BVSET(authAbort);
    BVCLR(keyRun);
    BVCLR(keyDone);
}

SM_STATE(AUTH_PAE, FORCE_AUTH)
{
    SM_ENTRY(AUTH_PAE, FORCE_AUTH, auth_pae);

    setPortAuthorized();
    AUTH_SM(sm)->auth_pae.portMode = sm->port_info->port_control;
    BVCLR(eapolStart);
    txCannedSuccess();
}

SM_STATE(AUTH_PAE, FAKE_FORCE_AUTH)
{
    // SM_ENTRY(AUTH_PAE, FAKE_FORCE_AUTH, auth_pae);
    AUTH_SM(sm)->authStatus = NAS_PORT_STATUS_AUTHORIZED;
    BVCLR(eapolStart);
}

SM_STATE(AUTH_PAE, FORCE_UNAUTH)
{
    SM_ENTRY(AUTH_PAE, FORCE_UNAUTH, auth_pae);

    AUTH_SM(sm)->eap_info.stop_reason = NAS_STOP_REASON_FORCED_UNAUTHORIZED;
    setPortUnauthorized();
    AUTH_SM(sm)->auth_pae.portMode = sm->port_info->port_control;
    BVCLR(eapolStart);
    txCannedFail();
}

SM_STEP(AUTH_PAE)
{
    /* Check for initialize, or "force"-states first */
    if (BVTST(fakeForceAuthorized)) {
        // Skip everything else when in Fake Force Authorized state.
        SM_ENTER(AUTH_PAE, FAKE_FORCE_AUTH);
    } else if ((NAS_PORT_CONTROL_IS_BPDU_BASED(sm->port_info->port_control) && AUTH_SM(sm)->auth_pae.portMode != sm->port_info->port_control) || BVTST(initialize) || BVTSTNOT(portEnabled)) {
        SM_ENTER(AUTH_PAE, INITIALIZE);

        /* this is not part of the default 802.1X state machines
         * but is used internally in this implementation to
         * make sure to cancel out any pending/old timeouts */
        auth_cancel_timeout(sm, NAS_IEEE8021X_TIMEOUT_ALL_TIMEOUTS);
    } else if (sm->port_info->port_control == NAS_PORT_CONTROL_FORCE_AUTHORIZED && AUTH_SM(sm)->auth_pae.portMode != sm->port_info->port_control && !(BVTST(initialize) || BVTSTNOT(portEnabled))) {
        SM_ENTER(AUTH_PAE, FORCE_AUTH);
    } else if (sm->port_info->port_control == NAS_PORT_CONTROL_FORCE_UNAUTHORIZED && AUTH_SM(sm)->auth_pae.portMode != sm->port_info->port_control && !(BVTST(initialize) || BVTSTNOT(portEnabled))) {
        SM_ENTER(AUTH_PAE, FORCE_UNAUTH);
    } else {
        switch (AUTH_SM(sm)->auth_pae.state) {
        case AUTH_PAE_INITIALIZE:
            SM_ENTER(AUTH_PAE, DISCONNECTED); /* UCT (Unconditional Transition) */
            break;

        case AUTH_PAE_DISCONNECTED:
            SM_ENTER(AUTH_PAE, RESTART);
            break;

        case AUTH_PAE_RESTART:
            if (BVTSTNOT(eapRestart)) {
                SM_ENTER(AUTH_PAE, CONNECTING);
            }
            break;

        case AUTH_PAE_HELD:
            if (AUTH_SM(sm)->timers.quietWhile == 0) {
                SM_ENTER(AUTH_PAE, RESTART);
            }
            break;

        case AUTH_PAE_CONNECTING:
            if (BVTST(eapolLogoff) || (AUTH_SM(sm)->auth_pae.reAuthCount > nas_os_get_reauth_max())) {
                SM_ENTER(AUTH_PAE, DISCONNECTED);
            } else if ((BVTST(eapReq) && (AUTH_SM(sm)->auth_pae.reAuthCount <= nas_os_get_reauth_max())) || BVTST(eapSuccess) || BVTST(eapFail)) {
                SM_ENTER(AUTH_PAE, AUTHENTICATING);
            }
            break;

        case AUTH_PAE_AUTHENTICATED:
            if (BVTST(eapolStart) || BVTST(reAuthenticate)) {
                SM_ENTER(AUTH_PAE, RESTART);
            } else if (BVTST(eapolLogoff) || BVTSTNOT(portValid)) {
                SM_ENTER(AUTH_PAE, DISCONNECTED);
            }
            break;

        case AUTH_PAE_AUTHENTICATING:
            if (BVTST(authSuccess) && BVTST(portValid)) {
                SM_ENTER(AUTH_PAE, AUTHENTICATED);
            } else if (BVTST(eapolStart) || BVTST(eapolLogoff) || BVTST(authTimeout)) {
                SM_ENTER(AUTH_PAE, ABORTING);
            } else if (BVTST(authFail) || (BVTST(keyDone) && BVTSTNOT(portValid))) {
                SM_ENTER(AUTH_PAE, HELD);
            }
            break;

        case AUTH_PAE_ABORTING:
            if (BVTST(eapolLogoff) && BVTSTNOT(authAbort)) {
                SM_ENTER(AUTH_PAE, DISCONNECTED);
            } else if (BVTSTNOT(eapolLogoff) && BVTSTNOT(authAbort)) {
                SM_ENTER(AUTH_PAE, RESTART);
            }
            break;

        case AUTH_PAE_FORCE_AUTH:
            if (BVTST(eapolStart)) {
                SM_ENTER(AUTH_PAE, FORCE_AUTH);
            }
            break;

        case AUTH_PAE_FORCE_UNAUTH:
            if (BVTST(eapolStart)) {
                SM_ENTER(AUTH_PAE, FORCE_UNAUTH);
            }
            break;
        }
    }
}

/* ------------------------------------ */
/* Backend Authentication State Machine */
/* ------------------------------------ */
SM_STATE(BE_AUTH, INITIALIZE)
{
    SM_ENTRY(BE_AUTH, INITIALIZE, be_auth);

    abortAuth();
    BVCLR(eapNoReq);
    BVCLR(authAbort);
}

SM_STATE(BE_AUTH, REQUEST)
{
    SM_ENTRY(BE_AUTH, REQUEST, be_auth);

    txReq();
    BVCLR(eapReq);
    AUTH_BACKEND_COUNTER_INCREMENT(backendOtherRequestsToSupplicant);
    /*
     * Clearing eapolEap here is not specified in IEEE Std 802.1X-2004, but
     * it looks like this would be logical thing to do there since the old
     * EAP response would not be valid anymore after the new EAP request
     * was sent out.
     *
     * A race condition has been reported, in which hostapd ended up
     * sending out EAP-Response/Identity as a response to the first
     * EAP-Request from the main EAP method. This can be avoided by
     * clearing eapolEap here.
     */
    BVCLR(eapolEap);
}

SM_STATE(BE_AUTH, RESPONSE)
{
    SM_ENTRY(BE_AUTH, RESPONSE, be_auth);

    BVCLR(authTimeout);
    BVCLR(eapolEap);
    BVCLR(eapNoReq);
    BVSET(eapResp);
    auth_backend_send_response(sm);
    AUTH_BACKEND_COUNTER_INCREMENT(backendResponses);
}

SM_STATE(BE_AUTH, SUCCESS)
{
    SM_ENTRY(BE_AUTH, SUCCESS, be_auth);

    txReq();
    BVSET(authSuccess);
    BVSET(keyRun);
}

SM_STATE(BE_AUTH, FAIL)
{
    SM_ENTRY(BE_AUTH, FAIL, be_auth);

    /* Note: IEEE 802.1X-REV-d11 has unconditional txReq() here.
     * txCannelFail() is used as a workaround for the case where
     * authentication server does not include EAP-Message with
     * Access-Reject. (For instance if wrong shared secret is used) */
    if (AUTH_SM(sm)->eap_info.last_frame_len == 0) {
        txCannedFail();
    } else {
        txReq();
    }

    BVSET(authFail);
}

SM_STATE(BE_AUTH, TIMEOUT)
{
    SM_ENTRY(BE_AUTH, TIMEOUT, be_auth);

    BVSET(authTimeout);
}

SM_STATE(BE_AUTH, IDLE)
{
    SM_ENTRY(BE_AUTH, IDLE, be_auth);

    BVCLR(authStart);
}

SM_STATE(BE_AUTH, IGNORE)
{
    SM_ENTRY(BE_AUTH, IGNORE, be_auth);

    BVCLR(eapNoReq);
}

SM_STEP(BE_AUTH)
{
    if (!NAS_PORT_CONTROL_IS_BPDU_BASED(sm->port_info->port_control) || BVTST(initialize) || BVTST(authAbort)) {
        SM_ENTER(BE_AUTH, INITIALIZE);
        return;
    }

    switch (AUTH_SM(sm)->be_auth.state) {
    case BE_AUTH_INITIALIZE:
        SM_ENTER(BE_AUTH, IDLE);
        break;
    case BE_AUTH_REQUEST:
        if (BVTST(eapolEap)) {
            SM_ENTER(BE_AUTH, RESPONSE);
        } else if (BVTST(eapReq)) {
            SM_ENTER(BE_AUTH, REQUEST);
        } else if (BVTST(eapTimeout)) {
            SM_ENTER(BE_AUTH, TIMEOUT);
        } else if (BVTSTNOT(portEnabled)) {
            /* Fix for "stuck" machine on disable/enable after frame transmission */
            /* with no reply (and with no timeout occuring)                       */
            /* And we need to free any backend resources we may have been using   */
            // SM_ENTER(BE_AUTH, IDLE);
            SM_ENTER(BE_AUTH, INITIALIZE);
        }
        break;
    case BE_AUTH_RESPONSE:
        if (BVTST(eapNoReq)) {
            SM_ENTER(BE_AUTH, IGNORE);
        } else if (BVTST(eapReq)) {
            AUTH_BACKEND_COUNTER_INCREMENT(backendAccessChallenges);
            SM_ENTER(BE_AUTH, REQUEST);
        } else if (AUTH_SM(sm)->timers.aWhile == 0) {
            SM_ENTER(BE_AUTH, TIMEOUT);
        } else if (BVTST(eapFail)) {
            AUTH_BACKEND_COUNTER_INCREMENT(backendAuthFails);
            SM_ENTER(BE_AUTH, FAIL);
        } else if (BVTST(eapSuccess)) {
            AUTH_BACKEND_COUNTER_INCREMENT(backendAuthSuccesses);
            SM_ENTER(BE_AUTH, SUCCESS);
        }
        break;
    case BE_AUTH_SUCCESS:
        SM_ENTER(BE_AUTH, IDLE);
        break;
    case BE_AUTH_FAIL:
        SM_ENTER(BE_AUTH, IDLE);
        break;
    case BE_AUTH_TIMEOUT:
        SM_ENTER(BE_AUTH, IDLE);
        break;
    case BE_AUTH_IDLE:
        if (BVTST(eapFail) && BVTST(authStart)) {
            SM_ENTER(BE_AUTH, FAIL);
        } else if (BVTST(eapReq) && BVTST(authStart)) {
            SM_ENTER(BE_AUTH, REQUEST);
        } else if (BVTST(eapSuccess) && BVTST(authStart)) {
            SM_ENTER(BE_AUTH, SUCCESS);
        }
        break;
    case BE_AUTH_IGNORE:
        if (BVTST(eapolEap)) {
            SM_ENTER(BE_AUTH, RESPONSE);
        } else if (BVTST(eapReq)) {
            SM_ENTER(BE_AUTH, REQUEST);
        } else if (BVTST(eapTimeout)) {
            SM_ENTER(BE_AUTH, TIMEOUT);
        }
        break;
    }
}

/* ------------------------------------ */
/* Reauthentication Timer state machine */
/* ------------------------------------ */
SM_STATE(REAUTH_TIMER, INITIALIZE)
{
    SM_ENTRY(REAUTH_TIMER, INITIALIZE, reauth_timer);

    AUTH_SM(sm)->timers.reAuthWhen = nas_os_get_reauth_timer();
}


SM_STATE(REAUTH_TIMER, REAUTHENTICATE)
{
    SM_ENTRY(REAUTH_TIMER, REAUTHENTICATE, reauth_timer);

    BVSET(reAuthenticate);
}


SM_STEP(REAUTH_TIMER)
{
    if (!NAS_PORT_CONTROL_IS_BPDU_BASED(sm->port_info->port_control) || BVTST(initialize) ||
        (AUTH_SM(sm)->authStatus == NAS_PORT_STATUS_UNAUTHORIZED) ||
        BVTSTNOT(reAuthEnabled)) {

        SM_ENTER(REAUTH_TIMER, INITIALIZE);
        return;
    }

    switch (AUTH_SM(sm)->reauth_timer.state) {
    case REAUTH_TIMER_INITIALIZE:
        if (AUTH_SM(sm)->timers.reAuthWhen == 0) {
            SM_ENTER(REAUTH_TIMER, REAUTHENTICATE);
        }
        break;
    case REAUTH_TIMER_REAUTHENTICATE:
        SM_ENTER(REAUTH_TIMER, INITIALIZE);
        break;
    }
}

/* ------------------------- */
/* Key Receive state machine */
/* ------------------------- */
SM_STATE(KEY_RX, NO_KEY_RECEIVE)
{
    SM_ENTRY(KEY_RX, NO_KEY_RECEIVE, key_rx);
}


SM_STATE(KEY_RX, KEY_RECEIVE)
{
    SM_ENTRY(KEY_RX, KEY_RECEIVE, key_rx);

    processKey();
    BVCLR(rxKey);
}

SM_STEP(KEY_RX)
{
    if (BVTST(initialize) || BVTSTNOT(portEnabled)) {
        SM_ENTER(KEY_RX, NO_KEY_RECEIVE);
        return;
    }

    switch (AUTH_SM(sm)->key_rx.state) {
    case KEY_RX_NO_KEY_RECEIVE:
        if (BVTST(rxKey)) {
            SM_ENTER(KEY_RX, KEY_RECEIVE);
        }
        break;

    case KEY_RX_KEY_RECEIVE:
        if (BVTST(rxKey)) {
            SM_ENTER(KEY_RX, KEY_RECEIVE);
        }
        break;
    }
}

/* ----------------------------------- */
/* Controlled Directions state machine */
/* ----------------------------------- */
SM_STATE(CTRL_DIR, FORCE_BOTH)
{
    SM_ENTRY(CTRL_DIR, FORCE_BOTH, ctrl_dir);
    AUTH_SM(sm)->ctrl_dir.operControlledDirections = Both;
}

SM_STATE(CTRL_DIR, IN_OR_BOTH)
{
    SM_ENTRY(CTRL_DIR, IN_OR_BOTH, ctrl_dir);
    AUTH_SM(sm)->ctrl_dir.operControlledDirections = AUTH_SM(sm)->ctrl_dir.adminControlledDirections;
}

SM_STEP(CTRL_DIR)
{
    if (BVTST(initialize)) {
        SM_ENTER(CTRL_DIR, IN_OR_BOTH);
        return;
    }

    switch (AUTH_SM(sm)->ctrl_dir.state) {
    case CTRL_DIR_FORCE_BOTH:
        if (BVTST(portEnabled) && BVTST(operEdge)) {
            SM_ENTER(CTRL_DIR, IN_OR_BOTH);
        }
        break;
    case CTRL_DIR_IN_OR_BOTH:
        if (AUTH_SM(sm)->ctrl_dir.operControlledDirections != AUTH_SM(sm)->ctrl_dir.adminControlledDirections) {
            SM_ENTER(CTRL_DIR, IN_OR_BOTH);
        } else  if (BVTSTNOT(portEnabled) || BVTSTNOT(operEdge)) {
            SM_ENTER(CTRL_DIR, FORCE_BOTH);
        }
        break;
    }
}

/* ************************************************************************ **
 *
 * Public functions
 *
 * ************************************************************************ */

/* ------------------------------------ */
/*          Step state machines         */
/* ------------------------------------ */
void auth_sm_step(nas_sm_t *sm)
{
    nas_enum_t prev_auth_pae;
    nas_enum_t prev_key_rx;
    nas_enum_t prev_reauth_timer;
    nas_enum_t prev_be_auth;
    nas_enum_t prev_ctrl_dir;

    if (AUTH_SM(sm)->eap_info.delete_me) {
        return;
    }

    do {
        /* register previous states of state machines */
        prev_auth_pae     = AUTH_SM(sm)->auth_pae.state;
        prev_key_rx       = AUTH_SM(sm)->key_rx.state;
        prev_reauth_timer = AUTH_SM(sm)->reauth_timer.state;
        prev_be_auth      = AUTH_SM(sm)->be_auth.state;
        prev_ctrl_dir     = AUTH_SM(sm)->ctrl_dir.state;

        /* run state machines */
        SM_STEP_RUN(AUTH_PAE);
        SM_STEP_RUN(BE_AUTH);
        SM_STEP_RUN(REAUTH_TIMER);
        SM_STEP_RUN(KEY_RX);
        SM_STEP_RUN(CTRL_DIR);

        /* repeat until no changes or requested to be deleted */
    } while (!AUTH_SM(sm)->eap_info.delete_me &&
             (prev_auth_pae     != AUTH_SM(sm)->auth_pae.state     ||
              prev_key_rx       != AUTH_SM(sm)->key_rx.state       ||
              prev_reauth_timer != AUTH_SM(sm)->reauth_timer.state ||
              prev_be_auth      != AUTH_SM(sm)->be_auth.state      ||
              prev_ctrl_dir     != AUTH_SM(sm)->ctrl_dir.state));
}

void auth_timers_tick(nas_sm_t *sm)
{
    if (AUTH_SM(sm)->timers.aWhile > 0) {
        AUTH_SM(sm)->timers.aWhile--;
    }

    if (AUTH_SM(sm)->timers.quietWhile > 0) {
        AUTH_SM(sm)->timers.quietWhile--;
    }

    if (AUTH_SM(sm)->timers.reAuthWhen > 0) {
        AUTH_SM(sm)->timers.reAuthWhen--;
    }

    /* Handle Timeouts */
    check_timeouts(sm);

    /* step state machines */
    auth_sm_step(sm);
}

void auth_sm_init(nas_sm_t *sm)
{
    /* Set default values for state machine constants */
    AUTH_SM(sm)->auth_pae.state = AUTH_PAE_INITIALIZE;

    AUTH_SM(sm)->be_auth.state = BE_AUTH_INITIALIZE;

    // Force a detection of state change
    // Problem if not set is that if portMode happens to be
    // NAS_PORT_CONTROL_FORCE_AUTHORIZED before allocating
    // a new SM with port mode set to NAS_PORT_CONTROL_FORCE_AUTHORIZED,
    // then the first SM_STEP(AUTH_PAE) invokation will go into the
    // DISCONNECTED state in which it will set the port to Unauthorized.
    // By setting the portMode to a value different from
    // the actual port control value, we ensure that it enters
    // the correct state in the first invokation of SM_STEP(AUTH_PAE),
    // so that in the Force Authorized case, it enters AUTH_PAE, FORCE_AUTH directly.
    AUTH_SM(sm)->auth_pae.portMode = NAS_PORT_CONTROL_DISABLED;

    AUTH_SM(sm)->reauth_timer.state = REAUTH_TIMER_INITIALIZE;
    if (nas_os_get_reauth_enabled()) {
        BVSET(reAuthEnabled);
    } else {
        BVCLR(reAuthEnabled);
    }

    AUTH_SM(sm)->key_rx.state = KEY_RX_NO_KEY_RECEIVE;

    AUTH_SM(sm)->ctrl_dir.state = CTRL_DIR_IN_OR_BOTH;

    BVSET(portEnabled);
    BVCLR(keyAvailable);
    BVCLR(keyTxEnabled);
    BVSET(portValid);

    AUTH_SM(sm)->timers.aWhile      = 0;
    AUTH_SM(sm)->timers.quietWhile  = 0;
    AUTH_SM(sm)->timers.reAuthWhen  = 0;

    T_NG(TRACE_GRP_BASE, "%u: Resetting 802.1X SM", sm->port_info->port_no);
    // Initialize the state machines by asserting initialize and then
    // deasserting it after one step. Don't step the SM after we deassert
    // the initialize flag, but wait until the next 1sec timer tick.
    // The reason is that the higher layer may not be ready for whatever
    // callbacks the deassertion may cause (e.g. because its internal
    // state doesn't yet match the lower layer's state, so calling
    // back nas_os_set_authorized() prematurely may cause confusion
    // for the upper layer).
    BVSET(initialize);
    auth_sm_step(sm);
    BVCLR(initialize);
    // auth_sm_step(sm);
}

