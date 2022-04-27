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

 $Id$
 $Revision$

*/

#include "main.h"
#include "conf_api.h"
#include "msg_api.h"
#include "critd_api.h"
#include "vtss_ecos_mutex_api.h"
#include "misc_api.h"
#include "sysutil_api.h"
#ifdef VTSS_SW_OPTION_LLDP
#include "lldp_api.h"
#endif /* VTSS_SW_OPTION_LLDP */
#include "sysutil.h"
#ifdef VTSS_SW_OPTION_AUTH
#include "vtss_auth_api.h"
#endif /* VTSS_SW_OPTION_AUTH */
#ifdef VTSS_SW_OPTION_VCLI
#include "cli.h"
void system_cli_req_init(void);
#endif

#ifdef VTSS_SW_OPTION_ICLI /* CP, 04/08/2013 12:54, Bugzilla#11469 */
#include "icli_api.h"
#endif
#if defined(VTSS_SW_OPTION_ICFG) && !defined(VTSS_SW_OPTION_USERS)
#include "sysutil_icfg.h"
#endif

/****************************************************************************/
/*  Global variables                                                        */
/****************************************************************************/

/* Global structure */
static system_global_t system_global;
static char tz_display[8];
static char system_descr[256];

#if (VTSS_TRACE_ENABLED)
/* Trace registration. Initialized by system_init() */
static vtss_trace_reg_t trace_reg = {
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "system",
    .descr     = "system (configuration)"
};

static vtss_trace_grp_t trace_grps[TRACE_GRP_CNT] = {
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

#define SYSTEM_CRIT_ENTER() critd_enter(&system_global.crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define SYSTEM_CRIT_EXIT()  critd_exit( &system_global.crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#else
#define SYSTEM_CRIT_ENTER() critd_enter(&system_global.crit)
#define SYSTEM_CRIT_EXIT()  critd_exit( &system_global.crit)
#endif /* VTSS_TRACE_ENABLED */


/****************************************************************************/
/*  Various local functions                                                 */
/****************************************************************************/

#ifndef VTSS_SW_OPTION_USERS
/* Determine if password has changed */
static int system_password_changed(system_conf_t *old, system_conf_t *new)
{
    return (strcmp(new->sys_passwd, old->sys_passwd));
}
#endif /* VTSS_SW_OPTION_USERS */

/* Determine if system configuration has changed */
static int system_conf_changed(system_conf_t *old, system_conf_t *new)
{
    return (strcmp(new->sys_contact, old->sys_contact) ||
            strcmp(new->sys_name, old->sys_name) ||
            strcmp(new->sys_location, old->sys_location) ||
#ifndef VTSS_SW_OPTION_USERS
            strcmp(new->sys_passwd, old->sys_passwd) ||
#endif /* VTSS_SW_OPTION_USERS */
            new->sys_services != old->sys_services ||
            new->tz_off != old->tz_off);
}

/* Set system defaults */
static void system_default_set(system_conf_t *conf)
{
    conf->sys_contact[0] = '\0'; /* Empty system contact by default */

#ifdef VTSS_SW_OPTION_ICLI /* CP, 04/08/2013 12:54, Bugzilla#11469 */
    strcpy(conf->sys_name, ICLI_DEFAULT_DEVICE_NAME);
#else
    conf->sys_name[0] = '\0'; /* Empty system name by default */
#endif

    conf->sys_location[0] = '\0'; /* Empty system location by default */
#ifndef VTSS_SW_OPTION_USERS
    conf->sys_passwd[0] = '\0'; /* Empty passwd by default */
#endif /* VTSS_SW_OPTION_USERS */
    conf->sys_services = SYSTEM_SERVICES_PHYSICAL | SYSTEM_SERVICES_DATALINK;
    conf->tz_off = 0x0;
}

static void system_update_tz_display(int tz_off)
{
    (void) snprintf(tz_display, sizeof(tz_display) - 1, "%c%02d%02d",
                    tz_off < 0 ? '-' : '+',
                    abs(tz_off) / 60,
                    abs(tz_off) % 60);
}

static void _system_set_config(system_conf_t *conf)
{
    system_update_tz_display(conf->tz_off);

#ifdef VTSS_SW_OPTION_VCLI
    cli_name_set(conf->sys_name);
#endif
}

int                             /* TZ offset in minutes */
system_get_tz_off(void)
{
    int  tz_off;

    /* This is is not protected by SYSTEM_CRIT_ENTER/EXIT, since this would create
       a deadlock for trace statements inside critical regions of this module */
    tz_off = system_global.system_conf.tz_off;

    return tz_off;
}

const char*                     /* TZ offset for display per ISO8601: +-hhmm */
system_get_tz_display(void)
{
    return tz_display;
}

char *system_get_descr(void)
{
    return system_descr;
}


/* check string is administratively name */
BOOL system_name_is_administratively(char string[VTSS_SYS_STRING_LEN])
{
    ulong i;
    size_t len = strlen(string);

    /* allow null string */
    if (string[0] == '\0') {
        return TRUE;
    }

    /* The first or last character must not be a minus sign */
    if (string[0] == '-' || string[len] == '-') {
        return FALSE;
    }

    /* The first character must be an alpha character */
    if (!((string[0] >= 'A' && string[0] <= 'Z') ||
            (string[0] >= 'a' && string[0] <= 'z'))) {
        return FALSE;
    }

    for (i = 0; i < len; i++) {
        /* No blank or space characters are permitted as part of a name */
        if (string[i] == '\0') {
            return FALSE;
        }

        /* A name is a text string drawn from the alphabet (A-Za-z),
           digits (0-9), minus sign (-) */
        if (!((string[i] >= '0' && string[i] <= '9') ||
                (string[i] >= 'A' && string[i] <= 'Z') ||
                (string[i] >= 'a' && string[i] <= 'z') ||
                (string[i] == '-'))) {
            return FALSE;
        }
    }

    return TRUE;
}

/****************************************************************************/
/*  Stack/switch functions                                                  */
/****************************************************************************/
#if (VTSS_TRACE_LVL_MIN <= VTSS_TRACE_LVL_DEBUG)
static char *system_msg_id_txt(system_msg_id_t msg_id)
{
    char *txt;

    switch (msg_id) {
    case SYSTEM_MSG_ID_CONF_SET_REQ:
        txt = "SYSTEM_CONF_SET_REQ";
        break;
    default:
        txt = "?";
        break;
    }
    return txt;
}
#endif /* VTSS_TRACE_LVL_DEBUG */

/* Allocate request buffer */
static system_msg_req_t *system_msg_req_alloc(system_msg_buf_t *buf, system_msg_id_t msg_id)
{
    system_msg_req_t *msg = &system_global.request.msg;

    buf->sem = &system_global.request.sem;
    buf->msg = msg;
    (void) VTSS_OS_SEM_WAIT(buf->sem);
    msg->msg_id = msg_id;
    return msg;
}

/* Free request/reply buffer */
static void system_msg_free(vtss_os_sem_t *sem)
{
    VTSS_OS_SEM_POST(sem);
}

static void system_msg_tx_done(void *contxt, void *msg, msg_tx_rc_t rc)
{
#if (VTSS_TRACE_LVL_MIN <= VTSS_TRACE_LVL_DEBUG)
    system_msg_id_t msg_id = *(system_msg_id_t *)msg;
#endif /* VTSS_TRACE_LVL_DEBUG */

    T_D("msg_id: %d, %s", msg_id, system_msg_id_txt(msg_id));
    system_msg_free(contxt);
}

static void system_msg_tx(system_msg_buf_t *buf, vtss_isid_t isid, size_t len)
{
#if (VTSS_TRACE_LVL_MIN <= VTSS_TRACE_LVL_DEBUG)
    system_msg_id_t msg_id = *(system_msg_id_t *)buf->msg;
#endif /* VTSS_TRACE_LVL_DEBUG */

    T_D("msg_id: %d, %s, len: %zd, isid: %d", msg_id, system_msg_id_txt(msg_id), len, isid);
    msg_tx_adv(buf->sem, system_msg_tx_done, MSG_TX_OPT_DONT_FREE,
               VTSS_MODULE_ID_SYSTEM, isid, buf->msg, len);
}

static BOOL system_msg_rx(void *contxt, const void *const rx_msg, const size_t len, const vtss_module_id_t modid, const ulong isid)
{
    system_msg_req_t  *msg;
    system_msg_id_t msg_id = *(system_msg_id_t *)rx_msg;

    T_D("msg_id: %d, %s, len: %zd, isid: %u", msg_id, system_msg_id_txt(msg_id), len, isid);

    msg = (system_msg_req_t *)rx_msg;

    switch (msg_id) {
    case SYSTEM_MSG_ID_CONF_SET_REQ: {
        _system_set_config(&msg->system_conf);
        break;
    }
    default:
        T_W("unknown message ID: %d", msg_id);
        break;
    }
    return TRUE;
}

static vtss_rc system_stack_register(void)
{
    msg_rx_filter_t filter;

    memset(&filter, 0, sizeof(filter));
    filter.cb = system_msg_rx;
    filter.modid = VTSS_MODULE_ID_SYSTEM;
    return msg_rx_filter_register(&filter);
}

/* Set stack system configuration */
static vtss_rc system_stack_conf_set(vtss_isid_t isid_add)
{
    system_msg_req_t  *msg;
    system_msg_buf_t  buf;
    vtss_isid_t     isid;

    T_D("enter, isid_add: %d", isid_add);
    for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
        if ((isid_add != VTSS_ISID_GLOBAL && isid_add != isid) ||
                !msg_switch_exists(isid)) {
            continue;
        }
        msg = system_msg_req_alloc(&buf, SYSTEM_MSG_ID_CONF_SET_REQ);
        SYSTEM_CRIT_ENTER();
        msg->system_conf = system_global.system_conf;
        SYSTEM_CRIT_EXIT();
        system_msg_tx(&buf, isid, sizeof(system_msg_req_t));
    }

    T_D("exit, isid_add: %d", isid_add);
    return VTSS_OK;
}

/****************************************************************************/
/*  Management functions                                                    */
/****************************************************************************/

/* system error text */
char *system_error_txt(vtss_rc rc)
{
    char *txt;

    switch (rc) {
    case SYSTEM_ERROR_GEN:
        txt = "system generic error";
        break;
    case SYSTEM_ERROR_PARM:
        txt = "system parameter error";
        break;
    case SYSTEM_ERROR_STACK_STATE:
        txt = "system stack state error";
        break;
    default:
        txt = "system unknown error";
        break;
    }
    return txt;
}

/* Get system configuration */
vtss_rc system_get_config(system_conf_t *conf)
{
    T_D("enter");
    SYSTEM_CRIT_ENTER();
    *conf = system_global.system_conf;
    SYSTEM_CRIT_EXIT();
    T_D("exit");

    return VTSS_OK;
}

/* Set system configuration */
vtss_rc system_set_config(system_conf_t *conf)
{
    vtss_rc rc      = VTSS_OK;
    int     changed = 0;
#ifndef VTSS_SW_OPTION_USERS
    int     passwd_changed = 0;
#endif /* VTSS_SW_OPTION_USERS */

    T_D("enter");

    SYSTEM_CRIT_ENTER();
    if (msg_switch_is_master()) {
        if (system_name_is_administratively(conf->sys_name)) {
            changed = system_conf_changed(&system_global.system_conf, conf);
#ifndef VTSS_SW_OPTION_USERS
            passwd_changed = system_password_changed(&system_global.system_conf, conf);
#endif /* VTSS_SW_OPTION_USERS */
            system_global.system_conf = *conf;
        } else {
            rc = SYSTEM_ERROR_NOT_ADMINISTRATIVELY_NAME;
        }
    } else {
        T_W("not master");
        rc = SYSTEM_ERROR_STACK_STATE;
    }
    SYSTEM_CRIT_EXIT();

    if (changed) {
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
        /* Save changed configuration */
        conf_blk_id_t     blk_id  = CONF_BLK_SYSTEM;
        system_conf_blk_t *system_blk;
        if ((system_blk = conf_sec_open(CONF_SEC_GLOBAL, blk_id, NULL)) == NULL) {
            T_E("failed to open system table");
        } else {
            system_blk->conf = *conf;
            conf_sec_close(CONF_SEC_GLOBAL, blk_id);
        }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */

        /* Activate changed configuration */
        rc = system_stack_conf_set(VTSS_ISID_GLOBAL);
#ifdef VTSS_SW_OPTION_LLDP
        lldp_something_has_changed();
#endif /* VTSS_SW_OPTION_LLDP */
    }
#if defined(VTSS_SW_OPTION_AUTH) && !defined(VTSS_SW_OPTION_USERS)
    if (passwd_changed) {
        vtss_auth_mgmt_httpd_cache_expire(); // Clear httpd cache
    }
#endif /* VTSS_SW_OPTION_AUTH */
    T_D("exit");

#ifdef VTSS_SW_OPTION_ICLI /* CP, 04/08/2013 12:54, Bugzilla#11469 */
    if ( rc == VTSS_OK ) {
        if ( icli_dev_name_set(conf->sys_name) != ICLI_RC_OK ) {
            T_E("%% Fail to set device name to ICLI engine.\n\n");
        }
    }
#endif

    return rc;
}

#ifndef VTSS_SW_OPTION_USERS
/* Get system passwd */
const char *system_get_passwd()
{
    return system_global.system_conf.sys_passwd;
}

/* Set system passwd */
vtss_rc system_set_passwd(char *pass)
{
    system_conf_t conf;

    if (strlen(pass) >= VTSS_SYS_PASSWD_LEN) {
        return VTSS_INVALID_PARAMETER;
    }

    if (system_get_config(&conf) == VTSS_OK && strcmp(pass, conf.sys_passwd) != 0) {
        SYSTEM_CRIT_ENTER();
        misc_strncpyz(conf.sys_passwd, pass, sizeof(conf.sys_passwd));
        SYSTEM_CRIT_EXIT();
        return system_set_config(&conf);
    }

    return VTSS_OK;             /* Unchanged */
}
#endif /* VTSS_SW_OPTION_USERS */

/****************************************************************************/
/*  Initialization functions                                                */
/****************************************************************************/

/* Read/create system switch configuration */
static vtss_rc system_conf_read_switch(vtss_isid_t isid_add)
{
    int               changed;
    BOOL              do_create;
    ulong             size;
    vtss_isid_t       isid;
    system_conf_t     *conf_p, new_conf;
    system_conf_blk_t *conf_blk_p;
    conf_blk_id_t     blk_id      = CONF_BLK_SYSTEM;
    ulong             blk_version = SYSTEM_CONF_BLK_VERSION;
    vtss_rc           rc = VTSS_OK;

    T_D("enter, isid_add: %d", isid_add);

    if (misc_conf_read_use()) {
        /* read configuration */
        if ((conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, &size)) == NULL ||
                size != sizeof(*conf_blk_p)) {
            T_W("conf_sec_open failed or size mismatch, creating defaults");
            conf_blk_p = conf_sec_create(CONF_SEC_GLOBAL, blk_id, sizeof(*conf_blk_p));
            do_create = 1;
        } else if (conf_blk_p->version != blk_version) {
            T_W("version mismatch, creating defaults");
            do_create = 1;
        } else {
            do_create = (isid_add != VTSS_ISID_GLOBAL);
        }
    } else {
        conf_blk_p = NULL;
        do_create  = TRUE;
    }

    for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
        if (isid_add != VTSS_ISID_GLOBAL && isid_add != isid) {
            continue;
        }

        changed = 0;
        SYSTEM_CRIT_ENTER();
        if (do_create) {
            /* Use default values */
            system_default_set(&new_conf);
            if (conf_blk_p != NULL) {
                conf_blk_p->conf = new_conf;
            }
        } else {
            /* Use new configuration */
            if (conf_blk_p != NULL) {  // Quiet lint
                new_conf = conf_blk_p->conf;
            }
        }
        conf_p = &system_global.system_conf;
        if (system_conf_changed(conf_p, &new_conf)) {
            changed = 1;
        }
        *conf_p = new_conf;
        SYSTEM_CRIT_EXIT();
        if (changed && isid_add != VTSS_ISID_GLOBAL && msg_switch_exists(isid)) {
            rc = system_stack_conf_set(isid);
        }
    }

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (conf_blk_p == NULL) {
        T_W("failed to open system table");
    } else {
        conf_blk_p->version = blk_version;
        conf_sec_close(CONF_SEC_GLOBAL, blk_id);
    }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */

    T_D("exit");
    return rc;
}

/* Read/create system stack configuration */
static vtss_rc system_conf_read_stack(BOOL create)
{
    int               changed;
    BOOL              do_create;
    ulong             size;
    system_conf_t     *old_conf_p, new_conf;
    system_conf_blk_t *conf_blk_p;
    conf_blk_id_t     blk_id      = CONF_BLK_SYSTEM;
    ulong             blk_version = SYSTEM_CONF_BLK_VERSION;
    vtss_rc           rc = VTSS_OK;

    T_D("enter, create: %d", create);

    if (misc_conf_read_use()) {
        /* Read/create configuration */
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
        do_create  = TRUE;
    }

    changed = 0;
    SYSTEM_CRIT_ENTER();
    if (do_create) {
        /* Use default values */
        system_default_set(&new_conf);
        if (conf_blk_p != NULL) {
            conf_blk_p->conf = new_conf;
        }
    } else {
        /* Use new configuration */
        if (conf_blk_p != NULL) {  // Quiet lint
            new_conf = conf_blk_p->conf;
        }
    }
    old_conf_p = &system_global.system_conf;
    if (system_conf_changed(old_conf_p, &new_conf)) {
        changed = 1;
    }
    system_global.system_conf = new_conf;
    SYSTEM_CRIT_EXIT();

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (conf_blk_p == NULL) {
        T_W("failed to open system table");
    } else {
        conf_blk_p->version = blk_version;
        conf_sec_close(CONF_SEC_GLOBAL, blk_id);
    }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */

    if (changed && create) {
        rc = system_stack_conf_set(VTSS_ISID_GLOBAL);
    }

    T_D("exit");

    return rc;
}

/* Module start */
static void system_start(BOOL init)
{
    system_conf_t       *conf_p;

    T_D("enter, init: %d", init);

    if (init) {
        /* Initialize system configuration */
        conf_p = &system_global.system_conf;
        system_default_set(conf_p);

        /* Initialize message buffers */
        VTSS_OS_SEM_CREATE(&system_global.request.sem, 1);

        /* Create semaphore for critical regions */
        critd_init(&system_global.crit, "system_global.crit", VTSS_MODULE_ID_SYSTEM, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
        SYSTEM_CRIT_EXIT();

        /* place any other initialization junk you need here,
           Initialize system description */
        if (vtss_switch_stackable()) {
            (void) snprintf(system_descr, sizeof(system_descr), "%s Stackable GigaBit Ethernet Switch", VTSS_PRODUCT_NAME);
        } else {
            (void) snprintf(system_descr, sizeof(system_descr), "%s GigaBit Ethernet Switch", VTSS_PRODUCT_NAME);
        }

    } else {
        /* Register for stack messages */
        (void) system_stack_register();
    }
    T_D("exit");
}

/* Initialize module */
vtss_rc system_init(vtss_init_data_t *data)
{
    vtss_rc     rc = VTSS_OK;
    vtss_isid_t isid = data->isid;

    if (data->cmd == INIT_CMD_INIT) {
        /* Initialize and register trace ressources */
        VTSS_TRACE_REG_INIT(&trace_reg, trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&trace_reg);
    }


    T_D("enter, cmd: %d, isid: %u, flags: 0x%x", data->cmd, data->isid, data->flags);

    switch (data->cmd) {
    case INIT_CMD_INIT:
        T_D("INIT");
        system_start(1);
#ifdef VTSS_SW_OPTION_VCLI
        system_cli_req_init();
#endif
#if defined(VTSS_SW_OPTION_ICFG) && !defined(VTSS_SW_OPTION_USERS)
        if ((rc = sysutil_icfg_init()) != VTSS_OK) {
            T_D("fail to init icfg registration, rc = %s", error_txt(rc));
        }
#endif
        break;
    case INIT_CMD_START:
        T_D("START");
        system_start(0);
        break;
    case INIT_CMD_CONF_DEF:
        T_D("CONF_DEF, isid: %d", isid);
        if (isid == VTSS_ISID_LOCAL) {
            /* Reset local configuration */
        } else if (isid == VTSS_ISID_GLOBAL) {
            /* Reset stack configuration */
            rc = system_conf_read_stack(1);
        } else if (VTSS_ISID_LEGAL(isid)) {
            /* Reset switch configuration */
            rc = system_conf_read_switch(isid);
        }
        break;
    case INIT_CMD_MASTER_UP:
        T_D("MASTER_UP");
        /* Read stack and switch configuration */
        if ((rc = system_conf_read_stack(0)) != VTSS_OK) {
            return rc;
        }
        rc = system_conf_read_switch(VTSS_ISID_GLOBAL);
        break;
    case INIT_CMD_MASTER_DOWN:
        T_D("MASTER_DOWN");
        break;
    case INIT_CMD_SWITCH_ADD:
        T_D("SWITCH_ADD, isid: %d", isid);
        /* Apply all configuration to switch */
        rc = system_stack_conf_set(isid);
        break;
    case INIT_CMD_SWITCH_DEL:
        T_D("SWITCH_DEL, isid: %d", isid);
        break;
    default:
        break;
    }

    T_D("exit");

    return rc;
}


/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/

