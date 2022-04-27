/*

 Vitesse Switch API software.

 Copyright (c) 2002-2011 Vitesse Semiconductor Corporation "Vitesse". All
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
#include "vtss_upnp_cli.h"
#include "vtss_upnp_api.h"
#include "cli_trace_def.h"

typedef struct {
    uchar   ttl;
    ulong   adv_interval;
} upnp_cli_req_t;

/****************************************************************************/
/*  Initialization                                                          */
/****************************************************************************/

void upnp_cli_init(void)
{
    /* register the size required for upnp req. structure */
    cli_req_size_register(sizeof(upnp_cli_req_t));
}

/****************************************************************************/
/*  Command functions                                                       */
/****************************************************************************/

/* UPnP configuration */
static void cli_cmd_upnp_conf(cli_req_t *req, BOOL mode, BOOL ttl, BOOL interval)
{
    upnp_conf_t conf;
    upnp_cli_req_t *upnp_req = req->module_req;

    if (cli_cmd_switch_none(req) ||
        cli_cmd_conf_slave(req) ||
        upnp_mgmt_conf_get(&conf) != VTSS_OK) {
        return;
    }

    if (req->set) {
        if (mode) {
            conf.mode = req->enable;
            if (upnp_mgmt_conf_set(&conf) != VTSS_OK) {
                CPRINTF("UPnP Mode configuration failed\n");
            }
        }
        if (ttl) {
            conf.ttl = upnp_req->ttl;
            if (upnp_mgmt_conf_set(&conf) != VTSS_OK) {
                CPRINTF("UPnP Ttl configuration failed\n");
            }
        }
        if (interval) {
            conf.adv_interval = upnp_req->adv_interval;
            if (upnp_mgmt_conf_set(&conf) != VTSS_OK) {
                CPRINTF("UPnP Ttl configuration failed\n");
            }
        }

    } else {
        if (mode) {
            CPRINTF("UPnP Mode                 : %s\n", cli_bool_txt(conf.mode));
        }
        if (ttl) {
            CPRINTF("UPnP TTL                  : %d\n", conf.ttl);
        }
        if (interval) {
            CPRINTF("UPnP Advertising Duration : %ld\n", conf.adv_interval);
        }
    }
}

static void cli_cmd_upnp_conf_disp(cli_req_t *req)
{
    if (!req->set) {
        cli_header("UPnP Configuration", 1);
    }
    cli_cmd_upnp_conf(req, 1, 1, 1);
    return;
}

static void cli_cmd_upnp_mode(cli_req_t *req)
{
    cli_cmd_upnp_conf(req, 1, 0, 0);
    return;
}

static void cli_cmd_upnp_ttl(cli_req_t *req)
{
    cli_cmd_upnp_conf(req, 0, 1, 0);
    return;
}

static void cli_cmd_upnp_interval(cli_req_t *req)
{
    cli_cmd_upnp_conf(req, 0, 0, 1);
    return;
}

/****************************************************************************/
/*  Parameter functions                                                     */
/****************************************************************************/

static int32_t cli_cmd_ttl_parse(char *cmd, char *cmd2, char *stx, char *cmd_org,
                                 cli_req_t *req)
{
    int32_t error = 0;
    ulong value = 0;
    upnp_cli_req_t *upnp_cli_req;

    req->parm_parsed = 1;
    upnp_cli_req = req->module_req;
    error = cli_parse_ulong(cmd, &value, 1, 255);
    upnp_cli_req->ttl = value;

    return error;
}

static int32_t cli_cmd_duration_parse(char *cmd, char *cmd2, char *stx,
                                      char *cmd_org, cli_req_t *req)
{

    int32_t error = 0;
    ulong value = 0;
    upnp_cli_req_t *upnp_cli_req;

    req->parm_parsed = 1;
    upnp_cli_req = req->module_req;
    error = cli_parse_ulong(cmd, &value, 100, 86400);
    upnp_cli_req->adv_interval = value;

    return error;
}

/****************************************************************************/
/*  Parameter table                                                         */
/****************************************************************************/
static cli_parm_t upnp_cli_parm_table[] = {
    {
        "enable|disable",
        "enable : Enable UPnP\n"
        "disable: Disable UPnP\n"
        "(default: Show UPnP mode)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        cli_cmd_upnp_mode
    },
    {
        "<ttl>",
        "ttl range (1..255), default: Show UPnP TTL",
        CLI_PARM_FLAG_NONE | CLI_PARM_FLAG_SET,
        cli_cmd_ttl_parse,
        NULL
    },
    {
        "<duration>",
        "duration range (100..86400), default: Show UPnP duration range",
        CLI_PARM_FLAG_NONE | CLI_PARM_FLAG_SET,
        cli_cmd_duration_parse,
        NULL
    },
    {NULL, NULL, 0, 0, NULL}
};

/****************************************************************************/
/*  Command table                                                           */
/****************************************************************************/
/* UPNP CLI Command Sorting Order */
enum {
    CLI_CMD_UPNP_CONF_PRIO = 0,
    CLI_CMD_UPNP_MODE_PRIO,
    CLI_CMD_UPNP_TTL_PRIO,
    CLI_CMD_UPNP_INTERVAL_PRIO,
};

cli_cmd_tab_entry(
    "UPnP Configuration",
    NULL,
    "Show UPnP configuration",
    CLI_CMD_UPNP_CONF_PRIO,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_UPNP,
    cli_cmd_upnp_conf_disp,
    NULL,
    upnp_cli_parm_table,
    CLI_CMD_FLAG_SYS_CONF
);

cli_cmd_tab_entry(
    "UPnP Mode",
    "UPnP Mode [enable|disable]",
    "Set or show the UPnP mode",
    CLI_CMD_UPNP_MODE_PRIO,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_UPNP,
    cli_cmd_upnp_mode,
    NULL,
    upnp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    "UPnP TTL",
    "UPnP TTL [<ttl>]",
    "Set or show the TTL value of the IP header in SSDP messages",
    CLI_CMD_UPNP_TTL_PRIO,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_UPNP,
    cli_cmd_upnp_ttl,
    NULL,
    upnp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    "UPnP AdvertisingDuration",
    "UPnP AdvertisingDuration [<duration>]",
    "Set or show UPnP Advertising Duration",
    CLI_CMD_UPNP_INTERVAL_PRIO,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_UPNP,
    cli_cmd_upnp_interval,
    NULL,
    upnp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
