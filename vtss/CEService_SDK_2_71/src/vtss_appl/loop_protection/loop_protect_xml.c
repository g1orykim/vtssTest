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

#include "conf_xml_api.h"
#include "conf_xml_trace_def.h"
#include "loop_protect_api.h"
#include "misc_api.h"

/* Tag IDs */
enum {
    /* Module tags */
    CX_TAG_LPROT,

    /* Group tags */
    CX_TAG_PORT_TABLE,

    /* Parameter tags */
    CX_TAG_GBL_ENABLED,
    CX_TAG_GBL_TRANSMISSION_TIME,
    CX_TAG_GBL_SHUTDOWN_TIME,
    CX_TAG_ENTRY,
    CX_TAG_PORT_ENABLED,
    CX_TAG_PORT_ACTION,
    CX_TAG_PORT_TRANSMIT,

    /* Last entry */
    CX_TAG_NONE
};

/* Tag table */
static cx_tag_entry_t lprot_cx_tag_table[CX_TAG_NONE + 1] =
{
    [CX_TAG_LPROT] = {
        .name  = "lprot",
        .descr = "Loop Protection",
        .type = CX_TAG_TYPE_MODULE
    },
    [CX_TAG_PORT_TABLE] = {
        .name  = "ports",
        .descr = "",
        .type = CX_TAG_TYPE_GROUP
    },
    [CX_TAG_GBL_ENABLED] = {
        .name  = "enabled",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_GBL_TRANSMISSION_TIME] = {
        .name  = "txinterval",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_GBL_SHUTDOWN_TIME] = {
        .name  = "shutdown",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_ENTRY] = {
        .name  = "entry",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_PORT_ENABLED] = {
        .name  = "enabled",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_PORT_ACTION] = {
        .name  = "action",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_PORT_TRANSMIT] = {
        .name  = "transmit",
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

/* Keyword for action */
static const cx_kw_t cx_kw_lprot_action[] = {
    { "shutdown", LOOP_PROTECT_ACTION_SHUTDOWN},
    { "shut_log", LOOP_PROTECT_ACTION_SHUT_LOG},
    { "log",      LOOP_PROTECT_ACTION_LOG_ONLY},
    { NULL,       0 }
};

static BOOL cx_lprot_match(const cx_table_context_t *context, ulong port_a, ulong port_b)
{
    loop_protect_port_conf_t conf_a, conf_b;
    return (loop_protect_conf_port_get(context->isid, port_a, &conf_a) == VTSS_OK &&
            loop_protect_conf_port_get(context->isid, port_b, &conf_b) == VTSS_OK &&
            memcmp(&conf_a, &conf_b, sizeof(conf_a)) == 0);
}

static vtss_rc cx_lprot_print(cx_get_state_t *s, const cx_table_context_t *context, ulong port_no, char *ports)
{
    loop_protect_port_conf_t conf;

    if (ports == NULL) {
        /* Syntax */
        CX_RC(cx_add_stx_start(s));
        CX_RC(cx_add_stx_port(s));
        CX_RC(cx_add_stx_bool(s, "enabled"));
        CX_RC(cx_add_stx_kw(s, "action", cx_kw_lprot_action));
        CX_RC(cx_add_stx_bool(s, "transmit"));
        return cx_add_stx_end(s);
    }

    CX_RC(loop_protect_conf_port_get(context->isid, port_no, &conf));
    CX_RC(cx_add_port_start(s, CX_TAG_ENTRY, ports));
    CX_RC(cx_add_attr_bool(s, "enabled", conf.enabled));
    CX_RC(cx_add_attr_kw(s, "action", cx_kw_lprot_action, conf.action));
    CX_RC(cx_add_attr_bool(s, "transmit", conf.transmit));
    return cx_add_port_end(s, CX_TAG_ENTRY);
}

static vtss_rc lprot_cx_parse_func(cx_set_state_t *s)
{
    switch (s->cmd) {
    case CX_PARSE_CMD_PARM:
    {
        BOOL           port_list[VTSS_PORT_ARRAY_SIZE+1];
        ulong          val;
        BOOL           global;
        loop_protect_conf_t conf;

        CX_RC(loop_protect_conf_get(&conf));
        global = (s->isid == VTSS_ISID_GLOBAL);

        switch (s->id) {
        case CX_TAG_GBL_ENABLED:
            if (global && cx_parse_val_bool(s, &conf.enabled, 1) != VTSS_OK)
                s->ignored = 1;
            break;
        case CX_TAG_GBL_TRANSMISSION_TIME:
            if (global && cx_parse_val_ulong(s, &val, 1, 10) == VTSS_OK)
                conf.transmission_time = val;
            else
                s->ignored = 1;
            break;
        case CX_TAG_GBL_SHUTDOWN_TIME:
            if (global && cx_parse_val_ulong(s, &val, 0, 60*60*24*7) == VTSS_OK)
                conf.shutdown_time = val;
            else
                s->ignored = 1;
            break;
        case CX_TAG_PORT_TABLE:
            if (global)
                s->ignored = 1;
            break;
        case CX_TAG_ENTRY:
            if (!global && s->group == CX_TAG_PORT_TABLE &&
                cx_parse_ports(s, port_list, 1) == VTSS_OK) {
                vtss_port_no_t port_idx;
                BOOL seen_ena = FALSE, seen_action = FALSE, seen_transmit = FALSE;
                BOOL enabled = FALSE, transmit = FALSE;
                ulong action = LOOP_PROTECT_ACTION_SHUTDOWN;

                s->p = s->next;
                for (; s->rc == VTSS_OK && cx_parse_attr(s) == VTSS_OK; s->p = s->next) {
                    if (cx_parse_bool(s, "enabled", &enabled, TRUE) == VTSS_OK) {
                        seen_ena = TRUE;
                    } else if (cx_parse_kw(s, "action", cx_kw_lprot_action, &action, TRUE) == VTSS_OK) {
                        seen_action = TRUE;
                    } else if(cx_parse_bool(s, "transmit", &transmit, TRUE) == VTSS_OK) {
                        seen_transmit = TRUE;
                    }
                }

                for (port_idx = VTSS_PORT_NO_START; port_idx < VTSS_PORT_NO_END; port_idx++) {
                    if (!port_list[iport2uport(port_idx)])
                        continue;
                    if (s->apply) {
                        loop_protect_port_conf_t pconf;
                        if(loop_protect_conf_port_get(s->isid, port_idx, &pconf) == VTSS_OK) {
                            if (seen_ena)
                                pconf.enabled = enabled;
                            if (seen_action)
                                pconf.action = action;
                            if (seen_transmit)
                                pconf.transmit = transmit;
                            CX_RC(loop_protect_conf_port_set(s->isid, port_idx, &pconf));
                        }
                    }
                }
            } else {
                s->ignored = 1;
            }
            break;
        default:
            break;
        }

        if (s->apply) {
            T_D("XML apply");
            CX_RC(loop_protect_conf_set(&conf));
        }

        break;
    } /* CX_PARSE_CMD_PARM */
    default:
        break;
    }
    
    return s->rc;
}

static vtss_rc lprot_cx_gen_func(cx_get_state_t *s)
{
    loop_protect_conf_t conf;

    switch (s->cmd) {
    case CX_GEN_CMD_GLOBAL:
        /* Global - LPROT */
        T_D("global - lprot");
        CX_RC(cx_add_tag_line(s, CX_TAG_LPROT, 0));
        if (loop_protect_conf_get(&conf) == VTSS_OK) {
            CX_RC(cx_add_val_bool(s, CX_TAG_GBL_ENABLED, conf.enabled));
            CX_RC(cx_add_val_ulong(s, CX_TAG_GBL_TRANSMISSION_TIME, conf.transmission_time, 1, 10));
            CX_RC(cx_add_val_ulong(s, CX_TAG_GBL_SHUTDOWN_TIME, conf.shutdown_time, 0, 60*60*24*7));
        }
        CX_RC(cx_add_tag_line(s, CX_TAG_LPROT, 1));
        break;
    case CX_GEN_CMD_SWITCH:
        /* Switch - LPROT */
        T_D("switch - lprot");
        CX_RC(cx_add_tag_line(s, CX_TAG_LPROT, 0));
        CX_RC(cx_add_port_table(s, s->isid, CX_TAG_PORT_TABLE, cx_lprot_match, cx_lprot_print));
        CX_RC(cx_add_tag_line(s, CX_TAG_LPROT, 1));
        break;
    default:
        T_E("Unknown command");
        return VTSS_RC_ERROR;
    } /* End of Switch */

    return VTSS_OK;
}

/* Register the info in to the cx_module_table */
CX_MODULE_TAB_ENTRY(
    VTSS_MODULE_ID_LOOP_PROTECT,
    lprot_cx_tag_table,
    0,
    0,
    NULL,                       /* init function       */
    lprot_cx_gen_func,        /* Generation fucntion */
    lprot_cx_parse_func       /* parse fucntion      */
);

