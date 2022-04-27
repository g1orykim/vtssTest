/*

 Vitesse Switch Software.

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
#include "ip2_api.h"
#include "ip2_utils.h"
#include "ip2_trace.h"
#include "ip2_os_api.h"
#include "ip2_chip_api.h"
#include "vtss_simple_fifo.h"

#include <network.h>
#include <net/route.h>
#include <sys/sysctl.h>
#include <sys/socket.h>
#include <net/route.h>
#include <net/if_dl.h>
#include <net/if.h>

#define __GRP VTSS_TRACE_IP2_GRP_OS_ROUTER_SOCKET
#define E(_fmt, ...) T(__GRP, VTSS_TRACE_LVL_ERROR, _fmt, ##__VA_ARGS__)
#define W(_fmt, ...) T(__GRP, VTSS_TRACE_LVL_WARNING, _fmt, ##__VA_ARGS__)
#define I(_fmt, ...) T(__GRP, VTSS_TRACE_LVL_INFO, _fmt, ##__VA_ARGS__)
#define D(_fmt, ...) T(__GRP, VTSS_TRACE_LVL_DEBUG, _fmt, ##__VA_ARGS__)
#define N(_fmt, ...) T(__GRP, VTSS_TRACE_LVL_NOISE, _fmt, ##__VA_ARGS__)
#define R(_fmt, ...) T(__GRP, VTSS_TRACE_LVL_RACKET, _fmt, ##__VA_ARGS__)

#define CHECK(expr, M, ...)                                       \
{                                                                 \
    vtss_rc __rc__ = (expr);                                      \
    if (__rc__ == IP2_ERROR_NOT_MASTER) {                         \
        I("Check not master: " #expr " ARGS: " M, ##__VA_ARGS__); \
    } else if (__rc__ != VTSS_RC_OK) {                            \
        E("Check failed: " #expr " ARGS: " M, ##__VA_ARGS__);     \
    }                                                             \
}

#define VTSS_MAC_IS_ZERO(M)                                      \
    ((M).addr[0] == 0 && (M).addr[1] == 0 && (M).addr[2] == 0 && \
     (M).addr[3] == 0 && (M).addr[4] == 0 && (M).addr[5] == 0)

#define P(...)
//#define P(...) diag_printf(__VA_ARGS__)


FIFO_DECL_STATIC(fifo, vtss_ipstack_msg_t, 4096);
static int IP2_OS_FIFO_overflow = 0;
static BOOL IP2_OS_FIFO_enable = FALSE;

static cyg_handle_t IP2_OS_FIFO_thread_handle;
static cyg_thread   IP2_OS_FIFO_thread_block;
static char         IP2_OS_FIFO_thread_stack[THREAD_DEFAULT_STACK_SIZE];

#define WAKEUP VTSS_BIT(0)
static cyg_flag_t   thread_control_flags;


#if defined(VTSS_SW_OPTION_L3RT ) && defined(VTSS_ARCH_JAGUAR_1)
typedef struct {
    u32 lpm_entries;
    u32 arp_entries;
} lpm_resources;

static lpm_resources lpm_usage;

void vtss_ip2_lpm_usage(u32 *lpm_entries, u32 *arp_entries)
{
    cyg_scheduler_lock();
    *lpm_entries = lpm_usage.lpm_entries;
    *arp_entries = lpm_usage.arp_entries;
    cyg_scheduler_unlock();
}

// Warning, called from scheduler-locked context!
static int vtss_ip2_lpm_rt_alloc_ipv6(void)
{
    int res;

    cyg_scheduler_lock();
    if (lpm_usage.lpm_entries + 4 < VTSS_LPM_CNT) {
        lpm_usage.lpm_entries += 4;
        res = 1;
    } else {
        res = 0;
    }
    cyg_scheduler_unlock();

    return res;
}

// Warning, called from scheduler-locked context!
static int vtss_ip2_lpm_nb_alloc_ipv6(void)
{
    int res;

    cyg_scheduler_lock();
    if (lpm_usage.arp_entries + 4 < VTSS_ARP_CNT) {
        lpm_usage.arp_entries += 4;
        res = 1;
    } else {
        res = 0;
    }
    cyg_scheduler_unlock();

    return res;
}

// Warning, called from scheduler-locked context!
static int vtss_ip2_lpm_rt_alloc_ipv4(void)
{
    int res;

    cyg_scheduler_lock();
    if (lpm_usage.lpm_entries + 1 < VTSS_LPM_CNT) {
        lpm_usage.lpm_entries += 1;
        res = 1;
    } else {
        res = 0;
    }
    cyg_scheduler_unlock();

    return res;
}

// Warning, called from scheduler-locked context!
static int vtss_ip2_lpm_nb_alloc_ipv4(void)
{
    int res;

    cyg_scheduler_lock();
    if (lpm_usage.arp_entries + 1 < VTSS_ARP_CNT) {
        lpm_usage.arp_entries += 1;
        res = 1;
    } else {
        res = 0;
    }
    cyg_scheduler_unlock();

    return res;
}

// Warning, called from scheduler-locked context!
static int vtss_ip2_lpm_rt_free_ipv6(void)
{
    int res;

    cyg_scheduler_lock();
    if (lpm_usage.lpm_entries >= 4) {
        lpm_usage.lpm_entries -= 4;
        res = 1;
    } else {
        res = 0;
    }
    cyg_scheduler_unlock();

    return res;
}

// Warning, called from scheduler-locked context!
static int vtss_ip2_lpm_nb_free_ipv6(void)
{
    int res;

    cyg_scheduler_lock();
    if (lpm_usage.arp_entries >= 4) {
        lpm_usage.arp_entries -= 4;
        res = 1;
    } else {
        res = 0;
    }
    cyg_scheduler_unlock();

    return res;
}

// Warning, called from scheduler-locked context!
static int vtss_ip2_lpm_rt_free_ipv4(void)
{
    int res;

    cyg_scheduler_lock();
    if (lpm_usage.lpm_entries >= 1 ) {
        lpm_usage.lpm_entries -= 1;
        res = 1;
    } else {
        res = 0;
    }
    cyg_scheduler_unlock();

    return res;
}

// Warning, called from scheduler-locked context!
static int vtss_ip2_lpm_nb_free_ipv4(void)
{
    int res;

    cyg_scheduler_lock();
    if (lpm_usage.arp_entries >= 1 ) {
        lpm_usage.arp_entries -= 1;
        res = 1;
    } else {
        res = 0;
    }
    cyg_scheduler_unlock();

    return res;
}

#else
// Warning, called from scheduler-locked context!
static int vtss_ip2_lpm_rt_alloc_ipv6(void)
{
    return 1;
}

// Warning, called from scheduler-locked context!
static int vtss_ip2_lpm_nb_alloc_ipv6(void)
{
    return 1;
}

// Warning, called from scheduler-locked context!
static int vtss_ip2_lpm_rt_alloc_ipv4(void)
{
    return 1;
}

// Warning, called from scheduler-locked context!
static int vtss_ip2_lpm_nb_alloc_ipv4(void)
{
    return 1;
}

// Warning, called from scheduler-locked context!
static int vtss_ip2_lpm_rt_free_ipv6(void)
{
    return 1;
}

// Warning, called from scheduler-locked context!
static int vtss_ip2_lpm_nb_free_ipv6(void)
{
    return 1;
}

// Warning, called from scheduler-locked context!
static int vtss_ip2_lpm_rt_free_ipv4(void)
{
    return 1;
}

// Warning, called from scheduler-locked context!
static int vtss_ip2_lpm_nb_free_ipv4(void)
{
    return 1;
}
#endif /* VTSS_SW_OPTION_L3RT && VTSS_ARCH_JAGUAR_1*/

// Warning, called from scheduler-locked context!
static int vtss_ip2_lpm_rt_alloc(const vtss_routing_entry_t *rt)
{
    if (rt->type == VTSS_ROUTING_ENTRY_TYPE_IPV6_UC) {
        return vtss_ip2_lpm_rt_alloc_ipv6();
    } else if (rt->type == VTSS_ROUTING_ENTRY_TYPE_IPV4_UC) {
        return vtss_ip2_lpm_rt_alloc_ipv4();
    } else {
        return 0;
    }
}

// Warning, called from scheduler-locked context!
static int vtss_ip2_lpm_nb_alloc(const vtss_ip_addr_t *dip)
{
    if (dip->type == VTSS_IP_TYPE_IPV6) {
        return vtss_ip2_lpm_nb_alloc_ipv6();
    } else if (dip->type == VTSS_IP_TYPE_IPV4) {
        return vtss_ip2_lpm_nb_alloc_ipv4();
    } else {
        return 0;
    }
}

// Warning, called from scheduler-locked context!
static int vtss_ip2_lpm_rt_free(const vtss_routing_entry_t *rt)
{
    if (rt->type == VTSS_ROUTING_ENTRY_TYPE_IPV6_UC) {
        return vtss_ip2_lpm_rt_free_ipv6();
    } else if (rt->type == VTSS_ROUTING_ENTRY_TYPE_IPV4_UC) {
        return vtss_ip2_lpm_rt_free_ipv4();
    } else {
        return 0;
    }
}

// Warning, called from scheduler-locked context!
static int vtss_ip2_lpm_nb_free(const vtss_ip_addr_t *dip)
{
    if (dip->type == VTSS_IP_TYPE_IPV6) {
        return vtss_ip2_lpm_nb_free_ipv6();
    } else if (dip->type == VTSS_IP_TYPE_IPV4) {
        return vtss_ip2_lpm_nb_free_ipv4();
    } else {
        return 0;
    }
}


/******************************************************************************
 * monitor_mask2prefix
 *
 * Converts a mask to a prefix (e.g. 255.255.254.0 -> 23)
 * The mask must be in network byte order.
 * Size is the number of 32 bit entries in the mask array (1 for IPv4 and 4 for
 * IPv6)
 *****************************************************************************/
// Warning, called from scheduler-locked context!
static u8 monitor_mask2prefix(u32 *mask, u32 size)
{
    u32 addrword = 0;
    int ix;
    u8  bits = 0;

    while (addrword < size) {
        if (mask[addrword] == 0) {
            break;
        }

        ix = __builtin_ctz((u32)ntohl(mask[addrword++]));

        bits += (32 - ix);
        if (ix != 0) {
            break;
        }
    }
    return bits;
}


// Warning, called from scheduler-locked context!
static vtss_rc build_rt(vtss_routing_entry_t *rt, struct sockaddr *addr,
                        struct sockaddr *mask, struct sockaddr *gate)
{
    if (!addr || (addr->sa_len == 0)) {
        return VTSS_RC_ERROR;
    }

    memset(rt, 0, sizeof(vtss_routing_entry_t));
    if (addr->sa_family == AF_INET) {
        rt->type = VTSS_ROUTING_ENTRY_TYPE_IPV4_UC;
        rt->route.ipv4_uc.network.address =
            ntohl(((struct sockaddr_in *)addr)->sin_addr.s_addr);

        if (mask && mask->sa_len) {
            u32 *m = (u32 *)(&((struct sockaddr_in *)mask)->sin_addr.s_addr);
            rt->route.ipv4_uc.network.prefix_size =
                monitor_mask2prefix(m, 1);
        } else {
            rt->route.ipv4_uc.network.prefix_size = 32;
        }

        if (rt->route.ipv4_uc.network.prefix_size == 32) {
            // host route
            rt->route.ipv4_uc.destination =
                ntohl(((struct sockaddr_in *)addr)->sin_addr.s_addr);
        } else if (gate && gate->sa_len && gate->sa_family == AF_INET) {
            rt->route.ipv4_uc.destination =
                ntohl(((struct sockaddr_in *)gate)->sin_addr.s_addr);
        }

    } else if (addr->sa_family == AF_INET6) {
        rt->type = VTSS_ROUTING_ENTRY_TYPE_IPV6_UC;
        memcpy(&rt->route.ipv6_uc.network.address,
               &((struct sockaddr_in6 *)addr)->sin6_addr, 16);

        if (mask && mask->sa_len) {
            u32 *m = (u32 *)(&((struct sockaddr_in6 *)mask)->sin6_addr);
            rt->route.ipv6_uc.network.prefix_size =
                monitor_mask2prefix(m, 4);// convert to prefix
        } else {
            rt->route.ipv6_uc.network.prefix_size = 128;
        }

        if ( rt->route.ipv6_uc.network.prefix_size == 128 ) {
            memcpy(&rt->route.ipv6_uc.destination,
                   &((struct sockaddr_in6 *)addr)->sin6_addr, 16);
        } else if (gate && gate->sa_len && gate->sa_family == AF_INET6) {
            memcpy(&rt->route.ipv6_uc.destination,
                   &((struct sockaddr_in6 *)gate)->sin6_addr, 16);
        }
    } else {
        P("Unknown type\n");
        return VTSS_RC_ERROR;
    }

    return VTSS_RC_OK;
}

// Warning, called from scheduler-locked context!
static vtss_rc build_nb(vtss_ipstack_msg_nb_t *nb, struct sockaddr *ip,
                        struct sockaddr *mac)
{
    if (!ip || (ip->sa_len == 0)) {
        P("No dip\n");
        return VTSS_RC_ERROR;
    }

    if (!mac || !((struct sockaddr_dl *)mac)->sdl_len) {
        P("No mac address\n");
        return VTSS_RC_ERROR;
    }

    if (((struct sockaddr_dl *)mac)->sdl_family != AF_LINK) {
        P("Unsupported protocoal family\n");
        return VTSS_RC_ERROR;
    }


    if (ip->sa_family == AF_INET) {
        nb->dip.type = VTSS_IP_TYPE_IPV4;
        nb->dip.addr.ipv4 = ntohl(((struct sockaddr_in *)ip)->sin_addr.s_addr);
    } else if (ip->sa_family == AF_INET6) {
        nb->dip.type = VTSS_IP_TYPE_IPV6;
        memcpy(&nb->dip.addr.ipv6, &((struct sockaddr_in6 *)ip)->sin6_addr, 16);
    } else {
        P("Invalid ip family (%d)\n", ip->sa_family);
        return VTSS_RC_ERROR;
    }

    memcpy(&nb->dmac, LLADDR((struct sockaddr_dl *)mac), sizeof(nb->dmac));
    if (VTSS_MAC_IS_ZERO(nb->dmac)) {
        P("Skipping zerro mac\n");
        return VTSS_RC_ERROR;
    }

    return VTSS_RC_OK;
}

// Warning, called from scheduler-locked context!
void vtss_ip2_fifo_interface_signal(int ifidx)
{
    cyg_scheduler_lock();
    if (FIFO_FULL(fifo)) {
        IP2_OS_FIFO_overflow ++;
    } else {
        vtss_ipstack_msg_t e;
        e.type = VTSS_IPSTACK_MSG_IF_SIG;
        e.u.interface.ifidx = ifidx;
        P("PUT-IF_SIG %d\n", ifidx);
        FIFO_PUT(fifo, e);
    }
    cyg_flag_setbits(&thread_control_flags, WAKEUP);
    cyg_scheduler_unlock();
}

// Warning, called from scheduler-locked context!
int vtss_ip2_fifo_route_add(int ifidx,
                            int flags,
                            struct sockaddr *addr,
                            struct sockaddr *mask,
                            struct sockaddr *gate)
{
    int res = -1;

    if (flags & (RTF_LOCAL | RTF_BROADCAST | RTF_MULTICAST)) {
        //P("Route add ignored due to flags: %s %s %s\n",
        //  (flags & RTF_LOCAL) ? "RTF_LOCAL" : "",
        //  (flags & RTF_BROADCAST) ? "RTF_BROADCAST" : "",
        //  (flags & RTF_MULTICAST) ? "RTF_MULTICAST" : "");
        return res;
    }

    cyg_scheduler_lock();
    if (FIFO_FULL(fifo)) {
        IP2_OS_FIFO_overflow ++;

    } else {
        vtss_ipstack_msg_t e;
        e.type = VTSS_IPSTACK_MSG_RT_ADD;
        e.u.rt.ifidx = ifidx;
        if (build_rt(&e.u.rt.route, addr, mask, gate) == VTSS_RC_OK) {
            char buf[128];
            (void) vtss_ip2_route_entry_to_txt(buf, 128, &e.u.rt.route);
            P("PUT-RT_ADD %s if=%d\n", buf, ifidx);

            if (vtss_ip2_lpm_rt_alloc(&e.u.rt.route)) {
                FIFO_PUT(fifo, e);
                res = 0; // success
            }

        } else {
            P("ADD: Failed to build route\n");
        }
    }

    cyg_flag_setbits(&thread_control_flags, WAKEUP);
    cyg_scheduler_unlock();

    return res;
}

// Warning, called from scheduler-locked context!
int vtss_ip2_fifo_route_del(int ifidx,
                            int flags,
                            struct sockaddr *addr,
                            struct sockaddr *mask,
                            struct sockaddr *gate)
{
    int res = -1;

    if (flags & (RTF_LOCAL | RTF_BROADCAST | RTF_MULTICAST)) {
        return res;
    }

    cyg_scheduler_lock();
    if (FIFO_FULL(fifo)) {
        IP2_OS_FIFO_overflow ++;
    } else {
        vtss_ipstack_msg_t e;
        e.type = VTSS_IPSTACK_MSG_RT_DEL;
        e.u.rt.ifidx = ifidx;
        if (build_rt(&e.u.rt.route, addr, mask, gate) == VTSS_RC_OK) {
            char buf[128];
            (void) vtss_ip2_route_entry_to_txt(buf, 128, &e.u.rt.route);
            P("PUT-RT_DEL %s if=%d\n", buf, ifidx);

            if (vtss_ip2_lpm_rt_free(&e.u.rt.route)) {
                FIFO_PUT(fifo, e);
                res = 0; // success
            }
        } else {
            P("DEL: Failed to build route\n");
        }
    }
    cyg_flag_setbits(&thread_control_flags, WAKEUP);
    cyg_scheduler_unlock();

    return res;
}

// Warning, called from scheduler-locked context!
static BOOL check_nb_flags(int f)
{
    if (!(f & RTF_LLINFO)) {
        P("check_nb_flags: No llinfo\n");
        return FALSE;
    }

    if (f & (RTF_BROADCAST | RTF_LOCAL)) {
        P("check_nb_flags: not RTF_BROADCAST | RTF_LOCAL\n");
        return FALSE;
    }

    return TRUE;
}

// Warning, called from scheduler-locked context!
int vtss_ip2_fifo_neighbour_add(int ifidx,
                                int flags,
                                struct sockaddr *ip_addr,
                                struct sockaddr *mac_addr)
{
    int res = -1;

    if (!check_nb_flags(flags)) {
        //P("vtss_ip2_fifo_neighbour_add: failed on flags\n");
        return res;
    }

    cyg_scheduler_lock();
    if (FIFO_FULL(fifo)) {
        IP2_OS_FIFO_overflow ++;

    } else {
        vtss_ipstack_msg_t e;
        e.type = VTSS_IPSTACK_MSG_NB_ADD;
        e.u.nb.ifidx = ifidx;
        if (build_nb(&e.u.nb, ip_addr, mac_addr) == VTSS_RC_OK) {
            char buf[128];
            (void) vtss_ip2_ip_addr_to_txt(buf, 128, &e.u.nb.dip);
            P("PUT-NB_ADD %s "VTSS_MAC_FORMAT" %d\n", buf,
              VTSS_MAC_ARGS(e.u.nb.dmac), ifidx);

            if (vtss_ip2_lpm_nb_alloc(&e.u.nb.dip)) {
                FIFO_PUT(fifo, e);
                res = 0; // success
            }

        } else {
            P("ADD: Failed to build neighbour\n");
        }
    }
    cyg_flag_setbits(&thread_control_flags, WAKEUP);
    cyg_scheduler_unlock();

    return res;
}

// Warning, called from scheduler-locked context!
int vtss_ip2_fifo_neighbour_del(int ifidx,
                                int flags,
                                struct sockaddr *ip_addr,
                                struct sockaddr *mac_addr)
{
    int res = -1;

    if (!check_nb_flags(flags)) {
        return res;
    }

    cyg_scheduler_lock();
    if (FIFO_FULL(fifo)) {
        IP2_OS_FIFO_overflow ++;
    } else {
        vtss_ipstack_msg_t e;
        e.type = VTSS_IPSTACK_MSG_NB_DEL;
        e.u.nb.ifidx = ifidx;
        if (build_nb(&e.u.nb, ip_addr, mac_addr) == VTSS_RC_OK) {
            char buf[128];
            (void) vtss_ip2_ip_addr_to_txt(buf, 128, &e.u.nb.dip);
            P("PUT-NB_DEL %s "VTSS_MAC_FORMAT" %d\n", buf,
              VTSS_MAC_ARGS(e.u.nb.dmac), ifidx);

            if (vtss_ip2_lpm_nb_free(&e.u.nb.dip)) {
                FIFO_PUT(fifo, e);
                res = 0; // success
            }
        } else {
            P("DEL: Failed to build route\n");
        }
    }
    cyg_flag_setbits(&thread_control_flags, WAKEUP);
    cyg_scheduler_unlock();

    return res;
}

static int vtss_ip2_fifo_overflow_read_clear(void)
{
    int res;

    cyg_scheduler_lock();
    res = IP2_OS_FIFO_overflow;
    IP2_OS_FIFO_overflow = 0;
    cyg_scheduler_unlock();

    return res;
}

static bool vtss_ip2_fifo_pop(vtss_ipstack_msg_t *msg)
{
    bool res;

    cyg_scheduler_lock();
    if (FIFO_EMPTY(fifo)) {
        res = FALSE;
    } else {
        res = TRUE;
        *msg = FIFO_HEAD(fifo);
        FIFO_DEL(fifo);
    }
    cyg_scheduler_unlock();

    return res;
}

#if defined(VTSS_SW_OPTION_L3RT)
static void process_msg(const vtss_ipstack_msg_t *msg)
{
    vtss_vid_t vlan;
    vtss_rc rc;
    vtss_neighbour_t nb;
#  define BUF_LENGTH 128
    char buf[BUF_LENGTH];

    switch (msg->type) {
    case VTSS_IPSTACK_MSG_RT_ADD:
    case VTSS_IPSTACK_MSG_RT_DEL:
        rc = vtss_ip2_os_if_index_to_vlan_if(msg->u.rt.ifidx, &vlan);

        if (rc != VTSS_RC_OK) {
            D("Failed to convert ifidx(%d) to vlan", msg->u.rt.ifidx);
            break;
        }

        D("IF_SIG: ifidx=%d, vlan=%u", msg->u.nb.ifidx, vlan);
        vtss_ip2_if_signal(vlan);

        (void) vtss_ip2_route_entry_to_txt(buf, BUF_LENGTH, &(msg->u.rt.route));

        // Filter out unwanted routes
        if (msg->u.rt.route.type == VTSS_ROUTING_ENTRY_TYPE_IPV4_UC) {
            N("Accepting ipv4 route");
        } else if (msg->u.rt.route.type == VTSS_ROUTING_ENTRY_TYPE_IPV6_UC) {
            const vtss_ipv6_uc_t *r = &msg->u.rt.route.route.ipv6_uc;

            if (vtss_ipv6_addr_is_multicast(&(r->network.address))) {
                D("IGNORE: multicast net: ifidx=%d, vlan=%u, route=%s",
                  msg->u.rt.ifidx, vlan, buf);
                break;
            } else if (vtss_ipv6_addr_is_multicast(&(r->destination))) {
                D("IGNORE: multicast dest: ifidx=%d, vlan=%u, route=%s",
                  msg->u.rt.ifidx, vlan, buf);
                break;
            } else if (vtss_ipv6_addr_is_link_local(&(r->network.address))) {
                D("IGNORE: link-local net: ifidx=%d, vlan=%u, route=%s",
                  msg->u.rt.ifidx, vlan, buf);
                break;
            } else if (vtss_ipv6_addr_is_link_local(&(r->destination))) {
                D("IGNORE: link-local dest: ifidx=%d, vlan=%u, route=%s",
                  msg->u.rt.ifidx, vlan, buf);
                break;
            } else {
                N("Accepting ipv6 route");
            }

        } else {
            E("Unknown type: %d", msg->u.rt.route.type);
            break;
        }

        if (msg->type == VTSS_IPSTACK_MSG_RT_ADD) {
            D("RT_ADD: ifidx=%d, vlan=%u, route=%s", msg->u.rt.ifidx, vlan, buf);
            CHECK(vtss_ip2_chip_route_add(&msg->u.rt.route), "rt=%s", buf);
        } else {
            D("RT_DEL: ifidx=%d, vlan=%u, route=%s", msg->u.rt.ifidx, vlan, buf);
            CHECK(vtss_ip2_chip_route_del(&msg->u.rt.route), "rt=%s", buf);
        }
        break;

    case VTSS_IPSTACK_MSG_NB_ADD:
    case VTSS_IPSTACK_MSG_NB_DEL:
        rc = vtss_ip2_os_if_index_to_vlan_if(msg->u.nb.ifidx, &vlan);

        if (rc != VTSS_RC_OK) {
            D("Failed to convert ifidx(%d) to vlan", msg->u.nb.ifidx);
            break;
        }

        (void) vtss_ip2_ip_addr_to_txt(buf, BUF_LENGTH, &(msg->u.nb.dip));

        // Filter out unwanted neighbours
        if (msg->u.nb.dip.type == VTSS_IP_TYPE_IPV4) {
            N("Accepting ipv4 nb");
        } else if (msg->u.nb.dip.type == VTSS_IP_TYPE_IPV6) {
            const vtss_ipv6_t *i = &msg->u.nb.dip.addr.ipv6;

            if (vtss_ipv6_addr_is_multicast(i)) {
                D("IGNORE: multicast NB: ifidx=%d, vlan=%u, mac="VTSS_MAC_FORMAT" ip=%s",
                  msg->u.rt.ifidx, vlan, VTSS_MAC_ARGS(msg->u.nb.dmac), buf);
                break;
            } else if (vtss_ipv6_addr_is_link_local(i)) {
                D("IGNORE: link-local NB: ifidx=%d, vlan=%u, mac="VTSS_MAC_FORMAT" ip=%s",
                  msg->u.rt.ifidx, vlan, VTSS_MAC_ARGS(msg->u.nb.dmac), buf);
                break;
            } else {
                N("Accepting ipv6 nb");
            }

        } else {
            E("Unknown type: %d", msg->u.nb.dip.type);
            break;
        }

        nb.vlan = vlan;
        nb.dip = msg->u.nb.dip;
        nb.dmac = msg->u.nb.dmac;

        if (msg->type == VTSS_IPSTACK_MSG_NB_ADD) {
            D("NB_ADD: ifidx=%d, vlan=%u, mac="VTSS_MAC_FORMAT" ip=%s",
              msg->u.nb.ifidx, vlan, VTSS_MAC_ARGS(msg->u.nb.dmac), buf);
            CHECK(vtss_ip2_chip_neighbour_add(&nb),
                  "{dmac="VTSS_MAC_FORMAT" ip=%s, vlan=%u}",
                  VTSS_MAC_ARGS(msg->u.nb.dmac), buf, vlan);
        } else {
            D("NB_DEL: ifidx=%d, vlan=%u, mac="VTSS_MAC_FORMAT" ip=%s",
              msg->u.nb.ifidx, vlan, VTSS_MAC_ARGS(msg->u.nb.dmac), buf);
            (void)vtss_ip2_chip_neighbour_del(&nb);
        }
        break;

    case VTSS_IPSTACK_MSG_IF_SIG:
        rc = vtss_ip2_os_if_index_to_vlan_if(msg->u.nb.ifidx, &vlan);
        if (rc != VTSS_RC_OK) {
            D("Failed to convert ifidx(%d) to vlan", msg->u.nb.ifidx);
            break;
        }

        D("IF_SIG: ifidx=%d, vlan=%u", msg->u.nb.ifidx, vlan);
        vtss_ip2_if_signal(vlan);
        break;

    default:
        D("Unknown message type %d", msg->type);
        break;
    }
#undef BUF_LENGTH
}
#else /* VTSS_SW_OPTION_L3RT */
static void process_msg(const vtss_ipstack_msg_t *msg)
{
    vtss_vid_t vlan;
    vtss_rc rc;

    switch (msg->type) {
    case VTSS_IPSTACK_MSG_RT_ADD:
    case VTSS_IPSTACK_MSG_RT_DEL:
        rc = vtss_ip2_os_if_index_to_vlan_if(msg->u.rt.ifidx, &vlan);

        if (rc != VTSS_RC_OK) {
            D("Failed to convert ifidx(%d) to vlan", msg->u.rt.ifidx);
            break;
        }

        D("IF_SIG: ifidx=%d, vlan=%u", msg->u.rt.ifidx, vlan);
        vtss_ip2_if_signal(vlan);
        break;


    case VTSS_IPSTACK_MSG_IF_SIG:
        rc = vtss_ip2_os_if_index_to_vlan_if(msg->u.interface.ifidx, &vlan);
        if (rc != VTSS_RC_OK) {
            D("Failed to convert ifidx(%d) to vlan", msg->u.interface.ifidx);
            break;
        }

        D("IF_SIG: ifidx=%d, vlan=%u", msg->u.interface.ifidx, vlan);
        vtss_ip2_if_signal(vlan);
        break;

    default:
        break;
    }
}
#endif /* VTSS_SW_OPTION_L3RT */


static void IP2_monitor_thread(cyg_addrword_t data)
{
    while (cyg_flag_wait(&thread_control_flags, WAKEUP,
                         CYG_FLAG_WAITMODE_OR | CYG_FLAG_WAITMODE_CLR)) {
        int overflow = 0;
        vtss_ipstack_msg_t msg;

        overflow = vtss_ip2_fifo_overflow_read_clear();
        if (overflow) {
            // TODO, we may need to take spacial action to recover from this
            E("IP2 os fifo has overflowed with %d entries", overflow);
        }

        while (vtss_ip2_fifo_pop(&msg)) {
            if (IP2_OS_FIFO_enable == TRUE) {
                process_msg(&msg);
            }
        }
    }
}

void vtss_ip2_routing_monitor_enable(void)
{
    cyg_scheduler_lock();
    IP2_OS_FIFO_enable = TRUE;
    cyg_scheduler_unlock();
}

void vtss_ip2_routing_monitor_disable(void)
{
    cyg_scheduler_lock();
    IP2_OS_FIFO_enable = FALSE;
    cyg_scheduler_unlock();
}

void vtss_ip2_fifo_init(void)
{
    cyg_flag_init(&thread_control_flags);
    cyg_thread_create(THREAD_DEFAULT_PRIO,
                      IP2_monitor_thread,
                      0,
                      "IP2.monitor",
                      IP2_OS_FIFO_thread_stack,
                      sizeof(IP2_OS_FIFO_thread_stack),
                      &IP2_OS_FIFO_thread_handle,
                      &IP2_OS_FIFO_thread_block);
    cyg_thread_resume(IP2_OS_FIFO_thread_handle);
}


