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
#include "conf_api.h"
#include "msg_api.h"
#include "critd_api.h"
#include "misc_api.h"
#include "vtss_users_api.h"
#include "vtss_users.h"
#include "sysutil_api.h"

#ifdef VTSS_SW_OPTION_PRIV_LVL
#include "vtss_privilege_api.h"
#endif

#ifdef VTSS_SW_OPTION_AUTH
#include "vtss_auth_api.h"
#endif /* VTSS_SW_OPTION_AUTH */

#ifdef VTSS_SW_OPTION_VCLI
#include "vtss_users_cli.h"
#endif

#ifdef VTSS_SW_OPTION_ICFG
#include "vtss_users_icfg.h"
#endif

/****************************************************************************/
/*  Global variables                                                        */
/****************************************************************************/

/* Global structure */
static users_global_t VTSS_USERS_global;

#if (VTSS_TRACE_ENABLED)
static vtss_trace_reg_t VTSS_USERS_trace_reg = {
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "users",
    .descr     = "Users module"
};

static vtss_trace_grp_t VTSS_USERS_trace_grps[TRACE_GRP_CNT] = {
    [VTSS_TRACE_GRP_DEFAULT] = {
        .name      = "default",
        .descr     = "Default",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [TRACE_GRP_CRIT] = {
        .name      = "crit",
        .descr     = "Critical regions ",
        .lvl       = VTSS_TRACE_LVL_ERROR,
        .timestamp = 1,
    }
};
#define USERS_CRIT_ENTER() critd_enter(&VTSS_USERS_global.crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define USERS_CRIT_EXIT()  critd_exit( &VTSS_USERS_global.crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#else
#define USERS_CRIT_ENTER() critd_enter(&VTSS_USERS_global.crit)
#define USERS_CRIT_EXIT()  critd_exit( &VTSS_USERS_global.crit)
#endif /* VTSS_TRACE_ENABLED */


/****************************************************************************/
/*  Various local functions                                                 */
/****************************************************************************/

/* Determine if users configuration has changed */
static int VTSS_USERS_conf_changed(u32 old_num, users_conf_t old[VTSS_USERS_NUMBER_OF_USERS], u32 new_num, users_conf_t new[VTSS_USERS_NUMBER_OF_USERS])
{
    return (old_num != new_num ||
            memcmp(new, old, sizeof(users_conf_t) * VTSS_USERS_NUMBER_OF_USERS));
}

/* Set users defaults */
static void VTSS_USERS_default_set(u32 *users_conf_num, users_conf_t conf[VTSS_USERS_NUMBER_OF_USERS])
{
    /* A least one administor user as default setting:
       user name        : admin
       password         :
       privilege level  : 15 */
    *users_conf_num = 1;
    memset(conf, 0x0, sizeof(users_conf_t) * VTSS_USERS_NUMBER_OF_USERS);
    conf[0].valid = 1;
    strcpy(conf[0].username, VTSS_SYS_ADMIN_NAME);
    strcpy(conf[0].password, ""); /* Should be same as sysutil.c/system_default_set() */
    conf[0].privilege_level = 15;
}

/****************************************************************************/
/*  Management functions                                                    */
/****************************************************************************/

/**
  * \brief Retrieve an error string based on a return code
  *        from one of the Users API functions.
  *
  * \param rc [IN]: Error code that must be in the VTSS_USERS_ERROR_xxx range.
  */
char *vtss_users_error_txt(vtss_rc rc)
{
    switch (rc) {
    case VTSS_USERS_ERROR_MUST_BE_MASTER:
        return "Operation only valid on master switch";

    case VTSS_USERS_ERROR_ISID:
        return "Invalid Switch ID";

    case VTSS_USERS_ERROR_INV_PARAM:
        return "Illegal parameter";

    case VTSS_USERS_ERROR_REJECT:
        return "Username and password combination not found";

    case VTSS_USERS_ERROR_CFG_INVALID_USERNAME:
        return "Invalid chars or not null terminated";

    case VTSS_USERS_ERROR_USERS_TABLE_FULL:
        return "Users table full";

    default:
        return "Users: Unknown error code";
    }
}

/* Get users configuration (only for local using) */
static vtss_rc VTSS_USERS_conf_get(u32 *users_conf_num, users_conf_t conf[VTSS_USERS_NUMBER_OF_USERS])
{
    T_D("enter");
    USERS_CRIT_ENTER();

    *users_conf_num = VTSS_USERS_global.users_conf_num;
    memcpy(conf, VTSS_USERS_global.users_conf, sizeof(users_conf_t) * VTSS_USERS_NUMBER_OF_USERS);

    USERS_CRIT_EXIT();
    T_D("exit");

    return VTSS_OK;
}

/* Set users configuration (only for local using) */
static vtss_rc VTSS_USERS_conf_set(u32 users_conf_num, users_conf_t conf[VTSS_USERS_NUMBER_OF_USERS])
{
    vtss_rc             rc      = VTSS_OK;
    int                 changed = 0;

    T_D("enter");

    USERS_CRIT_ENTER();
    if (msg_switch_is_master()) {
        changed = VTSS_USERS_conf_changed(VTSS_USERS_global.users_conf_num, VTSS_USERS_global.users_conf, users_conf_num, conf);
        VTSS_USERS_global.users_conf_num = users_conf_num;
        memcpy(VTSS_USERS_global.users_conf, conf, sizeof(users_conf_t) * VTSS_USERS_NUMBER_OF_USERS);
    } else {
        T_W("not master");
        rc = VTSS_USERS_ERROR_MUST_BE_MASTER;
    }
    USERS_CRIT_EXIT();

    if (changed) {
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
        /* Save changed configuration */
        conf_blk_id_t    blk_id = CONF_BLK_USERS_CONF;
        users_conf_blk_t *users_conf_blk_p;
        if ((users_conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, NULL)) == NULL) {
            T_W("failed to open users table");
        } else {
            users_conf_blk_p->users_conf_num = users_conf_num;
            memcpy(users_conf_blk_p->users_conf, conf, sizeof(users_conf_t) * VTSS_USERS_NUMBER_OF_USERS);
            conf_sec_close(CONF_SEC_GLOBAL, blk_id);
        }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */

#ifdef VTSS_SW_OPTION_AUTH
        vtss_auth_mgmt_httpd_cache_expire(); // Clear httpd cache
#endif /* VTSS_SW_OPTION_AUTH */
    }

    T_D("exit");

    return rc;
}

/**
  * \brief Get the global Users configuration.
  *
  * \param glbl_cfg [OUT]: Pointer to structure that receives
  *                        the current configuration.
  * \param next     [IN]:  Getnext?
  *
  * \return
  *    VTSS_OK on success.\n
  *    VTSS_USERS_ERROR_INV_PARAM if glbl_cfg is NULL.\n
  *    VTSS_USERS_ERROR_MUST_BE_MASTER if called on a slave switch.\n
  *    VTSS_USERS_ERROR_REJECT if get fail.\n
  */
vtss_rc vtss_users_mgmt_conf_get(users_conf_t *glbl_cfg, BOOL next)
{
    u32 i, num, getfirst = 0, found = 0;

    T_D("enter");

    if (glbl_cfg == NULL) {
        T_W("not master");
        T_D("exit");
        return VTSS_USERS_ERROR_INV_PARAM;
    }
    if (!msg_switch_is_master()) {
        T_W("not master");
        T_D("exit");
        return VTSS_USERS_ERROR_MUST_BE_MASTER;
    }

    USERS_CRIT_ENTER();
    if (!strcmp(glbl_cfg->username, "") && next) {
        getfirst = 1;
    }

    for (i = 0, num = 0;
         i < VTSS_USERS_NUMBER_OF_USERS && num < VTSS_USERS_global.users_conf_num;
         i++) {
        if (!VTSS_USERS_global.users_conf[i].valid) {
            continue;
        }
        num++;
        if (getfirst) {
            *glbl_cfg = VTSS_USERS_global.users_conf[i];
            found = 1;
            break;
        } else if (!strcmp(VTSS_USERS_global.users_conf[i].username, glbl_cfg->username)) {
            if (next) {
                if (num == VTSS_USERS_global.users_conf_num) {
                    break;
                }
                i++;
                while (i < VTSS_USERS_NUMBER_OF_USERS) {
                    if (VTSS_USERS_global.users_conf[i].valid) {
                        *glbl_cfg = VTSS_USERS_global.users_conf[i];
                        found = 1;
                        break;
                    }
                    i++;
                }
                break;
            } else {
                *glbl_cfg = VTSS_USERS_global.users_conf[i];
                found = 1;
            }
            break;
        }
    }
    USERS_CRIT_EXIT();

    T_D("exit");
    if (found) {
        return VTSS_OK;
    } else {
        return VTSS_USERS_ERROR_REJECT;
    }
}

/**
  * \brief Set the global Users configuration.
  *
  * \param glbl_cfg [IN]: Pointer to structure that contains the
  *                       global configuration to apply to the
  *                       voice VLAN module.
  *
  * \return
  *    VTSS_OK on success.\n
  *    VTSS_USERS_ERROR_INV_PARAM if glbl_cfg is NULL or parameters error.\n
  *    VTSS_USERS_ERROR_MUST_BE_MASTER if called on a slave switch.\n
  *    VTSS_USERS_ERROR_CFG_INVALID_USERNAME if user name is null string.\n
  *    VTSS_USERS_ERROR_USERS_TABLE_FULL if users table is full.\n
  *    Others value is caused form other modules.\n
  */
vtss_rc vtss_users_mgmt_conf_set(users_conf_t *glbl_cfg)
{
    vtss_rc         rc = VTSS_OK;
    int             changed = 0, found = 0;
    u32             i, num, users_conf_num;
    users_conf_t    users_conf[VTSS_USERS_NUMBER_OF_USERS];

    T_D("enter");

    if (glbl_cfg == NULL) {
        T_W("not master");
        T_D("exit");
        return VTSS_USERS_ERROR_INV_PARAM;
    }
    if (!msg_switch_is_master()) {
        T_W("not master");
        T_D("exit");
        return VTSS_USERS_ERROR_MUST_BE_MASTER;
    }

    /* Check illegal parameter */
    if (strcmp(glbl_cfg->username, VTSS_SYS_ADMIN_NAME) &&
        !vtss_users_mgmt_is_valid_username(glbl_cfg->username)) {
        T_D("exit");
        return VTSS_USERS_ERROR_CFG_INVALID_USERNAME;
    }
    if (!strcmp(glbl_cfg->username, "")) {
        T_D("exit");
        return VTSS_USERS_ERROR_CFG_INVALID_USERNAME;
    }

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (glbl_cfg->privilege_level > VTSS_USERS_MAX_PRIV_LEVEL) {
        T_D("exit");
        return VTSS_USERS_ERROR_INV_PARAM;
    }
    if ((!strcmp(glbl_cfg->username, VTSS_SYS_ADMIN_NAME)) &&
        (!vtss_priv_is_allowed_crw(VTSS_MODULE_ID_MISC, glbl_cfg->privilege_level))) {
        /* Change to lower privilege level will lock youself out */
        T_D("exit");
        return ((vtss_rc) VTSS_USERS_ERROR_INV_PARAM);
    }
#endif

    if ((rc = VTSS_USERS_conf_get(&users_conf_num, users_conf)) != VTSS_OK) {
        T_D("exit");
        return rc;
    }

    USERS_CRIT_ENTER();
    for (i = 0, num = 0;
         i < VTSS_USERS_NUMBER_OF_USERS && num < users_conf_num;
         i++) {
        if (!users_conf[i].valid) {
            continue;
        }
        num++;
        if (!strcmp(users_conf[i].username, glbl_cfg->username)) {
            found = 1;
            break;
        }
    }

    if (i < VTSS_USERS_NUMBER_OF_USERS && found) {
        /* Modify exist entry */
        glbl_cfg->valid = 1;
        if (memcmp(&users_conf[i], glbl_cfg, sizeof(users_conf_t))) {
            users_conf[i] = *glbl_cfg;
            changed = 1;
        }
    } else if (glbl_cfg->valid) {
        /* Add new entry */
        for (i = 0; i < VTSS_USERS_NUMBER_OF_USERS; i++) {
            if (users_conf[i].valid) {
                continue;
            }
            users_conf_num++;
            glbl_cfg->valid = 1;
            users_conf[i] = *glbl_cfg;
            break;
        }
        if (i < VTSS_USERS_NUMBER_OF_USERS) {
            changed = 1;
        } else {
            rc = VTSS_USERS_ERROR_USERS_TABLE_FULL;
        }
    }
    USERS_CRIT_EXIT();

    if (changed) {
        /* Save changed configuration */
        rc = VTSS_USERS_conf_set(users_conf_num, users_conf);
    }

    T_D("exit");
    return rc;
}

/**
 * Delete the Users configuration.
 * \param user_name [IN] The user name
 * \return : VTSS_OK or one of the following
 *  VTSS_USERS_ERROR_GEN (conf is a null pointer)
 *  VTSS_USERS_ERROR_MUST_BE_MASTER
 */
vtss_rc vtss_users_mgmt_conf_del(char *username)
{
    vtss_rc         rc = VTSS_OK;
    int             changed = 0, found = 0;
    u32             i, num, users_conf_num;
    users_conf_t    users_conf[VTSS_USERS_NUMBER_OF_USERS];

    T_D("enter");

    if (!msg_switch_is_master()) {
        T_W("not master");
        return VTSS_USERS_ERROR_MUST_BE_MASTER;
    }

    /* Check illegal parameter */
    if (username == NULL || !strcmp(username, "")) {
        T_D("exit");
        return rc;
    }

    if ((rc = VTSS_USERS_conf_get(&users_conf_num, users_conf)) != VTSS_OK) {
        T_D("exit");
        return rc;
    }

    USERS_CRIT_ENTER();

    if (msg_switch_is_master()) {
        for (i = 0, num = 0;
             i < VTSS_USERS_NUMBER_OF_USERS && num < users_conf_num;
             i++) {
            if (!users_conf[i].valid) {
                continue;
            }
            num++;
            if (!strcmp(users_conf[i].username, username)) {
                found = 1;
                break;
            }
        }

        if (i < VTSS_USERS_NUMBER_OF_USERS && found) {
            users_conf_num--;
            memset(&users_conf[i], 0x0, sizeof(users_conf_t));
            changed = 1;
        }
    } else {
        T_W("not master");
        T_D("exit");
        rc = VTSS_USERS_ERROR_MUST_BE_MASTER;
    }
    USERS_CRIT_EXIT();

    if (changed) {
        /* Save changed configuration */
        rc = VTSS_USERS_conf_set(users_conf_num, users_conf);
    }

    T_D("exit");
    return rc;
}

/**
 * Clear the Users configuration.
 */
vtss_rc vtss_users_mgmt_conf_clear(void)
{
    users_conf_t conf;
    int          admin_cnt = 0;

    T_D("enter");

    if (!msg_switch_is_master()) {
        T_W("not master");
        return VTSS_USERS_ERROR_MUST_BE_MASTER;
    }

    memset(&conf, 0x0, sizeof(conf));
    while (vtss_users_mgmt_conf_get(&conf, 1) == VTSS_OK) {
        if (strcmp(conf.username, VTSS_SYS_ADMIN_NAME)) {
            if (vtss_users_mgmt_conf_del(conf.username) != VTSS_OK) {
                T_D("Calling vtss_users_mgmt_conf_del(%s) failed\n", conf.username);
            }
        } else if (admin_cnt == 0) {
            admin_cnt++;
        } else {
            break;
        }
    }

    T_D("exit");
    return VTSS_OK;
}


/****************************************************************************/
/*  Initialization functions                                                */
/****************************************************************************/

/* Read/create users stack configuration */
static void VTSS_USERS_conf_read_stack(BOOL create)
{
    BOOL                do_create;
    u32                 size, new_users_conf_num = 0;
    users_conf_t        new_users_conf[VTSS_USERS_NUMBER_OF_USERS];
    users_conf_blk_t    *conf_blk_p;
    conf_blk_id_t       blk_id;
    u32                 blk_version;

    T_D("enter, create: %d", create);

    blk_id = CONF_BLK_USERS_CONF;
    blk_version = USERS_CONF_BLK_VERSION;

    /* Read/create users configuration */
    if (misc_conf_read_use()) {
        if ((conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, &size)) == NULL ||
            size != sizeof(*conf_blk_p)) {
            T_W("conf_sec_open failed or size mismatch, creating defaults");
            conf_blk_p = conf_sec_create(CONF_SEC_GLOBAL, blk_id, sizeof(*conf_blk_p));
            do_create = 1;
        } else if (conf_blk_p->version != blk_version) {
            T_W("version mismatch, creating defaults");
            do_create = 1;
        } else {
            do_create = create;
        }
    } else {
        conf_blk_p = NULL;
        do_create  = 1;
    }

    USERS_CRIT_ENTER();
    /* Use default values first. (Quiet lint/Coverity) */
    VTSS_USERS_default_set(&new_users_conf_num, new_users_conf);

    if (do_create) {
        if (conf_blk_p != NULL) {
            conf_blk_p->users_conf_num = new_users_conf_num;
            memcpy(conf_blk_p->users_conf, new_users_conf, sizeof(users_conf_t) * VTSS_USERS_NUMBER_OF_USERS);
        }
    } else {
        /* Use new configuration */
        if (conf_blk_p != NULL) {  // Quiet lint
            new_users_conf_num = conf_blk_p->users_conf_num;
            memcpy(new_users_conf, conf_blk_p->users_conf, sizeof(users_conf_t) * VTSS_USERS_NUMBER_OF_USERS);
        }
    }
    VTSS_USERS_global.users_conf_num = new_users_conf_num;
    memcpy(VTSS_USERS_global.users_conf, new_users_conf, sizeof(users_conf_t) * VTSS_USERS_NUMBER_OF_USERS);
    USERS_CRIT_EXIT();

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (conf_blk_p == NULL) {
        T_W("failed to open users table");
    } else {
        conf_blk_p->version = blk_version;
        conf_sec_close(CONF_SEC_GLOBAL, blk_id);
    }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */

    T_D("exit");
}

/* Module start */
static void VTSS_USERS_start(void)
{
    T_D("enter");

    /* Initialize users configuration */
    VTSS_USERS_default_set(&VTSS_USERS_global.users_conf_num, VTSS_USERS_global.users_conf);

    /* Create semaphore for critical regions */
    critd_init(&VTSS_USERS_global.crit, "VTSS_USERS_global.crit", VTSS_MODULE_ID_USERS, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
    USERS_CRIT_EXIT();

    T_D("exit");
}

/**
  * \brief Initialize the Users module
  *
  * \param cmd [IN]: Reason why calling this function.
  * \param p1  [IN]: Parameter 1. Usage varies with cmd.
  * \param p2  [IN]: Parameter 2. Usage varies with cmd.
  *
  * \return
  *    VTSS_OK.
  */
vtss_rc vtss_users_init(vtss_init_data_t *data)
{
    vtss_isid_t isid = data->isid;
#ifdef VTSS_SW_OPTION_ICFG
    vtss_rc     rc;
#endif

    if (data->cmd == INIT_CMD_INIT) {
        /* Initialize and register trace ressources */
        VTSS_TRACE_REG_INIT(&VTSS_USERS_trace_reg, VTSS_USERS_trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&VTSS_USERS_trace_reg);
    }

    T_D("enter, cmd: %d, isid: %u, flags: 0x%x", data->cmd, data->isid, data->flags);

    switch (data->cmd) {
    case INIT_CMD_INIT:
        T_D("INIT");
#ifdef VTSS_SW_OPTION_VCLI
        users_cli_init();
#endif
        VTSS_USERS_start();

#ifdef VTSS_SW_OPTION_ICFG
        rc = vtss_users_icfg_init();
        if (rc != VTSS_OK) {
            T_D("fail to init icfg registration, rc = %s", error_txt(rc));
        }
#endif
        break;
    case INIT_CMD_START:
        T_D("START");
        break;
    case INIT_CMD_CONF_DEF:
        T_D("CONF_DEF, isid: %d", isid);
        if (isid == VTSS_ISID_LOCAL) {
            /* Reset local configuration */
        } else if (isid == VTSS_ISID_GLOBAL) {
            /* Reset stack configuration */
            VTSS_USERS_conf_read_stack(1);
        } else if (VTSS_ISID_LEGAL(isid)) {
            /* Reset switch configuration */
        }
        break;
    case INIT_CMD_MASTER_UP: {
        T_D("MASTER_UP");

        /* Read stack and switch configuration */
        VTSS_USERS_conf_read_stack(0);
        break;
    }
    case INIT_CMD_MASTER_DOWN:
        T_D("MASTER_DOWN");
        break;
    case INIT_CMD_SWITCH_ADD:
        T_D("SWITCH_ADD, isid: %d", isid);
        break;
    case INIT_CMD_SWITCH_DEL:
        T_D("SWITCH_DEL, isid: %d", isid);
        break;
    default:
        break;
    }

    T_D("exit");

    return VTSS_OK;
}

/* Check if user name string */
BOOL vtss_users_mgmt_is_valid_username(const char *str)
{
    int idx, len = strlen(str);

    if (!len) {
        return FALSE;
    }
    for (idx = 0; idx < len; idx++) {
        if ((str[idx] >= '0' && str[idx] <= '9') ||
            (str[idx] >= 'A' && str[idx] <= 'Z') ||
            (str[idx] >= 'a' && str[idx] <= 'z') ||
            str[idx] == '_') {
            continue;
        } else {
            return FALSE;
        }
    }
    return TRUE;
}

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
