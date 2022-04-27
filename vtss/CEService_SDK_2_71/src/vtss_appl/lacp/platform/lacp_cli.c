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
#include "cli_api.h"
#include "lacp_cli.h"
#include "l2proto_api.h"
#include "vtss_lacp.h"
#include "lacp_api.h"
#include "aggr_api.h"
#include "vtss_module_id.h"
#include "cli_trace_def.h"
#include "mgmt_api.h"

typedef struct {
    /* Keywords */
    int32_t key;
    int32_t prio;
    BOOL    active;
    BOOL    passive;
    BOOL    fast;
} lacp_cli_req_t;


void lacp_cli_req_init(void)
{
    /* register the size required for port req. structure */
    cli_req_size_register(sizeof(lacp_cli_req_t));
}

static void cli_vtss_lacp_dump ( cli_req_t *req )
{
    vtss_lacp_dump();
}

static void cli_cmd_lacp_conf(cli_req_t *req,
                              BOOL mode,
                              BOOL key,
                              BOOL role,
                              BOOL status,
                              BOOL stats,
                              BOOL timeout,
                              BOOL prio,
                              BOOL sysprio)
{
    vtss_usid_t                  usid;
    vtss_isid_t                  isid;
    l2_port_no_t                 l2port;
    vtss_lacp_aggregatorstatus_t aggr;
    vtss_lacp_port_config_t      conf;
    vtss_lacp_portstatus_t       stat;
    BOOL                         first = 1;
    char                         buf[80], *p;
    vtss_rc                      rc;
    lacp_cli_req_t                *lacp_req = req->module_req;
    vtss_port_no_t               iport;
    vtss_uport_no_t              uport;
    u32                          port_count;
    vtss_lacp_system_config_t    sysconf;
    aggr_mgmt_group_no_t         aggr_no;
    int                          search_aid, return_aid;
    BOOL                         first_search = 1;

    if (cli_cmd_switch_none(req) || cli_cmd_slave(req)) {
        return;
    }

    while (aggr_mgmt_lacp_id_get_next(first_search ? NULL : &search_aid,  &return_aid) == VTSS_RC_OK && status) {
        search_aid = return_aid;
        first_search = 0;
        if (!lacp_mgmt_aggr_status_get(return_aid, &aggr)) {
            continue;
        }
        if (first) {
            cli_table_header("Aggr ID  Partner System ID  Partner Prio  Partner Key  Last Changed   Ports");
        }
        CPRINTF("%-7u  %s  %-13d %-11d  %-13s  ",
                mgmt_aggr_no2id(lacp_to_aggr_id(aggr.aggrid)),
                misc_mac_txt(aggr.partner_oper_system.macaddr, buf),
                aggr.partner_oper_system_priority,
                aggr.partner_oper_key,
                cli_time_txt(aggr.secs_since_last_change));
        for (first = 1, l2port = VTSS_PORT_NO_START; l2port < (VTSS_LACP_MAX_PORTS + VTSS_PORT_NO_START); l2port++)
            if (aggr.port_list[l2port - VTSS_PORT_NO_START]) {
                CPRINTF("%s%s", first ? "" : ",", cli_l2port2uport_str(l2port));
                first = 0;
            }
        first = 0;
        CPRINTF("\n");
    }
    if (sysprio) {
        if (lacp_mgmt_system_conf_get(&sysconf) == VTSS_RC_OK) {
            if (req->set) {
                sysconf.system_prio = lacp_req->prio;
                if (lacp_mgmt_system_conf_set(&sysconf) != VTSS_RC_OK) {
                    CPRINTF("Could not set system prio");
                    return;
                }
            } else {
                CPRINTF("System Priority: %d\n", sysconf.system_prio);
            }
        }
        if (!mode && !key && !role && !status && !stats && !prio) {
            return;
        }
    }

    for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) {
        if ((isid = req->stack.isid[usid]) == VTSS_ISID_END) {
            continue;
        }

        port_count = port_isid_port_count(isid);
        first = 1;
        for (iport = VTSS_PORT_NO_START; iport < port_count; iport++) {
            uport = iport2uport(iport);
            if (req->uport_list[uport] == 0 || port_isid_port_no_is_stack(isid, iport)) {
                continue;
            }

            l2port = L2PORT2PORT(isid, iport);
            if (lacp_mgmt_port_conf_get(isid, iport, &conf) != VTSS_RC_OK || lacp_mgmt_port_status_get(l2port, &stat) != VTSS_RC_OK) {
                continue;
            }

            /* Handle 'statistics clear' command */
            if (req->clear) {
                lacp_mgmt_statistics_clear(l2port);
                continue;
            }

            if (req->set) {
                if (mode) {
                    conf.enable_lacp = req->enable;
                }
                if (key) {
                    conf.port_key = lacp_req->key;
                }
                if (prio) {
                    conf.port_prio = lacp_req->prio;
                }
                if (role)
                    conf.active_or_passive = (lacp_req->active ? VTSS_LACP_ACTMODE_ACTIVE :
                                              VTSS_LACP_ACTMODE_PASSIVE);
                if (timeout) {
                    conf.xmit_mode = (lacp_req->fast ? VTSS_LACP_FSMODE_FAST : VTSS_LACP_FSMODE_SLOW);
                }

                rc = lacp_mgmt_port_conf_set(isid, iport, &conf);
                if (rc == LACP_ERROR_STATIC_AGGR_ENABLED) {
                    CPRINTF("Port %s is already included in a static aggregation\n", cli_l2port2uport_str(l2port));
                }
                if (rc == LACP_ERROR_DOT1X_ENABLED) {
                    CPRINTF("Can not enable LACP on port %s because it already has 802.1X enabled.\n", cli_l2port2uport_str(l2port));
                }
                continue;
            }

            if (first) {
                first = 0;
                cli_cmd_usid_print(usid, req, 1);
                p = &buf[0];
                p += sprintf(p, "Port  ");
                if (mode || status) {
                    p += sprintf(p, "Mode      ");
                }
                if (key || status) {
                    p += sprintf(p, "Key   ");
                }
                if (role) {
                    p += sprintf(p, "Role    ");
                }
                if (timeout) {
                    p += sprintf(p, "Timeout  ");
                }
                if (prio) {
                    p += sprintf(p, "Priority");
                }
                if (status) {
                    p += sprintf(p, " Aggr ID  Partner System ID  Partner Port  Partner Port Prio");
                }
                if (stats) {
                    p += sprintf(p, "Rx Frames   Tx Frames   Rx Unknown  Rx Illegal");
                }
                cli_table_header(buf);

            }

            /* Find aggregation */
            first_search = 1;
            while (aggr_mgmt_lacp_id_get_next(first_search ? NULL : &search_aid,  &return_aid) == VTSS_RC_OK && status) {
                search_aid = return_aid;
                first_search = 0;
                if (lacp_mgmt_aggr_status_get(return_aid, &aggr) && aggr.port_list[L2PORT2PORT(isid, iport) - VTSS_PORT_NO_START]) {
                    break;
                }
            }

            CPRINTF("%-2u    ", uport);
            if (mode) {
                CPRINTF("%s  ", cli_bool_txt(conf.enable_lacp));
            }
            if (key) {
                if (conf.port_key == VTSS_LACP_AUTOKEY) {
                    CPRINTF("Auto  ");
                } else {
                    CPRINTF("%-4d  ", conf.port_key);
                }
            }
            if (role)
                CPRINTF("%-8s", conf.active_or_passive == VTSS_LACP_ACTMODE_PASSIVE ?
                        "Passive" : "Active");

            if (timeout)
                CPRINTF("%-9s", conf.xmit_mode == VTSS_LACP_FSMODE_SLOW ?
                        "Slow" : "Fast");
            if (prio) {
                CPRINTF("%-5d", conf.port_prio);
            }

            if (status) {
                CPRINTF("%s  %-5d  ",
                        stat.port_state == LACP_PORT_STANDBY ? "Standby " : cli_bool_txt(stat.port_enabled), stat.actor_oper_port_key);

                aggr_no = lacp_to_aggr_id(stat.actor_port_aggregator_identifier);
                if ((stat.port_state == LACP_PORT_ACTIVE) && AGGR_MGMT_GROUP_IS_AGGR(aggr_no)) {
                    CPRINTF("%-7u  %s  %-13d %d",
                            mgmt_aggr_no2id(lacp_to_aggr_id(stat.actor_port_aggregator_identifier)),
                            misc_mac_txt(aggr.partner_oper_system.macaddr, buf),
                            stat.partner_oper_port_number,
                            stat.partner_oper_port_priority);
                } else {
                    CPRINTF("%-7s  %-17s  %-13s -", "-", "-", "-");
                }


            }
            if (stats) {
                CPRINTF("%-12lu%-12lu%-12lu%-12lu",
                        stat.port_stats.lacp_frame_recvs,
                        stat.port_stats.lacp_frame_xmits,
                        stat.port_stats.unknown_frame_recvs,
                        stat.port_stats.illegal_frame_recvs);
            }
            CPRINTF("\n");
        }
    }
}

static void cli_cmd_lacp_config ( cli_req_t *req )
{
    if (!req->set) {
        cli_header("LACP Configuration", 0);
    }

    cli_cmd_lacp_conf(req, 1, 1, 1, 0, 0, 1, 1, 1);
}

static void cli_cmd_lacp_port_mode ( cli_req_t *req )
{
    cli_cmd_lacp_conf(req, 1, 0, 0, 0, 0, 0, 0, 0);
}

static void cli_cmd_lacp_port_key ( cli_req_t *req )
{
    cli_cmd_lacp_conf(req, 0, 1, 0, 0, 0, 0, 0, 0);
}

static void cli_cmd_lacp_port_role ( cli_req_t *req )
{
    cli_cmd_lacp_conf(req, 0, 0, 1, 0, 0, 0, 0, 0);
}

static void cli_cmd_lacp_port_status ( cli_req_t *req )
{
    cli_cmd_lacp_conf(req, 0, 0, 0, 1, 0, 0, 0, 0);
}

static void cli_cmd_lacp_port_stats ( cli_req_t *req )
{
    cli_cmd_lacp_conf(req, 0, 0, 0, 0, 1, 0, 0, 0);
}

static void cli_cmd_lacp_port_prio ( cli_req_t *req )
{
    cli_cmd_lacp_conf(req, 0, 0, 0, 0, 0, 0, 1, 0);
}

static void cli_cmd_lacp_port_timeout ( cli_req_t *req )
{
    cli_cmd_lacp_conf(req, 0, 0, 0, 0, 0, 1, 0, 0);
}

static void cli_cmd_lacp_system_prio ( cli_req_t *req )
{
    cli_cmd_lacp_conf(req, 0, 0, 0, 0, 0, 0, 0, 1);
}


static int32_t cli_lacp_parse_key(char *cmd, char *cmd2,
                                  char *stx, char *cmd_org, cli_req_t *req)
{
    int error = 0;
    ulong value;
    lacp_cli_req_t *lacp_req = req->module_req;

    req->parm_parsed = 1;

    if ((error = cli_parse_word(cmd, "auto")) == 0) {
        lacp_req->key = VTSS_LACP_AUTOKEY;
    } else {
        error = cli_parse_ulong(cmd, &value, 1, 65535);
        lacp_req->key = value;
    }

    return (error);
}

static int32_t cli_lacp_parse_prio(char *cmd, char *cmd2,
                                   char *stx, char *cmd_org, cli_req_t *req)
{
    int error = 0;
    ulong value;
    lacp_cli_req_t *lacp_req = req->module_req;

    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &value, 0, 65535);
    lacp_req->prio = value;

    return (error);
}


static int32_t cli_lacp_keyword(char *cmd, char *cmd2,
                                char *stx, char *cmd_org, cli_req_t *req)
{
    char *found = cli_parse_find(cmd, stx);
    lacp_cli_req_t *lacp_req = NULL;

    lacp_req = req->module_req;

    T_I("ALL %s", found);
    if (found != NULL) {
        if (!strncmp(found, "active", 6)) {
            lacp_req->active = 1;
        } else if (!strncmp(found, "passive", 7)) {
            lacp_req->passive = 1;
        } else if (!strncmp(found, "fast", 4)) {
            lacp_req->fast = 1;
        }
    }

    req->parm_parsed = 1;

    return (found == NULL ? 1 : 0);
} /* cli_parse_keyword */


static cli_parm_t lacp_cli_parm_table[] = {
    {
        "enable|disable",
        "enable : Enable LACP protocol\n"
        "disable: Disable LACP protocol\n"
        "(default: Show LACP mode)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        cli_cmd_lacp_port_mode,
    },
    {
        "<key>",
        "LACP key (1-65535) or 'auto'",
        CLI_PARM_FLAG_SET,
        cli_lacp_parse_key,
        NULL,
    },

    {
        "<prio>",
        "LACP Prio (0-65535)",
        CLI_PARM_FLAG_SET,
        cli_lacp_parse_prio,
        NULL,
    },

    {
        "<sysprio>",
        "LACP System Prio (0-65535)",
        CLI_PARM_FLAG_SET,
        cli_lacp_parse_prio,
        NULL,
    },
    {
        "active|passive",
        "active : Initiate LACP negotiation\n"
        "passive: Listen for LACP packets\n"
        "(default: Show LACP role)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_lacp_keyword,
        cli_cmd_lacp_port_role,
    },
    {
        "fast|slow",
        "fast : Fast PDU transmissions (fast timeout)\n"
        "slow : Slow PDU transmissions (slow timeout)\n"
        "(default: Show LACP timeout)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_lacp_keyword,
        cli_cmd_lacp_port_timeout,
    },
    {
        "clear",
        "Clear LACP statistics",
        CLI_PARM_FLAG_NONE,
        cli_parm_parse_keyword,
        cli_cmd_lacp_port_stats,
    },
    {
        NULL,
        NULL,
        0,
        0,
        NULL
    },
};

enum {
    PRIO_LACP_CONF,
    PRIO_LACP_PORT_MODE,
    PRIO_LACP_PORT_KEY,
    PRIO_LACP_PORT_PRIO,
    PRIO_LACP_SYSTEM_PRIO,
    PRIO_LACP_PORT_ROLE,
    PRIO_LACP_PORT_STATUS,
    PRIO_LACP_PORT_STATIS,
    PRIO_LACP_PORT_TIMEOUT,
    PRIO_DEBUG_LACP = CLI_CMD_SORT_KEY_DEFAULT,
};

/* Command table entries */
cli_cmd_tab_entry (
    "LACP Configuration [<port_list>]",
    NULL,
    "Show LACP configuration",
    PRIO_LACP_CONF,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_LACP,
    cli_cmd_lacp_config,
    NULL,
    lacp_cli_parm_table,
    CLI_CMD_FLAG_SYS_CONF
);

cli_cmd_tab_entry (
    "LACP Mode [<port_list>]",
    "LACP Mode [<port_list>] [enable|disable]",
    "Set or show LACP mode",
    PRIO_LACP_PORT_MODE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_LACP,
    cli_cmd_lacp_port_mode,
    NULL,
    lacp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "LACP Key [<port_list>]",
    "LACP Key [<port_list>] [<key>]",
    "Set or show the LACP key",
    PRIO_LACP_PORT_KEY,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_LACP,
    cli_cmd_lacp_port_key,
    NULL,
    lacp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "LACP Prio [<port_list>]",
    "LACP Prio [<port_list>] [<prio>]",
    "Set or show the LACP prio",
    PRIO_LACP_PORT_PRIO,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_LACP,
    cli_cmd_lacp_port_prio,
    NULL,
    lacp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);


cli_cmd_tab_entry (
    "LACP Timeout [<port_list>]",
    "LACP Timeout [<port_list>] [fast|slow]",
    "Set or show the LACP timeout",
    PRIO_LACP_PORT_TIMEOUT,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_LACP,
    cli_cmd_lacp_port_timeout,
    NULL,
    lacp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "LACP System Prio",
    "LACP System Prio [<sysprio>]",
    "Set or show the LACP System prio",
    PRIO_LACP_SYSTEM_PRIO,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_LACP,
    cli_cmd_lacp_system_prio,
    NULL,
    lacp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);


cli_cmd_tab_entry (
    "LACP Role [<port_list>]",
    "LACP Role [<port_list>] [active|passive]",
    "Set or show the LACP role",
    PRIO_LACP_PORT_ROLE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_LACP,
    cli_cmd_lacp_port_role,
    NULL,
    lacp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "LACP Status [<port_list>]",
    NULL,
    "Show LACP Status",
    PRIO_LACP_PORT_STATUS,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_LACP,
    cli_cmd_lacp_port_status,
    NULL,
    lacp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "LACP Statistics [<port_list>]",
    "LACP Statistics [<port_list>] [clear]",
    "Show LACP Statistics",
    PRIO_LACP_PORT_STATIS,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_LACP,
    cli_cmd_lacp_port_stats,
    NULL,
    lacp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);


cli_cmd_tab_entry (
    "Debug LACP",
    NULL,
    "Show lacp details",
    PRIO_DEBUG_LACP,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    cli_vtss_lacp_dump,
    NULL,
    lacp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
