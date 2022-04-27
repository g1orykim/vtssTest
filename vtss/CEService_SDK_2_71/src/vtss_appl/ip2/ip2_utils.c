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

#include "ip2_utils.h"
#include "misc_api.h"
#include <network.h>         /* For htons() and htonl() */

#define PRINTF(...)                                         \
    if (size - s > 0) {                                     \
        int res = snprintf(buf + s, size - s, __VA_ARGS__); \
        if (res >0 ) {                                      \
            s += res;                                       \
        }                                                   \
    }

#define PRINTFUNC(F, ...)                       \
    if (size - s > 0) {                         \
        s += F(buf + s, size - s, __VA_ARGS__); \
    }


/* Compare 2 32 bit entities with a number of significant bits */
static int bitcmp32(u32 a1, u32 a2, int bitlen)
{
    a1 = (a1 >> (32 - bitlen));
    a2 = (a2 >> (32 - bitlen));
    return (a1 - a2);
}

int bitcmp(const u8 *a1, const u8 *a2, int bitlen)
{
    int bytes = (bitlen / 8);
    if (bytes) {
        int dif = memcmp(a1, a2, bytes);
        /* Return if difference seen sofar, or no more bits to compare */
        if (dif != 0 || (bitlen %= 8) == 0) {
            return dif;
        }
    }
    /* Residual bits difference */
    return (a1[0] >> (8 - bitlen)) - (a2[0] >> (8 - bitlen));
}

int vtss_route_compare(const vtss_routing_entry_t *rta,
                       const vtss_routing_entry_t *rtb)
{
    if (rta->type != rtb->type) {
        return rta->type < rtb->type ? -1 : 1;
    } else {
        int cmp, l;
        switch (rta->type) {
        case VTSS_ROUTING_ENTRY_TYPE_IPV4_UC:
            l = MIN(rta->route.ipv4_uc.network.prefix_size, rtb->route.ipv4_uc.network.prefix_size);
            if ((cmp = bitcmp32(rta->route.ipv4_uc.network.address, rtb->route.ipv4_uc.network.address, l)) != 0) {
                return cmp < 0 ? -1 : 1;
            } else if (rta->route.ipv4_uc.network.prefix_size != rtb->route.ipv4_uc.network.prefix_size) {
                return rta->route.ipv4_uc.network.prefix_size < rtb->route.ipv4_uc.network.prefix_size ? -1 : 1;
            } else if ((cmp = (rta->route.ipv4_uc.destination - rtb->route.ipv4_uc.destination)) != 0) {
                /* NB: Above assumes host order... */
                return cmp < 0 ? -1 : 1;
            }
            break;
        case VTSS_ROUTING_ENTRY_TYPE_IPV6_UC:
            if ((cmp = bitcmp(rta->route.ipv6_uc.network.address.addr,
                              rtb->route.ipv6_uc.network.address.addr,
                              rta->route.ipv6_uc.network.prefix_size)) != 0) {
                return cmp < 0 ? -1 : 1;
            } else if (rta->route.ipv6_uc.network.prefix_size != rtb->route.ipv6_uc.network.prefix_size) {
                return rta->route.ipv6_uc.network.prefix_size < rtb->route.ipv6_uc.network.prefix_size ? -1 : 1;
            } else if ((cmp = memcmp(rta->route.ipv6_uc.destination.addr,
                                     rtb->route.ipv6_uc.destination.addr,
                                     sizeof(rta->route.ipv6_uc.destination))) != 0) {
                return cmp < 0 ? -1 : 1;
            }
            break;
        case VTSS_ROUTING_ENTRY_TYPE_IPV4_MC:
        default:
            return -1;
        }
    }
    return 0;
}

static BOOL vtss_ipv4_addr_is_zero(vtss_ipv4_t addr)
{
    return addr == 0;
}

BOOL vtss_ipv6_addr_is_zero(const vtss_ipv6_t *const addr)
{
    static const vtss_ipv6_t zero_ipv6;
    return memcmp(addr->addr, zero_ipv6.addr, sizeof(zero_ipv6.addr)) == 0;
}

// Pure function, no side effects
/* Returns TRUE if IPv4/v6 is a zero-address. FALSE otherwise. */
BOOL vtss_ip_addr_is_zero(const vtss_ip_addr_t *ip_addr)
{
    switch (ip_addr->type) {
    case VTSS_IP_TYPE_IPV4:
        return vtss_ipv4_addr_is_zero(ip_addr->addr.ipv4);
    case VTSS_IP_TYPE_IPV6:
        return vtss_ipv6_addr_is_zero(&ip_addr->addr.ipv6);
    case VTSS_IP_TYPE_NONE:
        return TRUE;            /* Consider VTSS_IP_TYPE_NONE as zero */
    }

    return FALSE;               /* Undefined */
}

// Pure function, no side effects
BOOL vtss_ipv4_addr_is_multicast(const vtss_ipv4_t *const ip)
{
    if (!ip) {
        return FALSE;
    }

    /* IN_CLASSD */
    return (((*ip >> 24) & 0xF0) == 0xE0);
}

// Pure function, no side effects
BOOL vtss_ipv6_addr_is_link_local(const vtss_ipv6_t *const ip)
{
    if (ip->addr[0] == 0xfe && (ip->addr[1] >> 6) == 0x2) {
        return TRUE;
    }
    return FALSE;
}

// Pure function, no side effects
BOOL vtss_ipv6_addr_is_multicast(const vtss_ipv6_t *const ip)
{
    if (ip->addr[0] == 0xff) {
        return TRUE;
    }
    return FALSE;
}

// Pure function, no side effects
BOOL vtss_ipv6_addr_is_loopback(const vtss_ipv6_t *const addr)
{
    u8  idx;

    if (!addr) {
        return FALSE;
    }

    for (idx = 0; idx < 16; ++idx) {
        if (idx < 15) {
            if (addr->addr[idx]) {
                return FALSE;
            }
        } else {
            if (addr->addr[idx] != 0x1) {
                return FALSE;
            }
        }
    }

    return TRUE;
}

static BOOL _vtss_ipv6_addr_is_v4_form(const vtss_ipv6_t *const addr)
{
    u8  idx;

    if (!addr) {
        return FALSE;
    }

    for (idx = 0; idx < 16; ++idx) {
        if (idx < 10) {
            if (addr->addr[idx]) {
                return FALSE;
            }
        } else {
            if (idx < 12) {
                if (addr->addr[idx] &&
                    (addr->addr[idx] != 0xFF)) {
                    return FALSE;
                }
            } else {
                if (vtss_ipv6_addr_is_zero(addr)) {
                    return FALSE;
                }

                break;
            }
        }
    }

    return TRUE;
}

// Pure function, no side effects
BOOL vtss_ipv6_addr_is_mgmt_support(const vtss_ipv6_t *const addr)
{
    if (vtss_ipv6_addr_is_loopback(addr) ||
        _vtss_ipv6_addr_is_v4_form(addr)) {
        return FALSE;
    }

    return TRUE;
}

// Pure function, no side effects
int vtss_ipv4_addr_to_prefix(vtss_ipv4_t ip)
{
    if ((ip & 0xff000000) == 0x0a000000) { // 10.0.0.0/8
        return  8;
    } else if ((ip & 0xfff00000) == 0xac100000) { // 172.16.0.0/12
        return 12;
    } else if ((ip & 0xffff0000) == 0xc0a80000) { // 192.168.0.0/16
        return 16;
    } else if ((ip & 0x80000000) == 0x00000000) { // class A
        return  8;
    } else if ((ip & 0xC0000000) == 0x80000000) { // class B
        return 16;
    } else if ((ip & 0xE0000000) == 0x80000000) { // class C
        return 24;
    }
    return 0;
}

// Pure function, no side effects
vtss_rc vtss_prefix_cnt(const u8  *data,
                        const u32  length,
                        u32       *prefix)
{
    u32 i;
    u32 cnt = 0;
    BOOL prefix_ended = FALSE;

    for (i = 0; i < length; ++i) {
        if (prefix_ended && data[i] != 0) {
            return VTSS_RC_ERROR;
        }

        /*lint --e{616} ... Fallthrough intended */
        switch (data[i]) {
        case 0xff:
            cnt += 8;
            break;
        case 0xfe:
            cnt += 1;
        case 0xfc:
            cnt += 1;
        case 0xf8:
            cnt += 1;
        case 0xf0:
            cnt += 1;
        case 0xe0:
            cnt += 1;
        case 0xc0:
            cnt += 1;
        case 0x80:
            cnt += 1;
        case 0x00:
            prefix_ended = TRUE;
            break;
        default:
            return VTSS_RC_ERROR;
        }
    }

    *prefix = cnt;
    return VTSS_RC_OK;
}


// Pure function, no side effects
vtss_rc vtss_conv_ipv4mask_to_prefix(const vtss_ipv4_t  ipv4,
                                     u32               *const prefix)
{
    u32 data = htonl(ipv4);
    return vtss_prefix_cnt((u8 *)&data, 4, prefix);
}

// Pure function, no side effects
vtss_rc vtss_conv_prefix_to_ipv4mask(const u32     prefix,
                                     vtss_ipv4_t  *const rmask)
{
    unsigned i;
    vtss_ipv4_t mask = 0;

    if (prefix > 32u) {
        return VTSS_RC_ERROR;
    }

    for (i = 0; i < 32u; i++) {
        mask <<= 1;

        if (i < prefix) {
            mask |= 1;
        }
    }

    *rmask = mask;
    return VTSS_RC_OK;
}


// Pure function, no side effects
vtss_rc vtss_conv_ipv6mask_to_prefix(const vtss_ipv6_t *const ipv6,
                                     u32               *const prefix)
{
    return vtss_prefix_cnt(ipv6->addr, 16, prefix);
}

// Pure function, no side effects
vtss_rc vtss_conv_prefix_to_ipv6mask(const u32           prefix,
                                     vtss_ipv6_t        *const mask)
{
    u8 v = 0;
    u32 i = 0;
    u32 next_bit = 0;

    if (prefix > 128) {
        return VTSS_RC_ERROR;
    }

    /* byte-wise update or clear */
    for ( i = 0; i < 16; ++i ) {
        u32 b = (i + 1) * 8;
        if ( b <= prefix ) {
            mask->addr[i] = 0xff;
            next_bit = b;
        } else {
            mask->addr[i] = 0x0;
        }
    }

    switch ( prefix % 8 ) {
    case 1:
        v = 0x80;
        break;
    case 2:
        v = 0xc0;
        break;
    case 3:
        v = 0xe0;
        break;
    case 4:
        v = 0xf0;
        break;
    case 5:
        v = 0xf8;
        break;
    case 6:
        v = 0xfc;
        break;
    case 7:
        v = 0xfe;
        break;
    }
    mask->addr[(next_bit / 8)] = v;
    return VTSS_RC_OK;
}

// Pure function, no side effects
vtss_rc vtss_build_ipv4_network(vtss_ip_network_t *const network,
                                const vtss_ipv4_t   address,
                                const vtss_ipv4_t   mask)
{
    u32 prefix = 0;
    network->address.type = VTSS_IP_TYPE_NONE;
    VTSS_RC(vtss_conv_ipv4mask_to_prefix(mask, &prefix));
    network->address.type = VTSS_IP_TYPE_IPV4;
    network->address.addr.ipv4 = address;
    network->prefix_size = prefix;
    return VTSS_RC_OK;
}

// Pure function, no side effects
vtss_rc vtss_build_ipv4_uc(vtss_routing_entry_t *route,
                           const vtss_ipv4_t address,
                           const vtss_ipv4_t mask,
                           const vtss_ipv4_t gateway)
{
    vtss_ip_network_t net;
    route->type = VTSS_ROUTING_ENTRY_TYPE_INVALID;
    VTSS_RC(vtss_build_ipv4_network(&net, address, mask));
    route->type = VTSS_ROUTING_ENTRY_TYPE_IPV4_UC;
    route->route.ipv4_uc.network.address = net.address.addr.ipv4;
    route->route.ipv4_uc.network.prefix_size = net.prefix_size;
    route->route.ipv4_uc.destination = gateway;
    return VTSS_RC_OK;
}

// Pure function, no side effects
vtss_rc vtss_build_ipv6_network(vtss_ip_network_t   *const network,
                                const vtss_ipv6_t   *address,
                                const vtss_ipv6_t   *mask)
{
    u32 prefix;

    if (!network || !address || !mask) {
        return VTSS_RC_ERROR;
    }

    prefix = 0;
    network->address.type = VTSS_IP_TYPE_NONE;
    VTSS_RC(vtss_conv_ipv6mask_to_prefix(mask, &prefix));

    network->address.type = VTSS_IP_TYPE_IPV6;
    memcpy(&network->address.addr.ipv6, address, sizeof(vtss_ipv6_t));
    network->prefix_size = prefix;

    return VTSS_RC_OK;
}

// Pure function, no side effects
vtss_rc vtss_build_ipv6_uc(vtss_routing_entry_t *const route,
                           const vtss_ipv6_t    *address,
                           const vtss_ipv6_t    *mask,
                           const vtss_ipv6_t    *gateway,
                           const vtss_vid_t      vid)
{
    vtss_ip_network_t   net;

    if (!route || !address || !mask) {
        return VTSS_RC_ERROR;
    }

    memset(route, 0x0, sizeof(vtss_routing_entry_t));
    route->type = VTSS_ROUTING_ENTRY_TYPE_INVALID;
    VTSS_RC(vtss_build_ipv6_network(&net, address, mask));

    route->vlan = vid;
    route->type = VTSS_ROUTING_ENTRY_TYPE_IPV6_UC;
    memcpy(&route->route.ipv6_uc.network.address, &net.address.addr.ipv6, sizeof(vtss_ipv6_t));
    route->route.ipv6_uc.network.prefix_size = net.prefix_size;
    if (gateway) {
        memcpy(&route->route.ipv6_uc.destination, gateway, sizeof(vtss_ipv6_t));
    }

    return VTSS_RC_OK;
}

// Pure function, no side effects
int vtss_if_link_flag_to_txt(char *buf, int size, vtss_if_link_flag_t f)
{
    int s = 0;
    BOOL first = TRUE;

    /*lint --e{438} */
#define F(X)                           \
    if (f & VTSS_IF_LINK_FLAG_ ##X) {  \
        if (first) {                   \
            first = FALSE;             \
            PRINTF(#X);                \
        } else {                       \
            PRINTF(" " #X);            \
        }                              \
    }

    *buf = 0;
    F(UP);
    F(BROADCAST);
    F(LOOPBACK);
    F(RUNNING);
    F(NOARP);
    F(PROMISC);
    F(MULTICAST);
#undef F

    buf[MIN(size - 1, s)] = 0;
    return s;
}

// Pure function, no side effects
int vtss_if_ipv6_flag_to_txt(char *buf, int size, vtss_if_ipv6_flag_t f)
{
    int s = 0;
    BOOL first = TRUE;

    /*lint --e{438} */
#define F(X)                            \
    if (f & VTSS_IF_IPV6_FLAG_ ##X) {   \
        if (first) {                    \
            first = FALSE;              \
            PRINTF(#X);                 \
        } else {                        \
            PRINTF(" " #X);             \
        }                               \
    }

    *buf = 0;
    F(ANYCAST);
    F(TENTATIVE);
    F(DUPLICATED);
    F(DETACHED);
    F(DEPRECATED);
    F(NODAD);
    F(AUTOCONF);
    F(TEMPORARY);
    F(HOME);
#undef F

    buf[MIN(size - 1, s)] = 0;
    return s;
}

// Pure function, no side effects
int vtss_routing_flags_to_txt(char *buf, int size, vtss_routing_flags_t f)
{
    int s = 0;
    BOOL first = TRUE;

    /*lint --e{438} */
#define F(X)                           \
    if (f & VTSS_ROUTING_FLAG_ ##X) {  \
        if (first) {                   \
            PRINTF(#X);                \
        } else {                       \
            PRINTF(" " #X);            \
        }                              \
        first = FALSE;                 \
    }

    *buf = 0;
    F(UP);
    F(HOST);
    F(GATEWAY);
    F(REJECT);
    F(HW_RT);
#undef F

    buf[MIN(size - 1, s)] = 0;
    return s;
}

void vtss_ip2_if_id_vlan(vtss_vid_t vid, vtss_if_id_t  *if_id)
{
    if_id->type = VTSS_ID_IF_TYPE_VLAN;
    if_id->u.vlan = vid;
}

BOOL vtss_ip_equal(const vtss_ip_network_t *const a,
                   const vtss_ip_network_t *const b)
{
    if (a->address.type != b->address.type) {
        return FALSE;
    }

    switch (a->address.type) {
    case VTSS_IP_TYPE_IPV4:
        return a->address.addr.ipv4 == b->address.addr.ipv4;
    case VTSS_IP_TYPE_IPV6:
        return memcmp(a->address.addr.ipv6.addr,
                      b->address.addr.ipv6.addr,
                      sizeof(a->address.addr.ipv6.addr)) == 0;
    default:
        ;
    }

    return TRUE;
}

BOOL vtss_ipv4_network_equal(       const vtss_ipv4_network_t   *const a,
                                    const vtss_ipv4_network_t   *const b)
{
    if (a->prefix_size != b->prefix_size) {
        return FALSE;
    }

    return a->address == b->address;
}

BOOL vtss_ipv6_network_equal(       const vtss_ipv6_network_t   *const a,
                                    const vtss_ipv6_network_t   *const b)
{
    if (a->prefix_size != b->prefix_size) {
        return FALSE;
    }

    return memcmp(a->address.addr,
                  b->address.addr,
                  sizeof(a->address.addr)) == 0;
}

BOOL vtss_ip_network_equal(const vtss_ip_network_t     *const a,
                           const vtss_ip_network_t     *const b)
{
    // We may compare prefix size regardsless of type
    if (a->prefix_size != b->prefix_size) {
        return FALSE;
    }

    return vtss_ip_equal(a, b);
}

BOOL vtss_ipv4_net_equal(const vtss_ipv4_network_t   *const a,
                         const vtss_ipv4_network_t   *const b)
{
    vtss_ipv4_t mask = (u32) - 1;
    if (a->prefix_size != b->prefix_size) {
        return FALSE;
    }

    (void) vtss_conv_prefix_to_ipv4mask(a->prefix_size, &mask);
    return (a->address & mask) == (b->address & mask);
}

BOOL vtss_ip_net_equal(const vtss_ip_network_t *const a,
                       const vtss_ip_network_t *const b)
{
    if (a->address.type != b->address.type) {
        return FALSE;
    }

    switch (a->address.type) {
    case VTSS_IP_TYPE_IPV4: {
        vtss_ipv4_t mask = (u32) - 1;
        if (a->prefix_size == b->prefix_size &&
            vtss_conv_prefix_to_ipv4mask(a->prefix_size, &mask) == VTSS_OK) {
            return (a->address.addr.ipv4 & mask) == (b->address.addr.ipv4 & mask);
        }
        return FALSE;
    }
    case VTSS_IP_TYPE_IPV6: {
        vtss_ipv6_t mask;
        if (a->prefix_size == b->prefix_size &&
            vtss_conv_prefix_to_ipv6mask(a->prefix_size, &mask) == VTSS_OK) {
            size_t i;
            for (i = 0; i < sizeof(mask.addr); i++) {
                u8 maskb = mask.addr[i];
                /* Compare network (masked) only */
                if ((a->address.addr.ipv6.addr[i] & maskb) != (b->address.addr.ipv6.addr[i] & maskb)) {
                    return FALSE;
                }
            }
            return TRUE;
        }
        return FALSE;
    }
    default:
        ;
    }

    return FALSE;
}

static void operator_ipv6_and(const vtss_ipv6_t *const a,
                              const vtss_ipv6_t *const b,
                              vtss_ipv6_t       *const res)
{
    int i;
    for (i = 0; i < 16; ++i) {
        res->addr[i] = a->addr[i] & b->addr[i];
    }
}

static BOOL operator_ipv6_equal(const vtss_ipv6_t *const a,
                                const vtss_ipv6_t *const b)
{
    int i;
    for (i = 0; i < 16; ++i) {
        if (a->addr[i] != b->addr[i]) {
            return FALSE;
        }
    }
    return TRUE;
}

BOOL vtss_ipv4_net_overlap(const vtss_ipv4_network_t   *const a,
                           const vtss_ipv4_network_t   *const b)
{
    vtss_ipv4_t mask_a = 0, mask_b = 0;

    (void) vtss_conv_prefix_to_ipv4mask(a->prefix_size, &mask_a);
    (void) vtss_conv_prefix_to_ipv4mask(b->prefix_size, &mask_b);

    return (a->address & mask_a) == (b->address & mask_a) ||
           (b->address & mask_b) == (a->address & mask_b);
}

BOOL vtss_ipv6_net_overlap(const vtss_ipv6_network_t   *const a,
                           const vtss_ipv6_network_t   *const b)
{
    vtss_ipv6_t mask_a, mask_b;
    vtss_ipv6_t a_mask_a;
    vtss_ipv6_t a_mask_b;
    vtss_ipv6_t b_mask_a;
    vtss_ipv6_t b_mask_b;

    (void) vtss_conv_prefix_to_ipv6mask(a->prefix_size, &mask_a);
    (void) vtss_conv_prefix_to_ipv6mask(b->prefix_size, &mask_b);

    operator_ipv6_and(&a->address, &mask_a, &a_mask_a);
    operator_ipv6_and(&a->address, &mask_b, &a_mask_b);
    operator_ipv6_and(&b->address, &mask_a, &b_mask_a);
    operator_ipv6_and(&b->address, &mask_b, &b_mask_b);

    return operator_ipv6_equal(&a_mask_a, &b_mask_a) ||
           operator_ipv6_equal(&b_mask_b, &a_mask_b);
}

BOOL vtss_ip_net_overlap(const vtss_ip_network_t   *const a,
                         const vtss_ip_network_t   *const b)
{
    if (a->address.type != b->address.type) {
        return false;
    }

    switch (a->address.type) {
    case VTSS_IP_TYPE_IPV4: {
        vtss_ipv4_network_t _a, _b;
        _a.prefix_size = a->prefix_size;
        _a.address = a->address.addr.ipv4;
        _b.prefix_size = b->prefix_size;
        _b.address = b->address.addr.ipv4;
        return vtss_ipv4_net_overlap(&_a, &_b);
    }

    case VTSS_IP_TYPE_IPV6: {
        vtss_ipv6_network_t _a, _b;
        _a.prefix_size = a->prefix_size;
        _a.address = a->address.addr.ipv6;
        _b.prefix_size = b->prefix_size;
        _b.address = b->address.addr.ipv6;
        return vtss_ipv6_net_overlap(&_a, &_b);
    }

    default:
        return false;
    }
}

vtss_rc vtss_if_status_to_ip(const vtss_if_status_t *const status,
                             vtss_ip_addr_t         *const ip)
{
    if (status->type == VTSS_IF_STATUS_TYPE_IPV4) {
        ip->type = VTSS_IP_TYPE_IPV4;
        ip->addr.ipv4 = status->u.ipv4.net.address;
        return VTSS_RC_OK;
    }

    if (status->type == VTSS_IF_STATUS_TYPE_IPV6) {
        ip->type = VTSS_IP_TYPE_IPV6;
        memcpy(ip->addr.ipv6.addr, status->u.ipv6.net.address.addr, 16);
        return VTSS_RC_OK;
    }

    return VTSS_RC_ERROR;
}


BOOL vtss_if_status_match_vlan(vtss_vid_t        vlan,
                               vtss_if_status_t *if_status)
{
    if (if_status->if_id.type != VTSS_ID_IF_TYPE_VLAN) {
        return FALSE;
    }

    if (if_status->if_id.u.vlan == vlan) {
        return TRUE;
    }

    return FALSE;
}

BOOL vtss_if_status_match_ip_type(vtss_ip_type_t    type,
                                  vtss_if_status_t *if_status)
{
    if (type == VTSS_IP_TYPE_IPV4 &&
        if_status->type == VTSS_IF_STATUS_TYPE_IPV4) {
        return TRUE;
    }

    if (type == VTSS_IP_TYPE_IPV6 &&
        if_status->type == VTSS_IF_STATUS_TYPE_IPV6 ) {
        return TRUE;
    }

    return FALSE;
}

BOOL vtss_if_status_match_ip(const vtss_ip_addr_t *const ip,
                             vtss_if_status_t     *if_status)
{
    if (ip->type == VTSS_IP_TYPE_IPV4 &&
        if_status->type == VTSS_IF_STATUS_TYPE_IPV4 &&
        if_status->u.ipv4.net.address == ip->addr.ipv4) {
        return TRUE;
    }

    if (ip->type == VTSS_IP_TYPE_IPV6 &&
        if_status->type == VTSS_IF_STATUS_TYPE_IPV6 &&
        memcmp(if_status->u.ipv6.net.address.addr,
               ip->addr.ipv6.addr, 16) == 0) {
        return TRUE;
    }

    return FALSE;
}

void vtss_ipv4_default_route(vtss_ipv4_t gw, vtss_routing_entry_t *rt)
{
    rt->type = VTSS_ROUTING_ENTRY_TYPE_IPV4_UC;
    rt->route.ipv4_uc.network.address = 0;
    rt->route.ipv4_uc.network.prefix_size = 0;
    rt->route.ipv4_uc.destination = gw;
}

BOOL vtss_ip_route_valid(const vtss_routing_entry_t  *const rt)
{
    switch (rt->type) {
    case VTSS_ROUTING_ENTRY_TYPE_IPV4_UC: {
        vtss_ipv4_t mask = 0;
        if (rt->route.ipv4_uc.network.prefix_size <= 30 && /* 31&32 illegal */
            vtss_conv_prefix_to_ipv4mask(rt->route.ipv4_uc.network.prefix_size, &mask) == VTSS_OK) {
            if (rt->route.ipv4_uc.network.address & ~mask) {
                /* has bits set outside prefix */
                return FALSE;
            }
            return TRUE;
        }
        break;
    }
    case VTSS_ROUTING_ENTRY_TYPE_IPV6_UC:
        if (rt->route.ipv6_uc.network.prefix_size <= 128) {
            return TRUE;
        }
        break;
    default:
        ;                       /* Handled below */
    }
    return FALSE;
}

BOOL vtss_ip_route_nh_valid(const vtss_routing_entry_t *const rt)
{
    if (!rt) {
        return FALSE;
    }

    switch (rt->type) {
    case VTSS_ROUTING_ENTRY_TYPE_IPV4_UC:
    case VTSS_ROUTING_ENTRY_TYPE_IPV4_MC:
        if (!vtss_ipv4_addr_is_zero(rt->route.ipv4_uc.destination) &&
            !vtss_ipv4_addr_is_multicast(&rt->route.ipv4_uc.destination)) {
            return TRUE;
        }
        break;
    case VTSS_ROUTING_ENTRY_TYPE_IPV6_UC:
        if (!vtss_ipv6_addr_is_zero(&rt->route.ipv6_uc.destination) &&
            !vtss_ipv6_addr_is_multicast(&rt->route.ipv6_uc.destination)) {
            return TRUE;
        }
        break;
    default:
        break;
    }

    return FALSE;
}

BOOL vtss_ip_ifaddr_valid(const vtss_ip_network_t *const net)
{

    if (net->address.type == VTSS_IP_TYPE_IPV4) {
        vtss_ipv4_t host = net->address.addr.ipv4, mask = 0;

        /* IPMC/BCast check */
        if (((host >> 24) & 0xff) >= 224) {
            return FALSE;
        }

        /* Prefix check */
        if (net->prefix_size <= 30 &&
            vtss_conv_prefix_to_ipv4mask(net->prefix_size, &mask) == VTSS_OK) {
            /* Host part cheks */
            if ((host & ~mask) != (((u32) - 1) & ~mask) &&
                (host & ~mask) != 0) {
                return TRUE;    /* Not using subnet bcast/zero host part */
            }
        }
    } else if (net->address.type == VTSS_IP_TYPE_IPV6) {
        if (net->prefix_size <= 128 &&
            vtss_ipv6_addr_is_mgmt_support(&net->address.addr.ipv6) &&
            !vtss_ip_addr_is_zero(&net->address) &&
            !vtss_ipv6_addr_is_multicast(&net->address.addr.ipv6)) {
            return TRUE;    /* Not using subnet bcast/zero host part */
        }
    }

    return FALSE;               /* Bad address */
}

BOOL vtss_if_id_equal(const vtss_if_id_t          *const a,
                      const vtss_if_id_t          *const b)
{
    if (memcmp(a, b, sizeof(vtss_if_id_t )) == 0) {
        return TRUE;
    } else {
        return FALSE;
    }
}

int vtss_ip2_ip_addr_to_txt(char                 *buf,
                            int                   size,
                            const vtss_ip_addr_t *const ip)
{
    int s = 0;
    char _buf[41];

    switch (ip->type) {
    case VTSS_IP_TYPE_NONE:
        PRINTF("NONE");
        break;
    case VTSS_IP_TYPE_IPV4:
        PRINTF(VTSS_IPV4_FORMAT, VTSS_IPV4_ARGS(ip->addr.ipv4));
        break;
    case VTSS_IP_TYPE_IPV6:
        (void)misc_ipv6_txt(&(ip->addr.ipv6), _buf);
        _buf[40] = 0;
        PRINTF("%s", _buf);
        break;
    default:
        PRINTF("Unknown-type:%u", ip->type);
    }

    return s;
}

int vtss_ip2_ip_network_to_txt(char                        *buf,
                               int                          size,
                               const vtss_ip_network_t     *const ip)
{
    int s = 0;
    PRINTFUNC(vtss_ip2_ip_addr_to_txt, &ip->address);
    PRINTF("/%d", ip->prefix_size);
    return s;
}

int vtss_ip2_neighbour_to_txt(char                   *buf,
                              int                     size,
                              const vtss_neighbour_t *const nb)
{
    int s = 0;
    PRINTF("{DMAC: "VTSS_MAC_FORMAT" VLAN: %u IP: ",
           VTSS_MAC_ARGS(nb->dmac), nb->vlan);
    PRINTFUNC(vtss_ip2_ip_addr_to_txt, &(nb->dip));
    PRINTF("}");
    return s;
}

int vtss_ip2_route_entry_to_txt(char                        *buf,
                                int                          size,
                                const vtss_routing_entry_t  *const rt)
{
    int s = 0;
    char _buf[41];

    switch (rt->type) {
    case VTSS_ROUTING_ENTRY_TYPE_IPV4_UC:
        PRINTF(VTSS_IPV4N_FORMAT" via ",
               VTSS_IPV4N_ARG(rt->route.ipv4_uc.network));
        PRINTF(VTSS_IPV4_FORMAT,
               VTSS_IPV4_ARGS(rt->route.ipv4_uc.destination));
        break;

    case VTSS_ROUTING_ENTRY_TYPE_IPV6_UC:
        (void)misc_ipv6_txt(&(rt->route.ipv6_uc.network.address), _buf);
        _buf[40] = 0;
        PRINTF("%s/%d via ",
               _buf, rt->route.ipv6_uc.network.prefix_size);
        (void)misc_ipv6_txt(&(rt->route.ipv6_uc.destination), _buf);
        _buf[40] = 0;
        PRINTF("%s", _buf);
        break;

    case VTSS_ROUTING_ENTRY_TYPE_IPV4_MC:
        PRINTF("IPv4-MC-not-implemented");
        break;

    case VTSS_ROUTING_ENTRY_TYPE_INVALID:
        PRINTF("Invalid");
        break;

    default:
        PRINTF("Unknown-type:%u", rt->type);
        break;
    }

    return s;
}


int vtss_ip2_if_id_to_txt(char *buf, int size,
                          const vtss_if_id_t  *const if_id)
{
    int s = 0;

    switch (if_id->type) {
    case VTSS_ID_IF_TYPE_INVALID:
        PRINTF("invalid");
        break;

    case VTSS_ID_IF_TYPE_VLAN:
        PRINTF("VLAN%u", if_id->u.vlan);
        break;

    case VTSS_ID_IF_TYPE_OS_ONLY:
        PRINTF("OS:%s", if_id->u.os.name);
        break;

    default:
        PRINTF("unknown");
        break;
    }

    buf[MIN(size - 1, s)] = 0;
    return s;
}

int vtss_ip2_if_status_to_txt(char             *buf,
                              int               size,
                              vtss_if_status_t *st,
                              u32               length)
{
    /*lint --e{429} */

    u32 i;
    int s = 0;
#if defined(VTSS_SW_OPTION_IPV6)
    char _buf[128];
#endif

    if (length == 0) {
        return 0;
    }

    PRINTFUNC(vtss_ip2_if_id_to_txt, &(st->if_id));
    PRINTF("\n");

    for (i = 0; i < length; ++i, ++st) {
        switch (st->type) {
        case VTSS_IF_STATUS_TYPE_LINK:
            PRINTF("  LINK: "VTSS_MAC_FORMAT" Mtu:%u <",
                   VTSS_MAC_ARGS(st->u.link.mac),
                   st->u.link.mtu);
            PRINTFUNC(vtss_if_link_flag_to_txt, st->u.link.flags);
            PRINTF(">\n");
            break;

        case VTSS_IF_STATUS_TYPE_LINK_STAT:
            PRINTF("  LINK_STAT:""\n");
            break;

        case VTSS_IF_STATUS_TYPE_IPV4:
            PRINTF("  IPv4: "VTSS_IPV4N_FORMAT,
                   VTSS_IPV4N_ARG(st->u.ipv4.net));
            PRINTF(" "VTSS_IPV4_FORMAT"\n",
                   VTSS_IPV4_ARGS(st->u.ipv4.broadcast));
            break;

        case VTSS_IF_STATUS_TYPE_DHCP:
            PRINTF("  DHCP: ");
            PRINTFUNC(vtss_dhcp4c_status_to_txt, &st->u.dhcp4c);
            PRINTF("\n");
            break;

        case VTSS_IF_STATUS_TYPE_IPV6:
#if defined(VTSS_SW_OPTION_IPV6)
            (void)misc_ipv6_txt(&st->u.ipv6.net.address, _buf);
            PRINTF("  IPv6: %s/%d", _buf, st->u.ipv6.net.prefix_size);
            PRINTF(" <");
            PRINTFUNC(vtss_if_ipv6_flag_to_txt, st->u.ipv6.flags);
            PRINTF(">\n");
#endif  /* VTSS_SW_OPTION_IPV6 */
            break;

        default:
            PRINTF("  UNKNOWN\n");
        }
    }

    buf[MIN(size - 1, s)] = 0;
    return s;
}

int vtss_ip2_neighbour_status_to_txt(char                    *buf,
                                     int                      size,
                                     vtss_neighbour_status_t *st)
{
    int s = 0;

    PRINTFUNC(vtss_ip2_ip_addr_to_txt, &st->ip_address);

    if (st->flags & VTSS_NEIGHBOUR_FLAG_VALID) {
        PRINTF(" via ");
        PRINTFUNC(vtss_ip2_if_id_to_txt, &(st->interface));
        PRINTF(":" VTSS_MAC_FORMAT, VTSS_MAC_ARGS(st->mac_address));
    } else {
        if (st->interface.type == VTSS_ID_IF_TYPE_OS_ONLY) {
            PRINTF(" via ");
            PRINTFUNC(vtss_ip2_if_id_to_txt, &(st->interface));
        } else {
            PRINTF(" (Incomplete)");
        }
    }

    if (st->state[0]) {
        PRINTF(" %s", st->state);
    }
    if (st->flags & VTSS_NEIGHBOUR_FLAG_PERMANENT) {
        PRINTF(" Permanent");
    }
    if (st->flags & VTSS_NEIGHBOUR_FLAG_ROUTER) {
        PRINTF(" Router");
    }
    if (st->flags & VTSS_NEIGHBOUR_FLAG_HARDWARE) {
        PRINTF(" Hardware");
    }

    return s;
}

int vtss_ip2_stat_icmp_type_txt(     char                       *buf,
                                     int                        size,
                                     const vtss_ip_type_t       version,
                                     const u32                  icmp_type)
{
    int s = 0;

    switch ( icmp_type ) {
    case 0:
        if (version == VTSS_IP_TYPE_IPV4) {
            PRINTF("Echo Reply");
        } else if (version == VTSS_IP_TYPE_IPV6) {
            PRINTF("Unassigned");
        } else {
            PRINTF("Invalid!");
        }
        break;
    case 1:
        if (version == VTSS_IP_TYPE_IPV4) {
            PRINTF("Reserved");
        } else if (version == VTSS_IP_TYPE_IPV6) {
            PRINTF("Destination Unreachable");
        } else {
            PRINTF("Invalid!");
        }
        break;
    case 2:
        if (version == VTSS_IP_TYPE_IPV4) {
            PRINTF("Reserved");
        } else if (version == VTSS_IP_TYPE_IPV6) {
            PRINTF("Packet Too Big");
        } else {
            PRINTF("Invalid!");
        }
        break;
    case 3:
        if (version == VTSS_IP_TYPE_IPV4) {
            PRINTF("Destination Unreachable");
        } else if (version == VTSS_IP_TYPE_IPV6) {
            PRINTF("Time Exceeded");
        } else {
            PRINTF("Invalid!");
        }
        break;
    case 4:
        if (version == VTSS_IP_TYPE_IPV4) {
            PRINTF("Source Quench (Deprecated)");
        } else if (version == VTSS_IP_TYPE_IPV6) {
            PRINTF("Parameter Problem");
        } else {
            PRINTF("Invalid!");
        }
        break;
    case 5:
        if (version == VTSS_IP_TYPE_IPV4) {
            PRINTF("Redirect");
        } else if (version == VTSS_IP_TYPE_IPV6) {
            PRINTF("Unassigned");
        } else {
            PRINTF("Invalid!");
        }
        break;
    case 6:
        if (version == VTSS_IP_TYPE_IPV4) {
            PRINTF("Alternate Host Address");
        } else if (version == VTSS_IP_TYPE_IPV6) {
            PRINTF("Unassigned");
        } else {
            PRINTF("Invalid!");
        }
        break;
    case 7:
        if (version == VTSS_IP_TYPE_IPV4) {
            PRINTF("Unassigned");
        } else if (version == VTSS_IP_TYPE_IPV6) {
            PRINTF("Unassigned");
        } else {
            PRINTF("Invalid!");
        }
        break;
    case 8:
        if (version == VTSS_IP_TYPE_IPV4) {
            PRINTF("Echo");
        } else if (version == VTSS_IP_TYPE_IPV6) {
            PRINTF("Unassigned");
        } else {
            PRINTF("Invalid!");
        }
        break;
    case 9:
        if (version == VTSS_IP_TYPE_IPV4) {
            PRINTF("Router Advertisement");
        } else if (version == VTSS_IP_TYPE_IPV6) {
            PRINTF("Unassigned");
        } else {
            PRINTF("Invalid!");
        }
        break;
    case 10:
        if (version == VTSS_IP_TYPE_IPV4) {
            PRINTF("Router Solicitation");
        } else if (version == VTSS_IP_TYPE_IPV6) {
            PRINTF("Unassigned");
        } else {
            PRINTF("Invalid!");
        }
        break;
    case 11:
        if (version == VTSS_IP_TYPE_IPV4) {
            PRINTF("Time Exceeded");
        } else if (version == VTSS_IP_TYPE_IPV6) {
            PRINTF("Unassigned");
        } else {
            PRINTF("Invalid!");
        }
        break;
    case 12:
        if (version == VTSS_IP_TYPE_IPV4) {
            PRINTF("Parameter Problem");
        } else if (version == VTSS_IP_TYPE_IPV6) {
            PRINTF("Unassigned");
        } else {
            PRINTF("Invalid!");
        }
        break;
    case 13:
        if (version == VTSS_IP_TYPE_IPV4) {
            PRINTF("Timestamp");
        } else if (version == VTSS_IP_TYPE_IPV6) {
            PRINTF("Unassigned");
        } else {
            PRINTF("Invalid!");
        }
        break;
    case 14:
        if (version == VTSS_IP_TYPE_IPV4) {
            PRINTF("Timestamp Reply");
        } else if (version == VTSS_IP_TYPE_IPV6) {
            PRINTF("Unassigned");
        } else {
            PRINTF("Invalid!");
        }
        break;
    case 15:
        if (version == VTSS_IP_TYPE_IPV4) {
            PRINTF("Information Request");
        } else if (version == VTSS_IP_TYPE_IPV6) {
            PRINTF("Unassigned");
        } else {
            PRINTF("Invalid!");
        }
        break;
    case 16:
        if (version == VTSS_IP_TYPE_IPV4) {
            PRINTF("Information Reply");
        } else if (version == VTSS_IP_TYPE_IPV6) {
            PRINTF("Unassigned");
        } else {
            PRINTF("Invalid!");
        }
        break;
    case 17:
        if (version == VTSS_IP_TYPE_IPV4) {
            PRINTF("Address Mask Request");
        } else if (version == VTSS_IP_TYPE_IPV6) {
            PRINTF("Unassigned");
        } else {
            PRINTF("Invalid!");
        }
        break;
    case 18:
        if (version == VTSS_IP_TYPE_IPV4) {
            PRINTF("Address Mask Reply");
        } else if (version == VTSS_IP_TYPE_IPV6) {
            PRINTF("Unassigned");
        } else {
            PRINTF("Invalid!");
        }
        break;
    case 19:
        if (version == VTSS_IP_TYPE_IPV4) {
            PRINTF("Reserved (for Security)");
        } else if (version == VTSS_IP_TYPE_IPV6) {
            PRINTF("Unassigned");
        } else {
            PRINTF("Invalid!");
        }
        break;
    case 30:
        if (version == VTSS_IP_TYPE_IPV4) {
            PRINTF("Traceroute");
        } else if (version == VTSS_IP_TYPE_IPV6) {
            PRINTF("Unassigned");
        } else {
            PRINTF("Invalid!");
        }
        break;
    case 31:
        if (version == VTSS_IP_TYPE_IPV4) {
            PRINTF("Datagram Conversion Error");
        } else if (version == VTSS_IP_TYPE_IPV6) {
            PRINTF("Unassigned");
        } else {
            PRINTF("Invalid!");
        }
        break;
    case 32:
        if (version == VTSS_IP_TYPE_IPV4) {
            PRINTF("Mobile Host Redirect");
        } else if (version == VTSS_IP_TYPE_IPV6) {
            PRINTF("Unassigned");
        } else {
            PRINTF("Invalid!");
        }
        break;
    case 33:
        if (version == VTSS_IP_TYPE_IPV4) {
            PRINTF("IPv6 Where-Are-You");
        } else if (version == VTSS_IP_TYPE_IPV6) {
            PRINTF("Unassigned");
        } else {
            PRINTF("Invalid!");
        }
        break;
    case 34:
        if (version == VTSS_IP_TYPE_IPV4) {
            PRINTF("IPv6 I-Am-Here");
        } else if (version == VTSS_IP_TYPE_IPV6) {
            PRINTF("Unassigned");
        } else {
            PRINTF("Invalid!");
        }
        break;
    case 35:
        if (version == VTSS_IP_TYPE_IPV4) {
            PRINTF("Mobile Registration Request");
        } else if (version == VTSS_IP_TYPE_IPV6) {
            PRINTF("Unassigned");
        } else {
            PRINTF("Invalid!");
        }
        break;
    case 36:
        if (version == VTSS_IP_TYPE_IPV4) {
            PRINTF("Mobile Registration Reply");
        } else if (version == VTSS_IP_TYPE_IPV6) {
            PRINTF("Unassigned");
        } else {
            PRINTF("Invalid!");
        }
        break;
    case 37:
        if (version == VTSS_IP_TYPE_IPV4) {
            PRINTF("Domain Name Request");
        } else if (version == VTSS_IP_TYPE_IPV6) {
            PRINTF("Unassigned");
        } else {
            PRINTF("Invalid!");
        }
        break;
    case 38:
        if (version == VTSS_IP_TYPE_IPV4) {
            PRINTF("Domain Name Reply");
        } else if (version == VTSS_IP_TYPE_IPV6) {
            PRINTF("Unassigned");
        } else {
            PRINTF("Invalid!");
        }
        break;
    case 39:
        if (version == VTSS_IP_TYPE_IPV4) {
            PRINTF("SKIP");
        } else if (version == VTSS_IP_TYPE_IPV6) {
            PRINTF("Unassigned");
        } else {
            PRINTF("Invalid!");
        }
        break;
    case 40:
        if (version == VTSS_IP_TYPE_IPV4) {
            PRINTF("Photuris");
        } else if (version == VTSS_IP_TYPE_IPV6) {
            PRINTF("Unassigned");
        } else {
            PRINTF("Invalid!");
        }
        break;
    case 41:
        if (version == VTSS_IP_TYPE_IPV4) {
            PRINTF("ICMP messages utilized by experimental mobility protocols");
        } else if (version == VTSS_IP_TYPE_IPV6) {
            PRINTF("Unassigned");
        } else {
            PRINTF("Invalid!");
        }
        break;
    case 100:
        if (version == VTSS_IP_TYPE_IPV4) {
            PRINTF("Unassigned");
        } else if (version == VTSS_IP_TYPE_IPV6) {
            PRINTF("Private experimentation");
        } else {
            PRINTF("Invalid!");
        }
        break;
    case 101:
        if (version == VTSS_IP_TYPE_IPV4) {
            PRINTF("Unassigned");
        } else if (version == VTSS_IP_TYPE_IPV6) {
            PRINTF("Private experimentation");
        } else {
            PRINTF("Invalid!");
        }
        break;
    case 127:
        if (version == VTSS_IP_TYPE_IPV4) {
            PRINTF("Unassigned");
        } else if (version == VTSS_IP_TYPE_IPV6) {
            PRINTF("Reserved for expansion of ICMPv6 error messages");
        } else {
            PRINTF("Invalid!");
        }
        break;
    case 128:
        if (version == VTSS_IP_TYPE_IPV4) {
            PRINTF("Unassigned");
        } else if (version == VTSS_IP_TYPE_IPV6) {
            PRINTF("Echo Request");
        } else {
            PRINTF("Invalid!");
        }
        break;
    case 129:
        if (version == VTSS_IP_TYPE_IPV4) {
            PRINTF("Unassigned");
        } else if (version == VTSS_IP_TYPE_IPV6) {
            PRINTF("Echo Reply");
        } else {
            PRINTF("Invalid!");
        }
        break;
    case 130:
        if (version == VTSS_IP_TYPE_IPV4) {
            PRINTF("Unassigned");
        } else if (version == VTSS_IP_TYPE_IPV6) {
            PRINTF("Multicast Listener Query");
        } else {
            PRINTF("Invalid!");
        }
        break;
    case 131:
        if (version == VTSS_IP_TYPE_IPV4) {
            PRINTF("Unassigned");
        } else if (version == VTSS_IP_TYPE_IPV6) {
            PRINTF("Multicast Listener Report");
        } else {
            PRINTF("Invalid!");
        }
        break;
    case 132:
        if (version == VTSS_IP_TYPE_IPV4) {
            PRINTF("Unassigned");
        } else if (version == VTSS_IP_TYPE_IPV6) {
            PRINTF("Multicast Listener Done");
        } else {
            PRINTF("Invalid!");
        }
        break;
    case 133:
        if (version == VTSS_IP_TYPE_IPV4) {
            PRINTF("Unassigned");
        } else if (version == VTSS_IP_TYPE_IPV6) {
            PRINTF("Router Solicitation (NDP)");
        } else {
            PRINTF("Invalid!");
        }
        break;
    case 134:
        if (version == VTSS_IP_TYPE_IPV4) {
            PRINTF("Unassigned");
        } else if (version == VTSS_IP_TYPE_IPV6) {
            PRINTF("Router Advertisement (NDP)");
        } else {
            PRINTF("Invalid!");
        }
        break;
    case 135:
        if (version == VTSS_IP_TYPE_IPV4) {
            PRINTF("Unassigned");
        } else if (version == VTSS_IP_TYPE_IPV6) {
            PRINTF("Neighbor Solicitation (NDP)");
        } else {
            PRINTF("Invalid!");
        }
        break;
    case 136:
        if (version == VTSS_IP_TYPE_IPV4) {
            PRINTF("Unassigned");
        } else if (version == VTSS_IP_TYPE_IPV6) {
            PRINTF("Neighbor Advertisement (NDP)");
        } else {
            PRINTF("Invalid!");
        }
        break;
    case 137:
        if (version == VTSS_IP_TYPE_IPV4) {
            PRINTF("Unassigned");
        } else if (version == VTSS_IP_TYPE_IPV6) {
            PRINTF("Redirect Message (NDP)");
        } else {
            PRINTF("Invalid!");
        }
        break;
    case 138:
        if (version == VTSS_IP_TYPE_IPV4) {
            PRINTF("Unassigned");
        } else if (version == VTSS_IP_TYPE_IPV6) {
            PRINTF("Router Renumbering");
        } else {
            PRINTF("Invalid!");
        }
        break;
    case 139:
        if (version == VTSS_IP_TYPE_IPV4) {
            PRINTF("Unassigned");
        } else if (version == VTSS_IP_TYPE_IPV6) {
            PRINTF("ICMP Node Information Query");
        } else {
            PRINTF("Invalid!");
        }
        break;
    case 140:
        if (version == VTSS_IP_TYPE_IPV4) {
            PRINTF("Unassigned");
        } else if (version == VTSS_IP_TYPE_IPV6) {
            PRINTF("ICMP Node Information Response");
        } else {
            PRINTF("Invalid!");
        }
        break;
    case 141:
        if (version == VTSS_IP_TYPE_IPV4) {
            PRINTF("Unassigned");
        } else if (version == VTSS_IP_TYPE_IPV6) {
            PRINTF("Inverse Neighbor Discovery Solicitation Message");
        } else {
            PRINTF("Invalid!");
        }
        break;
    case 142:
        if (version == VTSS_IP_TYPE_IPV4) {
            PRINTF("Unassigned");
        } else if (version == VTSS_IP_TYPE_IPV6) {
            PRINTF("Inverse Neighbor Discovery Advertisement Message");
        } else {
            PRINTF("Invalid!");
        }
        break;
    case 143:
        if (version == VTSS_IP_TYPE_IPV4) {
            PRINTF("Unassigned");
        } else if (version == VTSS_IP_TYPE_IPV6) {
            PRINTF("Multicast Listener Discovery (MLDv2) reports");
        } else {
            PRINTF("Invalid!");
        }
        break;
    case 144:
        if (version == VTSS_IP_TYPE_IPV4) {
            PRINTF("Unassigned");
        } else if (version == VTSS_IP_TYPE_IPV6) {
            PRINTF("Home Agent Address Discovery Request Message");
        } else {
            PRINTF("Invalid!");
        }
        break;
    case 145:
        if (version == VTSS_IP_TYPE_IPV4) {
            PRINTF("Unassigned");
        } else if (version == VTSS_IP_TYPE_IPV6) {
            PRINTF("Home Agent Address Discovery Reply Message");
        } else {
            PRINTF("Invalid!");
        }
        break;
    case 146:
        if (version == VTSS_IP_TYPE_IPV4) {
            PRINTF("Unassigned");
        } else if (version == VTSS_IP_TYPE_IPV6) {
            PRINTF("Mobile Prefix Solicitation");
        } else {
            PRINTF("Invalid!");
        }
        break;
    case 147:
        if (version == VTSS_IP_TYPE_IPV4) {
            PRINTF("Unassigned");
        } else if (version == VTSS_IP_TYPE_IPV6) {
            PRINTF("Mobile Prefix Advertisement");
        } else {
            PRINTF("Invalid!");
        }
        break;
    case 148:
        if (version == VTSS_IP_TYPE_IPV4) {
            PRINTF("Unassigned");
        } else if (version == VTSS_IP_TYPE_IPV6) {
            PRINTF("Certification Path Solicitation (SEND)");
        } else {
            PRINTF("Invalid!");
        }
        break;
    case 149:
        if (version == VTSS_IP_TYPE_IPV4) {
            PRINTF("Unassigned");
        } else if (version == VTSS_IP_TYPE_IPV6) {
            PRINTF("Certification Path Advertisement (SEND)");
        } else {
            PRINTF("Invalid!");
        }
        break;
    case 151:
        if (version == VTSS_IP_TYPE_IPV4) {
            PRINTF("Unassigned");
        } else if (version == VTSS_IP_TYPE_IPV6) {
            PRINTF("Multicast Router Advertisement (MRD)");
        } else {
            PRINTF("Invalid!");
        }
        break;
    case 152:
        if (version == VTSS_IP_TYPE_IPV4) {
            PRINTF("Unassigned");
        } else if (version == VTSS_IP_TYPE_IPV6) {
            PRINTF("Multicast Router Solicitation (MRD)");
        } else {
            PRINTF("Invalid!");
        }
        break;
    case 153:
        if (version == VTSS_IP_TYPE_IPV4) {
            PRINTF("Unassigned");
        } else if (version == VTSS_IP_TYPE_IPV6) {
            PRINTF("Multicast Router Termination (MRD)");
        } else {
            PRINTF("Invalid!");
        }
        break;
    case 155:
        if (version == VTSS_IP_TYPE_IPV4) {
            PRINTF("Unassigned");
        } else if (version == VTSS_IP_TYPE_IPV6) {
            PRINTF("RPL Control Message");
        } else {
            PRINTF("Invalid!");
        }
        break;
    case 200:
        if (version == VTSS_IP_TYPE_IPV4) {
            PRINTF("Unassigned");
        } else if (version == VTSS_IP_TYPE_IPV6) {
            PRINTF("Private experimentation");
        } else {
            PRINTF("Invalid!");
        }
        break;
    case 201:
        if (version == VTSS_IP_TYPE_IPV4) {
            PRINTF("Unassigned");
        } else if (version == VTSS_IP_TYPE_IPV6) {
            PRINTF("Private experimentation");
        } else {
            PRINTF("Invalid!");
        }
        break;
    case 253:
        if (version == VTSS_IP_TYPE_IPV4) {
            PRINTF("RFC3692-style Experiment 1");
        } else if (version == VTSS_IP_TYPE_IPV6) {
            PRINTF("Unassigned");
        } else {
            PRINTF("Invalid!");
        }
        break;
    case 254:
        if (version == VTSS_IP_TYPE_IPV4) {
            PRINTF("RFC3692-style Experiment 2");
        } else if (version == VTSS_IP_TYPE_IPV6) {
            PRINTF("Unassigned");
        } else {
            PRINTF("Invalid!");
        }
        break;
    case 255:
        if (version == VTSS_IP_TYPE_IPV4) {
            PRINTF("Reserved");
        } else if (version == VTSS_IP_TYPE_IPV6) {
            PRINTF("Reserved for expansion of ICMPv6 informational messages");
        } else {
            PRINTF("Invalid!");
        }
        break;
    default:
        if ((version != VTSS_IP_TYPE_IPV4) &&
            (version != VTSS_IP_TYPE_IPV6)) {
            PRINTF("Invalid!");
            break;
        }

        if ((icmp_type > 19) && (icmp_type < 30)) {
            if (version != VTSS_IP_TYPE_IPV4) {
                PRINTF("Unassigned");
            } else {
                PRINTF("Reserved (for Robustness Experiment)");
            }
        } else if ((icmp_type > 41) && (icmp_type < 253)) {
            PRINTF("Unassigned");
        } else {
            PRINTF("Unknown!");
        }
        break;
    }

    return s;
}

static int int_cmp(int a, int b)
{
    if (a == b) {
        return 0;
    } else if (a < b) {
        return -1;
    } else {
        return 1;
    }
}

static int if_id_cmp(const void *_a, const void *_b)
{
    const vtss_if_id_t *a = _a;
    const vtss_if_id_t *b = _b;

    // first order compare
    int res = int_cmp(a->type, b->type);
    if (res != 0) {
        return res;
    }

    // second order compare
    switch (a->type) {
    case VTSS_ID_IF_TYPE_VLAN:
        return int_cmp(a->u.vlan, b->u.vlan);
    case VTSS_ID_IF_TYPE_OS_ONLY:
        return int_cmp(a->u.os.ifno, b->u.os.ifno);
    default:
        return 0;
    }
}

static int status_cmp(const void *_a, const void *_b)
{
    const vtss_if_status_t *a = _a;
    const vtss_if_status_t *b = _b;

    // first order parameter is interface
    int res = if_id_cmp(&(a->if_id), &(b->if_id));
    if (res != 0) {
        return res;
    }

    // second order parameter is status type
    return int_cmp(a->type, b->type);
}

void vtss_ip2_if_status_sort(u32 cnt, vtss_if_status_t *status)
{
    qsort(status, cnt, sizeof(vtss_if_status_t), status_cmp);
}

