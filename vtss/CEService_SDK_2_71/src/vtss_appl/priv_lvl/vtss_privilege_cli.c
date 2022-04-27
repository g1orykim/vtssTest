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

#include "cli.h"
#include "cli_grp_help.h"
#include "vtss_privilege_cli.h"
#include "vtss_privilege_api.h"
#include "sysutil_api.h"
#include "vtss_users_api.h"
#include "cli_trace_def.h"

typedef struct {
    vtss_module_id_t module_id;

    int              cro;
    int              crw;
    int              sro;
    int              srw;
    int              current_privilege_level;
} privilege_cli_req_t;

/****************************************************************************/
/*  Initialization                                                          */
/****************************************************************************/

void privilege_cli_init(void)
{
    /* register the size required for privilege req. structure */
    cli_req_size_register(sizeof(privilege_cli_req_t));
}

/****************************************************************************/
/*  Command functions                                                       */
/****************************************************************************/

static void PRIV_LVL_cli_cmd_current(cli_req_t *req)
{
    privilege_cli_req_t *privilege_cli_req = req->module_req;

    CPRINTF("Privilege Current Level: %d\n", privilege_cli_req->current_privilege_level);

    return;
}

static void PRIV_LVL_cli_cmd_conf(cli_req_t *req)
{
    vtss_priv_conf_t    conf;
    int                 cnt = 0;
    char                priv_group_name[VTSS_PRIV_LVL_NAME_LEN_MAX] = "";
    vtss_module_id_t    module_id;

    PRIV_LVL_cli_cmd_current(req);
    if (vtss_priv_mgmt_conf_get(&conf) != VTSS_OK) {
        return;
    }

    while (vtss_privilege_group_name_get(priv_group_name, &module_id, TRUE)) {
        if (++cnt == 1) {
            CPRINTF("Group Name                       Privilege Level\n");
            CPRINTF("                                 CRO CRW SRO SRW\n");
            CPRINTF("-------------------------------- --- --- --- ---\n");
        }
        CPRINTF("%-32s %3d %3d %3d %3d\n",
                priv_group_name,
                conf.privilege_level[module_id].cro,
                conf.privilege_level[module_id].crw,
                conf.privilege_level[module_id].sro,
                conf.privilege_level[module_id].srw);
    };
}

static void PRIV_LVL_cli_req_def_set(cli_req_t *req)
{
    privilege_cli_req_t *priv_req = req->module_req;

    priv_req->current_privilege_level = cli_get_io_handle()->current_privilege_level;
    priv_req->module_id = VTSS_MODULE_ID_NONE;
    return;
}

static void PRIV_LVL_cli_cmd_conf_disp(cli_req_t *req)
{
    if (!req->set) {
        cli_header("Privilege Level Configuration", 1);
    }

    PRIV_LVL_cli_cmd_conf(req);
    return;
}

static void PRIV_LVL_cli_cmd_group(cli_req_t *req)
{
    users_conf_t        users_conf;
    vtss_priv_conf_t    priv_conf;
    int                 changed = 0;
    privilege_cli_req_t *privilege_cli_req = req->module_req;

    if (vtss_priv_mgmt_conf_get(&priv_conf) != VTSS_OK) {
        return;
    }

    if (req->set) {
        if (privilege_cli_req->module_id == VTSS_MODULE_ID_MISC) {
            strcpy(users_conf.username, VTSS_SYS_ADMIN_NAME);
            if (vtss_users_mgmt_conf_get(&users_conf, FALSE) != VTSS_OK) {
                return;
            }

            if (privilege_cli_req->crw > users_conf.privilege_level) {
                CPRINTF("The privilege level of '%s' is %d.\n", VTSS_SYS_ADMIN_NAME, users_conf.privilege_level);
                CPRINTF("Change to lower privilege level will lock youself out.\n");
                return;
            }
        }

        if (privilege_cli_req->cro && priv_conf.privilege_level[privilege_cli_req->module_id].cro != privilege_cli_req->cro) {
            priv_conf.privilege_level[privilege_cli_req->module_id].cro = privilege_cli_req->cro;
            changed = 1;
        }
        if (privilege_cli_req->crw && priv_conf.privilege_level[privilege_cli_req->module_id].crw != privilege_cli_req->crw) {
            priv_conf.privilege_level[privilege_cli_req->module_id].crw = privilege_cli_req->crw;
            changed = 1;
        }
        if (privilege_cli_req->sro && priv_conf.privilege_level[privilege_cli_req->module_id].sro != privilege_cli_req->sro) {
            priv_conf.privilege_level[privilege_cli_req->module_id].sro = privilege_cli_req->sro;
            changed = 1;
        }
        if (privilege_cli_req->srw && priv_conf.privilege_level[privilege_cli_req->module_id].srw != privilege_cli_req->srw) {
            priv_conf.privilege_level[privilege_cli_req->module_id].srw = privilege_cli_req->srw;
            changed = 1;
        }
        if (changed) {
            if (priv_conf.privilege_level[privilege_cli_req->module_id].cro > priv_conf.privilege_level[privilege_cli_req->module_id].crw ||
                priv_conf.privilege_level[privilege_cli_req->module_id].sro > priv_conf.privilege_level[privilege_cli_req->module_id].srw) {
                CPRINTF("The privilege level of 'Read-only' should be less than or equal to 'Read-write'\n");
                return;
            }
            if (priv_conf.privilege_level[privilege_cli_req->module_id].crw < priv_conf.privilege_level[privilege_cli_req->module_id].sro) {
                CPRINTF("The privilege level of 'Configuration/Execute Read-write' should be greater than or equal to 'Status/Statistics Read-only'\n");
                return;
            }
            if (vtss_priv_mgmt_conf_set(&priv_conf) != VTSS_OK) {
                return;
            }
        }
    } else {
        CPRINTF("Group Name                       Privilege Level\n");
        CPRINTF("                                 CRO CRW SRO SRW\n");
        CPRINTF("-------------------------------- --- --- --- ---\n");
        CPRINTF("%-32s %3d %3d %3d %3d\n",
                vtss_module_names[privilege_cli_req->module_id],
                priv_conf.privilege_level[privilege_cli_req->module_id].cro,
                priv_conf.privilege_level[privilege_cli_req->module_id].crw,
                priv_conf.privilege_level[privilege_cli_req->module_id].sro,
                priv_conf.privilege_level[privilege_cli_req->module_id].srw);
    }

    return;
}

/****************************************************************************/
/*  Parameter functions                                                     */
/****************************************************************************/

static int32_t PRIV_LVL_cli_group_name_parse(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t error = 1;
    privilege_cli_req_t *privilege_cli_req;

    req->parm_parsed = 1;
    privilege_cli_req = req->module_req;

    if (vtss_privilege_module_to_val(cmd, &privilege_cli_req->module_id)) {
        error = 0;
    }

    return error;
}

static int32_t PRIV_LVL_cli_cro_parse(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;
    privilege_cli_req_t *privilege_cli_req;

    req->parm_parsed = 1;
    privilege_cli_req = req->module_req;
    error = cli_parse_ulong(cmd, (u32 *)&privilege_cli_req->cro, 1, VTSS_USERS_MAX_PRIV_LEVEL);

    return error;
}

static int32_t PRIV_LVL_cli_crw_parse(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;
    privilege_cli_req_t *privilege_cli_req;

    req->parm_parsed = 1;
    privilege_cli_req = req->module_req;
    error = cli_parse_ulong(cmd, (u32 *)&privilege_cli_req->crw, 1, VTSS_USERS_MAX_PRIV_LEVEL);

    return error;
}

static int32_t PRIV_LVL_cli_sro_parse(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;
    privilege_cli_req_t *privilege_cli_req;

    req->parm_parsed = 1;
    privilege_cli_req = req->module_req;
    error = cli_parse_ulong(cmd, (u32 *)&privilege_cli_req->sro, 1, VTSS_USERS_MAX_PRIV_LEVEL);

    return error;
}

static int32_t PRIV_LVL_cli_srw_parse(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;
    privilege_cli_req_t *privilege_cli_req;

    req->parm_parsed = 1;
    privilege_cli_req = req->module_req;
    error = cli_parse_ulong(cmd, (u32 *)&privilege_cli_req->srw, 1, VTSS_USERS_MAX_PRIV_LEVEL);

    return error;
}

/****************************************************************************/
/*  Parameter table                                                         */
/****************************************************************************/

static cli_parm_t privilege_cli_parm_table[] = {
    {
        "<group_name>",
        "Privilege group name",
        CLI_PARM_FLAG_NONE,
        PRIV_LVL_cli_group_name_parse,
        PRIV_LVL_cli_cmd_group
    },
    {
        "<cro>",
        "Configuration read-only privilege level (1-" vtss_xstr(VTSS_USERS_MAX_PRIV_LEVEL) ")",
        CLI_PARM_FLAG_SET,
        PRIV_LVL_cli_cro_parse,
        PRIV_LVL_cli_cmd_group
    },
    {
        "<crw>",
        "Configuration/Execute read-write privilege level (1-" vtss_xstr(VTSS_USERS_MAX_PRIV_LEVEL) ")",
        CLI_PARM_FLAG_SET,
        PRIV_LVL_cli_crw_parse,
        PRIV_LVL_cli_cmd_group
    },
    {
        "<sro>",
        "Status/Statistics read-only privilege level (1-" vtss_xstr(VTSS_USERS_MAX_PRIV_LEVEL) ")",
        CLI_PARM_FLAG_SET,
        PRIV_LVL_cli_sro_parse,
        PRIV_LVL_cli_cmd_group
    },
    {
        "<srw>",
        "Status/Statistics read-write privilege level (1-" vtss_xstr(VTSS_USERS_MAX_PRIV_LEVEL) ")",
        CLI_PARM_FLAG_SET,
        PRIV_LVL_cli_srw_parse,
        PRIV_LVL_cli_cmd_group
    },
    {NULL, NULL, 0, 0, NULL}
};

/****************************************************************************/
/*  Command table                                                           */
/****************************************************************************/

/* Users privilege level CLI Command Sorting Order */
enum {
    PRIV_LVL_PRIO_CONF,
    PRIV_LVL_PRIO_GROUP,
    PRIV_LVL_PRIO_CURRENT
};

cli_cmd_tab_entry(
    VTSS_CLI_GRP_SEC_SWITCH_PATH "Privilege Level Configuration",
    NULL,
    "Show privilege configuration",
    PRIV_LVL_PRIO_CONF,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_MISC,
    PRIV_LVL_cli_cmd_conf_disp,
    PRIV_LVL_cli_req_def_set,
    privilege_cli_parm_table,
    CLI_CMD_FLAG_SYS_CONF
);

cli_cmd_tab_entry(
    NULL,
    VTSS_CLI_GRP_SEC_SWITCH_PATH "Privilege Level Group <group_name>\n"
    "         [<cro>] [<crw>] [<sro>] [<srw>]",
    "Configure a privilege level group",
    PRIV_LVL_PRIO_GROUP,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_MISC,
    PRIV_LVL_cli_cmd_group,
    PRIV_LVL_cli_req_def_set,
    privilege_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    VTSS_CLI_GRP_SEC_SWITCH_PATH "Privilege Level Current",
    NULL,
    "Show the current privilege level",
    PRIV_LVL_PRIO_CURRENT,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_MISC,
    PRIV_LVL_cli_cmd_current,
    PRIV_LVL_cli_req_def_set,
    privilege_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
