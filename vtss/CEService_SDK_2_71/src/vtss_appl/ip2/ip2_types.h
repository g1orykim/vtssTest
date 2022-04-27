/*

 Vitesse API software.

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

#ifndef _IP2_TYPES_H_
#define _IP2_TYPES_H_

#include "main.h"
#include "vtss_types.h"

/*
 * Maximum router legs.
 */
#ifdef VTSS_RLEG_CNT
#  define IP2_MAX_INTERFACES VTSS_RLEG_CNT
#else
#  define IP2_MAX_INTERFACES 8u
#endif

#include "dhcp_client_api.h"

/*
 * Maximum static routes.
 */
#define IP2_MAX_ROUTES 32

enum {
    IP2_ERROR_EXISTS = MODULE_ERROR_START(VTSS_MODULE_ID_IP2),
    IP2_ERROR_NOTFOUND,
    IP2_ERROR_NOSPACE,
    IP2_ERROR_PARAMS,
    IP2_ERROR_FAILED,
    IP2_ERROR_ADDRESS_CONFLICT,
    IP2_ERROR_NOT_MASTER,
};

const char *ip2_error_txt(vtss_rc rc);
const char *ip2_chip_error_txt(vtss_rc rc);


typedef vtss_vid_t vtss_if_id_vlan_t;

typedef enum {
    VTSS_ID_IF_TYPE_INVALID = 0,
    VTSS_ID_IF_TYPE_OS_ONLY = 1,
    VTSS_ID_IF_TYPE_VLAN = 2,
} vtss_if_id_type_t;

#define IFNAMSIZ    16
#define IF_NAMESIZE IFNAMSIZ
typedef struct {
    char name[IF_NAMESIZE + 1];
    int  ifno;
} vtss_os_if_t;

typedef struct {
    vtss_if_id_type_t type;

    union {
        vtss_vid_t   vlan;
        vtss_os_if_t os;
    } u;
} vtss_if_id_t;

/* Not complete */
typedef struct {
    u32        mtu;
} vtss_if_param_t;

typedef struct {
    BOOL                dhcpc;     /**< enable dhcp v4 client */
    vtss_ip_network_t   network;   /**< Interface address/mask */
    u32                 fallback_timeout;
} vtss_ip_conf_t;

typedef struct {
    vtss_if_param_t link_layer;
    vtss_ip_conf_t  ipv4;
    vtss_ip_conf_t  ipv6;
} vtss_interface_ip_conf_t;

typedef struct {
    u64 in_packets;
    u64 out_packets;
    u64 in_bytes;
    u64 out_bytes;
    u64 in_multicasts;
    u64 out_multicasts;
    u64 in_broadcasts;
    u64 out_broadcasts;
} vtss_if_status_link_stat_t;

typedef enum {
    VTSS_IF_LINK_FLAG_UP        =  0x1,
    VTSS_IF_LINK_FLAG_BROADCAST =  0x2,
    VTSS_IF_LINK_FLAG_LOOPBACK  =  0x4,
    VTSS_IF_LINK_FLAG_RUNNING   =  0x8,
    VTSS_IF_LINK_FLAG_NOARP     = 0x10,
    VTSS_IF_LINK_FLAG_PROMISC   = 0x20,
    VTSS_IF_LINK_FLAG_MULTICAST = 0x40,
} vtss_if_link_flag_t;

typedef struct {
    u32                 os_if_index;
    u32                 mtu;
    vtss_mac_t          mac;
    vtss_if_link_flag_t flags;
} vtss_if_status_link_t;

typedef struct {
    vtss_ipv4_network_t net;
    vtss_ipv4_t broadcast;
    u32 reasm_max_size;
    u32 arp_retransmit_time;
} vtss_if_status_ipv4_t;

typedef enum {
    VTSS_IF_IPV6_FLAG_ANYCAST    =   0x1,
    VTSS_IF_IPV6_FLAG_TENTATIVE  =   0x2,
    VTSS_IF_IPV6_FLAG_DUPLICATED =   0x4,
    VTSS_IF_IPV6_FLAG_DETACHED   =   0x8,
    VTSS_IF_IPV6_FLAG_DEPRECATED =  0x10,
    VTSS_IF_IPV6_FLAG_NODAD      =  0x20,
    VTSS_IF_IPV6_FLAG_AUTOCONF   =  0x40,
    VTSS_IF_IPV6_FLAG_TEMPORARY  =  0x80,
    VTSS_IF_IPV6_FLAG_HOME       = 0x100,
} vtss_if_ipv6_flag_t;

typedef struct {
    vtss_ipv6_network_t net;
    vtss_if_ipv6_flag_t flags;
    u32                 os_if_index;
} vtss_if_status_ipv6_t;

typedef vtss_dhcp_client_status_t vtss_if_status_dhcp4c_t;

/* Statistics Section: RFC-4293 */
#if defined(VTSS_SW_OPTION_L3RT) && !defined(IP2_SOFTWARE_ROUTING)
#define IP2_STAT_REFRESH_RATE       1000
#else
#define IP2_STAT_REFRESH_RATE       500
#endif /* defined(VTSS_SW_OPTION_L3RT) && !defined(IP2_SOFTWARE_ROUTING) */
#define IP2_STAT_IMSG_MAX           0x100

typedef struct {
    u32                 InReceives;
    u64                 HCInReceives;
    u32                 InOctets;
    u64                 HCInOctets;
    u32                 InHdrErrors;
    u32                 InNoRoutes;
    u32                 InAddrErrors;
    u32                 InUnknownProtos;
    u32                 InTruncatedPkts;
    u32                 InForwDatagrams;
    u64                 HCInForwDatagrams;
    u32                 ReasmReqds;
    u32                 ReasmOKs;
    u32                 ReasmFails;
    u32                 InDiscards;
    u32                 InDelivers;
    u64                 HCInDelivers;
    u32                 OutRequests;
    u64                 HCOutRequests;
    u32                 OutNoRoutes;
    u32                 OutForwDatagrams;
    u64                 HCOutForwDatagrams;
    u32                 OutDiscards;
    u32                 OutFragReqds;
    u32                 OutFragOKs;
    u32                 OutFragFails;
    u32                 OutFragCreates;
    u32                 OutTransmits;
    u64                 HCOutTransmits;
    u32                 OutOctets;
    u64                 HCOutOctets;
    u32                 InMcastPkts;
    u64                 HCInMcastPkts;
    u32                 InMcastOctets;
    u64                 HCInMcastOctets;
    u32                 OutMcastPkts;
    u64                 HCOutMcastPkts;
    u32                 OutMcastOctets;
    u64                 HCOutMcastOctets;
    u32                 InBcastPkts;
    u64                 HCInBcastPkts;
    u32                 OutBcastPkts;
    u64                 HCOutBcastPkts;

    vtss_timestamp_t    DiscontinuityTime;
    u32                 RefreshRate;
} vtss_ip_stat_data_t;

typedef struct {
    vtss_ip_type_t      IPVersion;  /* INDEX */

    vtss_ip_stat_data_t data;
} vtss_ips_ip_stat_t;

typedef struct {
    u32                     InMsgs;
    u32                     InErrors;
    u32                     OutMsgs;
    u32                     OutErrors;
} vtss_icmp_stat_data_t;

typedef struct {
    vtss_ip_type_t          IPVersion;  /* INDEX */
    u32                     Type;       /* INDEX */

    vtss_icmp_stat_data_t   data;
} vtss_ips_icmp_stat_t;

typedef enum {
    VTSS_IPS_STATUS_TYPE_INVALID = 0,   /* Invalid IPS status entry */
    VTSS_IPS_STATUS_TYPE_ANY,           /* Used to query all IPS status */

    VTSS_IPS_STATUS_TYPE_STAT_IPV4,     /* IPV4 System Statistics */
    VTSS_IPS_STATUS_TYPE_STAT_IPV6,     /* IPV6 System Statistics */
    VTSS_IPS_STATUS_TYPE_STAT_ICMP4,    /* IPV4 System ICMP Statistics */
    VTSS_IPS_STATUS_TYPE_STAT_ICMP6     /* IPV6 System ICMP Statistics */
} vtss_ips_status_type_t;

typedef struct {
    vtss_ip_type_t          version;
    u32                     imsg;
    vtss_ips_status_type_t  type;

    union {
        vtss_ips_icmp_stat_t    icmp_stat;
        vtss_ips_ip_stat_t      ip_stat;
    } u;
} vtss_ips_status_t;

typedef struct {
    vtss_ip_type_t      IPVersion;  /* INDEX */
    vtss_if_id_t        IfIndex;    /* INDEX */

    vtss_ip_stat_data_t data;
} vtss_if_status_ip_stat_t;

typedef enum {
    VTSS_IF_STATUS_TYPE_INVALID = 0,   /* Invalid status entry */
    VTSS_IF_STATUS_TYPE_ANY = 1,       /* Used to query all status */
    VTSS_IF_STATUS_TYPE_LINK = 2,      /* Link layer status */
    VTSS_IF_STATUS_TYPE_LINK_STAT = 3, /* Link layer statistics */
    VTSS_IF_STATUS_TYPE_IPV4 = 4,      /* IPv4 status */
    VTSS_IF_STATUS_TYPE_DHCP = 5,      /* dhcp status */
    VTSS_IF_STATUS_TYPE_IPV6 = 6,      /* IPv6 status */

    /* Statistics Section: RFC-4293 */
    VTSS_IF_STATUS_TYPE_STAT_IPV4 = 7,  /* IPV4 Per-Interface Statistics */
    VTSS_IF_STATUS_TYPE_STAT_IPV6 = 8   /* IPV6 Per-Interface Statistics */
} vtss_if_status_type_t;

typedef struct {
    vtss_if_id_t if_id;
    vtss_if_status_type_t type;

    union {
        vtss_if_status_link_t      link;
        vtss_if_status_link_stat_t link_stat;
        vtss_if_status_ipv4_t      ipv4;
        vtss_if_status_ipv6_t      ipv6;
        vtss_if_status_dhcp4c_t    dhcp4c;
        vtss_if_status_ip_stat_t   ip_stat;
    } u;
} vtss_if_status_t;

enum {
    VTSS_NEIGHBOUR_FLAG_VALID      = (1 << 0),
    VTSS_NEIGHBOUR_FLAG_PERMANENT  = (1 << 1),
    VTSS_NEIGHBOUR_FLAG_ROUTER     = (1 << 2),
    VTSS_NEIGHBOUR_FLAG_HARDWARE   = (1 << 3),
};

typedef struct {
    vtss_mac_t     mac_address;
    vtss_if_id_t   interface;
    vtss_ip_addr_t ip_address;
    char           state[8];
    u8             flags;
} vtss_neighbour_status_t;

typedef enum {
    VTSS_ROUTING_PARAM_OWNER_INVALID,
    VTSS_ROUTING_PARAM_OWNER_DYNAMIC_USER,
    VTSS_ROUTING_PARAM_OWNER_STATIC_USER,
    VTSS_ROUTING_PARAM_OWNER_DHCP,
} vtss_routing_param_owner_t;

typedef enum {
    VTSS_ROUTING_FLAG_UP      = 0x1, /* route usable */
    VTSS_ROUTING_FLAG_HOST    = 0x2, /* host entry (net otherwise) */
    VTSS_ROUTING_FLAG_GATEWAY = 0x4, /* destination is a gateway */
    VTSS_ROUTING_FLAG_REJECT  = 0x8, /* destination is a gateway */
    VTSS_ROUTING_FLAG_HW_RT   = 0x10, /* HW-route */
} vtss_routing_flags_t;

/* Not complete */
/* This struct is for storing data associated to a route, but data which is not
 * need by the IP stack. */
typedef struct {
    vtss_routing_param_owner_t owner;
} vtss_routing_params_t;

typedef struct {
    u32                        owners; /* Bitmap of route owners */
} vtss_routing_info_t;

typedef struct {
    vtss_routing_entry_t  rt;
    vtss_routing_params_t params;
    vtss_routing_flags_t  flags;
    vtss_if_id_t          interface;
    int                   lifetime;  // ipDefaultRouterLifetime in RFC4293
    int                   preference;  // ipDefaultRouterPreference in RFC4293
} vtss_routing_status_t;

typedef struct {
    vtss_mac_t     dmac; /**< MAC address of destination */
    vtss_vid_t     vlan; /**< VLAN of destination */
    vtss_ip_addr_t dip;  /**< IP address of destination */
} vtss_neighbour_t;

typedef int (*vtss_ipstack_filter_cb_t) (vtss_vid_t, unsigned, const char *);

typedef enum {
    VTSS_IP_SRV_TYPE_DHCP_ANY  = 0x0,
    VTSS_IP_SRV_TYPE_NONE      = 0x1,
    VTSS_IP_SRV_TYPE_STATIC    = 0x2,
    VTSS_IP_SRV_TYPE_DHCP_VLAN = 0x4,
} vtss_ip_srv_conf_type_t;

typedef struct {
    vtss_ip_addr_t ip_srv_address;
} vtss_ip_srv_conf_static_t;

typedef struct {
    vtss_vid_t vlan;
} vtss_ip_srv_conf_dhcp_vlan;

typedef struct {
    vtss_ip_srv_conf_type_t type;

    union {
        vtss_ip_srv_conf_static_t static_conf;
        vtss_ip_srv_conf_dhcp_vlan dhcp_vlan_conf;
    } u;
} vtss_ip_srv_conf_t;

typedef enum {
    VTSS_IP_DBG_FLAG_NONE       = 0,
    VTSS_IP_DBG_FLAG_ND6_LOG    = (1 << 0)
} vtss_ip_debug_type_t;

typedef struct {
    BOOL enable_routing;        /**< Enable rounting (all interfaces) */
#ifdef VTSS_SW_OPTION_NTP
    vtss_ip_srv_conf_t ntp;
#endif /* VTSS_SW_OPTION_NTP */
    vtss_ip_debug_type_t dbg_flag;
    char reserved[252];
} vtss_ip2_global_param_t;


typedef enum {
    VTSS_IPSTACK_MSG_RT_ADD = 0x1,
    VTSS_IPSTACK_MSG_RT_DEL = 0x2,
    VTSS_IPSTACK_MSG_NB_ADD = 0x3,
    VTSS_IPSTACK_MSG_NB_DEL = 0x4,
    VTSS_IPSTACK_MSG_IF_SIG = 0x5,
} vtss_ipstack_msg_type_t;

typedef struct {
    int            ifidx;
    vtss_mac_t     dmac;
    vtss_ip_addr_t dip;
} vtss_ipstack_msg_nb_t;

typedef struct {
    int                  ifidx;
    vtss_routing_entry_t route;
} vtss_ipstack_msg_rt_t;

typedef struct {
    int                  ifidx;
} vtss_ipstack_msg_if_t;

typedef struct {
    vtss_ipstack_msg_type_t type;
    union {
        vtss_ipstack_msg_nb_t nb;
        vtss_ipstack_msg_rt_t rt;
        vtss_ipstack_msg_if_t interface;
    } u;
} vtss_ipstack_msg_t;

#endif /* _IP2_TYPES_H_ */
