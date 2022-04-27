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
#include "vtss_users_cli.h"
#include "vtss_users_api.h"
#include "cli_trace_def.h"
#include "sysutil_api.h"
#ifdef VTSS_SW_OPTION_PRIV_LVL
#include "vtss_module_id.h"
#include "vtss_privilege_api.h"
#endif

typedef struct {
    u8   username[VTSS_SYS_USERNAME_LEN];
} users_cli_req_t;

/****************************************************************************/
/*  Initialization                                                          */
/****************************************************************************/

void users_cli_init(void)
{
    /* register the size required for users req. structure */
    cli_req_size_register(sizeof(users_cli_req_t));
}

/****************************************************************************/
/*  Command functions                                                       */
/****************************************************************************/

static void USERS_cli_cmd_conf(cli_req_t *req)
{
    users_conf_t conf;
    int          cnt = 0;

    memset(&conf, 0x0, sizeof(conf));
    while (vtss_users_mgmt_conf_get(&conf, 1) == VTSS_OK) {
#ifdef VTSS_SW_OPTION_PRIV_LVL
        if (++cnt == 1) {
            CPRINTF("User Name                        Privilege Level\n");
            CPRINTF("-------------------------------- ---------------\n");
        }
        CPRINTF("%-32s %16d\n", conf.username, conf.privilege_level);
#else
        if (++cnt == 1) {
            CPRINTF("User Name                       \n");
            CPRINTF("--------------------------------\n");
        }
        CPRINTF("%-32s\n", conf.username);
#endif
    }
}

static void USERS_cli_cmd_conf_disp(cli_req_t *req)
{
    if (!req->set) {
        cli_header("Users Configuration", 1);
    }
    USERS_cli_cmd_conf(req);
    return;
}

static void USERS_cli_cmd_add(cli_req_t *req)
{
    users_conf_t    conf;
    vtss_rc         rc;
    users_cli_req_t *users_req = req->module_req;

    conf.valid = 1;
    strcpy(conf.username, (i8 *) users_req->username);
    /* Get user entry first if it existing.
       Ingnore the error return value, it will be a new user */
    (void) vtss_users_mgmt_conf_get(&conf, FALSE);
    strcpy(conf.password, req->parm);

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (!strcmp((i8 *) users_req->username, VTSS_SYS_ADMIN_NAME)) {
        if (conf.privilege_level != req->value) {
            CPRINTF("Cannot change the privilege level of '%s'.\n", VTSS_SYS_ADMIN_NAME);
            return;
        }
        if (!vtss_priv_is_allowed_crw(VTSS_MODULE_ID_MISC, conf.privilege_level)) {
            CPRINTF("The privilege level of group '%s' is %d.\n", vtss_module_names[VTSS_MODULE_ID_MISC], conf.privilege_level);
            CPRINTF("Change to lower privilege level will lock youself out.\n");
            return;
        }
    } else {
        conf.privilege_level = req->value;
    }
#endif

    if ((rc = vtss_users_mgmt_conf_set(&conf)) != VTSS_OK) {
        if (rc == VTSS_USERS_ERROR_USERS_TABLE_FULL) {
            CPRINTF("The maximum users number is %d.\n", VTSS_USERS_NUMBER_OF_USERS);
        } else {
            CPRINTF("%s\n", error_txt(rc));
        }
    }

    return;
}

static void USERS_cli_cmd_del(cli_req_t *req)
{
    vtss_rc         rc;
    users_cli_req_t *users_req = req->module_req;
    users_conf_t    conf;

    if (!strcmp((i8 *) users_req->username, VTSS_SYS_ADMIN_NAME)) {
        CPRINTF("The user name of %s doesn't allow delete\n", VTSS_SYS_ADMIN_NAME);
        return;
    }
    strcpy(conf.username, (i8 *) users_req->username);
    if (vtss_users_mgmt_conf_get(&conf, FALSE) != VTSS_OK) {
        CPRINTF("Non-existing entry\n");
        return;
    }
    if ((rc = vtss_users_mgmt_conf_del((i8 *) users_req->username)) != VTSS_OK) {
        CPRINTF("%s\n", error_txt(rc));
    }

    return;
}

/****************************************************************************/
/*  Parameter functions                                                     */
/****************************************************************************/

static int32_t USERS_cli_username_parse(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;
    users_cli_req_t *users_req;

    req->parm_parsed = 1;
    users_req = req->module_req;
    error = cli_parse_string(cmd_org, (i8 *) users_req->username, 1, VTSS_SYS_USERNAME_LEN);

    return error ? error : !vtss_users_mgmt_is_valid_username((i8 *) users_req->username);
}

static int32_t USERS_cli_password_parse(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;

    req->parm_parsed = 1;
    error = cli_parse_text(cmd_org, req->parm, VTSS_SYS_PASSWD_LEN);

    return error;
}

static int32_t USERS_cli_priv_lvl_parse(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;

    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &req->value, 1, VTSS_USERS_MAX_PRIV_LEVEL);

    return error;
}

/****************************************************************************/
/*  Parameter table                                                         */
/****************************************************************************/

static cli_parm_t users_cli_parm_table[] = {
    {
        "<user_name>",
        "A string identifying the user name that this entry should\n"
        "                   belong to. The allowed string length is (1-" vtss_xstr(VTSS_SYS_INPUT_USERNAME_LEN) "). The valid\n"
        "                   user name is a combination of letters, numbers\n"
        "                   and underscores",
        CLI_PARM_FLAG_SET,
        USERS_cli_username_parse,
        NULL
    },
    {
        "<password>",
        "The password for this user name. The allowed string length is\n"
        "                   (0-" vtss_xstr(VTSS_SYS_INPUT_PASSWD_LEN) "). Use 'clear' or \"\" as null string",
        CLI_PARM_FLAG_NONE,
        USERS_cli_password_parse,
        USERS_cli_cmd_add
    },
    {
        "<privilege_level>",
        "User privilege level (1-" vtss_xstr(VTSS_USERS_MAX_PRIV_LEVEL) ")",
        CLI_PARM_FLAG_SET,
        USERS_cli_priv_lvl_parse,
        USERS_cli_cmd_add
    },
    {NULL, NULL, 0, 0, NULL}
};

/****************************************************************************/
/*  Command table                                                           */
/****************************************************************************/

/* Users CLI Command Sorting Order */
enum {
    USERS_PRIO_CONF,
    USERS_PRIO_ADD,
    USERS_PRIO_DEL
};

cli_cmd_tab_entry(
    VTSS_CLI_GRP_SEC_SWITCH_PATH "Users Configuration",
    NULL,
    "Show users configuration",
    USERS_PRIO_CONF,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_MISC,
    USERS_cli_cmd_conf_disp,
    NULL,
    users_cli_parm_table,
    CLI_CMD_FLAG_SYS_CONF
);

#ifdef VTSS_SW_OPTION_PRIV_LVL
cli_cmd_tab_entry(
    NULL,
    VTSS_CLI_GRP_SEC_SWITCH_PATH "Users Add <user_name> <password> <privilege_level>",
    "Add or modify users entry",
    USERS_PRIO_ADD,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_MISC,
    USERS_cli_cmd_add,
    NULL,
    users_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#else
cli_cmd_tab_entry(
    NULL,
    VTSS_CLI_GRP_SEC_SWITCH_PATH "Users Add <user_name> <password>",
    "Add or modify users entry",
    USERS_PRIO_ADD,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_MISC,
    USERS_cli_cmd_add,
    NULL,
    users_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif

cli_cmd_tab_entry(
    NULL,
    VTSS_CLI_GRP_SEC_SWITCH_PATH "Users Delete <user_name>",
    "Delete users entry",
    USERS_PRIO_DEL,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_MISC,
    USERS_cli_cmd_del,
    NULL,
    users_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
