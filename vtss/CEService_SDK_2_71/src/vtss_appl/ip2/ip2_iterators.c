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
#include "ip2_os_api.h"
#include "ip2_iterators.h"
#include "vlan_api.h"

#define VTSS_TRACE_MODULE_ID    VTSS_MODULE_ID_IP2
#define VTSS_ALLOC_MODULE_ID    VTSS_MODULE_ID_IP2

#define IP2_ITER_IPS_GET(a, b, c, d, x)                                 \
(((x) = vtss_ip2_ips_status_get((a), (b), (c), (d))) == VTSS_OK)
#define IP2_ITER_IFS_GET(a, b, c, d, x)                                 \
(((x) = vtss_ip2_ifs_status_get((a), (b), (c), (d))) == VTSS_OK)
#define IP2_ITER_IF_GET(a, b, c, d, e, x)                               \
(((x) = vtss_ip2_if_status_get((a), (b), (c), (d), (e))) == VTSS_OK)
#define IP2_ITER_NBR_GET(a, b, c, d, x)                                 \
(((x) = vtss_ip2_nb_status_get((a), (b), (c), (d))) == VTSS_OK)
#define IP2_ITER_IF_STATUS_UP(a)                                        \
((a)->type != VTSS_IF_STATUS_TYPE_LINK) ? FALSE : ((a)->u.link.flags & VTSS_IF_LINK_FLAG_UP)
#define IP2_ITER_IF_STATUS_RUN(a)                                       \
((a)->type != VTSS_IF_STATUS_TYPE_LINK) ? FALSE : ((a)->u.link.flags & VTSS_IF_LINK_FLAG_RUNNING)


static i32 _vtss_ip2_iter_intf_ifidx_cmp_func(void *elm1, void *elm2)
{
    ip2_iter_intf_ifinf_t   *intf1, *intf2;

    if (!elm1 || !elm2) {
        T_W("IP2_ITER_ASSERT(NULL PTR %s%s%s)",
            elm1 ? "" : "ELM1",
            elm1 ? "" : (elm2 ? "" : "&"),
            elm2 ? "" : "ELM2");
        for (;;) {}
    }

    intf1 = (ip2_iter_intf_ifinf_t *)elm1;
    intf2 = (ip2_iter_intf_ifinf_t *)elm2;

    if (IP2_ITER_INTF_IFINFO_VERSION(intf1) > IP2_ITER_INTF_IFINFO_VERSION(intf2)) {
        return 1;
    } else if (IP2_ITER_INTF_IFINFO_VERSION(intf1) < IP2_ITER_INTF_IFINFO_VERSION(intf2)) {
        return -1;
    } else {
        if (IP2_ITER_INTF_IFINFO_IFVDX(intf1) > IP2_ITER_INTF_IFINFO_IFVDX(intf2)) {
            return 1;
        } else if (IP2_ITER_INTF_IFINFO_IFVDX(intf1) < IP2_ITER_INTF_IFINFO_IFVDX(intf2)) {
            return -1;
        } else {
            return 0;
        }
    }
}

static i32 _vtss_ip2_iter_intf_ifadr_cmp_func(void *elm1, void *elm2)
{
    ip2_iter_intf_ifadr_t   *addr1, *addr2;

    if (!elm1 || !elm2) {
        T_W("IP2_ITER_ASSERT(NULL PTR %s%s%s)",
            elm1 ? "" : "ELM1",
            elm1 ? "" : (elm2 ? "" : "&"),
            elm2 ? "" : "ELM2");
        for (;;) {}
    }

    addr1 = (ip2_iter_intf_ifadr_t *)elm1;
    addr2 = (ip2_iter_intf_ifadr_t *)elm2;

    if (IP2_ITER_INTF_IFADR_IPA_VERSION(addr1) > IP2_ITER_INTF_IFADR_IPA_VERSION(addr2)) {
        return 1;
    } else if (IP2_ITER_INTF_IFADR_IPA_VERSION(addr1) < IP2_ITER_INTF_IFADR_IPA_VERSION(addr2)) {
        return -1;
    } else {
        int cmp = 0;

        switch ( IP2_ITER_INTF_IFADR_IPA_VERSION(addr1) ) {
        case VTSS_IP_TYPE_IPV4:
            if (addr1->ipa.addr.ipv4 > addr2->ipa.addr.ipv4) {
                cmp++;
            } else {
                if (addr1->ipa.addr.ipv4 < addr2->ipa.addr.ipv4) {
                    cmp--;
                }
            }
            break;
        case VTSS_IP_TYPE_IPV6:
            cmp = memcmp(&addr1->ipa.addr.ipv6, &addr2->ipa.addr.ipv6, sizeof(vtss_ipv6_t));
            break;
        default:
            cmp = memcmp(&addr1->ipa, &addr2->ipa, sizeof(vtss_ip_addr_t));
            break;
        }

        if (cmp > 0) {
            return 1;
        } else if (cmp < 0) {
            return -1;
        } else {
            return 0;
        }
    }
}

static i32 _vtss_ip2_iter_intf_neighbor_cmp_func(void *elm1, void *elm2)
{
    ip2_iter_intf_nbr_t     *nbr1, *nbr2;

    if (!elm1 || !elm2) {
        T_W("IP2_ITER_ASSERT(NULL PTR %s%s%s)",
            elm1 ? "" : "ELM1",
            elm1 ? "" : (elm2 ? "" : "&"),
            elm2 ? "" : "ELM2");
        for (;;) {}
    }

    nbr1 = (ip2_iter_intf_nbr_t *)elm1;
    nbr2 = (ip2_iter_intf_nbr_t *)elm2;

    if (IP2_ITER_INTF_NBR_VERSION(nbr1) < IP2_ITER_INTF_NBR_VERSION(nbr2)) {
        return 1;
    } else if (IP2_ITER_INTF_NBR_VERSION(nbr1) < IP2_ITER_INTF_NBR_VERSION(nbr2)) {
        return -1;
    } else if (IP2_ITER_INTF_NBR_IFIDX(nbr1) > IP2_ITER_INTF_NBR_IFIDX(nbr2)) {
        return 1;
    } else if (IP2_ITER_INTF_NBR_IFIDX(nbr1) < IP2_ITER_INTF_NBR_IFIDX(nbr2)) {
        return -1;
    } else {
        int cmp = 0;

        switch ( IP2_ITER_INTF_NBR_VERSION(nbr1) ) {
        case VTSS_IP_TYPE_IPV4:
            if (nbr1->nbr.addr.ipv4 > nbr2->nbr.addr.ipv4) {
                cmp++;
            } else {
                if (nbr1->nbr.addr.ipv4 < nbr2->nbr.addr.ipv4) {
                    cmp--;
                }
            }
            break;
        case VTSS_IP_TYPE_IPV6:
            cmp = memcmp(&nbr1->nbr.addr.ipv6, &nbr2->nbr.addr.ipv6, sizeof(vtss_ipv6_t));
            break;
        default:
            cmp = memcmp(&nbr1->nbr, &nbr2->nbr, sizeof(vtss_ip_addr_t));
            break;
        }

        if (cmp > 0) {
            return 1;
        } else if (cmp < 0) {
            return -1;
        } else {
            return 0;
        }
    }
}

static BOOL _vtss_ip2_iter_intf_ifidx_prepare(  vtss_ip_type_t version,
                                                vtss_if_status_t *const ifs,
                                                ip2_iter_intf_ifinf_t *const ifinf)
{
    vtss_rc                 rc;
    vtss_ip2_global_param_t ip_global;
    vtss_if_status_type_t   stype;
    vtss_if_status_t        *fdx;
    u32                     cnt;
    vtss_vid_t              ifid;

    if (!ifs || !ifinf ||
        (ifs->type != VTSS_IF_STATUS_TYPE_LINK) ||
        (ifs->if_id.type != VTSS_ID_IF_TYPE_VLAN)) {
        return FALSE;
    }

    stype = VTSS_IF_STATUS_TYPE_INVALID;
    switch ( version ) {
    case VTSS_IP_TYPE_IPV4:
        stype = VTSS_IF_STATUS_TYPE_IPV4;
        break;
    case VTSS_IP_TYPE_IPV6:
        stype = VTSS_IF_STATUS_TYPE_IPV6;
        break;
    default:
        return FALSE;
    }

    if ((fdx = VTSS_CALLOC(IP2_ITER_SINGLE_IFS_OBJS, sizeof(vtss_if_status_t))) == NULL) {
        return FALSE;
    }

    ifid = ifs->if_id.u.vlan;
    IP2_ITER_INTF_IFINFO_VERSION_SET(ifinf, version);
    IP2_ITER_INTF_IFINFO_IFVDX_SET(ifinf, ifid);
    IP2_ITER_INTF_IFINFO_IFINDEX_SET(ifinf, ifs->u.link.os_if_index);
    IP2_ITER_INTF_IFINFO_MTU_SET(ifinf, ifs->u.link.mtu);

    cnt = 0;
    IP2_ITER_INTF_IFINFO_MGMT_STATE_CLR(ifinf);
    if (IP2_ITER_IF_GET(stype, ifid, IP2_ITER_SINGLE_IFS_OBJS, &cnt, fdx, rc)) {
        if (cnt) {
            IP2_ITER_INTF_IFINFO_MGMT_STATE_SET(ifinf, IP2_ITER_IFINFST_MGMT_ON);
        } else {
            IP2_ITER_INTF_IFINFO_MGMT_STATE_SET(ifinf, IP2_ITER_IFINFST_MGMT_OFF);
        }
        if (IP2_ITER_IF_STATUS_UP(ifs)) {
            IP2_ITER_INTF_IFINFO_MGMT_STATE_SET(ifinf, IP2_ITER_IFINFST_LINK_ON);
        } else {
            IP2_ITER_INTF_IFINFO_MGMT_STATE_SET(ifinf, IP2_ITER_IFINFST_LINK_OFF);
        }
    } else {
        IP2_ITER_INTF_IFINFO_MGMT_STATE_SET(ifinf, IP2_ITER_IFINFST_MGMT_OFF);
    }
    rc = vtss_ip2_global_param_get(&ip_global);
    if ((rc == VTSS_OK) && ip_global.enable_routing) {
        IP2_ITER_INTF_IFINFO_FORWARDING_SET(ifinf, IP2_ITER_DO_FORWARDING);
    } else {
        IP2_ITER_INTF_IFINFO_FORWARDING_SET(ifinf, IP2_ITER_DONOT_FORWARDING);
    }

    /* not yet support; hard coded */
    IP2_ITER_INTF_IFINFO_LIFE_TIME_SET(ifinf, 30000);   /* REACHABLE_TIME: msec */
    /* not yet support; hard coded */
    IP2_ITER_INTF_IFINFO_RXMT_TIME_SET(ifinf, 1000);    /* RETRANS_TIMER: msec */

    VTSS_FREE(fdx);
    return TRUE;
}

static void _vtss_ip2_iter_intf_ifidx_selection(BOOL *const hit,
                                                ip2_iter_intf_ifinf_t *const ifx,
                                                ip2_iter_intf_ifinf_t *const fnd,
                                                ip2_iter_intf_ifinf_t *const res)
{
    if (hit && ifx && fnd && res) {
        if (_vtss_ip2_iter_intf_ifidx_cmp_func((void *)fnd, (void *)ifx) > 0) {
            if (*hit == FALSE) {
                memcpy(res, fnd, sizeof(ip2_iter_intf_ifinf_t));
                *hit = TRUE;
            } else {
                if (_vtss_ip2_iter_intf_ifidx_cmp_func((void *)fnd, (void *)res) < 0) {
                    memcpy(res, fnd, sizeof(ip2_iter_intf_ifinf_t));
                }
            }
        }
    }
}

/*
    Return first interface general information found in IP stack.

    \param version (IN) - version to use as input key.
    \param vidx (IN) - vlan index to use as input key.

    \param entry (OUT) - general information of the matched IPv4 or IPv6 interfrace.

    \return VTSS_OK iff entry is found.
 */
vtss_rc vtss_ip2_intf_ifidx_iter_first( const vtss_ip_type_t        version,
                                        const vtss_if_id_vlan_t     vidx,
                                        ip2_iter_intf_ifinf_t       *const entry)
{
    vtss_rc                 ret;
    BOOL                    hit;
    u32                     idx, cnt;
    vtss_if_status_t        *ifs, *fdx;
    ip2_iter_intf_ifinf_t   *ifx, *fnd, *res;

    if (!entry) {
        return VTSS_RC_ERROR;
    }
    if ((ifs = VTSS_CALLOC(IP2_ITER_MAX_IFS_OBJS, sizeof(vtss_if_status_t))) == NULL) {
        return IP2_ERROR_NOSPACE;
    }
    if ((ifx = VTSS_MALLOC(sizeof(ip2_iter_intf_ifinf_t))) == NULL) {
        VTSS_FREE(ifs);
        return IP2_ERROR_NOSPACE;
    }
    if ((fnd = VTSS_MALLOC(sizeof(ip2_iter_intf_ifinf_t))) == NULL) {
        VTSS_FREE(ifx);
        VTSS_FREE(ifs);
        return IP2_ERROR_NOSPACE;
    }
    if ((res = VTSS_MALLOC(sizeof(ip2_iter_intf_ifinf_t))) == NULL) {
        VTSS_FREE(fnd);
        VTSS_FREE(ifx);
        VTSS_FREE(ifs);
        return IP2_ERROR_NOSPACE;
    }

    fdx = ifs;
    hit = FALSE;
    memset(res, 0x0, sizeof(ip2_iter_intf_ifinf_t));
    if (IP2_ITER_IFS_GET(VTSS_IF_STATUS_TYPE_LINK, IP2_ITER_MAX_IFS_OBJS, &cnt, ifs, ret)) {
        memset(ifx, 0x0, sizeof(ip2_iter_intf_ifinf_t));
        for (idx = 0; idx < cnt; ++idx, ++ifs) {
            if (_vtss_ip2_iter_intf_ifidx_prepare(VTSS_IP_TYPE_IPV4, ifs, fnd)) {
                _vtss_ip2_iter_intf_ifidx_selection(&hit, ifx, fnd, res);
            }
            if (_vtss_ip2_iter_intf_ifidx_prepare(VTSS_IP_TYPE_IPV6, ifs, fnd)) {
                _vtss_ip2_iter_intf_ifidx_selection(&hit, ifx, fnd, res);
            }
        }
    }

    if (hit && (ret == VTSS_OK)) {
        memcpy(entry, res, sizeof(ip2_iter_intf_ifinf_t));
    } else {
        if (ret == VTSS_OK) {
            ret = IP2_ERROR_NOTFOUND;
        }
    }

    VTSS_FREE(res);
    VTSS_FREE(fnd);
    VTSS_FREE(ifx);
    VTSS_FREE(fdx);

    return ret;
}

/*
    Return specific interface general information found in IP stack.

    \param version (IN) - version to use as input key.
    \param vidx (IN) - vlan index to use as input key.

    \param entry (OUT) - general information of the matched IPv4 or IPv6 interfrace.

    \return VTSS_OK iff entry is found.
 */
vtss_rc vtss_ip2_intf_ifidx_iter_get(   const vtss_ip_type_t        version,
                                        const vtss_if_id_vlan_t     vidx,
                                        ip2_iter_intf_ifinf_t       *const entry)
{
    vtss_rc                 ret;
    vtss_if_status_t        *fdx;
    u32                     cnt;

    if (!entry) {
        return VTSS_RC_ERROR;
    }

    switch ( version ) {
    case VTSS_IP_TYPE_IPV4:
    case VTSS_IP_TYPE_IPV6:
        break;
    default:
        return VTSS_RC_ERROR;
    }

    if ((fdx = VTSS_CALLOC(IP2_ITER_SINGLE_IFS_OBJS, sizeof(vtss_if_status_t))) == NULL) {
        return IP2_ERROR_NOSPACE;
    }

    cnt = 0;
    if (IP2_ITER_IF_GET(VTSS_IF_STATUS_TYPE_LINK, vidx, IP2_ITER_SINGLE_IFS_OBJS, &cnt, fdx, ret) && cnt) {
        if (!_vtss_ip2_iter_intf_ifidx_prepare(version, fdx, entry)) {
            ret = IP2_ERROR_FAILED;
        }
    }

    VTSS_FREE(fdx);
    return ret;
}

/*
    Return next interface general information found in IP stack.

    \param version (IN) - version to use as input key.
    \param vidx (IN) - vlan index to use as input key.

    \param entry (OUT) - general information of the matched IPv4 or IPv6 interfrace.

    \return VTSS_OK iff entry is found.
 */
vtss_rc vtss_ip2_intf_ifidx_iter_next(  const vtss_ip_type_t        version,
                                        const vtss_if_id_vlan_t     vidx,
                                        ip2_iter_intf_ifinf_t       *const entry)
{
    vtss_rc                 ret;
    BOOL                    hit;
    u32                     idx, cnt;
    vtss_if_status_t        *ifs, *fdx;
    ip2_iter_intf_ifinf_t   *ifx, *fnd, *res;

    if (!entry) {
        return VTSS_RC_ERROR;
    }
    if ((ifs = VTSS_CALLOC(IP2_ITER_MAX_IFS_OBJS, sizeof(vtss_if_status_t))) == NULL) {
        return IP2_ERROR_NOSPACE;
    }
    if ((ifx = VTSS_MALLOC(sizeof(ip2_iter_intf_ifinf_t))) == NULL) {
        VTSS_FREE(ifs);
        return IP2_ERROR_NOSPACE;
    }
    if ((fnd = VTSS_MALLOC(sizeof(ip2_iter_intf_ifinf_t))) == NULL) {
        VTSS_FREE(ifx);
        VTSS_FREE(ifs);
        return IP2_ERROR_NOSPACE;
    }
    if ((res = VTSS_MALLOC(sizeof(ip2_iter_intf_ifinf_t))) == NULL) {
        VTSS_FREE(fnd);
        VTSS_FREE(ifx);
        VTSS_FREE(ifs);
        return IP2_ERROR_NOSPACE;
    }

    fdx = ifs;
    hit = FALSE;
    memset(res, 0x0, sizeof(ip2_iter_intf_ifinf_t));
    if (IP2_ITER_IFS_GET(VTSS_IF_STATUS_TYPE_LINK, IP2_ITER_MAX_IFS_OBJS, &cnt, ifs, ret)) {
        IP2_ITER_INTF_IFINFO_VERSION_SET(ifx, version);
        IP2_ITER_INTF_IFINFO_IFVDX_SET(ifx, vidx);

        for (idx = 0; idx < cnt; ++idx, ++ifs) {
            if (_vtss_ip2_iter_intf_ifidx_prepare(VTSS_IP_TYPE_IPV4, ifs, fnd)) {
                _vtss_ip2_iter_intf_ifidx_selection(&hit, ifx, fnd, res);
            }
            if (_vtss_ip2_iter_intf_ifidx_prepare(VTSS_IP_TYPE_IPV6, ifs, fnd)) {
                _vtss_ip2_iter_intf_ifidx_selection(&hit, ifx, fnd, res);
            }
        }
    }

    if (hit && (ret == VTSS_OK)) {
        memcpy(entry, res, sizeof(ip2_iter_intf_ifinf_t));
    } else {
        if (ret == VTSS_OK) {
            ret = IP2_ERROR_NOTFOUND;
        }
    }

    VTSS_FREE(res);
    VTSS_FREE(fnd);
    VTSS_FREE(ifx);
    VTSS_FREE(fdx);

    return ret;
}

static BOOL _vtss_ip2_iter_intf_ifadr_prepare(  vtss_ip_type_t version,
                                                vtss_if_status_t *const ifs,
                                                ip2_iter_intf_ifadr_t *const ifadr)
{
    vtss_rc                 rc;
    u32                     cnt;
    vtss_vid_t              ifid;
    vtss_if_status_t        *fdx;
    vtss_if_status_ipv4_t   *ip4s;
    vtss_if_status_ipv6_t   *ip6s;

    if (!ifs || !ifadr ||
        (ifs->if_id.type != VTSS_ID_IF_TYPE_VLAN)) {
        return FALSE;
    }

    switch ( version ) {
    case VTSS_IP_TYPE_IPV4:
        ip4s = &ifs->u.ipv4;
        ip6s = NULL;
        break;
    case VTSS_IP_TYPE_IPV6:
        ip4s = NULL;
        ip6s = &ifs->u.ipv6;
        break;
    default:
        return FALSE;
    }

    if ((fdx = VTSS_CALLOC(IP2_ITER_SINGLE_IFS_OBJS, sizeof(vtss_if_status_t))) == NULL) {
        return FALSE;
    }

    cnt = 0;
    ifid = ifs->if_id.u.vlan;
    if (IP2_ITER_IF_GET(VTSS_IF_STATUS_TYPE_LINK, ifid, IP2_ITER_SINGLE_IFS_OBJS, &cnt, fdx, rc) && cnt) {
        vtss_timestamp_t        ips_ts;
        ip2_iter_intf_ifinf_t   *ifinf;
        vtss_ip2_global_param_t ip_global;

        ifinf = &ifadr->if_info;
        if (ip4s) {
            IP2_ITER_INTF_IFADR_IPV4_ADDR_SET(ifadr, ip4s->net.address);
            IP2_ITER_INTF_IFADR_TYPE_SET(ifadr, IP2_IFADR_TYPE_UNICAST);
            IP2_ITER_INTF_IFADR_STATUS_SET(ifadr, IP2_IFADR_STATUS_PREFER);

            IP2_ITER_INTF_IFINFO_MTU_SET(ifinf, ip4s->reasm_max_size);
            IP2_ITER_INTF_IFINFO_RXMT_TIME_SET(ifinf, ip4s->arp_retransmit_time);
        }
        if (ip6s) {
            IP2_ITER_INTF_IFADR_IPV6_ADDR_SET(ifadr, &ip6s->net.address);
            if (ip6s->flags & VTSS_IF_IPV6_FLAG_ANYCAST) {
                IP2_ITER_INTF_IFADR_TYPE_SET(ifadr, IP2_IFADR_TYPE_ANYCAST);
            } else {
                IP2_ITER_INTF_IFADR_TYPE_SET(ifadr, IP2_IFADR_TYPE_UNICAST);
            }
            if (ip6s->flags & VTSS_IF_IPV6_FLAG_TENTATIVE) {
                IP2_ITER_INTF_IFADR_STATUS_SET(ifadr, IP2_IFADR_STATUS_TENTATIVE);
            } else if (ip6s->flags & VTSS_IF_IPV6_FLAG_DETACHED) {
                IP2_ITER_INTF_IFADR_STATUS_SET(ifadr, IP2_IFADR_STATUS_INACCESS);
            } else if (ip6s->flags & VTSS_IF_IPV6_FLAG_DUPLICATED) {
                IP2_ITER_INTF_IFADR_STATUS_SET(ifadr, IP2_IFADR_STATUS_DUPLICATE);
            } else if (ip6s->flags & VTSS_IF_IPV6_FLAG_DEPRECATED) {
                IP2_ITER_INTF_IFADR_STATUS_SET(ifadr, IP2_IFADR_STATUS_DEPRECATE);
            } else {
                if (ip6s->flags & VTSS_IF_IPV6_FLAG_TEMPORARY) {
                    IP2_ITER_INTF_IFADR_STATUS_SET(ifadr, IP2_IFADR_STATUS_UNKNOWN);
                } else {
                    IP2_ITER_INTF_IFADR_STATUS_SET(ifadr, IP2_IFADR_STATUS_PREFER);
                }
            }

            IP2_ITER_INTF_IFINFO_MTU_SET(ifinf, fdx->u.link.mtu);
            /* not yet support; hard coded */
            IP2_ITER_INTF_IFINFO_RXMT_TIME_SET(ifinf, 1000);    /* RETRANS_TIMER: msec */
        }
        memset(&ips_ts, 0x0, sizeof(vtss_timestamp_t));
        IP2_ITER_INTF_IFADR_CREATED_SET(ifadr, &ips_ts);
        IP2_ITER_INTF_IFADR_LAST_CHANGE_SET(ifadr, &ips_ts);
        IP2_ITER_INTF_IFADR_ROW_STATUS_SET(ifadr, IP2_ITER_STATE_ENABLED);
        IP2_ITER_INTF_IFADR_STORAGE_TYPE_SET(ifadr, IP2_IFADR_STORAGE_VOLATILE);

        IP2_ITER_INTF_IFINFO_VERSION_SET(ifinf, version);
        IP2_ITER_INTF_IFINFO_IFVDX_SET(ifinf, ifid);
        IP2_ITER_INTF_IFINFO_IFINDEX_SET(ifinf, fdx->u.link.os_if_index);
        IP2_ITER_INTF_IFINFO_MGMT_STATE_CLR(ifinf);
        IP2_ITER_INTF_IFINFO_MGMT_STATE_SET(ifinf, IP2_ITER_IFINFST_MGMT_ON);
        if (IP2_ITER_IF_STATUS_UP(fdx)) {
            IP2_ITER_INTF_IFINFO_MGMT_STATE_SET(ifinf, IP2_ITER_IFINFST_LINK_ON);
        } else {
            IP2_ITER_INTF_IFINFO_MGMT_STATE_SET(ifinf, IP2_ITER_IFINFST_LINK_OFF);
        }
        if ((vtss_ip2_global_param_get(&ip_global) == VTSS_OK) &&
            ip_global.enable_routing) {
            IP2_ITER_INTF_IFINFO_FORWARDING_SET(ifinf, IP2_ITER_DO_FORWARDING);
        } else {
            IP2_ITER_INTF_IFINFO_FORWARDING_SET(ifinf, IP2_ITER_DONOT_FORWARDING);
        }
        /* not yet support; hard coded */
        IP2_ITER_INTF_IFINFO_LIFE_TIME_SET(ifinf, 30000);       /* REACHABLE_TIME: msec */
    }

    VTSS_FREE(fdx);
    return (cnt && (rc == VTSS_OK));
}

static void _vtss_ip2_iter_intf_ifadr_selection(BOOL *const hit,
                                                ip2_iter_intf_ifadr_t *const ifx,
                                                ip2_iter_intf_ifadr_t *const fnd,
                                                ip2_iter_intf_ifadr_t *const res)
{
    if (hit && ifx && fnd && res) {
        if (_vtss_ip2_iter_intf_ifadr_cmp_func((void *)fnd, (void *)ifx) > 0) {
            if (*hit == FALSE) {
                memcpy(res, fnd, sizeof(ip2_iter_intf_ifadr_t));
                *hit = TRUE;
            } else {
                if (_vtss_ip2_iter_intf_ifadr_cmp_func((void *)fnd, (void *)res) < 0) {
                    memcpy(res, fnd, sizeof(ip2_iter_intf_ifadr_t));
                }
            }
        }
    }
}

/*
    Return first interface address information found in IP stack.

    \param ifadr (IN) - version and address (defined in vtss_ip_addr_t) to use as input key.

    \param entry (OUT) - address information of the matched IPv4 or IPv6 interfrace.

    \return VTSS_OK iff entry is found.
 */
vtss_rc vtss_ip2_intf_ifadr_iter_first( const vtss_ip_addr_t        *ifadr,
                                        ip2_iter_intf_ifadr_t       *const entry)
{
    vtss_rc                 ret;
    BOOL                    hit;
    u32                     idx, cnt;
    vtss_if_status_t        *ifs, *fdx;
    ip2_iter_intf_ifadr_t   *ifx, *fnd, *res;

    if (!entry) {
        return VTSS_RC_ERROR;
    }
    if ((ifs = VTSS_CALLOC(IP2_ITER_MAX_IFS_OBJS, sizeof(vtss_if_status_t))) == NULL) {
        return IP2_ERROR_NOSPACE;
    }
    if ((ifx = VTSS_MALLOC(sizeof(ip2_iter_intf_ifadr_t))) == NULL) {
        VTSS_FREE(ifs);
        return IP2_ERROR_NOSPACE;
    }
    if ((fnd = VTSS_MALLOC(sizeof(ip2_iter_intf_ifadr_t))) == NULL) {
        VTSS_FREE(ifx);
        VTSS_FREE(ifs);
        return IP2_ERROR_NOSPACE;
    }
    if ((res = VTSS_MALLOC(sizeof(ip2_iter_intf_ifadr_t))) == NULL) {
        VTSS_FREE(fnd);
        VTSS_FREE(ifx);
        VTSS_FREE(ifs);
        return IP2_ERROR_NOSPACE;
    }

    fdx = ifs;
    hit = FALSE;
    memset(res, 0x0, sizeof(ip2_iter_intf_ifadr_t));
    if (IP2_ITER_IFS_GET(VTSS_IF_STATUS_TYPE_IPV4, IP2_ITER_MAX_IFS_OBJS, &cnt, ifs, ret)) {
        memset(ifx, 0x0, sizeof(ip2_iter_intf_ifadr_t));

        for (idx = 0; idx < cnt; ++idx, ++ifs) {
            if (_vtss_ip2_iter_intf_ifadr_prepare(VTSS_IP_TYPE_IPV4, ifs, fnd)) {
                _vtss_ip2_iter_intf_ifadr_selection(&hit, ifx, fnd, res);
            }
        }

        ifs = fdx;
        if (!hit && IP2_ITER_IFS_GET(VTSS_IF_STATUS_TYPE_IPV6, IP2_ITER_MAX_IFS_OBJS, &cnt, ifs, ret)) {
            for (idx = 0; idx < cnt; ++idx, ++ifs) {
                if (_vtss_ip2_iter_intf_ifadr_prepare(VTSS_IP_TYPE_IPV6, ifs, fnd)) {
                    _vtss_ip2_iter_intf_ifadr_selection(&hit, ifx, fnd, res);
                }
            }
        }
    }

    if (hit && (ret == VTSS_OK)) {
        memcpy(entry, res, sizeof(ip2_iter_intf_ifadr_t));
    } else {
        if (ret == VTSS_OK) {
            ret = IP2_ERROR_NOTFOUND;
        }
    }

    VTSS_FREE(res);
    VTSS_FREE(fnd);
    VTSS_FREE(ifx);
    VTSS_FREE(fdx);

    return ret;
}

/*
    Return specific interface address information found in IP stack.

    \param ifadr (IN) - version and address (defined in vtss_ip_addr_t) to use as input key.

    \param entry (OUT) - address information of the matched IPv4 or IPv6 interfrace.

    \return VTSS_OK iff entry is found.
 */
vtss_rc vtss_ip2_intf_ifadr_iter_get(   const vtss_ip_addr_t        *ifadr,
                                        ip2_iter_intf_ifadr_t       *const entry)
{
    vtss_rc                 ret;
    BOOL                    hit;
    u32                     idx, cnt;
    vtss_if_status_type_t   stype;
    vtss_if_status_t        *ifs, *fdx;
    ip2_iter_intf_ifadr_t   *ifx;

    if (!ifadr || !entry) {
        return VTSS_RC_ERROR;
    }

    stype = VTSS_IF_STATUS_TYPE_INVALID;
    switch ( ifadr->type ) {
    case VTSS_IP_TYPE_IPV4:
        stype = VTSS_IF_STATUS_TYPE_IPV4;
        break;
    case VTSS_IP_TYPE_IPV6:
        stype = VTSS_IF_STATUS_TYPE_IPV6;
        break;
    default:
        return VTSS_RC_ERROR;
    }

    if ((ifs = VTSS_CALLOC(IP2_ITER_MAX_IFS_OBJS, sizeof(vtss_if_status_t))) == NULL) {
        return IP2_ERROR_NOSPACE;
    }
    if ((ifx = VTSS_MALLOC(sizeof(ip2_iter_intf_ifadr_t))) == NULL) {
        VTSS_FREE(ifs);
        return IP2_ERROR_NOSPACE;
    }

    fdx = ifs;
    hit = FALSE;
    memset(ifx, 0x0, sizeof(ip2_iter_intf_ifadr_t));
    if (IP2_ITER_IFS_GET(stype, IP2_ITER_MAX_IFS_OBJS, &cnt, ifs, ret)) {
        for (idx = 0; idx < cnt; ++idx, ++ifs) {
            if (stype == VTSS_IF_STATUS_TYPE_IPV4) {
                if (ifadr->addr.ipv4 == ifs->u.ipv4.net.address) {
                    hit = TRUE;
                }
            }
            if (stype == VTSS_IF_STATUS_TYPE_IPV6) {
                if (!memcmp(&ifadr->addr.ipv6, &ifs->u.ipv6.net.address, sizeof(vtss_ipv6_t))) {
                    hit = TRUE;
                }
            }

            if (hit) {
                if (!_vtss_ip2_iter_intf_ifadr_prepare(ifadr->type, ifs, ifx)) {
                    hit = FALSE;
                }

                break;
            }
        }
    }

    if (hit && (ret == VTSS_OK)) {
        memcpy(entry, ifx, sizeof(ip2_iter_intf_ifadr_t));
    } else {
        if (ret == VTSS_OK) {
            ret = IP2_ERROR_NOTFOUND;
        }
    }

    VTSS_FREE(ifx);
    VTSS_FREE(fdx);

    return ret;
}

/*
    Return next interface address information found in IP stack.

    \param ifadr (IN) - version and address (defined in vtss_ip_addr_t) to use as input key.

    \param entry (OUT) - address information of the matched IPv4 or IPv6 interfrace.

    \return VTSS_OK iff entry is found.
 */
vtss_rc vtss_ip2_intf_ifadr_iter_next(  const vtss_ip_addr_t        *ifadr,
                                        ip2_iter_intf_ifadr_t       *const entry)
{
    vtss_rc                 ret;
    BOOL                    hit;
    u32                     idx, cnt;
    vtss_if_status_t        *ifs, *fdx;
    ip2_iter_intf_ifadr_t   *ifx, *fnd, *res;

    if (!ifadr || !entry) {
        return VTSS_RC_ERROR;
    }
    if ((ifs = VTSS_CALLOC(IP2_ITER_MAX_IFS_OBJS, sizeof(vtss_if_status_t))) == NULL) {
        return IP2_ERROR_NOSPACE;
    }
    if ((ifx = VTSS_MALLOC(sizeof(ip2_iter_intf_ifadr_t))) == NULL) {
        VTSS_FREE(ifs);
        return IP2_ERROR_NOSPACE;
    }
    if ((fnd = VTSS_MALLOC(sizeof(ip2_iter_intf_ifadr_t))) == NULL) {
        VTSS_FREE(ifx);
        VTSS_FREE(ifs);
        return IP2_ERROR_NOSPACE;
    }
    if ((res = VTSS_MALLOC(sizeof(ip2_iter_intf_ifadr_t))) == NULL) {
        VTSS_FREE(fnd);
        VTSS_FREE(ifx);
        VTSS_FREE(ifs);
        return IP2_ERROR_NOSPACE;
    }

    fdx = ifs;
    hit = FALSE;
    memset(res, 0x0, sizeof(ip2_iter_intf_ifadr_t));
    if (IP2_ITER_IFS_GET(VTSS_IF_STATUS_TYPE_IPV4, IP2_ITER_MAX_IFS_OBJS, &cnt, ifs, ret)) {
        memset(ifx, 0x0, sizeof(ip2_iter_intf_ifadr_t));
        switch ( ifadr->type ) {
        case VTSS_IP_TYPE_IPV4:
            IP2_ITER_INTF_IFADR_IPV4_ADDR_SET(ifx, ifadr->addr.ipv4);
            break;
        case VTSS_IP_TYPE_IPV6:
            IP2_ITER_INTF_IFADR_IPV6_ADDR_SET(ifx, &ifadr->addr.ipv6);
            break;
        default:
            IP2_ITER_INTF_IFADR_IPA_VERSION_SET(ifx, VTSS_IP_TYPE_NONE);
            break;
        }

        for (idx = 0; idx < cnt; ++idx, ++ifs) {
            if (_vtss_ip2_iter_intf_ifadr_prepare(VTSS_IP_TYPE_IPV4, ifs, fnd)) {
                _vtss_ip2_iter_intf_ifadr_selection(&hit, ifx, fnd, res);
            }
        }

        ifs = fdx;
        if (IP2_ITER_IFS_GET(VTSS_IF_STATUS_TYPE_IPV6, IP2_ITER_MAX_IFS_OBJS, &cnt, ifs, ret)) {
            for (idx = 0; idx < cnt; ++idx, ++ifs) {
                if (_vtss_ip2_iter_intf_ifadr_prepare(VTSS_IP_TYPE_IPV6, ifs, fnd)) {
                    _vtss_ip2_iter_intf_ifadr_selection(&hit, ifx, fnd, res);
                }
            }
        }
    }

    if (hit && (ret == VTSS_OK)) {
        memcpy(entry, res, sizeof(ip2_iter_intf_ifadr_t));
    } else {
        if (ret == VTSS_OK) {
            ret = IP2_ERROR_NOTFOUND;
        }
    }

    VTSS_FREE(res);
    VTSS_FREE(fnd);
    VTSS_FREE(ifx);
    VTSS_FREE(fdx);

    return ret;
}

/* Statistics Section: RFC-4293 */
/*
    Return first statistics found in IP stack.

    \param version (IN) - version to use as input key.

    \param entry (OUT) - statistics of either IPv4 or IPv6.

    \return VTSS_OK iff entry is found.
 */
vtss_rc vtss_ip2_cntr_syst_stat_iter_first( const vtss_ip_type_t        *version,
                                            vtss_ips_ip_stat_t          *const entry)
{
    vtss_rc             ret;
    vtss_ips_status_t   *fdx, all_ips[IP2_ITER_MAX_IPS_OBJS];
    u32                 cnt_ips;

    if (!version || !entry) {
        return VTSS_RC_ERROR;
    }

    memset(all_ips, 0x0, sizeof(all_ips));
    if (IP2_ITER_IPS_GET(VTSS_IPS_STATUS_TYPE_ANY, IP2_ITER_MAX_IPS_OBJS, &cnt_ips, all_ips, ret)) {
        u32 idx;

        ret = IP2_ERROR_NOTFOUND;
        for (idx = 0; idx < cnt_ips; idx++) {
            fdx = &all_ips[idx];

            if ((fdx->type == VTSS_IPS_STATUS_TYPE_STAT_IPV4) ||
                (fdx->type == VTSS_IPS_STATUS_TYPE_STAT_IPV6)) {
                memcpy(entry, &fdx->u.ip_stat, sizeof(vtss_ips_ip_stat_t));
                ret = VTSS_OK;
                break;
            }
        }
    }

    return ret;
}

/*
    Return specific statistics found in IP stack.

    \param version (IN) - version to use as input key.

    \param entry (OUT) - statistics of either IPv4 or IPv6.

    \return VTSS_OK iff entry is found.
 */
vtss_rc vtss_ip2_cntr_syst_stat_iter_get(   const vtss_ip_type_t        *version,
                                            vtss_ips_ip_stat_t          *const entry)
{
    vtss_rc                 ret;
    vtss_ips_status_t       *fdx, all_ips[IP2_ITER_SINGLE_IPS_OBJS];
    vtss_ips_status_type_t  get_type;
    u32                     cnt_ips;

    if (!version || !entry) {
        return VTSS_RC_ERROR;
    }

    get_type = VTSS_IPS_STATUS_TYPE_ANY;
    if (*version == VTSS_IP_TYPE_IPV4) {
        get_type = VTSS_IPS_STATUS_TYPE_STAT_IPV4;
    } else if (*version == VTSS_IP_TYPE_IPV6) {
        get_type = VTSS_IPS_STATUS_TYPE_STAT_IPV6;
    } else {
        return VTSS_RC_ERROR;
    }

    memset(all_ips, 0x0, sizeof(all_ips));
    if (IP2_ITER_IPS_GET(get_type, IP2_ITER_SINGLE_IPS_OBJS, &cnt_ips, all_ips, ret)) {
        u32 idx;

        ret = IP2_ERROR_NOTFOUND;
        for (idx = 0; idx < cnt_ips; idx++) {
            fdx = &all_ips[idx];

            if ((fdx->type == VTSS_IPS_STATUS_TYPE_STAT_IPV4) &&
                (*version == VTSS_IP_TYPE_IPV4)) {
                memcpy(entry, &fdx->u.ip_stat, sizeof(vtss_ips_ip_stat_t));
                ret = VTSS_OK;
                break;
            }
            if ((fdx->type == VTSS_IPS_STATUS_TYPE_STAT_IPV6) &&
                (*version == VTSS_IP_TYPE_IPV6)) {
                memcpy(entry, &fdx->u.ip_stat, sizeof(vtss_ips_ip_stat_t));
                ret = VTSS_OK;
                break;
            }
        }
    }

    return ret;
}

/*
    Return next statistics found in IP stack.

    \param version (IN) - version to use as input key.

    \param entry (OUT) - statistics of either IPv4 or IPv6.

    \return VTSS_OK iff entry is found.
 */
vtss_rc vtss_ip2_cntr_syst_stat_iter_next(  const vtss_ip_type_t        *version,
                                            vtss_ips_ip_stat_t          *const entry)
{
    vtss_rc                 ret;
    vtss_ips_status_t       *fdx, all_ips[IP2_ITER_SINGLE_IPS_OBJS];
    vtss_ips_status_type_t  get_type;
    u32                     cnt_ips;

    if (!version || !entry) {
        return VTSS_RC_ERROR;
    }

    get_type = VTSS_IPS_STATUS_TYPE_ANY;
    if (*version < VTSS_IP_TYPE_IPV4) {
        get_type = VTSS_IPS_STATUS_TYPE_STAT_IPV4;
    } else {
        if (*version < VTSS_IP_TYPE_IPV6) {
            get_type = VTSS_IPS_STATUS_TYPE_STAT_IPV6;
        } else {
            return VTSS_RC_ERROR;
        }
    }

    memset(all_ips, 0x0, sizeof(all_ips));
    if (IP2_ITER_IPS_GET(get_type, IP2_ITER_SINGLE_IPS_OBJS, &cnt_ips, all_ips, ret)) {
        u32 idx;

        ret = IP2_ERROR_NOTFOUND;
        for (idx = 0; idx < cnt_ips; idx++) {
            fdx = &all_ips[idx];

            if ((fdx->type == VTSS_IPS_STATUS_TYPE_STAT_IPV4) ||
                (fdx->type == VTSS_IPS_STATUS_TYPE_STAT_IPV6)) {
                memcpy(entry, &fdx->u.ip_stat, sizeof(vtss_ips_ip_stat_t));
                ret = VTSS_OK;
                break;
            }
        }
    }

    return ret;
}

static BOOL _vtss_ip2_cntr_intf_stat_iter_next(vtss_if_id_vlan_t vidx,
                                               u32 cnt,
                                               vtss_if_status_t *all_ifs,
                                               vtss_if_status_ip_stat_t *entry)
{
    u32                 idx;
    vtss_if_status_t    *fdx, *fnd;

    if (!all_ifs || !entry) {
        return FALSE;
    }

    fnd = NULL;
    for (idx = 0; idx < cnt; idx++) {
        fdx = &all_ifs[idx];
        if (fdx->if_id.type != VTSS_ID_IF_TYPE_VLAN) {
            continue;
        }

        if (fdx->if_id.u.vlan > vidx) {
            if (!fnd) {
                fnd = fdx;
            } else {
                if (fdx->if_id.u.vlan < fnd->if_id.u.vlan) {
                    fnd = fdx;
                }
            }
        }
    }

    if (fnd) {
        memcpy(entry, &fnd->u.ip_stat, sizeof(vtss_if_status_ip_stat_t));
        return TRUE;
    } else {
        return FALSE;
    }
}

/*
    Return first interface statistics found in IP stack.

    \param version (IN) - version to use as input key.
    \param vidx (IN) - vlan index to use as input key.

    \param entry (OUT) - statistics of the matched IPv4 or IPv6 interfrace.

    \return VTSS_OK iff entry is found.
 */
vtss_rc vtss_ip2_cntr_intf_stat_iter_first( const vtss_ip_type_t        *version,
                                            const vtss_if_id_vlan_t     *vidx,
                                            vtss_if_status_ip_stat_t    *const entry)
{
    vtss_rc             ret;
    vtss_if_status_t    *all_ifs;
    u32                 cnt_ifs;

    if (!version || !vidx || !entry) {
        return VTSS_RC_ERROR;
    }

    all_ifs = VTSS_CALLOC(IP2_ITER_MAX_IFS_OBJS, sizeof(vtss_if_status_t));
    if (all_ifs == NULL) {
        return IP2_ERROR_NOSPACE;
    }

    cnt_ifs = 0;
    memset(all_ifs, 0x0, sizeof(*all_ifs));
    if (IP2_ITER_IFS_GET(VTSS_IF_STATUS_TYPE_STAT_IPV4, IP2_ITER_MAX_IFS_OBJS, &cnt_ifs, all_ifs, ret)) {
        if (!_vtss_ip2_cntr_intf_stat_iter_next(VTSS_VID_NULL, cnt_ifs, all_ifs, entry)) {
            ret = IP2_ERROR_NOTFOUND;
        }
    } else {
        cnt_ifs = 0;
        memset(all_ifs, 0x0, sizeof(*all_ifs));
        if (IP2_ITER_IFS_GET(VTSS_IF_STATUS_TYPE_STAT_IPV6, IP2_ITER_MAX_IFS_OBJS, &cnt_ifs, all_ifs, ret)) {
            if (!_vtss_ip2_cntr_intf_stat_iter_next(VTSS_VID_NULL, cnt_ifs, all_ifs, entry)) {
                ret = IP2_ERROR_NOTFOUND;
            }
        }
    }

    VTSS_FREE(all_ifs);
    return ret;
}

/*
    Return specific interface statistics found in IP stack.

    \param version (IN) - version to use as input key.
    \param vidx (IN) - vlan index to use as input key.

    \param entry (OUT) - statistics of the matched IPv4 or IPv6 interfrace.

    \return VTSS_OK iff entry is found.
 */
vtss_rc vtss_ip2_cntr_intf_stat_iter_get(   const vtss_ip_type_t        *version,
                                            const vtss_if_id_vlan_t     *vidx,
                                            vtss_if_status_ip_stat_t    *const entry)
{
    vtss_rc                 ret;
    vtss_if_status_t        *fdx, *all_ifs;
    vtss_if_status_type_t   get_type;
    u32                     cnt_ifs;

    if (!version || !vidx || !entry) {
        return VTSS_RC_ERROR;
    }

    get_type = VTSS_IF_STATUS_TYPE_ANY;
    if (*version == VTSS_IP_TYPE_IPV4) {
        get_type = VTSS_IF_STATUS_TYPE_STAT_IPV4;
    } else if (*version == VTSS_IP_TYPE_IPV6) {
        get_type = VTSS_IF_STATUS_TYPE_STAT_IPV6;
    } else {
        return VTSS_RC_ERROR;
    }

    all_ifs = VTSS_CALLOC(IP2_ITER_SINGLE_IFS_OBJS, sizeof(vtss_if_status_t));
    if (all_ifs == NULL) {
        return IP2_ERROR_NOSPACE;
    }

    cnt_ifs = 0;
    memset(all_ifs, 0x0, sizeof(*all_ifs));
    if (IP2_ITER_IF_GET(get_type, *vidx, IP2_ITER_SINGLE_IFS_OBJS, &cnt_ifs, all_ifs, ret)) {
        u32 idx;

        ret = IP2_ERROR_NOTFOUND;
        for (idx = 0; idx < cnt_ifs; idx++) {
            fdx = &all_ifs[idx];

            if ((fdx->type == get_type) &&
                (fdx->if_id.u.vlan == *vidx)) {
                memcpy(entry, &fdx->u.ip_stat, sizeof(vtss_if_status_ip_stat_t));
                ret = VTSS_OK;
                break;
            }
        }
    }

    VTSS_FREE(all_ifs);
    return ret;
}

/*
    Return next interface statistics found in IP stack.

    \param version (IN) - version to use as input key.
    \param vidx (IN) - vlan index to use as input key.

    \param entry (OUT) - statistics of the matched IPv4 or IPv6 interfrace.

    \return VTSS_OK iff entry is found.
 */
vtss_rc vtss_ip2_cntr_intf_stat_iter_next(  const vtss_ip_type_t        *version,
                                            const vtss_if_id_vlan_t     *vidx,
                                            vtss_if_status_ip_stat_t    *const entry)
{
    vtss_rc                 ret;
    vtss_if_status_t        *all_ifs;
    vtss_if_status_type_t   get_type;
    vtss_if_id_vlan_t       get_vidx;
    u32                     cnt_ifs;

    if (!version || !vidx || !entry) {
        return VTSS_RC_ERROR;
    }

    get_type = VTSS_IF_STATUS_TYPE_ANY;
    get_vidx = VTSS_VID_NULL;
    if (*version < VTSS_IP_TYPE_IPV4) {
        get_type = VTSS_IF_STATUS_TYPE_STAT_IPV4;
    } else {
        if (*version < VTSS_IP_TYPE_IPV6) {
            get_type = VTSS_IF_STATUS_TYPE_STAT_IPV4;
            if (*vidx <= VLAN_ID_MAX) {
                get_vidx = *vidx;
            } else {
                get_type = VTSS_IF_STATUS_TYPE_STAT_IPV6;
            }
        } else if (*version == VTSS_IP_TYPE_IPV6) {
            get_type = VTSS_IF_STATUS_TYPE_STAT_IPV6;
            if (*vidx <= VLAN_ID_MAX) {
                get_vidx = *vidx;
            } else {
                return VTSS_RC_ERROR;
            }
        } else {
            return VTSS_RC_ERROR;
        }
    }

    all_ifs = VTSS_CALLOC(IP2_ITER_MAX_IFS_OBJS, sizeof(vtss_if_status_t));
    if (all_ifs == NULL) {
        return IP2_ERROR_NOSPACE;
    }

    cnt_ifs = 0;
    memset(all_ifs, 0x0, sizeof(*all_ifs));
    if (IP2_ITER_IFS_GET(get_type, IP2_ITER_MAX_IFS_OBJS, &cnt_ifs, all_ifs, ret)) {
        if (!_vtss_ip2_cntr_intf_stat_iter_next(get_vidx, cnt_ifs, all_ifs, entry)) {
            if (get_type == VTSS_IF_STATUS_TYPE_STAT_IPV6) {
                ret = IP2_ERROR_NOTFOUND;
            } else {
                cnt_ifs = 0;
                memset(all_ifs, 0x0, sizeof(*all_ifs));
                if (IP2_ITER_IFS_GET(VTSS_IF_STATUS_TYPE_STAT_IPV6, IP2_ITER_MAX_IFS_OBJS, &cnt_ifs, all_ifs, ret)) {
                    if (!_vtss_ip2_cntr_intf_stat_iter_next(VTSS_VID_NULL, cnt_ifs, all_ifs, entry)) {
                        ret = IP2_ERROR_NOTFOUND;
                    }
                }
            }
        }
    }

    VTSS_FREE(all_ifs);
    return ret;
}

/*
    Return first ICMP statistics found in IP stack.

    \param version (IN) - version to use as input key.

    \param entry (OUT) - statistics of either ICMP4 or ICMP6.

    \return VTSS_OK iff entry is found.
 */
vtss_rc vtss_ip2_cntr_icmp_ver_iter_first(  const vtss_ip_type_t        *version,
                                            vtss_ips_icmp_stat_t        *const entry)
{
    vtss_rc             ret;
    vtss_ips_status_t   *fdx, all_ips[IP2_ITER_MAX_IPS_OBJS];
    u32                 cnt_ips;

    if (!version || !entry) {
        return VTSS_RC_ERROR;
    }

    memset(all_ips, 0x0, sizeof(all_ips));
    if (IP2_ITER_IPS_GET(VTSS_IPS_STATUS_TYPE_ANY, IP2_ITER_MAX_IPS_OBJS, &cnt_ips, all_ips, ret)) {
        u32 idx;

        ret = IP2_ERROR_NOTFOUND;
        for (idx = 0; idx < cnt_ips; idx++) {
            fdx = &all_ips[idx];

            if ((fdx->type == VTSS_IPS_STATUS_TYPE_STAT_ICMP4) ||
                (fdx->type == VTSS_IPS_STATUS_TYPE_STAT_ICMP6)) {
                memcpy(entry, &fdx->u.icmp_stat, sizeof(vtss_ips_icmp_stat_t));
                ret = VTSS_OK;
                break;
            }
        }
    }

    return ret;
}

/*
    Return specific ICMP statistics found in IP stack.

    \param version (IN) - version to use as input key.

    \param entry (OUT) - statistics of either ICMP4 or ICMP6.

    \return VTSS_OK iff entry is found.
 */
vtss_rc vtss_ip2_cntr_icmp_ver_iter_get(    const vtss_ip_type_t        *version,
                                            vtss_ips_icmp_stat_t        *const entry)
{
    vtss_rc                 ret;
    vtss_ips_status_t       *fdx, all_ips[IP2_ITER_SINGLE_IPS_OBJS];
    vtss_ips_status_type_t  get_type;
    u32                     cnt_ips;

    if (!version || !entry) {
        return VTSS_RC_ERROR;
    }

    get_type = VTSS_IPS_STATUS_TYPE_ANY;
    if (*version == VTSS_IP_TYPE_IPV4) {
        get_type = VTSS_IPS_STATUS_TYPE_STAT_ICMP4;
    } else if (*version == VTSS_IP_TYPE_IPV6) {
        get_type = VTSS_IPS_STATUS_TYPE_STAT_ICMP6;
    } else {
        return VTSS_RC_ERROR;
    }

    memset(all_ips, 0x0, sizeof(all_ips));
    if (IP2_ITER_IPS_GET(get_type, IP2_ITER_SINGLE_IPS_OBJS, &cnt_ips, all_ips, ret)) {
        u32 idx;

        ret = IP2_ERROR_NOTFOUND;
        for (idx = 0; idx < cnt_ips; idx++) {
            fdx = &all_ips[idx];

            if ((fdx->type == VTSS_IPS_STATUS_TYPE_STAT_ICMP4) &&
                (*version == VTSS_IP_TYPE_IPV4)) {
                memcpy(entry, &fdx->u.icmp_stat, sizeof(vtss_ips_icmp_stat_t));
                ret = VTSS_OK;
                break;
            }
            if ((fdx->type == VTSS_IPS_STATUS_TYPE_STAT_ICMP6) &&
                (*version == VTSS_IP_TYPE_IPV6)) {
                memcpy(entry, &fdx->u.icmp_stat, sizeof(vtss_ips_icmp_stat_t));
                ret = VTSS_OK;
                break;
            }
        }
    }

    return ret;
}

/*
    Return next ICMP statistics found in IP stack.

    \param version (IN) - version to use as input key.

    \param entry (OUT) - statistics of either ICMP4 or ICMP6.

    \return VTSS_OK iff entry is found.
 */
vtss_rc vtss_ip2_cntr_icmp_ver_iter_next(   const vtss_ip_type_t        *version,
                                            vtss_ips_icmp_stat_t        *const entry)
{
    vtss_rc                 ret;
    vtss_ips_status_t       *fdx, all_ips[IP2_ITER_SINGLE_IPS_OBJS];
    vtss_ips_status_type_t  get_type;
    u32                     cnt_ips;

    if (!version || !entry) {
        return VTSS_RC_ERROR;
    }

    get_type = VTSS_IPS_STATUS_TYPE_ANY;
    if (*version < VTSS_IP_TYPE_IPV4) {
        get_type = VTSS_IPS_STATUS_TYPE_STAT_ICMP4;
    } else {
        if (*version < VTSS_IP_TYPE_IPV6) {
            get_type = VTSS_IPS_STATUS_TYPE_STAT_ICMP6;
        } else {
            return VTSS_RC_ERROR;
        }
    }

    memset(all_ips, 0x0, sizeof(all_ips));
    if (IP2_ITER_IPS_GET(get_type, IP2_ITER_SINGLE_IPS_OBJS, &cnt_ips, all_ips, ret)) {
        u32 idx;

        ret = IP2_ERROR_NOTFOUND;
        for (idx = 0; idx < cnt_ips; idx++) {
            fdx = &all_ips[idx];

            if ((fdx->type == VTSS_IPS_STATUS_TYPE_STAT_ICMP4) ||
                (fdx->type == VTSS_IPS_STATUS_TYPE_STAT_ICMP6)) {
                memcpy(entry, &fdx->u.icmp_stat, sizeof(vtss_ips_icmp_stat_t));
                ret = VTSS_OK;
                break;
            }
        }
    }

    return ret;
}

/*
    Return first ICMP MSG statistics found in IP stack.

    \param version (IN) - version to use as input key.
    \param message (IN) - message type to use as input key.

    \param entry (OUT) - statistics of matched ICMP4 or ICMP6 message.

    \return VTSS_OK iff entry is found.
 */
vtss_rc vtss_ip2_cntr_icmp_msg_iter_first(  const vtss_ip_type_t        *version,
                                            const u32                   *message,
                                            vtss_ips_icmp_stat_t        *const entry)
{
    if (!version || !message || !entry) {
        return VTSS_RC_ERROR;
    }

    return vtss_ip2_stat_imsg_cntr_getfirst(VTSS_IP_TYPE_NONE, IP2_STAT_IMSG_MAX, entry);
}

/*
    Return specific ICMP MSG statistics found in IP stack.

    \param version (IN) - version to use as input key.
    \param message (IN) - message type to use as input key.

    \param entry (OUT) - statistics of matched ICMP4 or ICMP6 message.

    \return VTSS_OK iff entry is found.
 */
vtss_rc vtss_ip2_cntr_icmp_msg_iter_get(    const vtss_ip_type_t        *version,
                                            const u32                   *message,
                                            vtss_ips_icmp_stat_t        *const entry)
{
    if (!version || !message || !entry) {
        return VTSS_RC_ERROR;
    }

    return vtss_ip2_stat_imsg_cntr_get(*version, *message, entry);
}

/*
    Return next ICMP MSG statistics found in IP stack.

    \param version (IN) - version to use as input key.
    \param message (IN) - message type to use as input key.

    \param entry (OUT) - statistics of matched ICMP4 or ICMP6 message.

    \return VTSS_OK iff entry is found.
 */
vtss_rc vtss_ip2_cntr_icmp_msg_iter_next(   const vtss_ip_type_t        *version,
                                            const u32                   *message,
                                            vtss_ips_icmp_stat_t        *const entry)
{
    vtss_ip_type_t  get_vers;
    u32             get_imsg;

    if (!version || !message || !entry) {
        return VTSS_RC_ERROR;
    }

    get_vers = VTSS_IP_TYPE_NONE;
    get_imsg = IP2_STAT_IMSG_MAX;
    if (*version < VTSS_IP_TYPE_IPV4) {
        get_vers = VTSS_IP_TYPE_IPV4;
        get_imsg = 0;

        return vtss_ip2_stat_imsg_cntr_get(get_vers, get_imsg, entry);
    } else {
        get_vers = *version;
        if (*version < VTSS_IP_TYPE_IPV6) {
            if (*message < IP2_STAT_IMSG_MAX) {
                get_imsg = *message;
            } else {
                get_vers = VTSS_IP_TYPE_IPV6;
                get_imsg = 0;

                return vtss_ip2_stat_imsg_cntr_get(get_vers, get_imsg, entry);
            }
        } else if (*version == VTSS_IP_TYPE_IPV6) {
            if (*message < IP2_STAT_IMSG_MAX) {
                get_imsg = *message;
            } else {
                return VTSS_RC_ERROR;
            }
        } else {
            return VTSS_RC_ERROR;
        }
    }

    return vtss_ip2_stat_imsg_cntr_getnext(get_vers, get_imsg, entry);
}

static BOOL _vtss_ip2_iter_intf_nbr_prepare(    vtss_neighbour_status_t *const nbr,
                                                ip2_iter_intf_nbr_t *const ifnbr)
{
    vtss_timestamp_t    nbr_ts;

    if (!nbr || !ifnbr) {
        return FALSE;
    }

    switch ( nbr->ip_address.type ) {
    case VTSS_IP_TYPE_IPV4:
        IP2_ITER_INTF_NBR_IPV4_ADDR_SET(ifnbr, nbr->ip_address.addr.ipv4);

        IP2_ITER_INTF_NBR_STATE_SET(ifnbr, IP2_IFNBR_STATE_REACHABLE);
        if (nbr->flags & VTSS_NEIGHBOUR_FLAG_VALID) {
            if (nbr->flags & VTSS_NEIGHBOUR_FLAG_PERMANENT) {
                IP2_ITER_INTF_NBR_TYPE_SET(ifnbr, IP2_IFNBR_TYPE_STATIC);
            } else {
                IP2_ITER_INTF_NBR_TYPE_SET(ifnbr, IP2_IFNBR_TYPE_OTHER);
            }
        } else {
            IP2_ITER_INTF_NBR_TYPE_SET(ifnbr, IP2_IFNBR_TYPE_INVALID);
            if (nbr->interface.type != VTSS_ID_IF_TYPE_OS_ONLY) {
                IP2_ITER_INTF_NBR_STATE_SET(ifnbr, IP2_IFNBR_STATE_INCOMPLETE);
            }
        }

        break;
    case VTSS_IP_TYPE_IPV6:
        IP2_ITER_INTF_NBR_IPV6_ADDR_SET(ifnbr, &nbr->ip_address.addr.ipv6);

        if (!strncmp(nbr->state, "NONE", 4)) {
            IP2_ITER_INTF_NBR_STATE_SET(ifnbr, IP2_IFNBR_STATE_UNKNOWN);
        } else if (!strncmp(nbr->state, "INCMP", 5)) {
            IP2_ITER_INTF_NBR_STATE_SET(ifnbr, IP2_IFNBR_STATE_INCOMPLETE);
        } else if (!strncmp(nbr->state, "REACH", 5)) {
            IP2_ITER_INTF_NBR_STATE_SET(ifnbr, IP2_IFNBR_STATE_REACHABLE);
        } else if (!strncmp(nbr->state, "STALE", 5)) {
            IP2_ITER_INTF_NBR_STATE_SET(ifnbr, IP2_IFNBR_STATE_STALE);
        } else if (!strncmp(nbr->state, "DELAY", 5)) {
            IP2_ITER_INTF_NBR_STATE_SET(ifnbr, IP2_IFNBR_STATE_DELAY);
        } else if (!strncmp(nbr->state, "PROBE", 5)) {
            IP2_ITER_INTF_NBR_STATE_SET(ifnbr, IP2_IFNBR_STATE_PROBE);
        } else {
            IP2_ITER_INTF_NBR_STATE_SET(ifnbr, IP2_IFNBR_STATE_INVALID);
        }

        if (nbr->flags & VTSS_NEIGHBOUR_FLAG_VALID) {
            if (nbr->flags & VTSS_NEIGHBOUR_FLAG_PERMANENT) {
                IP2_ITER_INTF_NBR_TYPE_SET(ifnbr, IP2_IFNBR_TYPE_STATIC);
            } else {
                IP2_ITER_INTF_NBR_TYPE_SET(ifnbr, IP2_IFNBR_TYPE_OTHER);
            }
        } else {
            IP2_ITER_INTF_NBR_TYPE_SET(ifnbr, IP2_IFNBR_TYPE_INVALID);
        }
        break;
    default:
        return FALSE;
    }

    memset(&nbr_ts, 0x0, sizeof(vtss_timestamp_t));
    IP2_ITER_INTF_NBR_LAST_UPDATE_SET(ifnbr, &nbr_ts);
    IP2_ITER_INTF_NBR_ROW_STATUS_SET(ifnbr, IP2_ITER_STATE_ENABLED);
    IP2_ITER_INTF_NBR_PHY_ADDR_SET(ifnbr, &nbr->mac_address);

    memset(ifnbr->if_name, 0x0, sizeof(ifnbr->if_name));
    if (nbr->interface.type != VTSS_ID_IF_TYPE_VLAN) {
        if (nbr->interface.type != VTSS_ID_IF_TYPE_OS_ONLY) {
            strcpy(ifnbr->if_name, "invalid");
        } else {
            strcpy(ifnbr->if_name, nbr->interface.u.os.name);
        }

        IP2_ITER_INTF_NBR_IFIDX_SET(ifnbr, VTSS_VID_NULL);
    } else {
        IP2_ITER_INTF_NBR_IFIDX_SET(ifnbr, nbr->interface.u.vlan);
    }

    return TRUE;
}

static void _vtss_ip2_iter_intf_nbr_selection(  BOOL exact,
                                                BOOL *const hit,
                                                ip2_iter_intf_nbr_t *const nbx,
                                                ip2_iter_intf_nbr_t *const fnd,
                                                ip2_iter_intf_nbr_t *const res)
{
    if (hit && nbx && fnd && res) {
        if (exact) {
            if (_vtss_ip2_iter_intf_neighbor_cmp_func((void *)fnd, (void *)nbx) == 0) {
                memcpy(res, fnd, sizeof(ip2_iter_intf_nbr_t));
                *hit = TRUE;
            }
        } else {
            if (_vtss_ip2_iter_intf_neighbor_cmp_func((void *)fnd, (void *)nbx) > 0) {
                if (*hit == FALSE) {
                    memcpy(res, fnd, sizeof(ip2_iter_intf_nbr_t));
                    *hit = TRUE;
                } else {
                    if (_vtss_ip2_iter_intf_neighbor_cmp_func((void *)fnd, (void *)res) < 0) {
                        memcpy(res, fnd, sizeof(ip2_iter_intf_nbr_t));
                    }
                }
            }
        }
    }
}

/*
    Return first interface neighbor information found in IP stack.

    \param vidx (IN) - vlan index to use as input key.
    \param nbra (IN) - neighbor address to use as input key.

    \param entry (OUT) - general information of the matched IPv4 or IPv6 neighbor.

    \return VTSS_OK iff entry is found.
 */
vtss_rc vtss_ip2_intf_nbr_iter_first(   const vtss_if_id_vlan_t     vidx,
                                        const vtss_ip_addr_t       *nbra,
                                        ip2_iter_intf_nbr_t        *const entry)
{
    vtss_rc                 ret;
    BOOL                    hit;
    u32                     idx, cnt;
    vtss_neighbour_status_t *nbr, *ndx;
    ip2_iter_intf_nbr_t     *nbx, *fnd, *res;

    if (!entry) {
        return VTSS_RC_ERROR;
    }
    if ((nbr = VTSS_CALLOC(IP2_ITER_MAX_NBR_OBJS, sizeof(vtss_neighbour_status_t))) == NULL) {
        return IP2_ERROR_NOSPACE;
    }
    if ((nbx = VTSS_MALLOC(sizeof(ip2_iter_intf_nbr_t))) == NULL) {
        VTSS_FREE(nbr);
        return IP2_ERROR_NOSPACE;
    }
    if ((fnd = VTSS_MALLOC(sizeof(ip2_iter_intf_nbr_t))) == NULL) {
        VTSS_FREE(nbx);
        VTSS_FREE(nbr);
        return IP2_ERROR_NOSPACE;
    }
    if ((res = VTSS_MALLOC(sizeof(ip2_iter_intf_nbr_t))) == NULL) {
        VTSS_FREE(fnd);
        VTSS_FREE(nbx);
        VTSS_FREE(nbr);
        return IP2_ERROR_NOSPACE;
    }

    ndx = nbr;
    memset(nbx, 0x0, sizeof(ip2_iter_intf_nbr_t));

    hit = FALSE;
    memset(res, 0x0, sizeof(ip2_iter_intf_nbr_t));
    if (IP2_ITER_NBR_GET(VTSS_IP_TYPE_IPV4, IP2_ITER_MAX_NBR_OBJS, &cnt, nbr, ret)) {
        for (idx = 0; idx < cnt; ++idx, ++nbr) {
            if (_vtss_ip2_iter_intf_nbr_prepare(nbr, fnd)) {
                _vtss_ip2_iter_intf_nbr_selection(FALSE, &hit, nbx, fnd, res);
            }
        }
    }

    if (!hit || (ret != VTSS_OK)) {
        nbr = ndx;
        if (IP2_ITER_NBR_GET(VTSS_IP_TYPE_IPV6, IP2_ITER_MAX_NBR_OBJS, &cnt, nbr, ret)) {
            for (idx = 0; idx < cnt; ++idx, ++nbr) {
                if (_vtss_ip2_iter_intf_nbr_prepare(nbr, fnd)) {
                    _vtss_ip2_iter_intf_nbr_selection(FALSE, &hit, nbx, fnd, res);
                }
            }
        }
    }

    if (hit && (ret == VTSS_OK)) {
        memcpy(entry, res, sizeof(ip2_iter_intf_nbr_t));
    } else {
        if (ret == VTSS_OK) {
            ret = IP2_ERROR_NOTFOUND;
        }
    }

    VTSS_FREE(res);
    VTSS_FREE(fnd);
    VTSS_FREE(nbx);
    VTSS_FREE(ndx);

    return ret;
}

/*
    Return specific interface neighbor information found in IP stack.

    \param vidx (IN) - vlan index to use as input key.
    \param nbra (IN) - neighbor address to use as input key.

    \param entry (OUT) - general information of the matched IPv4 or IPv6 neighbor.

    \return VTSS_OK iff entry is found.
 */
vtss_rc vtss_ip2_intf_nbr_iter_get(     const vtss_if_id_vlan_t     vidx,
                                        const vtss_ip_addr_t       *nbra,
                                        ip2_iter_intf_nbr_t        *const entry)
{
    vtss_rc                 ret;
    BOOL                    hit;
    u32                     idx, cnt;
    vtss_ip_type_t          stype;
    vtss_neighbour_status_t *nbr;
    ip2_iter_intf_nbr_t     *nbx, *fnd, *res;

    if (!nbra || !entry ||
        ((nbra->type != VTSS_IP_TYPE_IPV4) && (nbra->type != VTSS_IP_TYPE_IPV6))) {
        return VTSS_RC_ERROR;
    }
    if ((nbr = VTSS_CALLOC(IP2_ITER_MAX_NBR_OBJS, sizeof(vtss_neighbour_status_t))) == NULL) {
        return IP2_ERROR_NOSPACE;
    }
    if ((nbx = VTSS_MALLOC(sizeof(ip2_iter_intf_nbr_t))) == NULL) {
        VTSS_FREE(nbr);
        return IP2_ERROR_NOSPACE;
    }
    if ((fnd = VTSS_MALLOC(sizeof(ip2_iter_intf_nbr_t))) == NULL) {
        VTSS_FREE(nbx);
        VTSS_FREE(nbr);
        return IP2_ERROR_NOSPACE;
    }
    if ((res = VTSS_MALLOC(sizeof(ip2_iter_intf_nbr_t))) == NULL) {
        VTSS_FREE(fnd);
        VTSS_FREE(nbx);
        VTSS_FREE(nbr);
        return IP2_ERROR_NOSPACE;
    }

    stype = nbra->type;
    memset(nbx, 0x0, sizeof(ip2_iter_intf_nbr_t));
    IP2_ITER_INTF_NBR_IFIDX_SET(nbx, vidx);
    switch ( stype ) {
    case VTSS_IP_TYPE_IPV4:
        IP2_ITER_INTF_NBR_IPV4_ADDR_SET(nbx, nbra->addr.ipv4);
        break;
    case VTSS_IP_TYPE_IPV6:
        IP2_ITER_INTF_NBR_IPV6_ADDR_SET(nbx, &nbra->addr.ipv6);
        break;
    default:
        break;
    }

    hit = FALSE;
    ret = VTSS_RC_ERROR;
    memset(res, 0x0, sizeof(ip2_iter_intf_nbr_t));
    if ((stype == VTSS_IP_TYPE_IPV4) || (stype == VTSS_IP_TYPE_IPV6)) {
        if (IP2_ITER_NBR_GET(stype, IP2_ITER_MAX_NBR_OBJS, &cnt, nbr, ret)) {
            for (idx = 0; !hit && (idx < cnt); ++idx) {
                if (_vtss_ip2_iter_intf_nbr_prepare(&nbr[idx], fnd)) {
                    _vtss_ip2_iter_intf_nbr_selection(TRUE, &hit, nbx, fnd, res);
                }
            }
        }
    }

    if (hit && (ret == VTSS_OK)) {
        memcpy(entry, res, sizeof(ip2_iter_intf_nbr_t));
    } else {
        if (ret == VTSS_OK) {
            ret = IP2_ERROR_NOTFOUND;
        }
    }

    VTSS_FREE(res);
    VTSS_FREE(fnd);
    VTSS_FREE(nbx);
    VTSS_FREE(nbr);

    return ret;
}

/*
    Return next interface neighbor information found in IP stack.

    \param vidx (IN) - vlan index to use as input key.
    \param nbra (IN) - neighbor address to use as input key.

    \param entry (OUT) - general information of the matched IPv4 or IPv6 neighbor.

    \return VTSS_OK iff entry is found.
 */
vtss_rc vtss_ip2_intf_nbr_iter_next(    const vtss_if_id_vlan_t     vidx,
                                        const vtss_ip_addr_t       *nbra,
                                        ip2_iter_intf_nbr_t        *const entry)
{
    vtss_rc                 ret;
    BOOL                    hit;
    u32                     idx, cnt;
    vtss_neighbour_status_t *nbr, *ndx;
    ip2_iter_intf_nbr_t     *nbx, *fnd, *res;

    if (!nbra || !entry) {
        return VTSS_RC_ERROR;
    }
    if ((nbr = VTSS_CALLOC(IP2_ITER_MAX_NBR_OBJS, sizeof(vtss_neighbour_status_t))) == NULL) {
        return IP2_ERROR_NOSPACE;
    }
    if ((nbx = VTSS_MALLOC(sizeof(ip2_iter_intf_nbr_t))) == NULL) {
        VTSS_FREE(nbr);
        return IP2_ERROR_NOSPACE;
    }
    if ((fnd = VTSS_MALLOC(sizeof(ip2_iter_intf_nbr_t))) == NULL) {
        VTSS_FREE(nbx);
        VTSS_FREE(nbr);
        return IP2_ERROR_NOSPACE;
    }
    if ((res = VTSS_MALLOC(sizeof(ip2_iter_intf_nbr_t))) == NULL) {
        VTSS_FREE(fnd);
        VTSS_FREE(nbx);
        VTSS_FREE(nbr);
        return IP2_ERROR_NOSPACE;
    }

    ndx = nbr;
    memset(nbx, 0x0, sizeof(ip2_iter_intf_nbr_t));
    IP2_ITER_INTF_NBR_IFIDX_SET(nbx, vidx);
    switch ( nbra->type ) {
    case VTSS_IP_TYPE_IPV4:
        IP2_ITER_INTF_NBR_IPV4_ADDR_SET(nbx, nbra->addr.ipv4);
        break;
    case VTSS_IP_TYPE_IPV6:
        IP2_ITER_INTF_NBR_IPV6_ADDR_SET(nbx, &nbra->addr.ipv6);
        break;
    default:
        IP2_ITER_INTF_NBR_VERSION_SET(nbx, VTSS_IP_TYPE_NONE);
        break;
    }

    hit = FALSE;
    ret = VTSS_RC_ERROR;
    memset(res, 0x0, sizeof(ip2_iter_intf_nbr_t));
    if ((nbra->type != VTSS_IP_TYPE_IPV6) &&
        IP2_ITER_NBR_GET(VTSS_IP_TYPE_IPV4, IP2_ITER_MAX_NBR_OBJS, &cnt, nbr, ret)) {
        for (idx = 0; idx < cnt; ++idx, ++nbr) {
            if (_vtss_ip2_iter_intf_nbr_prepare(nbr, fnd)) {
                _vtss_ip2_iter_intf_nbr_selection(FALSE, &hit, nbx, fnd, res);
            }
        }
    }

    if (!hit || (ret != VTSS_OK)) {
        nbr = ndx;
        if (IP2_ITER_NBR_GET(VTSS_IP_TYPE_IPV6, IP2_ITER_MAX_NBR_OBJS, &cnt, nbr, ret)) {
            for (idx = 0; idx < cnt; ++idx, ++nbr) {
                if (_vtss_ip2_iter_intf_nbr_prepare(nbr, fnd)) {
                    _vtss_ip2_iter_intf_nbr_selection(FALSE, &hit, nbx, fnd, res);
                }
            }
        }
    }

    if (hit && (ret == VTSS_OK)) {
        memcpy(entry, res, sizeof(ip2_iter_intf_nbr_t));
    } else {
        if (ret == VTSS_OK) {
            ret = IP2_ERROR_NOTFOUND;
        }
    }

    VTSS_FREE(res);
    VTSS_FREE(fnd);
    VTSS_FREE(nbx);
    VTSS_FREE(ndx);

    return ret;
}
