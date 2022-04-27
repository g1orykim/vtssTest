/*

 Vitesse Switch API software.

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
#include "cli.h"
#include "cli_grp_help.h"
#include "access_mgmt_cli.h"
#include "access_mgmt_api.h"
#include "cli_trace_def.h"
#include "vlan_api.h"

#ifdef VTSS_CLI_SEC_GRP
#define GRP_CLI_PATH    VTSS_CLI_GRP_SEC_SWITCH_PATH
#else
#define GRP_CLI_PATH    "System "
#endif

typedef struct {
    vtss_ipv4_t ipv4_addr_1;
#ifdef VTSS_SW_OPTION_IPV6
    vtss_ipv6_t ipv6_addr_start;
    vtss_ipv6_t ipv6_addr_end;
#endif /* VTSS_SW_OPTION_IPV6 */
    u32         service_type;
    u32         entry_idx;
} access_mgmt_cli_req_t;

static void ACCESS_MGMT_cli_cmd_lookup(cli_req_t *req);

/****************************************************************************/
/*  Initialization                                                          */
/****************************************************************************/

void access_mgmt_cli_init(void)
{
    /* register the size required for access_mgmt req. structure */
    cli_req_size_register(sizeof(access_mgmt_cli_req_t));
}

/****************************************************************************/
/*  Command functions                                                       */
/****************************************************************************/

static void ACCESS_MGMT_cli_cmd_conf(cli_req_t *req, BOOL mode)
{
    vtss_rc             rc;
    access_mgmt_conf_t  conf;

    if (cli_cmd_switch_none(req) ||
        cli_cmd_conf_slave(req) ||
        (access_mgmt_conf_get(&conf) != VTSS_OK)) {
        return;
    }

    if (req->set) {
        if (mode) {
            conf.mode = req->enable;
            if ((rc = access_mgmt_conf_set(&conf)) != VTSS_OK) {
                CPRINTF("%s\n", error_txt(rc));
            }
        }
    } else {
        CPRINTF("System Access Mode : %s\n", cli_bool_txt(conf.mode));
        if (!mode) {
            ACCESS_MGMT_cli_cmd_lookup(req);
        }
    }
}

/*lint -e{525} */
static void ACCESS_MGMT_cli_cmd_add(cli_req_t *req, BOOL entry_type)
{
    vtss_rc rc;
    access_mgmt_entry_t conf;
    access_mgmt_cli_req_t *access_mgmt_req = req->module_req;
    int duplicated_id;

    if (cli_cmd_switch_none(req) || cli_cmd_slave(req)) {
        return;
    }

    memset(&conf, 0x0, sizeof(conf));
    conf.valid = 1;
    conf.vid = req->vid;

    if (access_mgmt_req->service_type == 0) {
        access_mgmt_req->service_type = ACCESS_MGMT_SERVICES_TYPE;
    }
    conf.service_type = access_mgmt_req->service_type;
    conf.entry_type = entry_type;
    if (conf.entry_type == ACCESS_MGMT_ENTRY_TYPE_IPV4) {
        conf.start_ip = req->ipv4_addr;
        conf.end_ip = access_mgmt_req->ipv4_addr_1;
    }
#ifdef VTSS_SW_OPTION_IPV6
    else {
        conf.start_ipv6 = access_mgmt_req->ipv6_addr_start;
        conf.end_ipv6 = access_mgmt_req->ipv6_addr_end;
    }
#endif /* VTSS_SW_OPTION_IPV6 */

    // check if need swap
    if (conf.entry_type == ACCESS_MGMT_ENTRY_TYPE_IPV4) {
        if (conf.start_ip > conf.end_ip) {
            vtss_ipv4_t temp_ip;
            temp_ip = conf.start_ip;
            conf.start_ip = conf.end_ip;
            conf.end_ip = temp_ip;
        }
    }
#ifdef VTSS_SW_OPTION_IPV6
    else {
        if (memcmp(&conf.start_ipv6, &conf.end_ipv6, sizeof(vtss_ipv6_t)) > 0) {
            vtss_ipv6_t temp_ipv6;
            memcpy(&temp_ipv6, &conf.start_ipv6, sizeof(vtss_ipv6_t));
            memcpy(&conf.start_ipv6, &conf.end_ipv6, sizeof(vtss_ipv6_t));
            memcpy(&conf.end_ipv6, &temp_ipv6, sizeof(vtss_ipv6_t));
        }
    }
#endif /* VTSS_SW_OPTION_IPV6 */

    if ((duplicated_id = access_mgmt_entry_content_is_duplicated((int) access_mgmt_req->entry_idx, &conf)) != 0) {
        CPRINTF("The entry content is duplicated of entry ID %d\n", duplicated_id);
        return;
    }

    if ((rc = access_mgmt_entry_add((int) access_mgmt_req->entry_idx, &conf)) != VTSS_OK) {
        CPRINTF("%s\n", error_txt(rc));
    }
}

static void ACCESS_MGMT_cli_cmd_config(cli_req_t *req)
{
    if (!req->set) {
        cli_header("Access Mgmt Configuration", 1);
    }
    ACCESS_MGMT_cli_cmd_conf(req, 0);
}

static void ACCESS_MGMT_cli_cmd_mode(cli_req_t *req)
{
    ACCESS_MGMT_cli_cmd_conf(req, 1);
}

static void ACCESS_MGMT_cli_cmd_add_v4(cli_req_t *req)
{
    ACCESS_MGMT_cli_cmd_add(req, ACCESS_MGMT_ENTRY_TYPE_IPV4);
}

#ifdef VTSS_SW_OPTION_IPV6
static void ACCESS_MGMT_cli_cmd_add_v6(cli_req_t *req)
{
    ACCESS_MGMT_cli_cmd_add(req, ACCESS_MGMT_ENTRY_TYPE_IPV6);
}
#endif /* VTSS_SW_OPTION_IPV6 */

static void ACCESS_MGMT_cli_cmd_del(cli_req_t *req)
{
    vtss_rc                 rc;
    access_mgmt_cli_req_t   *access_mgmt_req = req->module_req;
    access_mgmt_entry_t     conf;

    if (cli_cmd_switch_none(req) || cli_cmd_slave(req)) {
        return;
    }

    if (req->set) {
        if (access_mgmt_entry_get((int) access_mgmt_req->entry_idx, &conf) == VTSS_OK &&
            !conf.valid) {
            CPRINTF("Non-existing entry ID %d\n", access_mgmt_req->entry_idx);
            return;
        }
        if ((rc = access_mgmt_entry_del(access_mgmt_req->entry_idx)) != VTSS_OK) {
            CPRINTF("%s\n", error_txt(rc));
        }
    }
}

static void ACCESS_MGMT_cli_cmd_lookup(cli_req_t *req)
{
    access_mgmt_conf_t  conf;
    int                 access_id;
    char                ip_buf1[40], ip_buf2[40];
    access_mgmt_cli_req_t *access_mgmt_req = req->module_req;

    if (cli_cmd_switch_none(req) ||
        cli_cmd_slave(req) ||
        (access_mgmt_conf_get(&conf) != VTSS_OK)) {
        return;
    }

    if (req->set && !conf.entry[access_mgmt_req->entry_idx].valid) {
        CPRINTF("Non-existing entry ID %d\n", access_mgmt_req->entry_idx);
        return;
    }

    CPRINTF("W: WEB/HTTPS\n");
#if defined(VTSS_SW_OPTION_SNMP)
    CPRINTF("S: SNMP\n");
#endif /* VTSS_SW_OPTION_SNMP */
#if defined(VTSS_SW_OPTION_CLI_TELNET) || defined(VTSS_SW_OPTION_SSH)
    CPRINTF("T: TELNET/SSH\n\n");
#endif /* VTSS_SW_OPTION_CLI_TELNET || VTSS_SW_OPTION_SSH */

#ifdef VTSS_SW_OPTION_IPV6
    CPRINTF("Idx VID  Start IP Address                End IP Address                 W ");
#if defined(VTSS_SW_OPTION_SNMP)
    CPRINTF("S ");
#endif /* VTSS_SW_OPTION_SNMP */
#if defined(VTSS_SW_OPTION_CLI_TELNET) || defined(VTSS_SW_OPTION_SSH)
    CPRINTF("T ");
#endif /* VTSS_SW_OPTION_CLI_TELNET || VTSS_SW_OPTION_SSH */
    CPRINTF("\n");
    CPRINTF("--- ---  ------------------------------- ------------------------------ - ");
#if defined(VTSS_SW_OPTION_SNMP)
    CPRINTF("- ");
#endif /* VTSS_SW_OPTION_SNMP */
#if defined(VTSS_SW_OPTION_CLI_TELNET) || defined(VTSS_SW_OPTION_SSH)
    CPRINTF("- ");
#endif /* VTSS_SW_OPTION_CLI_TELNET || VTSS_SW_OPTION_SSH */
    CPRINTF("\n");
#else
    CPRINTF("Idx VID  Start IP Address End IP Address  W ");
#if defined(VTSS_SW_OPTION_SNMP)
    CPRINTF("S ");
#endif /* VTSS_SW_OPTION_SNMP */
#if defined(VTSS_SW_OPTION_CLI_TELNET) || defined(VTSS_SW_OPTION_SSH)
    CPRINTF("T ");
#endif /* VTSS_SW_OPTION_CLI_TELNET || VTSS_SW_OPTION_SSH */
    CPRINTF("\n");
    CPRINTF("--- ---  ---------------- --------------- - ");
#if defined(VTSS_SW_OPTION_SNMP)
    CPRINTF("- ");
#endif /* VTSS_SW_OPTION_SNMP */
#if defined(VTSS_SW_OPTION_CLI_TELNET) || defined(VTSS_SW_OPTION_SSH)
    CPRINTF("- ");
#endif /* VTSS_SW_OPTION_CLI_TELNET || VTSS_SW_OPTION_SSH */
    CPRINTF("\n");
#endif /* VTSS_SW_OPTION_IPV6 */

    for (access_id = ACCESS_MGMT_ACCESS_ID_START; access_id < ACCESS_MGMT_MAX_ENTRIES + ACCESS_MGMT_ACCESS_ID_START; access_id++) {
        if ((!req->set || (req->set && access_id == access_mgmt_req->entry_idx)) && conf.entry[access_id].valid) {
            CPRINTF("%-3d ", access_id);
            CPRINTF("%-4d ", conf.entry[access_id].vid);

#ifdef VTSS_SW_OPTION_IPV6
            CPRINTF("%-31s %-30s ",
                    conf.entry[access_id].entry_type == ACCESS_MGMT_ENTRY_TYPE_IPV4 ? misc_ipv4_txt(conf.entry[access_id].start_ip, ip_buf1) : misc_ipv6_txt(&conf.entry[access_id].start_ipv6, ip_buf1),
                    conf.entry[access_id].entry_type == ACCESS_MGMT_ENTRY_TYPE_IPV4 ? misc_ipv4_txt(conf.entry[access_id].end_ip,   ip_buf2) : misc_ipv6_txt(&conf.entry[access_id].end_ipv6,   ip_buf2));
#else
            CPRINTF("%-16s %-15s ",
                    misc_ipv4_txt(conf.entry[access_id].start_ip, ip_buf1),
                    misc_ipv4_txt(conf.entry[access_id].end_ip,   ip_buf2));
#endif /* VTSS_SW_OPTION_IPV6 */
            CPRINTF("%s ",
                    conf.entry[access_id].service_type & ACCESS_MGMT_SERVICES_TYPE_WEB ? "Y" : "N");
#if defined(VTSS_SW_OPTION_SNMP)
            CPRINTF("%s ",
                    conf.entry[access_id].service_type & ACCESS_MGMT_SERVICES_TYPE_SNMP ? "Y" : "N");
#endif /* VTSS_SW_OPTION_SNMP */
#if defined(VTSS_SW_OPTION_CLI_TELNET) || defined(VTSS_SW_OPTION_SSH)
            CPRINTF("%s ",
                    conf.entry[access_id].service_type & ACCESS_MGMT_SERVICES_TYPE_TELNET ? "Y" : "N");
#endif /* VTSS_SW_OPTION_CLI_TELNET || VTSS_SW_OPTION_SSH */
            CPRINTF("\n");
            if (req->set) {
                break;
            }
        }
    }
}

static void ACCESS_MGMT_cli_cmd_clear(cli_req_t *req)
{
    vtss_rc rc;

    if (cli_cmd_switch_none(req) || cli_cmd_slave(req)) {
        return;
    }

    if ((rc = access_mgmt_entry_clear()) != VTSS_OK) {
        CPRINTF("%s\n", error_txt(rc));
    }
}

static void ACCESS_MGMT_cli_cmd_stats(cli_req_t *req)
{
    access_mgmt_stats_t stats;

    if (cli_cmd_switch_none(req) || cli_cmd_slave(req)) {
        return;
    }

    if (req->clear) {
        access_mgmt_stats_clear();
    } else {
        access_mgmt_stats_get(&stats);
        CPRINTF("\nAccess Management Statistics:");
        CPRINTF("\n-----------------------------");
        CPRINTF("\nHTTP     Receive: %10d    Allow: %10d    Discard: %10d",     stats.http_receive_cnt,      stats.http_receive_cnt      - stats.http_discard_cnt,   stats.http_discard_cnt);
#if defined(VTSS_SW_OPTION_HTTPS)
        CPRINTF("\nHTTPS    Receive: %10d    Allow: %10d    Discard: %10d",     stats.https_receive_cnt,     stats.https_receive_cnt     - stats.https_discard_cnt,  stats.https_discard_cnt);
#endif /* VTSS_SW_OPTION_HTTPS */
#if defined(VTSS_SW_OPTION_SNMP)
        CPRINTF("\nSNMP     Receive: %10d    Allow: %10d    Discard: %10d",     stats.snmp_receive_cnt,      stats.snmp_receive_cnt      - stats.snmp_discard_cnt,   stats.snmp_discard_cnt);
#endif /* VTSS_SW_OPTION_SNMP */
#if defined(VTSS_SW_OPTION_CLI_TELNET)
        CPRINTF("\nTELNET   Receive: %10d    Allow: %10d    Discard: %10d",     stats.telnet_receive_cnt,    stats.telnet_receive_cnt    - stats.telnet_discard_cnt, stats.telnet_discard_cnt);
#endif /* VTSS_SW_OPTION_CLI_TELNET */
#if defined(VTSS_SW_OPTION_SSH)
        CPRINTF("\nSSH      Receive: %10d    Allow: %10d    Discard: %10d\n",   stats.ssh_receive_cnt,       stats.ssh_receive_cnt       - stats.ssh_discard_cnt,    stats.ssh_discard_cnt);
#endif /* VTSS_SW_OPTION_SSH */
    }
}

/****************************************************************************/
/*  Parameter functions                                                     */
/****************************************************************************/

static int32_t ACCESS_MGMT_cli_access_id_parse(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;
    u32     value = 0;
    access_mgmt_cli_req_t *access_mgmt_req = req->module_req;

    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &value, ACCESS_MGMT_ACCESS_ID_START, ACCESS_MGMT_MAX_ENTRIES);
    access_mgmt_req->entry_idx = value;

    return error;
}

static int32_t ACCESS_MGMT_cli_vid_parse(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;
    u32     value = 0;

    req->parm_parsed = 1;
    if ((error = cli_parse_ulong(cmd, &value, VLAN_ID_MIN, VLAN_ID_MAX)) == 0) {
        req->vid = (u16) value;
    }

    return error;
}

static int32_t ACCESS_MGMT_cli_start_ip_addr_parse(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t     error = 0;
    vtss_ipv4_t mask = 0xFFFFFFFF;

    req->parm_parsed = 1;
    error = cli_parse_ipv4(cmd, &req->ipv4_addr, &mask, &req->ipv4_addr_spec, 0);

    return error;
}

static int32_t ACCESS_MGMT_cli_end_ip_addr_parse(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t                 error = 0;
    vtss_ipv4_t             mask = 0xFFFFFFFF;
    access_mgmt_cli_req_t   *access_mgmt_req = req->module_req;

    req->parm_parsed = 1;
    error = cli_parse_ipv4(cmd, &access_mgmt_req->ipv4_addr_1, &mask, &req->ipv4_addr_spec, 0);

    return error;
}

#ifdef VTSS_SW_OPTION_IPV6
static int32_t ACCESS_MGMT_cli_start_ipv6_addr_parse(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;
    access_mgmt_cli_req_t *access_mgmt_req = req->module_req;

    req->parm_parsed = 1;
    error = cli_parse_ipv6(cmd, &access_mgmt_req->ipv6_addr_start, &req->ipv6_addr_spec);

    return error;
}

static int32_t ACCESS_MGMT_cli_end_ipv6_addr_parse(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;
    access_mgmt_cli_req_t *access_mgmt_req = req->module_req;

    req->parm_parsed = 1;
    error = cli_parse_ipv6(cmd, &access_mgmt_req->ipv6_addr_end, &req->ipv6_addr_spec);

    return error;
}
#endif /*VTSS_SW_OPTION_IPV6*/

static int32_t ACCESS_MGMT_cli_parse_keyword(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    char *found = cli_parse_find(cmd, stx);
    access_mgmt_cli_req_t *access_mgmt_req = req->module_req;

    req->parm_parsed = 1;

    if (found != NULL) {
        if (!strncmp(found, "web", 3)) {
            access_mgmt_req->service_type |= ACCESS_MGMT_SERVICES_TYPE_WEB;
#if defined(VTSS_SW_OPTION_SNMP)
        } else if (!strncmp(found, "snmp", 4)) {
            access_mgmt_req->service_type |= ACCESS_MGMT_SERVICES_TYPE_SNMP;
#endif /* VTSS_SW_OPTION_SNMP */
#if defined(VTSS_SW_OPTION_CLI_TELNET) || defined(VTSS_SW_OPTION_SSH)
        } else if (!strncmp(found, "telnet", 6)) {
            access_mgmt_req->service_type |= ACCESS_MGMT_SERVICES_TYPE_TELNET;
#endif /* VTSS_SW_OPTION_CLI_TELNET || VTSS_SW_OPTION_SSH */
        }
    }

    return (found == NULL ? 1 : 0);
}

/****************************************************************************/
/*  Parameter table                                                         */
/****************************************************************************/

static cli_parm_t access_mgmt_cli_parm_table[] = {
    {
        "enable|disable",
        "enable : Enable access management\n"
        "disable: Disable access management\n"
        "(default: Show access management mode)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        ACCESS_MGMT_cli_cmd_mode
    },
    {
        "<access_id>",
        "entry index (1-16)",
        CLI_PARM_FLAG_SET,
        ACCESS_MGMT_cli_access_id_parse,
        NULL
    },
    {
        "<vid>",
        "VLAN ID (1-4095)",
        CLI_PARM_FLAG_SET,
        ACCESS_MGMT_cli_vid_parse,
        NULL
    },
    {
        "<start_ip_addr>",
        "Start IP address (a.b.c.d)",
        CLI_PARM_FLAG_SET,
        ACCESS_MGMT_cli_start_ip_addr_parse,
        NULL
    },
    {
        "<end_ip_addr>",
        "End IP address (a.b.c.d)",
        CLI_PARM_FLAG_SET,
        ACCESS_MGMT_cli_end_ip_addr_parse,
        NULL
    },
    {
        "clear",
        "Clear access management statistics",
        CLI_PARM_FLAG_NONE,
        cli_parm_parse_keyword,
        ACCESS_MGMT_cli_cmd_stats
    },
    {
        "web",
        "Indicates that the host can access the switch from HTTP/HTTPS",
        CLI_PARM_FLAG_NONE,
        ACCESS_MGMT_cli_parse_keyword,
        NULL
    },
#if defined(VTSS_SW_OPTION_SNMP)
    {
        "snmp",
        "Indicates that the host can access the switch from SNMP",
        CLI_PARM_FLAG_NONE,
        ACCESS_MGMT_cli_parse_keyword,
        NULL
    },
#endif /* VTSS_SW_OPTION_SNMP */
#if defined(VTSS_SW_OPTION_CLI_TELNET) || defined(VTSS_SW_OPTION_SSH)
    {
        "telnet",
        "Indicates that the host can access the switch from TELNET/SSH",
        CLI_PARM_FLAG_NONE,
        ACCESS_MGMT_cli_parse_keyword,
        NULL
    },
#endif /* VTSS_SW_OPTION_CLI_TELNET || VTSS_SW_OPTION_SSH */
#ifdef VTSS_SW_OPTION_IPV6
    {
        "<start_ipv6_addr>",
        "Start IPv6 address.\n"
        "              IPv6 address is in 128-bit records represented as eight fields of up to\n"
        "              four hexadecimal digits with a colon separates each field (:). For example,\n"
        "              'fe80::215:c5ff:fe03:4dc7'. The symbol '::' is a special syntax that can\n"
        "              be used as a shorthand way of representing multiple 16-bit groups of\n"
        "              contiguous zeros; but it can only appear once. It also used a following\n"
        "              legally IPv4 address. For example,'::192.1.2.34'.",
        CLI_PARM_FLAG_SET,
        ACCESS_MGMT_cli_start_ipv6_addr_parse,
        NULL

    },
    {
        "<end_ipv6_addr>",
        "End IPv6 address.\n"
        "              IPv6 address is in 128-bit records represented as eight fields of up to\n"
        "              four hexadecimal digits with a colon separates each field (:). For example,\n"
        "              'fe80::215:c5ff:fe03:4dc7'. The symbol '::' is a special syntax that can\n"
        "              be used as a shorthand way of representing multiple 16-bit groups of\n"
        "              contiguous zeros; but it can only appear once. It also used a following\n"
        "              legally IPv4 address. For example,'::192.1.2.34'.",
        CLI_PARM_FLAG_SET,
        ACCESS_MGMT_cli_end_ipv6_addr_parse,
        NULL
    },
#endif /* VTSS_SW_OPTION_IPV6 */
    {NULL, NULL, 0, 0, NULL}
};

/****************************************************************************/
/*  Command table                                                           */
/****************************************************************************/

enum {
    ACCESS_MGMT_PRIO_CONF,
    ACCESS_MGMT_PRIO_MODE,
    ACCESS_MGMT_PRIO_ADD,
    ACCESS_MGMT_PRIO_ADD_V6,
    ACCESS_MGMT_PRIO_DEL,
    ACCESS_MGMT_PRIO_LOOKUP,
    ACCESS_MGMT_PRIO_CLEAR,
    ACCESS_MGMT_PRIO_STATS
};

cli_cmd_tab_entry(
    GRP_CLI_PATH "Access Configuration",
    NULL,
    "Show access management configuration",
    ACCESS_MGMT_PRIO_CONF,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_SECURITY,
    ACCESS_MGMT_cli_cmd_config,
    NULL,
    access_mgmt_cli_parm_table,
    CLI_CMD_FLAG_SYS_CONF
);

cli_cmd_tab_entry(
    GRP_CLI_PATH "Access Mode",
    GRP_CLI_PATH "Access Mode [enable|disable]",
    "Set or show the access management mode",
    ACCESS_MGMT_PRIO_MODE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SECURITY,
    ACCESS_MGMT_cli_cmd_mode,
    NULL,
    access_mgmt_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

CLI_CMD_TAB_ENTRY_DECL(ACCESS_MGMT_cli_cmd_add_v4) = {
    NULL,
    GRP_CLI_PATH "Access Add <access_id>"
    " <vid> <start_ip_addr> <end_ip_addr> [web]"
#if defined(VTSS_SW_OPTION_SNMP)
    " [snmp]"
#endif /* VTSS_SW_OPTION_SNMP */
#if defined(VTSS_SW_OPTION_CLI_TELNET) || defined(VTSS_SW_OPTION_SSH)
    " [telnet]"
#endif /* VTSS_SW_OPTION_CLI_TELNET || VTSS_SW_OPTION_SSH */
    ,
    "Add access management entry, default: Add all supported protocols",
    ACCESS_MGMT_PRIO_ADD,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SECURITY,
    ACCESS_MGMT_cli_cmd_add_v4,
    NULL,
    access_mgmt_cli_parm_table,
    CLI_CMD_FLAG_NONE
};

#ifdef VTSS_SW_OPTION_IPV6
CLI_CMD_TAB_ENTRY_DECL(ACCESS_MGMT_cli_cmd_add_v6) = {
    NULL,
    GRP_CLI_PATH "Access Ipv6 Add <access_id>"
    " <vid> <start_ipv6_addr> <end_ipv6_addr> [web]"
#if defined(VTSS_SW_OPTION_SNMP)
    " [snmp]"
#endif /* VTSS_SW_OPTION_SNMP */
#if defined(VTSS_SW_OPTION_CLI_TELNET) || defined(VTSS_SW_OPTION_SSH)
    " [telnet]"
#endif /* VTSS_SW_OPTION_CLI_TELNET || VTSS_SW_OPTION_SSH */
    ,
    "Add access management IPv6 entry, default: Add all supported protocols",
    ACCESS_MGMT_PRIO_ADD_V6,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SECURITY,
    ACCESS_MGMT_cli_cmd_add_v6,
    NULL,
    access_mgmt_cli_parm_table,
    CLI_CMD_FLAG_NONE
};
#endif /* VTSS_SW_OPTION_IPV6 */

cli_cmd_tab_entry(
    NULL,
    GRP_CLI_PATH "Access Delete <access_id>",
    "Delete access management entry",
    ACCESS_MGMT_PRIO_DEL,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SECURITY,
    ACCESS_MGMT_cli_cmd_del,
    NULL,
    access_mgmt_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    GRP_CLI_PATH "Access Lookup [<access_id>]",
    NULL,
    "Lookup access management entry",
    ACCESS_MGMT_PRIO_LOOKUP,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_SECURITY,
    ACCESS_MGMT_cli_cmd_lookup,
    NULL,
    access_mgmt_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    NULL,
    GRP_CLI_PATH "Access Clear",
    "Clear access management entry",
    ACCESS_MGMT_PRIO_CLEAR,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SECURITY,
    ACCESS_MGMT_cli_cmd_clear,
    NULL,
    access_mgmt_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    GRP_CLI_PATH "Access Statistics",
    GRP_CLI_PATH "Access Statistics [clear]",
    "Show or clear access management statistics",
    ACCESS_MGMT_PRIO_STATS,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_SECURITY,
    ACCESS_MGMT_cli_cmd_stats,
    NULL,
    access_mgmt_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
