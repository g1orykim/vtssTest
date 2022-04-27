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

*/

#ifndef _DOT1X_API_H_
#define _DOT1X_API_H_

#include "vtss_nas_api.h"  /* For nax_XXX, NAS_USES_PSEC, NAS_MULTI_CLIENT, etc. */
#include "vtss_types.h"    /* For uXXX, iXXX, and BOOL                           */

#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS
#include "qos_api.h"     /* For vtss_prio_t                                    */
#endif

/****************************************************************************/
// Various defines of min/max/defaults
/****************************************************************************/
#define DOT1X_REAUTH_PERIOD_SECS_MIN         1
#define DOT1X_REAUTH_PERIOD_SECS_MAX      3600
#define DOT1X_REAUTH_PERIOD_SECS_DEFAULT  3600
#define DOT1X_EAPOL_TIMEOUT_SECS_MIN         1
#define DOT1X_EAPOL_TIMEOUT_SECS_MAX     65535
#define DOT1X_EAPOL_TIMEOUT_SECS_DEFAULT    30

#ifdef NAS_USES_PSEC
#define NAS_PSEC_AGING_ENABLED_DEFAULT       TRUE /* NO SUPPORT IN CLI OR WEB FOR CHANGING IT, SO BE CAREFUL IF CHANGING DEFAULT */
#define NAS_PSEC_AGING_PERIOD_SECS_DEFAULT    300
#define NAS_PSEC_AGING_PERIOD_SECS_MIN         10
#define NAS_PSEC_AGING_PERIOD_SECS_MAX    1000000

#define NAS_PSEC_HOLD_ENABLED_DEFAULT        TRUE /* NO SUPPORT IN CLI OR WEB FOR CHANGING IT, SO BE CAREFUL IF CHANGING DEFAULT */
#define NAS_PSEC_HOLD_TIME_SECS_DEFAULT        10
#define NAS_PSEC_HOLD_TIME_SECS_MIN            10
#define NAS_PSEC_HOLD_TIME_SECS_MAX       1000000
#endif

/****************************************************************************/
// API Error Return Codes (vtss_rc)
/****************************************************************************/
enum {
    DOT1X_ERROR_INV_PARAM = MODULE_ERROR_START(VTSS_MODULE_ID_DOT1X), // NULL parameter passed to one of the dot1x_mgmt_XXX functions, where a non-NULL was expected.
    DOT1X_ERROR_INVALID_REAUTH_PERIOD,                                // Reauthentication period out of bounds.
    DOT1X_ERROR_INVALID_EAPOL_TIMEOUT,                                // EAPOL timeout out of bounds.
    DOT1X_ERROR_INVALID_ADMIN_STATE,                                  // Invalid administration state.
    DOT1X_ERROR_MUST_BE_MASTER,                                       // Management operation is not valid on slave switches.
    DOT1X_ERROR_ISID,                                                 // Invalid ISID.
    DOT1X_ERROR_PORT,                                                 // Invalid port number.
    DOT1X_ERROR_STATIC_AGGR_ENABLED,                                  // Static aggregation is enabled on a port that is attempted set to Force Unauthorized or Auto.
    DOT1X_ERROR_DYNAMIC_AGGR_ENABLED,                                 // Dynamic aggregation (LACP) is enabled on a port that is attempted set to Force Unauthorized or Auto.
    DOT1X_ERROR_STP_ENABLED,                                          // Spanning Tree is enabled on a port that is attempted set to Force Unauthorized or Auto.
    DOT1X_ERROR_MAC_ADDRESS_NOT_FOUND,                                // No state machine found corresponding to specified MAC address.
#ifdef NAS_USES_PSEC
    DOT1X_ERROR_INVALID_HOLD_TIME,                                    // The hold-time for clients whose authentication failed was out of bounds.
    DOT1X_ERROR_INVALID_AGING_PERIOD,                                 // The aging-period for clients whose authentication succeeded was out of bounds.
#endif
#ifdef VTSS_SW_OPTION_NAS_GUEST_VLAN
    DOT1X_ERROR_INVALID_GUEST_VLAN_ID,                                // The Guest VLAN ID is invalid.
    DOT1X_ERROR_INVALID_REAUTH_MAX,                                   // The maximum number of reauthentications is invalid.
#endif
};

/****************************************************************************/
// Global configuration. Common to all switches in the stack.
//
// The following structure defines 802.1X and MAC-based parameters.
/****************************************************************************/
typedef struct {

    // Globally enable/disable 802.1X/MAC-Based authentication
    BOOL enabled;

    // The following parameters define state machine behavior.
    // If enabled, the switch will re-authenticate after the interval
    // specified by .reauth_timer
    BOOL reauth_enabled;

    // If .reauth_enabled, this specifies the period between client
    // reauthentications in seconds.
    u16 reauth_period_secs;

    // If the supplicant doesn't reply before this timeout, the switch
    // retransmits EAPOL Request Identity packets.
    u16 eapol_timeout_secs;

#ifdef NAS_USES_PSEC
    // The following parameters define the amount of time that
    // an authenticated MAC address will reside in the MAC table
    // before it will be aged out if there's no traffic.
    // Currently there is no support for disabling this in CLI or Web.
    BOOL psec_aging_enabled;

    // If .psec_aging_enabled, this specifies the aging period in seconds.
    // At @psec_aging_period_secs, the CPU starts listening to frames from the
    // given MAC address, and if none arrives before @psec_aging_period_secs, the
    // entry will be removed.
    u32 psec_aging_period_secs;

    // The following parameters define the amount of time that
    // a MAC address resides in the MAC table if authentication failed.
    // Currently there is no support for disabling this in CLI or Web.
    BOOL psec_hold_enabled;

    // If .psec_hold_enabled, this specifies the amount of time in seconds
    // before a client whose authentication failed gets removed.
    u32 psec_hold_time_secs;
#endif

#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS
    // Set to TRUE if RADIUS-assigned QoS is globally enabled.
    BOOL qos_backend_assignment_enabled;

    // See the reasoning behind this in the backend_assigned_vlan_padding
    // comment below.
    u32 backend_assigned_qos_padding;
#endif

#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_VLAN
    // Set to TRUE if RADIUS-assigned VLAN is globally enabled.
    BOOL vlan_backend_assignment_enabled;

    // Well-well. Unfortunately, we cannot see the difference in size
    // of this structure and thereby the whole flash-layout
    // whether VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_VLAN is defined
    // or not. This causes the flash-contents to be kind of
    // nonsense when going from one build-option to the next.
    // Therefore we allocate a dummy-u32 here, to force a new size,
    // which in turn will cause the flash-size to be different
    // and thereby cause a force of defaults when enabling this
    // option. One could also have stepped the flash version
    // number (DOT1X_FLASH_CFG_VERSION), but that will only help
    // us the first time. The next time this option is taken
    // in or out, we will have the same problem.
    // Luckily, this structure is only instantiated once in the
    // flash, so it doesn't consume much memory.
    u32 backend_assigned_vlan_padding;
#endif

#ifdef VTSS_SW_OPTION_NAS_GUEST_VLAN
    // Set to TRUE if Guest VLAN is globally enabled
    BOOL guest_vlan_enabled;

    // This it the guest VLAN
    vtss_vid_t guest_vid;

    // This is the number of times that the switch sends
    // Request Identity EAPOL frames to the supplicant before
    // restarting the authentication process. And if Guest VLAN
    // is enabled, the supplicant will end up on this if no
    // EAPOL frames were received in the meanwhile.
    u32 reauth_max;

    // If the following is TRUE, then the supplicant is allowed into
    // the Guest VLAN even if an EAPOL frame is seen for the lifetime
    // of the port.
    // If FALSE, then if just one EAPOL frame is seen on the port,
    // then it will never get moved into the Guest VLAN.
    // Note that the reception of an EAPOL frame *after* the port
    // has been moved into Guest VLAN will take it out of Guest
    // VLAN mode.
    // Default is FALSE.
    BOOL guest_vlan_allow_eapols;

    // Note on padding: Padding is not needed in this case, because
    // the size of the structure grows with more than 4 bytes when
    // compiling with this feature.
#endif
} dot1x_glbl_cfg_t;

/****************************************************************************/
// Port configuration.
/****************************************************************************/
typedef struct {
    // Administrative state.
    //
    // NAS_PORT_CONTROL_FORCE_AUTHORIZED = 1:
    //   Forces a port to grant access to all clients, 802.1X-aware or not.
    //
    // NAS_PORT_CONTROL_AUTO = 2:
    //   Requires an 802.1X-aware client to be authorized by the authentication
    //   server. Clients that are not 802.1X-aware will be denied access.
    //
    // NAS_PORT_CONTROL_FORCE_UNAUTHORIZED = 3:
    //   Forces a port to deny access to all clients, 802.1X-aware or not.
    //
    // NAS_PORT_CONTROL_MAC_BASED:
    //   The switch authenticates on behalf of the client, using the client's
    //   MAC-address as the username and password and MD5 EAP method.

    // NAS_PORT_CONTROL_DOT1X_SINGLE:
    //   At most one supplicant is allowed to authenticate, and it authenticates
    //   using normal 802.1X frames.

    // NAS_PORT_CONTROL_DOT1X_MULTI:
    //   One or more supplicants are allowed to authenticate individually using
    //   an 802.1X variant, where EAPOL frames sent from the switch are directed
    //   towards the supplicants MAC address instead of using the multicast
    //   BPDU MAC address. Unauthenticated supplicants won't get access.
    nas_port_control_t admin_state;

#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS
    // Set to TRUE if RADIUS-assigned QoS is enabled for this port.
    BOOL qos_backend_assignment_enabled;
#endif

#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_VLAN
    // Set to TRUE if RADIUS-assigned VLAN is enabled for this port.
    BOOL vlan_backend_assignment_enabled;
#endif

#ifdef VTSS_SW_OPTION_NAS_GUEST_VLAN
    // Set to TRUE if Guest-VLAN is enabled for this port.
    BOOL guest_vlan_enabled;
#endif
} dot1x_port_cfg_t;

/****************************************************************************/
// Switch configuration.
/****************************************************************************/
typedef struct {
    // Administrative state.
    dot1x_port_cfg_t port_cfg[VTSS_PORTS];
} dot1x_switch_cfg_t;

/******************************************************************************/
// Statistics
/******************************************************************************/
typedef struct {
    // If top-level SM: Last client info
    // If sub-SM: Current client_info.
    nas_client_info_t      client_info;

    // Only valid for BPDU-based admin_states.
    nas_eapol_counters_t   eapol_counters;

    // Only valid for admin_states using a backend server.
    nas_backend_counters_t backend_counters;

    // The status is encoded as follows:
    // If top-level SM:
    //   0 = Link down, 1 = Authorized, 2 = Unauthorized, 3 = NAS globally disabled, > 3 = multi-client auth/unauth count.
    // Of sub-SM:
    //   1 = Authorized, 2 = Unauthorized.
    nas_port_status_t      status;

#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS
    // The QoS class that this port is assigned to by the backend server.
    // QOS_PORT_PRIO_UNDEF if unassigned.
    vtss_prio_t            qos_class;
#endif

#ifdef NAS_USES_VLAN
    // The VLAN that this port is assigned to and the reason (RADIUS- or Guest VLAN).
    // 0 if unassigned.
    vtss_vid_t             vid;
    nas_vlan_type_t        vlan_type;
#endif

    // This one saves a call to the management functions, since the caller
    // sometimes needs to know the administrative state of the port in
    // order to tell which counters are actually active.
    nas_port_control_t     admin_state;
} dot1x_statistics_t;

#ifdef NAS_MULTI_CLIENT
/******************************************************************************/
// State of a given client on a multi-client port.
/******************************************************************************/
typedef struct {
    nas_client_info_t client_info;
    nas_port_status_t status;
} dot1x_multi_client_status_t;
#endif

/******************************************************************************/
// State of ports.
/******************************************************************************/
typedef struct {
    // nas_port_status_t : NAS_PORT_STATUS_LINK_DOWN, NAS_PORT_STATUS_AUTHORIZED, NAS_PORT_STATUS_UNAUTHORIZED, NAS_PORT_STATUS_DISABLED (overrides everything else).
    nas_port_status_t  status[VTSS_PORTS];
    nas_port_control_t admin_state[VTSS_PORTS];
#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS
    vtss_prio_t        qos_class[VTSS_PORTS];
#endif
#ifdef NAS_USES_VLAN
    nas_vlan_type_t    vlan_type[VTSS_PORTS];
    vtss_vid_t         vid[VTSS_PORTS];
#endif
} dot1x_switch_status_t;

/******************************************************************************/
// dot1x_mgmt_glbl_cfg_get()
/******************************************************************************/
vtss_rc dot1x_mgmt_glbl_cfg_get(dot1x_glbl_cfg_t *glbl_cfg);

/******************************************************************************/
// dot1x_mgmt_glbl_cfg_set()
/******************************************************************************/
vtss_rc dot1x_mgmt_glbl_cfg_set(dot1x_glbl_cfg_t *glbl_cfg);

/******************************************************************************/
// dot1x_mgmt_switch_cfg_get()
/******************************************************************************/
vtss_rc dot1x_mgmt_switch_cfg_get(vtss_isid_t isid, dot1x_switch_cfg_t *switch_cfg);

/******************************************************************************/
// dot1x_mgmt_switch_cfg_set()
/******************************************************************************/
vtss_rc dot1x_mgmt_switch_cfg_set(vtss_isid_t isid, dot1x_switch_cfg_t *switch_cfg);

/******************************************************************************/
// dot1x_mgmt_port_status_get()
/******************************************************************************/
vtss_rc dot1x_mgmt_port_status_get(vtss_isid_t isid, vtss_port_no_t port, nas_port_status_t *port_status);

#ifdef NAS_MULTI_CLIENT
/******************************************************************************/
// dot1x_mgmt_multi_client_status_get()
/******************************************************************************/
vtss_rc dot1x_mgmt_multi_client_status_get(vtss_isid_t isid, vtss_port_no_t port, dot1x_multi_client_status_t *client_status, BOOL *found, BOOL start_all_over);
#endif

#ifdef NAS_MULTI_CLIENT
/******************************************************************************/
// dot1x_mgmt_multi_client_statistics_get()
// For the sake of CLI, we pass a dot1x_statistics_t structure to this
// function.
/******************************************************************************/
vtss_rc dot1x_mgmt_multi_client_statistics_get(vtss_isid_t isid, vtss_port_no_t port, dot1x_statistics_t *statistics, BOOL *found, BOOL start_all_over);
#endif

#ifdef NAS_MULTI_CLIENT
/******************************************************************************/
// dot1x_mgmt_decode_auth_unauth()
/******************************************************************************/
void dot1x_mgmt_decode_auth_unauth(nas_port_status_t status, u32 *auth_cnt, u32 *unauth_cnt);
#endif

/******************************************************************************/
// dot1x_mgmt_switch_status_get()
/******************************************************************************/
vtss_rc dot1x_mgmt_switch_status_get(vtss_isid_t isid, dot1x_switch_status_t *switch_status);

/******************************************************************************/
// dot1x_mgmt_statistics_get()
// If @vid_mac == NULL or vid_mac->vid == 0, get the port statistics,
// otherwise get the statistics given by @vid_mac.
/******************************************************************************/
vtss_rc dot1x_mgmt_statistics_get(vtss_isid_t isid, vtss_port_no_t port, vtss_vid_mac_t *vid_mac, dot1x_statistics_t *statistics);

/******************************************************************************/
// dot1x_mgmt_port_last_supplicant_info_get()
/******************************************************************************/
vtss_rc dot1x_mgmt_port_last_supplicant_info_get(vtss_isid_t isid, vtss_port_no_t port, nas_client_info_t *last_supplicant_info);

/******************************************************************************/
// dot1x_mgmt_statistics_clear()
// If @vid_mac == NULL or vid_mac->vid == 0, clear everything on that port,
// otherwise only clear entry given by @vid_mac.
/******************************************************************************/
vtss_rc dot1x_mgmt_statistics_clear(vtss_isid_t isid, vtss_port_no_t port, vtss_vid_mac_t *vid_mac);

/******************************************************************************/
// dot1x_mgmt_reauth()
/******************************************************************************/
vtss_rc dot1x_mgmt_reauth(vtss_isid_t isid, vtss_port_no_t port, BOOL now);

/******************************************************************************/
// dot1x_error_txt()
/******************************************************************************/
char *dot1x_error_txt(vtss_rc rc);

/******************************************************************************/
// dot1x_port_control_to_str()
/******************************************************************************/
char *dot1x_port_control_to_str(nas_port_control_t port_control, BOOL brief);

/******************************************************************************/
// dot1x_qos_class_to_str()
// Helper function to create a useful RADIUS-assigned QoS string.
// It'll become empty if the option is not supported.
// String must be at least 20 chars long.
/******************************************************************************/
void dot1x_qos_class_to_str(vtss_prio_t iprio, char *str);

/******************************************************************************/
// dot1x_vlan_to_str()
// Helper function to create a useful VLAN string.
// It'll become empty if the option is not supported.
// String must be at least 20 chars long.
/******************************************************************************/
void dot1x_vlan_to_str(vtss_vid_t vid, char *str);

/******************************************************************************/
// dot1x_init()
// Initialize 802.1X Module
/******************************************************************************/
vtss_rc dot1x_init(vtss_init_data_t *data);

/******************************************************************************/
// DOT1X_cfg_default_glbl()
// Initialize global settings to defaults.
/******************************************************************************/
void DOT1X_cfg_default_glbl(dot1x_glbl_cfg_t *cfg);

/******************************************************************************/
// DOT1X_cfg_default_switch()
// Initialize per-switch settings to defaults.
/******************************************************************************/
void DOT1X_cfg_default_switch(dot1x_switch_cfg_t *cfg);
#endif /* _DOT1X_API_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
