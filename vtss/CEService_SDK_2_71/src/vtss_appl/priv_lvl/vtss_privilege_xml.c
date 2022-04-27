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
#include "vtss_privilege_api.h"
#include "vtss_users_api.h"

#include "conf_xml_api.h"
#include "conf_xml_trace_def.h"

/* Tag IDs */
enum {
    /* Module tags */
    CX_TAG_PRIV_LVL,

    /* Group tags */
    CX_TAG_GROUP_TABLE,

    /* Parameter tags */
    CX_TAG_ENTRY,

    /* Last entry */
    CX_TAG_NONE
};

/* Tag table */
static cx_tag_entry_t priv_lvl_cx_tag_table[CX_TAG_NONE + 1] = {
    [CX_TAG_PRIV_LVL] = {
        .name  = "priv_lvl",
        .descr = "Privilege Level",
        .type = CX_TAG_TYPE_MODULE
    },
    [CX_TAG_GROUP_TABLE] = {
        .name  = "group_table",
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

static vtss_rc priv_lvl_cx_parse_func(cx_set_state_t *s)
{
    switch (s->cmd) {
    case CX_PARSE_CMD_PARM: {
        BOOL           global;
        vtss_priv_conf_t conf;

        global = (s->isid == VTSS_ISID_GLOBAL);
        if (!global) {
            s->ignored = 1;
            break;
        }

        switch (s->id) {
        case CX_TAG_GROUP_TABLE:
            break;
        case CX_TAG_ENTRY:
            if (s->group == CX_TAG_GROUP_TABLE) {
                vtss_module_id_t    module_id = 0;
                ulong               priv_lvl = 0;
                char                buf[VTSS_PRIV_LVL_NAME_LEN_MAX];
                BOOL                group_name = 0, cro = 0, crw = 0, sro = 0, srw = 0;

                memset(&conf, 0, sizeof(conf));
                CX_RC(s->apply && vtss_priv_mgmt_conf_get(&conf));

                for (; s->rc == VTSS_OK && cx_parse_attr(s) == VTSS_OK; s->p = s->next) {
                    if (cx_parse_txt(s, "group_name", buf, VTSS_PRIV_LVL_NAME_LEN_MAX) == VTSS_OK) {
                        if (vtss_privilege_module_to_val(buf, &module_id)) {
                            group_name = 1;
                        } else {
                            break;
                        }
                    } else if (group_name && cx_parse_ulong(s, "cro", &priv_lvl, 1, VTSS_USERS_MAX_PRIV_LEVEL) == VTSS_OK) {
                        cro = 1;
                        conf.privilege_level[module_id].cro = (int)priv_lvl;
                    } else if (group_name && cx_parse_ulong(s, "crw", &priv_lvl, 1, VTSS_USERS_MAX_PRIV_LEVEL) == VTSS_OK) {
                        crw = 1;
                        conf.privilege_level[module_id].crw = (int)priv_lvl;
                    } else if (group_name && cx_parse_ulong(s, "sro", &priv_lvl, 1, VTSS_USERS_MAX_PRIV_LEVEL) == VTSS_OK) {
                        sro = 1;
                        conf.privilege_level[module_id].sro = (int)priv_lvl;
                    } else if (group_name && cx_parse_ulong(s, "srw", &priv_lvl, 1, VTSS_USERS_MAX_PRIV_LEVEL) == VTSS_OK) {
                        srw = 1;
                        conf.privilege_level[module_id].srw = (int)priv_lvl;
                    } else {
                        CX_RC(cx_parm_unknown(s));
                    }
                }
                if (group_name == 0) {
                    CX_RC(cx_parm_found_error(s, "group_name"));
                }
                if (cro == 0) {
                    CX_RC(cx_parm_found_error(s, "cro"));
                }
                if (crw == 0) {
                    CX_RC(cx_parm_found_error(s, "crw"));
                }
                if (sro == 0) {
                    CX_RC(cx_parm_found_error(s, "sro"));
                }
                if (srw == 0) {
                    CX_RC(cx_parm_found_error(s, "srw"));
                }
                if (conf.privilege_level[module_id].cro > conf.privilege_level[module_id].crw ||
                    conf.privilege_level[module_id].sro > conf.privilege_level[module_id].srw) {
                    sprintf(s->msg, "The privilege level of 'Read-only' should be less than or equal to 'Read-write'\n");
                    s->rc = CONF_XML_ERROR_FILE_PARM;
                }
                if (conf.privilege_level[module_id].crw < conf.privilege_level[module_id].sro) {
                    sprintf(s->msg, "The privilege level of 'Configuration/Execute Read-write' should be greater than or equal to 'Status/Statistics Read-only'\n");
                    s->rc = CONF_XML_ERROR_FILE_PARM;
                }
            } else {
                s->ignored = 1;
            }
            break;
        default:
            s->ignored = 1;
            break;
        }
        if (s->apply) {
            CX_RC(vtss_priv_mgmt_conf_set(&conf));
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

static vtss_rc priv_lvl_cx_gen_func(cx_get_state_t *s)
{
    switch (s->cmd) {
    case CX_GEN_CMD_GLOBAL:
        /* Global - priv_lvl */
        T_D("global - priv_lvl");
        CX_RC(cx_add_tag_line(s, CX_TAG_PRIV_LVL, 0));
        CX_RC(cx_add_tag_line(s, CX_TAG_GROUP_TABLE, 0));
        {
            vtss_priv_conf_t    conf;
            char                priv_group_name[VTSS_PRIV_LVL_NAME_LEN_MAX] = "";
            vtss_module_id_t    module_id;

            /* Entry syntax */
            CX_RC(cx_add_stx_start(s));
            CX_RC(cx_add_attr_txt(s, "group_name", "group_name"));
            CX_RC(cx_add_stx_ulong(s, "cro", 1, VTSS_USERS_MAX_PRIV_LEVEL));
            CX_RC(cx_add_stx_ulong(s, "crw", 1, VTSS_USERS_MAX_PRIV_LEVEL));
            CX_RC(cx_add_stx_ulong(s, "sro", 1, VTSS_USERS_MAX_PRIV_LEVEL));
            CX_RC(cx_add_stx_ulong(s, "srw", 1, VTSS_USERS_MAX_PRIV_LEVEL));
            CX_RC(cx_add_stx_end(s));

            CX_RC(vtss_priv_mgmt_conf_get(&conf));
            while (vtss_privilege_group_name_get(priv_group_name, &module_id, TRUE)) {
                CX_RC(cx_add_attr_start(s, CX_TAG_ENTRY));
                CX_RC(cx_add_attr_txt(s, "group_name", vtss_module_names[module_id]));
                CX_RC(cx_add_attr_ulong(s, "cro", conf.privilege_level[module_id].cro));
                CX_RC(cx_add_attr_ulong(s, "crw", conf.privilege_level[module_id].crw));
                CX_RC(cx_add_attr_ulong(s, "sro", conf.privilege_level[module_id].sro));
                CX_RC(cx_add_attr_ulong(s, "srw", conf.privilege_level[module_id].srw));
                CX_RC(cx_add_attr_end(s, CX_TAG_ENTRY));
            };
        }
        CX_RC(cx_add_tag_line(s, CX_TAG_GROUP_TABLE, 1));
        CX_RC(cx_add_tag_line(s, CX_TAG_PRIV_LVL, 1));
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
CX_MODULE_TAB_ENTRY(VTSS_MODULE_ID_PRIV_LVL, priv_lvl_cx_tag_table,
                    0, 0,
                    NULL,                       /* init function       */
                    priv_lvl_cx_gen_func,       /* Generation fucntion */
                    priv_lvl_cx_parse_func);    /* parse fucntion      */
