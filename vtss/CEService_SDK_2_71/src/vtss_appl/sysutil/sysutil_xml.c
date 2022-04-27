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
#include "sysutil_api.h"

#include "conf_xml_api.h"
#include "conf_xml_trace_def.h"
#ifdef VTSS_SW_OPTION_USERS
#include "vtss_users_api.h"
#endif /* VTSS_SW_OPTION_USERS */

#ifdef VTSS_SW_OPTION_DAYLIGHT_SAVING
#include "daylight_saving_api.h"
#endif /* VTSS_SW_OPTION_DAYLIGHT_SAVING */

/* Tag IDs */
enum {
    /* Module tags */
    CX_TAG_SYSTEM,

    /* Parameter tags */
    CX_TAG_NAME,
    CX_TAG_CONTACT,
    CX_TAG_LOCATION,
    CX_TAG_PASSWORD,
    CX_TAG_TIMEZONE,

    /* Last entry */
    CX_TAG_NONE
};

/* Tag table */
static cx_tag_entry_t system_cx_tag_table[CX_TAG_NONE + 1] = {
    [CX_TAG_SYSTEM] = {
        .name  = "system",
        .descr = "System settings",
        .type = CX_TAG_TYPE_MODULE
    },
    [CX_TAG_NAME] = {
        .name  = "name",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_CONTACT] = {
        .name  = "contact",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_LOCATION] = {
        .name  = "location",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_PASSWORD] = {
        .name  = "password",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_TIMEZONE] = {
        .name  = "timezone",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },

    /* Last entry */
    [CX_TAG_NONE] = {
        .name  = "",
        .descr = "",
        .type = CX_TAG_TYPE_NONE
    }
};

static vtss_rc system_cx_parse_func(cx_set_state_t *s)
{
    switch (s->cmd) {
    case CX_PARSE_CMD_PARM: {
        system_conf_t conf;
        long          tz;
        BOOL          global;
#ifdef VTSS_SW_OPTION_USERS
        users_conf_t  users_conf;

        strcpy(users_conf.username, VTSS_SYS_ADMIN_NAME);
#endif /* VTSS_SW_OPTION_USERS */

        global = (s->isid == VTSS_ISID_GLOBAL);

        if (!global) {
            s->ignored = 1;
            break;
        }

        if (s->apply && system_get_config(&conf) != VTSS_OK) {
            break;
        }

#ifdef VTSS_SW_OPTION_USERS
        if (s->apply && vtss_users_mgmt_conf_get(&users_conf, 0) != VTSS_OK) {
            break;
        }
#endif /* VTSS_SW_OPTION_USERS */

        switch (s->id) {
        case CX_TAG_NAME:
        case CX_TAG_CONTACT:
        case CX_TAG_LOCATION:
        case CX_TAG_PASSWORD:
        {
            char buf[VTSS_SYS_STRING_LEN];
            if (cx_parse_val_txt(s, buf,
                                 s->id == CX_TAG_PASSWORD ? VTSS_SYS_PASSWD_LEN :
                                 VTSS_SYS_STRING_LEN) == VTSS_OK) {
                if (s->id == CX_TAG_NAME && !system_name_is_administratively(buf)) {
                    CX_RC(cx_parm_invalid(s));
                }
#ifdef VTSS_SW_OPTION_USERS
                strcpy(s->id == CX_TAG_NAME ? conf.sys_name :
                       s->id == CX_TAG_CONTACT ? conf.sys_contact :
                       s->id == CX_TAG_LOCATION ? conf.sys_location : users_conf.password, buf);
#else
                strcpy(s->id == CX_TAG_NAME ? conf.sys_name :
                       s->id == CX_TAG_CONTACT ? conf.sys_contact :
                       s->id == CX_TAG_LOCATION ? conf.sys_location : conf.sys_passwd, buf);
#endif /* VTSS_SW_OPTION_USERS */
            }
            break;
        }
#ifdef VTSS_SW_OPTION_DAYLIGHT_SAVING
        case CX_TAG_TIMEZONE:
            if (cx_parse_val_long(s, &tz, -720, 720) == VTSS_OK) {
                (void)time_dst_update_tz_offset(tz);
            }
            break;
#else
        case CX_TAG_TIMEZONE:
            if (cx_parse_val_long(s, &tz, -720, 720) == VTSS_OK) {
                conf.tz_off = tz;
            }
            break;
#endif
        default:
            s->ignored = 1;
            break;
        }
        if (s->apply) {
            CX_RC(system_set_config(&conf));
#ifdef VTSS_SW_OPTION_USERS
            CX_RC(vtss_users_mgmt_conf_set(&users_conf));
#endif /* VTSS_SW_OPTION_USERS */
        }
        break;
    } /* CX_PARSE_CMD_PARM */
    case CX_PARSE_CMD_GLOBAL:
        break;
    case CX_PARSE_CMD_SWITCH:
        break;
    default:
        break;
    }

    return s->rc;
}

static vtss_rc system_cx_gen_func(cx_get_state_t *s)
{
    char           buf[128];
#ifdef VTSS_SW_OPTION_USERS
    users_conf_t   users_conf;
#endif /* VTSS_SW_OPTION_USERS */

    switch (s->cmd) {
    case CX_GEN_CMD_GLOBAL:
        /* Global - System */
        T_D("global - system");
        CX_RC(cx_add_tag_line(s, CX_TAG_SYSTEM, 0));
        {
            system_conf_t conf;

            if (system_get_config(&conf) == VTSS_OK) {
                sprintf(buf, "0-%u alphanumeric characters or '-'",
                        VTSS_SYS_STRING_LEN - 1);
                CX_RC(cx_add_val_txt(s, CX_TAG_NAME, conf.sys_name, buf));
                sprintf(buf, "0-%u characters", VTSS_SYS_STRING_LEN - 1);
                CX_RC(cx_add_val_txt(s, CX_TAG_CONTACT, conf.sys_contact, buf));
                CX_RC(cx_add_val_txt(s, CX_TAG_LOCATION, conf.sys_location, buf));
                sprintf(buf, "0-%u characters", VTSS_SYS_PASSWD_LEN - 1);
#ifdef VTSS_SW_OPTION_USERS
                strcpy(users_conf.username, VTSS_SYS_ADMIN_NAME);
                if (vtss_users_mgmt_conf_get(&users_conf, 0) == VTSS_OK) {
                    CX_RC(cx_add_val_txt(s, CX_TAG_PASSWORD, users_conf.password,
                               "0-31 characters"));
                }
#else
                CX_RC(cx_add_val_txt(s, CX_TAG_PASSWORD, conf.sys_passwd,
                               "0-31 characters"));
#endif /* VTSS_SW_OPTION_USERS */
#ifndef VTSS_SW_OPTION_DAYLIGHT_SAVING
                /* for new version, we move the timezone value to daylight saving module */
                /* if the system has daylight saving module, we don't output the timezone value on xml file */
                CX_RC(cx_add_val_long(s, CX_TAG_TIMEZONE, conf.tz_off, "-720-720"));
#endif
            }
        }
        CX_RC(cx_add_tag_line(s, CX_TAG_SYSTEM, 1));
        break;
    case CX_GEN_CMD_SWITCH:
        break;
    default:
        T_E("Unknown command");
        return VTSS_RC_ERROR;
    } /* End of switch */

    return VTSS_OK;
}

/* Register the info in to the cx_module_table */
CX_MODULE_TAB_ENTRY(
    VTSS_MODULE_ID_SYSTEM,
    system_cx_tag_table,
    0,
    0,
    NULL,                  /* init function */
    system_cx_gen_func,    /* Generation fucntion */
    system_cx_parse_func   /* parse fucntion */
);

