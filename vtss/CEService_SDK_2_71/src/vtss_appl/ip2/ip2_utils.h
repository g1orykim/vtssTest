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

#ifndef _IP2_UTILS_H_
#define _IP2_UTILS_H_

#include "ip2_types.h"

#define VTSS_MAC_FORMAT "%02x-%02x-%02x-%02x-%02x-%02x"
#define VTSS_MAC_ARGS(X) \
    (X).addr[0], (X).addr[1], (X).addr[2], \
    (X).addr[3], (X).addr[4], (X).addr[5]

#define VTSS_IPV4_FORMAT "%u.%u.%u.%u"
#define VTSS_IPV4_ARGS(X) \
    (((X) >> 24) & 0xff), (((X) >> 16) & 0xff), \
    (((X) >> 8) & 0xff), ((X) & 0xff)

#define VTSS_IPV6_FORMAT "%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x"
#define VTSS_IPV6_ARGS(X) \
    ((((u32)((X).addr[ 0])) << 8) | ((u32)(X).addr[ 1])), \
    ((((u32)((X).addr[ 2])) << 8) | ((u32)(X).addr[ 3])), \
    ((((u32)((X).addr[ 4])) << 8) | ((u32)(X).addr[ 5])), \
    ((((u32)((X).addr[ 6])) << 8) | ((u32)(X).addr[ 7])), \
    ((((u32)((X).addr[ 8])) << 8) | ((u32)(X).addr[ 9])), \
    ((((u32)((X).addr[10])) << 8) | ((u32)(X).addr[11])), \
    ((((u32)((X).addr[12])) << 8) | ((u32)(X).addr[13])), \
    ((((u32)((X).addr[14])) << 8) | ((u32)(X).addr[15]))

#define VTSS_IPV6N_FORMAT VTSS_IPV6_FORMAT "/%d"
#define VTSS_IPV6N_ARG(X) VTSS_IPV6_ARGS((X).address), (X).prefix_size

#define VTSS_IPV6_UC_FORMAT \
    "{network = "VTSS_IPV6N_FORMAT", destination = "VTSS_IPV6_FORMAT"}"
#define VTSS_IPV6_UC_ARGS(X) VTSS_IPV6N_ARG((X).network), \
    VTSS_IPV6_ARGS((X).destination)

#define VTSS_IPV4N_FORMAT VTSS_IPV4_FORMAT "/%d"
#define VTSS_IPV4N_ARG(X) VTSS_IPV4_ARGS((X).address), (X).prefix_size

#define VTSS_IPV4_UC_FORMAT \
    "{network = "VTSS_IPV4N_FORMAT", destination = "VTSS_IPV4_FORMAT"}"
#define VTSS_IPV4_UC_ARGS(X) VTSS_IPV4N_ARG((X).network), \
    VTSS_IPV4_ARGS((X).destination)

#define IP2_MAX_ROUTES 32 /* Arbitrary number...? */


/* Misc helpers ------------------------------------------------------------ */
#define IP2_VALID_VLAN_ID(x)                (((x) >= VLAN_ID_MIN) && ((x) <= VLAN_ID_MAX))
#define IP2_IPV6_RT_IFID_ZERO               0x0
#define IP2_IPV6_RTNH_LLA_IFID_DEF          IP2_IPV6_RT_IFID_ZERO

#define IP2_IPV6_RTNH_LLA_IFID_GET(x, y)    do {                        \
    (x) = VTSS_VID_NULL;                                                \
    if (((y)->type != VTSS_ROUTING_ENTRY_TYPE_IPV6_UC) ||               \
        !vtss_ipv6_addr_is_link_local(&(y)->route.ipv6_uc.destination)) \
        break;                                                          \
    (x) = (y)->route.ipv6_uc.destination.addr[2] << 8;                  \
    (x) += (y)->route.ipv6_uc.destination.addr[3];                      \
} while (0)
#define IP2_IPV6_RTNH_LLA_IFID_SET(x, y)    do {                        \
    if (((y)->type != VTSS_ROUTING_ENTRY_TYPE_IPV6_UC) ||               \
        !vtss_ipv6_addr_is_link_local(&(y)->route.ipv6_uc.destination)) \
        break;                                                          \
    (y)->route.ipv6_uc.destination.addr[2] = (u8) (((x) >> 8) & 0xFF);  \
    (y)->route.ipv6_uc.destination.addr[3] = (u8) ((x) & 0xFF);         \
} while (0)

#define IP2_IPV6_RTNH_LLA_VLAN_GET(x, y)    do {                        \
    (x) = VTSS_VID_NULL;                                                \
    if (((y)->type != VTSS_ROUTING_ENTRY_TYPE_IPV6_UC) ||               \
        !vtss_ipv6_addr_is_link_local(&(y)->route.ipv6_uc.destination)) \
        break;                                                          \
    (x) = (y)->vlan;                                                    \
} while (0)

#define IP2_IPV6_RT_LL_IFID_GET(x, y)    do {                               \
    (x) = IP2_IPV6_RT_IFID_ZERO;                                            \
    if (((y)->type != VTSS_ROUTING_ENTRY_TYPE_IPV6_UC) ||                   \
        !vtss_ipv6_addr_is_link_local(&(y)->route.ipv6_uc.network.address)) \
        break;                                                              \
    (x) = (y)->route.ipv6_uc.network.address.addr[2] << 8;                  \
    (x) += (y)->route.ipv6_uc.network.address.addr[3];                      \
} while (0)
#define IP2_IPV6_RT_LL_IFID_SET(x, y)         do {                          \
    if (((y)->type != VTSS_ROUTING_ENTRY_TYPE_IPV6_UC) ||                   \
        !vtss_ipv6_addr_is_link_local(&(y)->route.ipv6_uc.network.address)) \
        break;                                                              \
    (y)->route.ipv6_uc.network.address.addr[2] = (u8) (((x) >> 8) & 0xFF);  \
    (y)->route.ipv6_uc.network.address.addr[3] = (u8) ((x) & 0xFF);         \
} while (0)

/* Returns TRUE if IPv4/v6 is a zero-address. FALSE otherwise. */
BOOL vtss_ip_addr_is_zero(          const vtss_ip_addr_t        *const ip_addr);
BOOL vtss_ipv4_addr_is_multicast(   const vtss_ipv4_t           *const ip);
BOOL vtss_ipv6_addr_is_link_local(  const vtss_ipv6_t           *const ip);
BOOL vtss_ipv6_addr_is_multicast(   const vtss_ipv6_t           *const ip);
BOOL vtss_ipv6_addr_is_zero(        const vtss_ipv6_t           *const addr);
BOOL vtss_ipv6_addr_is_loopback(    const vtss_ipv6_t           *const addr);
BOOL vtss_ipv6_addr_is_mgmt_support(const vtss_ipv6_t           *const addr);

// using the class system to derive a prefix
int vtss_ipv4_addr_to_prefix(       vtss_ipv4_t                  ip);

/* Type conversion --------------------------------------------------------- */

vtss_rc vtss_prefix_cnt(            const u8                    *data,
                                    const u32                    length,
                                    u32                         *prefix);
vtss_rc
vtss_conv_ipv4mask_to_prefix(       const vtss_ipv4_t            ipv4,
                                    u32                         *const prefix);

vtss_rc
vtss_conv_prefix_to_ipv4mask(       const u32                    prefix,
                                    vtss_ipv4_t                 *const ipv4);

vtss_rc
vtss_conv_ipv6mask_to_prefix(       const vtss_ipv6_t           *const ipv6,
                                    u32                         *const prefix);

vtss_rc
vtss_conv_prefix_to_ipv6mask(       const u32                    prefix,
                                    vtss_ipv6_t                 *const ipv6);

vtss_rc vtss_build_ipv4_network(    vtss_ip_network_t           *const net,
                                    const vtss_ipv4_t            address,
                                    const vtss_ipv4_t            mask);

vtss_rc vtss_build_ipv4_uc(         vtss_routing_entry_t        *route,
                                    const vtss_ipv4_t            address,
                                    const vtss_ipv4_t            mask,
                                    const vtss_ipv4_t            gateway);

vtss_rc vtss_build_ipv6_network(    vtss_ip_network_t           *const net,
                                    const vtss_ipv6_t           *address,
                                    const vtss_ipv6_t           *mask);

vtss_rc vtss_build_ipv6_uc(         vtss_routing_entry_t        *const route,
                                    const vtss_ipv6_t           *address,
                                    const vtss_ipv6_t           *mask,
                                    const vtss_ipv6_t           *gateway,
                                    const vtss_vid_t             vid);

void vtss_ip2_if_id_vlan(           vtss_vid_t                   vid,
                                    vtss_if_id_t                *if_id);

vtss_rc vtss_if_status_to_ip(       const vtss_if_status_t      *const status,
                                    vtss_ip_addr_t              *const ip);

BOOL vtss_if_status_match_vlan(     vtss_vid_t                  vlan,
                                    vtss_if_status_t           *if_status);

BOOL vtss_if_status_match_ip_type(  vtss_ip_type_t               type,
                                    vtss_if_status_t            *if_status);

BOOL vtss_if_status_match_ip(       const vtss_ip_addr_t        *const ip,
                                    vtss_if_status_t            *if_status);

void vtss_ipv4_default_route(       vtss_ipv4_t                  gw,
                                    vtss_routing_entry_t        *rt);

BOOL vtss_ip_ifaddr_valid(          const vtss_ip_network_t     *const net);

/* Various type equal checks ----------------------------------------------- */

BOOL vtss_ip_equal(                 const vtss_ip_network_t     *const a,
                                    const vtss_ip_network_t     *const b);

BOOL vtss_ipv4_network_equal(       const vtss_ipv4_network_t   *const a,
                                    const vtss_ipv4_network_t   *const b);

BOOL vtss_ipv6_network_equal(       const vtss_ipv6_network_t   *const a,
                                    const vtss_ipv6_network_t   *const b);

BOOL vtss_ip_network_equal(         const vtss_ip_network_t     *const a,
                                    const vtss_ip_network_t     *const b);

BOOL vtss_ipv4_net_equal(           const vtss_ipv4_network_t   *const a,
                                    const vtss_ipv4_network_t   *const b);

BOOL vtss_ip_net_equal(             const vtss_ip_network_t     *const a,
                                    const vtss_ip_network_t     *const b);

BOOL vtss_ipv4_net_overlap(         const vtss_ipv4_network_t   *const a,
                                    const vtss_ipv4_network_t   *const b);

BOOL vtss_ipv6_net_overlap(         const vtss_ipv6_network_t   *const a,
                                    const vtss_ipv6_network_t   *const b);

BOOL vtss_ip_net_overlap(           const vtss_ip_network_t   *const a,
                                    const vtss_ip_network_t   *const b);

int vtss_route_compare(             const vtss_routing_entry_t *rta,
                                    const vtss_routing_entry_t *rtb);

BOOL vtss_if_id_equal(              const vtss_if_id_t          *const a,
                                    const vtss_if_id_t          *const b);

BOOL vtss_ip_route_valid(           const vtss_routing_entry_t *const rt);
BOOL vtss_ip_route_nh_valid(        const vtss_routing_entry_t *const rt);

/* Printers */
int vtss_ip2_ip_addr_to_txt(        char                        *buf,
                                    int                          size,
                                    const vtss_ip_addr_t        *const ip);

int vtss_ip2_ip_network_to_txt(     char                        *buf,
                                    int                          size,
                                    const vtss_ip_network_t     *const ip);

int vtss_ip2_neighbour_to_txt(      char                        *buf,
                                    int                          size,
                                    const vtss_neighbour_t      *const nb);

int vtss_ip2_route_entry_to_txt(    char                        *buf,
                                    int                          size,
                                    const vtss_routing_entry_t  *const rt);

int vtss_ip2_if_id_to_txt(          char                        *buf,
                                    int                          size,
                                    const vtss_if_id_t          *const if_id);

int vtss_if_link_flag_to_txt(       char                        *buf,
                                    int                          size,
                                    vtss_if_link_flag_t          f);

int vtss_if_ipv6_flag_to_txt(       char                        *buf,
                                    int                          size,
                                    vtss_if_ipv6_flag_t          f);

int vtss_routing_flags_to_txt(      char                        *buf,
                                    int                          size,
                                    vtss_routing_flags_t         f);

int vtss_ip2_if_status_to_txt(      char                        *buf,
                                    int                          size,
                                    vtss_if_status_t            *st,
                                    u32                          length);

int vtss_ip2_neighbour_status_to_txt(char                       *buf,
                                     int                         size,
                                     vtss_neighbour_status_t    *st);

#define IP2_MAX_ICMP_TXT_LEN    64
int vtss_ip2_stat_icmp_type_txt(     char                       *buf,
                                     int                        size,
                                     const vtss_ip_type_t       version,
                                     const u32                  icmp_type);

/* Sorting */
void vtss_ip2_if_status_sort(u32 cnt, vtss_if_status_t *status);

#endif /* _IP2_UTILS_H_ */

