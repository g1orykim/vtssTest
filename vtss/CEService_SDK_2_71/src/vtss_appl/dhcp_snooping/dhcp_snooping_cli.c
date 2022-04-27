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
#include "port_api.h"
#include "cli.h"
#include "dhcp_snooping_api.h"
#include "dhcp_snooping_cli.h"
#include "network.h"
#include "topo_api.h"
#include "cli_trace_def.h"

typedef struct {
    u32 port_mode;
} dhcp_snooping_cli_req_t;

/****************************************************************************/
/*  Initialization                                                          */
/****************************************************************************/

void dhcp_snooping_cli_init(void)
{
    /* register the size required for dhcp snooping req. structure */
    cli_req_size_register(sizeof(dhcp_snooping_cli_req_t));
}

/****************************************************************************/
/*  Command functions                                                       */
/****************************************************************************/

/* DHCP snooping configuration */
static void DHCP_SNOOPING_cli_cmd_conf(cli_req_t *req, BOOL snooping_mode, BOOL port_mode)
{
    vtss_rc                     rc;
    switch_iter_t               sit;
    port_iter_t                 pit;
    BOOL                        first;
    dhcp_snooping_conf_t        conf;
    dhcp_snooping_port_conf_t   port_conf;
    char                        buf[80], *p;
    dhcp_snooping_cli_req_t     *dhcp_snooping_req = req->module_req;

    if (cli_cmd_switch_none(req) ||
        cli_cmd_conf_slave(req) ||
        dhcp_snooping_mgmt_conf_get(&conf) != VTSS_OK) {
        return;
    }

    if (req->set) {
        if (snooping_mode) {
            conf.snooping_mode = req->enable;
            if ((rc = dhcp_snooping_mgmt_conf_set(&conf)) != VTSS_OK) {
                CPRINTF("%s\n", error_txt(rc));
            }
        }
        if (port_mode) {
            (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_USID);
            while (switch_iter_getnext(&sit)) {
                if (req->stack.isid[sit.usid] == VTSS_ISID_END || dhcp_snooping_mgmt_port_conf_get(sit.isid, &port_conf) != VTSS_OK) {
                    continue;
                }
                (void) port_iter_init(&pit, NULL, sit.isid, PORT_ITER_SORT_ORDER_UPORT, PORT_ITER_FLAGS_NORMAL);
                while (port_iter_getnext(&pit)) {
                    if (req->uport_list[pit.uport] == 0) {
                        continue;
                    }
                    port_conf.port_mode[pit.iport] = dhcp_snooping_req->port_mode;
                }
                if ((rc = dhcp_snooping_mgmt_port_conf_set(sit.isid, &port_conf)) != VTSS_OK) {
                    CPRINTF("%s\n", error_txt(rc));
                }
            }
        }
    } else {
        if (snooping_mode) {
            CPRINTF("DHCP Snooping Mode : %s\n", cli_bool_txt(conf.snooping_mode));
        }
        if (port_mode) {
            (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_USID);
            while (switch_iter_getnext(&sit)) {
                if (req->stack.isid[sit.usid] == VTSS_ISID_END || dhcp_snooping_mgmt_port_conf_get(sit.isid, &port_conf) != VTSS_OK) {
                    continue;
                }
                first = 1;
                (void) port_iter_init(&pit, NULL, sit.isid, PORT_ITER_SORT_ORDER_UPORT, PORT_ITER_FLAGS_NORMAL);
                while (port_iter_getnext(&pit)) {
                    if (req->uport_list[pit.uport] == 0) {
                        continue;
                    }
                    if (first) {
                        first = 0;
                        cli_cmd_usid_print(sit.usid, req, 1);
                        p = &buf[0];
                        p += sprintf(p, "Port  ");
                        p += sprintf(p, "Port Mode    ");
                        cli_table_header(buf);
                    }
                    CPRINTF("%-2u    ", pit.uport);
                    CPRINTF("%s    \n", port_conf.port_mode[pit.iport] == DHCP_SNOOPING_PORT_MODE_TRUSTED ? "trusted" : "untrusted");
                }
            }
        }
    }
}

static void DHCP_SNOOPING_cli_cmd_debug_veri_mode(cli_req_t *req)
{
    switch_iter_t               sit;
    port_iter_t                 pit;
    BOOL                        first;
    dhcp_snooping_port_conf_t   port_conf;
    char                        buf[80], *p;

    if (cli_cmd_switch_none(req) ||
        cli_cmd_conf_slave(req)) {
        return;
    }

    (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_USID);
    while (switch_iter_getnext(&sit)) {
        if (req->stack.isid[sit.usid] == VTSS_ISID_END || dhcp_snooping_mgmt_port_conf_get(sit.isid, &port_conf) != VTSS_OK) {
            continue;
        }
        first = 1;
        (void) port_iter_init(&pit, NULL, sit.isid, PORT_ITER_SORT_ORDER_UPORT, PORT_ITER_FLAGS_NORMAL);
        while (port_iter_getnext(&pit)) {
            if (req->uport_list[pit.uport] == 0) {
                continue;
            }
            if (first) {
                first = 0;
                cli_cmd_usid_print(sit.usid, req, 1);
                p = &buf[0];
                p += sprintf(p, "Port  ");
                p += sprintf(p, "Verification Mode    ");
                cli_table_header(buf);
            }
            CPRINTF("%-2u    ", pit.uport);
            CPRINTF("%s    \n", port_conf.port_mode[pit.iport] == DHCP_SNOOPING_MGMT_ENABLED ? "enabled" : "disabled");
        }
    }
}

static void DHCP_SNOOPING_cli_cmd_debug_ip_assigned_info(cli_req_t *req)
{
    u8                                  mac[6] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
    vtss_vid_t                          vid = 0;
    dhcp_snooping_ip_assigned_info_t    info;
    BOOL                                rc;
    char                                buf[32];
    u32                                 cnt = 0;

    CPRINTF("DHCP Snooping IP Assigned Information :\n");
    CPRINTF("---------------------------------------\n");
    do {
        rc = dhcp_snooping_ip_assigned_info_getnext(mac, vid, &info);
        if (rc) {
            CPRINTF("\nEntry %d :\n", ++cnt);
            CPRINTF("---------------\n");
            CPRINTF("MAC Address    : %s\n", misc_mac_txt(info.mac, buf));
            CPRINTF("VLAN ID        : %d\n", info.vid);
            CPRINTF("USID           : %d\n", topo_isid2usid(info.isid));
            CPRINTF("Port NO        : %d\n", iport2uport(info.port_no));
            CPRINTF("OP Code        : %d\n", info.op_code);
            CPRINTF("Transaction ID : %u\n", ntohl(info.transaction_id));
            CPRINTF("IP Address     : %s\n", misc_ipv4_txt(info.assigned_ip, buf));
            CPRINTF("IP MAsk        : %s\n", misc_ipv4_txt(info.assigned_mask, buf));
            CPRINTF("DHCP Server    : %s\n", misc_ipv4_txt(info.dhcp_server_ip, buf));
            CPRINTF("Gateway        : %s\n", misc_ipv4_txt(info.gateway_ip, buf));
            CPRINTF("DNS Server     : %s\n", misc_ipv4_txt(info.dns_server_ip, buf));
            CPRINTF("Lease Time     : %d\n", info.lease_time);
            CPRINTF("Time Stamp     : %d\n", info.timestamp);
            memcpy(mac, info.mac, 6);
            vid = info.vid;
        }
    } while (rc);
    CPRINTF("\nTotal Entries Number : %d\n", cnt);
}

static void DHCP_SNOOPING_cli_cmd_stats(cli_req_t *req)
{
    vtss_rc                 rc;
    switch_iter_t           sit;
    port_iter_t             pit;
    dhcp_snooping_stats_t   stats;
    BOOL                    first;

    if (cli_cmd_switch_none(req) || cli_cmd_slave(req)) {
        return;
    }

    (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_USID);
    while (switch_iter_getnext(&sit)) {
        if (req->stack.isid[sit.usid] == VTSS_ISID_END) {
            continue;
        }
        first = 1;

        (void) port_iter_init(&pit, NULL, sit.isid, PORT_ITER_SORT_ORDER_UPORT, PORT_ITER_FLAGS_NORMAL);
        while (port_iter_getnext(&pit)) {
            if (req->uport_list[pit.uport] == 0) {
                continue;
            }
            if (req->clear) {
                (void) dhcp_snooping_stats_clear(sit.isid, pit.iport);
            } else {
                if (first) {
                    cli_cmd_usid_print(sit.usid, req, 0);
                }
                if ((rc = dhcp_snooping_stats_get(sit.isid, pit.iport, &stats)) != VTSS_OK) {
                    CPRINTF("%s\n", error_txt(rc));
                }
                CPRINTF("Port %u Statistics:", pit.uport);
                CPRINTF("\n--------------------\n");
                cli_cmd_stati("Rx Discover", "Tx Discover", stats.rx_stats.discover_rx, stats.tx_stats.discover_tx);
                cli_cmd_stati("Rx Offer", "Tx Offer", stats.rx_stats.offer_rx, stats.tx_stats.offer_tx);
                cli_cmd_stati("Rx Request", "Tx Request", stats.rx_stats.request_rx, stats.tx_stats.request_tx);
                cli_cmd_stati("Rx Decline", "Tx Decline", stats.rx_stats.decline_rx, stats.tx_stats.decline_tx);
                cli_cmd_stati("Rx ACK", "Tx ACK", stats.rx_stats.ack_rx, stats.tx_stats.ack_tx);
                cli_cmd_stati("Rx NAK", "Tx NAK", stats.rx_stats.nak_rx, stats.tx_stats.nak_tx);
                cli_cmd_stati("Rx Release", "Tx Release", stats.rx_stats.release_rx, stats.tx_stats.release_tx);
                cli_cmd_stati("Rx Inform", "Tx Inform", stats.rx_stats.inform_rx, stats.tx_stats.inform_tx);
                cli_cmd_stati("Rx Lease Query", "Tx Lease Query", stats.rx_stats.leasequery_rx, stats.tx_stats.leasequery_tx);
                cli_cmd_stati("Rx Lease Unassigned", "Tx Lease Unassigned", stats.rx_stats.leaseunassigned_rx, stats.tx_stats.leaseunassigned_tx);
                cli_cmd_stati("Rx Lease Unknown", "Tx Lease Unknown", stats.rx_stats.leaseunknown_rx, stats.tx_stats.leaseunknown_tx);
                cli_cmd_stati("Rx Lease Active", "Tx Lease Active", stats.rx_stats.leaseactive_rx, stats.tx_stats.leaseactive_tx);
                cli_cmd_stati("Rx Discarded from Untrusted", "Rx Discarded checksum error", stats.rx_stats.discard_untrust_rx, stats.rx_stats.discard_chksum_err_rx);
            }
        }
    }
}

static void DHCP_SNOOPING_cli_cmd_conf_show(cli_req_t *req)
{
    if (!req->set) {
        cli_header("DHCP Snooping Configuration", 1);
    }
    DHCP_SNOOPING_cli_cmd_conf(req, 1, 1);
}

static void DHCP_SNOOPING_cli_cmd_conf_mode(cli_req_t *req)
{
    DHCP_SNOOPING_cli_cmd_conf(req, 1, 0);
}

static void DHCP_SNOOPING_cli_cmd_conf_port_mode(cli_req_t *req)
{
    DHCP_SNOOPING_cli_cmd_conf(req, 0, 1);
}

/****************************************************************************/
/*  Parameter functions                                                     */
/****************************************************************************/

static int32_t DHCP_SNOOPING_cli_parse_keyword(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    char                    *found = cli_parse_find(cmd, stx);
    dhcp_snooping_cli_req_t *dhcp_snooping_req = req->module_req;

    req->parm_parsed = 1;
    if (found != NULL) {
        if (!strncmp(found, "trusted", 7)) {
            dhcp_snooping_req->port_mode = DHCP_SNOOPING_PORT_MODE_TRUSTED;
        } else if (!strncmp(found, "untrusted", 9)) {
            dhcp_snooping_req->port_mode = DHCP_SNOOPING_PORT_MODE_UNTRUSTED;
        }
    }

    return (found == NULL ? 1 : 0);
}

/****************************************************************************/
/*  Parameter table                                                         */
/****************************************************************************/

static cli_parm_t dhcp_snooping_cli_parm_table[] = {
    {
        "enable|disable",
        "enable : Enable DHCP snooping mode.\n"
        "         When enable DHCP snooping mode operation, the request\n"
        "         DHCP messages will be forwarded to trusted ports and\n"
        "         only allowed reply packets from trusted ports.\n"
        "disable: Disable DHCP snooping mode\n"
        "(default: Show flow DHCP snooping mode)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        DHCP_SNOOPING_cli_cmd_conf_mode
    },
    {
        "trusted|untrusted",
        "trusted : Configures the port as trusted sources of the DHCP message\n"
        "untrusted: Configures the port as untrusted sources of the DHCP message\n"
        "(default: Show flow DHCP snooping port mode)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        DHCP_SNOOPING_cli_parse_keyword,
        DHCP_SNOOPING_cli_cmd_conf_port_mode
    },
    {
        "clear",
        "Clear DHCP snooping statistics",
        CLI_PARM_FLAG_NONE,
        cli_parm_parse_keyword,
        DHCP_SNOOPING_cli_cmd_stats
    },
    {NULL, NULL, 0, 0, NULL}
};

/****************************************************************************/
/*  Command table                                                           */
/****************************************************************************/

enum {
#ifdef VTSS_CLI_SEC_GRP
    DHCP_SNOOPING_PRIO_CONF = 2 * CLI_CMD_SORT_KEY_DEFAULT, /* Seperate the cmd from DHCP relay */
#else
    DHCP_SNOOPING_PRIO_CONF,
#endif
    DHCP_SNOOPING_PRIO_MODE,
    DHCP_SNOOPING_PRIO_PORT_MODE,
    DHCP_SNOOPING_PRIO_STATS,
    PRIO_DEBUG_DHCP_SNOOPING_VERI_MODE = CLI_CMD_SORT_KEY_DEFAULT,
    PRIO_DEBUG_DHCP_SNOOPING_IP_ASSIGNED_INFO = CLI_CMD_SORT_KEY_DEFAULT,
};

cli_cmd_tab_entry(
    "DHCP Snooping Configuration",
    NULL,
    "Show DHCP snooping configuration",
    DHCP_SNOOPING_PRIO_CONF,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DHCP,
    DHCP_SNOOPING_cli_cmd_conf_show,
    NULL,
    dhcp_snooping_cli_parm_table,
    CLI_CMD_FLAG_SYS_CONF
);

cli_cmd_tab_entry(
    "DHCP Snooping Mode",
    "DHCP Snooping Mode [enable|disable]",
    "Set or show the DHCP snooping mode",
    DHCP_SNOOPING_PRIO_MODE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DHCP,
    DHCP_SNOOPING_cli_cmd_conf_mode,
    NULL,
    dhcp_snooping_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    "DHCP Snooping Port Mode [<port_list>]",
    "DHCP Snooping Port Mode [<port_list>] [trusted|untrusted]",
    "Set or show the DHCP snooping port mode",
    DHCP_SNOOPING_PRIO_PORT_MODE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DHCP,
    DHCP_SNOOPING_cli_cmd_conf_port_mode,
    NULL,
    dhcp_snooping_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    "DHCP Snooping Statistics [<port_list>]",
    "DHCP Snooping Statistics [<port_list>] [clear]",
    "Show or clear DHCP snooping statistics.",
    DHCP_SNOOPING_PRIO_STATS,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DHCP,
    DHCP_SNOOPING_cli_cmd_stats,
    NULL,
    dhcp_snooping_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    "Debug DHCP Snooping Verification Mode",
    NULL,
    "Show DHCP snooping MAC verification mode",
    PRIO_DEBUG_DHCP_SNOOPING_VERI_MODE,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    DHCP_SNOOPING_cli_cmd_debug_veri_mode,
    NULL,
    dhcp_snooping_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    "DHCP Snooping Table",
    NULL,
    "Show DHCP snooping dynamic IP assigned informaiton table",
    PRIO_DEBUG_DHCP_SNOOPING_IP_ASSIGNED_INFO,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DHCP,
    DHCP_SNOOPING_cli_cmd_debug_ip_assigned_info,
    NULL,
    dhcp_snooping_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
