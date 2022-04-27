/*
   Vitesse VLAN Translation software.

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
#include "cli_trace_def.h"
#include "vlan_translation_api.h"
#include "vlan_api.h"

/* VLAN Translation CLI request structure */
typedef struct {
    u16         trans_vid;
    cli_spec_t  trans_vid_spec;
    u16         group_id;
    cli_spec_t  group_id_spec;
} vlan_trans_cli_req_t;

/* VLAN Translation CLI request init function */
void vlan_trans_cli_req_init(void)
{
    /* register the size required for VLAN translation req. structure */
    cli_req_size_register(sizeof(vlan_trans_cli_req_t));
}

/* CLI command handlers */
static void cli_cmd_vlan_trans_entry_add(cli_req_t *req)
{
    vlan_trans_cli_req_t  *vlan_trans_req = req->module_req;
    if (cli_cmd_switch_none(req)) {
        return;
    }
    if (req->vid == vlan_trans_req->trans_vid) {
        CPRINTF("VLAN ID and Translated VLAN ID cannot be same\n");
        return;
    }
    if ((vlan_trans_mgmt_grp2vlan_entry_add(vlan_trans_req->group_id, req->vid,
                                            vlan_trans_req->trans_vid)) != VTSS_OK) {
        CPRINTF("Addition of VLAN Translation entry failed\n");
        return;
    }
}
static void cli_cmd_vlan_trans_entry_delete(cli_req_t *req)
{
    vlan_trans_cli_req_t  *vlan_trans_req = req->module_req;
    if (cli_cmd_switch_none(req)) {
        return;
    }
    if ((vlan_trans_mgmt_grp2vlan_entry_delete(vlan_trans_req->group_id, req->vid)) != VTSS_OK) {
        CPRINTF("Deletion of VLAN Translation entry failed\n");
        return;
    }
}
static void cli_cmd_vlan_trans_group_conf(cli_req_t *req)
{
    vlan_trans_cli_req_t                *vlan_trans_req = req->module_req;
    vlan_trans_mgmt_grp2vlan_conf_t     entry;
    vlan_trans_mgmt_port2grp_conf_t     conf;
    BOOL                                ports[VTSS_PORT_ARRAY_SIZE];
    BOOL                                next, first;
    vtss_port_no_t                      iport;
    i8                                  buf[64];
    if (cli_cmd_switch_none(req)) {
        return;
    }
    for (iport = VTSS_PORT_NO_START; iport < VTSS_PORT_NO_END; iport++) {
        ports[iport] = req->uport_list[iport2uport(iport)];
    }
    if (vlan_trans_req->group_id_spec == CLI_SPEC_VAL) {
        /* Both port_list and group_id are input. This will add the port to group  mapping */
        if ((vlan_trans_mgmt_port2grp_entry_add(vlan_trans_req->group_id, ports)) != VTSS_OK) {
            CPRINTF("Addition of ports to a group failed\n");
            return;
        }
    } else {
        /* Show Group to VLAN Translations */
        next = FALSE;
        first = TRUE;
        entry.group_id = VT_NULL_GROUP_ID;
        while (vlan_trans_mgmt_grp2vlan_entry_get(&entry, next) == VTSS_RC_OK) {
            next = TRUE;
            if (entry.group_id > port_isid_port_count(VTSS_ISID_LOCAL)) {
                continue;
            }
            if (first == TRUE) {
                CPRINTF("Group ID  VID   Trans_VID\n");
                CPRINTF("--------  ----  ---------\n");
                first = FALSE;
            }
            CPRINTF("%-8u  %-4u  %-4u\n", entry.group_id, entry.vid, entry.trans_vid);
        }
        /* Show port to Group mapping */
        first = TRUE;
        next = FALSE;
        conf.group_id = VT_NULL_GROUP_ID;
        while (vlan_trans_mgmt_port2grp_entry_get(&conf, next) == VTSS_RC_OK) {
            next = TRUE;
            if (conf.group_id > port_isid_port_count(VTSS_ISID_LOCAL)) {
                continue;
            }
            if (first == TRUE) {
                CPRINTF("Ports                                               Group ID \n");
                CPRINTF("-----                                               ---------\n");
                first = FALSE;
            }
            CPRINTF("%-50s  %-4u\n", cli_iport_list_txt(conf.ports, buf), conf.group_id);
        }
    }
}

/* Param parse functions */
static int32_t cli_vlan_trans_vid_parse (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    vlan_trans_cli_req_t *vlan_trans_req = req->module_req;
    i32                  error = 0;
    ulong                value = 0;
    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &value, VLAN_ID_MIN, VLAN_ID_MAX);
    if (!error) {
        vlan_trans_req->trans_vid = value;
        vlan_trans_req->trans_vid_spec = CLI_SPEC_VAL;
    }
    return error;
}
static int32_t cli_vlan_trans_group_id_parse (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    vlan_trans_cli_req_t *vlan_trans_req = req->module_req;
    i32                  error = 0;
    ulong                value = 0;

    req->parm_parsed = 1;
    /* Number of groups = number of ports */
    error = cli_parse_ulong(cmd, &value, 1, port_isid_port_count(VTSS_ISID_LOCAL));
    if (!error) {
        vlan_trans_req->group_id = value;
        vlan_trans_req->group_id_spec = CLI_SPEC_VAL;
    }
    return error;
}

/* Param Table Entries*/
static cli_parm_t vlan_trans_cli_parm_table[] = {
    {
        "<trans_vid>",
        "Translation VLAN ID",
        CLI_PARM_FLAG_NONE,
        cli_vlan_trans_vid_parse,
        NULL
    },
    {
        "<group_id>",
        "Group Id: 1 to port count",
        CLI_PARM_FLAG_NONE,
        cli_vlan_trans_group_id_parse,
        NULL
    },
    {NULL, NULL, 0, 0, NULL}
};

/* Vlan Translation CLI Commands sorting order */
enum {
    PRIO_VLAN_TRANS_ADD,
    PRIO_VLAN_TRANS_DEL,
    PRIO_VLAN_TRANS_GET,
};

/* VLAN Translation Command Table Entries */
cli_cmd_tab_entry(
    NULL,
    "VLAN Translation Add <group_id> <vid> <trans_vid>",
    "Add VLAN translation entry into a group",
    PRIO_VLAN_TRANS_ADD,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_VLAN_TRANSLATION,
    cli_cmd_vlan_trans_entry_add,
    NULL,
    vlan_trans_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry(
    NULL,
    "VLAN Translation Delete <group_id> <vid>",
    "Delete VLAN translation entry from a group",
    PRIO_VLAN_TRANS_DEL,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_VLAN_TRANSLATION,
    cli_cmd_vlan_trans_entry_delete,
    NULL,
    vlan_trans_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry(
    "VLAN Translation Group [<port_list>] [<group_id>]",
    "VLAN Translation Group [<port_list>] [<group_id>]",
    "VLAN Translation Group configuration - if only port_list is entered, it will show the\n"
    "Port to Group mapping for each port; if only Group is entered, it will show the VLAN\n"
    "Translations of that group; if both are entered, it will map the ports in port_list to\n"
    "the group",
    PRIO_VLAN_TRANS_GET,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_VLAN_TRANSLATION,
    cli_cmd_vlan_trans_group_conf,
    NULL,
    vlan_trans_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
