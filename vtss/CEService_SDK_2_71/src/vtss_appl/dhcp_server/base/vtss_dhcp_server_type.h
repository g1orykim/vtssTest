/*

 Vitesse Switch Application software.

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
//----------------------------------------------------------------------------
/**
 *  \file
 *      vtss_dhcp_server_type.h
 *
 *  \brief
 *      type's definitions
 *
 *  \author
 *      CP Wang
 *
 *  \date
 *      05/08/2013 16:38
 */
//----------------------------------------------------------------------------
#ifndef __VTSS_DHCP_SERVER_TYPE_H__
#define __VTSS_DHCP_SERVER_TYPE_H__
//----------------------------------------------------------------------------

/*
==============================================================================

    Include File

==============================================================================
*/
#include <main.h>
#include <vtss_types.h>

/*
==============================================================================

    Constant

==============================================================================
*/
#define DHCP_SERVER_EXCLUDED_MAX_CNT            16              /**< Number of excluded IP address range */
#define DHCP_SERVER_BINDING_MAX_CNT             1024            /**< Number of binding */
#define DHCP_SERVER_POOL_MAX_CNT                64              /**< Number of DHCP pool */
#define DHCP_SERVER_SERVER_MAX_CNT              4               /**< Number of each server, ex, default router, DNS, NTP ... */
#define DHCP_SERVER_POOL_NAME_LEN               32              /**< Length of DHCP pool name */
#define DHCP_SERVER_CLIENT_IDENTIFIER_LEN       128             /**< Length of client identifier */
#define DHCP_SERVER_DOMAIN_NAME_LEN             128             /**< Length of domain name */
#define DHCP_SERVER_HOST_NAME_LEN               32              /**< Length of host name */
#define DHCP_SERVER_LEASE_DEFAULT               (24 * 60 * 60)  /**< default lease time, in second, 1 day */
#define DHCP_SERVER_MESSAGE_MAX_LEN             2048            /**< maximum sent message length */
#define DHCP_SERVER_DEFAULT_MAX_MESSAGE_SIZE    576             /**< default max size of DHCP message from client */
#define DHCP_SERVER_ALLOCATION_EXPIRE_TIME      30              /**< expire time in second of allocation binding */
#define DHCP_SERVER_VENDOR_CLASS_INFO_CNT       8               /**< Number of vendor class specific infomation */
#define DHCP_SERVER_VENDOR_CLASS_ID_LEN         64              /**< Length of vendor class identifier */
#define DHCP_SERVER_VENDOR_SPECIFIC_INFO_LEN    32              /**< Length of vendor class identifier */
#define DHCP_SERVER_MAC_LEN                     6               /**< Length of MAC address */

/*
==============================================================================

    Macro

==============================================================================
*/
#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#ifndef INOUT
#define INOUT
#endif

/*
==============================================================================

    Type Definition

==============================================================================
*/
typedef enum {
    /* successful code */
    DHCP_SERVER_RC_OK = 0,

    /* failed code */
    DHCP_SERVER_RC_ERROR           = -1000,
    DHCP_SERVER_RC_ERR_PARAMETER   = -999,
    DHCP_SERVER_RC_ERR_NOT_EXIST   = -998,
    DHCP_SERVER_RC_ERR_MEMORY      = -997,
    DHCP_SERVER_RC_ERR_FULL        = -996,
    DHCP_SERVER_RC_ERR_DUPLICATE   = -995,
    DHCP_SERVER_RC_ERR_IP          = -994,
    DHCP_SERVER_RC_ERR_SUBNET      = -993,
} dhcp_server_rc_t;

/**
 *  \brief
 *      Excluded IP database.
 *      index : low_ip, high_ip
 *      both of low_ip and high_ip can not be 0 at the same time.
 *      high_ip >= low_ip
 */
typedef struct {
    vtss_ipv4_t         low_ip;     /**< Begin excluded IP address */
    vtss_ipv4_t         high_ip;    /**< End excluded IP address */
} dhcp_server_excluded_ip_t;

/**
 *  \brief
 *      Type of Pool.
 */
typedef enum {
    DHCP_SERVER_POOL_TYPE_NONE,     /**< Not defined */
    DHCP_SERVER_POOL_TYPE_NETWORK,  /**< Network */
    DHCP_SERVER_POOL_TYPE_HOST,     /**< Host    */
} dhcp_server_pool_type_t;

/**
 *  \brief
 *      Type of netbios node.
 */
typedef enum {
    DHCP_SERVER_NETBIOS_NODE_TYPE_NONE, /**< Node none(invalid) */
    DHCP_SERVER_NETBIOS_NODE_TYPE_B,    /**< Node B, 0x1 */
    DHCP_SERVER_NETBIOS_NODE_TYPE_P,    /**< Node P, 0x2 */
    DHCP_SERVER_NETBIOS_NODE_TYPE_M,    /**< Node M, 0x4 */
    DHCP_SERVER_NETBIOS_NODE_TYPE_H,    /**< Node H, 0x8 */
} dhcp_server_netbios_node_type_t;

/**
 *  \brief
 *      Type of data used for client identifier.
 *
 *      http://www.iana.org/assignments/arp-parameters/arp-parameters.xml
 *      if 0, it contains an identifier, e.g. a fully qualified domain name,
 *          other than a hardware address.
 *      if 1, Ethernet MAC address.
 *      others, ha, we do not understand it.
 */
typedef enum {
    DHCP_SERVER_CLIENT_IDENTIFIER_TYPE_NONE,  /**< Not defined */
    DHCP_SERVER_CLIENT_IDENTIFIER_TYPE_FQDN,  /**< Fully qualified domain name */
    DHCP_SERVER_CLIENT_IDENTIFIER_TYPE_MAC,   /**< Ethernet MAC address */
} dhcp_server_client_identifier_type_t;

/**
 *  \brief
 *      client identifier for option 61
 *      type 0: FQDN
 *           1: MAC address
 */
typedef struct {
    u32             type;   /**< dhcp_server_client_identifier_type_t, type of identifier */
    union {
        char        fqdn[DHCP_SERVER_CLIENT_IDENTIFIER_LEN + 4]; /**< FQDN */
        vtss_mac_t  mac;    /**< MAC address */
    } u;
} dhcp_server_client_identifier_t;

/**
 *  \brief
 *      Vendor class information for option 43 and 60
 *      strlen(class_id) == 0 means not used
 */
typedef struct {
    char        class_id[DHCP_SERVER_VENDOR_CLASS_ID_LEN + 4];          /**< Vendor class identifier */
    u8          specific_info[DHCP_SERVER_VENDOR_SPECIFIC_INFO_LEN];    /**< Vendor specific information */
    u32         specific_info_len;                                      /**< Length of specific_info */
} dhcp_server_vendor_class_info_t;

/**
 *  \brief
 *      DHCP IP allocation pool.
 *      index of name avlt              : name
 *      index of ip avlt                : ip & netmask
 *      index of client identifier avlt : identifier
 *      index of hardware address avlt  : chaddr
 */
typedef struct {
    /* configuration */
    char            pool_name[DHCP_SERVER_POOL_NAME_LEN + 4];           /**< Pool name */
    vtss_ipv4_t     ip;                                                 /**< IP address */
    u32             type;                                               /**< dhcp_server_pool_type_t */

    // the followings are for options
    vtss_ipv4_t     subnet_mask;                                        /**< Subnet mask */
    vtss_ipv4_t     subnet_broadcast;                                   /**< Subnet broadcast address */
    vtss_ipv4_t     default_router[DHCP_SERVER_SERVER_MAX_CNT];         /**< default router */
    u32             lease;                                              /**< Lease time in second */
    char            domain_name[DHCP_SERVER_DOMAIN_NAME_LEN + 4];       /**< Domain name */
    vtss_ipv4_t     dns_server[DHCP_SERVER_SERVER_MAX_CNT];             /**< DNS server */
    vtss_ipv4_t     ntp_server[DHCP_SERVER_SERVER_MAX_CNT];             /**< NTP server */
    vtss_ipv4_t     netbios_name_server[DHCP_SERVER_SERVER_MAX_CNT];    /**< Netbios name server */
    u32             netbios_node_type;                                  /**< dhcp_server_netbios_node_type_t */
    char            netbios_scope[DHCP_SERVER_DOMAIN_NAME_LEN + 4];     /**< Netbios scope */
    char            nis_domain_name[DHCP_SERVER_DOMAIN_NAME_LEN + 4];   /**< NIS domain name */
    vtss_ipv4_t     nis_server[DHCP_SERVER_SERVER_MAX_CNT];             /**< NIS server */

    dhcp_server_vendor_class_info_t     class_info[DHCP_SERVER_VENDOR_CLASS_INFO_CNT];  /**< Vendor class information */

    // for host, manual binding
    dhcp_server_client_identifier_t     client_identifier;                              /**< Client identifier */
    vtss_mac_t                          client_haddr;                                   /**< Client hardware address */
    char                                client_name[DHCP_SERVER_HOST_NAME_LEN + 4];     /**< Client Host name */

    /* runtime data */
    u32             total_cnt;      /**< total number of IP in the pool, = ~netmask */
    u32             alloc_cnt;      /**< number of IP's allocated */
} dhcp_server_pool_t;

/**
 *  \brief
 *      DHCP server statistics.
 */
typedef struct {
    /* database counts */
    u32             pool_cnt;               /**< Number of IP pools */
    u32             excluded_cnt;           /**< Number of excluded range */
    u32             declined_cnt;           /**< Number of declined IP */

    /* binding counts */
    u32             automatic_binding_cnt;  /**< Number of automatic bindings */
    u32             manual_binding_cnt;     /**< Number of manual bindings */
    u32             expired_binding_cnt;    /**< Number of expired bindings */

    /* the following fields can be cleared by UI */

    /* message counts */
    u32             discover_cnt; /**< Number of Discover packets */
    u32             offer_cnt;    /**< Number of Offer packets */
    u32             request_cnt;  /**< Number of Request packets */
    u32             ack_cnt;      /**< Number of ACK packets */
    u32             nak_cnt;      /**< Number of NACK packets */
    u32             decline_cnt;  /**< Number of Decline packets */
    u32             release_cnt;  /**< Number of Release packets */
    u32             inform_cnt;   /**< Number of Inform packets */
} dhcp_server_statistics_t;

/**
 *  \brief
 *      Client ID.
 */
typedef struct {
    u32     type; /**< dhcp_server_client_type_t */
    union {
        vtss_mac_t      chaddr; /**< MAC address */
        char            identifier[DHCP_SERVER_CLIENT_IDENTIFIER_LEN + 4]; /**< Client identifier */
    } u;
} dhcp_server_client_id_t;

/**
 *  \brief
 *      Binding type.
 */
typedef enum {
    DHCP_SERVER_BINDING_TYPE_NONE,        /**< No binding(invalid) */
    DHCP_SERVER_BINDING_TYPE_AUTOMATIC,   /**< Automatic binding with network-type pool */
    DHCP_SERVER_BINDING_TYPE_MANUAL,      /**< Manual binding with host-type pool */
    DHCP_SERVER_BINDING_TYPE_EXPIRED,     /**< Expired binding */
} dhcp_server_binding_type_t;

/**
 *  \brief
 *      Binding state.
 */
typedef enum {
    DHCP_SERVER_BINDING_STATE_NONE,        /**< No state(free list) */
    DHCP_SERVER_BINDING_STATE_ALLOCATED,   /**< Allocated, but wait for final confirm */
    DHCP_SERVER_BINDING_STATE_COMMITTED,   /**< Committed */
    DHCP_SERVER_BINDING_STATE_EXPIRED,     /**< Expired */
} dhcp_server_binding_state_t;

/**
 *  \brief
 *      DHCP binding.
 *          index of binding list : ip
 *          index of binding list : client_id
 *          index of binding list : pool_name, ip
 *          index of binding list : expire_time
 */
typedef struct {
    vtss_ipv4_t                         ip;             /**< IP address allocated */
    vtss_ipv4_t                         subnet_mask;    /**< Subnet mask */
    u32                                 state;          /**< dhcp_server_binding_state_t */
    dhcp_server_binding_type_t          type;           /**< dhcp_server_binding_type_t */
    char                                *pool_name;     /**< IP pool name */
    vtss_ipv4_t                         server_id;      /**< Server identifier to check if this is mine */
    u32                                 vid;            /**< VLAN */
    dhcp_server_client_identifier_t     identifier;     /**< Client identifier */
    vtss_mac_t                          chaddr;         /**< Client hardware address */
    u16                                 _align;         /**< align for MAC, 6 bytes */
    u32                                 lease;          /**< Lease time of the binding in sec */
    u32                                 time_to_start;  /**< system time to start allocation or lease in sec */

    //
    // if STATE_ALLOCATED, expire_time = time_to_start + DHCP_SERVER_ALLOCATION_EXPIRE_TIME
    // if STATE_COMMITTED, expire_time = time_to_start + lease
    //
    u32                                 expire_time;    /**< when the binding expires in sec */
} dhcp_server_binding_t;

//----------------------------------------------------------------------------
#endif //__VTSS_DHCP_SERVER_TYPE_H__
