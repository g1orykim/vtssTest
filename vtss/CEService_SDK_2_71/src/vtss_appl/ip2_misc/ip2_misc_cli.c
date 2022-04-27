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
#include "mgmt_api.h"
#include "ping_api.h"

typedef struct {
    /* Ping Target */
    cli_spec_t target_spec;
    vtss_ip_addr_t target;
    /* Packet Length for ECHO-TX */
    BOOL                len_flag;
    ulong               packet_length;
    /* Packet Count for ECHO-TX */
    BOOL                cnt_flag;
    ulong               packet_count;
    /* Interval between ECHO-TX */
    BOOL                itv_flag;
    ulong               packet_interval;
} IP2_MISC_cli_req_t;

void vtss_ip2_misc_cli_init(void)
{
    cli_req_size_register(sizeof(IP2_MISC_cli_req_t));
}

static int32_t IP2_MISC_parse_keyword(char *cmd, char *cmd2, char *stx,
                                      char *cmd_org, cli_req_t *req)
{
    IP2_MISC_cli_req_t *r = req->module_req;
    char *found = cli_parse_find(cmd, stx);

    req->parm_parsed = 1;

    if (found && r) {
        if (!strncmp(found, "length", 6)) {
            r->len_flag = TRUE;
        } else if (!strncmp(found, "count", 5)) {
            r->cnt_flag = TRUE;
        } else if (!strncmp(found, "interval", 8)) {
            r->itv_flag = TRUE;
        } else {
            found = NULL;
        }
    }

    return (found == NULL ? 1 : 0);
}

static int32_t IP2_MISC_ping_length_parse(char *cmd, char *cmd2, char *stx,
                                          char *cmd_org, cli_req_t *req)
{
    IP2_MISC_cli_req_t *r = req->module_req;

    req->parm_parsed = 1;
    return cli_parse_ulong(cmd, &r->packet_length,
                           PING_MIN_PACKET_LEN, PING_MAX_PACKET_LEN);
}

static int32_t IP2_MISC_ping_count_parse(char *cmd, char *cmd2, char *stx,
                                         char *cmd_org, cli_req_t *req)
{
    IP2_MISC_cli_req_t *r = req->module_req;

    req->parm_parsed = 1;
    return cli_parse_ulong(cmd, &r->packet_count,
                           PING_MIN_PACKET_CNT, PING_MAX_PACKET_CNT);
}

static int32_t IP2_MISC_ping_interval_parse(char *cmd, char *cmd2, char *stx,
                                            char *cmd_org, cli_req_t *req)
{
    IP2_MISC_cli_req_t *r = req->module_req;

    req->parm_parsed = 1;
    return cli_parse_ulong(cmd, (ulong *)&r->packet_interval,
                           PING_MIN_PACKET_INTERVAL, PING_MAX_PACKET_INTERVAL);
}

static int IP2_MISC_parse_ip_target(char *cmd, char *cmd2,
                                    char *stx, char *cmd_org, cli_req_t *req)
{
    IP2_MISC_cli_req_t *r = req->module_req;

    req->parm_parsed = 1;

    if (!cli_parse_ip(cmd, &r->target, &r->target_spec)) {
        (void) cli_parse_quoted_string(cmd, req->host_name, sizeof(req->host_name)); /* Store */
        return 0;               /* IPv6 or IPv4 direct */
    }

    if (!cli_parse_quoted_string(cmd, req->host_name, sizeof(req->host_name)) &&
        (misc_str_is_hostname(req->host_name) == VTSS_OK)) {
        req->host_name_spec = CLI_SPEC_VAL;
        return 0;               /* IP host name */
    }

    return 1;
}

static cli_parm_t IP2_cli_parm_table[] = {
    // TODO, move to IP2_misc
    {
        "length",
        "PING Length keyword",
        CLI_PARM_FLAG_NONE,
        IP2_MISC_parse_keyword,
        NULL,
    },
    {
        "<ping_length>",
        "Ping ICMP data length ("vtss_xstr(PING_MIN_PACKET_LEN)"-"
        vtss_xstr(PING_MAX_PACKET_LEN)"; Default is "
        vtss_xstr(PING_DEF_PACKET_LEN)
        "), excluding MAC, IP and ICMP headers",

        CLI_PARM_FLAG_NONE,
        IP2_MISC_ping_length_parse,
        NULL
    },
    {
        "count",
        "PING Count keyword",
        CLI_PARM_FLAG_NONE,
        IP2_MISC_parse_keyword,
        NULL,
    },
    {
        "<ping_count>",
        "Transmit ECHO_REQUEST packet count ("
        vtss_xstr(PING_MIN_PACKET_CNT)"-"
        vtss_xstr(PING_MAX_PACKET_CNT)
        "; Default is "vtss_xstr(PING_DEF_PACKET_CNT)")",

        CLI_PARM_FLAG_NONE,
        IP2_MISC_ping_count_parse,
        NULL
    },

    {
        "interval",
        "PING Interval keyword",
        CLI_PARM_FLAG_NONE,
        IP2_MISC_parse_keyword,
        NULL,
    },
    {
        "<ping_interval>",
        "Ping interval ("vtss_xstr(PING_MIN_PACKET_INTERVAL)"-"
        vtss_xstr(PING_MAX_PACKET_INTERVAL)"; Default is "
        vtss_xstr(PING_DEF_PACKET_INTERVAL)")",

        CLI_PARM_FLAG_NONE,
        IP2_MISC_ping_interval_parse,
        NULL
    },
    {
        "<ip_target>",
        "IP address or quoted host name (Like 'google.com')",
        CLI_PARM_FLAG_NONE,
        IP2_MISC_parse_ip_target,
        NULL,
    },
    {NULL, NULL, 0, 0, NULL}
};

static void IP2_ping(cli_req_t *req)
{
    IP2_MISC_cli_req_t *ip_req = req->module_req;
    size_t         len, cnt, itv;

    len = PING_DEF_PACKET_LEN;
    cnt = PING_DEF_PACKET_CNT;
    itv = PING_DEF_PACKET_INTERVAL;
    if (ip_req) {
        if (ip_req->len_flag) {
            len = ip_req->packet_length;
        }

        if (ip_req->cnt_flag) {
            cnt = ip_req->packet_count;
        }

        if (ip_req->itv_flag) {
            itv = ip_req->packet_interval;
        }
    }

#ifdef VTSS_SW_OPTION_IPV6
    if (ip_req &&
        ip_req->target_spec == CLI_SPEC_VAL &&
        ip_req->target.type == VTSS_IP_TYPE_IPV6) {
        (void) ping6_test(cli_printf, &ip_req->target.addr.ipv6, len, cnt, itv, PING_DEF_EGRESS_INTF_VID);
        return;
    }
#endif

    (void) ping_test(cli_printf, req->host_name, len, cnt, itv);
}
cli_cmd_tab_entry (
    "IP Ping <ip_target> [(Length <ping_length>)] "
    "[(Count <ping_count>)] [(Interval <ping_interval>)]",
    NULL,
    "Ping IP address (ICMP echo)",
    CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_IP2,
    IP2_ping,
    NULL,
    IP2_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

