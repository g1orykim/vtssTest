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

// Avoid "vtss_options.h not used in module vtss_l3.c"
/*lint --e{766} */
#include "vtss_options.h"
#if defined(VTSS_ARCH_JAGUAR_1)

#include "vtss_api.h"
#include "vtss_state.h"

#define E(...) VTSS_EG(VTSS_TRACE_GROUP_L3, ##__VA_ARGS__)
#define I(...) VTSS_IG(VTSS_TRACE_GROUP_L3, ##__VA_ARGS__)
#define D(...) VTSS_DG(VTSS_TRACE_GROUP_L3, ##__VA_ARGS__)
#define N(...) VTSS_NG(VTSS_TRACE_GROUP_L3, ##__VA_ARGS__)

#define MAC_FORMAT "%02x-%02x-%02x-%02x-%02x-%02x"
#define MAC_ARGS(X) \
    X.addr[0], X.addr[1], X.addr[2], X.addr[3], X.addr[4], X.addr[5]

#define IPV4_FORMAT "%u.%u.%u.%u"
#define IPV4_ARGS(X) \
    (((X) >> 24) & 0xff), (((X) >> 16) & 0xff), \
    (((X) >> 8) & 0xff), ((X) & 0xff)

#define IPV6_FORMAT "%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x"
#define IPV6_ARGS(X) \
    ((((u32)((X).addr[ 0])) << 8) | ((u32)(X).addr[ 1])), \
    ((((u32)((X).addr[ 2])) << 8) | ((u32)(X).addr[ 3])), \
    ((((u32)((X).addr[ 4])) << 8) | ((u32)(X).addr[ 5])), \
    ((((u32)((X).addr[ 6])) << 8) | ((u32)(X).addr[ 7])), \
    ((((u32)((X).addr[ 8])) << 8) | ((u32)(X).addr[ 9])), \
    ((((u32)((X).addr[10])) << 8) | ((u32)(X).addr[11])), \
    ((((u32)((X).addr[12])) << 8) | ((u32)(X).addr[13])), \
    ((((u32)((X).addr[14])) << 8) | ((u32)(X).addr[15]))

#define IPV6N_FORMAT IPV6_FORMAT "/%hhu"
#define IPV6N_ARG(X) IPV6_ARGS((X).address), (X).prefix_size

#define IPV6_UC_FORMAT \
    "{network = "IPV6N_FORMAT", destination = "IPV6_FORMAT"}"
#define IPV6_UC_ARGS(X) IPV6N_ARG((X).network), IPV6_ARGS((X).destination)

#define IPV4N_FORMAT IPV4_FORMAT "/%hhu"
#define IPV4N_ARG(X) IPV4_ARGS((X).address), (X).prefix_size

#define IPV4_UC_FORMAT \
    "{network = "IPV4N_FORMAT", destination = "IPV4_FORMAT"}"
#define IPV4_UC_ARGS(X) IPV4N_ARG((X).network), IPV4_ARGS((X).destination)

#define ARP_FORMAT "{ip="IPV4_FORMAT", mac="MAC_FORMAT ", rleg=%u}"
#define ARP_ARGS(X) IPV4_ARGS((X).dip), MAC_ARGS((X).dmac), (X).rleg

#define NDP_FORMAT "{ip="IPV6_FORMAT", mac="MAC_FORMAT ", rleg=%u}"
#define NDP_ARGS(X) IPV6_ARGS((X).dip), MAC_ARGS((X).dmac), (X).rleg

#define ENA_DIS_ARGS(X) (X) ? "enable":"disable"

#define L3VLANCONF_FORMAT "{%s, rleg_id:%u}"
#define L3VLANCONF_ARGS(X) ENA_DIS_ARGS((X).routing_enable), (X).rleg_id

#define RLEGCONF_FORMAT \
    "{IPv4UC: %s, IPv6UC: %s, IPv4ICMP: %s, IPv6ICMP: %s, EVID: %hu}"
#define RLEGCONF_ARGS(X) \
    ENA_DIS_ARGS((X).ipv4_unicast_enable), \
    ENA_DIS_ARGS((X).ipv6_unicast_enable), \
    ENA_DIS_ARGS((X).ipv4_icmp_redirect_enable), \
    ENA_DIS_ARGS((X).ipv6_icmp_redirect_enable), \
    (X).evid

#define VTSS_CHECK( __bool, r )                          \
    do {                                                 \
        if (!(__bool)) {                                 \
            E( "CHECK FAILED: \"%s\"",  #__bool );       \
            return r;                                    \
        }                                                \
    } while (0)

#define VTSS_RC_PRINT( _expr_ )              \
    do {                                     \
        vtss_rc __rc = (_expr_);             \
        if (__rc != VTSS_RC_OK) {            \
            E( "FAILED: \"%s\"",  #_expr_)   \
            return __rc;                     \
        }                                    \
    } while (0)

#define VTSS_RC_PRINT_INFO( _expr_ )         \
    do {                                     \
        vtss_rc __rc = (_expr_);             \
        if (__rc != VTSS_RC_OK) {            \
            I( "FAILED: \"%s\"",  #_expr_)   \
            return __rc;                     \
        }                                    \
    } while (0)

#define VTSS_FUNC_RT(func, ...) \
    (rt ? (VTSS_FUNC(func, ##__VA_ARGS__)) : (VTSS_RC_OK))

/* finds and returns an unused rleg id for the provided vlan. Will fail if the
 * given vlan is allready configured, or if no more rlegs are aviable. */
static inline
vtss_rc rleg_id_get_new(vtss_l3_rleg_conf_t       *rleg_conf,
                        const vtss_l3_rleg_conf_t *const new_rleg,
                        vtss_l3_rleg_id_t         *rleg_id)
{
    u32 i;
    u32 free_id = 0xffffffff; // just to make lint happy
    BOOL found_free = FALSE;

    for (i = 0; i < VTSS_RLEG_CNT; ++i) {
        if (rleg_conf[i].vlan == new_rleg->vlan) {
            E("rleg_id_get_new failed, as vlan %d is allready used by rleg %d",
              rleg_conf[i].vlan, i);

            return VTSS_RC_ERROR;
        }

        if (rleg_conf[i].vlan == 0 && found_free == FALSE) {
            found_free = TRUE;
            free_id = i;
        }
    }

    if (found_free) {
        D("%s:%d vlan: %u, rleg_id: %u",
          __FUNCTION__, __LINE__, new_rleg->vlan, free_id);
        rleg_conf[free_id] = *new_rleg;
        if (rleg_id) {
            *rleg_id = free_id;
        }
        return VTSS_RC_OK;
    }

    I("New rleg requested, but we are out of resources");
    return VTSS_RC_ERROR;
}

/* finds and returns an existing rleg id for the provided vlan. Will fail if the
 * given vlan does not allready exitst. */
static inline
vtss_rc rleg_id_get(vtss_l3_rleg_conf_t *rleg_conf,
                    const vtss_vid_t     vlan,
                    vtss_l3_rleg_id_t   *rleg_id,
                    vtss_l3_rleg_conf_t *rleg)
{
    u32 i;

    if (vlan == 0)
        return VTSS_RC_ERROR;

    for (i = 0; i < VTSS_RLEG_CNT; ++i) {
        if (rleg_conf[i].vlan == vlan) {
            if (rleg_id)
                *rleg_id = i;

            if (rleg)
                *rleg = rleg_conf[i];

            return VTSS_RC_OK;
        }
    }

    return VTSS_RC_ERROR;
}

/* finds delete and returns an existing rleg id for the provided vlan. Will fail
 * if the given vlan does not allready exitst. */
static inline
vtss_rc rleg_id_del(vtss_l3_rleg_conf_t *rleg_conf,
                    const vtss_vid_t     vlan,
                    vtss_l3_rleg_id_t   *rleg_id)
{
    vtss_l3_rleg_id_t id;
    VTSS_RC(rleg_id_get(rleg_conf, vlan, &id, 0));

    *rleg_id = id;
    rleg_conf[id].ipv4_unicast_enable = FALSE;
    rleg_conf[id].ipv6_unicast_enable = FALSE;
    rleg_conf[id].ipv4_icmp_redirect_enable = FALSE;
    rleg_conf[id].ipv6_icmp_redirect_enable = FALSE;
    rleg_conf[id].vlan = 0;

    I("rleg_id = %d, vlan = %d", id, vlan);

    return VTSS_RC_OK;
}

static vtss_rc rleg_del(vtss_state_t *vtss_state,
                        vtss_l3_rleg_conf_t *rleg_conf,
                        const vtss_vid_t     vlan)
{
    vtss_rc rc;
    vtss_l3_rleg_id_t rleg_id = 0xffffffff; // Just to make lint happy
    vtss_l3_rleg_conf_t conf = {
        .ipv4_unicast_enable = FALSE,
        .ipv6_unicast_enable = FALSE,
        .ipv4_icmp_redirect_enable = FALSE,
        .ipv6_icmp_redirect_enable = FALSE,
        .vlan = vlan
    };

#define DO(X)                                          \
    if (rc == VTSS_RC_OK) {                            \
        rc = (X);                                      \
        if (rc != VTSS_RC_OK) {                        \
            E("Failed: " #X "rleg_id = %d, vlan = %d", \
              rleg_id, vlan);                          \
        }                                              \
    }
    rc = rleg_id_del(rleg_conf, vlan, &rleg_id);
    I("Deleting rleg_id = %d, vlan = %d", rleg_id, vlan);
    DO(VTSS_FUNC(l3.vlan_set, rleg_id, vlan, FALSE));
    DO(VTSS_FUNC(l3.rleg_set, rleg_id, &conf));
#undef DO

    return rc;
}

static vtss_rc rleg_add(vtss_state_t *vtss_state,
                        vtss_l3_rleg_conf_t       *rleg_conf,
                        const vtss_l3_rleg_conf_t *const conf)
{
    vtss_rc rc;
    vtss_l3_rleg_id_t rleg_id = 0xffffffff; // Just to make lint happy

#define DO(X)                                                             \
    if (rc == VTSS_RC_OK) {                                               \
        rc = (X);                                                         \
        if (rc != VTSS_RC_OK) {                                           \
            E("Failed: " #X "rleg_id = %d, vlan = %d",                    \
              rleg_id, conf->vlan);                                       \
        }                                                                 \
    } else {                                                              \
        I("Skipping: " #X "rleg_id = %d, vlan = %d due to earlier error", \
          rleg_id, conf->vlan);                                           \
    }
    rc = rleg_id_get_new(vtss_state->l3.rleg_conf, conf, &rleg_id);
    D("Adding rleg: rleg_id = %d, vlan = %d", rleg_id, conf->vlan);
    DO(VTSS_FUNC(l3.rleg_set, rleg_id, conf));
    DO(VTSS_FUNC(l3.vlan_set, rleg_id, conf->vlan, TRUE));
#undef DO

    return rc;
}

static vtss_rc rleg_flush(vtss_state_t *vtss_state,
                          vtss_l3_rleg_conf_t *rleg_conf)
{
    u32 i;

    for (i = 0; i < VTSS_RLEG_CNT; ++i) {
        // Only consider valid entries
        if (rleg_conf[i].vlan == 0)
            continue;

        rleg_conf[i].ipv4_unicast_enable = FALSE;
        rleg_conf[i].ipv6_unicast_enable = FALSE;
        rleg_conf[i].ipv4_icmp_redirect_enable = FALSE;
        rleg_conf[i].ipv6_icmp_redirect_enable = FALSE;
        (void)VTSS_FUNC(l3.vlan_set, i, rleg_conf[i].vlan, FALSE);
        (void)VTSS_FUNC(l3.rleg_set, i, &rleg_conf[i]);
        rleg_conf[i].vlan = 0;
    }

    return VTSS_RC_OK;
}

static vtss_rc hardcode_null_arp_entry(vtss_state_t *vtss_state)
{
    vtss_mac_t dmac;
    memset(&dmac, 0, sizeof(dmac));
    VTSS_RC_PRINT(VTSS_FUNC(l3.ipv6_uc_set, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
    VTSS_RC_PRINT(VTSS_FUNC(l3.ipv4_uc_set, 4, 0, 0, 0, 0));
    VTSS_RC_PRINT(VTSS_FUNC(l3.arp_set, 0, &dmac, 0));
    return VTSS_RC_OK;
}

#if defined(VTSS_SW_OPTION_L3RT)
static
char * vtss_routing_entry_to_string(const vtss_routing_entry_t *const entry,
                                    char * buf,
                                    unsigned size,
                                    unsigned * consumed)
{
    unsigned c = 0;
    switch( entry->type ) {
    case VTSS_ROUTING_ENTRY_TYPE_INVALID:
        c += snprintf(buf, size, "INVALID");
        break;

    case VTSS_ROUTING_ENTRY_TYPE_IPV6_UC:
        c += snprintf(buf, size, IPV6_UC_FORMAT,
                      IPV6_UC_ARGS(entry->route.ipv6_uc));
        break;

    case VTSS_ROUTING_ENTRY_TYPE_IPV4_MC:
        c += snprintf(buf, size, "IPv4 MC (no print)");
        break;

    case VTSS_ROUTING_ENTRY_TYPE_IPV4_UC:
        c += snprintf(buf, size, IPV4_UC_FORMAT,
                      IPV4_UC_ARGS(entry->route.ipv4_uc));
        break;
    default:
        c += snprintf(buf, size, "UNKNOWN");
        break;
    }

    if (consumed)
        *consumed = c;

    return buf;
}

static
char * vtss_neighbour_to_string(const vtss_l3_neighbour_t* const entry,
                                char * buf,
                                unsigned size,
                                unsigned * consumed)
{
    unsigned c = 0;
    c += snprintf(buf, size, "{DMAC:"MAC_FORMAT", VLAN:%hu, IP:",
                  MAC_ARGS(entry->dmac), entry->vlan);

    if (c + 1 >= size)
        goto END;
    else
        size -= c;

    switch (entry->dip.type) {
    case VTSS_IP_TYPE_NONE:
        c += snprintf(buf + c, size, "INVALID");
        break;

    case VTSS_IP_TYPE_IPV4:
        c += snprintf(buf + c, size, IPV4_FORMAT,
                      IPV4_ARGS(entry->dip.addr.ipv4));
        break;

    case VTSS_IP_TYPE_IPV6:
        c += snprintf(buf + c, size, IPV6_FORMAT,
                      IPV6_ARGS(entry->dip.addr.ipv6));
        break;

    default:
        c += snprintf(buf + c, size, "UNKNOWN");
        break;
    }

    if (c + 1 >= size)
        goto END;
    else
        size -= c;

    c += snprintf(buf + c, size, "}");

END:
    if (consumed)
        *consumed = c;

    return buf;
}

static inline
vtss_rc arp_ip_find(const vtss_ipv4_t               *const ip,
                    const vtss_l3_neighbour_cache_t *const nc,
                    u32                             *const hw_ptr,
                    u32                             *const arp_ptr);

static inline
BOOL mac_equal(const vtss_mac_t *const a, const vtss_mac_t *const b)
{
    u32 i;

    for (i = 0; i < 6; ++i)
        if (a->addr[i] != b->addr[i])
            return FALSE;

    return TRUE;
}

static inline
void mac_cpy(vtss_mac_t *const dst, const vtss_mac_t *const src)
{
    u32 i;

    for (i = 0; i < 6; ++i)
        dst->addr[i] = src->addr[i];
}

/* IP convertion utilities */
typedef struct {
    u32 d;
} vtss_ipv4_uc_tcam_data;

typedef struct {
    u32 d[2];
} vtss_ipv4_mc_tcam_data;

typedef struct {
    u32 d[4];
} vtss_ipv6_uc_tcam_data;

static inline
BOOL vtss_ipv4_uc_tcam_data_equal(const vtss_ipv4_uc_tcam_data *const a,
                                  const vtss_ipv4_uc_tcam_data *const b)
{
    return a->d == b->d;
}

/*
   static inline
   BOOL vtss_ipv4_mc_tcam_data_equal( const vtss_ipv4_mc_tcam_data *const a,
   const vtss_ipv4_mc_tcam_data *const b )
   {
   return (a->d[0] == b->d[0]) && (a->d[1] == b->d[1]);
   }
   */

static inline
BOOL vtss_ipv6_uc_tcam_data_equal(const vtss_ipv6_uc_tcam_data *const a,
                                  const vtss_ipv6_uc_tcam_data *const b)
{
    return (a->d[0] == b->d[0]) && (a->d[1] == b->d[1]) &&
        (a->d[1] == b->d[1]) && (a->d[3] == b->d[3]);
}

static inline
void ipv4_to_tcam(const vtss_ipv4_t      *const ip,
                  vtss_ipv4_uc_tcam_data *const data)
{
    data->d = *ip;
}

static inline
void tcam_to_ipv4(const vtss_ipv4_uc_tcam_data *const data,
                  vtss_ipv4_t *const ip )
{
    *ip = data->d;
}

static inline
void ipv6_to_tcam(const vtss_ipv6_t      *const ip,
                  vtss_ipv6_uc_tcam_data *const data)
{
    u32 i;

    for (i = 0; i < 4; i++) {
        data->d[i] = 0;
    }

    for (i = 0; i < 16; i++) {
        u32 idx = i/4;
        data->d[idx] <<= 8;
        data->d[idx] |= ip->addr[i];
    }
}

static inline
void tcam_to_ipv6(const vtss_ipv6_uc_tcam_data *const data,
                  vtss_ipv6_t                  *const ip)
{
    u32 i;
    for ( i = 0; i < 4; i++ ) {
        ip->addr[(i*4)+0] = (u8)((data->d[i] >> 24) & 0xff);
        ip->addr[(i*4)+1] = (u8)((data->d[i] >> 16) & 0xff);
        ip->addr[(i*4)+2] = (u8)((data->d[i] >>  8) & 0xff);
        ip->addr[(i*4)+3] = (u8)((data->d[i]) & 0xff);
    }
}

static inline
u32 v4_mask_to_prefix(vtss_ipv4_t *const mask)
{
    u32 cnt = 0;
    u32 tmp = *mask;

    while (tmp & 0x80000000) {
        cnt ++;
        tmp <<= 1;
    }

    return cnt;
}

static inline
void prefix_to_v4_mask(const u32 prefix, vtss_ipv4_t *const mask)
{
    u32 i;
    *mask = 0;

    for (i = 0; i < 32; i++) {
        *mask <<= 1;

        if (i < prefix)
            *mask |= 1;
    }
}

static inline
u32 v6_mask_to_prefix(vtss_ipv6_t *const mask)
{
    u32 i;
    u32 cnt = 0;

    for (i = 0; i < 16; ++i) {
        /*lint --e{616} ... Fallthrough intended */
        switch (mask->addr[i]) {
        case 0xff: cnt += 8;
                   break;
        case 0xfe: cnt ++;
        case 0xfc: cnt ++;
        case 0xf8: cnt ++;
        case 0xf0: cnt ++;
        case 0xe0: cnt ++;
        case 0xc0: cnt ++;
        case 0x80: cnt ++;
        default:   return cnt;
        }
    }

    return cnt;
}


static inline
void prefix_to_v6_mask(const u32 prefix, vtss_ipv6_t *const mask)
{
    u8 v = 0;
    u32 i = 0;
    u32 next_bit = 0;

    /* byte-wise update or clear */
    for (i = 0; i < 16; ++i) {
        u32 b = (i + 1) * 8;
        if (b <= prefix) {
            mask->addr[i] = 0xff;
            next_bit = b;
        } else {
            mask->addr[i] = 0x0;
        }
    }

    switch (prefix % 8 ) {
    case 1: v = 0x80; break;
    case 2: v = 0xc0; break;
    case 3: v = 0xe0; break;
    case 4: v = 0xf0; break;
    case 5: v = 0xf8; break;
    case 6: v = 0xfc; break;
    case 7: v = 0xfe; break;
    }
    mask->addr[(next_bit/8)] = v;
}
/* ******************************************************** */

static inline
BOOL is_ipv6_uc_alligned(const u32 i)
{
    if ( (i & 3 ) == 0 )
        return TRUE;
    return FALSE;
}

static inline
BOOL is_ipv4_mc_alligned(const u32 i)
{
    if ( (i & 1 ) == 0 )
        return TRUE;
    return FALSE;
}

static inline
BOOL is_ipv4_uc_alligned(const u32 i)
{
    return TRUE;
}

static inline
BOOL ipv6_equal(const vtss_ipv6_t *const a, const vtss_ipv6_t *const b)
{
    u32 i;
    for (i = 0; i < 16; ++i) {
        if (a->addr[i] != b->addr[i])
            return FALSE;
    }

    return TRUE;
}

static inline
u32 arp_ptr_match(const vtss_l3_neighbour_cache_t *const nc, const u32 ptr)
{
    u32 i;
    u32 cnt = 0;

    for (i = 0; i < VTSS_ARP_IPV4_RELATIONS; ++i) {
        if (! nc->ipv4[i].valid)
            continue;

        if (nc->ipv4[i].ptr == ptr)
            ++cnt;
    }

    return cnt;
}

static inline
u32 ndp_ptr_match(const vtss_l3_neighbour_cache_t *const nc, const u32 ptr)
{
    u32 i;
    u32 cnt = 0;

    for (i = 0; i < VTSS_ARP_IPV6_RELATIONS; ++i) {
        if (!nc->ipv6[i].valid)
            continue;

        if (nc->ipv6[i].ptr == ptr)
            ++cnt;
    }

    return cnt;
}

static inline
u32 count_ones(u32 d)
{
    u32 c;

    for (c = 0; d; d >>= 1)
        c += d & 1;

    return c;
}

static void ip_null_address_get(vtss_ip_type_t t, vtss_ip_addr_t * ip)
{
    memset(ip, 0, sizeof(vtss_ip_addr_t));
    ip->type = t;
}

static inline
vtss_rc arp_ip_find( const vtss_ipv4_t               *const ip,
                     const vtss_l3_neighbour_cache_t *const nc,
                     u32                             *const hw_ptr,
                     u32                             *const relation_ptr )
{
    u32 i;

    N("arp_ip_find(ip="IPV4_FORMAT")", IPV4_ARGS(*ip));

    /* scan through the ipv4 relations to the hw nc entries and see if we can
     * find a match
     * */
    for (i = 0; i < VTSS_ARP_IPV4_RELATIONS; ++i) {
        if (nc->ipv4[i].valid && nc->ipv4[i].ip == *ip) {
            VTSS_CHECK( nc->hw[nc->ipv4[i].ptr].valid, VTSS_RC_ERROR );
            if (hw_ptr)
                *hw_ptr = nc->ipv4[i].ptr;

            if (relation_ptr)
                *relation_ptr = i;

            N("arp_ip_find -> hw_ptr=%u, relation_ptr=%u",
              nc->ipv4[i].ptr, i);

            return VTSS_RC_OK;
        }
    }

    N("arp_ip_find: entry not found");
    return VTSS_RC_ERROR;
}

static inline
vtss_rc ndp_ip_find( const vtss_ipv6_t               *const ip,
                     const vtss_l3_neighbour_cache_t *const nc,
                     u32                             *const hw_ptr,
                     u32                             *const relation_ptr )
{
    u32 i;

    N("ndp_ip_find(ip="IPV6_FORMAT")", IPV6_ARGS(*ip));

    /* scan through the ipv6 relations to the hw nc entries and see if we can
     * find a match
     * */
    for (i = 0; i < VTSS_ARP_IPV6_RELATIONS; ++i) {
        if (nc->ipv6[i].valid && ipv6_equal(&nc->ipv6[i].ip, ip)) {
            VTSS_CHECK(nc->hw[nc->ipv6[i].ptr].valid, VTSS_RC_ERROR);
            if (hw_ptr)
                *hw_ptr = nc->ipv6[i].ptr;

            if (relation_ptr)
                *relation_ptr = i;

            N("ndp_ip_find -> hw_ptr=%u, relation_ptr=%u",
              nc->ipv6[i].ptr, i);

            return VTSS_RC_OK;
        }
    }

    D("ndp_ip_find: entry not found");
    return VTSS_RC_ERROR;
}


static inline
vtss_rc nc_ip_find(const vtss_ip_addr_t            *const ip,
                   const vtss_l3_neighbour_cache_t *const nc,
                   u32                             *const hw_ptr,
                   u32                             *const relation_ptr)
{
    switch (ip->type) {
    case VTSS_IP_TYPE_IPV4:
        return arp_ip_find(&ip->addr.ipv4, nc, hw_ptr, relation_ptr);

    case VTSS_IP_TYPE_IPV6:
        return ndp_ip_find(&ip->addr.ipv6, nc, hw_ptr, relation_ptr);

    default:
        D("type %d not supported", ip->type);
        return VTSS_RC_ERROR;
    }
}

/* START OF LPM MANIPULATORS -------------------------------------------- */
static inline
vtss_rc vtss_routing_type_to_lpm_type(const vtss_routing_entry_type_t r,
                                      vtss_l3_lpm_entry_type *const l)
{
    switch (r) {
    case VTSS_ROUTING_ENTRY_TYPE_IPV6_UC:
        *l = VTSS_L3_LPM_ENTRY_IPV6_UC;
        break;

    case VTSS_ROUTING_ENTRY_TYPE_IPV4_MC:
        *l = VTSS_L3_LPM_ENTRY_IPV4_MC;
        break;

    case VTSS_ROUTING_ENTRY_TYPE_IPV4_UC:
        *l = VTSS_L3_LPM_ENTRY_IPV4_UC;
        break;

    default:
        E("Unsupported type %d", r);
        return VTSS_RC_ERROR;
    }

    return VTSS_RC_OK;
}

static inline
u32 lpm_type_size_get(const vtss_l3_lpm_entry_type t)
{
    switch ( t ) {
    case VTSS_L3_LPM_ENTRY_IPV4_UC:
        return 1;

    case VTSS_L3_LPM_ENTRY_IPV6_UC:
        return 4;

    case VTSS_L3_LPM_ENTRY_IPV4_MC:
        return 2;
    }

    E("Invalid value of t = %d", t);
    return 0;
}

static inline
BOOL lpm_ipv4_uc_entry_valid(const u32                 i,
                             const vtss_l3_lpm_table_t *const lpm)
{
    if (! is_ipv4_uc_alligned(i))
        return FALSE;

    VTSS_CHECK(i < VTSS_LPM_CNT, FALSE);

    if (! lpm->e[i].valid)
        return FALSE;

    if (lpm->e[i].type != VTSS_L3_LPM_ENTRY_IPV4_UC)
        return FALSE;

    return TRUE;
}

#if 0
static inline
BOOL lpm_ipv4_mc_entry_valid( const u32                 i,
                              const vtss_l3_lpm_table_t *const lpm )
{
    if ( ! is_ipv4_mc_alligned(i) )
        return FALSE;

    VTSS_CHECK( i + 2 <= VTSS_LPM_CNT, FALSE );

    if (!lpm->e[i].valid)
        return FALSE;

    if (lpm->e[i].type != VTSS_L3_LPM_ENTRY_IPV4_MC)
        return FALSE;

    VTSS_CHECK(lpm->e[i + 1].valid, FALSE);
    VTSS_CHECK(lpm->e[i + 1].type == VTSS_L3_LPM_ENTRY_IPV6_UC, FALSE);

    return TRUE;
}
#endif

static inline
BOOL lpm_ipv6_uc_entry_valid(const u32                 i,
                             const vtss_l3_lpm_table_t *const lpm)
{
    if (! is_ipv6_uc_alligned(i))
        return FALSE;

    VTSS_CHECK(i + 4 <= VTSS_LPM_CNT, FALSE);

    if (!lpm->e[i].valid)
        return FALSE;

    if (lpm->e[i].type != VTSS_L3_LPM_ENTRY_IPV6_UC)
        return FALSE;

    /* Assumptions regarding the layout in the LPM table */
    VTSS_CHECK(lpm->e[i + 1].valid, FALSE);
    VTSS_CHECK(lpm->e[i + 2].valid, FALSE);
    VTSS_CHECK(lpm->e[i + 3].valid, FALSE);
    VTSS_CHECK(lpm->e[i + 1].type == VTSS_L3_LPM_ENTRY_IPV6_UC, FALSE);
    VTSS_CHECK(lpm->e[i + 2].type == VTSS_L3_LPM_ENTRY_IPV6_UC, FALSE);
    VTSS_CHECK(lpm->e[i + 3].type == VTSS_L3_LPM_ENTRY_IPV6_UC, FALSE);

    return TRUE;
}

static inline
BOOL lpm_ipv4_uc_data_get(const vtss_l3_lpm_table_t *const lpm,
                          vtss_ipv4_uc_tcam_data    *const data,
                          const u32                 i)
{
    if (!lpm_ipv4_uc_entry_valid(i, lpm))
        return FALSE;

    data->d = lpm->e[i].data;
    return TRUE;
}

static inline
BOOL lpm_ipv4_uc_mask_get(const vtss_l3_lpm_table_t *const lpm,
                          vtss_ipv4_uc_tcam_data    *const data,
                          const u32                 i)
{
    if (!lpm_ipv4_uc_entry_valid(i, lpm))
        return FALSE;

    data->d = lpm->e[i].mask;
    return TRUE;
}

static inline
BOOL lpm_ipv4_uc_dest_get(const vtss_l3_lpm_table_t *const lpm,
                          vtss_ipv4_uc_tcam_data    *const data,
                          const u32                 i)
{
    if (!lpm_ipv4_uc_entry_valid(i, lpm))
        return FALSE;

    data->d = lpm->e[i].dest;
    return TRUE;
}

/*
static inline
BOOL lpm_ipv4_mc_data_get( const vtss_l3_lpm_table_t *const lpm,
                           vtss_ipv4_mc_tcam_data    *const data,
                           const u32                 i )
{
    if ( ! lpm_ipv4_mc_entry_valid(i, lpm) )
        return FALSE;

    data->d[0] = lpm->e[i].data;
    data->d[1] = lpm->e[i+1].data;
    return TRUE;
}

static inline
BOOL lpm_ipv4_mc_mask_get( const vtss_l3_lpm_table_t *const lpm,
                           vtss_ipv4_mc_tcam_data    *const data,
                           const u32                 i )
{
    if ( ! lpm_ipv4_mc_entry_valid(i, lpm) )
        return FALSE;

    data->d[0] = lpm->e[i].mask;
    data->d[1] = lpm->e[i+1].mask;
    return TRUE;
}
*/

static inline
BOOL lpm_ipv6_uc_data_get(const vtss_l3_lpm_table_t *const lpm,
                          vtss_ipv6_uc_tcam_data    *const data,
                          const u32                 i)
{
    if (!lpm_ipv6_uc_entry_valid(i, lpm))
        return FALSE;

    data->d[0] = lpm->e[i].data;
    data->d[1] = lpm->e[i+1].data;
    data->d[2] = lpm->e[i+2].data;
    data->d[3] = lpm->e[i+3].data;
    return TRUE;
}

static inline
BOOL lpm_ipv6_uc_mask_get(const vtss_l3_lpm_table_t *const lpm,
                          vtss_ipv6_uc_tcam_data    *const data,
                          const u32                 i)
{
    if (!lpm_ipv6_uc_entry_valid(i, lpm))
        return FALSE;

    data->d[0] = lpm->e[i].mask;
    data->d[1] = lpm->e[i+1].mask;
    data->d[2] = lpm->e[i+2].mask;
    data->d[3] = lpm->e[i+3].mask;
    return TRUE;
}

static inline
BOOL lpm_ipv6_uc_dest_get(const vtss_l3_lpm_table_t *const lpm,
                          vtss_ipv6_uc_tcam_data    *const data,
                          const u32                 i)
{
    if (!lpm_ipv6_uc_entry_valid(i, lpm))
        return FALSE;

    data->d[0] = lpm->e[i].dest;
    data->d[1] = lpm->e[i+1].dest;
    data->d[2] = lpm->e[i+2].dest;
    data->d[3] = lpm->e[i+3].dest;
    return TRUE;
}


static inline
vtss_rc lpm_move(vtss_state_t        *vtss_state,
                 BOOL                rt,
                 vtss_l3_lpm_table_t *const lpm,
                 const u32           idx,
                 const u32           distance,
                 const BOOL          down)
{
    vtss_rc rc;
    u32 i = 0, n = 0;

    N("LPM_MOVE(idx=%u, distance=%u, down=%d)", idx, distance, down);

    if (down && (idx + distance) >= VTSS_LPM_CNT) {
        D( "LPM_MOVE, invalid parameters: "
           "idx = %d, down = %d, distance = %d\n",
           idx, down, distance );
        return VTSS_RC_ERROR;
    }

    if ((!down) && idx < distance) {
        D( "LPM_MOVE, invalid parameters: "
           "idx = %d, down = %d, distance = %d\n",
           idx, down, distance );
        return VTSS_RC_ERROR;
    }

    if (distance == 0)
        return VTSS_RC_OK;

    /* calculate the number of elements to be moved */
    if ( down )
        n = VTSS_LPM_CNT - idx - distance;
    else
        n = VTSS_LPM_CNT - idx;

    N("LPM_MOVE(idx=%u, n=%d, distance=%u, down=%d)",
      idx, n, distance, down);
    rc = VTSS_FUNC_RT(l3.lpm_move, idx, n, distance, down);

    VTSS_CHECK(rc == VTSS_RC_OK, VTSS_RC_INCOMPLETE);

    /* make the change in the software cache */
    if (down) { /* we are moving elements down */
        /* move the entries down one by one */
        for (i = 0; i < n; ++i) {
            u32 src = VTSS_LPM_CNT - 1 - distance - i;
            u32 dst = VTSS_LPM_CNT - 1 - i;

            VTSS_CHECK(dst < VTSS_LPM_CNT && src < VTSS_LPM_CNT,
                       VTSS_RC_ERROR);
            lpm->e[dst] = lpm->e[src];
        }

        /* invalidate the newly opened entries */
        for (i = 0; i < distance; i++) {
            lpm->e[idx + i].valid  = 0;
        }

    } else { /* we are moving elements up */
        /* move the entries up one by one */
        for (i = 0; i < n; ++i) {
            u32 src = idx + i;
            u32 dst = idx - distance + i;

            VTSS_CHECK(dst < VTSS_LPM_CNT && src < VTSS_LPM_CNT,
                       VTSS_RC_ERROR);
            lpm->e[dst] = lpm->e[src];
        }

        /* invalidate the newly opened entries */
        for (i = 0; i < distance; i++) {
            u32 x = VTSS_LPM_CNT - 1 - i;

            VTSS_CHECK(x < VTSS_LPM_CNT, VTSS_RC_ERROR);
            lpm->e[x].valid  = 0;
        }
    }

    return VTSS_RC_OK;
}

static inline
vtss_rc lpm_insertpoint( const vtss_l3_lpm_table_t    *const lpm,
                         const u32                    prefix,
                         const u32                    start_row,
                         const vtss_l3_lpm_entry_type type,
                         u32                          *insertpoint )
{
    u32 row = start_row;
    u32 lpm_type_size = lpm_type_size_get(type);
    const vtss_l3_lpm_entry_t *e = &lpm->e[start_row];

    N("lpm_insertpoint(prefix=%u, start_row=%u, type=%u)",
      prefix, start_row, type);

    while ((e->valid) && (e->type == type)) {
        u32 i;
        u32 this_row = row;
        u32 this_prefix = count_ones(e->mask);

        /* step over multi row entries */
        for (i = 1; i < lpm_type_size; ++i) {
            row += 1;

            VTSS_CHECK( row < VTSS_LPM_CNT, VTSS_RC_ERROR );

            e = &lpm->e[row];

            VTSS_CHECK( (e->valid) && (e->type == type ), VTSS_RC_ERROR );
            this_prefix += count_ones( e->mask );
        }

        if (this_prefix > prefix) {
            /* we reached the first entry which must be moved */
            N("lpm_insertpoint -> %u", this_row);
            *insertpoint = this_row;
            return VTSS_RC_OK;
        }

        row += 1;

        if (row >= VTSS_LPM_CNT)
            return VTSS_RC_ERROR;

        e = &lpm->e[row];
    }

    if (row >= VTSS_LPM_CNT)
        return VTSS_RC_ERROR;

    /* The new entry must be inserted at the end of the list. The loop
     * above has allready incremented row to the end of this kind plus
     * one. Just what we need*/

    N("lpm_insertpoint -> %u", row);
    *insertpoint = row;
    return VTSS_RC_OK;
}

static inline
vtss_rc lpm_alloc(vtss_state_t                 *vtss_state,
                  BOOL                          rt,
                  vtss_l3_lpm_table_t          *const lpm,
                  const vtss_l3_lpm_entry_type  type,
                  const u32                     prefix,
                  u32                          *const ptr)
{
    /* Maintain the table sorted such that all valid entries exists as
     * a contiguous sequence.
     *
     * Segment the table such that the types appears in this order:
     *  IPv6 UC
     *  IPv4 MC
     *  IPv4 UC
     *
     * This will guarnties that IPv6_UC entreis are alwaus alligned to
     * mod4 address, and IPv4_MC are always alligned to mod2 addresses.
     * This is an requirement in JR1 */

    u32 i;
    u32 free_rows = VTSS_LPM_CNT;
    u32 used_rows = 0;
    u32 insert_point = 0;
    u32 start_of_kind = 0;
    u32 start_of_ipv6_uc = 0;
    u32 start_of_ipv4_mc = 0;
    u32 start_of_ipv4_uc = 0;
    u32 number_of_ipv6_uc = 0;
    u32 number_of_ipv4_mc = 0;
    u32 number_of_ipv4_uc = 0;

    N("lpm_alloc(type=%u, prefix=%u)", type, prefix);

    /* Index the LPM table */
    for (i = 0; i < VTSS_LPM_CNT; ++i) {
        const vtss_l3_lpm_entry_t *const e = &lpm->e[i];

        if (! e->valid)
            break;

        free_rows --;
        used_rows ++;

        if (is_ipv4_uc_alligned(i) &&
            e->type == VTSS_L3_LPM_ENTRY_IPV4_UC) {

            if (number_of_ipv4_uc == 0)
                start_of_ipv4_uc = i;

            number_of_ipv4_uc ++;
        }

        if (is_ipv4_mc_alligned(i) &&
            e->type == VTSS_L3_LPM_ENTRY_IPV4_MC) {

            if (number_of_ipv4_mc == 0)
                start_of_ipv4_mc = i;

            number_of_ipv4_mc ++;
        }

        if (is_ipv6_uc_alligned(i) &&
            e->type == VTSS_L3_LPM_ENTRY_IPV6_UC) {

            if (number_of_ipv6_uc == 0)
                start_of_ipv6_uc = i;

            number_of_ipv6_uc ++;
        }
    }

    /* Finish the indexing */
    if (number_of_ipv6_uc == 0)
        start_of_ipv6_uc = 0;
    if (number_of_ipv4_mc == 0)
        start_of_ipv4_mc = number_of_ipv6_uc * 4;
    if (number_of_ipv4_uc == 0)
        start_of_ipv4_uc = start_of_ipv4_mc + number_of_ipv4_uc * 2;

    N("free_rows = %d used_rows = %d", free_rows, used_rows);
    N("start_of_ipv6_uc = %d number_of_ipv6_uc = %d",
      start_of_ipv6_uc, number_of_ipv6_uc);
    N("start_of_ipv4_uc = %d number_of_ipv4_uc = %d",
      start_of_ipv4_uc, number_of_ipv4_uc);
    N("start_of_ipv4_mc = %d number_of_ipv4_mc = %d",
      start_of_ipv4_mc, number_of_ipv4_mc);

    /* Check we have room for the new entry */
    if (free_rows < lpm_type_size_get(type)) {
        D("LPM_MOVE, not enough free rows: "
          "free_rows = %d, needed = %d\n",
          free_rows, lpm_type_size_get(type));
        return VTSS_RC_ERROR;
    }

    /* Base case: the first entry to be inserted are always inserted
     * at possition zero */
    if (used_rows == 0) {
        *ptr = 0;
        N("lpm_alloc -> %u", *ptr);
        return VTSS_RC_OK;
    }

    /* Find the starting point for the group */
    switch (type) {
    case VTSS_L3_LPM_ENTRY_IPV6_UC:
        start_of_kind = start_of_ipv6_uc;
        break;

    case VTSS_L3_LPM_ENTRY_IPV4_MC:
        start_of_kind = start_of_ipv4_mc;
        break;

    case VTSS_L3_LPM_ENTRY_IPV4_UC:
        start_of_kind = start_of_ipv4_uc;
        break;
    }

    /* Calculate the actually insert point */
    VTSS_RC_PRINT(lpm_insertpoint(lpm,
                                  prefix,
                                  start_of_kind,
                                  type,
                                  &insert_point));
    D("insert point %u", insert_point);

    /* Check if we need to move */
    if (insert_point > used_rows) {
        *ptr = insert_point;
        N("lpm_alloc -> %u", *ptr);
        return VTSS_RC_OK;
    }

    /* move the elements */
    VTSS_RC_PRINT(lpm_move(vtss_state, rt, lpm, insert_point,
                           lpm_type_size_get(type), TRUE));
    *ptr = insert_point;

    D("lpm_alloc -> %u", *ptr);
    return VTSS_RC_OK;
}

static inline
vtss_rc lpm_entry_clear(vtss_state_t        *vtss_state,
                        BOOL                 rt,
                        vtss_l3_lpm_table_t *const lpm,
                        const u32            idx)
{
    vtss_rc rc = VTSS_RC_ERROR;
    const vtss_l3_lpm_entry_t *e = &lpm->e[idx];

    D("lpm_entry_clear(idx=%u)", idx);

    switch (e->type) {
    case VTSS_L3_LPM_ENTRY_IPV6_UC:
        rc = VTSS_FUNC_RT(l3.ipv6_uc_clear, idx);
        break;

    case VTSS_L3_LPM_ENTRY_IPV4_MC:
        rc = VTSS_FUNC_RT(l3.ipv4_mc_clear, idx);
        break;

    case VTSS_L3_LPM_ENTRY_IPV4_UC:
        rc = VTSS_FUNC_RT(l3.ipv4_uc_clear, idx);
        break;
    }

    VTSS_CHECK(rc == VTSS_RC_OK, VTSS_RC_INCOMPLETE);

    switch (e->type) { /* Lint is not happy with fall through... */
    case VTSS_L3_LPM_ENTRY_IPV6_UC:
        lpm->e[idx+3].valid = 0;
        lpm->e[idx+2].valid = 0;
        lpm->e[idx+1].valid = 0;
        lpm->e[idx].valid = 0;
        break;

    case VTSS_L3_LPM_ENTRY_IPV4_MC:
        lpm->e[idx+1].valid = 0;
        lpm->e[idx].valid = 0;
        break;

    case VTSS_L3_LPM_ENTRY_IPV4_UC:
        lpm->e[idx].valid = 0;
        break;
    }

    return VTSS_RC_OK;
}

static inline
vtss_rc lpm_free(vtss_state_t *vtss_state, BOOL rt, vtss_l3_lpm_table_t *const lpm, const u32 idx)
{
    /* If the entry at the given index is valid, we must delete the
     * number of row which the give type uses. If the entry is not
     * valid, we must delete the gap until we see valid entries agian.
     * */

    u32 nrows;
    const vtss_l3_lpm_entry_t *e = &lpm->e[idx];

    D("lpm_free(idx=%u)", idx);
    VTSS_CHECK(idx < VTSS_LPM_CNT, VTSS_RC_ERROR);

    if (e->valid) {
        nrows = lpm_type_size_get(e->type);

        /* check if we are the last entry in the table */
        if (idx + nrows + 1 >= VTSS_LPM_CNT ||
            (!lpm->e[idx + nrows].valid)) {
            /* no need to move anything, we just need in clear this
             * entry */
            return lpm_entry_clear(vtss_state, rt, lpm, idx);
        }

    } else {
        const vtss_l3_lpm_entry_t *_e = &lpm->e[idx];

        nrows = 0;
        while ((nrows + idx) < VTSS_LPM_CNT && (!_e->valid)) {
            _e = &lpm->e[nrows + idx];
            nrows += 1;
        }

        if (nrows > 0) /* The above loop goes one too far */
            nrows -= 1;

        /* Nothing to delete or move */
        if (nrows + idx == VTSS_LPM_CNT - 1)
            return VTSS_RC_OK;
    }

    VTSS_CHECK( idx + nrows < VTSS_LPM_CNT, VTSS_RC_ERROR );
    return lpm_move(vtss_state, rt, lpm, idx + nrows, nrows, FALSE);
}

static inline
vtss_rc lpm_ipv4_uc_deactivate_entry(vtss_state_t        *vtss_state,
                                     BOOL                 rt,
                                     const u32            idx,
                                     vtss_l3_lpm_table_t *const lpm,
                                     const vtss_l3_neighbour_cache_t *const nc)
{
    vtss_rc rc;
    u32 hw_ptr = 0;
    u32 relation_ptr = 0;
    u32 hw_ptr_valid;
    vtss_ip_addr_t null_destination;

    D("lpm_ipv4_uc_deactivate_entry(idx=%u)", idx);

    VTSS_CHECK(lpm_ipv4_uc_entry_valid(idx, lpm), VTSS_RC_ERROR);

    // We normally want to deactivate an entry by redirecting to the NULL entry
    ip_null_address_get(VTSS_IP_TYPE_IPV4, &null_destination);
    rc = nc_ip_find(&null_destination, nc, &hw_ptr, &relation_ptr);
    hw_ptr_valid = (rc == VTSS_RC_OK ? 1 : 0);

    if (hw_ptr_valid) {
        D("Found null ptr");
        rc = VTSS_FUNC_RT(l3.ipv4_uc_set, idx, hw_ptr, 0,
                          lpm->e[idx].data, lpm->e[idx].mask);
    } else {
        D("Disable in HW");
        rc = VTSS_FUNC_RT(l3.ipv4_uc_clear, idx);
    }

    VTSS_CHECK(rc == VTSS_RC_OK, VTSS_RC_INCOMPLETE);

    if (hw_ptr_valid) {
        lpm->e[idx].ptr = hw_ptr;
        lpm->e[idx].ptr_valid = 1;
    } else {
        lpm->e[idx].ptr = 0;
        lpm->e[idx].ptr_valid = 0;
    }


    return VTSS_RC_OK;
}

static inline
vtss_rc lpm_ipv6_uc_deactivate_entry(vtss_state_t        *vtss_state,
                                     BOOL                 rt,
                                     const u32            idx,
                                     vtss_l3_lpm_table_t *const lpm,
                                     const vtss_l3_neighbour_cache_t *const nc)
{
    vtss_rc rc;
    u32 i;
    u32 hw_ptr = 0;
    u32 relation_ptr = 0;
    u32 hw_ptr_valid;
    vtss_ip_addr_t null_destination;

    D("lpm_ipv6_uc_deactivate_entry(idx=%u)", idx);

    VTSS_CHECK(lpm_ipv6_uc_entry_valid(idx, lpm), VTSS_RC_ERROR);

    // We normally want to deactivate an entry by redirecting to the NULL entry
    ip_null_address_get(VTSS_IP_TYPE_IPV6, &null_destination);
    rc = nc_ip_find(&null_destination, nc, &hw_ptr, &relation_ptr);
    hw_ptr_valid = rc == VTSS_RC_OK ? 1 : 0;

    if (hw_ptr_valid) {
        D("Found null ptr");
        rc = VTSS_FUNC_RT(l3.ipv6_uc_set, idx, hw_ptr, 0,
                          lpm->e[idx + 0].data, lpm->e[idx + 1].data,
                          lpm->e[idx + 2].data, lpm->e[idx + 3].data,
                          lpm->e[idx + 0].mask, lpm->e[idx + 1].mask,
                          lpm->e[idx + 2].mask, lpm->e[idx + 3].mask);
    } else {
        D("Disable in HW");
        rc = VTSS_FUNC_RT(l3.ipv6_uc_clear, idx);
    }

    VTSS_CHECK(rc == VTSS_RC_OK, VTSS_RC_INCOMPLETE);

    for (i = 0; i < 4; ++i) {
        if (hw_ptr_valid) {
            lpm->e[idx + i].ptr = hw_ptr;
            lpm->e[idx + i].ptr_valid = 1;
        } else {
            lpm->e[idx + i].ptr = 0;
            lpm->e[idx + i].ptr_valid = 0;
        }
    }

    return VTSS_RC_OK;
}

static inline
vtss_rc lpm_ipv4_uc_activate_entry(vtss_state_t        *vtss_state,
                                   BOOL                 rt,
                                   vtss_l3_lpm_table_t *const lpm,
                                   const u32            idx,
                                   const u32            ptr)
{
    vtss_rc rc;
    D("lpm_ipv4_uc_activate_entry(idx=%u, ptr=%u)", idx, ptr);

    VTSS_CHECK(lpm_ipv4_uc_entry_valid(idx, lpm), VTSS_RC_ERROR);

    if (lpm->e[idx].ptr_valid && lpm->e[idx].ptr == ptr)
        return VTSS_RC_OK; // state is allready uptodate

    rc = VTSS_FUNC_RT(l3.ipv4_uc_set, idx, ptr, 0, lpm->e[idx].data,
                      lpm->e[idx].mask);

    VTSS_CHECK(rc == VTSS_RC_OK, VTSS_RC_INCOMPLETE);

    lpm->e[idx].ptr = ptr;
    lpm->e[idx].ptr_valid = 1;

    return VTSS_RC_OK;
}

static inline
vtss_rc lpm_ipv6_uc_activate_entry(vtss_state_t        *vtss_state,
                                   BOOL                 rt,
                                   vtss_l3_lpm_table_t *const lpm,
                                   const u32            idx,
                                   const u32            ptr)
{
    vtss_rc rc;
    D("lpm_ipv6_uc_activate_entry(idx=%u, ptr=%u)", idx, ptr);

    VTSS_CHECK( lpm_ipv6_uc_entry_valid(idx, lpm), VTSS_RC_ERROR );

    if (lpm->e[idx].ptr_valid && lpm->e[idx].ptr == ptr)
        return VTSS_RC_OK;

    N("row %d, data: %08x %08x %08x %08x", idx, lpm->e[idx + 0].data,
      lpm->e[idx + 1].data, lpm->e[idx + 2].data, lpm->e[idx + 3].data);
    rc = VTSS_FUNC_RT(l3.ipv6_uc_set, idx, ptr, 0,
                      lpm->e[idx + 0].data, lpm->e[idx + 1].data,
                      lpm->e[idx + 2].data, lpm->e[idx + 3].data,
                      lpm->e[idx + 0].mask, lpm->e[idx + 1].mask,
                      lpm->e[idx + 2].mask, lpm->e[idx + 3].mask);

    VTSS_CHECK(rc == VTSS_RC_OK, VTSS_RC_INCOMPLETE);

    lpm->e[idx + 0].ptr = ptr;
    lpm->e[idx + 1].ptr = ptr;
    lpm->e[idx + 2].ptr = ptr;
    lpm->e[idx + 3].ptr = ptr;
    lpm->e[idx + 0].ptr_valid = 1;
    lpm->e[idx + 1].ptr_valid = 1;
    lpm->e[idx + 2].ptr_valid = 1;
    lpm->e[idx + 3].ptr_valid = 1;

    return VTSS_RC_OK;
}

static inline
vtss_rc lpm_ipv4_uc_set(vtss_state_t         *vtss_state,
                        BOOL                  rt,
                        vtss_l3_lpm_table_t  *const lpm,
                        const vtss_ipv4_uc_t *const entry,
                        const u32             idx,
                        const u32             ptr,
                        const BOOL            ptr_valid)
{
    vtss_rc rc;
    vtss_ipv4_t mask;
    vtss_ipv4_uc_tcam_data tcam_ip, tcam_mask, tcam_dest;

    VTSS_CHECK(idx < VTSS_LPM_CNT, VTSS_RC_ERROR );

    /* convert to TCAM specific data types */
    prefix_to_v4_mask(entry->network.prefix_size, &mask);
    ipv4_to_tcam(&entry->network.address, &tcam_ip);
    ipv4_to_tcam(&mask, &tcam_mask);
    ipv4_to_tcam(&entry->destination, &tcam_dest);

    if (ptr_valid) { /* we got a valid ARP ptr */
        D("IPv4UC-HW set %d, %d, %08x %08x", idx, ptr, tcam_ip.d, tcam_mask.d);
        rc = VTSS_FUNC_RT(l3.ipv4_uc_set, idx, ptr, 0, tcam_ip.d, tcam_mask.d);

        VTSS_CHECK( rc == VTSS_RC_OK, VTSS_RC_INCOMPLETE);

        lpm->e[idx].ptr = ptr;
        lpm->e[idx].ptr_valid = 1;

    } else { /* ARP enty is missing, no changes in hardware */
        D("IPv4UC-CACHE set %d, %d, %08x %08x",
          idx, ptr, tcam_ip.d, tcam_mask.d);
        lpm->e[idx].ptr = 0;
        lpm->e[idx].ptr_valid = 0;

    }

    /* Setup the entry */
    lpm->e[idx].valid = 1;
    lpm->e[idx].type = VTSS_L3_LPM_ENTRY_IPV4_UC;
    lpm->e[idx].ecmp = FALSE;
    lpm->e[idx].data = tcam_ip.d;
    lpm->e[idx].mask = tcam_mask.d;
    lpm->e[idx].dest = tcam_dest.d;

    return VTSS_RC_OK;
}

static inline
vtss_rc lpm_ipv6_uc_set(vtss_state_t         *vtss_state,
                        BOOL                  rt,
                        vtss_l3_lpm_table_t  *const lpm,
                        const vtss_ipv6_uc_t *const entry,
                        const u32             idx,
                        const u32             ptr,
                        const BOOL            ptr_valid)
{
    u32 i;
    vtss_rc rc;
    vtss_ipv6_t mask;
    vtss_ipv6_uc_tcam_data tcam_ip, tcam_mask, tcam_dest;

    VTSS_CHECK(idx + 3 < VTSS_LPM_CNT, VTSS_RC_ERROR);

    /* convert to TCAM specific data types */
    prefix_to_v6_mask(entry->network.prefix_size, &mask);
    ipv6_to_tcam(&entry->network.address, &tcam_ip);
    ipv6_to_tcam(&mask, &tcam_mask);
    ipv6_to_tcam(&entry->destination, &tcam_dest);

    if (ptr_valid) { /* we got a valid ARP ptr */
        D("IPv6UC-HW set %d, %d, %08x %08x %08x %08x %08x %08x %08x %08x",
          idx, ptr, tcam_ip.d[0], tcam_ip.d[1], tcam_ip.d[2], tcam_ip.d[3],
          tcam_mask.d[0], tcam_mask.d[1], tcam_mask.d[2], tcam_mask.d[3]);

        rc = VTSS_FUNC_RT(l3.ipv6_uc_set, idx, ptr, 0,
                          tcam_ip.d[0], tcam_ip.d[1], tcam_ip.d[2], tcam_ip.d[3],
                          tcam_mask.d[0], tcam_mask.d[1], tcam_mask.d[2],
                          tcam_mask.d[3]);

        VTSS_CHECK( rc == VTSS_RC_OK, VTSS_RC_INCOMPLETE);

        for (i = 0; i < 4; ++i ) {
            lpm->e[idx + i].ptr = ptr;
            lpm->e[idx + i].ptr_valid = 1;
        }

    } else { /* ARP enty is missing, no changes in hardware */
        D("IPv6UC-CACHE set %d, %d, %08x %08x %08x %08x %08x %08x %08x %08x",
          idx, ptr, tcam_ip.d[0], tcam_ip.d[1], tcam_ip.d[2], tcam_ip.d[3],
          tcam_mask.d[0], tcam_mask.d[1], tcam_mask.d[2], tcam_mask.d[3]);
        for (i = 0; i < 4; ++i) {
            lpm->e[idx + i].ptr = 0;
            lpm->e[idx + i].ptr_valid = 0;
        }
    }

    /* Setup the entry */
    for (i = 0; i < 4; ++i) {
        lpm->e[idx + i].valid = 1;
        lpm->e[idx + i].type = VTSS_L3_LPM_ENTRY_IPV6_UC;
        lpm->e[idx + i].ecmp = FALSE;
        lpm->e[idx + i].data = tcam_ip.d[i];
        lpm->e[idx + i].mask = tcam_mask.d[i];
        lpm->e[idx + i].dest = tcam_dest.d[i];
    }

    return VTSS_RC_OK;
}

static inline
vtss_rc lpm_set(vtss_state_t               *vtss_state,
                BOOL                        rt,
                vtss_l3_lpm_table_t        *const lpm,
                const vtss_routing_entry_t *const entry,
                const u32                   idx,
                const u32                   ptr,
                const BOOL                  ptr_valid)
{
    switch (entry->type) {
    case VTSS_ROUTING_ENTRY_TYPE_IPV4_UC:
        return lpm_ipv4_uc_set(vtss_state, rt, lpm, &entry->route.ipv4_uc, idx,
                               ptr, ptr_valid);

    case VTSS_ROUTING_ENTRY_TYPE_IPV6_UC:
        return lpm_ipv6_uc_set(vtss_state, rt, lpm, &entry->route.ipv6_uc, idx,
                               ptr, ptr_valid);

    default:
        D("lpm_route_find: type %d not supported", entry->type);
        return VTSS_RC_ERROR;
    }
}

static inline
vtss_rc lpm_ipv4_uc_find( const vtss_l3_lpm_table_t *const lpm,
                          const vtss_ipv4_uc_t      *const entry,
                          u32                       *const idx )
{
    u32 i;
    vtss_ipv4_t _mask;
    vtss_ipv4_uc_tcam_data search_data, search_mask;

    N("lpm_ipv4_uc_find(entry="IPV4_UC_FORMAT")", IPV4_UC_ARGS(*entry));

    prefix_to_v4_mask(entry->network.prefix_size, &_mask);

    ipv4_to_tcam(&_mask, &search_mask);
    ipv4_to_tcam(&entry->network.address, &search_data);

    for (i = 0; i < VTSS_LPM_CNT; ++i) {
        vtss_ipv4_uc_tcam_data data, mask;

        if (!lpm_ipv4_uc_data_get(lpm, &data, i))
            continue;

        if (!vtss_ipv4_uc_tcam_data_equal(&data, &search_data))
            continue;

        if (!lpm_ipv4_uc_mask_get(lpm, &mask, i))
            continue;

        if (!vtss_ipv4_uc_tcam_data_equal(&mask, &search_mask))
            continue;

        /* We found a match */
        *idx = i;
        return VTSS_RC_OK;
    }

    return VTSS_RC_ERROR;
}

static inline
vtss_rc lpm_ipv6_uc_find(const vtss_l3_lpm_table_t *const lpm,
                         const vtss_ipv6_uc_t      *const entry,
                         u32                       *const idx)
{
    u32 i;
    vtss_ipv6_t _mask;
    vtss_ipv6_uc_tcam_data search_data, search_mask;

    /* TODO */
    /* N("lpm_ipv6_uc_find(idx=%u, entry="IPV6_UC_FORMAT")",
       idx, IPV6_UC_ARGS(*entry)); */

    prefix_to_v6_mask(entry->network.prefix_size, &_mask);

    ipv6_to_tcam(&_mask, &search_mask);
    ipv6_to_tcam(&entry->network.address, &search_data);

    for (i = 0; i < VTSS_LPM_CNT; ++i) {
        vtss_ipv6_uc_tcam_data data, mask;

        if (!lpm_ipv6_uc_data_get(lpm, &data, i))
            continue;

        if (!vtss_ipv6_uc_tcam_data_equal(&data, &search_data))
            continue;

        if (!lpm_ipv6_uc_mask_get(lpm, &mask, i))
            continue;

        if (!vtss_ipv6_uc_tcam_data_equal(&mask, &search_mask))
            continue;

        /* We found a match */
        *idx = i;
        return VTSS_RC_OK;
    }

    return VTSS_RC_ERROR;
}

static inline
vtss_rc lpm_route_find( const vtss_l3_lpm_table_t  *const lpm,
                        const vtss_routing_entry_t *const entry,
                        u32                        *const idx )
{
    switch (entry->type) {
    case VTSS_ROUTING_ENTRY_TYPE_IPV4_UC:
        return lpm_ipv4_uc_find(lpm, &entry->route.ipv4_uc, idx);

    case VTSS_ROUTING_ENTRY_TYPE_IPV6_UC:
        return lpm_ipv6_uc_find(lpm, &entry->route.ipv6_uc, idx);

    default:
        D("lpm_route_find: type %d not supported", entry->type);
        return VTSS_RC_ERROR;
    }
}

/* END OF LPM MANIPULATORS -------------------------------------------- */

/* START OF ARP MANIPULATORS ------------------------------------------ */
static inline
vtss_rc lpm_ipv4_uc_activate(vtss_state_t        *vtss_state,
                             BOOL                 rt,
                             vtss_l3_lpm_table_t *const lpm,
                             const vtss_ipv4_t   *const ip,
                             const u32            ptr)
{
    u32 i;
    vtss_ipv4_uc_tcam_data ip_data;

    /* Convert IP data to TCAM data */
    ipv4_to_tcam(ip, &ip_data);

    /* search the routing table, and see if we can find inactive
     * entries who matches the data field */
    for (i = 0; i < VTSS_LPM_CNT; ++i) {
        vtss_ipv4_uc_tcam_data d;

        if (!lpm_ipv4_uc_dest_get(lpm, &d, i))
            continue;

        if (vtss_ipv4_uc_tcam_data_equal(&ip_data, &d))
            VTSS_RC_PRINT(lpm_ipv4_uc_activate_entry(vtss_state, rt, lpm, i, ptr));
    }

    return VTSS_RC_OK;
}

static inline
vtss_rc lpm_ipv6_uc_activate(vtss_state_t        *vtss_state,
                             BOOL                 rt,
                             vtss_l3_lpm_table_t *const lpm,
                             const vtss_ipv6_t   *ip,
                             const u32            ptr)
{
    u32 i;
    vtss_ipv6_uc_tcam_data ip_data;

    /* Convert IP data to TCAM data */
    ipv6_to_tcam(ip, &ip_data);

    /* search the routing table, and see if we can find inactive
     * entries who matches the data field */
    for (i = 0; i < VTSS_LPM_CNT; ++i) {
        vtss_ipv6_uc_tcam_data d;

        if (!lpm_ipv6_uc_dest_get(lpm, &d, i))
            continue;

        if (vtss_ipv6_uc_tcam_data_equal(&ip_data, &d))
            VTSS_RC_PRINT(lpm_ipv6_uc_activate_entry(vtss_state, rt, lpm, i, ptr));
    }

    return VTSS_RC_OK;
}

static inline
vtss_rc lpm_activate(vtss_state_t         *vtss_state,
                     BOOL                  rt,
                     vtss_l3_lpm_table_t  *const lpm,
                     const vtss_ip_addr_t *const ip,
                     const u32             ptr)
{
    switch (ip->type) {
    case VTSS_IP_TYPE_IPV4:
        return lpm_ipv4_uc_activate(vtss_state, rt, lpm, &ip->addr.ipv4, ptr);

    case VTSS_IP_TYPE_IPV6:
        return lpm_ipv6_uc_activate(vtss_state, rt, lpm, &ip->addr.ipv6, ptr);

    default:
        D("type %d not supported", ip->type);
        return VTSS_RC_ERROR;
    }
}

static inline
vtss_rc lpm_ipv4_uc_deactivate(vtss_state_t        *vtss_state,
                               BOOL                 rt,
                               const vtss_ipv4_t   *const ip,
                               vtss_l3_lpm_table_t *const lpm,
                               const vtss_l3_neighbour_cache_t *const nc)
{
    u32 i;
    vtss_ipv4_uc_tcam_data ip_data;

    /* Convert IP data to TCAM data */
    ipv4_to_tcam(ip, &ip_data);

    /* search the routing table, and see if we can find entries who matches
     * the data field */
    for (i = 0; i < VTSS_LPM_CNT; ++i) {
        vtss_ipv4_uc_tcam_data d;

        if (!lpm_ipv4_uc_dest_get(lpm, &d, i))
            continue;

        if (vtss_ipv4_uc_tcam_data_equal(&ip_data, &d)) {
            N("ipv4_uc_deactivate: %d, %08x", i, d.d);
            VTSS_RC_PRINT(lpm_ipv4_uc_deactivate_entry(vtss_state, rt, i, lpm, nc));
        }
    }

    return VTSS_RC_OK;
}

static inline
vtss_rc lpm_ipv6_uc_deactivate(vtss_state_t        *vtss_state,
                               BOOL                 rt,
                               const vtss_ipv6_t   *ip,
                               vtss_l3_lpm_table_t *const lpm,
                               const vtss_l3_neighbour_cache_t *const nc)
{
    u32 i;
    vtss_ipv6_uc_tcam_data ip_data;

    /* Convert IP data to TCAM data */
    ipv6_to_tcam(ip, &ip_data);

    /* search the routing table, and see if we can find inactive
     * entries who matches the data field */
    for (i = 0; i < VTSS_LPM_CNT; ++i) {
        vtss_ipv6_uc_tcam_data d;

        if (!lpm_ipv6_uc_dest_get(lpm, &d, i))
            continue;

        if (vtss_ipv6_uc_tcam_data_equal(&ip_data, &d))
            VTSS_RC_PRINT(lpm_ipv6_uc_deactivate_entry(vtss_state, rt, i, lpm, nc));
    }

    return VTSS_RC_OK;
}

static inline
vtss_rc lpm_deactivate(vtss_state_t         *vtss_state,
                       BOOL                  rt,
                       const vtss_ip_addr_t *const ip,
                       vtss_l3_lpm_table_t  *const lpm,
                       const vtss_l3_neighbour_cache_t *const nc)
{
    switch (ip->type) {
    case VTSS_IP_TYPE_IPV4:
        return lpm_ipv4_uc_deactivate(vtss_state, rt, &ip->addr.ipv4, lpm, nc);

    case VTSS_IP_TYPE_IPV6:
        return lpm_ipv6_uc_deactivate(vtss_state, rt, &ip->addr.ipv6, lpm, nc);

    default:
        D("type %d not supported", ip->type);
        return VTSS_RC_ERROR;
    }
}



static inline
vtss_rc nc_del(vtss_state_t              *vtss_state,
               BOOL                       rt,
               const vtss_l3_neighbour_t *const n,
               vtss_l3_neighbour_cache_t *const nc,
               vtss_l3_lpm_table_t       *const lpm)
{
    u32 hw_ptr = 0;
    u32 relation_ptr = 0;
    u32 match_cnt = 0;

#define BUF_SIZE 128
    char buf[BUF_SIZE];
    N("nc_del%s", vtss_neighbour_to_string(n, buf, BUF_SIZE, 0));
#undef BUF_SIZE

    /* Find the pointer of the affected ARP entry (do not delete it yet) */
    VTSS_RC_PRINT_INFO(nc_ip_find(&n->dip, nc, &hw_ptr, &relation_ptr));

    /* Scan through the LPM table, and look for entries whos
     * destination matches the IP part of the ARP entry (do not match
     * LPM entries only because the pointer matches). Matched entries
     * must be redirected to the NULL entry. */
    VTSS_RC_PRINT(lpm_deactivate(vtss_state, rt, &n->dip, lpm, nc));

    /* Invalidate the entry in the neighbour cache */
    switch ( n->dip.type ) {
    case VTSS_IP_TYPE_IPV4:
        N("nc_del2: ipv4idx=%d", relation_ptr);
        nc->ipv4[relation_ptr].valid = FALSE;
        break;

    case VTSS_IP_TYPE_IPV6:
        N("nc_del2: ipv6idx=%d", relation_ptr);
        nc->ipv6[relation_ptr].valid = FALSE;
        break;

    default:
        return VTSS_RC_ERROR;
    }

    /* Scan through the IPv4 and IPv6 relations, and count references */
    match_cnt += arp_ptr_match(nc, hw_ptr);
    match_cnt += ndp_ptr_match(nc, hw_ptr);
    N("nc_del3: match_cnt=%d", match_cnt);

    /* If no references was found, deleete the entry in the HW entry */
    if (match_cnt == 0) {
        VTSS_RC_PRINT(VTSS_FUNC_RT(l3.arp_clear, hw_ptr));
        nc->hw[hw_ptr].valid = FALSE;
        N("nc_del4: hw_ptr=%d -> invalid", hw_ptr);
    }

    return VTSS_RC_OK;
}

static inline
BOOL hw_mac_find(const vtss_l3_neighbour_cache_t *const nc,
                 const vtss_mac_t                *const mac,
                 const u32                       rleg,
                 u32                             *const hw_ptr)
{
    u32 i;

    for (i = 0; i < VTSS_ARP_CNT; ++i) {
        if (! nc->hw[i].valid)
            continue;

        if (! nc->hw[i].arp_enable)
            continue;

        if (mac_equal(mac, &nc->hw[i].dmac) &&
            rleg == nc->hw[i].egress_rleg) {

            /* MAC address is allready present */
            *hw_ptr = i;
            return TRUE;
        }
    }

    return FALSE;
}

static inline
vtss_rc add_hw_mac(vtss_state_t              *vtss_state,
                   BOOL                       rt,
                   const vtss_mac_t          *const dmac,
                   const vtss_l3_rleg_id_t    rleg,
                   vtss_l3_neighbour_cache_t *const nc,
                   u32                       *const ptr)
{
    u32 i = 0;
    BOOL found_free_entry;

    /* Find a free spot in the hardware table */
    found_free_entry = FALSE;
    for (i = 0; i < VTSS_ARP_CNT; ++i) {
        if (!nc->hw[i].valid) {
            *ptr = i;
            found_free_entry = TRUE;
            break;
        }
    }

    if (! found_free_entry) {
        /* We are out of free space in the ARP table */
        D("add_hw_mac: no free slots in hw arp table");
        return VTSS_RC_ERROR;
    }

    /* Perform the update in HW. We have not done any thing yet, and
     * does therefor not need to handle errors */
    VTSS_RC_PRINT(VTSS_FUNC_RT(l3.arp_set, i, dmac, rleg));

    /* Update the software cached view of HW */
    VTSS_CHECK(i < VTSS_ARP_CNT, VTSS_RC_ERROR); /* Keep lint happy*/
    nc->hw[i].valid = TRUE;
    nc->hw[i].egress_rleg = rleg;
    nc->hw[i].secure_vmid = FALSE;
    nc->hw[i].secure_mac = FALSE;
    nc->hw[i].arp_enable = TRUE;
    mac_cpy(&nc->hw[i].dmac, dmac);

    return VTSS_RC_OK;
}

static inline
vtss_rc nc_free_relation_get(const vtss_l3_neighbour_cache_t *const nc,
                             const vtss_ip_type_t            type,
                             u32                             *const ptr)
{
    u32 i;

    switch (type) {
    case VTSS_IP_TYPE_IPV4:
        for (i = 0; i < VTSS_ARP_IPV4_RELATIONS; ++i)
            if ( ! nc->ipv4[i].valid ) {
                *ptr = i;
                N("nc_free_relation_get: %u", i);
                return VTSS_RC_OK;
            }
        break;

    case VTSS_IP_TYPE_IPV6:
        for (i = 0; i < VTSS_ARP_IPV6_RELATIONS; ++i)
            if (! nc->ipv6[i].valid) {
                *ptr = i;
                N("nc_free_relation_get: %u", i);
                return VTSS_RC_OK;
            }
        break;

    default:
        D("Type not supported: %d", type);
        return VTSS_RC_ERROR;
    }

    D("Out of resources");
    return VTSS_RC_ERROR;
}

static
vtss_rc nc_add(vtss_state_t              *vtss_state,
               BOOL                       rt,
               const vtss_l3_neighbour_t *const n,
               vtss_l3_rleg_id_t          const rleg,
               vtss_l3_neighbour_cache_t *const nc,
               vtss_l3_lpm_table_t       *const lpm)
{
    vtss_rc rc_;
    u32 hw_ptr = 0;
    u32 ip_ptr = 0;
    u32 relation_ptr = 0;
    BOOL mac_allready_present = FALSE;

#define BUF_SIZE 128
    char buf[BUF_SIZE];
    N("nc_add %s", vtss_neighbour_to_string(n, buf, BUF_SIZE, 0));

    rc_ = nc_ip_find(&n->dip, nc, &hw_ptr, &relation_ptr);

    if (rc_ == VTSS_RC_OK) {
        vtss_l3_neighbour_t _n;
        _n.dip = n->dip;
        _n.vlan = n->vlan;
        _n.dmac = nc->hw[hw_ptr].dmac;

        D("nc_add: "MAC_FORMAT" -> "MAC_FORMAT,
          MAC_ARGS(nc->hw[hw_ptr].dmac), MAC_ARGS(n->dmac));

        if (nc_del(vtss_state, rt, &_n, nc, lpm) != VTSS_RC_OK) {
            E("Check failed: nc_del(_n, nc, lpm), n = %s",
              vtss_neighbour_to_string(&_n, buf, BUF_SIZE, 0));
            return VTSS_RC_ERROR;
        }

        // Recursive call
        return nc_add(vtss_state, rt, n, rleg, nc, lpm);
    }
#undef BUF_SIZE

    /* Check if the mac address is allready present in HW */
    mac_allready_present = hw_mac_find(nc, &n->dmac, rleg, &hw_ptr);

    /* Find a free spot for the IP relation */
    VTSS_RC_PRINT(nc_free_relation_get(nc, n->dip.type, &ip_ptr));

    /* We need to add a new entry in HW */
    if (! mac_allready_present) {
        VTSS_RC_PRINT(add_hw_mac(vtss_state, rt, &n->dmac, rleg, nc, &hw_ptr));
    }

    /* Registerer the ip relation */
    switch ( n->dip.type ) {
    case VTSS_IP_TYPE_IPV4:
        N("nc_add2: ipv4idx=%d, hwptr=%d", ip_ptr, hw_ptr);
        nc->ipv4[ip_ptr].valid = TRUE;
        nc->ipv4[ip_ptr].ip = n->dip.addr.ipv4;
        nc->ipv4[ip_ptr].ptr = hw_ptr;
        break;

    case VTSS_IP_TYPE_IPV6:
        N("nc_add2: ipv6idx=%d, hwptr=%d", ip_ptr, hw_ptr);
        nc->ipv6[ip_ptr].valid = TRUE;
        nc->ipv6[ip_ptr].ip = n->dip.addr.ipv6;
        nc->ipv6[ip_ptr].ptr = hw_ptr;
        break;

    default:
        E("Invalid type %d", n->dip.type);
        return VTSS_RC_ERROR;
    }

    /* Announce the update to the LPM table */
    return lpm_activate(vtss_state, rt, lpm, &n->dip, hw_ptr);
}

static inline
vtss_rc nc_flush(vtss_state_t              *vtss_state,
                 BOOL                       rt,
                 vtss_l3_neighbour_cache_t *const nc,
                 vtss_l3_lpm_table_t       *const lpm)
{
    u32 i;
    vtss_l3_neighbour_t n;

    u32 ok_cnt = 0;
    u32 error_cnt = 0;
    u32 incomplete_cnt = 0;

    D("nc_flush");

    for (i = 0; i < VTSS_ARP_IPV4_RELATIONS; ++i) {
        vtss_rc rc_;

        if (! nc->ipv4[i].valid)
            continue;

        n.dip.type = VTSS_IP_TYPE_IPV4;
        n.dip.addr.ipv4 = nc->ipv4[i].ip;
        rc_ = nc_del(vtss_state, rt, &n, nc, lpm);

        switch (rc_) {
        case VTSS_RC_OK:
            ++ok_cnt;
            break;

        case VTSS_RC_ERROR:
            ++error_cnt;
            break;

        default:
            ++incomplete_cnt;
            break;
        }
    }

    for (i = 0; i < VTSS_ARP_IPV6_RELATIONS; ++i) {
        vtss_rc rc_;

        if (! nc->ipv6[i].valid)
            continue;

        n.dip.type = VTSS_IP_TYPE_IPV6;
        n.dip.addr.ipv6 = nc->ipv6[i].ip;
        rc_ = nc_del(vtss_state, rt, &n, nc, lpm);

        switch (rc_) {
        case VTSS_RC_OK:
            ++ok_cnt;
            break;

        case VTSS_RC_ERROR:
            ++error_cnt;
            break;

        default:
            ++incomplete_cnt;
            break;
        }
    }

    if (error_cnt == 0 && incomplete_cnt == 0)
        return VTSS_RC_OK;

    if (ok_cnt == 0 && incomplete_cnt == 0)
        return VTSS_RC_ERROR;

    return VTSS_RC_INCOMPLETE;
}
/* END OF ARP MANIPULATORS -------------------------------------------- */

static inline
vtss_rc route_prefix_get(const vtss_routing_entry_t *const route,
                         u32                        *prefix)
{
    switch (route->type) {
    case VTSS_ROUTING_ENTRY_TYPE_IPV4_UC:
        *prefix = route->route.ipv4_uc.network.prefix_size;
        return VTSS_RC_OK;

    case VTSS_ROUTING_ENTRY_TYPE_IPV6_UC:
        *prefix = route->route.ipv6_uc.network.prefix_size;
        return VTSS_RC_OK;

    default:
        D("get_route_prefix: type %d not supported", route->type);
        return VTSS_RC_ERROR;
    }
}

static inline
vtss_rc route_destinaiton_get(const vtss_routing_entry_t *const route,
                              vtss_ip_addr_t             *dest)
{
    switch (route->type) {
    case VTSS_ROUTING_ENTRY_TYPE_IPV4_UC:
        dest->type = VTSS_IP_TYPE_IPV4;
        dest->addr.ipv4 = route->route.ipv4_uc.destination;
        return VTSS_RC_OK;

    case VTSS_ROUTING_ENTRY_TYPE_IPV6_UC:
        dest->type = VTSS_IP_TYPE_IPV6;
        dest->addr.ipv6 = route->route.ipv6_uc.destination;
        return VTSS_RC_OK;

    default:
        D("route_destinaiton_get: type %d not supported", route->type);
        return VTSS_RC_ERROR;
    }
}

static inline
vtss_rc route_add(vtss_state_t                    *vtss_state,
                  BOOL                             rt,
                  const vtss_routing_entry_t      *const route,
                  vtss_l3_lpm_table_t             *const lpm,
                  const vtss_l3_neighbour_cache_t *const nc)
{
    u32 idx;
    vtss_rc rc_;
    u32 hw_ptr = 0;
    u32 prefix = 0;
    u32 relation_ptr = 0;
    vtss_ip_addr_t destination;
    BOOL hw_ptr_valid;
    vtss_l3_lpm_entry_type lpm_type;

    rc_ = lpm_route_find(lpm, route, &idx );

    if (rc_ == VTSS_RC_OK) {
#define BUF_LENGTH 128
        char buf[BUF_LENGTH];
        E("route_add: route %s allready exists",
          vtss_routing_entry_to_string(route, buf, BUF_LENGTH, 0));
#undef BUF_LENGTH
        return VTSS_RC_ERROR;
    }

    /* Extract informaiton from the abstract route type */
    VTSS_RC_PRINT(route_prefix_get(route, &prefix));
    VTSS_RC_PRINT(route_destinaiton_get(route, &destination));
    VTSS_RC_PRINT(vtss_routing_type_to_lpm_type(route->type, &lpm_type));

    /* Make room for the entry in the LPM table */
    VTSS_RC_PRINT(lpm_alloc(vtss_state, rt, lpm, lpm_type, prefix, &idx));

    /* Check of the destination is allready in the neighbour cache */
    rc_ = nc_ip_find(&destination, nc, &hw_ptr, &relation_ptr);
    hw_ptr_valid = (rc_ == VTSS_RC_OK ? TRUE : FALSE);

    /* If we did not find the destination in the ARP table, look for a NULL
     * entry and use that instead */
    ip_null_address_get(destination.type, &destination);
    rc_ = nc_ip_find(&destination, nc, &hw_ptr, &relation_ptr);
    hw_ptr_valid = (rc_ == VTSS_RC_OK ? TRUE : FALSE);

    /* Update the newly allocated slot in the LPM table */
    rc_ = lpm_set(vtss_state, rt, lpm, route, idx, hw_ptr, hw_ptr_valid);
    if (rc_ != VTSS_RC_OK) {
        E("route_add: lpm_ipv4_uc_set failed");
        goto Error;
    }

    return VTSS_RC_OK;

Error:
    /* the LPM set operation failed. Free the allocated LPM slot */
    rc_ = lpm_free(vtss_state, rt, lpm, idx); /* We have no way to handle errors here */

    if (rc_ != VTSS_RC_OK)
        E("ERROR: Unrecoverable state returned from %s", __FUNCTION__);

    return VTSS_RC_ERROR;
}

static inline
vtss_rc route_del(vtss_state_t               *vtss_state,
                  BOOL                        rt,
                  const vtss_routing_entry_t *const route,
                  vtss_l3_lpm_table_t        *const lpm)
{
    u32 idx = 0;
    VTSS_RC_PRINT_INFO(lpm_route_find(lpm, route, &idx));
    return lpm_free(vtss_state, rt, lpm, idx);
}

static inline
vtss_rc route_flush(vtss_state_t        *vtss_state,
                    BOOL                 rt,
                    vtss_l3_lpm_table_t *const lpm)
{
    u32 ok_cnt = 0;
    u32 error_cnt = 0;
    u32 incomplete_cnt = 0;
    D("route_flush");

    while (lpm->e[0].valid) {
        vtss_rc rc_ = lpm_free(vtss_state, rt, lpm, 0);

        switch( rc_ ) {
        case VTSS_RC_OK:
            ++ok_cnt;
            break;

        case VTSS_RC_ERROR:
            ++error_cnt;
            break;

        default:
            ++incomplete_cnt;
            break;
        }
    }

    if (error_cnt == 0 && incomplete_cnt == 0)
        return VTSS_RC_OK;

    if (ok_cnt == 0 && incomplete_cnt == 0)
        return VTSS_RC_ERROR;

    return VTSS_RC_INCOMPLETE;
}

static
vtss_rc create_null_arp_entry(vtss_state_t              *vtss_state,
                              vtss_l3_lpm_table_t       *const lpm,
                              vtss_l3_neighbour_cache_t *const nc)
{
    // NOTE: this function does not depend on whether routing is enabled or
    // disabled. We must be able to insert NULL entries in either case.
    u32 hw_ptr = 0;
    u32 relation_ptr = 0;
    vtss_l3_neighbour_t null_ipv4;
    vtss_l3_neighbour_t null_ipv6;
    memset(&null_ipv4, 0, sizeof(vtss_l3_neighbour_t));
    memset(&null_ipv6, 0, sizeof(vtss_l3_neighbour_t));
    null_ipv4.dip.type = VTSS_IP_TYPE_IPV4;
    null_ipv6.dip.type = VTSS_IP_TYPE_IPV6;

    if (nc_ip_find(&null_ipv4.dip, nc, &hw_ptr, &relation_ptr) != VTSS_RC_OK)
        VTSS_RC_PRINT(nc_add(vtss_state, TRUE, // force route enable
                             &null_ipv4, 0, nc, lpm));

    if (nc_ip_find(&null_ipv6.dip, nc, &hw_ptr, &relation_ptr) != VTSS_RC_OK)
        VTSS_RC_PRINT(nc_add(vtss_state, TRUE, // force route enable
                             &null_ipv6, 0, nc, lpm));

    return VTSS_RC_OK;
}

static vtss_rc routing_disable(vtss_state_t *vtss_state)
{
    unsigned i;

    for (i = 0; i < VTSS_LPM_CNT; ++i) {
        VTSS_RC_PRINT(VTSS_FUNC(l3.ipv4_uc_clear, i));
    }

    for (i = 0; i < VTSS_ARP_CNT; ++i) {
        VTSS_RC_PRINT(VTSS_FUNC(l3.arp_clear, i));
    }

    return VTSS_RC_OK;
}

static vtss_rc routing_enable(vtss_state_t                    *vtss_state,
                              const vtss_l3_lpm_table_t       *const lpm,
                              const vtss_l3_neighbour_cache_t *const nc)
{
    unsigned i = 0;

    (void) routing_disable(vtss_state);

    while (i < VTSS_LPM_CNT) {
        if (lpm_ipv4_uc_entry_valid(i, lpm) && lpm->e[i].ptr_valid) {
            VTSS_RC_PRINT(VTSS_FUNC(l3.ipv4_uc_set, i, lpm->e[i].ptr, 0,
                                    lpm->e[i].data, lpm->e[i].mask));
            i += 1;

        } else if (lpm_ipv6_uc_entry_valid(i, lpm) && lpm->e[i].ptr_valid) {
            /*lint --e{661} */
            /*lint --e{662} */
            /*lint --e{831} */
            VTSS_RC_PRINT(VTSS_FUNC(l3.ipv6_uc_set, i, lpm->e[i].ptr, 0,
                                    lpm->e[i + 0].data, lpm->e[i + 1].data,
                                    lpm->e[i + 2].data, lpm->e[i + 3].data,
                                    lpm->e[i + 0].mask, lpm->e[i + 1].mask,
                                    lpm->e[i + 2].mask, lpm->e[i + 3].mask));
            i += 4;

        } else {
            i += 1;

        }
    }

    for (i = 0; i < VTSS_ARP_CNT; ++i) {
        if (nc->hw[i].valid) {
            VTSS_RC_PRINT(VTSS_FUNC(l3.arp_set, i, &nc->hw[i].dmac,
                                    nc->hw[i].egress_rleg));
        }
    }

    return VTSS_RC_OK;
}
#  endif /* VTSS_SW_OPTION_L3RT */

/* Implementing the public API */
vtss_rc vtss_l3_common_get(const vtss_inst_t      inst,
                           vtss_l3_common_conf_t *const conf)
{
    vtss_state_t *vtss_state;
    vtss_rc rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        *conf = vtss_state->l3.common;
        rc = VTSS_RC_OK;
    }
    VTSS_EXIT();

    return rc;
}

#  if defined(VTSS_SW_OPTION_L3RT)
vtss_rc vtss_l3_common_set(const vtss_inst_t            inst,
                           const vtss_l3_common_conf_t *const conf)
{
    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_OK;
    BOOL is_initialized = FALSE;
    BOOL is_routing_enable = FALSE;

#define DO(X) if (rc == VTSS_RC_OK) { rc = (X); }
    I("%s %d " MAC_FORMAT, __FUNCTION__, conf->rleg_mode,
      MAC_ARGS(conf->base_address));

    VTSS_ENTER();
    DO(vtss_inst_check(inst, &vtss_state));
    DO(VTSS_FUNC(l3.common_set, conf));
    if (rc == VTSS_RC_OK) {
        is_routing_enable = vtss_state->l3.common.routing_enable;
        is_initialized = vtss_state->l3.initialized;
        vtss_state->l3.common = *conf;
    }

    if (rc == VTSS_RC_OK && !is_initialized) {
        // create some common entries which we will depend on later
        DO(create_null_arp_entry(vtss_state,
                                 &(vtss_state->l3.lpm),
                                 &(vtss_state->l3.neighbour_cache)));
        vtss_state->l3.initialized = TRUE;
    }

    if (rc == VTSS_RC_OK && is_initialized) {
        // We only need to take an action if routing mode is changed after
        // initialization.
        if (is_routing_enable && !conf->routing_enable) {
            I("Routing is disabled at runtime");
            DO(routing_disable(vtss_state));
            DO(hardcode_null_arp_entry(vtss_state));

        } else if (!is_routing_enable && conf->routing_enable) {
            I("Routing is enabled at runtime");
            DO(routing_enable(vtss_state,
                              &vtss_state->l3.lpm,
                              &vtss_state->l3.neighbour_cache));

        }
    }
    VTSS_EXIT();

#undef DO
    return rc;
}
#else
vtss_rc vtss_l3_common_set(const vtss_inst_t            inst,
                           const vtss_l3_common_conf_t *const conf)
{
    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_OK;

#define DO(X) if (rc == VTSS_RC_OK) { rc = (X); }
    I("%s %d " MAC_FORMAT, __FUNCTION__, conf->rleg_mode,
      MAC_ARGS(conf->base_address));

    VTSS_ENTER();
    DO(vtss_inst_check(inst, &vtss_state));
    DO(VTSS_FUNC(l3.common_set, conf));
    if (rc == VTSS_RC_OK) {
        vtss_state->l3.common = *conf;
    }
    (void) hardcode_null_arp_entry(vtss_state);
    VTSS_EXIT();
#undef DO
    return rc;
}
#endif /* VTSS_SW_OPTION_L3RT */


#if defined(VTSS_SW_OPTION_L3RT)
vtss_rc vtss_l3_flush(const vtss_inst_t inst)
{
    vtss_state_t *vtss_state;
    BOOL rt = FALSE;
    vtss_rc rc = VTSS_RC_OK;
    I("Flush");

#define DO(X)                    \
    if (rc == VTSS_RC_OK) {      \
        rc = (X);                \
        if (rc != VTSS_RC_OK) {  \
            E("Failed: " #X);    \
        }                        \
    }
    VTSS_ENTER();
    DO(vtss_inst_check(inst, &vtss_state));
    if (rc == VTSS_RC_OK) {
        rt = vtss_state->l3.common.routing_enable;
    }
    DO(route_flush(vtss_state, rt, &vtss_state->l3.lpm));
    DO(nc_flush(vtss_state, rt, &vtss_state->l3.neighbour_cache, &vtss_state->l3.lpm));
    DO(rleg_flush(vtss_state, vtss_state->l3.rleg_conf));
    DO(create_null_arp_entry(vtss_state,
                             &(vtss_state->l3.lpm),
                             &(vtss_state->l3.neighbour_cache)));
    VTSS_EXIT();
    // aquire mutex of it's own
    DO(vtss_l3_counters_reset(inst));
#undef DO
    return rc;
}
#else
vtss_rc vtss_l3_flush(const vtss_inst_t inst)
{
    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_OK;
    I("Flush");

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK)
        rc = rleg_flush(vtss_state, vtss_state->l3.rleg_conf);
    VTSS_EXIT();

    return rc;
}
#endif /* VTSS_SW_OPTION_L3RT */

vtss_rc vtss_l3_rleg_get(const vtss_inst_t     inst,
                         u32                  *cnt,
                         vtss_l3_rleg_conf_t   buf[VTSS_RLEG_CNT])
{
    vtss_state_t *vtss_state;
    vtss_rc rc;
    u32 i;
    u32 _cnt = 0;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        for (i = 0; i < VTSS_RLEG_CNT; ++i) {
            // Only consider valid entries
            if (vtss_state->l3.rleg_conf[i].vlan == 0)
                continue;
            
            // Stop when max is reached. Should not be needed
            if (_cnt >= VTSS_RLEG_CNT) {
                I("Early stop due to buffer limit");
                break;
            }
            
            buf[_cnt++] = vtss_state->l3.rleg_conf[i];
            buf[_cnt - 1].rl_idx = i;
        }
    }
    VTSS_EXIT();

    *cnt = _cnt;
    return rc;
}

vtss_rc vtss_l3_rleg_add(const vtss_inst_t          inst,
                         const vtss_l3_rleg_conf_t *const conf)
{
    vtss_state_t *vtss_state;
    vtss_rc rc;

    I("%s vlan:%u %s %s %s %s",
      __FUNCTION__,
      conf->vlan,
      (conf->ipv4_unicast_enable ? "ipv4_uc" : "no-ipv4_uc"),
      (conf->ipv6_unicast_enable ? "ipv6_uc" : "no-ipv6_uc"),
      (conf->ipv4_icmp_redirect_enable ? "ipv4_mc" : "no-ipv4_mc"),
      (conf->ipv6_icmp_redirect_enable ? "ipv4_mc" : "no-ipv4_mc")
    );

    if (conf->vlan == VTSS_VID_NULL || conf->vlan > VTSS_VID_RESERVED) {
        E("%u is not a valid vlan id", conf->vlan);
        return VTSS_RC_ERROR;
    }

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        rc = rleg_add(vtss_state, vtss_state->l3.rleg_conf, conf);
    }
    VTSS_EXIT();

    return rc;
}

vtss_rc vtss_l3_rleg_del(const vtss_inst_t inst,
                         const vtss_vid_t  vlan)
{
    vtss_state_t *vtss_state;
    vtss_rc rc;
    I("%s vlan:%u", __FUNCTION__, vlan);

    if (vlan == VTSS_VID_NULL || vlan > VTSS_VID_RESERVED) {
        E("%u is not a valid vlan id", vlan);
        return VTSS_RC_ERROR;
    }

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        rc = rleg_del(vtss_state, vtss_state->l3.rleg_conf, vlan);
    }
    VTSS_EXIT();

    return rc;
}

#  if defined(VTSS_SW_OPTION_L3RT)
vtss_rc vtss_l3_counters_reset(const vtss_inst_t inst)
{
    vtss_state_t *vtss_state;
    BOOL rt;
    vtss_rc rc;
    I("l3_counters_reset");

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        rt = vtss_state->l3.common.routing_enable;
        rc = VTSS_FUNC_RT(l3.rleg_counters_reset);
        memset(&(vtss_state->l3.statistics), 0, sizeof(vtss_l3_statistics_t));
    }
    VTSS_EXIT();

    return rc;
}

vtss_rc vtss_l3_counters_system_get(const vtss_inst_t  inst,
                                    vtss_l3_counters_t *const counters)
{
    vtss_state_t *vtss_state;
    vtss_rc rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        memcpy(counters, &(vtss_state->l3.statistics.system_counter),
               sizeof(vtss_l3_counters_t));
    }
    VTSS_EXIT();

    return rc;
}

vtss_rc vtss_l3_counters_rleg_get(const vtss_inst_t   inst,
                                  const vtss_vid_t    vlan,
                                  vtss_l3_counters_t *const counters)
{
#define DO(X)                                          \
    if (rc == VTSS_RC_OK) {                            \
        rc = (X);                                      \
        if (rc != VTSS_RC_OK) {                        \
            E("Failed: " #X "rleg_id = %d, vlan = %d", \
              rleg, vlan);                             \
        }                                              \
    }

    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_OK;
    vtss_l3_rleg_id_t rleg = 0;
    D("%s vlan:%u", __FUNCTION__, vlan);

    if (vlan == VTSS_VID_NULL || vlan > VTSS_VID_RESERVED) {
        E("%u is not a valid vlan id", vlan);
        return VTSS_RC_ERROR;
    }

    VTSS_ENTER();
    DO(vtss_inst_check(inst, &vtss_state));
    DO(rleg_id_get(vtss_state->l3.rleg_conf, vlan, &rleg, 0));
    if (rc == VTSS_RC_OK) {
        (void) memcpy(counters,
                      &(vtss_state->l3.statistics.interface_counter[rleg]),
                      sizeof(vtss_l3_counters_t));
    }
    VTSS_EXIT();

#undef DO
    return rc;
}

vtss_rc vtss_l3_counters_rleg_clear(const vtss_inst_t   inst,
                                    const vtss_vid_t    vlan)
{
#define DO(X)                                          \
    if (rc == VTSS_RC_OK) {                            \
        rc = (X);                                      \
        if (rc != VTSS_RC_OK) {                        \
            E("Failed: " #X "rleg_id = %d, vlan = %d", \
              rleg, vlan);                             \
        }                                              \
    }

    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_OK;
    vtss_l3_rleg_id_t rleg = 0;
    D("%s vlan:%u", __FUNCTION__, vlan);

    if (vlan == VTSS_VID_NULL || vlan > VTSS_VID_RESERVED) {
        E("%u is not a valid vlan id", vlan);
        return VTSS_RC_ERROR;
    }

    VTSS_ENTER();
    DO(vtss_inst_check(inst, &vtss_state));
    DO(rleg_id_get(vtss_state->l3.rleg_conf, vlan, &rleg, 0));
    if (rc == VTSS_RC_OK) {
        (void) memset(&(vtss_state->l3.statistics.interface_counter[rleg]), 0,
                      sizeof(vtss_l3_counters_t));
    }
    VTSS_EXIT();

#undef DO
    return rc;
}

vtss_rc vtss_l3_route_get(const vtss_inst_t     inst,
                          u32                  *cnt,
                          vtss_routing_entry_t  buf[VTSS_LPM_CNT])
{
#define BUF_LENGTH 128
    vtss_state_t *vtss_state;
    vtss_rc rc;
    u32 i, _cnt = 0;
    char msg[BUF_LENGTH];
    const vtss_l3_lpm_table_t *lpm;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        lpm = &vtss_state->l3.lpm;

        for (i = 0; i < VTSS_LPM_CNT;++i) {
            if (_cnt >= VTSS_LPM_CNT) {
                I("Early stop due to buffer limit");
                break;
            }

            if (lpm_ipv4_uc_entry_valid(i, lpm)) {
                u32 prefix = 0;
                vtss_ipv4_t ip, mask, gw;
                vtss_ipv4_uc_tcam_data _ip, _mask, _gw;
                memset(&_ip, 0, sizeof(_ip));
                memset(&_mask, 0, sizeof(_mask));
                memset(&_gw, 0, sizeof(_gw));

                buf[_cnt].type = VTSS_ROUTING_ENTRY_TYPE_IPV4_UC;

                (void)lpm_ipv4_uc_data_get(lpm, &_ip, i);
                (void)lpm_ipv4_uc_mask_get(lpm, &_mask, i);
                (void)lpm_ipv4_uc_dest_get(lpm, &_gw, i);

                tcam_to_ipv4(&_ip, &ip);
                tcam_to_ipv4(&_mask, &mask);
                tcam_to_ipv4(&_gw, &gw);

                prefix = v4_mask_to_prefix(&mask);

                buf[_cnt].route.ipv4_uc.network.address = ip;
                buf[_cnt].route.ipv4_uc.network.prefix_size = prefix;
                buf[_cnt].route.ipv4_uc.destination = gw;
                I("route_get - adding: %s",
                  vtss_routing_entry_to_string(&buf[_cnt], msg, BUF_LENGTH, 0));
                _cnt ++;
                /*
                   } else if (lpm_ipv4_mc_entry_valid(i, lpm)) {
                   */
            } else if (lpm_ipv6_uc_entry_valid(i, lpm)) {
                u32 prefix = 0;
                vtss_ipv6_t ip, mask, gw;
                vtss_ipv6_uc_tcam_data _ip, _mask, _gw;
                memset(&_ip, 0, sizeof(_ip));
                memset(&_mask, 0, sizeof(_mask));
                memset(&_gw, 0, sizeof(_gw));

                buf[_cnt].type = VTSS_ROUTING_ENTRY_TYPE_IPV6_UC;

                (void)lpm_ipv6_uc_data_get(lpm, &_ip, i);
                (void)lpm_ipv6_uc_mask_get(lpm, &_mask, i);
                (void)lpm_ipv6_uc_dest_get(lpm, &_gw, i);

                tcam_to_ipv6(&_ip, &ip);
                tcam_to_ipv6(&_mask, &mask);
                tcam_to_ipv6(&_gw, &gw);

                prefix = v6_mask_to_prefix(&mask);

                buf[_cnt].route.ipv6_uc.network.address = ip;
                buf[_cnt].route.ipv6_uc.network.prefix_size = prefix;
                buf[_cnt].route.ipv6_uc.destination = gw;
                _cnt ++;

                i += 3; // ipv6_uc entries consumes 4 entries
                        // and the forloop increment once.
            }
        }

        *cnt = _cnt;
    }
    VTSS_EXIT();

    return rc;
#undef BUF_LENGTH
}

vtss_rc vtss_l3_route_add(const vtss_inst_t          inst,
                          const vtss_routing_entry_t *const entry)
{
#define BUF_LENGTH 128
    vtss_state_t *vtss_state;
    BOOL rt;
    vtss_rc rc;
    char buf[BUF_LENGTH];

    I("route_add: %s",
      vtss_routing_entry_to_string(entry, buf, BUF_LENGTH, 0));

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        rt = vtss_state->l3.common.routing_enable;
        rc = route_add(vtss_state,
                       rt,
                       entry,
                       &vtss_state->l3.lpm,
                       &vtss_state->l3.neighbour_cache);
    }
    VTSS_EXIT();

#undef BUF_LENGTH
    return rc;
}

vtss_rc vtss_l3_route_del(const vtss_inst_t          inst,
                          const vtss_routing_entry_t *const entry)
{
#define BUF_LENGTH 128
    vtss_state_t *vtss_state;
    BOOL rt;
    vtss_rc rc;
    char buf[BUF_LENGTH];

    I("route_del: %s",
      vtss_routing_entry_to_string(entry, buf, BUF_LENGTH, 0));

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        rt = vtss_state->l3.common.routing_enable;
        rc = route_del(vtss_state, rt, entry, &vtss_state->l3.lpm);
    }
    VTSS_EXIT();

#undef BUF_LENGTH
    return rc;
}

vtss_rc vtss_l3_neighbour_get(const vtss_inst_t    inst,
                              u32                 *cnt,
                              vtss_l3_neighbour_t  buf[VTSS_ARP_CNT])
{
    vtss_state_t *vtss_state;
    vtss_rc rc;
    u32 i, _cnt = 0;
    const vtss_l3_neighbour_cache_t *nc;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        nc = &vtss_state->l3.neighbour_cache;

        for (i = 0; i < VTSS_ARP_IPV4_RELATIONS;++i) {
            const vtss_l3_arp_hw_entry_t *arp;
            vtss_vid_t vlan;

            if (_cnt >= VTSS_ARP_CNT) {
                I("Early stop due to buffer limit");
                break;
            }

            if (!nc->ipv4[i].valid)
                continue;

            arp = &nc->hw[nc->ipv4[i].ptr];

            if (!arp->valid) {
                E("CHECK FAILED: %s %d", __FILE__, __LINE__);
                continue;
            }

            // The null entry should not be forwarded
            if (arp->dmac.addr[0] == 0 && arp->dmac.addr[1] == 0 &&
                arp->dmac.addr[2] == 0 && arp->dmac.addr[3] == 0 &&
                arp->dmac.addr[4] == 0 && arp->dmac.addr[5] == 0) {
                continue;
            }

            vlan = vtss_state->l3.rleg_conf[arp->egress_rleg].vlan;

            if (vlan == 0) {
                E("CHECK FAILED: %s %d", __FILE__, __LINE__);
                continue;
            }

            buf[_cnt].dip.type = VTSS_IP_TYPE_IPV4;
            buf[_cnt].dip.addr.ipv4 = nc->ipv4[i].ip;
            buf[_cnt].dmac = arp->dmac;
            buf[_cnt].vlan = vlan;
            _cnt ++;
        }

        for (i = 0; i < VTSS_ARP_IPV6_RELATIONS;++i) {
            const vtss_l3_arp_hw_entry_t *arp;
            vtss_vid_t vlan;

            if (_cnt >= VTSS_ARP_CNT) {
                I("Early stop due to buffer limit");
                break;
            }

            if (!nc->ipv6[i].valid)
                continue;

            arp = &nc->hw[nc->ipv6[i].ptr];

            if (!arp->valid) {
                E("CHECK FAILED: %s %d", __FILE__, __LINE__);
                continue;
            }

            // The null entry should not be forwarded
            if (arp->dmac.addr[0] == 0 && arp->dmac.addr[1] == 0 &&
                arp->dmac.addr[2] == 0 && arp->dmac.addr[3] == 0 &&
                arp->dmac.addr[4] == 0 && arp->dmac.addr[5] == 0) {
                continue;
            }

            vlan = vtss_state->l3.rleg_conf[arp->egress_rleg].vlan;

            if (vlan == 0) {
                E("CHECK FAILED: %s %d", __FILE__, __LINE__);
                continue;
            }

            buf[_cnt].dip.type = VTSS_IP_TYPE_IPV6;
            buf[_cnt].dip.addr.ipv6 = nc->ipv6[i].ip;
            buf[_cnt].dmac = arp->dmac;
            buf[_cnt].vlan = vlan;
            _cnt ++;
        }
    }
    *cnt = _cnt;
    VTSS_EXIT();

    return rc;
}

vtss_rc vtss_l3_neighbour_add(const vtss_inst_t         inst,
                              const vtss_l3_neighbour_t *const entry)
{
#define BUF_LENGTH 128
#define DO(X) if (rc == VTSS_RC_OK) { rc = (X); }
    vtss_state_t *vtss_state;
    BOOL rt = FALSE;
    vtss_rc rc = VTSS_RC_OK;
    vtss_l3_rleg_id_t rleg = 0xffffffff;
    char buf[BUF_LENGTH];

    I("neighbour_add: %s",
      vtss_neighbour_to_string(entry, buf, BUF_LENGTH, 0));

    if (entry->vlan == 0) {
        E("Vlan 0 is invalid");
        return VTSS_RC_ERROR;
    }

    VTSS_ENTER();
    DO(vtss_inst_check(inst, &vtss_state));
    if (rc == VTSS_RC_OK) {
        rt = vtss_state->l3.common.routing_enable;
    }
    DO(rleg_id_get(vtss_state->l3.rleg_conf, entry->vlan, &rleg, 0));
    DO(nc_add(vtss_state, rt, entry, rleg,
              &vtss_state->l3.neighbour_cache,
              &vtss_state->l3.lpm));
    VTSS_EXIT();

#undef DO
#undef BUF_LENGTH
    return rc;
}

vtss_rc vtss_l3_neighbour_del(const vtss_inst_t         inst,
                              const vtss_l3_neighbour_t *const entry)
{
#define BUF_LENGTH 128
    vtss_state_t *vtss_state;
    BOOL rt;
    vtss_rc rc;
    char buf[BUF_LENGTH];

    I("neighbour_del: %s",
      vtss_neighbour_to_string(entry, buf, BUF_LENGTH, 0));

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        rt = vtss_state->l3.common.routing_enable;
        rc = nc_del(vtss_state, rt, entry,
                    &vtss_state->l3.neighbour_cache,
                    &vtss_state->l3.lpm);
    }
    VTSS_EXIT();

#undef BUF_LENGTH
    return rc;
}

vtss_rc vtss_l3_debug_sticky_clear(const vtss_inst_t inst);
vtss_rc vtss_l3_debug_sticky_clear(const vtss_inst_t inst)
{
    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_OK;
    I("sticky_clear");

#define DO(X)                    \
    if (rc == VTSS_RC_OK) {      \
        rc = (X);                \
        if (rc != VTSS_RC_OK) {  \
            E("Failed: " #X);    \
        }                        \
    }

    VTSS_ENTER();
    DO(vtss_inst_check(inst, &vtss_state));
    DO(VTSS_FUNC(l3.debug_sticky_clear));
    VTSS_EXIT();

    return rc;
}

#  endif /* VTSS_SW_OPTION_L3RT */

void vtss_debug_print_l3(vtss_state_t *vtss_state,
                         const vtss_debug_printf_t pr,
                         const vtss_debug_info_t *const info);
void vtss_debug_print_l3(vtss_state_t *vtss_state,
                         const vtss_debug_printf_t pr,
                         const vtss_debug_info_t *const info)
{
#if defined(VTSS_SW_OPTION_L3RT)
    u32 i;

    pr("\nCommon:\n");
    pr("=======\n");
    pr("Routing enable: %s\n",
       vtss_state->l3.common.routing_enable ? "TRUE" : "FALSE");
    pr("Rleg_mode:      ");
    switch ( vtss_state->l3.common.rleg_mode ) {
    case VTSS_ROUTING_RLEG_MAC_MODE_INVALID:
        pr("INVALID");
        break;

    case VTSS_ROUTING_RLEG_MAC_MODE_SINGLE:
        pr("SINGLE");
        break;

    default:
        pr("UNKNOWN");
        break;
    }
    pr("(%u)\n", vtss_state->l3.common.rleg_mode);
    pr("Base address:   " MAC_FORMAT "\n",
       MAC_ARGS(vtss_state->l3.common.base_address));

    pr("\nrleg_conf:\n");
    pr("==========\n");
    pr("  id IPv4UC IPv6UC IPv4ICMP IPv6ICMP vlan\n");
    pr("-----------------------------------------\n");
    /*("0000    ENA    ENA      ENA      ENA 0000\n");*/
    for ( i = 0; i < VTSS_RLEG_CNT; ++i ) {
        const vtss_l3_rleg_conf_t *r = &vtss_state->l3.rleg_conf[i];

        if ( (!r->ipv4_unicast_enable) &&
             (!r->ipv6_unicast_enable) &&
             (!r->ipv4_icmp_redirect_enable) &&
             (!r->ipv6_icmp_redirect_enable) )
            continue;

        pr("%4u", i);
        pr("    %s", (r->ipv4_unicast_enable ? "ENA":"DIS"));
        pr("    %s", (r->ipv6_unicast_enable ? "ENA":"DIS"));
        pr("      %s", (r->ipv4_icmp_redirect_enable ? "ENA":"DIS"));
        pr("      %s", (r->ipv6_icmp_redirect_enable ? "ENA":"DIS"));
        pr("%u", r->vlan);
        pr("\n");
    }

    pr("\nlpm:\n");
    pr("====\n");
    pr("  id    match    type data     mask     dst      ecmp "
       "ptr_valid  ptr\n");
    pr("------------------------------------------------------"
       "--------------\n");

    /*("0000 SECURITY IPv4-UC 00000000 00000000 00000000  YES       YES
      0000");*/
    for ( i = 0; i < VTSS_LPM_CNT; ++i ) {
        const vtss_l3_lpm_entry_t *const e = &vtss_state->l3.lpm.e[i];

        if ( ! e->valid )
            continue;

        /* id */
        pr("%4u ", i);

        /* match */
        switch (e->match) {
        case VTSS_L3_LPM_MATCH_BOTH:
            pr("BOTH     ");
            break;

        case VTSS_L3_LPM_MATCH_LPM:
            pr("LPM      ");
            break;

        case VTSS_L3_LPM_MATCH_SECURITY:
            pr("SECURITY ");
            break;

        default:
            pr("UNKNOWN  ");
            break;
        }

        /* type */
        switch (e->type){
        case VTSS_L3_LPM_ENTRY_IPV4_UC:
            pr("IPv4-UC ");
            break;

        case VTSS_L3_LPM_ENTRY_IPV6_UC:
            pr("IPv6-UC ");
            break;

        case VTSS_L3_LPM_ENTRY_IPV4_MC:
            pr("IPv4-MC ");
            break;

        default:
            pr("UNKNOWN ");
            break;
        }

        /* data */
        pr("%08x ", e->data);

        /* mask */
        pr("%08x ", e->mask);

        /* dest */
        pr("%08x ", e->dest);

        /* ecmp */
        pr(" %s ", (e->ecmp ? "YES":" NO"));

        /* ptr_valid */
        pr("      %s ", (e->ptr_valid ? "YES":" NO"));

        /* ptr */
        pr("%4u ", e->ptr);

        pr("\n");
    }

    pr("\nneighbour cache ipv4 relations:\n");
    pr("===============================\n");
    pr("  id ip              hw-ptr\n");
    pr("---------------------------\n");
    /*("0000 000.000.000.000   0000");*/
    for (i = 0; i < VTSS_ARP_IPV4_RELATIONS; ++i) {
        const vtss_l3_arp_ipv4_entry_t *const e =
            &vtss_state->l3.neighbour_cache.ipv4[i];

        if ( ! e->valid )
            continue;

        pr("%4u " IPV4_FORMAT "   %4u\n", i, IPV4_ARGS(e->ip) , e->ptr);
    }

    pr("\nneighbour cache ipv6 relations:\n");
    pr("===============================\n");
    pr("  id ip                                      hw-ptr\n");
    pr("---------------------------------------------------\n");
    /*("0000 0000:0000:0000:0000:0000:0000:0000:0000   0000");*/
    for (i = 0; i < VTSS_ARP_IPV6_RELATIONS; ++i) {
        const vtss_l3_ndp_ipv6_entry_t *const e =
            &vtss_state->l3.neighbour_cache.ipv6[i];

        if ( ! e->valid )
            continue;

        pr("%4u " IPV6_FORMAT "   %4u\n", i, IPV6_ARGS(e->ip) , e->ptr);
    }

    pr("\nneighbour cache layer2 address:\n");
    pr("===============================\n");
    pr("  id erleg secure-vmid secure-mac arp              dmac\n");
    pr("-------------------------------------------------------\n");
    /*("0000  0000         ENA        ENA ENA 00:00:00:00:00:00");*/
    for (i = 0; i < VTSS_ARP_CNT; ++i) {
        const vtss_l3_arp_hw_entry_t *const e =
            &vtss_state->l3.neighbour_cache.hw[i];

        if ( ! e->valid )
            continue;

        /* id */
        pr("%4u ", i);

        /* egress_rleg */
        pr(" %4u ", e->egress_rleg);

        /* secure_vmid */
        pr("        %s ", (e->secure_vmid ? "ENA": "DIS") );

        /* secure_mac */
        pr("       %s ", (e->secure_mac ? "ENA": "DIS") );

        /* arp_enable */
        pr("%s ", (e->arp_enable ? "ENA": "DIS") );

        /* dmac */
        pr(MAC_FORMAT " ", MAC_ARGS(e->dmac));

        pr("\n");
    }
#endif /* VTSS_SW_OPTION_L3RT) */
}

#endif /* VTSS_ARCH_JAGUAR_1 */
