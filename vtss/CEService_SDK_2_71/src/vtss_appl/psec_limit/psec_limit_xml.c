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
#include "psec_limit_api.h"

#include "conf_xml_api.h"
#include "conf_xml_trace_def.h"
#include "misc_api.h"

/* Tag IDs */
enum {
    /* Module tags */
    CX_TAG_PSEC_LIMIT,

    /* Group tags */
    CX_TAG_PORT_TABLE,

    /* Parameter tags */
    CX_TAG_ENTRY,
    CX_TAG_MODE,
    CX_TAG_AGE,
    CX_TAG_AGETIME,

    /* Last entry */
    CX_TAG_NONE
};

/* Tag table */
static cx_tag_entry_t psec_limit_cx_tag_table[CX_TAG_NONE + 1] = {
    [CX_TAG_PSEC_LIMIT] = {
        .name = "limit",
        .descr = "Port Security Limit Control",
        .type = CX_TAG_TYPE_MODULE
    },
    [CX_TAG_PORT_TABLE] = {
        .name  = "port_table",
        .descr = "",
        .type = CX_TAG_TYPE_GROUP
    },
    [CX_TAG_ENTRY] = {
        .name  = "entry",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_MODE] = {
        .name  = "mode",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_AGE] = {
        .name  = "age",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_AGETIME] = {
        .name  = "agetime",
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

/* psec_limit specific set state structure */
typedef struct {
    // One *could* avoid this if we saved the entire configuration
    // for every <entry> in the <psec_limit> section.
    psec_limit_switch_cfg_t psec_limit;
} psec_limit_cx_set_state_t;

/* Keyword for Port Security Limit Control Action */
static const cx_kw_t cx_kw_psec_limit_action[] = {
    { "none",          PSEC_LIMIT_ACTION_NONE },
    { "trap",          PSEC_LIMIT_ACTION_TRAP },
    { "shutdown",      PSEC_LIMIT_ACTION_SHUTDOWN },
    { "trap_shutdown", PSEC_LIMIT_ACTION_TRAP_AND_SHUTDOWN },
    { NULL,           0 }
};

static BOOL cx_psec_limit_match(const cx_table_context_t *context, ulong port_a, ulong port_b)
{
    psec_limit_switch_cfg_t switch_cfg;
    return (psec_limit_mgmt_switch_cfg_get(context->isid, &switch_cfg) == VTSS_OK && memcmp(&switch_cfg.port_cfg[port_a - VTSS_PORT_NO_START], &switch_cfg.port_cfg[port_b - VTSS_PORT_NO_START], sizeof(psec_limit_port_cfg_t)) == 0);
}

static vtss_rc cx_psec_limit_print(cx_get_state_t *s, const cx_table_context_t *context, ulong port_no, char *ports)
{
    psec_limit_switch_cfg_t switch_cfg;
    psec_limit_port_cfg_t   *port_cfg;

    if (ports == NULL) {
        /* Syntax */
        (void)cx_add_stx_start(s);
        (void)cx_add_stx_port(s);
        (void)cx_add_stx_bool (s, "mode");
        (void)cx_add_stx_ulong(s, "limit", PSEC_LIMIT_LIMIT_MIN, PSEC_LIMIT_LIMIT_MAX);
        (void)cx_add_stx_kw   (s, "action", cx_kw_psec_limit_action);
        return cx_add_stx_end(s);
    }

    CX_RC(psec_limit_mgmt_switch_cfg_get(context->isid, &switch_cfg));
    port_cfg = &switch_cfg.port_cfg[port_no - VTSS_PORT_NO_START];
    (void)cx_add_port_start(s, CX_TAG_ENTRY, ports);
    (void)cx_add_attr_bool (s, "mode", port_cfg->enabled);
    (void)cx_add_attr_ulong(s, "limit", port_cfg->limit);
    (void)cx_add_attr_kw   (s, "action", cx_kw_psec_limit_action, port_cfg->action);
    return cx_add_port_end(s, CX_TAG_ENTRY);
}

static vtss_rc psec_limit_cx_parse_func(cx_set_state_t *s)
{
    psec_limit_cx_set_state_t *psec_limit_state = s->mod_state;

    switch (s->cmd) {
    case CX_PARSE_CMD_PARM: {
        vtss_port_no_t port_idx;
        BOOL           port_list[VTSS_PORT_ARRAY_SIZE + 1];
        ulong          val;
        BOOL           global;
        psec_limit_glbl_cfg_t glbl_cfg;

        global = (s->isid == VTSS_ISID_GLOBAL);

        // Apply global config
        if (global && s->apply && psec_limit_mgmt_glbl_cfg_get(&glbl_cfg) != VTSS_OK) {
            break;
        }

        switch (s->id) {
        case CX_TAG_MODE:
            if (global) {
                (void)cx_parse_val_bool(s, &glbl_cfg.enabled, 1);
            } else {
                s->ignored = 1;
            }
            break;

        case CX_TAG_AGE:
            if (global) {
                (void)cx_parse_val_bool(s, &glbl_cfg.enable_aging, 1);
            } else {
                s->ignored = 1;
            }
            break;

        case CX_TAG_AGETIME:
            if (global && cx_parse_val_ulong(s, &val, PSEC_LIMIT_AGING_PERIOD_SECS_MIN, PSEC_LIMIT_AGING_PERIOD_SECS_MAX) == VTSS_OK) {
                glbl_cfg.aging_period_secs = val;
            } else {
                s->ignored = 1;
            }
            break;

        case CX_TAG_PORT_TABLE:
            if (global) {
                s->ignored = 1;
            }
            break;

        case CX_TAG_ENTRY: {
            if (!global && s->group == CX_TAG_PORT_TABLE && cx_parse_ports(s, port_list, 1) == VTSS_OK) {
                psec_limit_port_cfg_t port_cfg;
                BOOL  enabled_seen = FALSE;
                BOOL  limit_seen   = FALSE;
                BOOL  action_seen  = FALSE;

                s->p = s->next;
                for (; s->rc == VTSS_OK && cx_parse_attr(s) == VTSS_OK; s->p = s->next) {
                    if (cx_parse_bool(s, "mode", &port_cfg.enabled, 1) == VTSS_OK) {
                        enabled_seen = TRUE;
                    } else if (cx_parse_ulong(s, "limit", &port_cfg.limit, PSEC_LIMIT_LIMIT_MIN, PSEC_LIMIT_LIMIT_MAX) == VTSS_OK) {
                        limit_seen   = TRUE;
                    } else if (cx_parse_kw(s, "action", cx_kw_psec_limit_action, &val, 1) == VTSS_OK) {
                        port_cfg.action = val;
                        action_seen     = 1;
                    } else {
                        (void)cx_parm_unknown(s);
                    }
                }
                for (port_idx = VTSS_PORT_NO_START; port_idx < VTSS_PORT_NO_END; port_idx++) {
                    if (port_list[iport2uport(port_idx)]) {
                        if (enabled_seen) {
                            psec_limit_state->psec_limit.port_cfg[port_idx - VTSS_PORT_NO_START].enabled = port_cfg.enabled;
                        }
                        if (limit_seen) {
                            psec_limit_state->psec_limit.port_cfg[port_idx - VTSS_PORT_NO_START].limit   = port_cfg.limit;
                        }
                        if (action_seen) {
                            psec_limit_state->psec_limit.port_cfg[port_idx - VTSS_PORT_NO_START].action  = port_cfg.action;
                        }
                    }
                }
            } else {
                s->ignored = 1;
            }
            break;
        }

        default:
            s->ignored = 1;
            break;
        } /* switch (s->id) */

        if (global && s->apply) {
            (void)psec_limit_mgmt_glbl_cfg_set(&glbl_cfg);
        }
        break;
        } /* CX_PARSE_CMD_PARM */

    case CX_PARSE_CMD_GLOBAL:
        break;

    case CX_PARSE_CMD_SWITCH:
        if (s->init) {
            // Get Port Security Limit State
            (void)psec_limit_mgmt_switch_cfg_get(s->isid, &psec_limit_state->psec_limit);
        } else if (s->apply) {
            // Set Port Security Limit State
            (void)psec_limit_mgmt_switch_cfg_set(s->isid, &psec_limit_state->psec_limit);
        }
        break;
    default:
        break;
    }
    return s->rc;
}

static vtss_rc psec_limit_cx_gen_func(cx_get_state_t *s)
{
    switch (s->cmd) {
    case CX_GEN_CMD_GLOBAL: {
        psec_limit_glbl_cfg_t glbl_cfg;

        // Global - Port Security Limit Control
        (void)cx_add_tag_line(s, CX_TAG_PSEC_LIMIT, 0);
        if (psec_limit_mgmt_glbl_cfg_get(&glbl_cfg) == VTSS_OK) {
            (void)cx_add_val_bool (s, CX_TAG_MODE,    glbl_cfg.enabled);
            (void)cx_add_val_bool (s, CX_TAG_AGE,     glbl_cfg.enable_aging);
            (void)cx_add_val_ulong(s, CX_TAG_AGETIME, glbl_cfg.aging_period_secs, PSEC_LIMIT_AGING_PERIOD_SECS_MIN, PSEC_LIMIT_AGING_PERIOD_SECS_MAX);
        }
        CX_RC(cx_add_tag_line(s, CX_TAG_PSEC_LIMIT, 1));
        break;
    }

    case CX_GEN_CMD_SWITCH:
        /* Switch - Port Security Limit Control */
        (void)cx_add_tag_line(s, CX_TAG_PSEC_LIMIT, 0);
        CX_RC(cx_add_port_table(s, s->isid, CX_TAG_PORT_TABLE, cx_psec_limit_match, cx_psec_limit_print));
        CX_RC(cx_add_tag_line(s, CX_TAG_PSEC_LIMIT, 1));
        break;

    default:
        T_E("Unknown command");
        return VTSS_RC_ERROR;
    }

    return VTSS_OK;
}

/* Register the info in to the cx_module_table */
CX_MODULE_TAB_ENTRY(VTSS_MODULE_ID_PSEC_LIMIT, psec_limit_cx_tag_table,
                    sizeof(psec_limit_cx_set_state_t), 0,
                    NULL,                         /* init function       */
                    psec_limit_cx_gen_func,       /* Generation function */
                    psec_limit_cx_parse_func);    /* parse function      */
