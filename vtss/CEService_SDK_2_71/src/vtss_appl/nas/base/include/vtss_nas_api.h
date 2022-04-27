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
 * \brief NAS API
 * \details This header file defines the public types needed by both the
 *          platform-specific code and the platform-specific code's own
 *          API.
 */

#ifndef _VTSS_NAS_API_H_
#define _VTSS_NAS_API_H_

#if defined(VTSS_SW_OPTION_NAS_DOT1X_SINGLE) || defined(VTSS_SW_OPTION_NAS_DOT1X_MULTI) || defined(VTSS_SW_OPTION_NAS_MAC_BASED)
// At least one mode that uses the MAC table (and thereby the PSEC module) is defined
#define NAS_USES_PSEC
#else
#undef  NAS_USES_PSEC
#endif

#if defined(VTSS_SW_OPTION_NAS_DOT1X_MULTI) || defined(VTSS_SW_OPTION_NAS_MAC_BASED)
// At least one mode that allows more than one client attached to the port
#define NAS_MULTI_CLIENT
#else
#undef  NAS_MULTI_CLIENT
#endif

#if defined(VTSS_SW_OPTION_NAS_DOT1X_SINGLE) || defined(VTSS_SW_OPTION_NAS_DOT1X_MULTI)
#define NAS_DOT1X_SINGLE_OR_MULTI
#else
#undef  NAS_DOT1X_SINGLE_OR_MULTI
#endif

#if defined(VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS_RFC4675) || defined(VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS_CUSTOM)
#define VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS
#else
#undef  VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS
#endif

#if defined(VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_VLAN) || defined(VTSS_SW_OPTION_NAS_GUEST_VLAN)
#define NAS_USES_VLAN
#else
#undef  NAS_USES_VLAN
#endif

#include <time.h>        /* For time_t               */
#include "vtss_types.h"  /* For uXXX, iXXX, and BOOL */
#include "vtss_api.h"    /* For vtss_vid_mac_t       */

#define NAS_SUPPLICANT_ID_MAX_LENGTH (40)

typedef u32 nas_counter_t;

/****************************************************************************/
// If changing the enumeration, remember to also change dot1x.htm.
/****************************************************************************/
typedef enum {
    NAS_PORT_CONTROL_DISABLED           = 0,
    NAS_PORT_CONTROL_FORCE_AUTHORIZED   = 1,
    NAS_PORT_CONTROL_AUTO               = 2,
    NAS_PORT_CONTROL_FORCE_UNAUTHORIZED = 3,
#ifdef VTSS_SW_OPTION_NAS_MAC_BASED
    NAS_PORT_CONTROL_MAC_BASED          = 4,
#endif
#ifdef VTSS_SW_OPTION_NAS_DOT1X_SINGLE
    NAS_PORT_CONTROL_DOT1X_SINGLE       = 5,
#endif
#ifdef VTSS_SW_OPTION_NAS_DOT1X_MULTI
    NAS_PORT_CONTROL_DOT1X_MULTI        = 6,
#endif
} nas_port_control_t;

#ifdef NAS_USES_VLAN
typedef enum {
    NAS_VLAN_TYPE_NONE,
    NAS_VLAN_TYPE_BACKEND_ASSIGNED,
    NAS_VLAN_TYPE_GUEST_VLAN
} nas_vlan_type_t;
#endif

/******************************************************************************/
// nas_eapol_counters_t
/******************************************************************************/
typedef struct {
    nas_counter_t authEntersConnecting;
    nas_counter_t authEapLogoffsWhileConnecting;
    nas_counter_t authEntersAuthenticating;
    nas_counter_t authAuthSuccessesWhileAuthenticating;
    nas_counter_t authAuthTimeoutsWhileAuthenticating;
    nas_counter_t authAuthFailWhileAuthenticating;
    nas_counter_t authAuthEapStartsWhileAuthenticating;
    nas_counter_t authAuthEapLogoffWhileAuthenticating;
    nas_counter_t authAuthReauthsWhileAuthenticated;
    nas_counter_t authAuthEapStartsWhileAuthenticated;
    nas_counter_t authAuthEapLogoffWhileAuthenticated;

    // Authenticator Statistics Table
    nas_counter_t dot1xAuthEapolFramesRx;
    nas_counter_t dot1xAuthEapolFramesTx;
    nas_counter_t dot1xAuthEapolStartFramesRx;
    nas_counter_t dot1xAuthEapolLogoffFramesRx;
    nas_counter_t dot1xAuthEapolRespIdFramesRx;
    nas_counter_t dot1xAuthEapolRespFramesRx;
    nas_counter_t dot1xAuthEapolReqIdFramesTx;
    nas_counter_t dot1xAuthEapolReqFramesTx;
    nas_counter_t dot1xAuthInvalidEapolFramesRx;
    nas_counter_t dot1xAuthEapLengthErrorFramesRx;
    nas_counter_t dot1xAuthLastEapolFrameVersion;
} nas_eapol_counters_t;

/******************************************************************************/
// nas_backend_counters_t
/******************************************************************************/
typedef struct {
    nas_counter_t backendResponses;
    nas_counter_t backendAccessChallenges;
    nas_counter_t backendOtherRequestsToSupplicant;
    nas_counter_t backendAuthSuccesses;
    nas_counter_t backendAuthFails;
} nas_backend_counters_t;

/******************************************************************************/
// Supplicant/Client Info
/******************************************************************************/
typedef struct {
    vtss_vid_mac_t vid_mac;          // VLAN ID and binary version of mac_addr_str
    i8             mac_addr_str[18]; // Console-presentable string (e.g. "AA-BB-CC-DD-EE-FF")
    i8             identity[NAS_SUPPLICANT_ID_MAX_LENGTH];
    time_t         rel_creation_time_secs; // Uptime of switch in seconds when this SM was created.
    time_t         rel_auth_time_secs;     // Uptime of switch in seconds when it got authenticated (successfully as well as unsuccessfully).
} nas_client_info_t;

/****************************************************************************/
// If changing the enumeration, remember to also change dot1x.htm.
/****************************************************************************/
typedef enum {
    NAS_PORT_STATUS_LINK_DOWN      = 0, // Not used by the base lib itself
    NAS_PORT_STATUS_AUTHORIZED     = 1,
    NAS_PORT_STATUS_UNAUTHORIZED   = 2,
    NAS_PORT_STATUS_DISABLED       = 3, // Not used by the base lib itself
    NAS_PORT_STATUS_CNT            = 4, // Not used by the base lib itself
    NAS_PORT_STATUS_HIGHEST        = 0xFFFFFFFF, // Values >= 4 are used for MAC Count. Make sure this enum becomes a 4-byte thing.
} nas_port_status_t;

/******************************************************************************/
// NAS_PORT_CONTROL_IS_SINGLE_CLIENT()
// Useful "macro" to determine whether at most one client is allowed to be
// attached on a port.
/******************************************************************************/
static inline BOOL NAS_PORT_CONTROL_IS_SINGLE_CLIENT(nas_port_control_t admin_state)
{
    if (0
#ifdef VTSS_SW_OPTION_NAS_DOT1X_SINGLE
        || admin_state == NAS_PORT_CONTROL_DOT1X_SINGLE
#endif
#ifdef VTSS_SW_OPTION_DOT1X
        || admin_state == NAS_PORT_CONTROL_AUTO
#endif
       ) {
        return TRUE;
    }
    return FALSE;
}

/******************************************************************************/
// NAS_PORT_CONTROL_IS_MULTI_CLIENT()
// Useful "macro" to determine if a given admin state allows more than one
// MAC address on the port.
/******************************************************************************/
static inline BOOL NAS_PORT_CONTROL_IS_MULTI_CLIENT(nas_port_control_t admin_state)
{
    if (0
#ifdef VTSS_SW_OPTION_NAS_DOT1X_MULTI
        || admin_state == NAS_PORT_CONTROL_DOT1X_MULTI
#endif
#ifdef VTSS_SW_OPTION_NAS_MAC_BASED
        || admin_state == NAS_PORT_CONTROL_MAC_BASED
#endif
       ) {
        return TRUE;
    }
    return FALSE;
}

/******************************************************************************/
// NAS_PORT_CONTROL_IS_MAC_TABLE_BASED()
// Useful "macro" to determine if a given admin state requires the MAC table
// (i.e. the PSEC module) to store info about clients.
/******************************************************************************/
static inline BOOL NAS_PORT_CONTROL_IS_MAC_TABLE_BASED(nas_port_control_t admin_state)
{
    if (0
#ifdef VTSS_SW_OPTION_NAS_DOT1X_SINGLE
        || admin_state == NAS_PORT_CONTROL_DOT1X_SINGLE
#endif
#ifdef VTSS_SW_OPTION_NAS_DOT1X_MULTI
        || admin_state == NAS_PORT_CONTROL_DOT1X_MULTI
#endif
#ifdef VTSS_SW_OPTION_NAS_MAC_BASED
        || admin_state == NAS_PORT_CONTROL_MAC_BASED
#endif
       ) {
        return TRUE;
    }
    return FALSE;
}

/******************************************************************************/
// NAS_PORT_CONTROL_IS_BPDU_BASED()
// Useful "macro" to determine if a given admin state requires BPDUs or not.
/******************************************************************************/
static inline BOOL NAS_PORT_CONTROL_IS_BPDU_BASED(nas_port_control_t admin_state)
{
    if (0
#ifdef VTSS_SW_OPTION_DOT1X
        || admin_state == NAS_PORT_CONTROL_AUTO
#endif
#ifdef VTSS_SW_OPTION_NAS_DOT1X_SINGLE
        || admin_state == NAS_PORT_CONTROL_DOT1X_SINGLE
#endif
#ifdef VTSS_SW_OPTION_NAS_DOT1X_MULTI
        || admin_state == NAS_PORT_CONTROL_DOT1X_MULTI
#endif
       ) {
        return TRUE;
    }
    return FALSE;
}

/******************************************************************************/
// NAS_PORT_CONTROL_IS_MAC_TABLE_AND_BPDU_BASED()
// Useful "macro" to determine whether a given admin state requires BPDUs and
// is MAC-table based or not.
/******************************************************************************/
static inline BOOL NAS_PORT_CONTROL_IS_MAC_TABLE_AND_BPDU_BASED(nas_port_control_t admin_state)
{
    if (0
#ifdef VTSS_SW_OPTION_NAS_DOT1X_SINGLE
        || admin_state == NAS_PORT_CONTROL_DOT1X_SINGLE
#endif
#ifdef VTSS_SW_OPTION_NAS_DOT1X_MULTI
        || admin_state == NAS_PORT_CONTROL_DOT1X_MULTI
#endif
       ) {
        return TRUE;
    }
    return FALSE;
}

/******************************************************************************/
// NAS_PORT_CONTROL_IS_ACCOUNTABLE()
// Useful "macro" to determine whether a given admin state requires accounting
// or not.
/******************************************************************************/
static inline BOOL NAS_PORT_CONTROL_IS_ACCOUNTABLE(nas_port_control_t admin_state)
{
    if (0
#ifdef VTSS_SW_OPTION_DOT1X
        || admin_state == NAS_PORT_CONTROL_AUTO
#endif
#ifdef VTSS_SW_OPTION_NAS_DOT1X_SINGLE
        || admin_state == NAS_PORT_CONTROL_DOT1X_SINGLE
#endif
#ifdef VTSS_SW_OPTION_NAS_DOT1X_MULTI
        || admin_state == NAS_PORT_CONTROL_DOT1X_MULTI
#endif
#ifdef VTSS_SW_OPTION_NAS_MAC_BASED
        || admin_state == NAS_PORT_CONTROL_MAC_BASED
#endif
       ) {
        return TRUE;
    }
    return FALSE;
}

#endif /* _VTSS_NAS_API_H__ */
