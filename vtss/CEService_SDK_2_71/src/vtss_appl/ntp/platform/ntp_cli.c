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
#include "cli_api.h"
#include "vtss_module_id.h"
#include "vtss_ntp_api.h"
#include "cli_trace_def.h"

/****************************************************************************/
/*  Command functions                                                       */
/****************************************************************************/
static void  cli_cmd_debug_sys_ntp_reinit_timer(cli_req_t *req)
{
    vtss_ntp_timer_reset();
}

static void  cli_cmd_debug_sys_ntp_init_timer(cli_req_t *req)
{
    vtss_ntp_timer_init();
}

static void  cli_cmd_debug_ntp_interval(cli_req_t *req)
{
    ntp_conf_t  conf;
    vtss_rc     rc;

    rc = ntp_mgmt_conf_get(&conf);
    if (rc != VTSS_OK) {
        return;
    }

    if (req->int_values[0] < 4 || req->int_values[1] > 8) {
        CPRINTF("Error: Valid values are [4; 8]\n");
        CPRINTF("MIN    : %u\n", conf.interval_min);
        CPRINTF("MAX    : %u\n", conf.interval_max);
    } else {
        conf.interval_min = req->int_values[0];
        conf.interval_max  =  req->int_values[1];
        rc = ntp_mgmt_conf_set(&conf);
        if (rc != VTSS_OK) {
            CPRINTF("NTP interval configuration failed\n");
        }
        CPRINTF("MIN    : %u\n", conf.interval_min);
        CPRINTF("MAX    : %u\n", conf.interval_max);
    }
}

static void cli_cmd_debug_sys_ntp_info(cli_req_t *req)
{
    ntp_sys_status_t    ntp_status;
    ntp_freq_data_t     ntp_freq;
    int i;

    ntp_mgmt_sys_status_get(&ntp_status);

    CPRINTF("current time   : %u\n", ntp_status.currentime);
    CPRINTF("default PPM    : %d\n", ntp_status.drift);
    CPRINTF("Current PPM    : %d\n",  ntp_status.currentppm);
    CPRINTF("Update time    : %d\n", ntp_status.updatime);
    CPRINTF("Current offset : %.6f\n", ntp_status.offset);
    CPRINTF("Server IP      : %s\n",  ntp_status.ip_string);

    CPRINTF("First PPM      : %d\n",  ntp_status.first_dppm);
    CPRINTF("Max   PPM      : %d\n",  ntp_status.max_dppm);
    CPRINTF("Min   PPM      : %d\n",  ntp_status.min_dppm);
    CPRINTF("Step  count    : %u\n",  ntp_status.step_count);
    for (i = 0; i < STEP_ENTRY_NO; i++) {
        if (ntp_status.step_time[i] != 0) {
            CPRINTF("Step  time     : %u\n", ntp_status.step_time[i]);
        }
    }
    CPRINTF("Timer  reset   : %d\n",  ntp_status.timer_rest_count);
    CPRINTF("Latest offset  : %.6f\n",  ntp_status.last_offest);
    CPRINTF("Current status : %d\n",  ntp_status.current_status);

    CPRINTF("(0)-S_NSET\t (1)-S_FSET\t (2)-S_SPIK\t (3)-S_FREQ\t (4)-S_SYNC\n");
    CPRINTF("******************\n");

    vtss_ntp_freq_get(&ntp_freq);
    CPRINTF("mu time        : %u\n", ntp_freq.mu);
    CPRINTF("current offset : %.6f\n", ntp_freq.curr_offset);
    CPRINTF("last    offset : %.6f\n",  ntp_freq.last_offset);
    CPRINTF("frequency      : %.6f\n", ntp_freq.result_frequency * 1e6);
}

static void  cli_cmd_debug_sys_ntp_server(cli_req_t *req)
{
    ntp_server_status_t status[5];
    ulong curr_time;
    int i;


    memset(status, 0, sizeof(ntp_server_status_t) * 5);
    ntp_mgmt_sys_server_get(status, &curr_time, 5);

    CPRINTF("current time   : %u\n\n", curr_time);
    for (i = 0; i < 5; i++) {
        if (status[i].poll_int != 0) {
            CPRINTF("server id      : %d\n", i);
            CPRINTF("server ip      : %s\n", status[i].ip_string);
            CPRINTF("updte time     : %u\n", status[i].curr_time);
            CPRINTF("poll interval  : %d\n", status[i].poll_int);
            CPRINTF("last update    : %u\n", status[i].lastupdate);
            CPRINTF("next update    : %u\n", status[i].nextupdate);
            CPRINTF("offset         : %.6f\n", status[i].offset);
            CPRINTF("******************\n");

        }
    }
}

static void  cli_cmd_debug_sys_ntp_drift(cli_req_t *req)
{
    ntp_conf_t  conf;
    vtss_rc     rc;

    rc = ntp_mgmt_conf_get(&conf);
    if (rc != VTSS_OK) {
        return;
    }

    if (req->set) {
        if (req->int_values[0] >= -500 && req->int_values[0] <= 500) {
            conf.drift_valid = 1;
            conf.drift_data = req->int_values[0];
            conf.drift_trained  =  req->int_values[1];
            rc = ntp_mgmt_conf_set(&conf);
            if (rc != VTSS_OK) {
                CPRINTF("configure Error\n");
            }
        } else {
            CPRINTF("Error: Valid values are [-500; 500]\n");
        }
    } else {
        CPRINTF("[Usage]: [drift_value] [is_from_drift_file]\n");
        CPRINTF("Drift    : %d\n", conf.drift_data);
        CPRINTF("Used       : %d\n", conf.drift_trained);
    }
}

static void cli_cmd_ip_ntp_conf(cli_req_t *req, BOOL mode, BOOL interval, BOOL add, BOOL del, BOOL add_v6)
{
    ntp_conf_t  conf;
    int         i;
#ifdef VTSS_SW_OPTION_IPV6
    char        ip_buf1[40];
#endif
    vtss_rc     rc;

    if (cli_cmd_switch_none(req) ||
        cli_cmd_conf_slave(req)  ||
        ntp_mgmt_conf_get(&conf) != VTSS_OK) {
        return;
    }

    if (req->set) {
        if (mode) {
            conf.mode_enabled = req->enable;
            rc = ntp_mgmt_conf_set(&conf);
            if (rc != VTSS_OK) {
                CPRINTF("NTP mode configuration failed\n");
            }
        }
        if (interval) {
#if 0
            conf.ntp_interval = req->value;
            rc = ntp_mgmt_conf_set(&conf);
            if (rc != VTSS_OK) {
                CPRINTF("NTP interval configuration failed\n");
            }
#endif
            cli_cmd_debug_ntp_interval(req);
        }
        if (add) {
            conf.server[req->value - 1].ip_type = NTP_IP_TYPE_IPV4;
            strcpy((char *) conf.server[req->value - 1].ip_host_string, req->host_name);
            rc = ntp_mgmt_conf_set(&conf);
            if (rc != VTSS_OK) {
                CPRINTF("NTP entry addition  failed\n");
            }
        }
        if (del) {
            if (conf.server[req->value - 1].ip_type == NTP_IP_TYPE_IPV4) {
                if (strcmp((char *) conf.server[req->value - 1].ip_host_string, "") == 0) {
                    CPRINTF("Doesn't allowed to delete empty server\n");
                    return;
                }
                memset(&conf.server[req->value - 1], 0, sizeof(conf.server[req->value - 1]));
                rc = ntp_mgmt_conf_set(&conf);
                if (rc != VTSS_OK) {
                    CPRINTF("NTP entry deletion  failed\n");
                }
            }
#ifdef VTSS_SW_OPTION_IPV6
            else if (conf.server[req->value - 1].ip_type == NTP_IP_TYPE_IPV6) {
                vtss_ipv6_t ipv6addr_0;

                memset(&ipv6addr_0, 0x0, sizeof(vtss_ipv6_t));

                if (memcmp(&conf.server[req->value - 1].ipv6_addr, &ipv6addr_0, sizeof(vtss_ipv6_t)) == 0) {
                    CPRINTF("Doesn't allowed to delete empty server\n");
                    return;
                }
                memset(&conf.server[req->value - 1], 0, sizeof(conf.server[req->value - 1]));
                rc = ntp_mgmt_conf_set(&conf);
                if (rc != VTSS_OK) {
                    CPRINTF("NTP entry addition  failed\n");
                }
            }
#endif /* VTSS_SW_OPTION_IPV6 */
            else {
                return;
            }
        }
#ifdef VTSS_SW_OPTION_IPV6
        if (add_v6) {
            vtss_ipv6_t ipv6addr_0, ipv6addr_1;
            if (req->ipv6_addr.addr[0] == 0xff) {
                CPRINTF("Using IPv6 multicast address is not allowed here.\n");
                return;
            }
            memset(&ipv6addr_0, 0x0, sizeof(vtss_ipv6_t));
            memset(&ipv6addr_1, 0x1, sizeof(vtss_ipv6_t));

            if (memcmp(&req->ipv6_addr, &ipv6addr_0, sizeof(vtss_ipv6_t)) == 0 ||
                memcmp(&req->ipv6_addr, &ipv6addr_1, sizeof(vtss_ipv6_t)) == 0) {
                CPRINTF("Parameter <server_ipv6> doesn't allowed all zero or all 'ff'\n");
                return;
            }
            if (memcmp(&conf.server[req->value - 1].ipv6_addr, &ipv6addr_0, sizeof(vtss_ipv6_t)) != 0) {
                CPRINTF("Sever already exist! Delete it first.\n");
                return;
            }

            conf.server[req->value - 1].ip_type = NTP_IP_TYPE_IPV6;
            memcpy(&conf.server[req->value - 1].ipv6_addr, &req->ipv6_addr, sizeof(conf.server[req->value - 1].ipv6_addr));
            rc = ntp_mgmt_conf_set(&conf);
            if (rc != VTSS_OK) {
                CPRINTF("NTP entry addition  failed\n");
            }
        }
#endif /* VTSS_SW_OPTION_IPV6 */
    } else {
        if (mode) {
            CPRINTF("NTP Mode : %s\n", cli_bool_txt(conf.mode_enabled));
        }
#if 0
        if (interval) {
            CPRINTF("NTP update interval : %d\n", (unsigned int)conf.ntp_interval);
        }
#endif
        if (mode && interval) {
#ifdef VTSS_SW_OPTION_DNS
            CPRINTF("Idx   Server IP host address (a.b.c.d) or a host name string\n");
#else
            CPRINTF("Idx   Server IP host address (a.b.c.d)\n");
#endif
            CPRINTF("---   ------------------------------------------------------\n");
            for (i = 0; i < NTP_MAX_SERVER_COUNT; i++) {
                CPRINTF("%-5d ", i + 1);
#ifdef VTSS_SW_OPTION_IPV6
                CPRINTF("%-31s \n",
                        conf.server[i].ip_type == NTP_IP_TYPE_IPV4 ? (char *)conf.server[i].ip_host_string : misc_ipv6_txt(&conf.server[i].ipv6_addr, ip_buf1));
#else
                CPRINTF("%-16s \n",
                        conf.server[i].ip_host_string);
#endif /* VTSS_SW_OPTION_IPV6 */
            }
        }
    }
}

static void cli_cmd_ip_ntp_config ( cli_req_t *req )
{
    if (!req->set) {
        cli_header("IP NTP Configuration", 1);
    }
    cli_cmd_ip_ntp_conf(req, 1, 1, 1, 1, 1);
}

static void cli_cmd_ip_ntp_mode ( cli_req_t *req )
{
    cli_cmd_ip_ntp_conf(req, 1, 0, 0 , 0 , 0);
}

static void cli_cmd_ip_ntp_add ( cli_req_t *req )
{
    cli_cmd_ip_ntp_conf(req, 0 , 0, 1, 0, 0);
}

static void cli_cmd_ip_ntp_del ( cli_req_t *req )
{
    cli_cmd_ip_ntp_conf(req, 0 , 0, 0, 1, 0);
}

#ifdef VTSS_SW_OPTION_IPV6
static void cli_cmd_ip_ntp_add_v6 ( cli_req_t *req )
{
    cli_cmd_ip_ntp_conf(req, 0 , 0, 0, 0, 1);
}
#endif /* VTSS_SW_OPTION_IPV6 */

/****************************************************************************/
/*  Parameter functions                                                     */
/****************************************************************************/

static int32_t cli_ntp_interval_parse(char *cmd, char *cmd2, char *stx,
                                      char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;

    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &req->value, NTP_MININTERVAL, NTP_MAXINTERVAL);

    return error;
}

static int32_t cli_ntp_server_index_parse(char *cmd, char *cmd2, char *stx,
                                          char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;

    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &req->value, 1, NTP_MAX_SERVER_COUNT);

    return error;
}

static int32_t cli_ntp_server_ip_parse(char *cmd, char *cmd2, char *stx,
                                       char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;

    req->parm_parsed = 1;
    error = cli_parse_ipv4(cmd, &req->ipv4_addr, NULL, &req->ipv4_addr_spec, 0);

    return error;
}

#ifdef VTSS_SW_OPTION_IPV6
static int32_t cli_ntp_server_ipv6_parse(char *cmd, char *cmd2, char *stx,
                                         char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;

    req->parm_parsed = 1;
    error = cli_parse_ipv6(cmd, &req->ipv6_addr, &req->ipv6_addr_spec);

    return error;
}
#endif /* VTSS_SW_OPTION_IPV6 */

static int32_t cli_ntp_draft_parse(char *cmd, char *cmd2, char *stx,
                                   char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;

    req->parm_parsed = 1;
    error = cli_parse_integer(cmd, req, stx);

    return error;
}

/****************************************************************************/
/*  Parameter table                                                         */
/****************************************************************************/
static cli_parm_t ntp_cli_parm_table[] = {
    {
        "enable|disable",
        "enable       : Enable NTP mode\n"
        "disable      : Disable NTP mode\n"
        "(default: Show NTP mode)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        cli_cmd_ip_ntp_mode
    },
    {
        "<integer>",
        "Integer value",
        CLI_PARM_FLAG_SET,
        cli_ntp_draft_parse,
        cli_cmd_debug_sys_ntp_drift,
    },

    {
        "<interval>",
        "NTP transmission interval (4-14)\n"
        "The poll interval is a power of two seconds.",
        CLI_PARM_FLAG_SET,
        cli_ntp_interval_parse,
        cli_cmd_debug_ntp_interval
    },
    {
        "<server_index>",
        "The server index (1-" vtss_xstr(NTP_MAX_SERVER_COUNT) ")",
        CLI_PARM_FLAG_SET,
        cli_ntp_server_index_parse,
        cli_cmd_ip_ntp_add
    },
#ifdef VTSS_SW_OPTION_IPV6
    {
        "<server_index>",
        "The server index (1-" vtss_xstr(NTP_MAX_SERVER_COUNT) ")",
        CLI_PARM_FLAG_SET,
        cli_ntp_server_index_parse,
        cli_cmd_ip_ntp_add_v6
    },
#endif /* VTSS_SW_OPTION_IPV6 */
    {
        "<server_index>",
        "The server index (1-" vtss_xstr(NTP_MAX_SERVER_COUNT) ")",
        CLI_PARM_FLAG_SET,
        cli_ntp_server_index_parse,
        cli_cmd_ip_ntp_del
    },
    {
        "<server_ip>",
        "Server IP address (a.b.c.d)",
        CLI_PARM_FLAG_SET,
        cli_ntp_server_ip_parse,
        cli_cmd_ip_ntp_add
    },
#ifdef VTSS_SW_OPTION_IPV6
    {
        "<server_ipv6>",
        "IPv6 server address.\n"
        "               IPv6 address is in 128-bit records represented as eight fields of up to\n"
        "               four hexadecimal digits with a colon separates each field (:). For example,\n"
        "               'fe80::215:c5ff:fe03:4dc7'. The symbol '::' is a special syntax that can\n"
        "               be used as a shorthand way of representing multiple 16-bit groups of\n"
        "               contiguous zeros; but it can only appear once. It also used a following\n"
        "               legally IPv4 address. For example,'::192.1.2.34'.",
        CLI_PARM_FLAG_NONE,
        cli_ntp_server_ipv6_parse,
        cli_cmd_ip_ntp_add_v6
    },
#endif /* VTSS_SW_OPTION_IPV6 */
    {
        NULL,
        NULL,
        0,
        0,
        NULL
    },
};


/****************************************************************************/
/*  Command table                                                           */
/****************************************************************************/
/* NTP CLI Command Sorting Order */
enum {
    PRIO_IP_NTP_CONF = 2 * CLI_CMD_SORT_KEY_DEFAULT,
    PRIO_IP_NTP_MODE,
    PRIO_IP_NTP_ADD,
    PRIO_IP_NTP_ADD_V6,
    PRIO_IP_NTP_DEL,
    PRIO_IP_NTP_INTERVAL = CLI_CMD_SORT_KEY_DEFAULT,
    PRIO_DEBUG_SYS_NTP_INFO = CLI_CMD_SORT_KEY_DEFAULT,
    PRIO_DEBUG_SYS_NTP_DRIFT = CLI_CMD_SORT_KEY_DEFAULT,
    PRIO_DEBUG_SYS_NTP_SERVER = CLI_CMD_SORT_KEY_DEFAULT,
    PRIO_DEBUG_SYS_NTP_REINIT_TIMER = CLI_CMD_SORT_KEY_DEFAULT,
};

cli_cmd_tab_entry (
    "IP NTP Configuration",
    NULL,
    "Show NTP configuration",
    PRIO_IP_NTP_CONF,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_IP,
    cli_cmd_ip_ntp_config,
    NULL,
    ntp_cli_parm_table,
    CLI_CMD_FLAG_SYS_CONF
);

cli_cmd_tab_entry (
    "IP NTP Mode",
    "IP NTP Mode [enable|disable]",
    "Set or show the NTP mode",
    PRIO_IP_NTP_MODE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_IP,
    cli_cmd_ip_ntp_mode,
    NULL,
    ntp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

#if 0
cli_cmd_tab_entry (
    "IP NTP Polling Interval",
    "IP NTP Polling Interval [<interval>]",
    "Set or show the NTP polling interval",
    PRIO_IP_NTP_INTERVAL,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_IP,
    cli_cmd_ip_ntp_interval,
    NULL,
    ntp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#else
cli_cmd_tab_entry (
    "Debug NTP Polling Interval",
    "Debug NTP Polling Interval [<integer>] [<integer>]",
    "Set or show the NTP polling interval",
    PRIO_IP_NTP_INTERVAL,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_ntp_interval,
    NULL,
    ntp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif


cli_cmd_tab_entry (
    NULL,
    "IP NTP Server Add <server_index> <ip_addr_string>",
    "Add NTP server entry",
    PRIO_IP_NTP_ADD,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_IP,
    cli_cmd_ip_ntp_add,
    NULL,
    ntp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    NULL,
    "IP NTP Server Delete <server_index>",
    "Delete NTP server entry",
    PRIO_IP_NTP_DEL,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_IP,
    cli_cmd_ip_ntp_del,
    NULL,
    ntp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

#ifdef VTSS_SW_OPTION_IPV6
cli_cmd_tab_entry (
    NULL,
    "IP NTP Server Ipv6 Add <server_index> <server_ipv6>",
    "Add NTP server IPv6 entry",
    PRIO_IP_NTP_ADD_V6,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_IP,
    cli_cmd_ip_ntp_add_v6,
    NULL,
    ntp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

#endif /* VTSS_SW_OPTION_IPV6 */
cli_cmd_tab_entry (
    "Debug NTP System Status",
    NULL,
    "Show NTP status",
    PRIO_DEBUG_SYS_NTP_INFO,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_sys_ntp_info,
    NULL,
    ntp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry (
    "Debug NTP Drift",
    "Debug NTP Drift [<integer>] [<integer>]",
    "Show or set NTP drif data and usage by ntp software",
    PRIO_DEBUG_SYS_NTP_DRIFT,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_sys_ntp_drift,
    NULL,
    ntp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry (
    "Debug NTP Server Status",
    NULL,
    "Show NTP server status",
    PRIO_DEBUG_SYS_NTP_SERVER,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_sys_ntp_server,
    NULL,
    ntp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry (
    "Debug NTP Reinit Timer",
    NULL,
    "ReInit NTP timer",
    PRIO_DEBUG_SYS_NTP_REINIT_TIMER,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_sys_ntp_reinit_timer,
    NULL,
    ntp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "Debug NTP Init Timer",
    NULL,
    "Init NTP timer",
    PRIO_DEBUG_SYS_NTP_REINIT_TIMER,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_sys_ntp_init_timer,
    NULL,
    ntp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
