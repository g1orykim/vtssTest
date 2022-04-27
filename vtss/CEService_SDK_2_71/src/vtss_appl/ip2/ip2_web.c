/*

   Vitesse Switch API software.

 Copyright (c) 2002-2014 Vitesse Semiconductor Corporation "Vitesse". All
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

#include "web_api.h"
#include "ip2_api.h"
#include "ip2_legacy.h"
#include "ip2_utils.h"
#include "ping_api.h"
#include "mgmt_api.h"

#ifdef VTSS_SW_OPTION_PRIV_LVL
#include "vtss_privilege_api.h"
#include "vtss_privilege_web_api.h"
#endif

#ifdef VTSS_SW_OPTION_DNS
#include "ip_dns_api.h"
#include "vlan_api.h"
#endif /* VTSS_SW_OPTION_DNS */

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_IP2

/* =================
 * Trace definitions
 * -------------- */
#include <vtss_module_id.h>
#include <vtss_trace_lvl_api.h>
#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_WEB
#include <vtss_trace_api.h>
/* ============== */

static BOOL rt_parse(CYG_HTTPD_STATE *p,
                     vtss_routing_entry_t *r,
                     vtss_routing_params_t *o,
                     int ix)
{
    int prefix = 0;
    memset(r, 0, sizeof(*r));
    r->type = VTSS_ROUTING_ENTRY_TYPE_INVALID;
    if (vtss_ip2_hasipv6() &&
        (web_parse_ipv6_fmt(p, &r->route.ipv6_uc.network.address, "rt_net_%d", ix) == VTSS_OK) &&
        (web_parse_ipv6_fmt(p, &r->route.ipv6_uc.destination, "rt_dest_%d", ix) == VTSS_OK) &&
        cyg_httpd_form_variable_int_fmt(p, &prefix, "rt_mask_%d", ix) && prefix >= 0 && prefix <= 128) {
        int nhvid = 0;

        if (cyg_httpd_form_variable_int_fmt(p, &nhvid, "rt_nhvid_%d", ix) &&
            (nhvid > 0)) {
            r->vlan = (vtss_if_id_vlan_t) (nhvid & 0xFFFF);
        } else {
            r->vlan = 0x0;
        }
        r->route.ipv6_uc.network.prefix_size = prefix;
        r->type = VTSS_ROUTING_ENTRY_TYPE_IPV6_UC;
        return TRUE;
    }
    if ((web_parse_ipv4_fmt(p, &r->route.ipv4_uc.network.address, "rt_net_%d", ix) == VTSS_OK) &&
        (web_parse_ipv4_fmt(p, &r->route.ipv4_uc.destination, "rt_dest_%d", ix) == VTSS_OK) &&
        cyg_httpd_form_variable_int_fmt(p, &prefix, "rt_mask_%d", ix) && prefix >= 0 && prefix <= 32) {
        r->route.ipv4_uc.network.prefix_size = prefix;
        r->type = VTSS_ROUTING_ENTRY_TYPE_IPV4_UC;
        return TRUE;
    }
    return FALSE;
}

static bool ip2_config_differs(const vtss_ip_conf_t *ipc_old, const vtss_ip_conf_t *ipc_new, BOOL chk_dhcp)
{
    if (chk_dhcp) {
        if (ipc_old->dhcpc            != ipc_new->dhcpc || /* DHCP Change? */
            ipc_old->fallback_timeout != ipc_new->fallback_timeout) { /* Fallback time change */
            return TRUE;
        }
    }
    /* Chk address differs */
    return memcmp(&ipc_old->network, &ipc_new->network, sizeof(vtss_ip_network_t)) != 0;
}

static BOOL net_parse(CYG_HTTPD_STATE *p,
                      vtss_ip_type_t type,
                      vtss_ip_network_t *n,
                      int ix)
{
    int prefix = 0;
    memset(n, 0, sizeof(*n));
    if (type == VTSS_IP_TYPE_IPV4 &&
        (web_parse_ipv4_fmt(p, &n->address.addr.ipv4, "if_addr_%d", ix) == VTSS_OK) &&
        cyg_httpd_form_variable_int_fmt(p, &prefix, "if_mask_%d", ix) && prefix >= 0 && prefix <= 32) {
        n->prefix_size = prefix;
        n->address.type = type;
        return TRUE;
    }
    if (type == VTSS_IP_TYPE_IPV6 &&
        (web_parse_ipv6_fmt(p, &n->address.addr.ipv6, "if_addr6_%d", ix) == VTSS_OK) &&
        cyg_httpd_form_variable_int_fmt(p, &prefix, "if_mask6_%d", ix) && prefix >= 0 && prefix <= 128) {
        n->prefix_size = prefix;
        n->address.type = type;
        return TRUE;
    }
    return FALSE;
}

static cyg_int32 handler_ip_config(CYG_HTTPD_STATE *p)
{
    vtss_ip2_global_param_t conf;
    vtss_routing_entry_t *routes;
    vtss_rc rc;
    int i, route_ct, ct;
    char buf[128];
#ifdef VTSS_SW_OPTION_DNS
    BOOL dns_proxy = 0;
    vtss_dns_srv_conf_t dns_conf;
#endif

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_IP)) {
        return -1;
    }
#endif

#ifdef VTSS_SW_OPTION_DNS
    (void) ip_dns_mgmt_get_proxy_status(&dns_proxy);
    (void) vtss_dns_mgmt_get_server(DNS_DEF_SRV_IDX, &dns_conf);
#endif

    if ((rc = vtss_ip2_global_param_get(&conf)) != VTSS_OK) {
        T_E("get global config: %s", error_txt(rc));
    }
    route_ct = 0;
    if ((routes = VTSS_CALLOC((ct = IP2_MAX_ROUTES), sizeof(*routes))) != NULL) {
        if ((rc = vtss_ip2_route_conf_get(ct, routes, &route_ct)) != VTSS_OK) {
            T_E("get routes: %s", error_txt(rc));
            ct = 0;
        }
    }

    if (p->method == CYG_HTTPD_METHOD_POST) {
        int val;
        if (cyg_httpd_form_varable_int(p, "ip_mode", &val)) {
            conf.enable_routing = val;
        }
        if ((rc = vtss_ip2_global_param_set(&conf)) != VTSS_OK) {
            T_E("set global:  %s", error_txt(rc));
        }
#ifdef VTSS_SW_OPTION_DNS
        if (cyg_httpd_form_varable_int(p, "ip_dns_src", &val) &&
            val >= VTSS_DNS_SRV_TYPE_DHCP_ANY &&
            val <= VTSS_DNS_SRV_TYPE_DHCP_VLAN) {
            BOOL set_cfg = TRUE;
            VTSS_DNS_TYPE_SET(&dns_conf, val);
            if (VTSS_DNS_TYPE_GET(&dns_conf) == VTSS_DNS_SRV_TYPE_STATIC &&
                (web_parse_ip_fmt(p, VTSS_DNS_ADDR_PTR(&dns_conf), "ip_dns_value") != VTSS_OK)) {
                T_E("Invalid IP address, not changing DNS config");
                set_cfg = FALSE;
            }
            if (VTSS_DNS_TYPE_GET(&dns_conf) == VTSS_DNS_SRV_TYPE_DHCP_VLAN) {
                if (cyg_httpd_form_varable_int(p, "ip_dns_value", &val) &&
                    val >= VLAN_ID_MIN && val <= VLAN_ID_MAX) {
                    VTSS_DNS_VLAN_SET(&dns_conf, val);
                } else {
                    T_E("Invalid VLAN, not changing DNS config");
                    set_cfg = FALSE;
                }
            }
            if (set_cfg) {
                if ((rc = vtss_dns_mgmt_set_server(DNS_DEF_SRV_IDX, &dns_conf)) != VTSS_OK) {
                    T_E("vtss_ip2_dns_set_conf: %s", error_txt(rc));
                }
            }
        }
        dns_proxy = cyg_httpd_form_variable_check_fmt(p, "ip_dns_proxy");
        if ((rc = ip_dns_mgmt_set_proxy_status(&dns_proxy)) != VTSS_OK) {
            T_E("set proxy dns:  %s", error_txt(rc));
        }
#endif
        if (cyg_httpd_form_varable_int(p, "if_ct", &ct)) {
            struct {
                vtss_vid_t vid;
                BOOL change_ipv4, change_ipv6;
                vtss_ip_conf_t ipv4, ipv6;
            } cfg[IP2_MAX_INTERFACES];
            int j = 0;
            for (i = 0; i < ct; i++) {
                int vid;
                BOOL vid_changed;
                cfg[j].change_ipv4 = cfg[j].change_ipv6 = vid_changed = FALSE;
                if (cyg_httpd_form_variable_int_fmt(p, &vid, "if_vid_%d", i)) {
                    /* Have a vid - what to do? */
                    if (cyg_httpd_form_variable_check_fmt(p, "if_del_%d", i)) {
                        /* Delete the interface */
                        if ((rc = vtss_ip2_if_conf_del(vid)) != VTSS_OK) {
                            T_E("Delete IP interface vid %d: %s", vid, error_txt(rc));
                        }
                    } else {
                        vtss_if_param_t param;
                        vtss_ip_conf_t ipconf, ipconf_old;
                        if (vtss_ip2_if_conf_get(vid, &param) != VTSS_OK) {
                            vtss_if_default_param(&param);
                            if ((rc = vtss_ip2_if_conf_set(vid, &param)) != VTSS_OK) {
                                T_E("Add IP interface vid %d: %s", vid, error_txt(rc));
                                continue;
                            }
                        }
                        /* IPv4 */
                        if ((rc = vtss_ip2_ipv4_conf_get(vid, &ipconf_old)) == VTSS_OK) {
                            vtss_ip_network_t network;
                            /* Track original config */
                            ipconf = ipconf_old;
                            /* DHCP enabled? */
                            ipconf.dhcpc = cyg_httpd_form_variable_check_fmt(p, "if_dhcp_%d", i);
                            if (cyg_httpd_form_variable_int_fmt(p, &val, "if_tout_%d", i))  {
                                ipconf.fallback_timeout = val;
                            }
                            if (net_parse(p, VTSS_IP_TYPE_IPV4, &network, i)) {
                                ipconf.network = network;
                            } else {
                                ipconf.network.address.type = VTSS_IP_TYPE_NONE;
                            }
                            if (ip2_config_differs(&ipconf_old, &ipconf, TRUE)) {
                                cfg[j].vid = vid;
                                cfg[j].change_ipv4 = vid_changed = TRUE;
                                cfg[j].ipv4 = ipconf;
                            }
                        } else {
                            T_E("Get IPv4 conf, vid %d: %s", vid, error_txt(rc));
                        }
                        /* IPv6 */
                        if (vtss_ip2_hasipv6()) {
                            if ((rc = vtss_ip2_ipv6_conf_get(vid, &ipconf_old)) == VTSS_OK) {
                                vtss_ip_network_t network;
                                ipconf = ipconf_old;
                                if (net_parse(p, VTSS_IP_TYPE_IPV6, &network, i)) {
                                    ipconf.network = network;
                                } else {
                                    ipconf.network.address.type = VTSS_IP_TYPE_NONE;
                                }
                                if (ip2_config_differs(&ipconf_old, &ipconf, FALSE)) {
                                    cfg[j].vid = vid;
                                    cfg[j].change_ipv6 = vid_changed = TRUE;
                                    cfg[j].ipv6 = ipconf;
                                }
                            } else {
                                T_E("Get IPv6 conf, vid %d: %s", vid, error_txt(rc));
                            }
                        }
                    }
                }
                if (vid_changed) {
                    j++;        /* One more vid changed */
                }
            }
            if (j) {
                vtss_ip_conf_t ipconf;
                T_D("Need to reconfigure %d interfaces", j);
                memset(&ipconf, 0, sizeof(ipconf));
                /* De-activate changed IP interfaces */
                for (i = 0; i < j; i++) {
                    int vid = cfg[i].vid;
                    if (cfg[i].change_ipv4) {
                        if ((rc = vtss_ip2_ipv4_conf_set(vid, &ipconf)) != VTSS_OK) {
                            T_E("Clear IPv4 conf, vid %d: %s", vid, error_txt(rc));
                        }
                    }
                    if (cfg[i].change_ipv6) {
                        if ((rc = vtss_ip2_ipv6_conf_set(vid, &ipconf)) != VTSS_OK) {
                            T_E("Clear IPv6 conf, vid %d: %s", vid, error_txt(rc));
                        }
                    }
                }
                /* Re-activate changed IP interfaces */
                for (i = 0; i < j; i++) {
                    int vid = cfg[i].vid;
                    if (cfg[i].change_ipv4) {
                        if ((rc = vtss_ip2_ipv4_conf_set(vid, &cfg[i].ipv4)) != VTSS_OK) {
                            T_E("Set IPv4 conf, vid %d: %s", vid, error_txt(rc));
                        }
                    }
                    if (cfg[i].change_ipv6) {
                        if ((rc = vtss_ip2_ipv6_conf_set(vid, &cfg[i].ipv6)) != VTSS_OK) {
                            T_E("Set IPv6 conf, vid %d: %s", vid, error_txt(rc));
                        }
                    }
                }
            }
        }
        if (cyg_httpd_form_varable_int(p, "rt_ct", &ct) && ct > 0) {
            vtss_routing_entry_t r;
            vtss_routing_params_t owner = { .owner = VTSS_ROUTING_PARAM_OWNER_STATIC_USER };
            for (i = 0; i < ct; i++) {
                if (rt_parse(p, &r, &owner, i)) {
                    BOOL do_delete = cyg_httpd_form_variable_check_fmt(p, "rt_del_%d", i);
                    if (do_delete) {
                        if ((rc = vtss_ip2_route_del(&r, &owner)) != VTSS_OK) {
                            T_W("Route del fails: %s", error_txt(rc));
                        }
                    } else {
                        /* Just re-add, this may be existing or new */
                        if ((rc = vtss_ip2_route_add(&r, &owner)) != VTSS_OK && rc != IP2_ERROR_EXISTS) {
                            T_W("Route add fails: %s", error_txt(rc));
                        }
                    }
                }
            }
        }
        redirect(p, "/ip_config.htm");
    } else {
        vtss_if_id_vlan_t vid;
        cyg_httpd_start_chunked("html");
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer),
                      "%d|%d|%d",
                      IP2_MAX_INTERFACES, IP2_MAX_ROUTES,
                      conf.enable_routing);
        cyg_httpd_write_chunked(p->outbuffer, ct);
#ifdef VTSS_SW_OPTION_DNS
        switch ( VTSS_DNS_TYPE_GET(&dns_conf) ) {
        case VTSS_DNS_SRV_TYPE_STATIC:
            (void) vtss_ip2_ip_addr_to_txt(buf, sizeof(buf), VTSS_DNS_ADDR_PTR(&dns_conf));
            break;
        case VTSS_DNS_SRV_TYPE_DHCP_VLAN:
            (void) snprintf(buf, sizeof(buf), "%d", VTSS_DNS_VLAN_GET(&dns_conf));
            break;
        default:
            buf[0] = '\0';
        }
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer),
                      "|%d|%s|%d", VTSS_DNS_TYPE_GET(&dns_conf), buf, dns_proxy);
        cyg_httpd_write_chunked(p->outbuffer, ct);
#endif
        cyg_httpd_write_chunked(",", 1);
        vid = VTSS_VID_NULL;
        while (vtss_ip2_if_id_next(vid, &vid) == VTSS_OK) {
            vtss_ip_conf_t ipconf, ipconf6;
            vtss_if_status_t ifstat;
            u32 ifct;
            if ((rc = vtss_ip2_ipv4_conf_get(vid, &ipconf)) == VTSS_OK) {
                /* Vid + IPv4 (dhcp+static) */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer),
                              "%d#%d#%d#",
                              vid,
                              ipconf.dhcpc,
                              ipconf.fallback_timeout);
                cyg_httpd_write_chunked(p->outbuffer, ct);
                if (ipconf.dhcpc || ipconf.network.address.type != VTSS_IP_TYPE_IPV4) {
                    cyg_httpd_write_chunked("##", 2);
                } else {
                    (void) vtss_ip2_ip_addr_to_txt(buf, sizeof(buf), &ipconf.network.address);
                    ct = snprintf(p->outbuffer, sizeof(p->outbuffer),
                                  "%s#%d#", buf, ipconf.network.prefix_size);
                    cyg_httpd_write_chunked(p->outbuffer, ct);
                }
                /* IPv4 - current */
                if (vtss_ip2_if_status_get(VTSS_IF_STATUS_TYPE_IPV4, vid, 1, &ifct, &ifstat) == VTSS_OK &&
                    ifct == 1 &&
                    ifstat.type == VTSS_IF_STATUS_TYPE_IPV4) {
                    ct = snprintf(p->outbuffer, sizeof(p->outbuffer),
                                  VTSS_IPV4N_FORMAT,
                                  VTSS_IPV4N_ARG(ifstat.u.ipv4.net));
                    cyg_httpd_write_chunked(p->outbuffer, ct);
                }
                cyg_httpd_write_chunked("#", 1);
                if (vtss_ip2_hasipv6() && (rc = vtss_ip2_ipv6_conf_get(vid, &ipconf6)) == VTSS_OK) {
                    /* IPv6 - configured */
                    if (ipconf6.network.address.type == VTSS_IP_TYPE_IPV6) {
                        (void) misc_ipv6_txt(&ipconf6.network.address.addr.ipv6, buf);
                        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s#%d#", buf, ipconf6.network.prefix_size);
                        cyg_httpd_write_chunked(p->outbuffer, ct);
                    } else {
                        cyg_httpd_write_chunked("##", 2);
                    }
                }
            } else {
                T_E("interface_config(%d): %s", vid, error_txt(rc));
            }
            /* End of interface */
            cyg_httpd_write_chunked("|", 1);
        }
        cyg_httpd_write_chunked(",", 1);
        for (i = 0; routes && i < route_ct; i++) {
            vtss_routing_entry_t const *rt = &routes[i];
            switch (rt->type) {
            case VTSS_ROUTING_ENTRY_TYPE_IPV4_UC:
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer),
                              VTSS_IPV4_FORMAT "#%d#" VTSS_IPV4_FORMAT "#0|",
                              VTSS_IPV4N_ARG(rt->route.ipv4_uc.network),
                              VTSS_IPV4_ARGS(rt->route.ipv4_uc.destination));
                cyg_httpd_write_chunked(p->outbuffer, ct);
                break;
            case VTSS_ROUTING_ENTRY_TYPE_IPV6_UC:
                if (vtss_ip2_hasipv6()) {
                    (void) misc_ipv6_txt(&rt->route.ipv6_uc.network.address, buf);
                    ct = snprintf(p->outbuffer, sizeof(p->outbuffer),
                                  "%s#%d#", buf, rt->route.ipv6_uc.network.prefix_size);

                    memset(buf, 0x0, sizeof(buf));
                    IP2_IPV6_RTNH_LLA_VLAN_GET(vid, rt);
                    if (vid) {
                        vtss_routing_entry_t    rte;
                        vtss_vid_t              run_vid = VTSS_VID_NULL;

                        memcpy(&rte, rt, sizeof(vtss_routing_entry_t));
                        IP2_IPV6_RTNH_LLA_IFID_SET(run_vid, &rte);
                        (void) misc_ipv6_txt(&rte.route.ipv6_uc.destination, buf);
                    } else {
                        (void) misc_ipv6_txt(&rt->route.ipv6_uc.destination, buf);
                    }
                    ct += snprintf(p->outbuffer + ct, sizeof(p->outbuffer) - ct,
                                   "%s#%u|", buf, vid);
                    cyg_httpd_write_chunked(p->outbuffer, ct);
                }
                break;
            default:
                ;
            }
        }
        cyg_httpd_end_chunked();
    }

    if (routes) {
        VTSS_FREE(routes);
    }

    return -1; // Do not further search the file system.
}

CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_ip, "/config/ip2_config", handler_ip_config);

static cyg_int32 handler_diag_ping(CYG_HTTPD_STATE *p)
{
    char            ipaddr[32];
    const char      *buf;
    size_t          len;
    cli_iolayer_t   *web_io = NULL;
    ulong           ioindex;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_STAT_TYPE, VTSS_MODULE_ID_PING)) {
        return -1;
    }
#endif

    if (p->method == CYG_HTTPD_METHOD_POST) {
        u32 ping_length = 2, count = 5, interval = 0;
        char str_buff[64];

        buf = cyg_httpd_form_varable_string(p, "ip_addr", &len);
        (void) cyg_httpd_form_varable_long_int(p, "length", &ping_length);
        (void) cyg_httpd_form_varable_long_int(p, "count", &count);
        (void) cyg_httpd_form_varable_long_int(p, "interval", &interval);
        if (buf && ping_length) {
            len = MIN(len, sizeof(ipaddr) - 1);
            strncpy(ipaddr, buf, len);
            ipaddr[len] = '\0';
            web_io = ping_test_async(ipaddr, ping_length, count, interval);
            if (web_io) {
                sprintf(str_buff, "/ping_result.htm?ioIndex=%x", (unsigned int) web_io);
                redirect(p, str_buff);
            } else {
                char *err = "No available internal resource, please try again later.";
                send_custom_error(p, "Ping Fail", err, strlen(err));
            }
        }
    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        if ((buf = cyg_httpd_form_varable_find(p, "ioIndex"))) {
            (void) cyg_httpd_str_to_hex(buf, &ioindex);
            web_io = (cli_iolayer_t *) ioindex;
            web_send_iolayer_output(WEB_CLI_IO_TYPE_PING, web_io, "html");
        }
    }

    return -1; // Do not further search the file system.
}

CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_diag_ping, "/config/ping", handler_diag_ping);

#ifdef VTSS_SW_OPTION_IPV6
static cyg_int32 handler_diag_ping_ipv6(CYG_HTTPD_STATE *p)
{
    vtss_ipv6_t   ipv6_addr;
    const char    *buf;
    cli_iolayer_t *web_io = NULL;
    ulong         ioindex;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_STAT_TYPE, VTSS_MODULE_ID_PING)) {
        return -1;
    }
#endif

    if (p->method == CYG_HTTPD_METHOD_POST) {
        u32 ping_length = 2, count = 5, interval = 0, intfidx = PING_DEF_EGRESS_INTF_VID;
        char str_buff[64];

        (void)web_parse_ipv6(p, "ipv6_addr", &ipv6_addr);
        (void)cyg_httpd_form_varable_long_int(p, "length", &ping_length);
        (void)cyg_httpd_form_varable_long_int(p, "count", &count);
        (void)cyg_httpd_form_varable_long_int(p, "interval", &interval);
        (void)cyg_httpd_form_varable_long_int(p, "intfidx", &intfidx);
        web_io = ping6_test_async(&ipv6_addr, ping_length, count, interval, intfidx);
        if (web_io) {
            sprintf(str_buff, "/ping_ipv6_result.htm?ioIndex=%x", (unsigned int) web_io);
            redirect(p, str_buff);
        } else {
            char *err = "No available internal resource, please try again later.";
            send_custom_error(p, "Ping Fail", err, strlen(err));
        }
    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        if ((buf = cyg_httpd_form_varable_find(p, "ioIndex"))) {
            (void) cyg_httpd_str_to_hex(buf, &ioindex);
            web_io = (cli_iolayer_t *) ioindex;
            web_send_iolayer_output(WEB_CLI_IO_TYPE_PING, web_io, "html");
        }
    }

    return -1; // Do not further search the file system.
}

CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_diag_ping_ipv6, "/config/ping_ipv6", handler_diag_ping_ipv6);
#endif /* VTSS_SW_OPTION_IPV6 */

/*lint -sem(ip2_printf, thread_protected) ... function only called from web server */
static int ip2_printf(const char *fmt, ...)
{
    int ct;
    va_list ap; /*lint -e{530} ... 'ap' is initialized by va_start() */

    va_start(ap, fmt);
    ct = vsnprintf(httpstate.outbuffer, sizeof(httpstate.outbuffer), fmt, ap);
    va_end(ap);
    cyg_httpd_write_chunked(httpstate.outbuffer, ct);

    return ct;
}

static cyg_int32 handler_ip_status(CYG_HTTPD_STATE *p)
{
#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_STAT_TYPE, VTSS_MODULE_ID_IP)) {
        return -1;
    }
#endif

    cyg_httpd_start_chunked("html");

    (void) vtss_ip2_if_print(ip2_printf, FALSE, VTSS_IF_STATUS_TYPE_ANY);

    cyg_httpd_write_chunked("^@^@^", 5); /* Split interfaces and routes */

    (void) vtss_ip2_route_print(VTSS_ROUTING_ENTRY_TYPE_IPV4_UC, ip2_printf);
    (void) vtss_ip2_route_print(VTSS_ROUTING_ENTRY_TYPE_IPV6_UC, ip2_printf);

    cyg_httpd_write_chunked("^@^@^", 5); /* Split routes and neighbours */

    (void) vtss_ip2_nb_print(VTSS_IP_TYPE_IPV4, ip2_printf);
    (void) vtss_ip2_nb_print(VTSS_IP_TYPE_IPV6, ip2_printf);

    cyg_httpd_end_chunked();

    return -1; // Do not further search the file system.
}

CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_stat_ip, "/stat/ip2_status", handler_ip_status);

/****************************************************************************/
/*  Module JS lib config routine                                            */
/****************************************************************************/

static size_t ip2_lib_config_js(char **base_ptr, char **cur_ptr, size_t *length)
{
    char buff[512];
    (void) snprintf(buff, sizeof(buff),
                    "var configIPMaxInterfaces = %d;\n"
                    "var configIPMaxRoutes = %d;\n"
                    "var configIPv6Support = %d;\n"
                    "var configIPRoutingSupport = %d;\n"
                    "var configIPDNSSupport = %d;\n"
                    "var configPingLenMin = %d;\n"
                    "var configPingLenMax = %d;\n"
                    "var configPingCntMin = %d;\n"
                    "var configPingCntMax = %d;\n"
                    "var configPingIntervalMin = %d;\n"
                    "var configPingIntervalMax = %d;\n",
                    IP2_MAX_INTERFACES,
                    IP2_MAX_ROUTES,
                    vtss_ip2_hasipv6(),
                    vtss_ip2_hasrouting(),
                    vtss_ip2_hasdns(),
                    PING_MIN_PACKET_LEN, PING_MAX_PACKET_LEN,
                    PING_MIN_PACKET_CNT, PING_MAX_PACKET_CNT,
                    PING_MIN_PACKET_INTERVAL, PING_MAX_PACKET_INTERVAL);

    return webCommonBufferHandler(base_ptr, cur_ptr, length, buff);
}

/****************************************************************************/
/*  JS lib_config table entry                                               */
/****************************************************************************/

web_lib_config_js_tab_entry(ip2_lib_config_js);
