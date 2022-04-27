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

#ifndef _NAS_H_
#define _NAS_H_

#include "dot1x_trace.h"
#include "vtss_nas_platform_api.h" /* For public types */
#ifdef VTSS_SW_OPTION_DOT1X
#include "../auth/auth_sm.h"     /* For auth_sm_t */
#endif
#ifdef VTSS_SW_OPTION_NAS_MAC_BASED
#include "../peer/peer_sm.h"     /* For peer_sm_t */
#endif

/******************************************************************************/
/******************************************************************************/
typedef enum {
    NAS_SM_TYPE_TOP,

#ifdef VTSS_SW_OPTION_DOT1X
    NAS_SM_TYPE_IEEE8021X,
#endif

#ifdef VTSS_SW_OPTION_NAS_MAC_BASED
    NAS_SM_TYPE_MAC_BASED,
#endif

    // This must come last
    NAS_SM_TYPE_LAST,
} nas_sm_type_t;
#define NAS_SM_TYPE_FIRST ((nas_sm_type_t)((int)NAS_SM_TYPE_TOP + 1))

/******************************************************************************/
// This is the structure that higher levels of software massage.
// It is a generic structure that supports both the entry to the
// NAS library (sm_type = NAS_SM_TYPE_TOP), and sub-statemacines.
// Users are allowed to access the port_info member directly.
// All other fields are private to the NAS library.
/******************************************************************************/
struct nas_sm {
    // Top-level stuff
    nas_sm_type_t sm_type;

    // In single- and multi- 802.1X modes, we need to let the last_eap_tx_identifier survive.
    // When initializing an SM, the last_eap_identifier of the new SM is therefore copied
    // from the top SM.
    u8 last_eap_tx_identifier;

    // General port configuration and status. nas.c allocates one per port and
    // when nas_alloc_sm() is called, it assigns it to the pointer of the allocated SM.
    nas_port_info_t *port_info;

    // If this is a top-SM, then @client_info contains the "last client" info.
    // Otherwise it contains this SM's client info.
    nas_client_info_t client_info;

    // If sm_type is NAS_SM_TYPE_TOP_LEVEL, then this one
    // points to the first sub-state machine, which can
    // be either IEEE8021X or MAC-Based.
    struct nas_sm *next;

    // Pointer to the 'real' state machine, which can be either
    // NULL      (for NAS_SM_TYPE_TOP_LEVEL),
    // auth_sm_t (for NAS_SM_TYPE_IEEE8021X),
    // peer_sm_t (for NAS_SM_TYPE_MAC_BASED),
    union {
        void      *real_sm;
        auth_sm_t *auth_sm;
#ifdef VTSS_SW_OPTION_NAS_MAC_BASED
        peer_sm_t *peer_sm;
#endif
    } u;
};

// Union of all agent SMs (attached to a struct nas_sm through the real_sm pointer), so that the same
// array or nas_sm_t SMs can address any agent's SM:
typedef union {
#ifdef VTSS_SW_OPTION_DOT1X
    auth_sm_t auth_sm;
#endif
#ifdef VTSS_SW_OPTION_NAS_MAC_BASED
    peer_sm_t peer_sm;
#endif
} nas_agent_sm_t;

// The sub-state-machines can register functions that will get called
// for generic events.
typedef struct {
    // All SMs:
    void                    (*init)(void);
    void                    (*init_sm)(nas_sm_t *sm, vtss_vid_mac_t *vid_mac, BOOL is_req_identity_sm); // This SM has just been allocated. The vid_mac arg may not be used for single-client agents.
    void                    (*uninit_sm)(nas_sm_t *sm); // This SM is going to be freed in a split-second
    void                    (*clear_statistics)(nas_sm_t *top_or_sub_sm); // If SM is top_sm, clear all statistics counters (including global), otherwise clear only for that SM
    nas_stop_reason_t       (*timer_tick)(nas_sm_t *sm); // Non-Top-SM. Gets called for every SM on a port. Mandatory. Must return NAS_STOP_REASON_NONE unless the SM is going to be deleted.
    nas_port_status_t       (*get_port_status)(nas_sm_t *top_sm); // Returns ((auth_cnt + 1) << 16) | (unauth_cnt + 1) if it's a multi-client
    nas_port_status_t       (*get_status)(nas_sm_t *sub_sm); // Never a top_sm
    void                    (*set_status)(nas_sm_t *sub_sm, nas_port_status_t status); // Never a top_sm. Optional

    // IEEE8021X SMs:
    nas_eapol_counters_t   *(*get_eapol_counters)(nas_sm_t *top_or_sub_sm);
    void                    (*ieee8021x_eapol_frame_received)(nas_sm_t *top_or_sub_sm, u8 *frame, vtss_vid_t vid, size_t len);
    void                    (*fake_force_authorized)(nas_sm_t *sub_sm, BOOL ena);
    void                    (*reset_reauth_cnt)(nas_sm_t *sub_sm);

    // Authentication SMs:
    void                    (*reauth_param_changed)(nas_sm_t *top_sm, BOOL ena, u16 reauth_period);
    void                    (*reauthenticate)(nas_sm_t *top_sm);
    void                    (*reinitialize)(nas_sm_t *top_sm);
    nas_sm_t               *(*get_sm_from_radius_handle)(nas_sm_t *top_sm, int radius_handle);
    void                    (*backend_frame_received)(nas_sm_t *sm, nas_backend_code_t code);
    void                    (*backend_server_timeout_occurred)(nas_sm_t *sm);
    nas_backend_counters_t *(*get_backend_counters)(nas_sm_t *top_or_sub_sm);  // If SM is top_sm, get overall counters for that port, otherwise get for that SM
    nas_eap_info_t         *(*get_eap_info)(nas_sm_t *sub_sm); // Never a top_sm
} nas_funcs_t;

// Common functions accessible by the sub-SMs:
u32 NAS_new_session_id(void);

#endif /* _NAS_H_ */

