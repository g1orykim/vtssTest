/*

 Vitesse Switch API software.

 Copyright (c) 2002-2012 Vitesse Semiconductor Corporation "Vitesse". All
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
#include "cli.h"
#include "cli_grp_help.h"
#include "cli_api.h"
#include "misc_api.h"
#include "ip_source_guard_api.h"
#include "ip_source_guard_cli.h"
#include "cli_trace_def.h"
#include "port_api.h"

typedef struct {
    /* Keywords */
    BOOL    del;
    BOOL    add;
    BOOL    dynamic;

    int     limit_number;
    int     dynamic_unlimited;
} ip_source_guard_cli_req_t;

void ips_cli_req_init(void)
{
    /* register the size required for port req. structure */
    cli_req_size_register(sizeof(ip_source_guard_cli_req_t));
}

static void cli_cmd_ip_source_guard_conf(cli_req_t *req,
                                         BOOL ip_source_guard_mode,
                                         BOOL ip_source_guard_port_mode,
                                         BOOL ip_source_guard_port_dym_limit,
                                         BOOL static_entry_set,
                                         BOOL status)
{
    vtss_rc                                     rc;
    vtss_isid_t                                 isid;
    vtss_uport_no_t                             uport;
    BOOL                                        first;
    ulong                                       mode;
    char                                        buf[80], buf1[80], buf2[80], *p;
    ip_source_guard_entry_t                     entry;
    ip_source_guard_port_mode_conf_t            port_mode_conf;
    ip_source_guard_port_dynamic_entry_conf_t   port_dynamic_entry;
    ip_source_guard_cli_req_t                   *ips_req = NULL;
    switch_iter_t                               sit;
    port_iter_t                                 pit;

    if (cli_cmd_switch_none(req) ||
        cli_cmd_conf_slave(req) ||
        ip_source_guard_mgmt_conf_get_mode(&mode) != VTSS_OK) {
        return;
    }

    if (req->set) {
        if (ip_source_guard_mode) {
            mode = req->enable;
            if ((rc = ip_source_guard_mgmt_conf_set_mode(mode)) != VTSS_OK) {
                CPRINTF("%s\n", error_txt(rc));
            }
        }
    } else {
        if (ip_source_guard_mode) {
            CPRINTF("IP Source Guard Mode : %s\n", cli_bool_txt(mode));
        }
    }

    ips_req = req->module_req;

    if (ip_source_guard_port_mode | ip_source_guard_port_dym_limit) {
        (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_USID);
        while (switch_iter_getnext(&sit)) {
            if ((isid = req->stack.isid[sit.usid]) == VTSS_ISID_END ||
                ip_source_guard_mgmt_conf_get_port_mode(isid, &port_mode_conf) != VTSS_OK  ||
                ip_source_guard_mgmt_conf_get_port_dynamic_entry_cnt(isid, &port_dynamic_entry) != VTSS_OK) {
                continue;
            }
            first = 1;

            (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT);
            while (port_iter_getnext(&pit)) {
                uport = iport2uport(pit.iport);

                if (req->uport_list[uport] == 0) {
                    continue;
                }

                if (port_isid_port_no_is_stack(isid, pit.iport)) {
                    continue;
                }

                if (req->set) {
                    if (ip_source_guard_port_mode) {
                        port_mode_conf.mode[pit.iport] = req->enable;
                    }
                    if (ip_source_guard_port_dym_limit) {
                        if (ips_req->dynamic_unlimited == 1) {
                            port_dynamic_entry.entry_cnt[pit.iport] = IP_SOURCE_GUARD_DYNAMIC_UNLIMITED;
                        } else {
                            port_dynamic_entry.entry_cnt[pit.iport] = ips_req->limit_number;
                        }
                    }
                } else {
                    if (first) {
                        first = 0;
                        cli_cmd_usid_print(sit.usid, req, 1);
                        p = &buf[0];
                        p += sprintf(p, "Port  ");
                        if (ip_source_guard_port_mode) {
                            p += sprintf(p, "Port Mode    ");
                        }
                        if (ip_source_guard_port_dym_limit) {
                            p += sprintf(p, "Dynamic Entry Limit    ");
                        }
                        cli_table_header(buf);
                    }
                    CPRINTF("%-2u    ", uport);
                    if (ip_source_guard_port_mode) {
                        CPRINTF("%s     ", cli_bool_txt(port_mode_conf.mode[pit.iport]));
                    }
                    if (ip_source_guard_port_dym_limit) {
                        if (port_dynamic_entry.entry_cnt[pit.iport] == IP_SOURCE_GUARD_DYNAMIC_UNLIMITED) {
                            CPRINTF("unlimited");
                        } else {
                            CPRINTF("%d    ", port_dynamic_entry.entry_cnt[pit.iport]);
                        }
                    }
                    CPRINTF("\n");
                }
            }
            if (req->set) {
                if (ip_source_guard_port_mode) {
                    if ((rc = ip_source_guard_mgmt_conf_set_port_mode(isid, &port_mode_conf)) != VTSS_OK) {
                        CPRINTF("%s\n", error_txt(rc));
                    }
                }
                if (ip_source_guard_port_dym_limit) {
                    if ((rc = ip_source_guard_mgmt_conf_set_port_dynamic_entry_cnt(isid, &port_dynamic_entry)) != VTSS_OK) {
                        CPRINTF("%s\n", error_txt(rc));
                    }
                }
            }
        }
    }

    // Add/Delete static entry
    if (static_entry_set) {
        (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_USID);
        while (switch_iter_getnext(&sit)) {
            if ((isid = req->stack.isid[sit.usid]) == VTSS_ISID_END) {
                continue;
            }
            first = 1;
            (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT);
            while (port_iter_getnext(&pit)) {
                uport = iport2uport(pit.iport);
                if (req->uport_list[uport] == 0) {
                    continue;
                }
                if (req->set) {
#if defined(VTSS_FEATURE_ACL_V2)
                    int i;
                    for (i = 0; i < 6; i++) {
                        entry.assigned_mac[i] = req->mac_addr[i];
                    }
#else
                    entry.ip_mask = req->ipv4_mask;
#endif /* VTSS_FEATURE_ACL_V2 */
                    entry.vid = req->vid;
                    entry.assigned_ip = req->ipv4_addr;
                    entry.isid = isid;
                    entry.port_no = pit.iport;
                    entry.type = IP_SOURCE_GUARD_STATIC_TYPE;
                    entry.valid = 1;
                    if (ips_req->del) {
                        if ((rc = ip_source_guard_mgmt_conf_del_static_entry(&entry)) != VTSS_OK) {
                            CPRINTF("%s\n", error_txt(rc));
                        }
                    } else {
                        if ((rc = ip_source_guard_mgmt_conf_set_static_entry(&entry)) != VTSS_OK) {
                            CPRINTF("%s\n", error_txt(rc));
                        }
                    }
                }
            }
        }
    }

    if (status) {
        (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_USID);
        while (switch_iter_getnext(&sit)) {
            if ((isid = req->stack.isid[sit.usid]) == VTSS_ISID_END) {
                continue;
            }
            first = 1;
            (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT);
            while (port_iter_getnext(&pit)) {
                uport = iport2uport(pit.iport);
                if (req->uport_list[uport] == 0) {
                    continue;
                }
                if (first) {
                    CPRINTF("%s", "\nIP Source Guard Entry Table:\n");
                    first = 0;
                    cli_cmd_usid_print(sit.usid, req, 1);
                    p = &buf[0];
                    p += sprintf(p, "Type     ");
                    p += sprintf(p, "Port  ");
                    p += sprintf(p, "VLAN  ");
                    p += sprintf(p, "IP Address       ");
#if defined(VTSS_FEATURE_ACL_V2)
                    p += sprintf(p, "MAC Address        ");
#else
                    p += sprintf(p, "IP Mask          ");
#endif /* VTSS_FEATURE_ACL_V2 */
                    cli_table_header(buf);
                }

                // Get static entries
                if (ip_source_guard_mgmt_conf_get_first_static_entry(&entry) == VTSS_OK) {
                    if ((entry.isid == isid) && (entry.port_no == pit.iport)) {
#if defined(VTSS_FEATURE_ACL_V2)
                        CPRINTF("Static   %4d  %4d  %-15s  %-15s\n",
                                iport2uport(entry.port_no),
                                entry.vid,
                                misc_ipv4_txt(entry.assigned_ip, buf1),
                                misc_mac_txt(entry.assigned_mac, buf2));
#else
                        CPRINTF("Static   %4d  %4d  %-15s  %-15s\n",
                                iport2uport(entry.port_no),
                                entry.vid,
                                misc_ipv4_txt(entry.assigned_ip, buf1),
                                misc_ipv4_txt(entry.ip_mask, buf2));
#endif /* VTSS_FEATURE_ACL_V2 */
                    }
                    while (ip_source_guard_mgmt_conf_get_next_static_entry(&entry) == VTSS_OK) {
                        if ((entry.isid == isid) && (entry.port_no == pit.iport)) {
#if defined(VTSS_FEATURE_ACL_V2)
                            CPRINTF("Static   %4d  %4d  %-15s  %-15s\n",
                                    iport2uport(entry.port_no),
                                    entry.vid,
                                    misc_ipv4_txt(entry.assigned_ip, buf1),
                                    misc_mac_txt(entry.assigned_mac, buf2));
#else
                            CPRINTF("Static   %4d  %4d  %-15s  %-15s\n",
                                    iport2uport(entry.port_no),
                                    entry.vid,
                                    misc_ipv4_txt(entry.assigned_ip, buf1),
                                    misc_ipv4_txt(entry.ip_mask, buf2));
#endif /* VTSS_FEATURE_ACL_V2 */

                        }
                    }
                }

                // Get dyanmic entries
                if (ip_source_guard_mgmt_conf_get_first_dynamic_entry(&entry) == VTSS_OK) {
                    if ((entry.isid == isid) && (entry.port_no == pit.iport)) {
#if defined(VTSS_FEATURE_ACL_V2)
                        CPRINTF("Dynamic  %4d  %4d  %-15s  %-15s\n",
                                iport2uport(entry.port_no),
                                entry.vid,
                                misc_ipv4_txt(entry.assigned_ip, buf1),
                                misc_mac_txt(entry.assigned_mac, buf2));
#else
                        CPRINTF("Dynamic  %4d  %4d  %-15s  %-15s\n",
                                iport2uport(entry.port_no),
                                entry.vid,
                                misc_ipv4_txt(entry.assigned_ip, buf1),
                                misc_ipv4_txt(entry.ip_mask, buf2));
#endif /* VTSS_FEATURE_ACL_V2 */
                    }
                    while (ip_source_guard_mgmt_conf_get_next_dynamic_entry(&entry) == VTSS_OK) {
                        if ((entry.isid == isid) && (entry.port_no == pit.iport)) {
#if defined(VTSS_FEATURE_ACL_V2)
                            CPRINTF("Dynamic  %4d  %4d  %-15s  %-15s\n",
                                    iport2uport(entry.port_no),
                                    entry.vid,
                                    misc_ipv4_txt(entry.assigned_ip, buf1),
                                    misc_mac_txt(entry.assigned_mac, buf2));
#else
                            CPRINTF("Dynamic  %4d  %4d  %-15s  %-15s\n",
                                    iport2uport(entry.port_no),
                                    entry.vid,
                                    misc_ipv4_txt(entry.assigned_ip, buf1),
                                    misc_ipv4_txt(entry.ip_mask, buf2));
#endif /* VTSS_FEATURE_ACL_V2 */
                        }
                    }
                }
            }
        }
    }

    /* translate dynamic entries into static entries */
    if (!ip_source_guard_mode && !ip_source_guard_port_mode &&
        !ip_source_guard_port_dym_limit && !static_entry_set && !status) {
        if ((rc = ip_source_guard_mgmt_conf_translate_dynamic_into_static()) >= VTSS_OK) {
            CPRINTF("IP Source Guard:\n\tTranslate %d dynamic entries into static entries.\n", rc);
        } else {
            CPRINTF("%s\n", error_txt(rc));
        }
    }
}

static void cli_cmd_ip_source_guard_config(cli_req_t *req)
{
    if (!req->set) {
        cli_header("IP Source guard Configuration", 1);
    }

    cli_cmd_ip_source_guard_conf(req, 1, 1, 1, 1, 1);
}

static void cli_cmd_ip_source_guard_mode(cli_req_t *req)
{
    cli_cmd_ip_source_guard_conf(req, 1, 0, 0, 0, 0);
}

static void cli_cmd_ip_source_guard_port_mode(cli_req_t *req)
{
    cli_cmd_ip_source_guard_conf(req, 0, 1, 0, 0, 0);
}

static void cli_cmd_ip_source_guard_port_dynlimit(cli_req_t *req)
{
    cli_cmd_ip_source_guard_conf(req, 0, 0, 1, 0, 0);
}

static void cli_cmd_ip_source_guard_entry(cli_req_t *req)
{
    cli_cmd_ip_source_guard_conf(req, 0, 0, 0, 1, 0);
}

static void cli_cmd_ip_source_guard_status(cli_req_t *req)
{
    cli_cmd_ip_source_guard_conf(req, 0, 0, 0, 0, 1);
}

static void cli_cmd_ip_source_guard_translate(cli_req_t *req)
{
    cli_cmd_ip_source_guard_conf(req, 0, 0, 0, 0, 0);
}

static int32_t cli_ips_parse_allowed_ipv4(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    req->parm_parsed = 1;
    return (cli_parse_ipv4(cmd, &req->ipv4_addr, NULL, &req->ipv4_addr_spec, 0));
}

#if defined(VTSS_FEATURE_ACL_V2)
static int32_t cli_generic_parse_allowed_mac(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    req->parm_parsed = 1;
    return (cli_parse_mac(cmd, req->mac_addr, &req->mac_addr_spec, 0));
}
#else
static int32_t cli_generic_parse_ipv4_mask(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    req->parm_parsed = 1;
    return (cli_parse_ipv4(cmd, &req->ipv4_mask, NULL, &req->ipv4_mask_spec, 1));
}
#endif /* VTSS_FEATURE_ACL_V2 */

static int32_t cli_ips_parse_keyword(char *cmd, char *cmd2, char *stx, char *cmd_org,
                                     cli_req_t *req)
{
    ip_source_guard_cli_req_t   *ips_req = NULL;
    char                        *found = cli_parse_find(cmd, stx);

    req->parm_parsed = 1;
    ips_req = req->module_req;

    if (found != NULL) {
        if (!strncmp(found, "add", 3)) {
            ips_req->add = 1;
        } else if (!strncmp(found, "delete", 6)) {
            ips_req->del = 1;
        } else if (!strncmp(found, "unlimited", 9)) {
            ips_req->dynamic_unlimited = 1;
        }
    }

    return (found == NULL ? 1 : 0);
}

static int32_t cli_dynamic_entry_parse_keyword(char *cmd, char *cmd2, char *stx, char *cmd_org,
                                               cli_req_t *req)
{
    ip_source_guard_cli_req_t   *ips_req = NULL;
    ulong                       value;
    ulong                       error = 0;

    req->parm_parsed = 1;
    ips_req = req->module_req;
    error = cli_parse_ulong(cmd, &value, 0, 2) &&
            cli_ips_parse_keyword(cmd, cmd2, stx, cmd_org, req);
    ips_req->limit_number = value;
    return (error);
}


static cli_parm_t ip_source_guard_cli_parm_table[] = {
    {
        "enable|disable",
        "enable : Enable IP Source Guard\n"
        "disable: Disable IP Source Guard",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        cli_cmd_ip_source_guard_mode,
    },
    {
        "enable|disable",
        "enable  : Enable IP Source Guard port\n"
        "disable : Disable IP Source Guard port\n"
        "(default: Show IP Source Guard port mode)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        cli_cmd_ip_source_guard_port_mode,
    },
    {
        "add|delete",
        "add    : Add new port IP source guard static entry\n"
        "delete : Delete existing port IP source guard static entry",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_ips_parse_keyword,
        cli_cmd_ip_source_guard_entry,
    },
    {
        "<dynamic_entry_limit>|unlimited",
        "dynamic entry limit (0-2) or unlimited",
        CLI_PARM_FLAG_SET,
        cli_dynamic_entry_parse_keyword,
        cli_cmd_ip_source_guard_port_dynlimit,
    },
    {
        "<vid>",
        "VLAN ID (1-4095)",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_vid,
        cli_cmd_ip_source_guard_entry,
    },
    {
        "<allowed_ip>",
        "IPv4 address (a.b.c.d), IP address allowed for doing IP source guard",
        CLI_PARM_FLAG_SET,
        cli_ips_parse_allowed_ipv4,
        cli_cmd_ip_source_guard_entry,
    },
#if defined(VTSS_FEATURE_ACL_V2)
    {
        "<allowed_mac>",
        "MAC address ('xx-xx-xx-xx-xx-xx' or 'xx.xx.xx.xx.xx.xx' or 'xxxxxxxxxxxx', x is a hexadecimal digit), MAC address allowed for doing IP source guard",
        CLI_PARM_FLAG_SET,
        cli_generic_parse_allowed_mac,
        cli_cmd_ip_source_guard_entry,
    },
#else
    {
        "<ip_mask>",
        "IPv4 mask (a.b.c.d), IP mask for allowed IP address",
        CLI_PARM_FLAG_SET,
        cli_generic_parse_ipv4_mask,
        cli_cmd_ip_source_guard_entry,
    },
#endif /* VTSS_FEATURE_ACL_V2 */
    {
        NULL,
        NULL,
        0,
        0,
        NULL
    },
};

enum {
    PRIO_IP_SOURCE_GUARD_CONF,
    PRIO_IP_SOURCE_GUARD_MODE,
    PRIO_IP_SOURCE_GUARD_PORT_MODE,
    PRIO_IP_SOURCE_GUARD_PORT_DYM_LIMIT,
    PRIO_IP_SOURCE_GUARD_STATIC_ENTRY_SET,
    PRIO_IP_SOURCE_GUARD_STATUS,
    PRIO_IP_SOURCE_GUARD_TRANSLATE
};

cli_cmd_tab_entry (
    VTSS_CLI_GRP_SEC_NETWORK_PATH "IP Source Guard Configuration",
    NULL,
    "Show IP source guard configuration",
    PRIO_IP_SOURCE_GUARD_CONF,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_SECURITY,
    cli_cmd_ip_source_guard_config,
    NULL,
    ip_source_guard_cli_parm_table,
    CLI_CMD_FLAG_SYS_CONF
);

cli_cmd_tab_entry (
    VTSS_CLI_GRP_SEC_NETWORK_PATH "IP Source Guard Mode",
    VTSS_CLI_GRP_SEC_NETWORK_PATH "IP Source Guard Mode [enable|disable]",
    "Set or show IP source guard mode",
    PRIO_IP_SOURCE_GUARD_MODE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SECURITY,
    cli_cmd_ip_source_guard_mode,
    NULL,
    ip_source_guard_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    VTSS_CLI_GRP_SEC_NETWORK_PATH "IP Source Guard Port Mode [<port_list>]",
    VTSS_CLI_GRP_SEC_NETWORK_PATH "IP Source Guard Port Mode [<port_list>] [enable|disable]",
    "Set or show the IP Source Guard port mode",
    PRIO_IP_SOURCE_GUARD_PORT_MODE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SECURITY,
    cli_cmd_ip_source_guard_port_mode,
    NULL,
    ip_source_guard_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    VTSS_CLI_GRP_SEC_NETWORK_PATH "IP Source Guard limit [<port_list>]",
    VTSS_CLI_GRP_SEC_NETWORK_PATH "IP Source Guard limit [<port_list>]\n"
    "        [<dynamic_entry_limit>|unlimited]",
    "Set or show the IP Source Guard port limitation for dynamic entries",
    PRIO_IP_SOURCE_GUARD_PORT_DYM_LIMIT,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SECURITY,
    cli_cmd_ip_source_guard_port_dynlimit,
    NULL,
    ip_source_guard_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

#if defined(VTSS_FEATURE_ACL_V2)
cli_cmd_tab_entry (
    NULL,
    VTSS_CLI_GRP_SEC_NETWORK_PATH "IP Source Guard Entry [<port_list>] add|delete\n"
    "        <vid> <allowed_ip> <allowed_mac>",
    "Add or delete IP source guard static entry",
    PRIO_IP_SOURCE_GUARD_STATIC_ENTRY_SET,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SECURITY,
    cli_cmd_ip_source_guard_entry,
    NULL,
    ip_source_guard_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#else
cli_cmd_tab_entry (
    NULL,
    VTSS_CLI_GRP_SEC_NETWORK_PATH "IP Source Guard Entry [<port_list>] add|delete\n"
    "        <vid> <allowed_ip> <ip_mask>",
    "Add or delete IP source guard static entry",
    PRIO_IP_SOURCE_GUARD_STATIC_ENTRY_SET,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SECURITY,
    cli_cmd_ip_source_guard_entry,
    NULL,
    ip_source_guard_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif /* VTSS_FEATURE_ACL_V2 */

cli_cmd_tab_entry (
    VTSS_CLI_GRP_SEC_NETWORK_PATH "IP Source Guard Status [<port_list>]",
    NULL,
    "Show IP source guard static and dynamic entries",
    PRIO_IP_SOURCE_GUARD_STATUS,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_SECURITY,
    cli_cmd_ip_source_guard_status,
    NULL,
    ip_source_guard_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    VTSS_CLI_GRP_SEC_NETWORK_PATH "IP Source Guard Translation",
    NULL,
    "Translate IP source guard dynamic entries into static entries",
    PRIO_IP_SOURCE_GUARD_TRANSLATE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SECURITY,
    cli_cmd_ip_source_guard_translate,
    NULL,
    ip_source_guard_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
