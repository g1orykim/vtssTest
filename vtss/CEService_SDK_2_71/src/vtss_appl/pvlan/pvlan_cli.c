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
#include "pvlan_cli.h"
#include "cli_trace_def.h"
#include "pvlan_api.h"
#include "cli.h"
#include "mgmt_api.h"
#include "port_api.h" /* For PORT_NO_IS_STACK() */

typedef struct {
#if defined(PVLAN_SRC_MASK_ENA)
    cli_spec_t      privatevid_spec;
    vtss_pvlan_no_t privatevid;
#else
    ulong           dummy;
#endif /* PVLAN_SRC_MASK_ENA */
} pvlan_cli_req_t;

/****************************************************************************/
/*  Initialization                                                          */
/****************************************************************************/

void pvlan_cli_init(void)
{
    /* register the size required for pvlan req. structure */
    cli_req_size_register(sizeof(pvlan_cli_req_t));
}

/****************************************************************************/
/*  Command functions                                                       */
/****************************************************************************/

#if defined(PVLAN_SRC_MASK_ENA)
static void cli_pvlan_entry_print(cli_req_t *req, vtss_usid_t usid,
                                  pvlan_mgmt_entry_t *conf, BOOL *first)
{
    char buf[MGMT_PORT_BUF_SIZE];

    if (*first) {
        *first = 0;
        cli_cmd_usid_print(usid, req, 1);
        cli_table_header("PVLAN ID  Ports");
    }
    CPRINTF("%-8u  %s\n", conf->privatevid + 1, cli_iport_list_txt(conf->ports, buf));
}

/* Private VLAN configuration */
static void cli_cmd_pvlan_privatevid_conf(cli_req_t *req, BOOL add, BOOL delete, BOOL lookup)
{
    vtss_usid_t        usid;
    vtss_isid_t        isid;
    pvlan_mgmt_entry_t conf;
    BOOL               first;
    pvlan_cli_req_t    *pvlan_req = req->module_req;
    port_iter_t        pit;

    if (cli_cmd_switch_none(req) || (lookup == FALSE && cli_cmd_conf_slave(req))) {
        return;
    }

    memset(&conf, 0, sizeof(conf));
    conf.privatevid = (pvlan_req->privatevid - 1);
    if (delete) {
        // Delete of private VLAN works globally, i.e. we shouldn't loop over SIDs
        if ((pvlan_mgmt_pvlan_del(conf.privatevid)) != VTSS_RC_OK) {
            CPRINTF("PVLAN %u deletion failed!\n", pvlan_req->privatevid);
        }
        return;
    }

    for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) {
        if ((isid = req->stack.isid[usid]) == VTSS_ISID_END) {
            continue;
        }

        first = 1;
        if (add) {
            (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
            while (port_iter_getnext(&pit)) {
                conf.ports[pit.iport] = req->uport_list[pit.uport];
            }
            (void)pvlan_mgmt_pvlan_add(isid, &conf);
            continue;
        }

        if (lookup && pvlan_req->privatevid_spec == CLI_SPEC_VAL) {
            /* Lookup specific private vid */
            if (pvlan_mgmt_pvlan_get(isid, conf.privatevid, &conf, 0) == VTSS_OK) {
                cli_pvlan_entry_print(req, usid, &conf, &first);
            }
        } else {
            /* Lookup all */
            conf.privatevid = VTSS_PVLAN_NO_START;
            while ((first && pvlan_mgmt_pvlan_get(isid, conf.privatevid, &conf, 0) == VTSS_OK) ||
                   pvlan_mgmt_pvlan_get(isid, conf.privatevid, &conf, 1) == VTSS_OK) {
                cli_pvlan_entry_print(req, usid, &conf, &first);
            }
        }

        if (first) {
            if (pvlan_req->privatevid_spec == CLI_SPEC_VAL) {
                CPRINTF("Private VLAN ID %u not found\n", pvlan_req->privatevid);
            } else {
                CPRINTF("Private VLAN table is empty\n");
            }
        }
    }
}
#endif /* PVLAN_SRC_MASK_ENA */

/* PVLAN Port isolation */
static void cli_cmd_pvlan_isolate_conf(cli_req_t *req)
{
    vtss_usid_t     usid;
    vtss_isid_t     isid;
    BOOL            member[VTSS_PORT_ARRAY_SIZE];
    port_iter_t     pit;

    if (cli_cmd_switch_none(req) || cli_cmd_conf_slave(req)) {
        return;
    }

    memset(member, 0, sizeof(member));
    for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) {
        if ((isid = req->stack.isid[usid]) == VTSS_ISID_END ||
            pvlan_mgmt_isolate_conf_get(isid, member) != VTSS_OK) {
            continue;
        }

        (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
        while (port_iter_getnext(&pit)) {
            if (req->uport_list[pit.uport] == 0) {
                continue;
            }
            if (req->set) {
                member[pit.iport] = req->enable;
            } else {
                if (pit.first) {
                    cli_cmd_usid_print(usid, req, 1);
                    cli_table_header("Port  Isolation");
                }
                CPRINTF("%-2u    %s\n", pit.uport, cli_bool_txt(member[pit.iport]));
            }
        }
        if (req->set) {
            (void)pvlan_mgmt_isolate_conf_set(isid, member);
        }
    }
}

static void cli_cmd_pvlan_conf(cli_req_t *req)
{
    if (!req->set) {
        cli_header("Private VLAN Configuration", 1);
    }

    cli_cmd_pvlan_isolate_conf(req);
#if defined(PVLAN_SRC_MASK_ENA)
    cli_cmd_pvlan_privatevid_conf(req, 0, 0, 0);
#endif /* PVLAN_SRC_MASK_ENA */
}

#if defined(PVLAN_SRC_MASK_ENA)
static void cli_cmd_pvlan_add(cli_req_t *req)
{
    cli_cmd_pvlan_privatevid_conf(req, 1, 0, 0);
}

static void cli_cmd_pvlan_delete(cli_req_t *req)
{
    cli_cmd_pvlan_privatevid_conf(req, 0, 1, 0);
}

static void cli_cmd_pvlan_lookup(cli_req_t *req)
{
    cli_cmd_pvlan_privatevid_conf(req, 0, 0, 1);
}
#endif /* PVLAN_SRC_MASK_ENA */

static void cli_cmd_pvlan_default_func(cli_req_t *req)
{
#if defined(PVLAN_SRC_MASK_ENA)
    pvlan_cli_req_t *pvlan_req = req->module_req;

    pvlan_req->privatevid = VTSS_PVLAN_NO_START;
#endif /* PVLAN_SRC_MASK_ENA */
}

/****************************************************************************/
/*  Parameter functions                                                     */
/****************************************************************************/
#if defined(PVLAN_SRC_MASK_ENA)
static int32_t cli_pvlan_id_parse (char *cmd, char *cmd2, char *stx, char *cmd_org,
                                   cli_req_t *req)
{
    int32_t error = 0;
    ulong value = 0;
    pvlan_cli_req_t *pvlan_req = req->module_req;

    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &value, 1, port_isid_port_count(VTSS_ISID_LOCAL));
    if (!error) {
        pvlan_req->privatevid_spec = CLI_SPEC_VAL;
        pvlan_req->privatevid = value;
    }

    return error;
}
#endif /* PVLAN_SRC_MASK_ENA */

/****************************************************************************/
/*  Parameter table                                                         */
/****************************************************************************/

static cli_parm_t pvlan_cli_parm_table[] = {
#if defined(PVLAN_SRC_MASK_ENA)
    {
        "<pvlan_id>",
        "Private VLAN ID, default: Show all PVLANs. The allowed range for a Private\n"
        "VLAN ID is the same as the switch port number range.",
        CLI_PARM_FLAG_SET,
        cli_pvlan_id_parse,
        cli_cmd_pvlan_lookup
    },
    {
        "<pvlan_id>",
        "Private VLAN ID. The allowed range for a Private VLAN ID is the same\n"
        "as the switch port number range.",
        CLI_PARM_FLAG_SET,
        cli_pvlan_id_parse,
        NULL
    },
#endif /* PVLAN_SRC_MASK_ENA */
    {
        "enable|disable",
        "enable     : Enable port isolation\n"
        "disable    : Disable port isolation\n"
        "(default: Show port isolation port list)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        cli_cmd_pvlan_isolate_conf
    },
    {NULL, NULL, 0, 0, NULL}
};

/****************************************************************************/
/*  Command table                                                           */
/****************************************************************************/

enum {
    PRIO_PVLAN_CONF,
    PRIO_PVLAN_ADD,
    PRIO_PVLAN_DEL,
    PRIO_PVLAN_LOOKUP,
    PRIO_PVLAN_ISOLATE,
};

cli_cmd_tab_entry (
    "PVLAN Configuration [<port_list>]",
    NULL,
    "Show Private VLAN configuration",
    PRIO_PVLAN_CONF,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_PVLAN,
    cli_cmd_pvlan_conf,
    cli_cmd_pvlan_default_func,
    pvlan_cli_parm_table,
    CLI_CMD_FLAG_SYS_CONF
);
#if defined(PVLAN_SRC_MASK_ENA)
cli_cmd_tab_entry (
    NULL,
    "PVLAN Add <pvlan_id> [<port_list>]",
    "Add or modify Private VLAN entry",
    PRIO_PVLAN_ADD,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_PVLAN,
    cli_cmd_pvlan_add,
    cli_cmd_pvlan_default_func,
    pvlan_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry (
    NULL,
    "PVLAN Delete <pvlan_id>",
    "Delete Private VLAN entry",
    PRIO_PVLAN_DEL,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_PVLAN,
    cli_cmd_pvlan_delete,
    cli_cmd_pvlan_default_func,
    pvlan_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry (
    "PVLAN Lookup [<pvlan_id>]",
    NULL,
    "Lookup Private VLAN entry",
    PRIO_PVLAN_LOOKUP,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_PVLAN,
    cli_cmd_pvlan_lookup,
    cli_cmd_pvlan_default_func,
    pvlan_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif /* PVLAN_SRC_MASK_ENA */
cli_cmd_tab_entry (
    "PVLAN Isolate [<port_list>]",
    "PVLAN Isolate [<port_list>] [enable|disable]",
    "Set or show the port isolation mode",
    PRIO_PVLAN_ISOLATE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_PVLAN,
    cli_cmd_pvlan_isolate_conf,
    cli_cmd_pvlan_default_func,
    pvlan_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
