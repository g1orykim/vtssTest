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

#include "main.h"
#include "conf_api.h"
#include "ip2_priv.h"
#include "ip2_utils.h"
#include "ip2_trace.h"
#include "ip2_os_api.h"
#include "ip2_ecos_driver.h"
#include "ip2_chip_api.h"
#include "packet_api.h"
#include "critd_api.h"
#include "misc_api.h"

#include <stdlib.h>
#include <network.h>
#include <sys/sysctl.h>
#include <sys/param.h>
#include <net/if_var.h>
#include <net/if.h>
#include <netinet6/in6_var.h>
#include <netinet/icmp6.h>
#include <netinet6/nd6.h>


static critd_t crit;    // general purpose mutex
static critd_t crit_cb; // portect callback function. can not use "crit" as it may dead lock with the packet module

#if VTSS_TRACE_ENABLED
#  define IP2_CRIT_ENTER()                 \
    critd_enter(&crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#  define IP2_CRIT_EXIT()                  \
    critd_exit(&crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#  define IP2_CRIT_ASSERT_LOCKED()         \
    critd_assert_locked(&crit, TRACE_GRP_CRIT, __FILE__, __LINE__)

#  define IP2_CRIT_CB_ENTER()                 \
    critd_enter(&crit_cb, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#  define IP2_CRIT_CB_EXIT()                  \
    critd_exit(&crit_cb, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#  define IP2_CRIT_CB_ASSERT_LOCKED()         \
    critd_assert_locked(&crit_cb, TRACE_GRP_CRIT, __FILE__, __LINE__)

#else
// Leave out function and line arguments
#  define IP2_CRIT_ENTER() critd_enter(&crit)
#  define IP2_CRIT_EXIT()  critd_exit(&crit)
#  define IP2_CRIT_ASSERT_LOCKED() critd_assert_locked(&crit)

#  define IP2_CRIT_CB_ENTER() critd_enter(&crit_cb)
#  define IP2_CRIT_CB_EXIT()  critd_exit(&crit_cb)
#  define IP2_CRIT_CB_ASSERT_LOCKED() critd_assert_locked(&crit_cb)
#endif

#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_IP2
#define __GRP VTSS_TRACE_IP2_GRP_OS
#define E(_fmt, ...) T(__GRP, VTSS_TRACE_LVL_ERROR, _fmt, ##__VA_ARGS__)
#define W(_fmt, ...) T(__GRP, VTSS_TRACE_LVL_WARNING, _fmt, ##__VA_ARGS__)
#define I(_fmt, ...) T(__GRP, VTSS_TRACE_LVL_INFO, _fmt, ##__VA_ARGS__)
#define D(_fmt, ...) T(__GRP, VTSS_TRACE_LVL_DEBUG, _fmt, ##__VA_ARGS__)
#define N(_fmt, ...) T(__GRP, VTSS_TRACE_LVL_NOISE, _fmt, ##__VA_ARGS__)
#define R(_fmt, ...) T(__GRP, VTSS_TRACE_LVL_RACKET, _fmt, ##__VA_ARGS__)

/**************************************************************************
 * Private data
 */

typedef struct os_data {
    void      *rx_filter_id; // Rx Packet filter
} os_data_t;

static os_data_t  IP2_OS_state[VTSS_VIDS];

static vtss_rc IP2_OS_if_index_to_vlan_if(u32 _if_index, vtss_if_id_vlan_t *vlan)
{
    vtss_vid_t tmp;

    IP2_CRIT_ASSERT_LOCKED();

    tmp = vtss_ip2_ecos_driver_if_index_to_vid(_if_index);
    if (tmp == 0) {
        return VTSS_RC_ERROR;
    }

    *vlan = tmp;
    return VTSS_RC_OK;
}

vtss_rc vtss_ip2_os_if_index_to_vlan_if(u32 _if_index, vtss_if_id_vlan_t *vlan)
{
    vtss_rc rc;
    IP2_CRIT_ENTER();
    D("%s", __FUNCTION__);
    rc = IP2_OS_if_index_to_vlan_if(_if_index, vlan);
    IP2_CRIT_EXIT();

    return rc;
}

static int ip2_socket_lo0(int family, struct ifreq *ifr)
{
    int s;

    /* Setup network interface */
    if ((s = socket(family, SOCK_DGRAM, 0)) < 0) {
        E("failed to open socket");
    } else {
        memset(ifr, 0, sizeof(*ifr));
        strcpy(ifr->ifr_name, "lo0");
    }
    return s;
}

static int IP2_OS_socket(vtss_vid_t vid, struct ifreq *ifr)
{
    int s;

    IP2_CRIT_ASSERT_LOCKED();

    if ((s = socket(AF_INET, SOCK_DGRAM, 0)) >= 0) {
        memset(ifr, 0, sizeof(*ifr));
        (void) snprintf(ifr->ifr_name, 16, "eth%u", vid);
    } else {
        E("failed to open socket: %s", strerror(errno));
    }
    return s;
}

static int IP2_OS_socket_in6(vtss_vid_t vid, struct in6_aliasreq *ifr6)
{
    int s;

    IP2_CRIT_ASSERT_LOCKED();

    if ((s = socket(AF_INET6, SOCK_DGRAM, 0)) >= 0) {
        memset(ifr6, 0x0, sizeof(*ifr6));
        (void) snprintf(ifr6->ifra_name, 16, "eth%u", vid);
    } else {
        E("failed to open socket: %s", strerror(errno));
    }
    return s;
}

static vtss_rc IP2_OS_iface_setaddr_ipv4(vtss_vid_t vid,
                                         unsigned long addr,
                                         unsigned long mask)
{
    int                 s;
    struct ifreq        ifr;
    vtss_rc rc = IP2_ERROR_FAILED; /* Assume the worst */

    IP2_CRIT_ASSERT_LOCKED();

    if (addr == 0) {
        return VTSS_RC_ERROR;
    }

    if ((s = IP2_OS_socket(vid, &ifr)) < 0) {
        return rc;
    }

    addr = htonl(addr);
    mask = htonl(mask);

    /* Delete old IP address, if any */
    /* for IPv4 */
    if (ioctl(s, SIOCGIFADDR, &ifr) == 0 &&
        ioctl(s, SIOCDIFADDR, &ifr) < 0) {
        E("SIOCDIFADDR failed");
    }

    if (ioctl(s, SIOCGIFADDR, &ifr) && errno == EADDRNOTAVAIL) {
        /* Initialize socket address */
        struct sockaddr_in *saddr = (struct sockaddr_in *)&ifr.ifr_addr;
        saddr->sin_family = AF_INET;
        saddr->sin_len = sizeof(*saddr);
        saddr->sin_port = 0;

        /* Set new IP address */
        saddr->sin_addr.s_addr = addr;
        if (ioctl(s, SIOCSIFADDR, &ifr)) {
            E("SIOCSIFADDR failed");
        }

        /* Set IP mask */
        saddr->sin_addr.s_addr = mask;
        if (ioctl(s, SIOCSIFNETMASK, &ifr)) {
            E("SIOCSIFNETMASK failed");
        }

        /* Set IP address again */
        saddr->sin_addr.s_addr = addr;
        if (ioctl(s, SIOCSIFADDR, &ifr)) {
            E("SIOCSIFADDR failed 2");
        }

        /* Set broadcast address */
        saddr->sin_addr.s_addr = (addr | ~mask);
        if (ioctl(s, SIOCSIFBRDADDR, &ifr)) {
            E("SIOCSIFBRDADDR failed");
        } else {
            rc = VTSS_OK;
        }
    } else {
        struct ifaliasreq ifra;
        (void) snprintf(ifra.ifra_name, 16, "eth%d", vid);
        ifra.ifra_addr.sa_family = AF_INET;
        ((struct sockaddr_in *) &ifra.ifra_addr)->sin_addr.s_addr = addr;
        ifra.ifra_broadaddr.sa_family = AF_INET;
        ((struct sockaddr_in *) &ifra.ifra_broadaddr)->sin_addr.s_addr = (addr | ~mask);
        ifra.ifra_mask.sa_family = AF_INET;
        ((struct sockaddr_in *) &ifra.ifra_mask)->sin_addr.s_addr = mask;
        if (ioctl(s, SIOCAIFADDR, (caddr_t) &ifra) < 0) {
            if (errno != EEXIST) {
                E("interface address add: %d", errno);
            } else {
                E("interface address add: Address exists");
            }
        } else {
            I("Address added.");
            rc = VTSS_OK;
        }
    }

    close(s);
    return rc;
}

// can not use IP2_OS_iface_setaddr_ipv4 because broadcast address
// must _NOT_ be set
static vtss_rc ip2_lo0_setaddr_ipv4(void)
{
    int                 s;
    struct ifreq        ifr;
    struct sockaddr_in *saddr;

    unsigned long addr = htonl(0x7F000001);
    unsigned long mask = htonl(0xFF000000);

    vtss_rc rc = VTSS_RC_OK;

    if ((s = ip2_socket_lo0(AF_INET, &ifr)) < 0) {
        return rc;
    }

    saddr = (struct sockaddr_in *)&ifr.ifr_addr;
    saddr->sin_family = AF_INET;
    saddr->sin_len = sizeof(*saddr);
    saddr->sin_port = 0;

    /* Set new IP address */
    saddr->sin_addr.s_addr = addr;
    if (ioctl(s, SIOCSIFADDR, &ifr)) {
        E("SIOCSIFADDR failed");
        rc = VTSS_RC_ERROR;
    }

    /* Set IP mask */
    saddr->sin_addr.s_addr = mask;
    if (ioctl(s, SIOCSIFNETMASK, &ifr)) {
        E("SIOCSIFNETMASK failed");
        rc = VTSS_RC_ERROR;
    }

    close(s);
    return rc;
}

static vtss_rc IP2_OS_iface_setaddr_ipv6(vtss_vid_t vid,
                                         const unsigned char *ipv6_addr,
                                         int ipv6_prefix)
{
    int                 s, i, j;
    struct in6_aliasreq ifra;
    struct sockaddr_in6 *addr6;
    unsigned long       prefix;
    vtss_rc rc = VTSS_RC_OK;

    IP2_CRIT_ASSERT_LOCKED();

    if ((s = IP2_OS_socket_in6(vid, &ifra)) >= 0) {
        /* Add static address */
        addr6 = (struct sockaddr_in6 *) &ifra.ifra_addr;
        addr6->sin6_family = AF_INET6;
        addr6->sin6_len = sizeof(*addr6);
        addr6->sin6_port = 0;
        for (i = 0; i < 16; i++) {
            addr6->sin6_addr.s6_addr[i] = ipv6_addr[i];
        }

        /* Set IPv6 prefix length */
        addr6 = (struct sockaddr_in6 *) &ifra.ifra_prefixmask;
        addr6->sin6_family = AF_INET6;
        addr6->sin6_len = sizeof(*addr6);
        addr6->sin6_port = 0;
        for (prefix = 0, i = 0; i < 128; i++) {
            j = (31 - (i % 32));
            if (i < ipv6_prefix) {
                prefix |= (1 << j);
            }
            if (j == 0) {
                addr6->sin6_addr.__u6_addr.__u6_addr32[i / 32] = htonl(prefix);
                prefix = 0;
            }
        }

        /* Set IPv6 lifetime, the address should nerver expire */
        ifra.ifra_lifetime.ia6t_vltime = ND6_INFINITE_LIFETIME;
        ifra.ifra_lifetime.ia6t_pltime = ND6_INFINITE_LIFETIME;
        ifra.ifra_flags = (IN6_IFF_TENTATIVE | IN6_IFF_DUPLICATED);
        if (ioctl(s, SIOCAIFADDR_IN6, &ifra)) {
            E("failed to set interface address: %s", strerror(errno));
            rc = VTSS_RC_ERROR;
        } else {
            I("Address added.");
        }

        close(s);
    } else {
        E("socket: %s", strerror(errno));
        rc = VTSS_RC_ERROR;
    }
    return rc;
}

static vtss_rc ipv4_route_op(const vtss_ipv4_uc_t *rt, unsigned long op)
{
    int                 s;
    struct ifreq        ifr;
    struct sockaddr_in  *addr;
    struct ecos_rtentry route;
    vtss_rc             rc;
    vtss_ipv4_t         mask;

    if ((rc = vtss_conv_prefix_to_ipv4mask(rt->network.prefix_size, &mask)) != VTSS_OK) {
        return rc;
    }

    if ((s = ip2_socket_lo0(AF_INET, &ifr)) < 0) {
        return VTSS_RC_ERROR;
    }

    addr = (struct sockaddr_in *)&ifr.ifr_addr;
    memset(addr, 0, sizeof(*addr));
    addr->sin_family = AF_INET;
    addr->sin_len = sizeof(*addr);
    addr->sin_port = 0;

    memset(&route, 0, sizeof(route));
    addr->sin_addr.s_addr = htonl(rt->network.address);
    memcpy(&route.rt_dst, addr, sizeof(*addr));
    addr->sin_addr.s_addr = htonl(mask);
    memcpy(&route.rt_genmask, addr, sizeof(*addr));
    addr->sin_addr.s_addr = htonl(rt->destination);
    memcpy(&route.rt_gateway, addr, sizeof(*addr));
    route.rt_dev = "lo0";      /* Dummy? */
    route.rt_flags = RTF_UP | RTF_GATEWAY;
    route.rt_metric = 0;
    if (ioctl(s, op, &route)) {
        I("%s failed - %s", op == SIOCADDRT ? "SIOCADDRT" : "SIOCDELRT", strerror(errno));
        rc = IP2_ERROR_FAILED;
    }

    close(s);

    return rc;
}

static vtss_rc ipv6_route_op(const vtss_ipv6_uc_t *rt, unsigned long op)
{
    int                 s;
    struct ifreq        ifr;
    struct sockaddr_in6 *addr;
    struct ecos_rtentry route;
    vtss_rc             rc;
    vtss_ipv6_t         mask;

    if ((rc = vtss_conv_prefix_to_ipv6mask(rt->network.prefix_size, &mask)) != VTSS_OK) {
        return rc;
    }

    if ((s = ip2_socket_lo0(AF_INET6, &ifr)) < 0) {
        return VTSS_RC_ERROR;
    }

    addr = (struct sockaddr_in6 *)&ifr.ifr_addr;
    memset(addr, 0, sizeof(*addr));
    addr->sin6_family = AF_INET6;
    addr->sin6_len = sizeof(*addr);
    addr->sin6_port = 0;

    memset(&route, 0, sizeof(route));

    /* Network */
    memcpy(addr->sin6_addr.s6_addr, rt->network.address.addr, sizeof(rt->network.address.addr));
    memcpy(&route.rt_dst, addr, sizeof(*addr));

    /* Mask */
    memcpy(addr->sin6_addr.s6_addr, mask.addr, sizeof(mask.addr));
    memcpy(&route.rt_genmask, addr, sizeof(*addr));

    /* Gateway */
    memcpy(addr->sin6_addr.s6_addr, rt->destination.addr, sizeof(rt->destination.addr));
    memcpy(&route.rt_gateway, addr, sizeof(*addr));

    route.rt_flags = RTF_UP | RTF_GATEWAY;
    route.rt_metric = 0;
    if (ioctl(s, op, &route)) {
        I("%s failed - %s", op == SIOCADDRT ? "SIOCADDRT" : "SIOCDELRT", strerror(errno));
        rc = IP2_ERROR_FAILED;
    }

    close(s);

    return rc;
}

static vtss_ipstack_filter_cb_t IP2_OS_vtss_ipstack_filter_cb = 0;

// TODO, wron name
vtss_rc vtss_ip2_if_filter_reg(vtss_ipstack_filter_cb_t cb)
{
    vtss_rc rc = VTSS_RC_ERROR;

    IP2_CRIT_CB_ENTER();
    D("%s", __FUNCTION__);
    if (IP2_OS_vtss_ipstack_filter_cb == 0) {
        IP2_OS_vtss_ipstack_filter_cb = cb;
        rc = VTSS_RC_OK;
    }
    IP2_CRIT_CB_EXIT();

    return rc;
}

// TODO, wrong name
vtss_rc vtss_ip2_if_filter_unreg(vtss_ipstack_filter_cb_t cb)
{
    vtss_rc rc = VTSS_RC_ERROR;

    IP2_CRIT_CB_ENTER();
    D("%s", __FUNCTION__);
    if (IP2_OS_vtss_ipstack_filter_cb == cb) {
        IP2_OS_vtss_ipstack_filter_cb = 0;
        rc = VTSS_RC_OK;
    }
    IP2_CRIT_CB_EXIT();

    return rc;
}

// VLAN/MSTP/ERPS/.. ingress filtering
static BOOL port_filter_rx_discard(u32 src_port, vtss_vid_t vid)
{
    vtss_packet_filter_t     filter;
    vtss_packet_frame_info_t info;

    if (src_port == VTSS_PORT_NO_NONE) {
        return FALSE;
    }

    vtss_packet_frame_info_init(&info);
    info.port_no = src_port;
    info.vid = vid;

    if (vtss_packet_frame_filter(NULL, &info, &filter) == VTSS_RC_OK &&
        filter == VTSS_PACKET_FILTER_DISCARD) {
        I("frame discarded by ingress filtering, port: %u, vid: %u",
          info.port_no, info.vid);
        return TRUE;
    }

    return FALSE;
}

/****************************************************************************/
// ip2_rx_packet()
// This function is called back when a packet matching our subscription
// parameters is received from the FDMA.
// The len parameter doesn't include IFH, CMD, or FCS.
// It is the only entry point before the incoming packets hit the TCP/IP stack.
/****************************************************************************/
static BOOL vtss_ip2_os_rx_packet(void *contxt, const u8 *const frm,
                                  const vtss_packet_rx_info_t *const rx_info)
{
    // WARNING: this function may not be lock using the same mutex as is hold
    // when the filter is registered, changed or unregistered

    vtss_vid_t vid;
    BOOL res = TRUE, filtered_out = FALSE;

    vid = rx_info->tag.vid;

    if (port_filter_rx_discard(rx_info->port_no, vid)) {
        T_I_HEX(frm, MIN(60, rx_info->length));
        return FALSE;
    }

    IP2_CRIT_CB_ENTER();
    if (IP2_OS_vtss_ipstack_filter_cb &&
        IP2_OS_vtss_ipstack_filter_cb(vid, rx_info->length, (char *) frm)) {
        N("Frame filtered out: vlan %d, length %d", vid, rx_info->length);
        filtered_out = TRUE;
    }
    IP2_CRIT_CB_EXIT();

    if (!filtered_out) {
        N("Frame inject vlan %d, length %d", vid, rx_info->length);
        (void) vtss_ip2_ecos_driver_inject(vid, rx_info->length, frm);
        res = TRUE;
    }

    return res; // Allow the frame to be dispatched to other subscribers.
}

static void ip2_rx_filter_register(vtss_vid_t vid, void **filter_id)
{
    // Register for IPv4, IPv6 (possibly), and ARP packet Rx
    packet_rx_filter_t filter;
    memset(&filter, 0, sizeof(filter));
    filter.modid  = VTSS_MODULE_ID_IP_STACK_GLUE;
    filter.match  = PACKET_RX_FILTER_MATCH_IP_ANY | PACKET_RX_FILTER_MATCH_VID;
    filter.vid    = vid;
    filter.cb     = vtss_ip2_os_rx_packet;
    filter.prio   = PACKET_RX_FILTER_PRIO_LOW;

    if (*filter_id) {
        (void) packet_rx_filter_change(&filter, filter_id);
    } else {
        (void) packet_rx_filter_register(&filter, filter_id);
    }
}

/**************************************************************************
 * Exported Interface
 */

int sysctl(int *name, u_int namelen, void *old, size_t *oldlenp,
           void *new, size_t newlen);
vtss_rc vtss_ip2_os_global_param_set(const vtss_ip2_global_param_t *const param)
{
    vtss_rc rc;
    u32 forwarding;
    int mib4[] = { CTL_NET, PF_INET,  IPPROTO_IP,   IPCTL_FORWARDING };   /* "net.inet.ip.forwarding" */
    int mib6[] = { CTL_NET, PF_INET6, IPPROTO_IPV6, IPV6CTL_FORWARDING }; /* "net.inet.ipv6.forwarding" */

    rc = VTSS_OK;
#if defined(VTSS_SW_OPTION_L3RT)
    forwarding = param->enable_routing ? 1 : 0;

    if (vtss_ip2_chip_routing_enable(forwarding) != VTSS_RC_OK) {
        E("Failed to %s routing in chip layer",
          forwarding ? "enable" : "disable");
    }
#else
    forwarding = 0;
#endif /* VTSS_SW_OPTION_L3RT */

    D("%s", __FUNCTION__);
    if (sysctl(mib4, 4, NULL, 0, (void *)&forwarding, sizeof(forwarding)) == -1 ||
        sysctl(mib6, 4, NULL, 0, (void *)&forwarding, sizeof(forwarding)) == -1) {
        E("Unable to control IP routing");
        rc = VTSS_UNSPECIFIED_ERROR;
    } else {
        /* IPv6 router should not accept RA */
        u32 accept_ra = forwarding == 0 ? 1 : 0;
        int mib_rtadv[] = { CTL_NET, PF_INET6, IPPROTO_IPV6, IPV6CTL_ACCEPT_RTADV };

        if (sysctl(mib_rtadv, 4, NULL, 0, (void *)&accept_ra, sizeof(accept_ra)) == -1) {
            E("Unable to control IPv6 ACCEPT_RTADV");
            rc = VTSS_UNSPECIFIED_ERROR;
        }
    }

    if (rc == VTSS_OK) {
        int mib_icmp6[] = { CTL_NET, PF_INET6, IPPROTO_ICMPV6, ICMPV6CTL_ND6_DEBUG }; /* net.inet6.icmp6.nd6_debug */
        u32 icmp6_dbg;

        icmp6_dbg = 0;
        if ( param->dbg_flag & VTSS_IP_DBG_FLAG_ND6_LOG ) {
            icmp6_dbg = 1;
        }

        if (sysctl(mib_icmp6, 4, NULL, 0, (void *)&icmp6_dbg, sizeof(icmp6_dbg)) == -1) {
            E("Unable to control IPv6 ICMPV6CTL_ND6_DEBUG");
            rc = VTSS_UNSPECIFIED_ERROR;
        }
    }

    return rc;
}

static vtss_rc IP2_OS_if_flags_set(vtss_vid_t vid, unsigned val, unsigned mask)
{
    int s;
    struct ifreq ifr;

    IP2_CRIT_ASSERT_LOCKED();

    if ((s = IP2_OS_socket(vid, &ifr)) < 0) {
        return VTSS_RC_ERROR;
    }

    if (ioctl(s, SIOCGIFFLAGS, &ifr) < 0) {
        perror("SIOCSIFFLAGS");
        close(s);
        return VTSS_RC_ERROR;
    }

    ifr.ifr_flags = (ifr.ifr_flags & (~mask)) | val;
    if (ioctl(s, SIOCSIFFLAGS, &ifr) < 0) {
        perror("SIOCSIFFLAGS");
        close(s);
        return VTSS_RC_ERROR;
    }

    close(s);
    D("eth%d - set ifflags 0x%0x", vid, ifr.ifr_flags);
    return VTSS_OK;
}

vtss_rc vtss_ip2_os_if_add(vtss_vid_t vid)
{
    int if_no;
    vtss_rc rc;
    vtss_mac_t mac;

    IP2_CRIT_ENTER();

    rc = vtss_ip2_chip_rleg_add(vid);
    if (rc != VTSS_RC_OK) {
        D("ERROR %u %s:%d", vid, __FILE__, __LINE__);
        goto ERROR0;
    }

    (void)conf_mgmt_mac_addr_get(mac.addr, 0);

    if_no = vtss_ip2_ecos_driver_if_add(vid, &mac);

    if (if_no < 0) {
        D("ERROR %u %s:%d", vid, __FILE__, __LINE__);
        rc = VTSS_RC_ERROR;
        goto ERROR1;
    }

    D("New if: vlan=%u ifno=%d", vid, if_no);
    ip2_rx_filter_register(vid, &IP2_OS_state[vid].rx_filter_id);
    rc = VTSS_RC_OK;
    goto ERROR0;

ERROR1:
    if (vtss_ip2_chip_rleg_del(vid) != VTSS_RC_OK) {
        E("Cleanup of vlan %u failed", vid);
    }

ERROR0:
    IP2_CRIT_EXIT();
    return rc;
}

vtss_rc vtss_ip2_os_if_set(vtss_vid_t             vid,
                           const vtss_if_param_t *const if_params)
{
    // TODO
    return VTSS_RC_OK;
}

vtss_rc vtss_ip2_os_if_ctl(vtss_vid_t vid, BOOL up)
{
    struct ifreq ifr;
    vtss_rc rc = IP2_ERROR_FAILED;

    IP2_CRIT_ENTER();
    D("%s", __FUNCTION__);
    if (up) {
        int s;
        if ((s = IP2_OS_socket(vid, &ifr)) < 0) {
            D("ERROR %d", __LINE__);
            goto ERROR;
        }

        /* Re-set address to force gratuitous arp - also UP's interface */
        if (ioctl(s, SIOCGIFADDR, &ifr) == 0) {
            if (ioctl(s, SIOCSIFADDR, &ifr) == 0) {
                rc = VTSS_RC_OK;
            }
        } else {
            I("eth%d: interface up, but has no ipv4 address", vid);
            rc = IP2_OS_if_flags_set(vid, IFF_UP, IFF_UP);
        }
        close(s);

    } else {
        rc = IP2_OS_if_flags_set(vid, 0, IFF_UP);

    }

ERROR:
    IP2_CRIT_EXIT();
    return rc;
}

vtss_rc vtss_ip2_os_if_del(vtss_vid_t vid)
{
    vtss_rc rc;

    IP2_CRIT_ENTER();
    D("%s", __FUNCTION__);

    rc = vtss_ip2_chip_rleg_del(vid);
    if (rc != VTSS_RC_OK) {
        E("Failed to delete rleg! vlan=%u", vid);
    }

    if (IP2_OS_state[vid].rx_filter_id == 0) {
        D("Failed to delete filter vlan=%u", vid);
        rc = VTSS_RC_ERROR;
        goto ERROR;
    }

    if ((rc = IP2_OS_if_flags_set(vid, 0, IFF_UP)) != VTSS_OK) {
        D("ifdown: returns %d", rc);
        goto ERROR;
    }

    D("Deleting interface: vlan = %u", vid);
    (void) packet_rx_filter_unregister(IP2_OS_state[vid].rx_filter_id);
    IP2_OS_state[vid].rx_filter_id = 0;

    // TODO
    rc = vtss_ip2_ecos_driver_if_del(vid);

ERROR:
    IP2_CRIT_EXIT();
    return rc;
}

vtss_rc vtss_ip2_os_if_status(vtss_if_status_type_t   type,
                              const u32               max,
                              u32                    *cnt,
                              vtss_if_id_vlan_t       id,
                              vtss_if_status_t       *status)
{
    vtss_rc rc;

    IP2_CRIT_ENTER();
    if (IP2_OS_state[id].rx_filter_id == 0) {
        D("ERROR %d", __LINE__);
        rc = VTSS_RC_ERROR;
        goto ERR;
    }

    rc = vtss_ip2_ecos_driver_if_status(type, max, cnt, id, status);

ERR:
    IP2_CRIT_EXIT();
    return rc;
}

vtss_rc vtss_ip2_os_if_status_all(vtss_if_status_type_t   type,
                                  const u32               max,
                                  u32                    *cnt,
                                  vtss_if_status_t       *status)
{
    vtss_rc rc;

    IP2_CRIT_ENTER();
    rc = vtss_ip2_ecos_driver_if_status_all(type, max, cnt, status);
    IP2_CRIT_EXIT();

    return rc;
}

vtss_rc vtss_ip2_os_inject(vtss_vid_t  vid,
                           u32         length,
                           const u8   *const data)
{
    vtss_rc rc = VTSS_RC_OK;
    BOOL filtered_out = FALSE;

    IP2_CRIT_CB_ENTER();
    if (IP2_OS_vtss_ipstack_filter_cb &&
        IP2_OS_vtss_ipstack_filter_cb(vid, length, (char *) data)) {
        N("Frame filtered out: vlan %d, length %d", vid, length);
        rc = VTSS_RC_OK;
        filtered_out = TRUE;
    }
    IP2_CRIT_CB_EXIT();

    if (filtered_out) {
        return VTSS_RC_OK;
    }

    IP2_CRIT_ENTER();
    if (IP2_OS_state[vid].rx_filter_id == 0) {
        N("No such interface vlan %d", vid);
        rc = VTSS_RC_ERROR;
    }
    IP2_CRIT_EXIT();

    if (rc == VTSS_RC_OK) {
        N("Frame inject vlan %d, length %d", vid, length);
        (void) vtss_ip2_ecos_driver_inject(vid, length, data);
    }

    return rc;
}

vtss_rc vtss_ip2_os_ip_add(vtss_vid_t vid,
                           const vtss_ip_network_t *const network)
{
    vtss_rc rc = VTSS_RC_ERROR;

    IP2_CRIT_ENTER();
    switch (network->address.type) {
    case VTSS_IP_TYPE_IPV4: {
        u32 a = network->address.addr.ipv4;
        vtss_ipv4_t mask;

        rc = vtss_conv_prefix_to_ipv4mask(network->prefix_size, &mask);
        if (rc == VTSS_OK) {
            D("vtss_ip2_os_ip_adds vid=%u "VTSS_IPV4_FORMAT"/%u",
              vid, VTSS_IPV4_ARGS(network->address.addr.ipv4),
              network->prefix_size);
            rc = IP2_OS_iface_setaddr_ipv4(vid, a, mask);
        }
        break;
    }

    case VTSS_IP_TYPE_IPV6:
        rc = IP2_OS_iface_setaddr_ipv6(vid, network->address.addr.ipv6.addr, network->prefix_size);
        break;

    default:
        ;
    }
    IP2_CRIT_EXIT();

    return rc;
}

vtss_rc vtss_ip2_os_ip_del(vtss_vid_t vid,
                           const vtss_ip_network_t *const network)
{
    vtss_rc       rc = VTSS_RC_ERROR;
    int           s, i;

    IP2_CRIT_ENTER();
    D("%s", __FUNCTION__);
    switch (network->address.type) {
    case VTSS_IP_TYPE_IPV4: {
        struct  ifreq ifr;
        if ((s = IP2_OS_socket(vid, &ifr)) < 0) {
            rc = IP2_ERROR_FAILED;
            break;
        }
        (void) ioctl(s, SIOCDIFADDR, &ifr);
        rc = VTSS_OK;
        close(s);
    }
    break;

    case VTSS_IP_TYPE_IPV6: {
        struct in6_aliasreq ifra;

        if (vtss_ip_addr_is_zero(&network->address)) {
            rc = VTSS_OK;
            break;
        }

        if ((s = IP2_OS_socket_in6(vid, &ifra)) >= 0) {
            struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)&ifra.ifra_addr;
            memset(addr6, 0, sizeof(*addr6));
            addr6->sin6_family = AF_INET6;
            addr6->sin6_len = sizeof(*addr6);
            addr6->sin6_port = 0;
            for (i = 0; i < 16; i++) {
                addr6->sin6_addr.s6_addr[i] = network->address.addr.ipv6.addr[i];
            }
            if (ioctl(s, SIOCGIFADDR_IN6, &ifra) < 0) {
                E("SIOCGIFADDR_IN6 failed %s", strerror(errno));
            } else if (ioctl(s, SIOCDIFADDR_IN6, &ifra) < 0) {
                E("SIOCDIFADDR_IN6 failed %s", strerror(errno));
            } else {
                D("IPv6 deleted: if %s vid %d", ifra.ifra_name, vid);
                rc = VTSS_OK;
            }
            close(s);
        }
    }
    break;

    case VTSS_IP_TYPE_NONE:
        rc = VTSS_OK;
        break;

    default:
        W("Unknown type: %d", network->address.type);
    }
    IP2_CRIT_EXIT();

    return rc;
}

vtss_rc vtss_ip2_os_route_add(const vtss_routing_entry_t *const rt)
{
    vtss_rc rc;

    switch (rt->type) {
    case VTSS_ROUTING_ENTRY_TYPE_IPV4_UC:
        rc = ipv4_route_op(&rt->route.ipv4_uc, SIOCADDRT);
        D("Added route: " VTSS_IPV4_UC_FORMAT ": %s", VTSS_IPV4_UC_ARGS(rt->route.ipv4_uc), error_txt(rc));
        break;
    case VTSS_ROUTING_ENTRY_TYPE_IPV6_UC:
        rc = ipv6_route_op(&rt->route.ipv6_uc, SIOCADDRT);
        D("Added route: " VTSS_IPV6_UC_FORMAT ": %s", VTSS_IPV6_UC_ARGS(rt->route.ipv6_uc), error_txt(rc));
        break;
    case VTSS_ROUTING_ENTRY_TYPE_IPV4_MC:
    default:
        E("Invalid route type %d", rt->type);
        rc = IP2_ERROR_PARAMS;
    }

    return rc;
}

vtss_rc vtss_ip2_os_route_del(const vtss_routing_entry_t *const rt)
{
    vtss_rc rc;

    switch (rt->type) {
    case VTSS_ROUTING_ENTRY_TYPE_IPV4_UC:
        rc = ipv4_route_op(&rt->route.ipv4_uc, SIOCDELRT);
        D("Deleted IPv4 Route: " VTSS_IPV4_UC_FORMAT ": %s", VTSS_IPV4_UC_ARGS(rt->route.ipv4_uc), error_txt(rc));
        break;
    case VTSS_ROUTING_ENTRY_TYPE_IPV6_UC:
        rc = ipv6_route_op(&rt->route.ipv6_uc, SIOCDELRT);
        D("Deleted IPv6 route: " VTSS_IPV6_UC_FORMAT ": %s", VTSS_IPV6_UC_ARGS(rt->route.ipv6_uc), error_txt(rc));
        break;
    case VTSS_ROUTING_ENTRY_TYPE_IPV4_MC:
    default:
        E("Invalid route type %d", rt->type);
        rc = IP2_ERROR_PARAMS;
    }

    return rc;
}

vtss_rc vtss_ip2_os_route_get( vtss_routing_entry_type_t type,
                               u32                       max,
                               vtss_routing_status_t     *rt,
                               u32                       *const cnt)
{
    vtss_rc rc;

    IP2_CRIT_ENTER();
    rc = vtss_ip2_ecos_driver_route_get(type, max, rt, cnt);
    IP2_CRIT_EXIT();

    return rc;
}

vtss_rc vtss_ip2_os_nb_clear(vtss_ip_type_t type)
{
    vtss_rc rc;

    IP2_CRIT_ENTER();
    rc = vtss_ip2_ecos_driver_nb_clear(type);
    IP2_CRIT_EXIT();

    return rc;
}

vtss_rc vtss_ip2_os_nb_status_get(vtss_ip_type_t               type,
                                  const u32                    max,
                                  u32                         *cnt,
                                  vtss_neighbour_status_t     *status)
{
    vtss_rc rc;

    IP2_CRIT_ENTER();
    rc = vtss_ip2_ecos_driver_nb_status_get(type, max, cnt, status);
    IP2_CRIT_EXIT();

    return rc;
}

void vtss_ip2_fifo_init(void);
vtss_rc vtss_ip2_os_init(void)
{
    (void)ip2_lo0_setaddr_ipv4();

    vtss_ip2_ecos_driver_init();
    critd_init(&crit, "ip.os.crit", VTSS_MODULE_ID_IP2,
               VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
    critd_init(&crit_cb, "ip.os.crit_cb", VTSS_MODULE_ID_IP2,
               VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);

    IP2_CRIT_EXIT();
    IP2_CRIT_CB_EXIT();

    vtss_ip2_fifo_init();

    return VTSS_RC_OK;
}

/* Statistics Section: RFC-4293 */
vtss_rc vtss_ip2_os_stat_ipoutnoroute_get(      vtss_ip_type_t          version,
                                                u32                     *val)
{
    if (!val) {
        return VTSS_RC_ERROR;
    }

    return vtss_ip2_ecos_driver_stat_ipoutnoroute_get(version, val);
}

vtss_rc vtss_ip2_os_stat_syst_cntr_clear(vtss_ip_type_t version)
{
    return vtss_ip2_ecos_driver_stat_syst_cntr_clear(version);
}

vtss_rc vtss_ip2_os_stat_intf_cntr_clear(vtss_ip_type_t version, vtss_if_id_t *ifidx)
{
    vtss_if_id_vlan_t   vidx;
    vtss_if_id_t        entry;

    if (!ifidx) {
        return VTSS_RC_ERROR;
    }

    if (ifidx->type == VTSS_ID_IF_TYPE_VLAN) {
        memset(&entry, 0x0, sizeof(vtss_if_id_t));
        entry.type = VTSS_ID_IF_TYPE_OS_ONLY;
        vidx = ifidx->u.vlan;

        IP2_CRIT_ENTER();
        entry.u.os.ifno = vtss_ip2_ecos_driver_vid_to_if_index(vidx);
        IP2_CRIT_EXIT();
        if (entry.u.os.ifno < 0) {
            D("Failed in vtss_ip2_ecos_driver_vid_to_if_index!");
            return VTSS_RC_ERROR;
        }
    } else if (ifidx->type == VTSS_ID_IF_TYPE_OS_ONLY) {
        IP2_CRIT_ENTER();
        if (IP2_OS_if_index_to_vlan_if(ifidx->u.os.ifno, &vidx) != VTSS_RC_OK) {
            IP2_CRIT_EXIT();
            D("Failed in IP2_OS_if_index_to_vlan_if!");
            return VTSS_RC_ERROR;
        }
        IP2_CRIT_EXIT();

        memcpy(&entry, ifidx, sizeof(vtss_if_id_t));
    } else {
        D("Invalid VTSS_ID_IF_TYPE!");
        return VTSS_RC_ERROR;
    }

    return vtss_ip2_ecos_driver_stat_intf_cntr_clear(version, &entry);
}

vtss_rc vtss_ip2_os_stat_icmp_cntr_get(         vtss_ip_type_t          version,
                                                vtss_ips_icmp_stat_t    *entry)
{
    if (!entry) {
        return VTSS_RC_ERROR;
    }

    return vtss_ip2_ecos_driver_stat_icmp_cntr_get(version, entry);
}

vtss_rc vtss_ip2_os_stat_icmp_cntr_get_first(   vtss_ip_type_t          version,
                                                vtss_ips_icmp_stat_t    *entry)
{
    if (!entry) {
        return VTSS_RC_ERROR;
    }

    return vtss_ip2_ecos_driver_stat_icmp_cntr_get_first(version, entry);
}

vtss_rc vtss_ip2_os_stat_icmp_cntr_get_next(    vtss_ip_type_t          version,
                                                vtss_ips_icmp_stat_t    *entry)
{
    if (!entry) {
        return VTSS_RC_ERROR;
    }

    return vtss_ip2_ecos_driver_stat_icmp_cntr_get_next(version, entry);
}

vtss_rc vtss_ip2_os_stat_icmp_cntr_clear(vtss_ip_type_t version)
{
    return vtss_ip2_ecos_driver_stat_icmp_cntr_clear(version);
}

vtss_rc vtss_ip2_os_stat_imsg_cntr_get(         vtss_ip_type_t          version,
                                                u32                     type,
                                                vtss_ips_icmp_stat_t    *entry)
{
    if (!entry) {
        return VTSS_RC_ERROR;
    }

    return vtss_ip2_ecos_driver_stat_imsg_cntr_get(version, type, entry);
}

vtss_rc vtss_ip2_os_stat_imsg_cntr_get_first(   vtss_ip_type_t          version,
                                                u32                     type,
                                                vtss_ips_icmp_stat_t    *entry)
{
    if (!entry) {
        return VTSS_RC_ERROR;
    }

    return vtss_ip2_ecos_driver_stat_imsg_cntr_get_first(version, type, entry);
}

vtss_rc vtss_ip2_os_stat_imsg_cntr_get_next(    vtss_ip_type_t          version,
                                                u32                     type,
                                                vtss_ips_icmp_stat_t    *entry)
{
    if (!entry) {
        return VTSS_RC_ERROR;
    }

    return vtss_ip2_ecos_driver_stat_imsg_cntr_get_next(version, type, entry);
}

vtss_rc vtss_ip2_os_stat_imsg_cntr_clear(vtss_ip_type_t version, u32 type)
{
    return vtss_ip2_ecos_driver_stat_imsg_cntr_clear(version, type);
}
