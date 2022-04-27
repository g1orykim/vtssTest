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

/**
 * \file
 * \brief NAS Platform API
 * \details This header file defines semi-private types needed by the
 *          platform-specific code in order to implement the functions
 *          needed by the base library, and the vice versa API functions
 *          (i.e. the API that the base lib offers).
 */

#ifndef _VTSS_NAS_PLATFORM_API_H_
#define _VTSS_NAS_PLATFORM_API_H_

#include <time.h>            /* For time_t                      */
#include "vtss_nas_api.h"    /* For general NAS types           */
#include "vtss_radius_api.h" /* For VTSS_RADIUS_CODE_ACCESS_xxx */
#include "main.h"            /* For VTSS_ISID_CNT               */
#include "port_api.h"        /* For VTSS_PORTS                  */
#ifdef NAS_USES_PSEC
#include "psec_api.h"      /* For PSEC_MAC_ADDR_ENTRY_CNT     */
#endif

/** Opaque NAS handle */
typedef struct nas_sm nas_sm_t;

/** One-based. */
typedef u32 nas_port_t;

#define NAS_RADIUS_STATE_MAX_LENGTH         (  64)
#define NAS_RADIUS_EXTRA_SIZE               (  72) /* Extra bytes to encapsulate in RADIUS protocol */
#define NAS_IEEE8021X_MAX_EAPOL_PACKET_SIZE (RADIUS_MAX_FRAME_SIZE_BYTES - NAS_RADIUS_EXTRA_SIZE)
#define NAS_MAX_FRAME_SIZE                  (NAS_IEEE8021X_MAX_EAPOL_PACKET_SIZE + NAS_RADIUS_EXTRA_SIZE)
#define NAS_PORT_CNT                        (VTSS_PORTS * VTSS_ISID_CNT)

#ifdef NAS_USES_PSEC
#ifdef NAS_MULTI_CLIENT
// We're configured with at least one of MAC-based or Multi-802.1X and can therefore
// have more than one state machine (SM) client per port.
// Allocate as many SMs as there are entries in the MAC table
// as seen from the PSEC module's point of view.
// We live with the fact that it could be we need PSEC_MAC_ADDR_ENTRY_CNT
// entries for one single port, and therefore don't have entries left for
// the non-multi-client ports. If only PSEC_MAC_ADDR_ENTRY_CNT is just
// as big as the MAC table, this is not a problem at all anyway.
#define NAS_STATE_MACHINE_CNT PSEC_MAC_ADDR_ENTRY_CNT
#else
// Well - we're using the PSEC module, but we're not compiled with any
// of the multi-clients-per-port modules, so allocate only one
// state machine per port.
#define NAS_STATE_MACHINE_CNT NAS_PORT_CNT
#endif
#else
// We're not built with a mode that requires MAC table intervention,
// and therefore there can only be one SM per port.
#define NAS_STATE_MACHINE_CNT NAS_PORT_CNT
#endif

// Compile-time check: If it happens that PSEC_MAC_ADDR_ENTRY_CNT
// is less than the number of ports, then allocate NAS_PORT_CNT
// entries instead
#if NAS_STATE_MACHINE_CNT < NAS_PORT_CNT
#undef NAS_STATE_MACHINE_CNT
#define NAS_STATE_MACHINE_CNT NAS_PORT_CNT
#endif

#define NAS_IEEE8021X_ETH_TYPE (0x888E)
#define NAS_IEEE8021X_MAC_ADDR {0x01, 0x80, 0xc2, 0x00, 0x00, 0x03}

/****************************************************************************/
// To keep the NAS base library backend server type independent, we make
// these enums (which happen to match the RADIUS codes).
/****************************************************************************/
typedef enum {
    NAS_BACKEND_CODE_ACCESS_ACCEPT    = VTSS_RADIUS_CODE_ACCESS_ACCEPT,
    NAS_BACKEND_CODE_ACCESS_REJECT    = VTSS_RADIUS_CODE_ACCESS_REJECT,
    NAS_BACKEND_CODE_ACCESS_CHALLENGE = VTSS_RADIUS_CODE_ACCESS_CHALLENGE,
} nas_backend_code_t;

/******************************************************************************/
// Special definition for Vitesse switch applications
/******************************************************************************/
typedef enum { FRAME_TYPE_EAPOL, FRAME_TYPE_RADIUS, FRAME_TYPE_NO_RETRANSMISSION } nas_frame_type_t;

/******************************************************************************/
// Enumeration of reasons for an authentication failure or freeing of an SM.
/******************************************************************************/
typedef enum {
    NAS_STOP_REASON_NONE,                   /**< 802.1X, MB: No reason to stop.                                                   */
    NAS_STOP_REASON_UNKNOWN,                /**< 802.1X, MB: Unknown reason.                                                      */
    NAS_STOP_REASON_INITIALIZING,           /**< 802.1X, MB: (Re-)initializing                                                    */
    NAS_STOP_REASON_AUTH_FAILURE,           /**< 802.1X, MB: Backend server sent an Auth Fail                                     */
    NAS_STOP_REASON_AUTH_NOT_CONFIGURED,    /**<         MB: Backend server not configured                                        */
    NAS_STOP_REASON_AUTH_TOO_MANY_ROUNDS,   /**<         MB: Too many authentication rounds                                       */
    NAS_STOP_REASON_AUTH_TIMEOUT,           /**< 802.1X, MB: Backend server didn't reply                                          */
    NAS_STOP_REASON_EAPOL_START,            /**< 802.1X    : Supplicant sent start frame                                          */
    NAS_STOP_REASON_EAPOL_LOGOFF,           /**< 802.1X    : Supplicant sent logoff frame and we're now in the DISCONNECTED state */
    NAS_STOP_REASON_EAPOL_LOGOFF_FROM_HELD, /**< 802.1X    : Supplicant sent logoff frame and we're now in the HELD state         */
    NAS_STOP_REASON_REAUTH_COUNT_EXCEEDED,  /**< 802.1X    : Reauth count exceeded                                                */
    NAS_STOP_REASON_FORCED_UNAUTHORIZED,    /**< 802.1X    : Port mode forced unauthorized                                        */
    NAS_STOP_REASON_MAC_TABLE_ERROR,        /**<       , MB: MAC table full (H/W or S/W)                                          */
    NAS_STOP_REASON_SWITCH_DOWN,
    NAS_STOP_REASON_PORT_LINK_DOWN,
    NAS_STOP_REASON_STATION_MOVED,
    NAS_STOP_REASON_AGED_OUT,
    NAS_STOP_REASON_HOLD_TIME_EXPIRED,
    NAS_STOP_REASON_PORT_MODE_CHANGED,
    NAS_STOP_REASON_PORT_SHUT_DOWN,
    NAS_STOP_REASON_SWITCH_REBOOT,
    NAS_STOP_REASON_GUEST_VLAN_ENTER,
    NAS_STOP_REASON_GUEST_VLAN_EXIT,
} nas_stop_reason_t;

#ifdef VTSS_SW_OPTION_DOT1X_ACCT
/******************************************************************************/
// Shared data for accounting
/******************************************************************************/
typedef struct acct_sm {
    int                 handle;             // -1 = not in use.
    u32                 session_id;         //  0 = no session id
    u32                 status_type;        // Last Acct-Status_Type transmitted

    BOOL                counters;           // TRUE if the counters below can be trusted
    vtss_port_counter_t input_octets;       // Contains the number of the input octets when the session started
    vtss_port_counter_t output_octets;      // Contains the number of the output octets when the session started
    vtss_port_counter_t input_packets;      // Contains the number of the input packets when the session started
    vtss_port_counter_t output_packets;     // Contains the number of the output packets when the session started

    u32                 session_time;       // Contains the time when the session started (seconds from boot)

    u32                 state;              // Accounting state
    BOOL                authorize_disabled; // Authorized state has changed to diaabled
    BOOL                authorize_enabled;  // Authorized state has changed to enabled

    BOOL                releasing;          // The state machine is being released/freed (helps us to send correct terminate cause in acc stop messages)

    u32                 interim_interval;   // The interval between interim update messages - set from RADIUS Access-Accept message
    u32                 interim_timer;      // The interim count-down timer

} acct_sm_t;
#endif

/******************************************************************************/
// Data shared between both the auth and peer state machines.
// This structure is used in all RADIUS-related calls, since the RADIUS
// functionality is needed by both authenticator and peer state machines.
/******************************************************************************/
typedef struct {
    // RADIUS stuff
    u8                     radius_state_attribute[NAS_RADIUS_STATE_MAX_LENGTH];
    u8                     radius_state_attribute_len;
    int                    radius_handle; // -1 = not in use.

    nas_frame_type_t       last_frame_type;
    u16                    last_frame_len;
    u8                     last_frame[NAS_MAX_FRAME_SIZE];

    // Backend counters for both Peer and Authenticator SMs.
    nas_backend_counters_t backend_counters;

    nas_stop_reason_t      stop_reason;

    // Can be signalled from within SM in order to force a deletion on the next timer tick.
    BOOL                   delete_me;

    // VID of the client before the VLAN was overridden.
    // Not used by the base-lib. The client_info->vid_mac always corresponds to
    // what is in the MAC table.
    vtss_vid_t             revert_vid;

#ifdef VTSS_SW_OPTION_DOT1X_ACCT
    acct_sm_t              acct;
#endif
} nas_eap_info_t;

/******************************************************************************/
// Port Info
/******************************************************************************/
typedef struct {
    nas_port_t             port_no;
    nas_port_control_t     port_control;
    u32                    cur_client_cnt;    // Counts number of SMs attached to this port.
    nas_sm_t               *top_sm;           // For fast reference to the top-level SM
    nas_backend_counters_t *backend_counters; // Per-port backend counters (the summed if multi-client).
    nas_eapol_counters_t   *eapol_counters;   // Per-port EAPOL counters (the summed if multi-client).

#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS
    vtss_prio_t            qos_class;
#endif

#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_VLAN
    vtss_vid_t             backend_assigned_vid; // Only non-zero sporadically.
#endif

#ifdef VTSS_SW_OPTION_NAS_GUEST_VLAN
    BOOL                   eapol_frame_seen;     // FALSE if no EAPOL frames have been seen for the life-time of this link, TRUE otherwise.
#endif

#ifdef NAS_USES_VLAN
    vtss_vid_t             current_vid;          // 0 if not overridden.
    nas_vlan_type_t        vlan_type;            // To be able to see why the current vid is non-zero.
#endif
} nas_port_info_t;

// Functions common to all types of port security
void                    nas_1sec_timer_tick(void);
void                    nas_init(void);
nas_sm_t               *nas_get_top_sm(nas_port_t port_no);
nas_sm_t               *nas_get_sm_from_radius_handle(nas_port_t port_no, int radius_handle);
u32                     nas_get_client_cnt(nas_port_t port_no);
nas_port_control_t      nas_get_port_control(nas_port_t port_no);
void                    nas_set_port_control(nas_port_t port_no, nas_port_control_t new_mode);
nas_port_status_t       nas_get_port_status(nas_port_t port_no); // Returns ((auth_cnt + 1) << 16) | (unauth_cnt + 1) if it's a multi-client
nas_port_status_t       nas_get_status(nas_sm_t *sm); // Never a Top-SM
void                    nas_set_status(nas_sm_t *sm, nas_port_status_t status); // Never a Top-SM. Use with care. Only implemented for MAC-limit
void                    nas_reauth_param_changed(BOOL ena, u16 reauth_period);
void                    nas_reauthenticate(nas_port_t port_no);
void                    nas_reinitialize(nas_port_t port_no);
void                    nas_backend_server_timeout_occurred(nas_sm_t *sm);
void                    nas_backend_frame_received(nas_sm_t *sm, nas_backend_code_t code);
nas_backend_counters_t *nas_get_backend_counters(nas_sm_t *top_or_sub_sm);
nas_client_info_t      *nas_get_client_info(nas_sm_t *top_or_sub_sm); // SM = Top => last client. SM = Sub => the one's
void                    nas_clear_statistics(nas_port_t port_no, vtss_vid_mac_t *vid_mac);
nas_sm_t               *nas_alloc_sm(nas_port_t port_no, vtss_vid_mac_t *vid_mac);
void                    nas_free_sm(nas_sm_t *sm, nas_stop_reason_t stop_reason); // Free a specific state machine
void                    nas_free_all_sms(nas_port_t port_no, nas_stop_reason_t stop_reason); // Free all state machines on a port.
nas_sm_t               *nas_get_sm_from_vid_mac_port(vtss_vid_mac_t *vid_mac, nas_port_t port_no);

#ifdef VTSS_SW_OPTION_NAS_GUEST_VLAN
void nas_set_fake_force_authorized(nas_sm_t *top_sm, BOOL enable);
void nas_reset_reauth_cnt(nas_sm_t *top_sm);
#endif

// IEEE8021X specific calls
void     nas_ieee8021x_eapol_frame_received(nas_port_t port_no, u8 *frame, vtss_vid_t vid, size_t len);
#ifdef NAS_DOT1X_SINGLE_OR_MULTI
nas_sm_t *nas_get_sm_from_mac(vtss_vid_mac_t *mac);
void     nas_ieee8021x_eapol_frame_received_sm(nas_sm_t *sub_sm, u8 *frame, vtss_vid_t vid, size_t len);
#endif
nas_eapol_counters_t *nas_get_eapol_counters(nas_sm_t *top_or_sub_sm);

// Authentication-specific calls (IEEE8021X & MAC-Based)
nas_eap_info_t  *nas_get_eap_info(nas_sm_t *sm);
nas_port_info_t *nas_get_port_info(nas_sm_t *sm);
nas_sm_t        *nas_get_next(nas_sm_t *sm);

/****************************************************************************/
// Platform-specific functions needed by the NAS base lib (callouts).
/****************************************************************************/
BOOL          nas_os_get_reauth_enabled(void);
u16           nas_os_get_reauth_timer(void);
void          nas_os_mac2str(u8 *mac, i8 *str);
void          nas_os_get_port_mac(nas_port_t port_no, u8 *frame);
void          nas_os_ieee8021x_send_eapol_frame(nas_port_t port_no, u8 *frame, size_t len);
u16           nas_os_get_eapol_request_identity_timeout(void);
u16           nas_os_get_eapol_challenge_timeout(void);
BOOL          nas_os_backend_server_tx_request(nas_sm_t *sm, int *handle, u8 *eap, u16 eap_len, u8 *state, u8 state_len, i8 *user, nas_port_t nas_port, u8 *mac_addr);
void          nas_os_backend_server_free_resources(nas_sm_t *sm);
void          nas_os_set_authorized(nas_sm_t *sm, BOOL authorized, BOOL chgd);
time_t        nas_os_get_uptime_secs(void);
void          nas_os_freeing_sm(nas_sm_t *sm);
void          nas_os_init(nas_port_info_t *port_info);
nas_counter_t nas_os_get_reauth_max(void);

#ifdef NAS_MULTI_CLIENT
nas_port_status_t nas_os_encode_auth_unauth(u32 auth_cnt, u32 unauth_cnt);
#endif

#ifdef VTSS_SW_OPTION_DOT1X_ACCT
// RADIUS Accounting callbacks
void nas_os_acct_timer_tick(nas_sm_t *sm);
void nas_os_acct_sm_init(nas_sm_t *sm);
void nas_os_acct_sm_release(nas_sm_t *sm);
#endif

#endif /* _VTSS_NAS_PLATFORM_API_H_ */
