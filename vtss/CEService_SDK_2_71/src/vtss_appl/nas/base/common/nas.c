/*

 Vitesse Switch API software.

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

#include <string.h>
#include "nas.h"

// Pretty private functions only used by this file.
// It should be the only interface we have towards the sub-SMs.
nas_funcs_t const *ieee8021x_get_funcs(void);
nas_funcs_t const *mac_based_get_funcs(void);

// Compile-time check
// Size of the supplicant_identity field must be greater than 17 when
// MAC-based authentication is enabled, because it must be able to hold
// a MAC-address in string form, i.e. "xx-xx-xx-xx-xx-xx", which is 17 + NULL.
// For normal 802.1X authentication the same applies, because if the real
// user name is wider than NAS_SUPPLICANT_ID_MAX_LENGTH, this code
// falls back to using the MAC address as user name/supplicant ID.
#if NAS_SUPPLICANT_ID_MAX_LENGTH < 18
#error "Supplicant ID Max length must be greater than 17 which is the length of a stringified MAC address"
#endif

// The individual agents called upon generic OS-level calls register their call-ins in this array.
static nas_funcs_t const *NAS_funcs[NAS_SM_TYPE_LAST];

// And this will get called if there's no agent associated with the call (if e.g. NAS is globally disabled).
// In most cases it will print an error, but in some, it allows the call to succeed.
static nas_funcs_t NAS_top_level_funcs;

// Top-level SMs. If NAS is globally disabled, you can still get a top-level SM handle, but
// it won't have any real SMs attached to it.
static nas_sm_t        NAS_top_level_state_machines[NAS_PORT_CNT];
static nas_port_info_t NAS_top_level_port_info[NAS_PORT_CNT];

// Pool of SMs that can be used by any agent. These SMs need to be attached to
// the individual top-level SMs once allocated.
static nas_sm_t        NAS_state_machines[NAS_STATE_MACHINE_CNT];
// This is a union of all agents, and each one of them will get attached to
// a SM from the NAS_state_machines[] array.
static nas_agent_sm_t  NAS_agent_state_machines[NAS_STATE_MACHINE_CNT];
// And then a counter of free SMs left
static u32             NAS_state_machines_left;
// And the list of free SMs
static nas_sm_t        *NAS_state_machines_free_pool;

// Per-port backend counters.
static nas_backend_counters_t NAS_port_backend_counters[NAS_PORT_CNT];

// Per-port EAPOL counters.
static nas_eapol_counters_t NAS_port_eapol_counters[NAS_PORT_CNT];

// When running single or multi 802.1X, we need a mechanism to send the initial
// Request Identity EAPOL frames to the supplicant. These cannot be attached
// as real SMs to the port, but have to be attached to a special per-port
// array, which is the one we define here:
#ifdef NAS_DOT1X_SINGLE_OR_MULTI
static nas_sm_t  *NAS_request_identity_state_machines[NAS_PORT_CNT];
#endif

/******************************************************************************/
//
//  PRIVATE FUNCTIONS
//
/******************************************************************************/

/******************************************************************************/
// NAS_error_func()
// Called if someone happens to do an indirect call to
// the NAS_SM_TYPE_TOP agent.
/******************************************************************************/
static void NAS_error_func(void)
{
    T_EG(TRACE_GRP_BASE, "Oops. Called with invalid SM type");
}

/******************************************************************************/
// NAS_get_nas_funcs()
/******************************************************************************/
static void NAS_get_nas_funcs(void)
{
    size_t i;
    void **p = (void **)&NAS_top_level_funcs;

    // If at any point in time NAS_funcs[NAS_SM_TYPE_TOP] is called, call an
    // error function. So fill in the nas_top_level_funcs structure with
    // pointers to the function.
    for (i = 0; i < sizeof(NAS_top_level_funcs) / sizeof(void *); i++) {
        *(p++) = NAS_error_func;
    }

    // Do not print error when calling top-level with any of the below
    // entries. Instead allow it, and let the corresponding nas_XXX()
    // return NULL
    NAS_top_level_funcs.get_eapol_counters   = NULL;
    NAS_top_level_funcs.get_backend_counters = NULL;
    NAS_top_level_funcs.reauth_param_changed = NULL;
    NAS_top_level_funcs.reinitialize         = NULL;
    NAS_top_level_funcs.reauthenticate       = NULL;
    NAS_top_level_funcs.clear_statistics     = NULL;

    NAS_funcs[NAS_SM_TYPE_TOP]       = &NAS_top_level_funcs;

    // First get the callback structures from the individual agents
    NAS_funcs[NAS_SM_TYPE_IEEE8021X] = ieee8021x_get_funcs();
#ifdef VTSS_SW_OPTION_NAS_MAC_BASED
    NAS_funcs[NAS_SM_TYPE_MAC_BASED] = mac_based_get_funcs();
#endif
}

#ifdef VTSS_SW_OPTION_DOT1X_ACCT
/******************************************************************************/
// NAS_new_session_id()
// Get a new unique session_id.
// The session id is a 32 bit number "0xRRIIIIII".
// RR is a random part that is initialized at the first call to new_session_id()
// RR is never zero (0)
// IIIIII is an incrementing part that is incremented at each call to new_session_id()
/******************************************************************************/
u32 NAS_new_session_id(void)
{
    static u32 sid_inc = 0; // The incrementing part of the session id
    static u8 sid_rnd  = 0; // The random part of the session id

    while (sid_rnd == 0) {
        sid_rnd = rand() & 0xFF;
    }
    sid_inc++;
    return (sid_rnd << 24) | (sid_inc & 0x00FFFFFF);
}
#endif

/******************************************************************************/
// NAS_get_type_from_port_mode()
/******************************************************************************/
static nas_sm_type_t NAS_get_type_from_port_mode(nas_port_control_t mode)
{
    switch (mode) {
    case NAS_PORT_CONTROL_AUTO:
    case NAS_PORT_CONTROL_FORCE_AUTHORIZED:
    case NAS_PORT_CONTROL_FORCE_UNAUTHORIZED:
        return NAS_SM_TYPE_IEEE8021X;

#ifdef VTSS_SW_OPTION_NAS_MAC_BASED
    case NAS_PORT_CONTROL_MAC_BASED:
        return NAS_SM_TYPE_MAC_BASED;
#endif

#ifdef VTSS_SW_OPTION_NAS_DOT1X_SINGLE
    case NAS_PORT_CONTROL_DOT1X_SINGLE:
        return NAS_SM_TYPE_IEEE8021X;
#endif

#ifdef VTSS_SW_OPTION_NAS_DOT1X_MULTI
    case NAS_PORT_CONTROL_DOT1X_MULTI:
        return NAS_SM_TYPE_IEEE8021X;
#endif

    default:
        T_EG(TRACE_GRP_BASE, "Invalid mode");
        return NAS_SM_TYPE_TOP;
    }
}

/******************************************************************************/
// NAS_get_type_from_port_mode_allow_top()
/******************************************************************************/
static nas_sm_type_t NAS_get_type_from_port_mode_allow_top(nas_port_control_t mode)
{
    switch (mode) {
    case NAS_PORT_CONTROL_AUTO:
    case NAS_PORT_CONTROL_FORCE_AUTHORIZED:
    case NAS_PORT_CONTROL_FORCE_UNAUTHORIZED:
        return NAS_SM_TYPE_IEEE8021X;

#ifdef VTSS_SW_OPTION_NAS_MAC_BASED
    case NAS_PORT_CONTROL_MAC_BASED:
        return NAS_SM_TYPE_MAC_BASED;
#endif

#ifdef VTSS_SW_OPTION_NAS_DOT1X_SINGLE
    case NAS_PORT_CONTROL_DOT1X_SINGLE:
        return NAS_SM_TYPE_IEEE8021X;
#endif

#ifdef VTSS_SW_OPTION_NAS_DOT1X_MULTI
    case NAS_PORT_CONTROL_DOT1X_MULTI:
        return NAS_SM_TYPE_IEEE8021X;
#endif

    case NAS_PORT_CONTROL_DISABLED:
        return NAS_SM_TYPE_TOP;

    default:
        T_EG(TRACE_GRP_BASE, "Invalid mode");
        return NAS_SM_TYPE_TOP;
    }
}

/******************************************************************************/
// NAS_do_free_sm()
// Release an SM.
/******************************************************************************/
static void NAS_do_free_sm(nas_sm_t *sm, nas_stop_reason_t stop_reason, BOOL is_request_identity_sm)
{
    nas_sm_t *temp_sm;
    nas_sm_t *prev_sm;
    nas_sm_t *top_sm;

    if (!sm) {
        T_EG(TRACE_GRP_BASE, "Cannot free NULL SM");
        return;
    }

    top_sm = sm->port_info->top_sm;

    if (!is_request_identity_sm) {
        // Obtain a prev ptr for the current SM.
        prev_sm = top_sm;
        temp_sm = prev_sm->next;
        while (temp_sm) {
            if (temp_sm == sm) {
                break;
            }
            prev_sm = temp_sm;
            temp_sm = temp_sm->next;
        }

        if (!temp_sm) {
            T_EG(TRACE_GRP_BASE, "%u: Couldn't find the state machine", sm->port_info->port_no);
            return;
        }

        // Before we call the uninit_sm(), make sure we set the reason for freeing it.
        nas_get_eap_info(sm)->stop_reason = stop_reason;

        // Give the agent a chance to uninitialize itself before we remove it from its list.
        NAS_funcs[sm->sm_type]->uninit_sm(sm);

        // Remove it from the agent's current list and place it in the free pool of SMs.
        prev_sm->next = sm->next;

        // One less client on this port.
        if (top_sm->port_info->cur_client_cnt == 0) {
            T_EG(TRACE_GRP_BASE, "%u: cur_client_cnt was 0 before freeing", top_sm->port_info->port_no);
        } else {
            top_sm->port_info->cur_client_cnt--;
        }
    }

    sm->next = NAS_state_machines_free_pool;
    NAS_state_machines_free_pool = sm;
    NAS_state_machines_left++;

    if (!is_request_identity_sm) {
        // We're not called because of a mode change, but because the user has
        // freed the SM.
        // If we're running Single- or Multi 802.1X, we cannot remove the very last
        // SM without re-instantiating a Request Identity SM.
        if (NAS_PORT_CONTROL_IS_MAC_TABLE_AND_BPDU_BASED(top_sm->port_info->port_control) && top_sm->port_info->cur_client_cnt == 0) {
            T_DG(TRACE_GRP_BASE, "%u: Auto-allocating request identity SM", top_sm->port_info->port_no);
            vtss_vid_mac_t vid_mac;
            memset(&vid_mac, 0, sizeof(vid_mac));
            (void)nas_alloc_sm(top_sm->port_info->port_no, &vid_mac);
        }
    }
}

/******************************************************************************/
//
//  PUBLIC FUNCTIONS
//
/******************************************************************************/

/******************************************************************************/
// nas_init()
/******************************************************************************/
void nas_init(void)
{
    nas_sm_type_t agent;
    nas_sm_t      *sm;
    u32           i;

    T_DG(TRACE_GRP_BASE, "Initializing");

    // Make sure the callback functions are initialized properly.
    NAS_get_nas_funcs();

    // Initialize the top-level SMs
    memset(NAS_top_level_state_machines, 0, sizeof(NAS_top_level_state_machines));
    memset(NAS_top_level_port_info,      0, sizeof(NAS_top_level_port_info));
    memset(NAS_port_backend_counters,    0, sizeof(NAS_port_backend_counters));
    memset(NAS_port_eapol_counters,      0, sizeof(NAS_port_eapol_counters));
    for (i = 0; i < NAS_PORT_CNT; i++) {
        NAS_top_level_state_machines[i].sm_type     = NAS_SM_TYPE_TOP;
        NAS_top_level_state_machines[i].port_info   = &NAS_top_level_port_info[i];
        NAS_top_level_port_info[i].top_sm           = &NAS_top_level_state_machines[i];
        NAS_top_level_port_info[i].backend_counters = &NAS_port_backend_counters[i];
        NAS_top_level_port_info[i].eapol_counters   = &NAS_port_eapol_counters[i];
        NAS_top_level_port_info[i].port_no          = i + 1;
        // Allow for some OS-specific initializations of the port_info
        nas_os_init(&NAS_top_level_port_info[i]);
    }

#ifdef NAS_DOT1X_SINGLE_OR_MULTI
    memset(NAS_request_identity_state_machines, 0, sizeof(NAS_request_identity_state_machines));
#endif

    // Also initialize the normal state machines.
    memset(NAS_state_machines,       0, sizeof(NAS_state_machines));
    memset(NAS_agent_state_machines, 0, sizeof(NAS_agent_state_machines));

    // Create a list of free state machines
    NAS_state_machines_left = 0;
    for (i = 0; i < sizeof(NAS_state_machines) / sizeof(NAS_state_machines[0]); i++) {
        sm            = &NAS_state_machines[i];
        sm->u.real_sm = &NAS_agent_state_machines[NAS_state_machines_left++];
        sm->next      = &NAS_state_machines[i + 1];
    }
    // Terminate
    NAS_state_machines[(sizeof(NAS_state_machines) / sizeof(NAS_state_machines[0])) - 1].next = NULL;
    NAS_state_machines_free_pool = &NAS_state_machines[0];

    for (agent = NAS_SM_TYPE_FIRST; agent < NAS_SM_TYPE_LAST; agent++) {
        NAS_funcs[agent]->init();
    }
}

/******************************************************************************/
// nas_get_top_sm()
/******************************************************************************/
nas_sm_t *nas_get_top_sm(nas_port_t port_no)
{
    if (port_no > NAS_PORT_CNT) {
        T_EG(TRACE_GRP_BASE, "%u: Invalid port number", port_no);
        return NULL;
    }
    return &NAS_top_level_state_machines[port_no - 1];
}

/******************************************************************************/
// nas_backend_server_timeout_occurred()
// Called whenever a backend server (e.g. RADIUS) timeout occurs.
/******************************************************************************/
void nas_backend_server_timeout_occurred(nas_sm_t *sm)
{
    if (NAS_funcs[sm->sm_type]->backend_server_timeout_occurred) {
        NAS_funcs[sm->sm_type]->backend_server_timeout_occurred(sm);
    }
}

/******************************************************************************/
// nas_backend_frame_received()
// Called whenever we've received a frame from the backend server (e.g. RADIUS).
// It is guaranteed that the SM's eap_info member is updated with the relevant
// attributes.
/******************************************************************************/
void nas_backend_frame_received(nas_sm_t *sm, nas_backend_code_t code)
{
    if (!sm) {
        T_EG(TRACE_GRP_BASE, "SM is NULL");
        return;
    }

    if (NAS_funcs[sm->sm_type]->backend_frame_received) {
        NAS_funcs[sm->sm_type]->backend_frame_received(sm, code);
    } else {
        T_EG(TRACE_GRP_BASE, "%u: Invalid Type (%d)", sm->port_info->port_no, sm->sm_type);
    }
}

/******************************************************************************/
// nas_1sec_timer_tick()
/******************************************************************************/
void nas_1sec_timer_tick(void)
{
    nas_port_t    port;
    nas_sm_t      *top_sm, *sm;
    nas_sm_type_t agent;

    for (port = 1; port <= NAS_PORT_CNT; port++) {
        top_sm = nas_get_top_sm(port);
        if (top_sm->port_info->port_control == NAS_PORT_CONTROL_DISABLED) {
            continue;
        }
        agent = NAS_get_type_from_port_mode(top_sm->port_info->port_control);

#ifdef NAS_DOT1X_SINGLE_OR_MULTI
        sm = NAS_request_identity_state_machines[port - 1];
        if (sm) {
            // The port has a request identity state machine attached. Use that one.
            (void)NAS_funcs[agent]->timer_tick(sm); // Can't use the return value for anything

            // It cannot have normal state machines attached in that case.
            if (top_sm->next) {
                T_EG(TRACE_GRP_BASE, "%u: Both request identity and normal SMs attached", port);
            }
        } else
#endif
        {
            sm = top_sm->next;
            while (sm) {
                nas_sm_t *sm_next = sm->next;
                nas_stop_reason_t stop_reason;
                if ((stop_reason = NAS_funcs[agent]->timer_tick(sm)) != NAS_STOP_REASON_NONE) {
                    NAS_do_free_sm(sm, stop_reason, FALSE);
                }
                sm = sm_next;
            }
        }
    }
}

/******************************************************************************/
// nas_alloc_sm()
// Allocate and initialize a new agent SM and return it.
/******************************************************************************/
nas_sm_t *nas_alloc_sm(nas_port_t port_no, vtss_vid_mac_t *vid_mac)
{
    nas_sm_t        *top_sm = nas_get_top_sm(port_no);
    nas_sm_t        *sm = NULL, *temp_sm;
    nas_port_info_t *port_info;
    BOOL            new_is_a_req_identity_sm      = FALSE;
#ifdef NAS_DOT1X_SINGLE_OR_MULTI
    BOOL            old_was_a_request_identity_sm = FALSE;
#endif

    if (!top_sm) {
        return NULL; // Error already printed
    }

    port_info = top_sm->port_info;

    if (port_info->port_control == NAS_PORT_CONTROL_DISABLED) {
        T_EG(TRACE_GRP_BASE, "%u: Cannot allocate SMs for a disabled port", port_no);
        return NULL;
    }

    if (NAS_PORT_CONTROL_IS_MULTI_CLIENT(port_info->port_control) == FALSE && port_info->cur_client_cnt >= 1) {
        // The number of SMs on a non-multi-client port can at most be 1.
        T_EG(TRACE_GRP_BASE, "%u: Single-client modes allow at most one SM per port (mode=%d)", port_no, port_info->port_control);
        return NULL;
    }

#ifdef NAS_DOT1X_SINGLE_OR_MULTI
    if (NAS_PORT_CONTROL_IS_MAC_TABLE_AND_BPDU_BASED(port_info->port_control)) {
        // We're running Single or Multi 802.1X.
        sm = NAS_request_identity_state_machines[port_no - 1];
        NAS_request_identity_state_machines[port_no - 1] = NULL;
        // Now @sm may be either the current request identity SM or NULL.
        old_was_a_request_identity_sm = sm != NULL;
    }
#endif

    if (!sm) {
        // No SMs were attached to the request identity SM port array.
        // Allocate and initialize a new one.
        sm = NAS_state_machines_free_pool;
        if (sm) {
            NAS_state_machines_free_pool = sm->next;
            NAS_state_machines_left--;
            sm->next = NULL;
        } else {
            T_EG(TRACE_GRP_BASE, "%u: Out of state machines", port_no);
            return NULL;
        }
    }

    memset(sm->u.real_sm,    0, sizeof(nas_agent_sm_t));
    memset(&sm->client_info, 0, sizeof(sm->client_info));

    // Provide quick access to the port_info.
    sm->port_info = port_info;

    // Get the SM's type
    sm->sm_type = NAS_get_type_from_port_mode(top_sm->port_info->port_control);

    // Update both this SM's creation time and the top SM's last-client-info's creation time.
    sm->client_info.rel_creation_time_secs = sm->port_info->top_sm->client_info.rel_creation_time_secs = nas_os_get_uptime_secs();

    // Insert the SM in the correct array.
    // If we're in single or multi 802.1X mode and a request identity SM is not currently
    // installed, and the number of clients is 0, then we need to insert it into the
    // requesty identity array.
    // In all other cases, we insert it into the normal SM array.
#ifdef NAS_DOT1X_SINGLE_OR_MULTI
    if (NAS_PORT_CONTROL_IS_MAC_TABLE_AND_BPDU_BASED(port_info->port_control) && !old_was_a_request_identity_sm && port_info->cur_client_cnt == 0) {
        NAS_request_identity_state_machines[port_no - 1] = sm;
        new_is_a_req_identity_sm = TRUE;
    } else
#endif
    {
        // One more SM allocated on that port.
        port_info->cur_client_cnt++;

        // Insert the state machine at the end of the list of state_machines for that port.
        temp_sm = top_sm;
        while (temp_sm->next) {
            temp_sm = temp_sm->next;
        }
        temp_sm->next = sm;
    }

    // Let the agent initialize the rest.
    // The third arg indicates whether we've just allocated a "Request Identity" SM.
    NAS_funcs[sm->sm_type]->init_sm(sm, vid_mac, new_is_a_req_identity_sm);

    return sm;
}

/******************************************************************************/
// nas_free_sm()
// Release an SM.
/******************************************************************************/
void nas_free_sm(nas_sm_t *sm, nas_stop_reason_t stop_reason)
{
    NAS_do_free_sm(sm, stop_reason, FALSE);
}

/******************************************************************************/
// nas_free_all_sms()
// Release all SMs on a port.
/******************************************************************************/
void nas_free_all_sms(nas_port_t port_no, nas_stop_reason_t stop_reason)
{
    nas_sm_t *top_sm = nas_get_top_sm(port_no), *sm;

    if (!top_sm) {
        return; // Error already printed
    }

    sm = top_sm->next;
    while (sm) {
        // Get a pointer to the next before freeing
        nas_sm_t *sm_next = sm->next;
        // Free it
        NAS_do_free_sm(sm, stop_reason, FALSE);
        // Go on with the next
        sm = sm_next;
    }
}

/******************************************************************************/
// nas_get_client_cnt()
/******************************************************************************/
u32 nas_get_client_cnt(nas_port_t port_no)
{
    nas_sm_t *top_sm = nas_get_top_sm(port_no);

    if (top_sm) {
        return top_sm->port_info->cur_client_cnt;
    }

    return 0;
}

/******************************************************************************/
// nas_get_port_control()
/******************************************************************************/
nas_port_control_t nas_get_port_control(nas_port_t port_no)
{
    nas_sm_t *top_sm = nas_get_top_sm(port_no);

    if (top_sm) {
        return top_sm->port_info->port_control;
    }

    return NAS_PORT_CONTROL_DISABLED;
}

/******************************************************************************/
// nas_set_port_control()
/******************************************************************************/
void nas_set_port_control(nas_port_t port_no, nas_port_control_t new_mode)
{
    nas_port_control_t old_mode;
    nas_sm_t           *top_sm = nas_get_top_sm(port_no);
#ifdef NAS_DOT1X_SINGLE_OR_MULTI
    nas_sm_t           *sm;
#endif

    if (!top_sm) {
        return; // Error already printed
    }

    old_mode = top_sm->port_info->port_control;

    // Nothing to do if the mode really doesn't change.
    if (new_mode == old_mode) {
        return;
    }

    if (top_sm->port_info->cur_client_cnt != 0) {
        // All SMs must be freed by now.
        T_EG(TRACE_GRP_BASE, "%u: State machines not freed", port_no);
        nas_free_all_sms(port_no, NAS_STOP_REASON_PORT_MODE_CHANGED);
    }

#ifdef NAS_DOT1X_SINGLE_OR_MULTI
    if ((sm = NAS_request_identity_state_machines[port_no - 1]) != NULL) {
        // Gotta free the request identity SM as well.
        NAS_do_free_sm(sm, NAS_STOP_REASON_PORT_MODE_CHANGED, TRUE);
        NAS_request_identity_state_machines[port_no - 1] = NULL;
    }
#endif

    // Set the new port_control up front.
    top_sm->port_info->port_control = new_mode;

    if (top_sm->next) {
        T_EG(TRACE_GRP_BASE, "%u: next-ptr is non-NULL. Leaking SMs", port_no);
    }

    // Clear the last supplicant/client info
    memset(&top_sm->client_info, 0, sizeof(top_sm->client_info));

    // Clear port counters
    memset(top_sm->port_info->backend_counters, 0, sizeof(*top_sm->port_info->backend_counters));
    memset(top_sm->port_info->eapol_counters,   0, sizeof(*top_sm->port_info->eapol_counters));

    if (new_mode != NAS_PORT_CONTROL_DISABLED) {
        if (NAS_PORT_CONTROL_IS_MAC_TABLE_AND_BPDU_BASED(new_mode) || !NAS_PORT_CONTROL_IS_MULTI_CLIENT(new_mode)) {
            // In Single- or Multi 802.1X, we need a Request Identity SM,
            // and in Port-based modes, we need a normal SM.
            // Currently, the only reason for *not* getting here is in MAC-based mode.
            // The nas_alloc_sm() takes care of storing the SM in the correct position.
            vtss_vid_mac_t vid_mac;
            memset(&vid_mac, 0, sizeof(vid_mac));
            T_DG(TRACE_GRP_BASE, "%u: Auto-allocating SM", port_no);
            // If we're in Single- or Multi 802.1X, we need a Request Identity SM.
            (void)nas_alloc_sm(port_no, &vid_mac); // Errors already printed, so nothing more to do with the return value.
        }
    }
}

/******************************************************************************/
// nas_set_fake_force_authorized()
/******************************************************************************/
#ifdef VTSS_SW_OPTION_NAS_GUEST_VLAN
void nas_set_fake_force_authorized(nas_sm_t *top_sm, BOOL enable)
{
    nas_sm_t   *sub_sm;
    nas_port_t port_no;

    if (top_sm == NULL || top_sm->sm_type != NAS_SM_TYPE_TOP) {
        T_EG(TRACE_GRP_BASE, "Invalid top_sm");
        return;
    }

    port_no = top_sm->port_info->port_no;

    // Various checks.
    if (NAS_PORT_CONTROL_IS_MAC_TABLE_AND_BPDU_BASED(top_sm->port_info->port_control)) {
        // Single- or Multi- 802.1X
        if (top_sm->port_info->cur_client_cnt != 0) {
            T_EG(TRACE_GRP_BASE, "%u: In Single- and Multi- 802.1X mode, the number of attached clients must be 0", port_no);
        }
        sub_sm = NAS_request_identity_state_machines[port_no - 1];
    } else if (top_sm->port_info->port_control == NAS_PORT_CONTROL_AUTO) {
        if (top_sm->port_info->cur_client_cnt != 1) {
            T_EG(TRACE_GRP_BASE, "%u: In Port-based 802.1X mode, the number of attached clients must be 1", port_no);
        }
        sub_sm = top_sm->next;
    } else {
        T_EG(TRACE_GRP_BASE, "%u: Invalid port state (%d) for this call", port_no, top_sm->port_info->port_control);
        return;
    }

    if (!sub_sm) {
        T_EG(TRACE_GRP_BASE, "%u: Invalid sub_sm", port_no);
        return;
    }

    if (NAS_funcs[sub_sm->sm_type]->fake_force_authorized == NULL) {
        T_EG(TRACE_GRP_BASE, "%u: fake_force_authorized() not implemented for Port Control = %d", port_no, top_sm->port_info->port_control);
        return;
    }
    NAS_funcs[sub_sm->sm_type]->fake_force_authorized(sub_sm, enable);
}
#endif

#ifdef VTSS_SW_OPTION_NAS_GUEST_VLAN
/******************************************************************************/
// nas_reset_reauth_cnt()
/******************************************************************************/
void nas_reset_reauth_cnt(nas_sm_t *top_sm)
{
    nas_sm_t   *sub_sm;

    if (top_sm == NULL || top_sm->sm_type != NAS_SM_TYPE_TOP) {
        T_EG(TRACE_GRP_BASE, "Invalid top_sm");
        return;
    }

    for (sub_sm = top_sm->next; sub_sm; sub_sm = sub_sm->next) {
        if (NAS_funcs[sub_sm->sm_type]->reset_reauth_cnt != NULL) {
            NAS_funcs[sub_sm->sm_type]->reset_reauth_cnt(sub_sm);
        }
    }
}
#endif

#ifdef NAS_DOT1X_SINGLE_OR_MULTI
/******************************************************************************/
// nas_get_sm_from_mac()
// Returns a state machine based on the MAC address, only (not the VLAN ID).
/******************************************************************************/
nas_sm_t *nas_get_sm_from_mac(vtss_vid_mac_t *vid_mac)
{
    nas_sm_t      *top_sm, *sm;
    nas_port_t    port_no;

    if (!vid_mac) {
        T_EG(TRACE_GRP_BASE, "Invalid parameter");
        return NULL;
    }

    for (port_no = 1; port_no <= NAS_PORT_CNT; port_no++) {
        top_sm = nas_get_top_sm(port_no);
        for (sm = top_sm->next; sm; sm = sm->next) {
            if (memcmp(vid_mac->mac.addr, sm->client_info.vid_mac.mac.addr, sizeof(vid_mac->mac.addr)) == 0) {
                return sm;
            }
        }
    }
    return NULL;
}
#endif

/******************************************************************************/
// nas_get_sm_from_vid_mac_port()
// Returns a state machine based on the MAC address, VLAN ID, and port number.
/******************************************************************************/
nas_sm_t *nas_get_sm_from_vid_mac_port(vtss_vid_mac_t *vid_mac, nas_port_t port_no)
{
    nas_sm_t *top_sm = nas_get_top_sm(port_no), *sm;

    if (!vid_mac) {
        T_EG(TRACE_GRP_BASE, "Invalid parameter");
        return NULL;
    }

    if (!top_sm) {
        return NULL;
    }

    for (sm = top_sm->next; sm; sm = sm->next) {
        if (memcmp(vid_mac, &sm->client_info.vid_mac, sizeof(*vid_mac)) == 0) {
            return sm;
        }
    }

    return NULL;
}

/******************************************************************************/
// nas_reauth_param_changed()
/******************************************************************************/
void nas_reauth_param_changed(BOOL ena, u16 reauth_period)
{
    nas_port_t    port_no;
    nas_sm_t      *top_sm;
    nas_sm_type_t agent;

    for (port_no = 1; port_no <= NAS_PORT_CNT; port_no++) {
        top_sm = nas_get_top_sm(port_no);
        agent  = NAS_get_type_from_port_mode_allow_top(top_sm->port_info->port_control);
        if (NAS_funcs[agent]->reauth_param_changed) {
            NAS_funcs[agent]->reauth_param_changed(top_sm, ena, reauth_period);
        }
    }
}

/******************************************************************************/
// nas_get_port_status()
/******************************************************************************/
nas_port_status_t nas_get_port_status(nas_port_t port_no)
{
    nas_sm_t      *top_sm = nas_get_top_sm(port_no);
    nas_sm_type_t agent;

    if (!top_sm) {
        return  NAS_PORT_STATUS_UNAUTHORIZED; // Error already printed.
    }

    agent = NAS_get_type_from_port_mode(top_sm->port_info->port_control);

#ifdef NAS_DOT1X_SINGLE_OR_MULTI
    {
        // In some cases (Guest VLAN), the request identity machine
        // is the actual SM from which we should draw the status.
        nas_sm_t *sub_sm = NAS_request_identity_state_machines[port_no - 1];
        if (sub_sm) {
            return NAS_funcs[agent]->get_status(sub_sm);
        }
    }
#endif

    return NAS_funcs[agent]->get_port_status(top_sm);
}

/******************************************************************************/
// nas_get_status()
// sm is never a top_sm
/******************************************************************************/
nas_port_status_t nas_get_status(nas_sm_t *sub_sm)
{
    if (!sub_sm) {
        T_EG(TRACE_GRP_BASE, "Invalid parameter");
        return NAS_PORT_STATUS_UNAUTHORIZED;
    }

    return NAS_funcs[sub_sm->sm_type]->get_status(sub_sm);
}

/******************************************************************************/
// nas_set_status()
// sm is never a top_sm
/******************************************************************************/
void nas_set_status(nas_sm_t *sub_sm, nas_port_status_t status)
{
    if (!sub_sm) {
        T_EG(TRACE_GRP_BASE, "Invalid parameter");
        return;
    }

    if (NAS_funcs[sub_sm->sm_type]->set_status) {
        NAS_funcs[sub_sm->sm_type]->set_status(sub_sm, status);
    } else {
        T_EG(TRACE_GRP_BASE, "Not supported by SM");
    }
}

/******************************************************************************/
// nas_get_eapol_counters()
/******************************************************************************/
nas_eapol_counters_t *nas_get_eapol_counters(nas_sm_t *top_or_sub_sm)
{
    nas_sm_type_t agent;

    if (!top_or_sub_sm) {
        T_EG(TRACE_GRP_BASE, "Invalid parameter");
        return NULL;
    }

    agent = NAS_get_type_from_port_mode_allow_top(top_or_sub_sm->port_info->port_control);

    if (NAS_funcs[agent]->get_eapol_counters) {
        return NAS_funcs[agent]->get_eapol_counters(top_or_sub_sm);
    }
    return NULL;
}

/******************************************************************************/
// nas_get_backend_counters()
// sm may either be a top_sm or a sub_sm. We really don't care, but use the
// port_control to figure out which function to call.
/******************************************************************************/
nas_backend_counters_t *nas_get_backend_counters(nas_sm_t *top_or_sub_sm)
{
    nas_sm_type_t agent;

    if (!top_or_sub_sm) {
        T_EG(TRACE_GRP_BASE, "Invalid parameter");
        return NULL;
    }

    agent = NAS_get_type_from_port_mode_allow_top(top_or_sub_sm->port_info->port_control);

    if (NAS_funcs[agent]->get_backend_counters) {
        return NAS_funcs[agent]->get_backend_counters(top_or_sub_sm);
    }
    return NULL;
}

/******************************************************************************/
// nas_clear_statistics()
// Clear statistics counters. If @vid_mac == NULL or @vid_mac->vid == 0,
// clear all counters on the port, otherwise clear only the selected.
/******************************************************************************/
void nas_clear_statistics(nas_port_t port_no, vtss_vid_mac_t *vid_mac)
{
    nas_sm_t      *top_or_sub_sm;
    nas_sm_type_t agent;

    if (vid_mac == NULL || vid_mac->vid == 0) {
        top_or_sub_sm = nas_get_top_sm(port_no);
    } else {
        top_or_sub_sm = nas_get_sm_from_vid_mac_port(vid_mac, port_no);
    }

    if (!top_or_sub_sm) {
        return;
    }

    agent = NAS_get_type_from_port_mode_allow_top(top_or_sub_sm->port_info->port_control);
    if (NAS_funcs[agent]->clear_statistics) {
        NAS_funcs[agent]->clear_statistics(top_or_sub_sm);
    }
}

/******************************************************************************/
// nas_get_client_info()
// Can either get the top client info or for a specific client.
/******************************************************************************/
nas_client_info_t *nas_get_client_info(nas_sm_t *sm)
{
    if (!sm) {
        T_EG(TRACE_GRP_BASE, "Invalid parameter");
        return NULL;
    }

    return &sm->client_info;
}

/******************************************************************************/
// nas_ieee8021x_eapol_frame_received()
/******************************************************************************/
void nas_ieee8021x_eapol_frame_received(nas_port_t port_no, u8 *frame, vtss_vid_t vid, size_t len)
{
    nas_sm_t      *top_sm = nas_get_top_sm(port_no);
    nas_sm_type_t agent;

    if (!top_sm) {
        return; // Error already printed.
    }

    agent = NAS_get_type_from_port_mode(top_sm->port_info->port_control);

    if (NAS_funcs[agent]->ieee8021x_eapol_frame_received) {
        NAS_funcs[agent]->ieee8021x_eapol_frame_received(top_sm, frame, vid, len);
    } else {
        T_DG(TRACE_GRP_BASE, "%u: Dropping 802.1X MAC frame with len %zu as the port is not in port-based 802.1X mode", port_no, len);
        return;
    }
}

#ifdef NAS_DOT1X_SINGLE_OR_MULTI
/******************************************************************************/
// nas_ieee8021x_eapol_frame_received_sm()
/******************************************************************************/
void nas_ieee8021x_eapol_frame_received_sm(nas_sm_t *sub_sm, u8 *frame, vtss_vid_t vid, size_t len)
{
    nas_sm_type_t agent;

    if (!sub_sm) {
        T_EG(TRACE_GRP_BASE, "NULL SM");
        return;
    }

    agent = NAS_get_type_from_port_mode(sub_sm->port_info->port_control);

    if (NAS_funcs[agent]->ieee8021x_eapol_frame_received) {
        NAS_funcs[agent]->ieee8021x_eapol_frame_received(sub_sm, frame, vid, len);
    } else {
        T_DG(TRACE_GRP_BASE, "%u: Dropping 802.1X MAC frame with len %zu as the port is not in Single- or Multi- 802.1X mode", sub_sm->port_info->port_no, len);
        return;
    }
}
#endif

/******************************************************************************/
// nas_reauthenticate()
/******************************************************************************/
void nas_reauthenticate(nas_port_t port_no)
{
    nas_sm_t      *top_sm = nas_get_top_sm(port_no);
    nas_sm_type_t agent;

    if (!top_sm) {
        return;
    }

    agent = NAS_get_type_from_port_mode_allow_top(top_sm->port_info->port_control);

    if (NAS_funcs[agent]->reauthenticate) {
        NAS_funcs[agent]->reauthenticate(top_sm);
    }
}

/******************************************************************************/
// nas_reinitialize()
// Causes a reauthentication NOW.
/******************************************************************************/
void nas_reinitialize(nas_port_t port_no)
{
    nas_sm_t      *top_sm = nas_get_top_sm(port_no);
    nas_sm_type_t agent;

    if (!top_sm) {
        return;
    }

    agent = NAS_get_type_from_port_mode_allow_top(top_sm->port_info->port_control);

    if (NAS_funcs[agent]->reinitialize) {
        NAS_funcs[agent]->reinitialize(top_sm);
    }
}

/******************************************************************************/
// nas_get_sm_from_radius_handle()
/******************************************************************************/
nas_sm_t *nas_get_sm_from_radius_handle(nas_port_t port_no, int radius_handle)
{
    nas_sm_t      *top_sm = nas_get_top_sm(port_no);
    nas_sm_type_t agent;

    if (!top_sm) {
        return NULL;
    }

    agent = NAS_get_type_from_port_mode(top_sm->port_info->port_control);

    if (NAS_funcs[agent]->get_sm_from_radius_handle) {
        return NAS_funcs[agent]->get_sm_from_radius_handle(top_sm, radius_handle);
    } else {
        return NULL;
    }
}

/******************************************************************************/
// nas_get_eap_info()
/******************************************************************************/
nas_eap_info_t *nas_get_eap_info(nas_sm_t *sm)
{
    if (!sm) {
        T_EG(TRACE_GRP_BASE, "SM is NULL");
        return NULL;
    }

    if (NAS_funcs[sm->sm_type]->get_eap_info) {
        return NAS_funcs[sm->sm_type]->get_eap_info(sm);
    } else {
        T_EG(TRACE_GRP_BASE, "%u: Invalid Type (%d)", sm->port_info->port_no, sm->sm_type);
        return NULL;
    }
}

/******************************************************************************/
// nas_get_port_info()
/******************************************************************************/
nas_port_info_t *nas_get_port_info(nas_sm_t *sm)
{
    if (!sm) {
        return NULL;
    }

    return sm->port_info;
}

/******************************************************************************/
// nas_get_next()
/******************************************************************************/
nas_sm_t *nas_get_next(nas_sm_t *sm)
{
    if (!sm) {
        return NULL;
    }

    return sm->next;
}

