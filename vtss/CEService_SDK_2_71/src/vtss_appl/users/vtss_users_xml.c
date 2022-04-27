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
#include "vtss_users_api.h"

#include "conf_xml_api.h"
#include "conf_xml_trace_def.h"

/* Tag IDs */
enum {
    /* Module tags */
    CX_TAG_USERS,

    /* Group tags */
    CX_TAG_USER_TABLE,

    /* Parameter tags */
    CX_TAG_ENTRY,

    /* Last entry */
    CX_TAG_NONE
};

#define   USERS_BUFF_SIZE   256
/* Tag table */
static cx_tag_entry_t users_cx_tag_table[CX_TAG_NONE + 1] = {
    [CX_TAG_USERS] = {
        .name  = "users",
        .descr = "Users",
        .type = CX_TAG_TYPE_MODULE
    },
    [CX_TAG_USER_TABLE] = {
        .name  = "user_table",
        .descr = "",
        .type = CX_TAG_TYPE_GROUP
    },
    [CX_TAG_ENTRY] = {
        .name  = "entry",
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

/* Users specific set state structure */
typedef struct {
    int users_count;
} users_cx_set_state_t;

/* Parse username string */
static vtss_rc cx_parse_username(cx_set_state_t *s, char *name, char *username)
{
    uint idx;

    CX_RC(cx_parse_txt(s, name, username, VTSS_SYS_USERNAME_LEN));

    for (idx = 0; idx < strlen(username); idx++) {
        if (username[idx] < 33 || username[idx] > 126) {
            CX_RC(cx_parm_invalid(s));
            break;
        }
    }

    return s->rc;
}

static vtss_rc users_cx_parse_func(cx_set_state_t *s)
{
    switch (s->cmd) {
    case CX_PARSE_CMD_PARM: {
        ulong          val;
        BOOL           global;
        users_cx_set_state_t *users_state = s->mod_state;

        global = (s->isid == VTSS_ISID_GLOBAL);
        if (!global) {
            s->ignored = 1;
            break;
        }

        switch (s->id) {
        case CX_TAG_USER_TABLE:
            /* Flush users table */
            users_state->users_count = 0;
            CX_RC(vtss_users_mgmt_conf_clear());
            break;
        case CX_TAG_ENTRY:
            if (s->group == CX_TAG_USER_TABLE) {
                users_conf_t    conf;
                BOOL            username = 0, password = 0;
#ifdef VTSS_SW_OPTION_PRIV_LVL
                BOOL            priv_lvl = 0;
#endif

                memset(&conf, 0x0, sizeof(conf));
                for (; s->rc == VTSS_OK && cx_parse_attr(s) == VTSS_OK; s->p = s->next) {
                    if (cx_parse_username(s, "username", conf.username) == VTSS_OK &&
                        vtss_users_mgmt_is_valid_username(conf.username)) {
                        username = 1;
                    } else if (cx_parse_txt(s, "password", conf.password, VTSS_SYS_PASSWD_LEN) == VTSS_OK) {
                        password = 1;
#ifdef VTSS_SW_OPTION_PRIV_LVL
                    } else if (cx_parse_ulong(s, "priv_lvl", &val, 1, VTSS_USERS_MAX_PRIV_LEVEL) == VTSS_OK) {
                        priv_lvl = 1;
                        conf.privilege_level = val;
                    }
#endif
                    else {
                        CX_RC(cx_parm_unknown(s));
                    }
                }
                if (username == 0) {
                    CX_RC(cx_parm_found_error(s, "username"));
                }
                if (password == 0) {
                    CX_RC(cx_parm_found_error(s, "password"));
                }
#ifdef VTSS_SW_OPTION_PRIV_LVL
                if (priv_lvl == 0) {
                    CX_RC(cx_parm_found_error(s, "priv_lvl"));
                }
#endif
                users_state->users_count++;
                if (users_state->users_count > VTSS_USERS_NUMBER_OF_USERS - 1) { //Don't count username is "admin"
                    sprintf(s->msg, "The maximum users number is %d", VTSS_USERS_NUMBER_OF_USERS);
                    s->rc = CONF_XML_ERROR_FILE_PARM;
                    break;
                }
                if (s->apply) {
                    conf.valid = 1;
                    CX_RC(vtss_users_mgmt_conf_set(&conf));
                }
            } else {
                s->ignored = 1;
            }
            break;
        default:
            s->ignored = 1;
            break;
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

static vtss_rc users_cx_gen_func(cx_get_state_t *s)
{
    switch (s->cmd) {
    case CX_GEN_CMD_GLOBAL:
        /* Global - users */
        T_D("global - users");
        CX_RC(cx_add_tag_line(s, CX_TAG_USERS, 0));
        CX_RC(cx_add_tag_line(s, CX_TAG_USER_TABLE, 0));
        {
            users_conf_t   conf;
            char           buf[USERS_BUFF_SIZE];

            /* Entry syntax */
            CX_RC(cx_add_stx_start(s));
            sprintf(buf, "1-%u characters", VTSS_SYS_USERNAME_LEN - 1);
            CX_RC(cx_add_attr_txt(s, "username", buf));
            sprintf(buf, "0-%u characters", VTSS_SYS_PASSWD_LEN - 1);
            CX_RC(cx_add_attr_txt(s, "password", buf));
            CX_RC(cx_add_stx_ulong(s, "priv_lvl", 1, VTSS_USERS_MAX_PRIV_LEVEL));
            CX_RC(cx_add_stx_end(s));

            memset(&conf, 0x0, sizeof(conf));
            while (vtss_users_mgmt_conf_get(&conf, 1) == VTSS_OK) {
                if (!strcmp(conf.username, VTSS_SYS_ADMIN_NAME)) {
                    continue;
                }
                CX_RC(cx_add_attr_start(s, CX_TAG_ENTRY));
                CX_RC(cx_add_attr_txt(s, "username", conf.username));
                CX_RC(cx_add_attr_txt(s, "password", conf.password));
#ifdef VTSS_SW_OPTION_PRIV_LVL
                CX_RC(cx_add_attr_ulong(s, "priv_lvl", conf.privilege_level));
#endif
                CX_RC(cx_add_attr_end(s, CX_TAG_ENTRY));
            }
        }
        CX_RC(cx_add_tag_line(s, CX_TAG_USER_TABLE, 1));
        CX_RC(cx_add_tag_line(s, CX_TAG_USERS, 1));
        break;
    case CX_GEN_CMD_SWITCH:
        break;
    default:
        T_E("Unknown command");
        return VTSS_RC_ERROR;
    } /* End of Switch */

    return VTSS_OK;
}

/* Register the info in to the cx_module_table */
CX_MODULE_TAB_ENTRY(VTSS_MODULE_ID_USERS, users_cx_tag_table,
                    sizeof(users_cx_set_state_t), 0,
                    NULL,                    /* init function       */
                    users_cx_gen_func,       /* Generation fucntion */
                    users_cx_parse_func);    /* parse fucntion      */
