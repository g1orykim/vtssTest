/*

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

#ifndef __VTSS_RADIUS_API_H__
#define __VTSS_RADIUS_API_H__

#include "main.h"          /* For MODULE_ERROR_START (and VTSS_MODULE_ID_RADIUS). */

/**
 * \brief RADIUS defines.
 */
#define VTSS_RADIUS_NUMBER_OF_SERVERS   5 /**< Number of supported RADIUS servers */
#define VTSS_RADIUS_HOST_LEN          256 /**< Maximum length for a hostname (incl. NULL) */
#define VTSS_RADIUS_KEY_LEN            64 /**< Maximum length for a key (incl. NULL) */

/**
 * \brief Maximum RADIUS frame length (protocol part, only)
 *
 * An EAPoL frame from a supplicant can hold 1514 bytes of which 18 bytes
 * are protocol ("over LAN") overhead => 1496 EAP message.
 *
 * Since the RADIUS protocol segregates information into TLVs
 * with a max size of 253 information bytes (two for Type and Len),
 * a 1496 byte EAPoL EAP will require 6 TLVs and therefore
 * 1496 + 2 * 6 = 1508 bytes.
 *
 * On top of this comes the following required fields in a RADIUS packet:
 *
 * RADIUS Protocol Overhead:
 *   Code:           1 byte
 *   Id:             1 byte
 *   Length:         2 bytes
 *   Authenticator: 16 bytes
 * -------------------------
 * Subtotal         20 bytes
 *
 * Mandatory TLVs added by RADIUS module:
 *   Framed-MTU to get the RADIUS server to limit its packet sizes:  6 bytes
 *   Message-Authenticator:                                         18 bytes
 * -------------------------------------------------------------------------
 * Subtotal                                                         24 bytes
 *
 * Mandatory TLVs added by NAS module:
 *   EAP-Message (as computed above):        1508 bytes
 *   State:                                    18 bytes
 *   User (NAS_SUPPLICANT_ID_MAX_LENGTH + 2)   42 bytes
 *   Accounting session ID                     10 bytes
 * ----------------------------------------------------
 * Subtotal                                  1578 bytes
 *
 *   20 bytes
 *   24 bytes
 * 1578 bytes
 * ----------
 * 1622 bytes.
 *
 * It is expected that the IP stack fragments the frames if larger than IP stack MTU.
 */
#define RADIUS_MAX_FRAME_SIZE_BYTES 1622

/**
 * \brief RADIUS codes indentifying the RADIUS packet type
 *
 * VTSS_RADIUS_CODE_ACCESS_* are specified in RFC2865.
 * VTSS_RADIUS_CODE_ACCOUNTING_* are specified in RFC2866.
 * If the code should one day support other types, refer to RFC3575 for a summary.
 *
 * Values not specified below are illegal or reserved.
 */
typedef enum {
    VTSS_RADIUS_CODE_ACCESS_REQUEST      =   1, /**< Sent to RADIUS server with info about user.                             */
    VTSS_RADIUS_CODE_ACCESS_ACCEPT       =   2, /**< Sent by RADIUS server if all attribute values were accepted.            */
    VTSS_RADIUS_CODE_ACCESS_REJECT       =   3, /**< Sent by RADIUS server if any attribute value is not accepted.           */
    VTSS_RADIUS_CODE_ACCOUNTING_REQUEST  =   4, /**< Sent to RADIUS accounting server with accounting information.           */
    VTSS_RADIUS_CODE_ACCOUNTING_RESPONSE =   5, /**< Sent by RADIUS accounting server if request was processed successfully. */
    VTSS_RADIUS_CODE_ACCESS_CHALLENGE    =  11, /**< Sent by RADIUS server if it needs more info.                            */
} vtss_radius_access_codes_e;

/**
 * \brief RADIUS attribute types used in RADIUS TLVs.
 *
 *   1- 39: Specified in RFC2865, RADIUS.
 *  40- 51: Specified in RFC2866, RADIUS Accounting.
 *  52- 55: Specified in RFC2869, RADIUS Extensions.
 *  56- 59: Specified in RFC4675, RADIUS Attributes for Virtual LAN and Priority Support.
 *  60- 63: Specified in RFC2865, RADIUS.
 *  64- 67: Specified in RFC2868, RADIUS, Tunnel Protocol.
 *      68: Specified in RFC2867, RADIUS Accounting, Tunnel Protocol.
 *      69: Specified in RFC2868, RADIUS, Tunnel Protocol.
 *  70- 80: Specified in RFC2869, RADIUS Extensions.
 *  81- 83: Specified in RFC2868, RADIUS, Tunnel Protocol.
 *  84- 85: Specified in RFC2869, RADIUS Extensions.
 *      86: Specified in RFC2867, RADIUS Accounting, Tunnel Protocol.
 *  87- 89: Specified in RFC2869, RADIUS Extensions.
 *  90- 91: Specified in RFC2868, RADIUS, Tunnel Protocol.
 *      92: Specified in RFC4849, RADIUS Filter Rule Attribute.
 *      93: Unassigned.
 *      94: Specified in RFC4005, Diameter Network Access Server Application.
 *  95-100: Specified in RFC3162, RADIUS and IPv6.
 *     101: Specified in RFC3576, Dynamic Authorization Extensions to RADIUS.
 *     102: Specified in RFC4072, Diameter EAP Application.
 * 103-122: Specified in RFC5090, RADIUS Extension for Digest Authentication.
 *     123: Specified in RFC4818, RADIUS Delegated-IPv6-Prefix Attribute.
 * 124-125: Specified in RFC5447, Diameter Mobile IPv6.
 * 126-132: Specified in draft-ietf-geopriv-radius-lo-24, Carrying Location Objects in RADIUS and Diameter.
 * 133-136: Specified in draft-ietf-radext-management-authorization-07, RADIUS Authorization for NAS Management.
 * 137-191: Unassigned.
 * 192-223: Experimental use as per RFC3575.
 * 224-240: Implementation specific as per RFC3575.
 * 241-255: Reserved as per RFC3575.
 *
 * For a list of up-to-date RADIUS type, see http://www.iana.org/assignments/radius-types.
 */
typedef enum {
    VTSS_RADIUS_ATTRIBUTE_USER_NAME                       =   1, /**< Name of user to be authenticated.                               */
    VTSS_RADIUS_ATTRIBUTE_USER_PASSWORD                   =   2,
    VTSS_RADIUS_ATTRIBUTE_CHAP_PASSWORD                   =   3,
    VTSS_RADIUS_ATTRIBUTE_NAS_IP_ADDRESS                  =   4, /**< IP address of switch.                                           */
    VTSS_RADIUS_ATTRIBUTE_NAS_PORT                        =   5, /**< Physical port number for user on switch.                        */
    VTSS_RADIUS_ATTRIBUTE_SERVICE_TYPE                    =   6,
    VTSS_RADIUS_ATTRIBUTE_FRAMED_PROTOCOL                 =   7,
    VTSS_RADIUS_ATTRIBUTE_FRAMED_IP_ADDRESS               =   8,
    VTSS_RADIUS_ATTRIBUTE_FRAMED_IP_NETMASK               =   9,
    VTSS_RADIUS_ATTRIBUTE_FRAMED_ROUTING                  =  10,
    VTSS_RADIUS_ATTRIBUTE_FILTER_ID                       =  11,
    VTSS_RADIUS_ATTRIBUTE_FRAMED_MTU                      =  12, /**< Ask server to limit size of packets to MTU.                     */
    VTSS_RADIUS_ATTRIBUTE_FRAMED_COMPRESSION              =  13,
    VTSS_RADIUS_ATTRIBUTE_LOGIN_IP_HOST                   =  14,
    VTSS_RADIUS_ATTRIBUTE_LOGIN_SERVICE                   =  15,
    VTSS_RADIUS_ATTRIBUTE_LOGIN_TCP_PORT                  =  16,
    VTSS_RADIUS_ATTRIBUTE_REPLY_MESSAGE                   =  18,
    VTSS_RADIUS_ATTRIBUTE_CALLBACK_NUMBER                 =  19,
    VTSS_RADIUS_ATTRIBUTE_CALLBACK_ID                     =  20,
    VTSS_RADIUS_ATTRIBUTE_FRAMED_ROUTE                    =  22,
    VTSS_RADIUS_ATTRIBUTE_FRAMED_IPX_NETWORK              =  23,
    VTSS_RADIUS_ATTRIBUTE_STATE                           =  24, /**< Sent by server to client, and unmodified in opp. dir.           */
    VTSS_RADIUS_ATTRIBUTE_CLASS                           =  25,
    VTSS_RADIUS_ATTRIBUTE_VENDOR_SPECIFIC                 =  26,
    VTSS_RADIUS_ATTRIBUTE_SESSION_TIMEOUT                 =  27,
    VTSS_RADIUS_ATTRIBUTE_IDLE_TIMEOUT                    =  28,
    VTSS_RADIUS_ATTRIBUTE_TERMINATION_ACTION              =  29,
    VTSS_RADIUS_ATTRIBUTE_CALLED_STATION_ID               =  30, /**< Stringified version of switch's MAC address.                    */
    VTSS_RADIUS_ATTRIBUTE_CALLING_STATION_ID              =  31, /**< Stringified MAC address of supplicant/client's MAC address.     */
    VTSS_RADIUS_ATTRIBUTE_NAS_IDENTIFIER                  =  32,
    VTSS_RADIUS_ATTRIBUTE_PROXY_STATE                     =  33,
    VTSS_RADIUS_ATTRIBUTE_LOGIN_LAT_SERVICE               =  34,
    VTSS_RADIUS_ATTRIBUTE_LOGIN_LAT_NODE                  =  35,
    VTSS_RADIUS_ATTRIBUTE_LOGIN_LAT_GROUP                 =  36,
    VTSS_RADIUS_ATTRIBUTE_FRAMED_APPLETALK_LINK           =  37,
    VTSS_RADIUS_ATTRIBUTE_FRAMED_APPLETALK_NETWORK        =  38,
    VTSS_RADIUS_ATTRIBUTE_FRAMED_APPLETALK_ZONE           =  39,
    VTSS_RADIUS_ATTRIBUTE_ACCT_STATUS_TYPE                =  40,
    VTSS_RADIUS_ATTRIBUTE_ACCT_DELAY_TIME                 =  41,
    VTSS_RADIUS_ATTRIBUTE_ACCT_INPUT_OCTETS               =  42,
    VTSS_RADIUS_ATTRIBUTE_ACCT_OUTPUT_OCTETS              =  43,
    VTSS_RADIUS_ATTRIBUTE_ACCT_SESSION_ID                 =  44,
    VTSS_RADIUS_ATTRIBUTE_ACCT_AUTHENTIC                  =  45,
    VTSS_RADIUS_ATTRIBUTE_ACCT_SESSION_TIME               =  46,
    VTSS_RADIUS_ATTRIBUTE_ACCT_INPUT_PACKETS              =  47,
    VTSS_RADIUS_ATTRIBUTE_ACCT_OUTPUT_PACKETS             =  48,
    VTSS_RADIUS_ATTRIBUTE_ACCT_TERMINATE_CAUSE            =  49,
    VTSS_RADIUS_ATTRIBUTE_ACCT_MULTI_SESSION_ID           =  50,
    VTSS_RADIUS_ATTRIBUTE_ACCT_LINK_COUNT                 =  51,
    VTSS_RADIUS_ATTRIBUTE_ACCT_INPUT_GIGAWORDS            =  52,
    VTSS_RADIUS_ATTRIBUTE_ACCT_OUTPUT_GIGAWORDS           =  53,
    VTSS_RADIUS_ATTRIBUTE_EVENT_TIMESTAMP                 =  55,
    VTSS_RADIUS_ATTRIBUTE_EGRESS_VLANID                   =  56,
    VTSS_RADIUS_ATTRIBUTE_INGRESS_FILTERS                 =  57,
    VTSS_RADIUS_ATTRIBUTE_EGRESS_VLAN_NAME                =  58,
    VTSS_RADIUS_ATTRIBUTE_USER_PRIORITY_TABLE             =  59,
    VTSS_RADIUS_ATTRIBUTE_CHAP_CHALLENGE                  =  60,
    VTSS_RADIUS_ATTRIBUTE_NAS_PORT_TYPE                   =  61, /**< We insert the value 15, which means 'Ethernet'.                 */
    VTSS_RADIUS_ATTRIBUTE_PORT_LIMIT                      =  62,
    VTSS_RADIUS_ATTRIBUTE_LOGIN_LAT_PORT                  =  63,
    VTSS_RADIUS_ATTRIBUTE_TUNNEL_TYPE                     =  64,
    VTSS_RADIUS_ATTRIBUTE_TUNNEL_MEDIUM_TYPE              =  65,
    VTSS_RADIUS_ATTRIBUTE_TUNNEL_CLIENT_ENDPOINT          =  66,
    VTSS_RADIUS_ATTRIBUTE_TUNNEL_SERVER_ENDPOINT          =  67,
    VTSS_RADIUS_ATTRIBUTE_TUNNEL_CONNECTION               =  68,
    VTSS_RADIUS_ATTRIBUTE_TUNNEL_PASSWORD                 =  69,
    VTSS_RADIUS_ATTRIBUTE_ARAP_PASSWORD                   =  70,
    VTSS_RADIUS_ATTRIBUTE_ARAP_FEATURES                   =  71,
    VTSS_RADIUS_ATTRIBUTE_ARAP_ZONE_ACCESS                =  72,
    VTSS_RADIUS_ATTRIBUTE_ARAP_SECURITY                   =  73,
    VTSS_RADIUS_ATTRIBUTE_ARAP_SECURITY_DATA              =  74,
    VTSS_RADIUS_ATTRIBUTE_PASSWORD_RETRY                  =  75,
    VTSS_RADIUS_ATTRIBUTE_PROMPT                          =  76,
    VTSS_RADIUS_ATTRIBUTE_CONNECT_INFO                    =  77,
    VTSS_RADIUS_ATTRIBUTE_CONFIGURATION_TOKEN             =  78,
    VTSS_RADIUS_ATTRIBUTE_EAP_MESSAGE                     =  79, /**< Embedded EAP message.                                           */
    VTSS_RADIUS_ATTRIBUTE_MESSAGE_AUTHENTICATOR           =  80, /**< Sign Access-Request packets w/ shared secret to avoid spoofing. */
    VTSS_RADIUS_ATTRIBUTE_TUNNEL_PRIVATE_GROUP_ID         =  81,
    VTSS_RADIUS_ATTRIBUTE_TUNNEL_ASSIGNMENT_ID            =  82,
    VTSS_RADIUS_ATTRIBUTE_TUNNEL_PREFERENCE               =  83,
    VTSS_RADIUS_ATTRIBUTE_ARAP_CHALLENGE_RESPONSE         =  84,
    VTSS_RADIUS_ATTRIBUTE_ACCT_INTERIM_INTERVAL           =  85,
    VTSS_RADIUS_ATTRIBUTE_ACCT_TUNNEL_PACKETS_LOST        =  86,
    VTSS_RADIUS_ATTRIBUTE_NAS_PORT_ID                     =  87, /**< String identifiying client port.                                */
    VTSS_RADIUS_ATTRIBUTE_FRAMED_POOL                     =  88,
    VTSS_RADIUS_ATTRIBUTE_TUNNEL_CLIENT_AUTH_ID           =  90,
    VTSS_RADIUS_ATTRIBUTE_TUNNEL_SERVER_AUTH_ID           =  91,
    VTSS_RADIUS_ATTRIBUTE_NAS_FILTER_RULE                 =  92,
    VTSS_RADIUS_ATTRIBUTE_ORIGINATING_LINE_INFO           =  94,
    VTSS_RADIUS_ATTRIBUTE_NAS_IPV6_ADDRESS                =  95,
    VTSS_RADIUS_ATTRIBUTE_FRAMED_INTERFACE_ID             =  96,
    VTSS_RADIUS_ATTRIBUTE_FRAMED_IPV6_PREFIX              =  97,
    VTSS_RADIUS_ATTRIBUTE_LOGIN_IPV6_HOST                 =  98,
    VTSS_RADIUS_ATTRIBUTE_FRAMED_IPV6_ROUTE               =  99,
    VTSS_RADIUS_ATTRIBUTE_FRAMED_IPV6_POOL                = 100,
    VTSS_RADIUS_ATTRIBUTE_ERROR_CAUSE                     = 101,
    VTSS_RADIUS_ATTRIBUTE_EAP_KEY_NAME                    = 102,
    VTSS_RADIUS_ATTRIBUTE_DIGEST_RESPONSE                 = 103,
    VTSS_RADIUS_ATTRIBUTE_DIGEST_REALM                    = 104,
    VTSS_RADIUS_ATTRIBUTE_DIGEST_NONCE                    = 105,
    VTSS_RADIUS_ATTRIBUTE_DIGEST_RESPONSE_AUTH            = 106,
    VTSS_RADIUS_ATTRIBUTE_DIGEST_NEXTNONCE                = 107,
    VTSS_RADIUS_ATTRIBUTE_DIGEST_METHOD                   = 108,
    VTSS_RADIUS_ATTRIBUTE_DIGEST_URI                      = 109,
    VTSS_RADIUS_ATTRIBUTE_DIGEST_QOP                      = 110,
    VTSS_RADIUS_ATTRIBUTE_DIGEST_ALGORITHM                = 111,
    VTSS_RADIUS_ATTRIBUTE_DIGEST_ENTITY_BODY_HASH         = 112,
    VTSS_RADIUS_ATTRIBUTE_DIGEST_CNONCE                   = 113,
    VTSS_RADIUS_ATTRIBUTE_DIGEST_NONCE_COUNT              = 114,
    VTSS_RADIUS_ATTRIBUTE_DIGEST_USERNAME                 = 115,
    VTSS_RADIUS_ATTRIBUTE_DIGEST_OPAQUE                   = 116,
    VTSS_RADIUS_ATTRIBUTE_DIGEST_AUTH_PARAM               = 117,
    VTSS_RADIUS_ATTRIBUTE_DIGEST_AKA_AUTS                 = 118,
    VTSS_RADIUS_ATTRIBUTE_DIGEST_DOMAIN                   = 119,
    VTSS_RADIUS_ATTRIBUTE_DIGEST_STALE                    = 120,
    VTSS_RADIUS_ATTRIBUTE_DIGEST_HA1                      = 121,
    VTSS_RADIUS_ATTRIBUTE_SIP_AOR                         = 122,
    VTSS_RADIUS_ATTRIBUTE_DELEGATED_IPV6_PREFIX           = 123,
    VTSS_RADIUS_ATTRIBUTE_MIP6_FEATURE_VECTOR             = 124,
    VTSS_RADIUS_ATTRIBUTE_MIP6_HOME_LINK_PREFIX           = 125,
    VTSS_RADIUS_ATTRIBUTE_OPERATOR_NAME                   = 126,
    VTSS_RADIUS_ATTRIBUTE_LOCATION_INFORMATION            = 127,
    VTSS_RADIUS_ATTRIBUTE_LOCATION_DATA                   = 128,
    VTSS_RADIUS_ATTRIBUTE_BASIC_LOCATION_POLICY_RULES     = 129,
    VTSS_RADIUS_ATTRIBUTE_EXTENDED_LOCATION_POLICY_RULES  = 130,
    VTSS_RADIUS_ATTRIBUTE_LOCATION_CAPABLE                = 131,
    VTSS_RADIUS_ATTRIBUTE_REQUESTED_LOCATION_INFO         = 132,
    VTSS_RADIUS_ATTRIBUTE_FRAMED_MANAGEMENT_PROTOCOL      = 133,
    VTSS_RADIUS_ATTRIBUTE_MANAGEMENT_TRANSPORT_PROTECTION = 134,
    VTSS_RADIUS_ATTRIBUTE_MANAGEMENT_POLICY_ID            = 135,
    VTSS_RADIUS_ATTRIBUTE_MANAGEMENT_PRIVILEGE_LEVEL      = 136,
} vtss_radius_attributes_e;

/**
 * \brief RADIUS accounting termination causes
 *
 * Used in VTSS_RADIUS_ATTRIBUTE_ACCT_TERMINATE_CAUSE TLVs
 */
enum {
    VTSS_RADIUS_ACCT_TERMINATE_CAUSE_USER_REQUEST         =  1,
    VTSS_RADIUS_ACCT_TERMINATE_CAUSE_LOST_CARRIER         =  2,
    VTSS_RADIUS_ACCT_TERMINATE_CAUSE_LOST_SERVICE         =  3,
    VTSS_RADIUS_ACCT_TERMINATE_CAUSE_ACCT_IDLE_TIMEOUT    =  4,
    VTSS_RADIUS_ACCT_TERMINATE_CAUSE_ACCT_SESSION_TIMEOUT =  5,
    VTSS_RADIUS_ACCT_TERMINATE_CAUSE_ADMIN_RESET          =  6,
    VTSS_RADIUS_ACCT_TERMINATE_CAUSE_ADMIN_REBOOT         =  7,
    VTSS_RADIUS_ACCT_TERMINATE_CAUSE_PORT_ERROR           =  8,
    VTSS_RADIUS_ACCT_TERMINATE_CAUSE_NAS_ERROR            =  9,
    VTSS_RADIUS_ACCT_TERMINATE_CAUSE_NAS_REQUEST          = 10,
    VTSS_RADIUS_ACCT_TERMINATE_CAUSE_NAS_REBOOT           = 11,
    VTSS_RADIUS_ACCT_TERMINATE_CAUSE_PORT_UNNEEDED        = 12,
    VTSS_RADIUS_ACCT_TERMINATE_CAUSE_PORT_PREEMPTED       = 13,
    VTSS_RADIUS_ACCT_TERMINATE_CAUSE_PORT_SUSPENDED       = 14,
    VTSS_RADIUS_ACCT_TERMINATE_CAUSE_SERVICE_UNAVAILABLE  = 15,
    VTSS_RADIUS_ACCT_TERMINATE_CAUSE_CALLBACK             = 16,
    VTSS_RADIUS_ACCT_TERMINATE_CAUSE_USER_ERROR           = 17,
    VTSS_RADIUS_ACCT_TERMINATE_CAUSE_HOST_REQUEST         = 18,
};

/**
 * \brief The state that a server can be in.
 */
typedef enum {
    VTSS_RADIUS_SERVER_STATE_DISABLED,  /**< The server is not enabled. */
    VTSS_RADIUS_SERVER_STATE_NOT_READY, /**< The server is enabled, but IP communication not yet ready */
    VTSS_RADIUS_SERVER_STATE_READY,     /**< The server is enabled, and ready */
    VTSS_RADIUS_SERVER_STATE_DEAD,      /**< The server is enabled, but didn't reply to latest access request */
} vtss_radius_server_state_e;

/**
 * \brief Counters for implementing RFC4668 RADIUS Authentication Client MIB for a given server
 *
 * May be used to implement RFC4668, which obsoletes RFC2618.
 */
typedef struct {
    vtss_radius_server_state_e state;                /**< Indicates the state of this server, i.e., whether it's disabled, active or dead. */
    u32 dead_time_left_secs;                         /**< Number of seconds left of this server's dead time. Valid if @state = VTSS_RADIUS_SERVER_STATE_DEAD */
    u32 radiusAuthServerInetAddress;                 /**< The IP address of the RADIUS authentication server. */
    u16 radiusAuthClientServerInetPortNumber;        /**< The UDP port the client is using to send requests to this server. The value of zero (0) is invalid. */
    u32 radiusAuthClientExtRoundTripTime;            /**< The time interval (in hundredths of a second) between the most recent Access-Reply/Access-Challenge and the Access-Request that matched it from the RADIUS authentication server. */
    u32 radiusAuthClientExtAccessRequests;           /**< The number of RADIUS Access-Request packets sent to this server. This does not include retransmissions. */
    u32 radiusAuthClientExtAccessRetransmissions;    /**< The number of RADIUS Access-Request packets retransmitted to this RADIUS authentication server. */
    u32 radiusAuthClientExtAccessAccepts;            /**< The number of RADIUS Access-Accept packets (valid or invalid) received from this server. */
    u32 radiusAuthClientExtAccessRejects;            /**< The number of RADIUS Access-Reject packets (valid or invalid) received from this server. */
    u32 radiusAuthClientExtAccessChallenges;         /**< The number of RADIUS Access-Challenge packets (valid or invalid) received from this server. */
    u32 radiusAuthClientExtMalformedAccessResponses; /**< The number of malformed RADIUS Access-Response packets received from this server. */
    u32 radiusAuthClientExtBadAuthenticators;        /**< The number of RADIUS Access-Response packets containing invalid authenticators or Message Authenticator attributes received from this server. */
    u32 radiusAuthClientExtPendingRequests;          /**< The number of RADIUS Access-Request packets destined for this server that have not yet timed out. This variable is incremented when an Access-Request is sent and decremented due to receipt of an Access-Accept, Access-Reject, Access-Challenge, timeout, or retransmission. */
    u32 radiusAuthClientExtTimeouts;                 /**< The number of authentication timeouts to this server. Retries are counted as timeouts as well. */
    u32 radiusAuthClientExtUnknownTypes;             /**< The number of RADIUS packets of unknown type that were received from this server on the authentication port. */
    u32 radiusAuthClientExtPacketsDropped;           /**< The number of RADIUS packets that were received from this server on the authentication port and dropped for some other reason. */
    u32 radiusAuthClientCounterDiscontinuity;        /**< The number of centiseconds since the last discontinuity in the RADIUS Authentication Client counters. */
} vtss_radius_auth_client_server_mib_s;

/**
 * \brief Counters for implementing RFC4668 RADIUS Authentication Client MIB
 *
 * May be used to implement RFC4668, which obsoletes RFC2618.
 */
typedef struct {
    u32                                  radiusAuthClientInvalidServerAddresses; /**< Number of RADIUS Access-Response packets received from unknown addresses. */
    vtss_radius_auth_client_server_mib_s radiusAuthServerExtTable[VTSS_RADIUS_NUMBER_OF_SERVERS]; /**< Table of servers. */
} vtss_radius_auth_client_mib_s;

/**
 * \brief Counters for implementing RFC4670 RADIUS Accounting Client MIB for a given server
 *
 * May be used to implement RFC4670, which obsoletes RFC2620.
 */
typedef struct {
    vtss_radius_server_state_e state;         /**< Indicates the state of this server, i.e., whether it's disabled, active or dead. */
    u32 dead_time_left_secs;                  /**< Number of seconds left of this server's dead time. Valid if @state = VTSS_RADIUS_SERVER_STATE_DEAD */
    u32 radiusAccServerInetAddress;           /**< The IP address of the RADIUS accounting server. */
    u16 radiusAccClientServerInetPortNumber;  /**< The UDP port the client is using to send requests to this accounting server. The value of zero (0) is invalid. */
    u32 radiusAccClientExtRoundTripTime;      /**< The time interval between the most recent Accounting-Response and the Accounting-Request that matched it from the RADIUS accounting server [TimeTicks]. */
    u32 radiusAccClientExtRequests;           /**< The number of RADIUS Accounting-Request packets sent. This does not include retransmissions. */
    u32 radiusAccClientExtRetransmissions;    /**< The number of RADIUS Accounting-Request packets retransmitted to the RADIUS accounting server. */
    u32 radiusAccClientExtResponses;          /**< The number of RADIUS packets received on the accounting port from this server. */
    u32 radiusAccClientExtMalformedResponses; /**< The number of malformed RADIUS Accounting-Response packets received from this server. */
    u32 radiusAccClientExtBadAuthenticators;  /**< The number of RADIUS Accounting-Response packets that contained invalid authenticators received from this server. */
    u32 radiusAccClientExtPendingRequests;    /**< The number of RADIUS Accounting-Request packets sent to this server that have not yet timed out or received a response. */
    u32 radiusAccClientExtTimeouts;           /**< The number of accounting timeouts to this server. Retries are counted as timeouts as well. */
    u32 radiusAccClientExtUnknownTypes;       /**< The number of RADIUS packets of unknown type that were received from this server on the accounting port. */
    u32 radiusAccClientExtPacketsDropped;     /**< The number of RADIUS packets that were received from this server on the accounting port and dropped for some other reason. */
    u32 radiusAccClientCounterDiscontinuity;  /**< The number of centiseconds since the last discontinuity in the RADIUS Accounting Client counters. */
} vtss_radius_acct_client_server_mib_s;

/**
 * \brief Counters for implementing RFC4670 RADIUS Accounting Client MIB
 *
 * May be used to implement RFC4670, which obsoletes RFC2620.
 */
typedef struct {
    u32                                  radiusAccClientInvalidServerAddresses; /**< Number of RADIUS Accounting-Response packets received from unknown addresses. */
    u32                                  radiusAccServerExtTableCnt;            /**< Number of valid servers in the array below. */
    vtss_radius_acct_client_server_mib_s radiusAccServerExtTable[VTSS_RADIUS_NUMBER_OF_SERVERS]; /**< Table of servers. */
} vtss_radius_acct_client_mib_s;

/**
 * \brief Status of a single server.
 */
typedef struct {
    u32                        ip_addr;              /**< The IP address of this server */
    u16                        port;                 /**< The UDP port number of this server */
    vtss_radius_server_state_e state;                /**< Indicates the state of this server, i.e., whether it's disabled, active, or dead. */
    u32                        dead_time_left_secs;  /**< Number of seconds left of this server's dead time. Valid if @state = VTSS_RADIUS_SERVER_STATE_DEAD */
} vtss_radius_server_status_s;

/**
 * \brief Status of all servers.
 */
typedef struct {
    vtss_radius_server_status_s status[VTSS_RADIUS_NUMBER_OF_SERVERS];
} vtss_radius_all_server_status_s;

/**
 * \brief Value to put in NAS-Port-Type attribute.
 */
#define VTSS_RADIUS_NAS_PORT_TYPE_ETHERNET 15

/**
 * \brief Results codes returned in the @res parameter to the rx callback.
 */
typedef enum {
    VTSS_RADIUS_RX_CALLBACK_OK                 = 0, /**< RADIUS reply received successfully.         */
    VTSS_RADIUS_RX_CALLBACK_TIMEOUT            = 1, /**< No reply from either server within timeout. */
    VTSS_RADIUS_RX_CALLBACK_CFG_CHANGED        = 2, /**< RADIUS configuration has changed.           */
    VTSS_RADIUS_RX_CALLBACK_MAX_ID_CNT_CHANGED = 3, /**< Max. number of IDs has changed.      */
    VTSS_RADIUS_RX_CALLBACK_MASTER_DOWN        = 4, /**< We are no longer master.                    */
} vtss_radius_rx_callback_result_e;

/**
 * \brief Results codes returned in the @res parameter to the RADIUS module init/deinit callback.
 */
typedef enum {
    VTSS_RADIUS_INIT_CHG_CALLBACK_UNINITIALIZED  = 0, /**< RADIUS module is not ready to serve. */
    VTSS_RADIUS_INIT_CHG_CALLBACK_INITIALIZED    = 1, /**< RADIUS module is ready to serve.     */
} vtss_radius_init_chg_callback_result_e;

/**
 * \brief Results codes returned in the @res parameter to the RADIUS module ID state change callback.
 */
typedef enum {
    VTSS_RADIUS_ID_STATE_CHG_CALLBACK_OUT_OF_IDS = 0, /**< RADIUS module is out of IDs (handles).   */
    VTSS_RADIUS_ID_STATE_CHG_CALLBACK_READY      = 1, /**< RADIUS module is ready to serve (again). */
} vtss_radius_id_state_chg_callback_result_e;

/**
 * \brief Return codes used by this module.
 */
enum {
    VTSS_RADIUS_ERROR_PARAM = MODULE_ERROR_START(VTSS_MODULE_ID_RADIUS), /**< Invalid parameter (e.g. function called with NULL-pointer). */
    VTSS_RADIUS_ERROR_MUST_BE_MASTER,                                    /**< The API functions don't work when we aren't master. */
    VTSS_RADIUS_ERROR_NOT_CONFIGURED,                                    /**< Until the RADIUS module is configured, this API function must not be called. */
    VTSS_RADIUS_ERROR_NOT_INITIALIZED,                                   /**< Until the RADIUS module is configured and initialized, this API function must not be called. See vtss_radius_init_chg_callback_register(). */
    VTSS_RADIUS_ERROR_NO_NAME,                                           /**< Unable to resolve hostname via DNS. */
    VTSS_RADIUS_ERROR_CFG_TIMEOUT,                                       /**< Invalid timeout_secs configuration parameter. */
    VTSS_RADIUS_ERROR_CFG_DEAD_TIME,                                     /**< Invalid dead-time configuration parameter. */
    VTSS_RADIUS_ERROR_CFG_IP,                                            /**< Invalid IP address was detected in configuration parameter. */
    VTSS_RADIUS_ERROR_CFG_SECRET,                                        /**< Shared secret is not NULL-terminated. */
    VTSS_RADIUS_ERROR_CFG_SAME_SERVER_AND_PORT,                          /**< Same server and UDP port number is configured twice. */
    VTSS_RADIUS_ERROR_CALLBACK,                                          /**< Invalid callback function specified. */
    VTSS_RADIUS_ERROR_OUT_OF_REGISTRANT_ENTRIES,                         /**< No more vacant entries in the registrants array. Increase RADIUS_INIT_CHG_REGISTRANT_MAX_CNT. */
    VTSS_RADIUS_ERROR_INVALID_HANDLE,                                    /**< The @handle parameter supplied to the function is invalid. */
    VTSS_RADIUS_ERROR_ATTRIB_UNSUPPORTED,                                /**< The RADIUS module doesn't support this attribute (currently this is VTSS_RADIUS_ATTRIBUTE_CHAP_PASSWORD). */
    VTSS_RADIUS_ERROR_ATTRIB_AUTO_APPENDED,                              /**< The RADIUS module will append this attribute itself if needed. */
    VTSS_RADIUS_ERROR_ATTRIB_PASSWORD_TOO_LONG,                          /**< The RADIUS module doesn't support User-Passwords longer than VTSS_SYS_PASSWD_LEN. */
    VTSS_RADIUS_ERROR_ATTRIB_NAS_IDENTIFIER_TOO_LONG,                    /**< The NAS_IDENTIFIER must be able to fit into one TLV (<= 253 bytes). */
    VTSS_RADIUS_ERROR_ATTRIB_MISSING_ACCT_STATUS_TYPE,                   /**< Missing Acct-Status-Type attribute.  */
    VTSS_RADIUS_ERROR_ATTRIB_MISSING_ACCT_SESSION_ID,                    /**< Missing Acct-Session-Id attribute.  */
    VTSS_RADIUS_ERROR_ATTRIB_BOTH_USER_PW_AND_EAP_MSG_FOUND,             /**< Both a User-Password and EAP-Message attribute was found in the frame to tx. This makes it hard to compute an MD5. */
    VTSS_RADIUS_ERROR_ATTRIB_NEITHER_USER_PW_NOR_EAP_MSG_FOUND,          /**< Neither a User-Password nor an EAP-Message attribute was found in the frame to tx. This makes it hard to compute an MD5. */
    VTSS_RADIUS_ERROR_NO_MORE_TLVS,                                      /**< Returned by vtss_radius_tlv_iterate() when there are no more TLVs in the received frame. */
    VTSS_RADIUS_ERROR_NOT_ROOM_FOR_TLV,                                  /**< Returned by vtss_radius_tlv_set() when the TLV is too larget to fit into the pre-allocated Tx frame, or by vtss_radius_tlv_get() when there's not room in the supplied array. */
    VTSS_RADIUS_ERROR_TLV_NOT_FOUND,                                     /**< Returned by vtss_radius_tlv_get() when the requested TLV was not found in the frame. */
    VTSS_RADIUS_ERROR_OUT_OF_HANDLES,                                    /**< Returned by vtss_radius_alloc() when no more handles are available. Try again later. */
}; // Due to lint, we make this an anonymous/tagless enum. Original name was 'vtss_radius_error_e'

/**
 * \brief Function prototype for the callback made by this module as result of vtss_radius_tx()
 *
 * \param handle [IN] The handle that this RADIUS frame was received on.
 * \param ctx    [IN] Context that the user specified in the vtss_radius_tx() call.
 * \param code   [IN] The RADIUS access code from the recevied frame. Only valid if rc == VTSS_OK.
 * \param res    [IN] Return code used to indicate why the callback is called.
 */
typedef void (vtss_radius_rx_callback_f)(u8 handle, void *ctx, vtss_radius_access_codes_e code, vtss_radius_rx_callback_result_e res);

/**
 * \brief Function prototype for the callback made when the RADIUS module is either initialized or uninitialized.
 * The callback function is called from the RADIUS thread and the RADIUS semaphore is taken, so you must not
 * call other RADIUS API functions from the callback.
 *
 * \param ctx    [IN] Context that the user specified in the vtss_radius_init_chg_callback_register() call.
 * \param res    [IN] Return code used to indicate why the callback is called.
 */
typedef void (vtss_radius_init_chg_callback_f)(void *ctx, vtss_radius_init_chg_callback_result_e res);
vtss_rc vtss_radius_init_chg_callback_register(void **register_id, void *ctx, vtss_radius_init_chg_callback_f *cb);
vtss_rc vtss_radius_init_chg_callback_unregister(void *register_id);

/**
 * \brief Function prototype for the callback made when the RADIUS module runs out of RADIUS IDs (handles) or again is able to serve.
 * The callback function is called from the RADIUS thread and the RADIUS semaphore is taken, so you must not
 * call other RADIUS API functions from the callback.
 *
 * \param ctx    [IN] Context that the user specified in the vtss_radius_id_state_chg_callback_register() call.
 * \param res    [IN] Return code used to indicate why the callback is called.
 */
typedef void (vtss_radius_id_state_chg_callback_f)(void *ctx, vtss_radius_id_state_chg_callback_result_e res);
vtss_rc vtss_radius_id_state_chg_callback_register(void **register_id, void *ctx, vtss_radius_id_state_chg_callback_f *cb);
vtss_rc vtss_radius_id_state_chg_callback_unregister(void *register_id);

/**
 * \brief Get the ready-state of the RADIUS autentication module.
 * If the RADIUS module is not yet initialized or if all authenticaton servers are currently dead,
 * this function returns FALSE, otherwise it returns TRUE.
 * It is not guaranteed that a subsequent call to vtss_radius_tx() succeeds, since
 * other modules may cause the authenticaton servers to be considered dead.
 *
 * \return TRUE if a subsequent call to vtss_radius_tx() is likely to succeed, FALSE otherwise.
 */
BOOL vtss_radius_auth_ready(void);

/**
 * \brief Get the ready-state of the RADIUS accounting module.
 * If the RADIUS module is not yet initialized or if all accounting servers are currently dead,
 * this function returns FALSE, otherwise it returns TRUE.
 * It is not guaranteed that a subsequent call to vtss_radius_tx() succeeds, since
 * other modules may cause the accounting servers to be considered dead.
 *
 * \return TRUE if a subsequent call to vtss_radius_tx() is likely to succeed, FALSE otherwise.
 */
BOOL vtss_radius_acct_ready(void);

/**
 * \brief Allocate a RADIUS ID and frame.
 * Use this function to allocate a RADIUS ID and corresponding tx frame. Use the returned handle
 * afterwards to fill the frame with RADIUS TLVs with vtss_radius_tlv_set().
 * Use vtss_radius_tx() to transmit the frame once populated.
 * If at least one VTSS_RADIUS_ATTRIBUTE_EAP_MESSAGE was added to the frame,
 * the Request-Authenticator field will be filled with random data, and a
 * VTSS_RADIUS_ATTRIBUTE_MESSAGE_AUTHENTICATOR attribute will be added automatically.
 * Otherwise, the Request-Authenticator field will be an MD5-hash using the server's
 * secret.
 * The callback specified in the call to vtss_radius_tx() will *always* be called
 * no matter what. The callback's @res parameter tells how the request/response
 * dialog went. If it went OK, you may call the vtss_radius_tlv_get() function
 * to iterate across the TLVs in the RADIUS response packet. The callback will
 * be called from the RADIUS thread, and the only RADIUS API function, you're
 * allowed to call in the callback is the vtss_radius_tlv_get(). Once the callback
 * returns, the @handle is no longer valid.
 * In case of a switch in master, all callbacks are invoked with an error code, and
 * again, the handle will be auto-freed on return. It's up to the caller to implement
 * a semaphore that he signals from the callback to wake up his own thread, if he
 * wants his own thread to be blocking while a RADIUS tx/rx is in progress.
 *
 * \param handle [OUT] Receives a unique value that must be used\
 *                     in subsequent calls to this module.
 * \param code [IN] Indicates what type of RADIUS frame this is. Valid values\
 *                  are VTSS_RADIUS_CODE_ACCESS_REQUEST and\
 *                  VTSS_RADIUS_CODE_ACCOUNTING_REQUEST.
 *
 * \return VTSS_RADIUS_ERROR_PARAM: Invalid parameter supplied.\
 *         VTSS_RADIUS_ERROR_NOT_INITIALIZED: The IP-stack is not yet up.\
 *         VTSS_RADIUS_ERROR_OUT_OF_HANDLES: No more RADIUS Identifiers.\
 *         VTSS_OK: Success. @handle is now valid.
 */
vtss_rc vtss_radius_alloc(u8 *handle, vtss_radius_access_codes_e code);

/**
 * \brief Get a TLV from a received frame.
 * This function can be used to iterate through all TLVs of a received frame.
 * This function must *ONLY* be called from within the Rx callback function,
 * and only when the callback's @res parameter is VTSS_RADIUS_RX_CALLBACK_OK.
 * VTSS_RADIUS_ATTRIBUTE_MESSAGE_AUTHENTICATOR attributes will be filtered and not
 * returned, since this is in fact an internal RADIUS protocol attribute.
 * See also vtss_radius_tlv_get().
 *
 * This function may only be called from the within the callback function,
 * which is called from the RADIUS thread.
 *
 * \param handle     [IN]  Handle previously allocated with vtss_radius_alloc().
 * \param type       [OUT] Type-part of the TLV. One of the vtss_radius_attributes_e.
 * \param len        [OUT] Length-part of the TLV. Number of chars that are valid in @val. Does not include type and len fields.
 * \param val        [OUT] Value-part of the TLV. Number of valid chars is given by @len.
 * \param start_over [IN]  If TRUE, the iteration will start with the first TLV in the received RADIUS packet
 *                         Otherwise it'll start where the last call left off. If iteration
 *                         is only done once, you may set this to FALSE in all calls, since it'll
 *                         be reset correctly the very first time.
 *
 * \return VTSS_OK if the params are OK and the next TLV is OK,\
 *         VTSS_RADIUS_ERROR_NO_MORE_TLVS when last TLV is processed,\
 *         any other of the vtss_radius_error_e codes on failure.
 */
vtss_rc vtss_radius_tlv_iterate(u8 handle, vtss_radius_attributes_e *type, u8 *len, u8 const **val, BOOL start_over);

/**
 * \brief Get a TLV from a received frame.
 * This function can be used to get a specific TLV from a received frame.
 * This function must *ONLY* be called from within the Rx callback function,
 * and only when the callback's @res parameter is VTSS_RADIUS_RX_CALLBACK_OK.
 * The @len parameter is an INOUT. It's input value specifies the maximum
 * size to be copied into @val. On return, it contains the actual length
 * copied into @val.
 * All TLVs of the specified type found in the RADIUS frame are concatenated
 * into @val if there's room.
 *
 * \param handle [IN]    Handle previously allocated with vtss_radius_alloc().
 * \param type   [IN]    Type-part of the TLV. One of the vtss_radius_attributes_e.
 * \param len    [INOUT] Length-part of the TLV. Number of chars that are valid in @val. Does not include type and len fields.
 * \param val    [OUT]   Value-part of the TLV. User-allocated array at least @len chars long. All found TLVs are copied and concatenated into this array.
 *
 * \return VTSS_RADIUS_ERROR_PARAM: A supplied parameter is invalid.\
 *         VTSS_RADIUS_ERROR_INVALID_HANDLE: The handle is invalid.\
 *         VTSS_RADIUS_ERROR_NOT_ROOM_FOR_TLV: All found TLVs concatenated are longer than the supplied @len.
 *         VTSS_RADIUS_ERROR_TLV_NOT_FOUND: The TLV given by @type was not found in the RADIUS packet.
 *         VTSS_OK if the params are OK and the next TLV is OK,\
 */
vtss_rc vtss_radius_tlv_get(u8 handle, vtss_radius_attributes_e type, u16 *len, u8 *val);

/**
 * \brief Add a TLV to a RADIUS frame.
 * Use this function to add a TLV to a previously allocated
 * RADIUS frame, given by @handle.
 *
 * \param handle  [IN] Handle previously allocated with vtss_radius_alloc().
 * \param type    [IN] Type-part of the TLV. One of the vtss_radius_attributes_e.
 * \param len     [IN] Length-part of the TLV. Number of chars that are valid in @val. Does not include type and len fields. Multiple TLVs will be added if @len > 253.
 * \param val     [IN] Value-part of the TLV. Number of chars to copy from this to the frame is given by @len.
 * \param dealloc [IN] If there's not room for this TLV, then if @dealloc is TRUE, the @handle will be silently deallocated, otherwise it remains valid. Useful when adding optional attributes.
 *
 * \return VTSS_OK: if @len == 0 (TLV not added) or TLV successfully added.
 *         VTSS_RADIUS_ERROR_PARAM if @val == NULL (when @len != 0).
 *         VTSS_RADIUS_ERROR_INVALID_HANDLE if @handle is not allocated.
 *         VTSS_RADIUS_ATTRIB_UNSUPPORTED if @type is VTSS_RADIUS_ATTRIBUTE_CHAP_PASSWORD
 *         VTSS_RADIUS_ATTRIB_AUTO_APPENDED if @type is VTSS_RADIUS_ATTRIBUTE_NAS_IP_ADDRESS, VTSS_RADIUS_ATTRIBUTE_NAS_IDENTIFIER, VTSS_RADIUS_ATTRIBUTE_NAS_IPV6_ADDRESS,
 *                                                      VTSS_RADIUS_ATTRIBUTE_FRAMED_MTU or VTSS_RADIUS_ATTRIBUTE_MESSAGE_AUTHENTICATOR
 *         VTSS_RADIUS_ERROR_ATTRIB_PASSWORD_TOO_LONG if @type is VTSS_RADIUS_ATTRIBUTE_USER_PASSWORD and strlen(val) >= VTSS_SYS_PASSWD_LEN
 *         VTSS_RADIUS_ERROR_NOT_ROOM_FOR_TLV if there's not room for the TLV. If @dealloc is TRUE, then the handle is deallocated, otherwise it remains allocated.
 * The reason that the handle can only be deallocated on the VTSS_RADIUS_ERROR_NOT_ROOM_FOR_TLV is that the other error codes are due to coding errors. Only
 * the specified error code is a valid run-time error code.
 */
vtss_rc vtss_radius_tlv_set(u8 handle, vtss_radius_attributes_e type, u16 len, const u8 *val, BOOL dealloc);

/**
 * \brief Transmit the RADIUS frame given by @handle, and callback @cb when reply received.
 * This function will also create the Request-Authenticator field and possibly add
 * Message-Authenticator if at least one EAP-Message TLV was added.
 *
 *   If this function returns VTSS_OK, the frame will be transmitted and the callback function
 *   will be called from the RADIUS thread, unless the callback's @res parameter is
 *   VTSS_RADIUS_RX_CALLBACK_MASTER_DOWN, in which case it will be called from the msg module's
 *   "init modules" thread.
 *
 *   If this function returns any of the error codes outlined below, the frame will not be
 *   transmitted and the handle will be automatically deallocated (except for VTSS_RADIUS_ERROR_PARAM).
 *
 *   If this function returns VTSS_RADIUS_ERROR_PARAM, the specified callback function is NULL.
 *
 *   If this function returns VTSS_RADIUS_ERROR_INVALID_HANDLE, something is wrong with
 *   your code.
 *
 *   If this function returns VTSS_RADIUS_ERROR_MUST_BE_MASTER or VTSS_RADIUS_ERROR_NOT_INITIALIZED.
 *
 *   For authentication only:
 *   If this function returns VTSS_RADIUS_ERROR_ATTRIB_BOTH_USER_PW_AND_EAP_MSG_FOUND or
 *   VTSS_RADIUS_ERROR_ATTRIB_NEITHER_USER_PW_NOR_EAP_MSG_FOUND, there's a bug in your code, since
 *   you haven't added either an EAP-Message or User-Password attribute.
 *
 *   For accounting only:
 *   If this function returns VTSS_RADIUS_ERROR_ATTRIB_MISSING_ACCT_STATUS_TYPE or
 *   VTSS_RADIUS_ERROR_ATTRIB_MISSING_ACCT_SESSION_ID, there's a bug in your code, since
 *   you haven't added an Acct-Status-Type or Acct-Session-Id attribute.
 *
 * \param handle [IN] Handle previously allocated with vtss_radius_alloc() identifying the frame to transmit.
 * \param ctx    [IN] User-defined value passed to the callback.
 * \param cb     [IN] Function to call back when a reply is received, or timeout or other errors have occurred.
 *
 * \return VTSS_OK if the RADIUS frame was successfully transmitted. In this case the @cb will be called upon
 *         a RADIUS reply or timeout. If this function returns anything else but VTSS_OK, the callback
 *         function will not be called.
 */
vtss_rc vtss_radius_tx(u8 handle, void *ctx, vtss_radius_rx_callback_f callback);


/**
 * \brief Release a previously allocated handle.
 * This function is useful if some state-machine cannot wait for a RADIUS response to arrive or a
 * RADIUS timeout to occur before that state-machine transitions to another state.
 *
 * \param handle [IN] Handle previously allocated with vtss_radius_alloc().
 *
 * \return VTSS_OK if the handle was successfully released.
 */
vtss_rc vtss_radius_free(u8 handle);


/******************************************************************************/
// Configuration functions
/******************************************************************************/

/**
 * \brief Called upon boot and stack-changes.
 *
 * \param cmd [IN] Either of the INIT_CMD_* commands.
 * \param p1 [IN] isid that this init call pertains to.
 * \param p2 [IN] Reserved for future implementations.
 *
 * \return VTSS_OK on success, or any of the vtss_radius_error_e codes on failure.
 */
vtss_rc vtss_radius_init(vtss_init_data_t *data);

/**
 * \brief RADIUS server info
 */
typedef struct {
    char   host[VTSS_RADIUS_HOST_LEN]; /* IPv4, IPv6 or hostname of this server. Entry not used if zero. */
    ushort port;                       /* Port number to use on this server. */
    u32    timeout;                    /* Seconds to wait for a response from this server. */
    u32    retransmit;                 /* Number of times a request is resent to an unresponding server. */
    char   key[VTSS_RADIUS_KEY_LEN];   /* The secret to use on this server */
} vtss_radius_server_info_t;

/**
 * \brief Configuration data of the RADIUS module
 */
typedef struct {
    vtss_radius_server_info_t servers_auth[VTSS_RADIUS_NUMBER_OF_SERVERS]; /**< Array of authentication servers to try in order. */
    vtss_radius_server_info_t servers_acct[VTSS_RADIUS_NUMBER_OF_SERVERS]; /**< Array of accounting servers to try in order. */
    u32                       dead_time_secs;                              /**< Number of seconds to wait before trying the same server again. 0 = disable => always try all servers. */
    BOOL                      nas_ip_address_enable;                       /**< Use NAS-IP-Address if TRUE. */
    vtss_ipv4_t               nas_ip_address;                              /**< NAS-IP-Address. NOTE: In network order!. */
    BOOL                      nas_ipv6_address_enable;                     /**< Use NAS-IPv6-Address if TRUE. */
    vtss_ipv6_t               nas_ipv6_address;                            /**< NAS-IPv6-Address. */
    char                      nas_identifier[VTSS_RADIUS_HOST_LEN];        /**< NAS-Identifier. Used if not empty string. */
} vtss_radius_cfg_s;

/**
 * \brief Get the current module configuration.
 *
 * \param cfg [OUT] Configuration.
 *
 * \return VTSS_OK on success, or any of the vtss_radius_error_e codes on failure.
 */
vtss_rc vtss_radius_cfg_get(vtss_radius_cfg_s *cfg);

/**
 * \brief Configure this module through this function.
 *
 * \param cfg [IN] Configuration.
 *
 * \return VTSS_OK if configuration successfully applied, or any of the\
 *         vtss_radius_error_e codes on failure.
 */
vtss_rc vtss_radius_cfg_set(vtss_radius_cfg_s *cfg);

/******************************************************************************/
// Status and MIB functions
/******************************************************************************/

/**
 * \brief Get the current RADIUS Authentication Client MIB counters (RFC4668).
 *
 * \param idx [IN]  Server index. Value must be in range [0; VTSS_RADIUS_NUMBER_OF_SERVER[.
 * \param mib [OUT] Pointer to a structure that'll receive the current MIB for the specified server.
 *
 * \return VTSS_OK on success, or any of the vtss_radius_error_e codes on failure.
 */
vtss_rc vtss_radius_auth_client_mib_get(int idx, vtss_radius_auth_client_server_mib_s *mib);

/**
 * \brief Get the current RADIUS Accounting Client MIB counters (RFC4670).
 *
 * \param idx [IN]  Server index. Value must be in range [0; VTSS_RADIUS_NUMBER_OF_SERVER[.
 * \param mib [OUT] Pointer to a structure that'll receive the current MIB for the specified server.
 *
 * \return VTSS_OK on success, or any of the vtss_radius_error_e codes on failure.
 */
vtss_rc vtss_radius_acct_client_mib_get(int idx, vtss_radius_acct_client_server_mib_s *mib);

/**
 * \brief Clear current RADIUS Authentication Client MIB counters (RFC4668).
 * All status for the server indicated by @idx will be cleared, except for
 * radiusAuthClientExtPendingRequests, which is used internally by the
 * RADIUS module.
 *
 * \param idx [IN] Value in range [0; VTSS_RADIUS_NUMBER_OF_SERVERS[, specifying which server to clear counters for.\
 *                 It is not possible to clear for all servers in one go (because of the PendingRequests note above).
 *
 * \return VTSS_OK on success, or any of the vtss_radius_error_e codes on failure.
 */
vtss_rc vtss_radius_auth_client_mib_clr(int idx);

/**
 * \brief Clear current RADIUS Accounting Client MIB counters (RFC4670).
 * All status for the server indicated by @idx will be cleared, except for
 * radiusAccClientExtPendingRequests, which is used internally by the
 * RADIUS module.
 *
 * \param idx [IN] Value in range [0; VTSS_RADIUS_NUMBER_OF_SERVERS[, specifying which server to clear counters for.\
 *                 It is not possible to clear for all servers in one go (because of the PendingRequests note above).
 *
 * \return VTSS_OK on success, or any of the vtss_radius_error_e codes on failure.
 */
vtss_rc vtss_radius_acct_client_mib_clr(int idx);

/**
 * \brief Get the current status of all authentication servers.
 *
 */
vtss_rc vtss_radius_auth_server_status_get(vtss_radius_all_server_status_s *status);

/**
 * \brief Get the current status of all accounting servers.
 *
 */
vtss_rc vtss_radius_acct_server_status_get(vtss_radius_all_server_status_s *status);

/**
 * \brief Converts an error returned from this module to a text string.
 *
 */
char *vtss_radius_error_txt(vtss_rc rc);

/******************************************************************************/
// Debug functions
/******************************************************************************/

typedef int (*vtss_radius_dbg_printf_f)(const char *fmt, ...) __attribute__ ((format (__printf__, 1, 2)));

/****************************************************************************/
// vtss_radius_dbg()
// Entry point to RADIUS debug features. Should be called from CLI only.
/****************************************************************************/
void vtss_radius_dbg(vtss_radius_dbg_printf_f dbg_printf, ulong parms_cnt, ulong *parms);

#endif /* __VTSS_RADIUS_API_H__ */

