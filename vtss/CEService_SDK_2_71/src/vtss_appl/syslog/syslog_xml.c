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
#include "syslog_api.h"
#include "misc_api.h"

#include "conf_xml_api.h"
#include "conf_xml_trace_def.h"

/* Tag IDs */
enum {
    /* Module tags */
    CX_TAG_SYSLOG,

    /* Parameter tags */
    CX_TAG_SERVER_MODE,
    CX_TAG_SERVER_ADDR,
    CX_TAG_SYSLOG_LEVEL,

    /* Last entry */
    CX_TAG_NONE
};

/* Tag table */
static cx_tag_entry_t syslog_cx_tag_table[CX_TAG_NONE + 1] = {
    [CX_TAG_SYSLOG] = {
        .name  = "syslog",
        .descr = "Sytem Log",
        .type = CX_TAG_TYPE_MODULE
    },
    [CX_TAG_SERVER_MODE] = {
        .name  = "server_mode",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_SERVER_ADDR] = {
        .name  = "server_addr",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_SYSLOG_LEVEL] = {
        .name  = "syslog_level",
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

/* Keyword for syslog level */
static const cx_kw_t cx_kw_syslog_level[] = {
    { "info", SYSLOG_LVL_INFO },
    { "warning", SYSLOG_LVL_WARNING },
    { "error", SYSLOG_LVL_ERROR },
    { NULL,       0 }
};

static vtss_rc SL_cx_parse_func(cx_set_state_t *s)
{
    switch (s->cmd) {
    case CX_PARSE_CMD_PARM: {
        BOOL            global;
        BOOL            mode;
        syslog_conf_t   conf;
        char            *p = NULL;
        ulong           val;

        global = (s->isid == VTSS_ISID_GLOBAL);
        if (!global) {
            s->ignored = 1;
            break;
        }

        if (s->apply && syslog_mgmt_conf_get(&conf) != VTSS_OK) {
            break;
        }

        switch (s->id) {
        case CX_TAG_SERVER_MODE:
            CX_RC(cx_parse_val_bool(s, &mode, 1));
            conf.server_mode = mode;
            break;
        case CX_TAG_SERVER_ADDR:
            p = conf.syslog_server;
            if (cx_parse_val_txt(s, p, VTSS_SYS_HOSTNAME_LEN) == VTSS_OK) {
                for ( ; *p != '\0'; p++) {
                    if (*p == ' ') { /* Spaces not allowed */
                        CX_RC(cx_parm_invalid(s));
                    }
                }
                if (conf.syslog_server[0] != '\0') {
                    CX_RC(misc_str_is_hostname(conf.syslog_server));
                }
            }
            break;
        case CX_TAG_SYSLOG_LEVEL:
            CX_RC(cx_parse_val_kw(s, cx_kw_syslog_level, &val, 1));
            conf.syslog_level = val;
            break;
        default:
            s->ignored = 1;
            break;
        }
        if (s->apply) {
            CX_RC(syslog_mgmt_conf_set(&conf));
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

static vtss_rc SL_cx_gen_func(cx_get_state_t *s)
{
    switch (s->cmd) {
    case CX_GEN_CMD_GLOBAL:
        /* Global - syslog */
        T_D("global - syslog");
        CX_RC(cx_add_tag_line(s, CX_TAG_SYSLOG, 0));
        {
            syslog_conf_t conf;
            char *buf = "IP host address (a.b.c.d) or a host name string";

            if (syslog_mgmt_conf_get(&conf) == VTSS_OK) {
                CX_RC(cx_add_val_bool(s, CX_TAG_SERVER_MODE, conf.server_mode));
                CX_RC(cx_add_val_txt(s, CX_TAG_SERVER_ADDR, conf.syslog_server, buf));
                CX_RC(cx_add_val_kw(s, CX_TAG_SYSLOG_LEVEL, cx_kw_syslog_level, conf.syslog_level));
            }
        }
        CX_RC(cx_add_tag_line(s, CX_TAG_SYSLOG, 1));
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
CX_MODULE_TAB_ENTRY(
    VTSS_MODULE_ID_SYSLOG,
    syslog_cx_tag_table,
    0,
    0,
    NULL,                   /* init function       */
    SL_cx_gen_func,         /* Generation fucntion */
    SL_cx_parse_func        /* parse fucntion      */
);
