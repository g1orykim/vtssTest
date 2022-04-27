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

#include "cli.h"
#include "ip2_api.h"
#include "ip2_utils.h"
#include "ip2_chip_api.h"
#ifdef VTSS_SW_OPTION_SNMP
#include "ip2_snmp.h"
#endif
#ifdef VTSS_SW_OPTION_DNS
#include "ip_dns_api.h"
#endif /* VTSS_SW_OPTION_DNS */
#include "vlan_api.h"
#include "mgmt_api.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_IP2

#if defined(VTSS_SW_OPTION_IPV6)
#define __IPvX    "IPv4/IPv6"
#define __IPvX_EX "(IPv4: a.b.c.d/x, IPv6: a:b::c/x)"
#else
#define __IPvX    "IPv4"
#define __IPvX_EX "(a.b.c.d/x)"
#endif

#define DO(X, W)                                        \
    {                                                   \
        vtss_rc __rc = (X);                             \
        if (__rc != VTSS_RC_OK) {                       \
            cli_printf("%s: %s\n", W, error_txt(__rc)); \
        }                                               \
    }

typedef struct {
    vtss_vid_t          vlan;
    cli_spec_t          vlan_spec;

    BOOL                vlan_list[VTSS_BF_SIZE(VTSS_VIDS)];
    cli_spec_t          vlan_list_spec;

    vtss_ip_network_t   ip_net;
    cli_spec_t          ip_net_spec;

    vtss_ip_addr_t      ip_router;
    cli_spec_t          ip_router_spec;

#ifdef VTSS_SW_OPTION_DNS
    vtss_ip_addr_t      dns_ip;
    cli_spec_t          dns_ip_spec;
#endif /* VTSS_SW_OPTION_DNS */

    BOOL                router;
    BOOL                host;
    BOOL                dhcp_any;
} IP2_cli_req_t;

void vtss_ip2_cli_init(void)
{
    cli_req_size_register(sizeof(IP2_cli_req_t));
}

static int IP2_parse_uint_range(char *cmd, int min, int max, u32 *res )
{
    int i, tmp;
    int length = strlen(cmd);

    if ( length <= 0 ) {
        return 1;
    }

    for (i = 0; i < length; i++) {
        if ( i == 0 && ( cmd[i] == '+' || cmd[i] == '-' ) ) {
            continue;
        }

        if (cmd[i] < '0' || cmd[i] > '9' ) {
            return 1;    /* Error, invalid chareter */
        }
    }

    tmp = atoi(cmd);
    if ( tmp > max || tmp < min ) {
        return 1;
    }

    *res = tmp;
    return 0;
}

static int IP2_parse_vlan(char *cmd, char *cmd2, char *stx, char *cmd_org,
                          cli_req_t *req)
{
    u32 tmp;
    IP2_cli_req_t *r = (IP2_cli_req_t *)req->module_req;

    req->parm_parsed = 1;
    if (IP2_parse_uint_range(cmd, VLAN_ID_MIN, VLAN_ID_MAX, &tmp)) {
        return 1;
    }

    r->vlan = tmp;
    r->vlan_spec = CLI_SPEC_VAL;

    return 0;
}

static int IP2_parse_vlan_list(char *cmd, char *cmd2, char *stx, char *cmd_org,
                               cli_req_t *req)
{
    IP2_cli_req_t *r = (IP2_cli_req_t *)req->module_req;

    req->parm_parsed = 1;
    if (mgmt_txt2list_bf(cmd, r->vlan_list, VLAN_ID_MIN,
                         VLAN_ID_MAX, FALSE, TRUE) == VTSS_RC_OK) {
        r->vlan_list_spec = CLI_SPEC_VAL;
        return 0;
    } else {
        r->vlan_list_spec = CLI_SPEC_ANY;
        return 1;
    }
}

static int IP2_parse_ipvx_net(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
#define BUFLEN 256
    u32 tmp = 0;
    char buf[BUFLEN];
    char *prefix;
    IP2_cli_req_t *r = (IP2_cli_req_t *)req->module_req;

    req->parm_parsed = 1;
    if ( strlen(cmd) >= BUFLEN - 1 ) {
        return 1;    /* Buffer overflow */
    }

    strcpy(buf, cmd);
    prefix = index(buf, '/');

    if (prefix == NULL) {
        return 1;    /* Error, prefix part not found */
    }
    *(prefix++) = 0;

    if (vtss_ip2_hasipv6() && mgmt_txt2ipv6(buf, &r->ip_net.address.addr.ipv6) == VTSS_OK ) {
        r->ip_net.address.type = VTSS_IP_TYPE_IPV6;
        if (IP2_parse_uint_range(prefix, 0, 128, &tmp)) {
            return 1;    /* Error, could not parse prefix part */
        }
    } else if (mgmt_txt2ipv4(buf, &r->ip_net.address.addr.ipv4, NULL, false) == VTSS_OK) {
        r->ip_net.address.type = VTSS_IP_TYPE_IPV4;
        if (IP2_parse_uint_range(prefix, 0, 30, &tmp)) {
            return 1;    /* Error, could not parse prefix part */
        }
    } else {
        return 1;               /* Malformed IP */
    }

    r->ip_net.prefix_size = tmp;
    r->ip_net_spec = CLI_SPEC_VAL;

    return 0;
#undef BUFLEN
}

static int IP2_parse_ipvx_ifaddr(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    IP2_cli_req_t *r = (IP2_cli_req_t *)req->module_req;

    if (IP2_parse_ipvx_net(cmd, cmd2, stx, cmd_org, req)) {
        return 1;
    }

    if (!vtss_ip_ifaddr_valid(&r->ip_net)) {
        r->ip_net_spec = CLI_SPEC_NONE; /* Base sets 'VAL' */
        return 1;               /* Other checks failed */
    }

    return 0;
}

static int IP2_parse_ip_gateway(char *cmd, char *cmd2, char *stx,
                                char *cmd_org, cli_req_t *req)
{
    IP2_cli_req_t *r = (IP2_cli_req_t *)req->module_req;

    req->parm_parsed = 1;

    if (vtss_ip2_hasipv6() && mgmt_txt2ipv6(cmd, &r->ip_router.addr.ipv6) == VTSS_OK ) {
        r->ip_router.type = VTSS_IP_TYPE_IPV6;
        r->ip_router_spec = CLI_SPEC_VAL;
    } else if (mgmt_txt2ipv4(cmd, &r->ip_router.addr.ipv4, NULL, false) == VTSS_OK) {
        r->ip_router.type = VTSS_IP_TYPE_IPV4;
        r->ip_router_spec = CLI_SPEC_VAL;
    } else {
        r->ip_router.type = VTSS_IP_TYPE_NONE;
        r->ip_router_spec = CLI_SPEC_NONE;
    }

    return r->ip_router_spec == CLI_SPEC_NONE;
}

#ifdef VTSS_SW_OPTION_DNS
static int IP2_parse_dns_src(char *cmd, char *cmd2, char *stx,
                             char *cmd_org, cli_req_t *req)
{
    IP2_cli_req_t *r = (IP2_cli_req_t *)req->module_req;

    req->parm_parsed = 1;
    if (mgmt_txt2ipv4(cmd, &r->dns_ip.addr.ipv4, NULL, false) == VTSS_OK) {
        r->dns_ip.type = VTSS_IP_TYPE_IPV4;
        r->dns_ip_spec = CLI_SPEC_VAL;
        return 0;
    } else if (!strncmp(cmd, "none", 4)) {
        r->dns_ip.type = VTSS_IP_TYPE_NONE;
        r->dns_ip_spec = CLI_SPEC_VAL;
        return 0;
    } else if (!strncmp(cmd, "dhcp_any", 8)) {
        r->dhcp_any = TRUE;
        return 0;
    }
    return IP2_parse_vlan(cmd, cmd2, stx, cmd_org, req);
}
#endif /* VTSS_SW_OPTION_DNS */

static int32_t IP2_parse_keyword(char *cmd, char *cmd2, char *stx,
                                 char *cmd_org, cli_req_t *req)
{
    IP2_cli_req_t *r = req->module_req;
    char *found = cli_parse_find(cmd, stx);

    req->parm_parsed = 1;

    if (found && r) {
        if (!strncmp(found, "router", 6)) {
            r->router = TRUE;
        } else if (!strncmp(found, "host", 4)) {
            r->host = TRUE;
        } else {
            found = NULL;
        }
    }

    return (found == NULL ? 1 : 0);
}

static int32_t IP2_parse_value(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    req->parm_parsed = 1;
    return cli_parse_ulong(cmd, &req->value, 0, 0xffffffff);
}

static cli_parm_t IP2_cli_parm_table[] = {
    {
        "<vlan>",
        "VLAN",
        CLI_PARM_FLAG_NONE,
        IP2_parse_vlan,
        NULL
    },
    {
        "<vlan_list>",
        "List of VLANS",
        CLI_PARM_FLAG_NONE,
        IP2_parse_vlan_list,
        NULL
    },
    {
        "<vlan_list>",
        "List of VLANS",
        CLI_PARM_FLAG_NONE,
        IP2_parse_vlan_list,
        NULL
    },
    {
        "enable|disable",
        "enable|disable",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        NULL
    },
    {
        "<ip_net>",
        __IPvX " network address " __IPvX_EX,
        CLI_PARM_FLAG_SET,
        IP2_parse_ipvx_net,
        NULL
    },
    {
        "<ip_ifaddr>",
        __IPvX " interface address " __IPvX_EX,
        CLI_PARM_FLAG_SET,
        IP2_parse_ipvx_ifaddr,
        NULL
    },
    {
        "<ip_gateway>",
        __IPvX " gateway host address",
        CLI_PARM_FLAG_SET,
        IP2_parse_ip_gateway,
        NULL
    },
    {
        "host|router",
        "Role configuration",
        CLI_PARM_FLAG_SET,
        IP2_parse_keyword,
        NULL,
    },
#ifdef VTSS_SW_OPTION_DNS
    {
        "<dns_source>",
        "DNS server source: <ip_address>|<vlan>|'dhcp_any'|'none'",
        CLI_PARM_FLAG_SET,
        IP2_parse_dns_src,
        NULL,
    },
#endif /* VTSS_SW_OPTION_DNS */
    {
        "<value>",
        "value",
        CLI_PARM_FLAG_SET,
        IP2_parse_value,
        NULL,
    },

    {NULL, NULL, 0, 0, NULL}
};

// IP configuration dump
static void IP2_configuration(cli_req_t *req)
{
    vtss_ip2_global_param_t conf;
    vtss_rc rc;
    vtss_if_id_vlan_t ifid;
    int ct;
    vtss_routing_entry_t *routes;

    /* Global */
    if ((rc = vtss_ip2_global_param_get(&conf)) == VTSS_OK) {
        CPRINTF("Mode: %s\n", conf.enable_routing ? "router" : "host");
    } else {
        CPRINTF("Error getting configuration: %s", error_txt(rc));
    }

    /* Interfaces */
    for (ct = 0, ifid = VLAN_ID_MIN; ifid <= VLAN_ID_MAX; ifid++) {
        vtss_if_param_t if_conf;
        vtss_ip_conf_t ip_conf;

        if (vtss_ip2_if_conf_get(ifid, &if_conf) == VTSS_RC_OK) {
            CPRINTF("vlan%d: mtu=%u", ifid, if_conf.mtu);
        } else {
            continue;
        }

        if (vtss_ip2_ipv4_conf_get(ifid, &ip_conf) == VTSS_RC_OK) {
            if (ip_conf.dhcpc) {
                CPRINTF(" dhcpv4=enable");
            }

            if (ip_conf.network.address.type == VTSS_IP_TYPE_IPV4) {
                CPRINTF(" IPv4=" VTSS_IPV4_FORMAT "/%d",
                        VTSS_IPV4_ARGS(ip_conf.network.address.addr.ipv4),
                        ip_conf.network.prefix_size);
            }

            if (ip_conf.fallback_timeout) {
                CPRINTF(" fallback-timer=%u", ip_conf.fallback_timeout);
            }
        }

        if (vtss_ip2_hasipv6() && vtss_ip2_ipv6_conf_get(ifid, &ip_conf) == VTSS_RC_OK) {
            if (ip_conf.dhcpc) {
                CPRINTF(" dhcpv6=enable");
            } else if (ip_conf.network.address.type == VTSS_IP_TYPE_IPV6) {
                char buf[128];
                (void) misc_ipv6_txt(&ip_conf.network.address.addr.ipv6, buf);
                CPRINTF(" IPv6=%s/%d", buf, ip_conf.network.prefix_size);
            }
        }
        CPRINTF("\n");
        ct++;
    }

    if (ct == 0) {
        CPRINTF("Zero IP interfaces.\n");
    }

    if ((routes = VTSS_CALLOC((ct = IP2_MAX_ROUTES), sizeof(*routes))) != NULL) {
        if (vtss_ip2_route_conf_get(ct, routes, &ct) == VTSS_OK) {
            int j;
            for (j = 0; j < ct; j++) {
                vtss_routing_entry_t const *rt = &routes[j];
                if (rt->type == VTSS_ROUTING_ENTRY_TYPE_IPV4_UC) {
                    CPRINTF("route %2d: IPv4 " VTSS_IPV4_UC_FORMAT "\n", j, VTSS_IPV4_UC_ARGS(rt->route.ipv4_uc));
                } else if (vtss_ip2_hasipv6() && rt->type == VTSS_ROUTING_ENTRY_TYPE_IPV6_UC) {
                    char buf[128];
                    (void) misc_ipv6_txt(&rt->route.ipv6_uc.network.address, buf);
                    CPRINTF("route %2d: IPv6 {network = %s/%d, ", j, buf, rt->route.ipv6_uc.network.prefix_size);
                    (void) misc_ipv6_txt(&rt->route.ipv6_uc.destination, buf);
                    CPRINTF("destination = %s}\n", buf);
                } else {
                    CPRINTF("route %2d: Invalid route type %d\n", j, rt->type);
                }
            }
        }
        VTSS_FREE(routes);
    }
}
cli_cmd_tab_entry (
    "IP Configuration",
    NULL,
    "Show IP configuration settings",
    CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_IP2,
    IP2_configuration,
    NULL,
    IP2_cli_parm_table,
    CLI_CMD_FLAG_NONE
);


#if 0
// IP sorted routes dump
static void IP2_sort_routes(cli_req_t *req)
{
    vtss_routing_entry_t route, *key;
    key = NULL;                 /* Getfirst */
    int j = 0;
    while (vtss_ip2_route_getnext(key, &route) == VTSS_OK) {
        switch (route.type) {
        case VTSS_ROUTING_ENTRY_TYPE_IPV4_UC:
            CPRINTF("route %2d: IPv4 " VTSS_IPV4_UC_FORMAT "\n", j, VTSS_IPV4_UC_ARGS(route.route.ipv4_uc));
            break;
        default:
            CPRINTF("route %2d: Invalid route type %d\n", j, route.type);
        }
        j++;
        key = &route;           /* getnext */
    }
}
cli_cmd_tab_entry (
    "IP2 sort_routes",
    NULL,
    "Show IP in sorted fashion",
    CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_IP2,
    IP2_sort_routes,
    NULL,
    IP2_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif


#if defined(VTSS_SW_OPTION_L3RT)
// Global routing enable switch ///////////////////////////////////////////////
static void IP2_routing_enable_disable(cli_req_t *req)
{
    vtss_rc rc;
    vtss_ip2_global_param_t conf;
    IP2_cli_req_t *r = (IP2_cli_req_t *)req->module_req;

    if ((rc = vtss_ip2_global_param_get(&conf)) != VTSS_OK) {
        CPRINTF("Error getting configuration: %s", error_txt(rc));
        return;
    }

    if (r->host && r->router) {
        CPRINTF("Error parsing input\n");
        return;
    }

    if ((!r->host) && (!r->router)) {
        CPRINTF("Mode: %s\n", conf.enable_routing ? "router" : "host");
        return;
    }

    conf.enable_routing = r->router;
    if ((rc = vtss_ip2_global_param_set(&conf)) != VTSS_OK) {
        CPRINTF("Error setting configuration: %s", error_txt(rc));
    }

}
cli_cmd_tab_entry (
    "IP Mode [host|router]",
    NULL,
    "Control/Inspect if IP routing is enabled",
    CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_IP2,
    IP2_routing_enable_disable,
    NULL,
    IP2_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif /* VTSS_SW_OPTION_L3RT) */

// Router legs ////////////////////////////////////////////////////////////////
static void IP2_interface_add(cli_req_t *req)
{
    vtss_rc rc;
    vtss_vid_t vlan;
    vtss_if_param_t f;
    IP2_cli_req_t *r = (IP2_cli_req_t *)req->module_req;

    vtss_if_default_param(&f);

    for (vlan = 1; vlan < VTSS_VIDS; ++vlan) {
        if (VTSS_BF_GET(r->vlan_list, vlan)) {
            rc = vtss_ip2_if_conf_set(vlan, &f);
            if (rc != VTSS_RC_OK) {
                CPRINTF("Failed to add vlan interface %u\n", vlan);
            }
        }
    }

}
cli_cmd_tab_entry (
    "IP Interface add <vlan_list>",
    NULL,
    "Add router leg",
    CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_IP2,
    IP2_interface_add,
    NULL,
    IP2_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

static void IP2_interface_del_single(const vtss_vid_t vlan)
{
    vtss_rc rc = vtss_ip2_if_conf_del(vlan);
    if (rc != VTSS_RC_OK) {
        CPRINTF("Failed to delete vlan interface %u\n", vlan);
    }
}
static void IP2_interface_del(cli_req_t *req)
{
    IP2_cli_req_t *r = (IP2_cli_req_t *)req->module_req;

    if (r->vlan_list_spec == CLI_SPEC_VAL) {
        vtss_vid_t vlan;
        for (vlan = 1; vlan < VTSS_VIDS; ++vlan) {
            if (VTSS_BF_GET(r->vlan_list, vlan)) {
                IP2_interface_del_single(vlan);
            }
        }
    } else {
        vtss_if_id_vlan_t cur = 0, next = 0;
        while (vtss_ip2_if_id_next(cur, &next) == VTSS_RC_OK) {
            IP2_interface_del_single(next);
            cur = next;
        }
    }
}
cli_cmd_tab_entry (
    "IP Interface delete [<vlan_list>]",
    NULL,
    "Delete router leg",
    CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_IP2,
    IP2_interface_del,
    NULL,
    IP2_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

static void IP2_interface_list_single(vtss_vid_t vlan)
{
#define BUF_SIZE 1024
#define MAX_OBJS 16
    vtss_rc rc;
    u32 if_st_cnt = 0;

    char buf[BUF_SIZE];
    vtss_if_status_t status[16];

    rc = vtss_ip2_if_status_get(VTSS_IF_STATUS_TYPE_ANY, vlan, MAX_OBJS,
                                &if_st_cnt, status);

    if (rc != VTSS_RC_OK) {
        CPRINTF("Failed to get status on interface: %u\n", vlan);
        return;
    }

    vtss_ip2_if_status_sort(if_st_cnt, status);
    (void) vtss_ip2_if_status_to_txt(buf, BUF_SIZE, status, if_st_cnt);
    CPRINTF("%s", buf);
#undef MAX_OBJS
#undef BUF_SIZE
}
static void IP2_interface_list(cli_req_t *req)
{
    BOOL first = TRUE;
    IP2_cli_req_t *r = (IP2_cli_req_t *)req->module_req;

    if (r->vlan_list_spec == CLI_SPEC_VAL) {
        vtss_if_id_vlan_t vlan;

        for (vlan = 1; vlan < VTSS_VIDS; ++vlan) {
            if (VTSS_BF_GET(r->vlan_list, vlan)) {
                if (first) {
                    first = FALSE;
                } else {
                    CPRINTF("\n");
                }

                IP2_interface_list_single(vlan);
            }
        }

    } else {
        (void)vtss_ip2_if_print(cli_printf, FALSE, VTSS_IF_STATUS_TYPE_ANY);

    }
}
cli_cmd_tab_entry (
    "IP Interface list [<vlan_list>]",
    NULL,
    "List router leg(s)",
    CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_IP2,
    IP2_interface_list,
    NULL,
    IP2_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

// Ipv4 Addresses /////////////////////////////////////////////////////////////
static void IP2_address_assign(cli_req_t *req)
{
    vtss_ip_conf_t conf;
    IP2_cli_req_t *r = (IP2_cli_req_t *)req->module_req;

    if (vtss_ip2_hasipv6() && r->ip_net.address.type == VTSS_IP_TYPE_IPV6) {
        DO(vtss_ip2_ipv6_conf_get(r->vlan, &conf), "Get IPv6 address");
        conf.network = r->ip_net;
        DO(vtss_ip2_ipv6_conf_set(r->vlan, &conf), "Set IPv6 address");
    } else if (r->ip_net.address.type == VTSS_IP_TYPE_IPV4) {
        DO(vtss_ip2_ipv4_conf_get(r->vlan, &conf), "Get IPv4 address");
        conf.network = r->ip_net;
        DO(vtss_ip2_ipv4_conf_set(r->vlan, &conf), "Set IPv4 address");
    } else {
        cli_printf("Invalid [<ip_ifaddr>] parameter\n");
    }
}
cli_cmd_tab_entry (
    "IP Address <vlan> <ip_ifaddr>",
    NULL,
    "Assign an static IP address for a VLAN interface",
    CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_IP2,
    IP2_address_assign,
    NULL,
    IP2_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

static void IP2_address_assign_dhcp(cli_req_t *req)
{
    vtss_ip_conf_t conf;
    IP2_cli_req_t *r = (IP2_cli_req_t *)req->module_req;

    DO(vtss_ip2_ipv4_conf_get(r->vlan, &conf), "Get dhcp");
    if (req->set) {
        conf.dhcpc = req->enable;
        DO(vtss_ip2_ipv4_conf_set(r->vlan, &conf), "Set dhcp");
    } else {
        CPRINTF("DHCP : %s\n", cli_bool_txt(conf.dhcpc));
    }
}
cli_cmd_tab_entry (
    "IP DHCP <vlan> [enable|disable]",
    NULL,
    "Set or show the DHCP client mode",
    CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_IP2,
    IP2_address_assign_dhcp,
    NULL,
    IP2_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

static void IP2_address_assign_dhcp_fallback_timeout(cli_req_t *req)
{
    vtss_ip_conf_t conf;
    IP2_cli_req_t *r = (IP2_cli_req_t *)req->module_req;

    DO(vtss_ip2_ipv4_conf_get(r->vlan, &conf), "Get interface conf");

    if (req->set) {
        conf.fallback_timeout = req->value;
        DO(vtss_ip2_ipv4_conf_set(r->vlan, &conf), "Set IPv4 fallback timeout");

    } else {
        CPRINTF("Fallback timeout: %u\n", conf.fallback_timeout);

    }
}
cli_cmd_tab_entry (
    "IP DHCP fallback timeout <vlan> [<value>]",
    NULL,
    "Set or show the DHCP fallback timeout in seconds. Zero disables fallback",
    CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_IP2,
    IP2_address_assign_dhcp_fallback_timeout,
    NULL,
    IP2_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

static void IP2_dhcp_retry(cli_req_t *req)
{
    IP2_cli_req_t *r = (IP2_cli_req_t *)req->module_req;

    if (vtss_ip2_ipv4_dhcp_restart(r->vlan) != VTSS_RC_OK) {
        CPRINTF("Failed to restart dhcp client on VLAN %u\n", r->vlan);
    }
}
cli_cmd_tab_entry (
    "IP DHCP retry <vlan>",
    NULL,
    "Restart the DHCP client",
    CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_IP2,
    IP2_dhcp_retry,
    NULL,
    IP2_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

static void IP2_address_delete(cli_req_t *req)
{
    vtss_ip_conf_t conf, dconf = {
        .dhcpc = FALSE,
        .network = { .address = { .type = VTSS_IP_TYPE_NONE } }
    };

    IP2_cli_req_t *r = (IP2_cli_req_t *)req->module_req;

    if (vtss_ip2_hasipv6() && r->ip_net.address.type == VTSS_IP_TYPE_IPV6) {
        if (vtss_ip2_if_exists(r->vlan) &&
            vtss_ip2_ipv6_conf_get(r->vlan, &conf) == VTSS_OK) {
            if (vtss_ip_network_equal(&r->ip_net, &conf.network)) {
                DO(vtss_ip2_ipv6_conf_set(r->vlan, &dconf), "Delete IPv6 address");
            } else {
                CPRINTF("Delete failed: Address not found on interface.\n");
            }
        } else {
            CPRINTF("%u: No such interface.\n", r->vlan);
        }
    } else if (r->ip_net.address.type == VTSS_IP_TYPE_IPV4) {
        if (vtss_ip2_if_exists(r->vlan) &&
            vtss_ip2_ipv4_conf_get(r->vlan, &conf) == VTSS_OK) {
            if (vtss_ip_network_equal(&r->ip_net, &conf.network)) {
                conf.network.address.type = VTSS_IP_TYPE_NONE;
                DO(vtss_ip2_ipv4_conf_set(r->vlan, &conf), "Delete IPv4 address");

            } else {
                CPRINTF("Delete failed: Address not found on interface.\n");
            }

        } else {
            CPRINTF("%u: No such interface.\n", r->vlan);
        }
    } else {
        cli_printf("Invalid [<ip_ifaddr>] parameter\n");
    }
}

cli_cmd_tab_entry (
    "IP Address Delete <vlan> <ip_ifaddr>",
    NULL,
    "Delete the interface address(es). Also disables dhcp client.",
    CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_IP2,
    IP2_address_delete,
    NULL,
    IP2_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

// IPv4 routes ////////////////////////////////////////////////////////////////

static vtss_rc IP2_route_normalize(const vtss_ip_network_t *net,
                                   const vtss_ip_addr_t    *gw,
                                   vtss_routing_entry_t    *rt)
{
    vtss_rc rc = VTSS_INVALID_PARAMETER;

    if (net->address.type != gw->type) {
        CPRINTF("Network and gateway must be same type!\n");
        return rc;
    }

    if (net->address.type == VTSS_IP_TYPE_IPV4) {
        vtss_ipv4_t mask;
        rt->type = VTSS_ROUTING_ENTRY_TYPE_IPV4_UC;
        rt->route.ipv4_uc.network.address = net->address.addr.ipv4;
        rt->route.ipv4_uc.network.prefix_size = net->prefix_size;
        rt->route.ipv4_uc.destination = gw->addr.ipv4;
        if (vtss_conv_prefix_to_ipv4mask(rt->route.ipv4_uc.network.prefix_size, &mask) == VTSS_OK) {
            if (rt->route.ipv4_uc.network.address & ~mask) {
                rt->route.ipv4_uc.network.address &= mask;
                CPRINTF("Normalized route: "VTSS_IPV4_UC_FORMAT"\n", VTSS_IPV4_UC_ARGS(rt->route.ipv4_uc));
            }
        }
        rc = VTSS_OK;
    } else if (net->address.type == VTSS_IP_TYPE_IPV6) {
        vtss_ipv6_t mask;
        size_t i;
        if ((rc = vtss_conv_prefix_to_ipv6mask(net->prefix_size, &mask)) != VTSS_OK) {
            return rc;
        }
        rt->type = VTSS_ROUTING_ENTRY_TYPE_IPV6_UC;
        for (i = 0; i < sizeof(mask.addr); i++) {
            rt->route.ipv6_uc.network.address.addr[i] = mask.addr[i] & net->address.addr.ipv6.addr[i];
        }
        rt->route.ipv6_uc.network.prefix_size = net->prefix_size;
        rt->route.ipv6_uc.destination = gw->addr.ipv6;
        rc = VTSS_OK;
    } else {
        CPRINTF("Invalid address type: %d\n", net->address.type);
    }

    return rc;
}

static void IP2_route_add(cli_req_t *req)
{
    IP2_cli_req_t *r = (IP2_cli_req_t *)req->module_req;
    vtss_routing_params_t p = { .owner = VTSS_ROUTING_PARAM_OWNER_STATIC_USER };
    vtss_routing_entry_t rt;
    if (IP2_route_normalize(&r->ip_net, &r->ip_router, &rt) == VTSS_OK) {
        DO(vtss_ip2_route_add(&rt, &p), "Add route");
    }
}
cli_cmd_tab_entry (
    "IP Route Add <ip_net> <ip_gateway>",
    NULL,
    "Add Network route",
    CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_IP2,
    IP2_route_add,
    NULL,
    IP2_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

static void IP2_route_del(cli_req_t *req)
{
    IP2_cli_req_t *r = (IP2_cli_req_t *)req->module_req;
    vtss_routing_params_t p = { .owner = VTSS_ROUTING_PARAM_OWNER_STATIC_USER };
    vtss_routing_entry_t rt;
    if (IP2_route_normalize(&r->ip_net, &r->ip_router, &rt) == VTSS_OK) {
        DO(vtss_ip2_route_del(&rt, &p), "Delete route");
    }
}
cli_cmd_tab_entry (
    "IP Route Delete <ip_net> <ip_gateway>",
    NULL,
    "Delete network route",
    CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_IP2,
    IP2_route_del,
    NULL,
    IP2_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

static void IP2_route_list(cli_req_t *req)
{
    (void)vtss_ip2_route_print(VTSS_ROUTING_ENTRY_TYPE_IPV4_UC, cli_printf);
    (void)vtss_ip2_route_print(VTSS_ROUTING_ENTRY_TYPE_IPV6_UC, cli_printf);
}
cli_cmd_tab_entry (
    "IP Route List",
    NULL,
    "List network routes",
    CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_IP2,
    IP2_route_list,
    NULL,
    IP2_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

static void IP2_debug_route_list(cli_req_t *req)
{
    extern void show_network_routes(vtss_ip2_cli_pr *);
    show_network_routes(cli_printf);
}
cli_cmd_tab_entry (
    "debug IP Route",
    NULL,
    "List network routes (direcly from kernel)",
    CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_IP2,
    IP2_debug_route_list,
    NULL,
    IP2_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

static void IP2_neighbour_list(cli_req_t *req)
{
    (void)vtss_ip2_nb_print(VTSS_IP_TYPE_IPV4, cli_printf);
    (void)vtss_ip2_nb_print(VTSS_IP_TYPE_IPV6, cli_printf);
}
cli_cmd_tab_entry (
    "IP Neighbour List",
    NULL,
    "List neighbours",
    CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_IP2,
    IP2_neighbour_list,
    NULL,
    IP2_cli_parm_table,
    CLI_CMD_FLAG_NONE
);


static void IP2_neighbour_clear(cli_req_t *req)
{
    DO(vtss_ip2_nb_clear(VTSS_IP_TYPE_IPV4), "Clear IPv4 neighbour cache");
    DO(vtss_ip2_nb_clear(VTSS_IP_TYPE_IPV6), "Clear IPv6 neighbour cache");
}
cli_cmd_tab_entry (
    "IP Neighbour Clear",
    NULL,
    "Clear neighbours",
    CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_IP2,
    IP2_neighbour_clear,
    NULL,
    IP2_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

#ifdef VTSS_SW_OPTION_DNS
static void IP2_dns_status(cli_req_t *req)
{
    vtss_rc rc;
    char buf[128];
    vtss_ip_addr_t addr;

    rc = vtss_dns_mgmt_get_server4(&addr.addr.ipv4);

    if (rc == VTSS_RC_OK) {
        addr.type = VTSS_IP_TYPE_IPV4;
        (void) vtss_ip2_ip_addr_to_txt(buf, 128, &addr);
        CPRINTF("DNS server used: %s\n", buf);
    } else {
        CPRINTF("Failed to get dns status\n");
    }
}
cli_cmd_tab_entry (
    "IP DNS Status",
    NULL,
    "Get dns status",
    CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_IP2,
    IP2_dns_status,
    NULL,
    IP2_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

static void IP2_dns_conf(cli_req_t *req)
{
    vtss_rc rc;
    char buf[128];
    vtss_dns_srv_conf_t conf;
    IP2_cli_req_t *r = (IP2_cli_req_t *)req->module_req;

    memset(&conf, 0, sizeof(conf));

    if (!req->set) {
        rc = vtss_dns_mgmt_get_server(DNS_DEF_SRV_IDX, &conf);
        if (rc != VTSS_RC_OK) {
            CPRINTF("Failed to get dns config\n");
        } else {
            switch ( VTSS_DNS_TYPE_GET(&conf) ) {
            case VTSS_DNS_SRV_TYPE_DHCP_ANY:
                CPRINTF("Get dns server from arbitary dhcp client\n");
                break;
            case VTSS_DNS_SRV_TYPE_NONE:
                CPRINTF("No dns server\n");
                break;
            case VTSS_DNS_SRV_TYPE_STATIC:
                (void) vtss_ip2_ip_addr_to_txt(buf, 128,
                                               VTSS_DNS_ADDR_PTR(&conf));
                CPRINTF("Static dns server: %s\n", buf);
                break;
            case VTSS_DNS_SRV_TYPE_DHCP_VLAN:
                CPRINTF("Get dns server from dhcp client on vlan %u\n",
                        VTSS_DNS_VLAN_GET(&conf));
                break;
            default:
                CPRINTF("Unknown conf type: %u\n", VTSS_DNS_TYPE_GET(&conf));
                break;
            }
        }
        // Show dns config
        return;
    }

    if (r->vlan_spec == CLI_SPEC_VAL) {
        VTSS_DNS_TYPE_SET(&conf, VTSS_DNS_SRV_TYPE_DHCP_VLAN);
        VTSS_DNS_VLAN_SET(&conf, r->vlan);
    }

    if (r->vlan_spec == CLI_SPEC_ANY || r->dhcp_any) {
        VTSS_DNS_TYPE_SET(&conf, VTSS_DNS_SRV_TYPE_DHCP_ANY);
    }

    if (r->dns_ip_spec != CLI_SPEC_NONE) {
        if (vtss_ip_addr_is_zero(&r->dns_ip)) {
            VTSS_DNS_TYPE_SET(&conf, VTSS_DNS_SRV_TYPE_NONE);
        } else {
            VTSS_DNS_TYPE_SET(&conf, VTSS_DNS_SRV_TYPE_STATIC);
            VTSS_DNS_ADDR4_SET(&conf, r->dns_ip.addr.ipv4);
        }
    }
    DO(vtss_dns_mgmt_set_server(DNS_DEF_SRV_IDX, &conf), "Set DNS configuration");
}
cli_cmd_tab_entry (
    "IP DNS Conf [<dns_source>]",
    NULL,
    "Get or set DNS configuration",
    CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_IP2,
    IP2_dns_conf,
    NULL,
    IP2_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif /* VTSS_SW_OPTION_DNS */

static void debug_iplog(cli_req_t *req)
{
    int cyg_net_log_mask;

    if (req->set) {
        cyg_net_log_mask = req->value;
        CPRINTF("IP log mask: 0x%04x\n", cyg_net_log_mask);
    }
    if (!req->set) {
        cli_puts(
            "LOG_ERR      0x0001 - error conditions\n"
            "LOG_WARNING  0x0002 - interesting, but not errors\n"
            "LOG_NOTICE   0x0004 - things to look out for\n"
            "LOG_INFO     0x0008 - parm comments\n"
            "LOG_DEBUG    0x0010 - for finding obscure problems\n"
            "LOG_MDEBUG   0x0020 - additional information about memory allocations\n"
            "LOG_IOCTL    0x0040 - information about ioctl calls\n"
            "LOG_INIT     0x0080 - information as system initializes\n"
            "LOG_ADDR     0x0100 - information about IPv6 addresses\n"
            "LOG_FAIL     0x0200 - why packets (IPv6) are ignored, etc.\n"
            "LOG_EMERG    0x4000 - emergency conditions\n"
            "LOG_CRIT     0x8000 - critical error\n"
        );
    }
}
cli_cmd_tab_entry (
    "Debug IP Log",
    "Debug IP Log [<value>]",
    "Set or show IP logging mask",
    CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    debug_iplog,
    NULL,
    IP2_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

static void debug_ipkmem(cli_req_t *req)
{
    extern void cyg_kmem_print_stats(void);
    cyg_kmem_print_stats();
}
cli_cmd_tab_entry (
    "Debug IP KMem",
    NULL,
    "Show IP kernel buffer stats",
    CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    debug_ipkmem,
    NULL,
    IP2_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

static void cli_cmd_debug_ipsockets(cli_req_t *req)
{
    struct sockaddr_in peeraddr, myaddr;
    socklen_t addrlen = sizeof(peeraddr);
    int so_type;
    socklen_t so_type_len = sizeof(so_type);
    int i;

    for (i = 3; i < CYGNUM_FILEIO_NFD; i++) {
        if (getpeername(i, (struct sockaddr *)&peeraddr, &addrlen) == 0) {
            (void) getsockname(i, (struct sockaddr *)&myaddr, &addrlen);
            CPRINTF("Fd %d from %s:%d to %d\n", i,
                    inet_ntoa(peeraddr.sin_addr),
                    ntohs(peeraddr.sin_port),
                    ntohs(myaddr.sin_port));
        } else if (getsockname(i, (struct sockaddr *)&myaddr, &addrlen) == 0) {
            if (getsockopt(i, SOL_SOCKET, SO_TYPE, &so_type, &so_type_len) != 0) {
                so_type = -1;
            }
            CPRINTF("Fd %3d %s %s LISTEN %5d\n", i,
                    so_type == SOCK_DGRAM  ? "UDP" :
                    so_type == SOCK_STREAM ? "TCP" :
                    so_type == SOCK_RAW    ? "RAW" :
                    "UNK",
                    myaddr.sin_family == AF_INET  ? "IPv4" :
                    myaddr.sin_family == AF_INET6 ? "IPv6" :
                    "Unkn",
                    ntohs(myaddr.sin_port));
        }
    }
}
cli_cmd_tab_entry (
    "Debug IP Sockets",
    NULL,
    "Show IP socket state",
    CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_ipsockets,
    NULL,
    IP2_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

#if defined(VTSS_SW_OPTION_L3RT) && defined(VTSS_ARCH_JAGUAR_1)
static void cli_cmd_debug_lpm_stat_get_single(const vtss_vid_t vlan,
                                              const BOOL ipv4)
{
    vtss_rc rc;
    vtss_l3_counters_t c;

    rc = vtss_ip2_chip_counters_vlan_get(vlan, &c);
    if (rc != VTSS_RC_OK) {
        CPRINTF("Failed to get counters on vlan %u\n", vlan);
        return;
    }

    if (ipv4) {
        CPRINTF("IPv4 %4u %12llu %15llu %12llu %15llu\n",
                vlan,
                c.ipv4uc_received_frames,
                c.ipv4uc_received_octets,
                c.ipv4uc_transmitted_frames,
                c.ipv4uc_transmitted_octets);
    } else {
        CPRINTF("IPv6 %4u %12llu %15llu %12llu %15llu\n",
                vlan,
                c.ipv6uc_received_frames,
                c.ipv6uc_received_octets,
                c.ipv6uc_transmitted_frames,
                c.ipv6uc_transmitted_octets);
    }
}
static void cli_cmd_debug_lpm_stat_get(cli_req_t *req, BOOL ipv4)
{
    IP2_cli_req_t *r = (IP2_cli_req_t *)req->module_req;

    CPRINTF("TYPE VLAN RX-Frames    RX-Octets       TX-Frames    TX-Octets\n");
    CPRINTF("---- ---- ------------ --------------- ------------ ---------------\n");

    if (r->vlan_list_spec == CLI_SPEC_VAL) {
        vtss_vid_t vlan;
        for (vlan = 1; vlan < VTSS_VIDS; ++vlan) {
            if (VTSS_BF_GET(r->vlan_list, vlan)) {
                cli_cmd_debug_lpm_stat_get_single(vlan, ipv4);
            }
        }

    } else {
        vtss_if_id_vlan_t cur = 0, next = 0;
        while (vtss_ip2_if_id_next(cur, &next) == VTSS_RC_OK) {
            cli_cmd_debug_lpm_stat_get_single(next, ipv4);
            cur = next;
        }
    }
}

static void cli_cmd_debug_lpm_stat_get4(cli_req_t *req)
{
    cli_cmd_debug_lpm_stat_get(req, TRUE);
}
cli_cmd_tab_entry (
    "Debug LPM STAT ipv4_get [<vlan_list>]",
    NULL,
    "HW router counter",
    CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_lpm_stat_get4,
    NULL,
    IP2_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

static void cli_cmd_debug_lpm_stat_get6(cli_req_t *req)
{
    cli_cmd_debug_lpm_stat_get(req, FALSE);
}
cli_cmd_tab_entry (
    "Debug LPM STAT ipv6_get [<vlan_list>]",
    NULL,
    "HW router counter",
    CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_lpm_stat_get6,
    NULL,
    IP2_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

static void cli_cmd_debug_lpm_stat_clear_single(vtss_vid_t vlan)
{
    vtss_rc rc;

    rc = vtss_ip2_chip_counters_vlan_clear(vlan);
    if (rc != VTSS_RC_OK) {
        CPRINTF("Failed to clear counters on vlan %u\n", vlan);
    }
}
static void cli_cmd_debug_lpm_stat_clear(cli_req_t *req)
{
    IP2_cli_req_t *r = (IP2_cli_req_t *)req->module_req;

    if (r->vlan_list_spec == CLI_SPEC_VAL) {
        vtss_vid_t vlan = 0;
        for (vlan = 1; vlan < VTSS_VIDS; ++vlan) {
            if (VTSS_BF_GET(r->vlan_list, vlan)) {
                cli_cmd_debug_lpm_stat_clear_single(vlan);
            }
        }

    } else {
        vtss_if_id_vlan_t cur = 0, next = 0;
        while (vtss_ip2_if_id_next(cur, &next) == VTSS_RC_OK) {
            cli_cmd_debug_lpm_stat_clear_single(next);
            cur = next;
        }
    }
}
cli_cmd_tab_entry (
    "Debug lpm stat clr [<vlan_list>]",
    NULL,
    "HW router counter",
    CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_lpm_stat_clear,
    NULL,
    IP2_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

vtss_rc vtss_l3_debug_sticky_clear(const vtss_inst_t inst);
static void cli_cmd_debug_lpm_sticky_clear(cli_req_t *req)
{
    (void)vtss_l3_debug_sticky_clear(0);
}
cli_cmd_tab_entry (
    "Debug lpm stat sticky clear",
    NULL,
    "Clear lpm related sticky bits",
    CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_lpm_sticky_clear,
    NULL,
    IP2_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

void vtss_ip2_lpm_usage(u32 *lpm_entries, u32 *arp_entries);
static void cli_cmd_debug_lpm_usage(cli_req_t *req)
{
    u32 lpm, arp;
    vtss_ip2_lpm_usage(&lpm, &arp);
    CPRINTF("LPM: %u, ARP: %u\n", lpm, arp);
}
cli_cmd_tab_entry (
    "Debug lpm usage",
    NULL,
    "LPM usage according to book-keeping function",
    CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_lpm_usage,
    NULL,
    IP2_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif /* VTSS_SW_OPTION_L3RT && VTSS_ARCH_JAGUAR_1*/

#ifdef VTSS_SW_OPTION_SNMP
static void cli_cmd_debug_ip2_global_interface_table_changed(cli_req_t *req)
{
    u64 t;

    if (vtss_ip2_interfaces_last_change(&t) == VTSS_RC_OK) {
        CPRINTF("%llu\n", t);
    } else {
        CPRINTF("Failed\n");
    }
}
cli_cmd_tab_entry (
    "Debug ip2 global interface table changed",
    NULL,
    "",
    CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_ip2_global_interface_table_changed,
    NULL,
    IP2_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

static void cli_cmd_debug_ip2_vlan_ipv4_created(cli_req_t *req)
{
    u64 t;
    IP2_cli_req_t *r = (IP2_cli_req_t *)req->module_req;

    if (vtss_ip2_address_created_ipv4(r->vlan, &t) == VTSS_RC_OK) {
        CPRINTF("%llu\n", t);
    } else {
        CPRINTF("Failed\n");
    }
}
cli_cmd_tab_entry (
    "Debug ip2 vlan ipv4 created <vlan>",
    NULL,
    "",
    CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_ip2_vlan_ipv4_created,
    NULL,
    IP2_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

static void cli_cmd_debug_ip2_vlan_ipv4_changed(cli_req_t *req)
{
    u64 t;
    IP2_cli_req_t *r = (IP2_cli_req_t *)req->module_req;

    if (vtss_ip2_address_changed_ipv4(r->vlan, &t) == VTSS_RC_OK) {
        CPRINTF("%llu\n", t);
    } else {
        CPRINTF("Failed\n");
    }
}
cli_cmd_tab_entry (
    "Debug ip2 vlan ipv4 changed <vlan>",
    NULL,
    "",
    CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_ip2_vlan_ipv4_changed,
    NULL,
    IP2_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

static void cli_cmd_debug_ip2_vlan_ipv6_created(cli_req_t *req)
{
    u64 t;
    IP2_cli_req_t *r = (IP2_cli_req_t *)req->module_req;

    if (vtss_ip2_address_created_ipv6(r->vlan, &t) == VTSS_RC_OK) {
        CPRINTF("%llu\n", t);
    } else {
        CPRINTF("Failed\n");
    }
}
cli_cmd_tab_entry (
    "Debug ip2 vlan ipv6 created <vlan>",
    NULL,
    "",
    CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_ip2_vlan_ipv6_created,
    NULL,
    IP2_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

static void cli_cmd_debug_ip2_vlan_ipv6_changed(cli_req_t *req)
{
    u64 t;
    IP2_cli_req_t *r = (IP2_cli_req_t *)req->module_req;

    if (vtss_ip2_address_changed_ipv6(r->vlan, &t) == VTSS_RC_OK) {
        CPRINTF("%llu\n", t);
    } else {
        CPRINTF("Failed\n");
    }
}
cli_cmd_tab_entry (
    "Debug ip2 vlan ipv6 changed <vlan>",
    NULL,
    "",
    CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_ip2_vlan_ipv6_changed,
    NULL,
    IP2_cli_parm_table,
    CLI_CMD_FLAG_NONE
);






//static fd mfd;
//static struct ip_mreq mreq;
//
//static void cli_cmd_mcast_start() {
//    int res;
//
//    mfd = socket(AF_INET, SOCK_DGRAM, 0);
//    CPRINTF("Socket = %d\n", mfd);
//
//    memset(&mreq, 0, sizeof(mreq));
//    mreq.imr_multiaddr.s_addr = inet_addr("224.1.2.3");
//
//    res = setsockopt(s, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
//    CPRINTF("ip_add_membership = %d\n", res);
//
//    res = setsockopt(s, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse));
//    CPRINTF("so_reuseport = %d\n", res);
//
//}
//
//cli_cmd_tab_entry (
//    "Debug ip2 mcast start",
//    NULL,
//    "",
//    CLI_CMD_SORT_KEY_DEFAULT,
//    CLI_CMD_TYPE_STATUS,
//    VTSS_MODULE_ID_DEBUG,
//    cli_cmd_mcast_start,
//    NULL,
//    IP2_cli_parm_table,
//    CLI_CMD_FLAG_NONE
//);
//
//static void cli_cmd_mcast_end() {
//    int res;
//
//    memset(&mreq, 0, sizeof(mreq));
//    mreq.imr_multiaddr.s_addr = inet_addr("224.1.2.3");
//    res = setsockopt(s, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq));
//    CPRINTF("ip_drop_membership = %d\n", res);
//
//    res = close(mfd);
//    CPRINTF("close = %d\n", res);
//}
//
//cli_cmd_tab_entry (
//    "Debug ip2 mcast end",
//    NULL,
//    "",
//    CLI_CMD_SORT_KEY_DEFAULT,
//    CLI_CMD_TYPE_STATUS,
//    VTSS_MODULE_ID_DEBUG,
//    cli_cmd_mcast_end,
//    NULL,
//    IP2_cli_parm_table,
//    CLI_CMD_FLAG_NONE
//);






#endif /* VTSS_SW_OPTION_SNMP */

