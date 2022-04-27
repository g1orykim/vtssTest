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
#include "vtss_privilege_api.h"
#include "vtss_privilege.h"
#include "sysutil_api.h"
#include "vtss_users_api.h"

#ifdef VTSS_SW_OPTION_VCLI
#include "vtss_privilege_cli.h"
#endif

#ifdef VTSS_SW_OPTION_ICFG
#include "vtss_privilege_icfg.h"
#endif


#define VTSS_ALLOC_MODULE_ID    VTSS_MODULE_ID_PRIV_LVL

/****************************************************************************/
/*  Global variables                                                        */
/****************************************************************************/

/* Global structure */
static privilege_global_t VTSS_PRIVILEGE_global;

#if (VTSS_TRACE_ENABLED)
static vtss_trace_reg_t VTSS_PRIVILEGE_trace_reg = {
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "priv_lvl",
    .descr     = "Privilege level"
};

static vtss_trace_grp_t VTSS_PRIVILEGE_trace_grps[TRACE_GRP_CNT] = {
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
#define PRIVILEGE_CRIT_ENTER() critd_enter(&VTSS_PRIVILEGE_global.crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define PRIVILEGE_CRIT_EXIT()  critd_exit( &VTSS_PRIVILEGE_global.crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#else
#define PRIVILEGE_CRIT_ENTER() critd_enter(&VTSS_PRIVILEGE_global.crit)
#define PRIVILEGE_CRIT_EXIT()  critd_exit( &VTSS_PRIVILEGE_global.crit)
#endif /* VTSS_TRACE_ENABLED */


/****************************************************************************/
/*  Various local functions                                                 */
/****************************************************************************/

/* Determine if privilege configuration has changed */
static int VTSS_PRIVILEGE_conf_changed(vtss_priv_conf_t *old, vtss_priv_conf_t *new)
{
    return (memcmp(old, new, sizeof(*new)));
}

/* Set privilege defaults */
void VTSS_PRIVILEGE_default_get(vtss_priv_conf_t *conf)
{
    int i;

    for (i = 0; i < VTSS_MODULE_ID_NONE; i++) {
        switch (i) {
        case VTSS_MODULE_ID_SYSTEM:
        case VTSS_MODULE_ID_PORT:
        case VTSS_MODULE_ID_TOPO:
            conf->privilege_level[i].cro = 5;
            conf->privilege_level[i].crw = 10;
            conf->privilege_level[i].sro = 1;
            conf->privilege_level[i].srw = 10;
            break;
        case VTSS_MODULE_ID_MISC:
        case VTSS_MODULE_ID_DEBUG:
            conf->privilege_level[i].cro = 15;
            conf->privilege_level[i].crw = 15;
            conf->privilege_level[i].sro = 15;
            conf->privilege_level[i].srw = 15;
            break;
        default:
            conf->privilege_level[i].cro = 5;
            conf->privilege_level[i].crw = 10;
            conf->privilege_level[i].sro = 5;
            conf->privilege_level[i].srw = 10;
            break;
        }
    }
}

/****************************************************************************/
/*  Management functions                                                    */
/****************************************************************************/

/**
  * \brief Retrieve an error string based on a return code
  *        from one of the Privilege Level API functions.
  *
  * \param rc [IN]: Error code that must be in the VTSS_PRIV_ERROR_xxx range.
  */
char *vtss_privilege_error_txt(vtss_rc rc)
{
    switch (rc) {
    case VTSS_PRIV_ERROR_MUST_BE_MASTER:
        return "Operation only valid on master switch";

    case VTSS_PRIV_ERROR_ISID:
        return "Invalid Switch ID";

    case VTSS_PRIV_ERROR_INV_PARAM:
        return "Invalid parameter supplied to function";

    default:
        return "Privilege Level: Unknown error code";
    }
}

/**
  * \brief Get the global Privilege Level configuration.
  *
  * \param glbl_cfg [OUT]: Pointer to structure that receives
  *                        the current configuration.
  *
  * \return
  *    VTSS_OK on success.\n
  *    VTSS_PRIV_ERROR_INV_PARAM if glbl_cfg is NULL.\n
  *    VTSS_PRIV_ERROR_MUST_BE_MASTER if called on a slave switch.\n
  */
vtss_rc vtss_priv_mgmt_conf_get(vtss_priv_conf_t *glbl_cfg)
{
    T_D("enter");
    if (glbl_cfg == NULL) {
        T_W("not master");
        T_D("exit");
        return VTSS_PRIV_ERROR_INV_PARAM;
    }
    if (!msg_switch_is_master()) {
        T_W("not master");
        T_D("exit");
        return VTSS_PRIV_ERROR_MUST_BE_MASTER;
    }

    PRIVILEGE_CRIT_ENTER();
    *glbl_cfg = VTSS_PRIVILEGE_global.privilege_conf;
    PRIVILEGE_CRIT_EXIT();

    T_D("exit");
    return VTSS_OK;
}

/**
  * \brief Set the global Privilege Level configuration.
  *
  * \param glbl_cfg [IN]: Pointer to structure that contains the
  *                       global configuration to apply to the
  *                       voice VLAN module.
  *
  * \return
  *    VTSS_OK on success.\n
  *    VTSS_PRIV_ERROR_INV_PARAM if glbl_cfg is NULL or parameters error.\n
  *    VTSS_PRIV_ERROR_MUST_BE_MASTER if called on a slave switch.\n
  *    Others value is caused form other modules.\n
  */
vtss_rc vtss_priv_mgmt_conf_set(vtss_priv_conf_t *glbl_cfg)
{
    vtss_rc                 rc = VTSS_OK;
    int                     i, changed = 0;
    users_conf_t            users_conf;

    T_D("enter");

    if (glbl_cfg == NULL) {
        T_W("not master");
        T_D("exit");
        return VTSS_PRIV_ERROR_INV_PARAM;
    }
    if (!msg_switch_is_master()) {
        T_W("not master");
        T_D("exit");
        return VTSS_PRIV_ERROR_MUST_BE_MASTER;
    }

    /* Check illegal parameter */
    for (i = 0; i < VTSS_MODULE_ID_NONE; i++) {
        /* The privilege level of 'Read-only' should be less or equal 'Read-write' */
        if (glbl_cfg->privilege_level[i].cro < VTSS_PRIV_LVL_MIN ||
            glbl_cfg->privilege_level[i].cro > VTSS_PRIV_LVL_MAX ||
            glbl_cfg->privilege_level[i].crw < VTSS_PRIV_LVL_MIN ||
            glbl_cfg->privilege_level[i].crw > VTSS_PRIV_LVL_MAX ||
            glbl_cfg->privilege_level[i].sro < VTSS_PRIV_LVL_MIN ||
            glbl_cfg->privilege_level[i].sro > VTSS_PRIV_LVL_MAX ||
            glbl_cfg->privilege_level[i].srw < VTSS_PRIV_LVL_MIN ||
            glbl_cfg->privilege_level[i].srw > VTSS_PRIV_LVL_MAX) {
            T_D("exit");
            return VTSS_PRIV_ERROR_INV_PARAM;
        }

        /* The privilege level of 'Read-only' should be less or equal 'Read-write' */
        if (glbl_cfg->privilege_level[i].cro > glbl_cfg->privilege_level[i].crw || glbl_cfg->privilege_level[i].sro > glbl_cfg->privilege_level[i].srw) {
            T_D("exit");
            return VTSS_PRIV_ERROR_INV_PARAM;
        }

        /* The privilege level of 'Configuration/Execute Read-write' should be great or equal 'Status/Statistics Read-only' */
        if (glbl_cfg->privilege_level[i].crw < glbl_cfg->privilege_level[i].sro) {
            T_D("exit");
            return VTSS_PRIV_ERROR_INV_PARAM;
        }
    }

    /* Change to lower privilege level will lock youself out */
    strcpy(users_conf.username, VTSS_SYS_ADMIN_NAME);
    if (vtss_users_mgmt_conf_get(&users_conf, FALSE) != VTSS_OK) {
        T_D("exit");
        return VTSS_PRIV_ERROR_INV_PARAM;
    }
    if (glbl_cfg->privilege_level[VTSS_MODULE_ID_MISC].crw > users_conf.privilege_level) {
        T_D("exit");
        return VTSS_PRIV_ERROR_INV_PARAM;
    }

    PRIVILEGE_CRIT_ENTER();
    changed = VTSS_PRIVILEGE_conf_changed(&VTSS_PRIVILEGE_global.privilege_conf, glbl_cfg);
    VTSS_PRIVILEGE_global.privilege_conf = *glbl_cfg;
    PRIVILEGE_CRIT_EXIT();

    if (changed) {
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
        /* Save changed configuration */
        conf_blk_id_t        blk_id  = CONF_BLK_PRIVILEGE_CONF;
        privilege_conf_blk_t *privilege_conf_blk_p;
        if ((privilege_conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, NULL)) == NULL) {
            T_W("failed to open privilege table");
        } else {
            privilege_conf_blk_p->privilege_conf = *glbl_cfg;
            conf_sec_close(CONF_SEC_GLOBAL, blk_id);
        }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
    }

    T_D("exit");
    return rc;
}

/**
  * \brief Verify privilege level is allowed for 'Configuration Read-only'
  *
  * Used by CLI and Web module in order to get the configured privilege
  * levels for a specific privilege level group.
  *
  * \param id               [IN]: The module ID.
  * \param current_level    [IN]: The current privilege level.
  *
  * \return
  *    TRUE if the module name is found.\n
  *    FALSE otherwise.\n
  */
BOOL vtss_priv_is_allowed_cro(vtss_module_id_t id, int current_level)
{
    T_D("enter");
    PRIVILEGE_CRIT_ENTER();
    if (current_level >= VTSS_PRIVILEGE_global.privilege_conf.privilege_level[id].cro) {
        PRIVILEGE_CRIT_EXIT();
        T_D("exit");
        return TRUE;
    } else {
        PRIVILEGE_CRIT_EXIT();
        T_D("exit");
        return FALSE;
    }
}

/**
  * \brief Verify privilege level is allowed for 'Configuration Read-write'
  *
  * Used by CLI and Web module in order to get the configured privilege
  * levels for a specific privilege level group.
  *
  * \param id               [IN]: The module ID.
  * \param current_level    [IN]: The current privilege level.
  *
  * \return
  *    TRUE if the module name is found.\n
  *    FALSE otherwise.\n
  */
BOOL vtss_priv_is_allowed_crw(vtss_module_id_t id, int current_level)
{
    T_D("enter");
    PRIVILEGE_CRIT_ENTER();
    if (current_level >= VTSS_PRIVILEGE_global.privilege_conf.privilege_level[id].crw) {
        PRIVILEGE_CRIT_EXIT();
        T_D("exit");
        return TRUE;
    } else {
        PRIVILEGE_CRIT_EXIT();
        T_D("exit");
        return FALSE;
    }
}

/**
  * \brief Verify privilege level is allowed for ''Status/Statistics Read-only'
  *
  * Used by CLI and Web module in order to get the configured privilege
  * levels for a specific privilege level group.
  *
  * \param id               [IN]: The module ID.
  * \param current_level    [IN]: The current privilege level.
  *
  * \return
  *    TRUE if the module name is found.\n
  *    FALSE otherwise.\n
  */
BOOL vtss_priv_is_allowed_sro(vtss_module_id_t id, int current_level)
{
    T_D("exit");
    PRIVILEGE_CRIT_ENTER();
    if (current_level >= VTSS_PRIVILEGE_global.privilege_conf.privilege_level[id].sro) {
        PRIVILEGE_CRIT_EXIT();
        T_D("exit");
        return TRUE;
    } else {
        PRIVILEGE_CRIT_EXIT();
        T_D("exit");
        return FALSE;
    }
}

/**
  * \brief Verify privilege level is allowed for ''Status/Statistics Read-write'
  *
  * Used by CLI and Web module in order to get the configured privilege
  * levels for a specific privilege level group.
  *
  * \param id               [IN]: The module ID.
  * \param current_level    [IN]: The current privilege level.
  *
  * \return
  *    TRUE if the module name is found.\n
  *    FALSE otherwise.\n
  */
BOOL vtss_priv_is_allowed_srw(vtss_module_id_t id, int current_level)
{
    T_D("enter");
    PRIVILEGE_CRIT_ENTER();
    if (current_level >= VTSS_PRIVILEGE_global.privilege_conf.privilege_level[id].srw) {
        PRIVILEGE_CRIT_EXIT();
        T_D("exit");
        return TRUE;
    } else {
        PRIVILEGE_CRIT_EXIT();
        T_D("exit");
        return FALSE;
    }
}


/****************************************************************************/
/*  Configuration silent upgrade                                            */
/****************************************************************************/

/* Silent upgrade from old configuration to new one.
 * Returns a (malloc'ed) pointer to the upgraded new configuration
 * or NULL if conversion failed.
 */
static privilege_conf_blk_t *VTSS_PRIVILEGE_conf_flash_silent_upgrade(const void *blk, u32 old_size)
{
    privilege_conf_blk_t *new_blk = NULL;

    if ((new_blk = VTSS_MALLOC(sizeof(*new_blk)))) {
        privilege_conf_blk_t    *old_blk = (privilege_conf_blk_t *)blk;
        u32                     idx, new_cnt, old_cnt, min_cnt;

        new_cnt = sizeof(new_blk->privilege_conf.privilege_level) / sizeof(vtss_priv_module_conf_t);
        old_cnt = (old_size - sizeof(old_blk->version)) / sizeof(vtss_priv_module_conf_t);
        min_cnt = new_cnt > old_cnt ? old_cnt : new_cnt;

        /* upgrade configuration from old to current version */
        VTSS_PRIVILEGE_default_get(&new_blk->privilege_conf); // Initiate1 with default values
        new_blk->version = PRIVILEGE_CONF_BLK_VERSION;
        for (idx = 0; idx < min_cnt - 1; idx++) {   // Don't count the last one.(VTSS_MODULE_ID_NONE)
            memcpy(&new_blk->privilege_conf.privilege_level[idx], &old_blk->privilege_conf.privilege_level[idx], sizeof(vtss_priv_module_conf_t));
        }
    }
    return new_blk;
}


/****************************************************************************/
/*  Initialization functions                                                */
/****************************************************************************/

/* Read/create privilege stack configuration */
static void VTSS_PRIVILEGE_conf_read_stack(BOOL create)
{
    BOOL                    do_create = create;
    u32                     size;
    privilege_conf_blk_t    *conf_blk_p;
    conf_blk_id_t           blk_id;

    T_D("enter, create: %d", create);

    /* Read/create privilege configuration */
    blk_id = CONF_BLK_PRIVILEGE_CONF;

    if (misc_conf_read_use()) {
        if ((conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, &size)) == NULL) {
            T_W("conf_sec_open failed, creating defaults");
            conf_blk_p = conf_sec_create(CONF_SEC_GLOBAL, blk_id, sizeof(*conf_blk_p));
            do_create = 1;
        } else if (conf_blk_p && size != sizeof(*conf_blk_p)) {
            privilege_conf_blk_t *new_blk;
            T_I("version upgrade, run silent upgrade");
            new_blk = VTSS_PRIVILEGE_conf_flash_silent_upgrade(conf_blk_p, size);
            conf_blk_p = conf_sec_create(CONF_SEC_GLOBAL, blk_id, sizeof(*conf_blk_p));
            if (conf_blk_p && new_blk) {
                T_I("upgrade ok");
                *conf_blk_p = *new_blk;
            } else {
                T_W("upgrade failed, creating defaults");
                do_create = TRUE;
            }
            if (new_blk) {
                VTSS_FREE(new_blk);
            }
        }
    } else {
        conf_blk_p = NULL;
        do_create  = 1;
    }

    PRIVILEGE_CRIT_ENTER();
    if (do_create) {
        /* Use default values */
        VTSS_PRIVILEGE_default_get(&VTSS_PRIVILEGE_global.privilege_conf);
        if (conf_blk_p != NULL) {
            conf_blk_p->privilege_conf = VTSS_PRIVILEGE_global.privilege_conf;
        }
    } else {
        /* Use new configuration */
        if (conf_blk_p != NULL) {  // Quiet lint
            VTSS_PRIVILEGE_global.privilege_conf = conf_blk_p->privilege_conf;
        }
    }
    PRIVILEGE_CRIT_EXIT();

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (conf_blk_p == NULL) {
        T_W("failed to open privilege table");
    } else {
        conf_blk_p->version = PRIVILEGE_CONF_BLK_VERSION;
        conf_sec_close(CONF_SEC_GLOBAL, blk_id);
    }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */

    T_D("exit");
}

/* Module start */
static void VTSS_PRIVILEGE_start(void)
{
    vtss_priv_conf_t    *conf_p;

    T_D("enter");

    /* Initialize privilege configuration */
    conf_p = &VTSS_PRIVILEGE_global.privilege_conf;
    VTSS_PRIVILEGE_default_get(conf_p);

    /* Create semaphore for critical regions */
    critd_init(&VTSS_PRIVILEGE_global.crit, "VTSS_PRIVILEGE_global.crit", VTSS_MODULE_ID_PRIV_LVL, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
    PRIVILEGE_CRIT_EXIT();

    T_D("exit");
}

/**
  * \brief Initialize the Privilege Level module
  *
  * \param cmd [IN]: Reason why calling this function.
  * \param p1  [IN]: Parameter 1. Usage varies with cmd.
  * \param p2  [IN]: Parameter 2. Usage varies with cmd.
  *
  * \return
  *    VTSS_OK.
  */
vtss_rc vtss_priv_init(vtss_init_data_t *data)
{
#ifdef VTSS_SW_OPTION_ICFG
    vtss_rc rc = VTSS_OK;
#endif

    vtss_isid_t isid = data->isid;

    if (data->cmd == INIT_CMD_INIT) {
        /* Initialize and register trace ressources */
        VTSS_TRACE_REG_INIT(&VTSS_PRIVILEGE_trace_reg, VTSS_PRIVILEGE_trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&VTSS_PRIVILEGE_trace_reg);
    }

    T_D("enter, cmd: %d, isid: %u, flags: 0x%x", data->cmd, data->isid, data->flags);

    switch (data->cmd) {
    case INIT_CMD_INIT:
        T_D("INIT");

#ifdef VTSS_SW_OPTION_VCLI
        privilege_cli_init();
#endif
        VTSS_PRIVILEGE_start();
#ifdef VTSS_SW_OPTION_ICFG
        if ((rc = priv_icfg_init()) != VTSS_OK) {
            T_D("Calling priv_icfg_init() failed rc = %s", error_txt(rc));
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
            VTSS_PRIVILEGE_conf_read_stack(1);
        } else if (VTSS_ISID_LEGAL(isid)) {
            /* Reset switch configuration */
        }
        break;
    case INIT_CMD_MASTER_UP: {
        T_D("MASTER_UP");

        /* Read stack and switch configuration */
        VTSS_PRIVILEGE_conf_read_stack(0);
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

static void VTSS_PRIVILEGE_strn_tolower(char *str, int len)
{
    int i;
    for (i = 0; i < len; i++) {
        str[i] = tolower(str[i]);
    }
} /* str_tolower */

/**
  * \brief Get module ID by privilege level group name
  *
  * \param name         [IN]: The privilege level group name.
  * \param module_id_p  [OUT]: The module ID.
  *
  * \return
  *    TRUE if the module name is found.\n
  *    FALSE otherwise.\n
  */
BOOL vtss_privilege_module_to_val(const char *name, vtss_module_id_t *module_id_p)
{
    int i;
    char name_lc[VTSS_PRIV_LVL_NAME_LEN_MAX], module_name_lc[VTSS_PRIV_LVL_NAME_LEN_MAX];
    int  mid_match = -1;

    T_D("enter");

    /* Convert name to lowercase */
    strncpy(name_lc, name, VTSS_PRIV_LVL_NAME_LEN_MAX - 1);
    name_lc[VTSS_PRIV_LVL_NAME_LEN_MAX - 1] = 0;
    VTSS_PRIVILEGE_strn_tolower(name_lc, strlen(name_lc));

    for (i = 0; i < VTSS_MODULE_ID_NONE; i++) {
        if (vtss_priv_lvl_groups_filter[i]) {
            continue;
        }
        strncpy(module_name_lc, vtss_module_names[i], VTSS_PRIV_LVL_NAME_LEN_MAX - 1);
        module_name_lc[VTSS_PRIV_LVL_NAME_LEN_MAX - 1] = 0;
        VTSS_PRIVILEGE_strn_tolower(module_name_lc, strlen(module_name_lc));
        if (strncmp(name_lc, module_name_lc, strlen(name_lc)) == 0) {
            if (strlen(module_name_lc) == strlen(name_lc)) {
                /* Exact match found */
                mid_match = i;
                break;
            }
            if (mid_match == -1) {
                /* First match found */
                mid_match = i;
            } else {
                /* >1 match found */
                T_D("exit");
                return FALSE;
            }
        }
    }

    if (mid_match != -1) {
        *module_id_p = (vtss_module_id_t) mid_match;
        T_D("exit");
        return TRUE;
    }

    T_D("exit");
    return FALSE;
}

/**
  * \brief Get privilege group name
  *
  * \param name         [OUT]: The privilege level group name.
  * \param module_id_p  [OUT]: The module ID.
  * \param next         [IN]: is getnext operation.
  *
  * \return
  *    TRUE if the module name is found.\n
  *    FALSE otherwise.\n
  */
BOOL vtss_privilege_group_name_get(char *name, vtss_module_id_t *module_id_p, BOOL next)
{
    int i, found = 0;
    char upper_name[VTSS_PRIV_LVL_NAME_LEN_MAX] = "";
    char name_lc[VTSS_PRIV_LVL_NAME_LEN_MAX], module_name_lc[VTSS_PRIV_LVL_NAME_LEN_MAX];

    /* Convert name to lowercase */
    strncpy(name_lc, name, VTSS_PRIV_LVL_NAME_LEN_MAX - 1);
    name_lc[VTSS_PRIV_LVL_NAME_LEN_MAX - 1] = 0;
    VTSS_PRIVILEGE_strn_tolower(name_lc, strlen(name_lc));

    T_D("enter");
    for (i = 0; i < VTSS_MODULE_ID_NONE; i++) {
        if (vtss_priv_lvl_groups_filter[i]) {
            continue;
        }
        if (vtss_module_names[i] == NULL) {
            T_W("Cannot find the module name. module ID = %d", i);
            continue;
        }
        strncpy(module_name_lc, vtss_module_names[i], VTSS_PRIV_LVL_NAME_LEN_MAX - 1);
        module_name_lc[VTSS_PRIV_LVL_NAME_LEN_MAX - 1] = 0;
        VTSS_PRIVILEGE_strn_tolower(module_name_lc, strlen(module_name_lc));
        if (next) {
            if ((found && (strcmp(module_name_lc, name_lc) > 0 && strcmp(module_name_lc, upper_name) < 0)) ||
                (!found && strcmp(module_name_lc, name_lc) > 0)) {
                strcpy(upper_name, module_name_lc);
                *module_id_p = (vtss_module_id_t) i;
                if (!found && strcmp(module_name_lc, name_lc) > 0) {
                    found = 1;
                }
            }
        } else if (strcmp(module_name_lc, name_lc) == 0) {
            *module_id_p = (vtss_module_id_t) i;
            found = 1;
            break;
        }
    }

    if (found) {
        strcpy(name, vtss_module_names[*module_id_p]);
        T_D("exit");
        return TRUE;
    } else {
        T_D("exit");
        return FALSE;
    }
}

/**
  * \brief Get privilege group name list
  *
  * \param max_cnt  [IN]: The maximum count of privilege group name.
  * \param list_p   [OUT]: The point list of  privilege group name.
  *
  * \return
  *    TRUE if the module name is found.\n
  *    FALSE otherwise.\n
  */
void vtss_privilege_group_name_list_get(const u32 max_cnt, const char *list_p[])
{
    u32 i, n;

    // Insert unique entries into list. We do that by searching for existing
    // matching entries. That's expensive, of course, but it's a rare operation
    // so we live with it.

    T_D("enter");
    for (i = n = 0; i < VTSS_MODULE_ID_NONE  &&  n < max_cnt - 1; i++) {
        if (vtss_priv_lvl_groups_filter[i] || vtss_module_names[i] == NULL) {
            continue;
        }
        list_p[n++] = vtss_module_names[i];
    }

    if (i < VTSS_MODULE_ID_NONE  &&  n == max_cnt - 1) {
        T_E("web privilege group name list full; truncating. i = %d, n = %d", i, n);
    }
    list_p[n] = NULL;
}

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
