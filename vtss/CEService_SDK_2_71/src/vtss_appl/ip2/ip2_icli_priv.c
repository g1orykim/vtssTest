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


#include "mgmt_api.h"
#include "misc_api.h"
#include "msg_api.h"
#include "ip2_api.h"
#include "ip2_utils.h"
#include "ip2_iterators.h"
#include "ip2_icli_priv.h"
#include "icli_porting_util.h"

#ifdef VTSS_SW_OPTION_ICFG
#include "icfg_api.h"

#define IP2_ICFG_REG(w, x, y, z)        (((w) = vtss_icfg_query_register((x), (y), (z))) == VTSS_OK)
#define PRINTF(...)                     (void) vtss_icfg_printf(result, __VA_ARGS__);
#endif /* VTSS_SW_OPTION_ICFG */

#define VTSS_TRACE_MODULE_ID            VTSS_MODULE_ID_IP2
#define VTSS_ALLOC_MODULE_ID            VTSS_MODULE_ID_IP2

/*
******************************************************************************

    Static Function

******************************************************************************
*/
#ifdef VTSS_SW_OPTION_ICFG
static vtss_rc IP2_ipv4_icfg_conf(const vtss_icfg_query_request_t *req,
                                  vtss_icfg_query_result_t *result)
{
    switch (req->cmd_mode) {
    case ICLI_CMD_MODE_GLOBAL_CONFIG: {
        vtss_rc rc;
        int cnt, i;
        vtss_ip2_global_param_t conf;
        vtss_routing_entry_t *routes = VTSS_CALLOC(IP2_MAX_ROUTES, sizeof(*routes));

        if (!routes) {
            return VTSS_RC_ERROR;
        }

        rc = vtss_ip2_global_param_get(&conf);
        if (rc != VTSS_OK) {
            goto ROUTE_DONE;
        }

        if (conf.enable_routing) {
            PRINTF("ip routing\n");
        } else if (req->all_defaults) {
            PRINTF("no ip routing\n");
        }

        /*lint --e{429} */
        rc = vtss_ip2_route_conf_get(IP2_MAX_ROUTES, routes, &cnt);
        if (rc != VTSS_OK) {
            goto ROUTE_DONE;
        }

        for (i = 0; i < cnt; i++) {
            vtss_ipv4_t mask;
            vtss_routing_entry_t const *rt = &routes[i];

            (void)
            vtss_conv_prefix_to_ipv4mask(rt->route.ipv4_uc.network.prefix_size,
                                         &mask);
            if (rt->type == VTSS_ROUTING_ENTRY_TYPE_IPV4_UC) {
                PRINTF("ip route "VTSS_IPV4_FORMAT" "VTSS_IPV4_FORMAT
                       " "VTSS_IPV4_FORMAT"\n",
                       VTSS_IPV4_ARGS(rt->route.ipv4_uc.network.address),
                       VTSS_IPV4_ARGS(mask),
                       VTSS_IPV4_ARGS(rt->route.ipv4_uc.destination));
            }
        }

ROUTE_DONE:
        VTSS_FREE(routes);
        if (rc != VTSS_RC_OK) {
            return rc;
        }
        break;
    }

    case ICLI_CMD_MODE_INTERFACE_VLAN: {
        vtss_vid_t vid = (vtss_vid_t)req->instance_id.vlan;
        vtss_ip_conf_t ip_conf;

        if (vtss_ip2_ipv4_conf_get(vid, &ip_conf) == VTSS_RC_OK) {
            vtss_ipv4_t mask;
            (void) vtss_conv_prefix_to_ipv4mask(ip_conf.network.prefix_size,
                                                &mask);
            if (ip_conf.dhcpc) {
                PRINTF(" ip address dhcp");

                if (ip_conf.network.address.type == VTSS_IP_TYPE_IPV4) {
                    PRINTF(" fallback "VTSS_IPV4_FORMAT" "VTSS_IPV4_FORMAT,
                           VTSS_IPV4_ARGS(ip_conf.network.address.addr.ipv4),
                           VTSS_IPV4_ARGS(mask));

                    if (ip_conf.fallback_timeout) {
                        PRINTF(" timeout %d", ip_conf.fallback_timeout);
                    }
                }
                PRINTF("\n");

            } else {
                if (ip_conf.network.address.type == VTSS_IP_TYPE_IPV4) {
                    PRINTF(" ip address "VTSS_IPV4_FORMAT" "VTSS_IPV4_FORMAT"\n",
                           VTSS_IPV4_ARGS(ip_conf.network.address.addr.ipv4),
                           VTSS_IPV4_ARGS(mask));

                } else {
                    PRINTF(" no ip address\n");
                }
            }

        } else {
            PRINTF(" no ip address\n");
        }

        break;
    }

    default:
        //Not needed
        break;
    }

    return VTSS_RC_OK;
}

#ifdef VTSS_SW_OPTION_IPV6
static vtss_rc IP2_ipv6_icfg_interface(const vtss_icfg_query_request_t *req,
                                       vtss_icfg_query_result_t *result)
{
    vtss_rc rc = VTSS_OK;

    if (req && result) {
        vtss_ip_conf_t  ip6_conf;
        vtss_vid_t      vid6 = (vtss_vid_t)req->instance_id.vlan;

        /*
            COMMAND = ipv6 address <ipv6_subnet>
            T.B.D
            COMMAND = ipv6 address autoconfig [default]
            COMMAND = ipv6 mtu <1280-1500>
            COMMAND = ipv6 nd managed-config-flag
            COMMAND = ipv6 nd other-config-flag
            COMMAND = ipv6 nd reachable-time <milliseconds:0-3600000>
            COMMAND = ipv6 nd prefix <ipv6_subnet> valid-lifetime <seconds:0-4294967295>
            COMMAND = ipv6 nd prefix <ipv6_subnet> preferred-lifetime <seconds:0-4294967295>
            COMMAND = ipv6 nd prefix <ipv6_subnet> off-link
            COMMAND = ipv6 nd prefix <ipv6_subnet> no-autoconfig
            COMMAND = ipv6 nd prefix <ipv6_subnet> no-rtr-address
            COMMAND = ipv6 nd ra interval <maximum_secs:4-1800> [<minimum_secs:3-1350>]
            COMMAND = ipv6 nd ra lifetime <seconds:0-9000>

        */

        if (vtss_ip2_ipv6_conf_get(vid6, &ip6_conf) == VTSS_RC_OK) {
            if (req->all_defaults ||
                (ip6_conf.network.address.type == VTSS_IP_TYPE_IPV6)) {
                if (ip6_conf.network.address.type == VTSS_IP_TYPE_IPV6) {
                    PRINTF(" ipv6 address "VTSS_IPV6_FORMAT"/%u\n",
                           VTSS_IPV6_ARGS(ip6_conf.network.address.addr.ipv6),
                           ip6_conf.network.prefix_size);
                } else {
                    PRINTF(" no ipv6 address\n");
                }
            }
        } else {
            if (req->all_defaults) {
                PRINTF(" no ipv6 address\n");
            }
        }
    }

    return rc;
}

static vtss_rc IP2_ipv6_icfg_global(const vtss_icfg_query_request_t *req,
                                    vtss_icfg_query_result_t *result)
{
    vtss_rc rc = VTSS_OK;

    if (req && result) {
        vtss_ip2_global_param_t conf;

        /*
            COMMAND = ipv6 route <ipv6_subnet> { <ipv6_ucast> | interface vlan <vlan_id> <ipv6_addr> }
            T.B.D
            COMMAND = ipv6 unicast-routing
        */
        if ((rc = vtss_ip2_global_param_get(&conf)) == VTSS_OK) {
            vtss_routing_entry_t    *routes;

//            if (req->all_defaults ||
//                (conf.enable_routing != IP2_ICFG_DEF_ROUTING6)) {
//                PRINTF("%sipv6 unicast-routing\n", conf.enable_routing ? "" : "no ");
//            }

            if ((routes = VTSS_CALLOC(IP2_MAX_ROUTES, sizeof(vtss_routing_entry_t))) != NULL) {
                int i, cnt;

                if ((rc = vtss_ip2_route_conf_get(IP2_MAX_ROUTES, routes, &cnt)) == VTSS_OK) {
                    vtss_vid_t              cfg_vid, run_vid;
                    vtss_routing_entry_t    *rt;

                    for (i = 0; i < cnt; i++) {
                        rt = &routes[i];
                        if (rt->type != VTSS_ROUTING_ENTRY_TYPE_IPV6_UC) {
                            continue;
                        }

                        IP2_IPV6_RTNH_LLA_VLAN_GET(cfg_vid, rt);
                        if (cfg_vid) {
                            run_vid = VTSS_VID_NULL;
                            IP2_IPV6_RTNH_LLA_IFID_SET(run_vid, rt);
                            PRINTF("ipv6 route "VTSS_IPV6N_FORMAT" interface vlan %u "VTSS_IPV6_FORMAT"\n",
                                   VTSS_IPV6N_ARG(rt->route.ipv6_uc.network),
                                   cfg_vid,
                                   VTSS_IPV6_ARGS(rt->route.ipv6_uc.destination));
                        } else {
                            PRINTF("ipv6 route "VTSS_IPV6N_FORMAT" "VTSS_IPV6_FORMAT"\n",
                                   VTSS_IPV6N_ARG(rt->route.ipv6_uc.network),
                                   VTSS_IPV6_ARGS(rt->route.ipv6_uc.destination));
                        }
                    }
                }

                VTSS_FREE(routes);
            } else {
                rc = VTSS_RC_ERROR;
            }
        }
    }

    return rc;
}
#endif /* VTSS_SW_OPTION_IPV6 */
#endif /* VTSS_SW_OPTION_ICFG */

/*
******************************************************************************

    Public functions

******************************************************************************
*/
vtss_rc vtss_ip2_ipv4_icfg_init(void)
{
    vtss_rc ip2_icfg_rc = VTSS_RC_OK;

#ifdef VTSS_SW_OPTION_ICFG
    VTSS_RC(vtss_icfg_query_register(VTSS_ICFG_IPV4_GLOBAL, "ipv4",
                                     IP2_ipv4_icfg_conf));
    VTSS_RC(vtss_icfg_query_register(VTSS_ICFG_IPV4_INTERFACE, "ipv4",
                                     IP2_ipv4_icfg_conf));

#ifdef VTSS_SW_OPTION_IPV6
    if (IP2_ICFG_REG(ip2_icfg_rc, VTSS_ICFG_IPV6_GLOBAL, "ipv6", IP2_ipv6_icfg_global)) {
        if (IP2_ICFG_REG(ip2_icfg_rc, VTSS_ICFG_IPV6_INTERFACE, "ipv6", IP2_ipv6_icfg_interface)) {
            T_I("IP2(IPv6) ICFG done");
        }
    }
#endif /* VTSS_SW_OPTION_IPV6 */
#endif

    return ip2_icfg_rc;
}

vtss_rc get_vlan_if(vtss_vid_t vid)
{
    vtss_if_param_t param;
    if (vtss_ip2_if_exists(vid)) {
        return VTSS_OK;
    }
    vtss_if_default_param(&param);
    return vtss_ip2_if_conf_set(vid, &param);
}

void icli_ip2_intf_status_display(i32 session_id,
                                  BOOL brief,
                                  void *intf_status)
{
    vtss_rc                 rc;
    u8                      do_prt;
    ip2_iter_intf_ifinf_t   *entry;
    vtss_ip_type_t          version;
    vtss_ip_conf_t          ip_config;
    ip2_iter_intf_ifadr_t   intf_addr;
    i8                      adrString[40];

    if (!intf_status) {
        return;
    }

    memset(&ip_config, 0x0, sizeof(vtss_ip_conf_t));
    entry = (ip2_iter_intf_ifinf_t *)intf_status;
    rc = VTSS_RC_ERROR;
    version = IP2_ITER_INTF_IFINFO_VERSION(entry);
    switch ( version ) {
    case VTSS_IP_TYPE_IPV4:
        rc = vtss_ip2_ipv4_conf_get(IP2_ITER_INTF_IFINFO_IFVDX(entry), &ip_config);
        ICLI_PRINTF("\n\rIPv4 Vlan%u interface is %s.\n\r",
                    IP2_ITER_INTF_IFINFO_IFVDX(entry),
                    IP2_ITER_IFINFST_OPR_ACT(IP2_ITER_INTF_IFINFO_MGMT_STATE(entry)) ? "up" : "down");
        VTSS_IP2_ITER_INTF4_ADDR_FIRST(rc, &intf_addr);
        break;
    case VTSS_IP_TYPE_IPV6:
        rc = vtss_ip2_ipv6_conf_get(IP2_ITER_INTF_IFINFO_IFVDX(entry), &ip_config);
        ICLI_PRINTF("\n\rIPv6 Vlan%u interface is %s.\n\r",
                    IP2_ITER_INTF_IFINFO_IFVDX(entry),
                    IP2_ITER_IFINFST_OPR_ACT(IP2_ITER_INTF_IFINFO_MGMT_STATE(entry)) ? "up" : "down");
        VTSS_IP2_ITER_INTF6_ADDR_FIRST(rc, &intf_addr);
        break;
    default:
        return;
    }

    do_prt = 0;
    if (rc == VTSS_OK) {
        do {
            if (IP2_ITER_INTF_IFADR_IFINDEX(&intf_addr) == IP2_ITER_INTF_IFINFO_IFINDEX(entry)) {
                memset(adrString, 0x0, sizeof(adrString));
                if (version == VTSS_IP_TYPE_IPV6) {
                    vtss_ipv6_t adr6;

                    memcpy(&adr6, &intf_addr.ipa.addr.ipv6, sizeof(vtss_ipv6_t));
                    if (vtss_ipv6_addr_is_link_local(&adr6)) {
                        adr6.addr[2] = adr6.addr[3] = 0x0;
                    }
                    (void) icli_ipv6_to_str(adr6, adrString);
                } else {
                    (void) icli_ipv4_to_str(intf_addr.ipa.addr.ipv4, adrString);
                }

                ICLI_PRINTF("  Internet address is %s\n\r", adrString);
                do_prt++;
            }

            if (version == VTSS_IP_TYPE_IPV6) {
                VTSS_IP2_ITER_INTF6_ADDR_NEXT(rc, &intf_addr.ipa.addr.ipv6, &intf_addr);
            } else {
                VTSS_IP2_ITER_INTF4_ADDR_NEXT(rc, intf_addr.ipa.addr.ipv4, &intf_addr);
            }
        } while (rc == VTSS_OK);
    }

    if (!do_prt) {
        ICLI_PRINTF("  Internet address is not available\n\r");
    }

    if (vtss_ip_addr_is_zero(&ip_config.network.address)) {
        ICLI_PRINTF("  Static address is not set\n\r");
    } else {
        memset(adrString, 0x0, sizeof(adrString));
        if (version == VTSS_IP_TYPE_IPV6) {
            (void) icli_ipv6_to_str(ip_config.network.address.addr.ipv6, adrString);
        } else {
            (void) icli_ipv4_to_str(ip_config.network.address.addr.ipv4, adrString);
        }

        ICLI_PRINTF("  Static address is %s/%u\n\r",
                    adrString, ip_config.network.prefix_size);
    }

    if (!brief) {
        ICLI_PRINTF("  IP stack index (IFID) is %u\n\r",
                    IP2_ITER_INTF_IFINFO_IFINDEX(entry));
        ICLI_PRINTF("  Routing is %s on this interface\n\r",
                    IP2_ITER_INTF_IFINFO_FORWARDING(entry) == IP2_ITER_DO_FORWARDING ? "enabled" : "disabled");
        ICLI_PRINTF("  MTU is %u bytes\n\r",
                    IP2_ITER_INTF_IFINFO_MTU(entry));
    }
}

void icli_ip2_intf_neighbor_display(i32 session_id,
                                    vtss_ip_type_t version,
                                    BOOL by_vlan,
                                    vtss_vid_t vid)
{
    vtss_if_id_vlan_t   vidx;
    vtss_ip_addr_t      nbra;
    ip2_iter_intf_nbr_t entry;
    i8                  adrString[40];

    vidx = VTSS_VID_NULL;
    if (by_vlan) {
        vidx = vid;
    }
    memset(&nbra, 0x0, sizeof(vtss_ip_addr_t));
    nbra.type = version;
    while (vtss_ip2_intf_nbr_iter_next(vidx, &nbra, &entry) == VTSS_OK) {
        vidx = IP2_ITER_INTF_NBR_IFIDX(&entry);
        if (vid != VTSS_VID_NULL) {
            if (vidx != vid) {
                break;
            }
        }
        if (version != VTSS_IP_TYPE_NONE) {
            if (IP2_ITER_INTF_NBR_VERSION(&entry) != version) {
                break;
            }
        }
        memcpy(&nbra, &entry.nbr, sizeof(vtss_ip_addr_t));

        memset(adrString, 0x0, sizeof(adrString));
        if (version == VTSS_IP_TYPE_IPV6) {
            vtss_ipv6_t adr6;

            memcpy(&adr6, &entry.nbr.addr.ipv6, sizeof(vtss_ipv6_t));
            if (vtss_ipv6_addr_is_link_local(&adr6)) {
                adr6.addr[2] = adr6.addr[3] = 0x0;
            }
            (void) icli_ipv6_to_str(adr6, adrString);
        } else {
            (void) icli_ipv4_to_str(entry.nbr.addr.ipv4, adrString);
        }

        if (IP2_ITER_INTF_NBR_IFIDX(&entry) != VTSS_VID_NULL) {
            ICLI_PRINTF("\n\r%s via VLAN%u:",
                        adrString, vidx);
        } else {
            ICLI_PRINTF("\n\r%s via OS:%s",
                        adrString, entry.if_name);
        }
        ICLI_PRINTF(" %02x-%02x-%02x-%02x-%02x-%02x", VTSS_MAC_ARGS(entry.nbr_phy_address));

        switch ( IP2_ITER_INTF_NBR_TYPE(&entry) ) {
        case IP2_IFNBR_TYPE_STATIC:
            ICLI_PRINTF(" Permanent");
            break;
        case IP2_IFNBR_TYPE_DYNAMIC:
        case IP2_IFNBR_TYPE_OTHER:
            ICLI_PRINTF(" Dynamic");
            break;
        case IP2_IFNBR_TYPE_LOCAL:
            ICLI_PRINTF(" Local");
            break;
        case IP2_IFNBR_TYPE_INVALID:
        default:
            ICLI_PRINTF(" Invalid");
            break;
        }

        switch ( IP2_ITER_INTF_NBR_STATE(&entry) ) {
        case IP2_IFNBR_STATE_REACHABLE:
            ICLI_PRINTF("/REACHABLE");
            break;
        case IP2_IFNBR_STATE_STALE:
            ICLI_PRINTF("/STALE");
            break;
        case IP2_IFNBR_STATE_DELAY:
            ICLI_PRINTF("/DELAY");
            break;
        case IP2_IFNBR_STATE_PROBE:
            ICLI_PRINTF("/PROBE");
            break;
        case IP2_IFNBR_STATE_INVALID:
            ICLI_PRINTF("/INVALID");
            break;
        case IP2_IFNBR_STATE_INCOMPLETE:
            ICLI_PRINTF("/INCOMPLETE");
            break;
        case IP2_IFNBR_STATE_UNKNOWN:
        default:
            ICLI_PRINTF("/NONE");
            break;
        }

        if (icli_session_printf(session_id, "%s", "") == ICLI_RC_ERR_BYPASS) {
            break;
        }
    }

    ICLI_PRINTF("\n\n\r");
}

void icli_ip2_stat_ip_syst_display(i32 session_id,
                                   vtss_ips_ip_stat_t *entry)
{
    vtss_ip_stat_data_t *stat;

    if (!entry) {
        return;
    }

    stat = &entry->data;
    stat = &entry->data;
    switch ( entry->IPVersion ) {
    case VTSS_IP_TYPE_IPV4:
        ICLI_PRINTF("\n\rIPv4 statistics:\n\r");
        break;
    case VTSS_IP_TYPE_IPV6:
        ICLI_PRINTF("\n\rIPv6 statistics:\n\r");
        break;
    case VTSS_IP_TYPE_NONE:
    default:
        ICLI_PRINTF("\n\rUnknown statistics:\n\r");
        break;
    }

    ICLI_PRINTF("\n\r  Rcvd:  %llu total in %llu byte%s",
                stat->HCInReceives,
                stat->HCInOctets,
                (stat->HCInOctets > 1) ? "s" : "");
    ICLI_PRINTF("\n\r         %llu local destination, %llu forwarding",
                stat->HCInDelivers,
                stat->HCInForwDatagrams);
    ICLI_PRINTF("\n\r         %u header error, %u address error, %u unknown protocol",
                stat->InHdrErrors,
                stat->InAddrErrors,
                stat->InUnknownProtos);
    ICLI_PRINTF("\n\r         %u no route, %u truncated, %u discarded",
                stat->InNoRoutes,
                stat->InTruncatedPkts,
                stat->InDiscards);

    ICLI_PRINTF("\n\r  Sent:  %llu total in %llu byte%s",
                stat->HCOutTransmits,
                stat->HCOutOctets,
                (stat->HCOutOctets > 1) ? "s" : "");
    ICLI_PRINTF("\n\r         %llu generated, %llu forwarded",
                stat->HCOutRequests,
                stat->HCOutForwDatagrams);
    ICLI_PRINTF("\n\r         %u no route, %u discarded",
                stat->OutNoRoutes,
                stat->OutDiscards);

    ICLI_PRINTF("\n\r  Frags: %u reassemble (%u reassembled, %u couldn't reassemble)",
                stat->ReasmReqds,
                stat->ReasmOKs,
                stat->ReasmFails);
    ICLI_PRINTF("\n\r         %u fragment (%u fragmented, %u couldn't fragment)",
                stat->OutFragReqds,
                stat->OutFragOKs,
                stat->OutFragFails);
    ICLI_PRINTF("\n\r         %u fragment%s created",
                stat->OutFragCreates,
                (stat->OutFragCreates > 1) ? "s" : "");

    ICLI_PRINTF("\n\r  Mcast: %llu received in %llu byte%s",
                stat->HCInMcastPkts,
                stat->HCInMcastOctets,
                (stat->HCInMcastOctets > 1) ? "s" : "");
    ICLI_PRINTF("\n\r         %llu sent in %llu byte%s",
                stat->HCOutMcastPkts,
                stat->HCOutMcastOctets,
                (stat->HCOutMcastOctets > 1) ? "s" : "");

    ICLI_PRINTF("\n\r  Bcast: %llu received, %llu sent",
                stat->HCInBcastPkts,
                stat->HCOutBcastPkts);

    ICLI_PRINTF("\n\r");
//    ICLI_PRINTF("\n\r  Timestamp: (%u)%usecs%uns (Refresh every %u msecs)\n\r",
//                stat->DiscontinuityTime.sec_msb,
//                stat->DiscontinuityTime.seconds,
//                stat->DiscontinuityTime.nanoseconds,
//                stat->RefreshRate);
}

void icli_ip2_stat_ip_intf_display(i32 session_id,
                                   BOOL *first_pr,
                                   vtss_if_status_ip_stat_t *entry)
{
    vtss_ip_stat_data_t *stat;

    if (!entry) {
        return;
    }

    if (*first_pr) {
        ICLI_PRINTF("\n\rIP interface statistics:\n\r");
        *first_pr = FALSE;
    }

    stat = &entry->data;
    switch ( entry->IPVersion ) {
    case VTSS_IP_TYPE_IPV4:
        ICLI_PRINTF("\n\r  IPv4 Statistics on Interface VLAN: %u", entry->IfIndex.u.vlan);
        break;
    case VTSS_IP_TYPE_IPV6:
        ICLI_PRINTF("\n\r  IPv6 Statistics on Interface VLAN: %u", entry->IfIndex.u.vlan);
        break;
    case VTSS_IP_TYPE_NONE:
    default:
        ICLI_PRINTF("\n\r  Unknown Statistics on Interface VLAN: %u", entry->IfIndex.u.vlan);
        break;
    }

    ICLI_PRINTF("\n\r  Rcvd:  %llu total in %llu byte%s",
                stat->HCInReceives,
                stat->HCInOctets,
                (stat->HCInOctets > 1) ? "s" : "");
    ICLI_PRINTF("\n\r         %llu local destination, %llu forwarding",
                stat->HCInDelivers,
                stat->HCInForwDatagrams);
    ICLI_PRINTF("\n\r         %u header error, %u address error, %u unknown protocol",
                stat->InHdrErrors,
                stat->InAddrErrors,
                stat->InUnknownProtos);
    ICLI_PRINTF("\n\r         %u no route, %u truncated, %u discarded",
                stat->InNoRoutes,
                stat->InTruncatedPkts,
                stat->InDiscards);

    ICLI_PRINTF("\n\r  Sent:  %llu total in %llu byte%s",
                stat->HCOutTransmits,
                stat->HCOutOctets,
                (stat->HCOutOctets > 1) ? "s" : "");
    ICLI_PRINTF("\n\r         %llu generated, %llu forwarded",
                stat->HCOutRequests,
                stat->HCOutForwDatagrams);
    ICLI_PRINTF("\n\r         %u discarded",
                stat->OutDiscards);

    ICLI_PRINTF("\n\r  Frags: %u reassemble (%u reassembled, %u couldn't reassemble)",
                stat->ReasmReqds,
                stat->ReasmOKs,
                stat->ReasmFails);
    ICLI_PRINTF("\n\r         %u fragment (%u fragmented, %u couldn't fragment)",
                stat->OutFragReqds,
                stat->OutFragOKs,
                stat->OutFragFails);
    ICLI_PRINTF("\n\r         %u fragment%s created",
                stat->OutFragCreates,
                (stat->OutFragCreates > 1) ? "s" : "");

    ICLI_PRINTF("\n\r  Mcast: %llu received in %llu byte%s",
                stat->HCInMcastPkts,
                stat->HCInMcastOctets,
                (stat->HCInMcastOctets > 1) ? "s" : "");
    ICLI_PRINTF("\n\r         %llu sent in %llu byte%s",
                stat->HCOutMcastPkts,
                stat->HCOutMcastOctets,
                (stat->HCOutMcastOctets > 1) ? "s" : "");

    ICLI_PRINTF("\n\r  Bcast: %llu received, %llu sent",
                stat->HCInBcastPkts,
                stat->HCOutBcastPkts);

    ICLI_PRINTF("\n\r");
//    ICLI_PRINTF("\n\r  Timestamp: (%u)%usecs%uns (Refresh every %u msecs)\n\r",
//                stat->DiscontinuityTime.sec_msb,
//                stat->DiscontinuityTime.seconds,
//                stat->DiscontinuityTime.nanoseconds,
//                stat->RefreshRate);
}

void icli_ip2_stat_icmp_syst_display(i32 session_id,
                                     vtss_ips_icmp_stat_t *entry)
{
    vtss_icmp_stat_data_t   *stat;

    if (!entry) {
        return;
    }

    stat = &entry->data;
    switch ( entry->IPVersion ) {
    case VTSS_IP_TYPE_IPV4:
        ICLI_PRINTF("\n\rIPv4 ICMP statistics:\n\r");
        break;
    case VTSS_IP_TYPE_IPV6:
        ICLI_PRINTF("\n\rIPv6 ICMP statistics:\n\r");
        break;
    case VTSS_IP_TYPE_NONE:
    default:
        ICLI_PRINTF("\n\rUnknown ICMP statistics:\n\r");
        break;
    }

    ICLI_PRINTF("\n\r  Rcvd: %u Message%s, %u Error%s",
                stat->InMsgs,
                (stat->InMsgs > 1) ? "s" : "",
                stat->InErrors,
                (stat->InErrors > 1) ? "s" : "");
    ICLI_PRINTF("\n\r  Sent: %u Message%s, %u Error%s\n\r",
                stat->OutMsgs,
                (stat->OutMsgs > 1) ? "s" : "",
                stat->OutErrors,
                (stat->OutErrors > 1) ? "s" : "");
}

void icli_ip2_stat_icmp_type_display(i32 session_id,
                                     BOOL force_pr,
                                     BOOL *first_pr,
                                     vtss_ips_icmp_stat_t *entry)
{
    vtss_icmp_stat_data_t   *stat;
    char                    buf[IP2_MAX_ICMP_TXT_LEN];

    if (!entry || !first_pr) {
        return;
    }

    if (*first_pr) {
        ICLI_PRINTF("\n\rICMP message statistics:\n\r");
        *first_pr = FALSE;
    }

    stat = &entry->data;
    if (!force_pr && (!stat->InMsgs && !stat->OutMsgs)) {
        return;
    }

    if (vtss_ip2_stat_icmp_type_txt(buf, IP2_MAX_ICMP_TXT_LEN, entry->IPVersion, entry->Type)) {
        switch ( entry->IPVersion ) {
        case VTSS_IP_TYPE_IPV4:
            ICLI_PRINTF("\n\r  IPv4 ICMP Message: %s", buf);
            break;
        case VTSS_IP_TYPE_IPV6:
            ICLI_PRINTF("\n\r  IPv6 ICMP Message: %s", buf);
            break;
        case VTSS_IP_TYPE_NONE:
        default:
            ICLI_PRINTF("\n\r  Unknown ICMP Message: %s", buf);
            break;
        }
    } else {
        ICLI_PRINTF("\n\r  Unknown ICMP Message: %s", buf);
    }

    ICLI_PRINTF("\n\r  Rcvd: %u Packet%s",
                stat->InMsgs,
                (stat->InMsgs > 1) ? "s" : "");
    ICLI_PRINTF("\n\r  Sent: %u Packet%s",
                stat->OutMsgs,
                (stat->OutMsgs > 1) ? "s" : "");
}
