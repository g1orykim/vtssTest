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

#include "conf_xml_api.h"
#include "conf_xml_trace_def.h"
#include "misc_api.h"
#include "mgmt_api.h"

#include "aggr_xml.h"

/* Tag IDs */
enum {
    /* Module tags */
    CX_TAG_AGGR,

    /* Group tags */
    CX_TAG_PORT_TABLE,
    CX_TAG_AGGR_TABLE,

    /* Parameter tags */
    CX_TAG_ENTRY,
    CX_TAG_MODE_SMAC,
    CX_TAG_MODE_DMAC,
    CX_TAG_MODE_IP,
    CX_TAG_MODE_PORT,

    /* Last entry */
    CX_TAG_NONE
};

/* Tag table */
static cx_tag_entry_t aggr_cx_tag_table[CX_TAG_NONE + 1] = {
    [CX_TAG_AGGR] = {
        .name  = "aggr",
        .descr = "Link aggregation",
        .type = CX_TAG_TYPE_MODULE
    },
    [CX_TAG_PORT_TABLE] = {
        .name  = "port_table",
        .descr = "",
        .type = CX_TAG_TYPE_GROUP
    },
    [CX_TAG_AGGR_TABLE] = {
        .name  = "aggr_table",
        .descr = "",
        .type = CX_TAG_TYPE_GROUP
    },
    [CX_TAG_ENTRY] = {
        .name  = "entry",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_MODE_SMAC] = {
        .name  = "mode_smac",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_MODE_DMAC] = {
        .name  = "mode_dmac",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_MODE_IP] = {
        .name  = "mode_ip",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_MODE_PORT] = {
        .name  = "mode_port",
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
#ifdef VTSS_SW_OPTION_LACP
/* Keyword for LACP role */
static const cx_kw_t cx_kw_lacp_role[] = {
    { "active",  VTSS_LACP_ACTMODE_ACTIVE },
    { "passive", VTSS_LACP_ACTMODE_PASSIVE },
    { NULL,      0 }
};

static const cx_kw_t cx_kw_lacp_timeout[] = {
    { "fast",  VTSS_LACP_FSMODE_FAST },
    { "slow", VTSS_LACP_FSMODE_SLOW },
    { NULL,      0 }
};


static BOOL cx_lacp_match(const cx_table_context_t *context, ulong port_a, ulong port_b)
{
    vtss_lacp_port_config_t conf_a, conf_b;

    return (lacp_mgmt_port_conf_get(context->isid, port_a, &conf_a) == VTSS_RC_OK &&
            lacp_mgmt_port_conf_get(context->isid, port_b, &conf_b) == VTSS_RC_OK &&
            memcmp(&conf_a, &conf_b, sizeof(conf_a)) == 0);
}

static vtss_rc cx_lacp_print(cx_get_state_t *s, const cx_table_context_t *context, ulong port_no, char *ports)
{
    vtss_lacp_port_config_t conf;

    if (ports == NULL) {
        /* Syntax */
        cx_add_stx_start(s);
        cx_add_stx_port(s);
        cx_add_stx_bool(s, "mode");
        cx_add_attr_txt(s, "key", "auto or 1-65535");
        cx_add_stx_kw(s, "role", cx_kw_lacp_role);
        cx_add_stx_kw(s, "timeout", cx_kw_lacp_timeout);
        cx_add_attr_txt(s, "prio", "0-65535");
        return cx_add_stx_end(s);
    }

    CX_RC(lacp_mgmt_port_conf_get(context->isid, port_no, &conf));
    cx_add_port_start(s, CX_TAG_ENTRY, ports);
    cx_add_attr_bool(s, "mode", conf.enable_lacp);
    cx_add_attr_ulong_word(s, "key", conf.port_key, "auto", VTSS_LACP_AUTOKEY);
    cx_add_attr_kw(s, "role", cx_kw_lacp_role, conf.active_or_passive);
    cx_add_attr_kw(s, "timeout", cx_kw_lacp_timeout, conf.xmit_mode);
    cx_add_attr_ulong(s, "prio", conf.port_prio);
    return cx_add_port_end(s, CX_TAG_ENTRY);
}
#endif /* VTSS_SW_OPTION_LACP */

static vtss_rc aggr_cx_parse_func(cx_set_state_t *s)
{
    switch (s->cmd) {
    case CX_PARSE_CMD_PARM: {
        BOOL                port_list[VTSS_PORT_ARRAY_SIZE + 1];
        vtss_port_no_t      port_idx;
        vtss_aggr_mode_t    mode;
        BOOL                global;
        aggr_cx_set_state_t *aggr_state = s->mod_state;
#ifdef VTSS_SW_OPTION_LACP
        ulong               val;
#endif
        global = (s->isid == VTSS_ISID_GLOBAL);
        if (global && s->apply && aggr_mgmt_aggr_mode_get(&mode) != VTSS_RC_OK) {
            break;
        }

        switch (s->id) {
        case CX_TAG_MODE_SMAC:
            if (global) {
                cx_parse_val_bool(s, &mode.smac_enable, 1);
            } else {
                s->ignored = 1;
            }
            break;
        case CX_TAG_MODE_DMAC:
            if (global) {
                cx_parse_val_bool(s, &mode.dmac_enable, 1);
            } else {
                s->ignored = 1;
            }
            break;
        case CX_TAG_MODE_IP:
            if (global) {
                cx_parse_val_bool(s, &mode.sip_dip_enable, 1);
            } else {
                s->ignored = 1;
            }
            break;
        case CX_TAG_MODE_PORT:
            if (global) {
                cx_parse_val_bool(s, &mode.sport_dport_enable, 1);
            } else {
                s->ignored = 1;
            }
            break;
        case CX_TAG_PORT_TABLE:
            if (global) {
                s->ignored = 1;
            }
            break;
        case CX_TAG_AGGR_TABLE: {
            if (global) {
                s->ignored = 1;
            } else {
                /* Delete aggregation state */
                for (port_idx = VTSS_PORT_NO_START; port_idx < VTSS_PORT_NO_END; port_idx++) {
                    aggr_state->aggr_no[port_idx] = 0;
                }
            }
            break;
        }
        case CX_TAG_ENTRY: {
            ulong                aggr_id;
            aggr_mgmt_group_no_t aggr_no;
            BOOL                 port_mem[VTSS_PORT_ARRAY_SIZE];

            if (!global && s->group == CX_TAG_AGGR_TABLE &&
                (s->rc = cx_parse_ulong(s, "aggr_id", &aggr_id, MGMT_AGGR_ID_START,
                                        MGMT_AGGR_ID_END - 1)) == VTSS_RC_OK) {
                s->p = s->next;
                aggr_no = mgmt_aggr_id2no(aggr_id);
                if (cx_parse_ports(s, port_list, 1) == VTSS_RC_OK) {
                    aggr_state->line_aggr = s->line;

                    for (port_idx = VTSS_PORT_NO_START;
                         port_idx < VTSS_PORT_NO_END; port_idx++) {
                        port_mem[port_idx] = port_list[iport2uport(port_idx)];
                    }
                    if (mgmt_aggr_add_check(s->isid, aggr_no, port_mem, s->msg) != VTSS_RC_OK) {
                        s->rc = CONF_XML_ERROR_FILE_PARM;
                    }

                    for (port_idx = VTSS_PORT_NO_START;
                         s->rc == VTSS_RC_OK && port_idx < VTSS_PORT_NO_END; port_idx++) {
                        if (port_list[iport2uport(port_idx)]) {
                            if (aggr_state->aggr_no[port_idx]) {
                                sprintf(s->msg, "Port %u is included in multiple aggregations",
                                        iport2uport(port_idx));
                                s->rc = CONF_XML_ERROR_FILE_PARM;
                            }
                            aggr_state->aggr_no[port_idx] = aggr_no;
                        }
                    }
                }
#ifdef VTSS_SW_OPTION_LACP
            } else if (!global && s->group == CX_TAG_PORT_TABLE &&
                       cx_parse_ports(s, port_list, 1) == VTSS_RC_OK) {
                vtss_lacp_port_config_t conf;
                BOOL                    mode = 0, key = 0, role = 0, prio = 0, timeout = 0;

                s->p = s->next;
                for (; s->rc == VTSS_RC_OK && cx_parse_attr(s) == VTSS_RC_OK; s->p = s->next) {
                    if (cx_parse_bool(s, "mode", &conf.enable_lacp, 1) == VTSS_RC_OK) {
                        mode = 1;
                        aggr_state->line_lacp = s->line;
                    } else if (cx_parse_ulong_word(s, "key", &val, 1, 65535, "auto",
                                                   VTSS_LACP_AUTOKEY) == VTSS_RC_OK) {
                        key = 1;
                        conf.port_key = val;
                    } else if (cx_parse_kw(s, "role", cx_kw_lacp_role, &val, 1) == VTSS_RC_OK) {
                        role = 1;
                        conf.active_or_passive = val;
                    } else if (cx_parse_ulong(s, "prio", &val, 0, 65535) == VTSS_RC_OK) {
                        prio = 1;
                        conf.port_prio = val;
                    } else if (cx_parse_kw(s, "timeout", cx_kw_lacp_timeout, &val, 1) == VTSS_RC_OK) {
                        timeout = 1;
                        conf.xmit_mode = val;
                    } else {
                        cx_parm_unknown(s);
                    }
                }
                for (port_idx = VTSS_PORT_NO_START; port_idx < VTSS_PORT_NO_END; port_idx++)
                    if (port_list[iport2uport(port_idx)]) {
                        if (mode) {
                            aggr_state->lacp[port_idx].enable_lacp = conf.enable_lacp;
                        }
                        if (key) {
                            aggr_state->lacp[port_idx].port_key = conf.port_key;
                        }
                        if (role) {
                            aggr_state->lacp[port_idx].active_or_passive = conf.active_or_passive;
                        }
                        if (prio) {
                            aggr_state->lacp[port_idx].port_prio = conf.port_prio;
                        }
                        if (timeout) {
                            aggr_state->lacp[port_idx].xmit_mode = conf.xmit_mode;
                        }

                    }
#endif /* VTSS_SW_OPTION_LACP */
            } else {
                s->ignored = 1;
            }
            break;
        }
        default:
            s->ignored = 1;
            break;
        }
        if (global && s->apply) {
            aggr_mgmt_aggr_mode_set(&mode);
        }
        break;
        } /* CX_PARSE_CMD_PARM */
    case CX_PARSE_CMD_GLOBAL:
        break;
    case CX_PARSE_CMD_SWITCH: {
        aggr_mgmt_group_member_t group;
        aggr_mgmt_group_no_t     aggr_no = 0;
        vtss_port_no_t           port_idx;
        aggr_cx_set_state_t *aggr_state = s->mod_state;

        if (s->init) {
            aggr_state->line_aggr.number = 0;
            aggr_state->line_lacp.number = 0;
            for (port_idx = VTSS_PORT_NO_START; port_idx < VTSS_PORT_NO_END;
                 port_idx++) {
                aggr_state->aggr_no[port_idx] = 0;
            }
            /* Aggregation state */
            while (aggr_mgmt_port_members_get(s->isid,
                                              aggr_no, &group, 1) == VTSS_RC_OK) {
                aggr_no = group.aggr_no;
                for (port_idx = VTSS_PORT_NO_START;
                     port_idx < VTSS_PORT_NO_END; port_idx++)
                    if (group.entry.member[port_idx]) {
                        aggr_state->aggr_no[port_idx] = aggr_no;
                    }
            } /* end of while */
#ifdef VTSS_SW_OPTION_LACP
            /* LACP state */
            for (port_idx = VTSS_PORT_NO_START;
                 port_idx < VTSS_PORT_NO_END; port_idx++) {
                aggr_state->lacp[port_idx].enable_lacp = 0;
                if (!PORT_NO_IS_STACK(port_idx)) {
                    lacp_mgmt_port_conf_get(s->isid, port_idx, &aggr_state->lacp[port_idx]);
                }
            }
#endif /* VTSS_SW_OPTION_LACP */

        } else if (s->apply) {
            /* Delete aggregations */
            while (aggr_mgmt_port_members_get(s->isid,
                                              aggr_no, &group, 1) == VTSS_RC_OK) {
                aggr_mgmt_group_del(s->isid, group.aggr_no);
            }

            /* Add aggregations */
            while (1) {
                aggr_no = 0;
                for (port_idx = VTSS_PORT_NO_START;
                     port_idx < VTSS_PORT_NO_END; port_idx++) {
                    group.entry.member[port_idx] = 0;
                    if (aggr_state->aggr_no[port_idx]) {
                        if (aggr_no == 0) {
                            aggr_no = aggr_state->aggr_no[port_idx];
                        }
                        if (aggr_state->aggr_no[port_idx] == aggr_no) {
                            group.entry.member[port_idx] = 1;
                            aggr_state->aggr_no[port_idx] = 0;
                        }
                    }
                }
                if (aggr_no == 0) {
                    break;
                }
                aggr_mgmt_port_members_add(s->isid, aggr_no, &group.entry);
            }
#ifdef VTSS_SW_OPTION_LACP
            /*
             * Disable LACP mode. Must be done last because
             * lacp_mgmt_port_conf_set() does not allow any changes if
             * the other protocols are enabled.
             */
            for (port_idx = VTSS_PORT_NO_START;
                 port_idx < VTSS_PORT_NO_END; port_idx++) {
                vtss_lacp_port_config_t conf;
                if (!PORT_NO_IS_STACK(port_idx) &&
                    lacp_mgmt_port_conf_get(s->isid, port_idx, &conf) == VTSS_RC_OK && conf.enable_lacp) {
                    conf.enable_lacp = 0;
                    (void)lacp_mgmt_port_conf_set(s->isid, port_idx, &conf);
                } /* end of if (!PORT....) */
            } /* end of for */
#endif /* VTSS_SW_OPTION_LACP */

#ifdef VTSS_SW_OPTION_LACP
            /* Set LACP mode */
            for (port_idx = VTSS_PORT_NO_START; port_idx < VTSS_PORT_NO_END; port_idx++)
                if (!PORT_NO_IS_STACK(port_idx)) {
                    (void)lacp_mgmt_port_conf_set(s->isid, port_idx, &aggr_state->lacp[port_idx]);
                }
#endif /* VTSS_SW_OPTION_LACP */
        }
        break;
    }
    default:
        break;
    }
    return s->rc;
}


static vtss_rc  aggr_cx_gen_func(cx_get_state_t *s)
{
    char buf[MGMT_PORT_BUF_SIZE];

    switch (s->cmd) {
    case CX_GEN_CMD_GLOBAL:
        /* Global - Aggregation */
        T_D("global - aggr");
        cx_add_tag_line(s, CX_TAG_AGGR, 0);
        {
            vtss_aggr_mode_t mode;

            if (aggr_mgmt_aggr_mode_get(&mode) == VTSS_RC_OK) {
                cx_add_val_bool(s, CX_TAG_MODE_SMAC, mode.smac_enable);
                cx_add_val_bool(s, CX_TAG_MODE_DMAC, mode.dmac_enable);
                cx_add_val_bool(s, CX_TAG_MODE_IP, mode.sip_dip_enable);
                cx_add_val_bool(s, CX_TAG_MODE_PORT, mode.sport_dport_enable);
            }
        }
        CX_RC(cx_add_tag_line(s, CX_TAG_AGGR, 1));
        break;
    case CX_GEN_CMD_SWITCH:
        /* Switch - Aggregation */
        T_D("switch - aggr");
        cx_add_tag_line(s, CX_TAG_AGGR, 0);
        {
            aggr_mgmt_group_no_t     aggr_no = 0;
            aggr_mgmt_group_member_t group;

            cx_add_tag_line(s, CX_TAG_AGGR_TABLE, 0);

            /* Entry syntax */
            cx_add_stx_start(s);
            cx_add_stx_ulong(s, "aggr_id", MGMT_AGGR_ID_START, MGMT_AGGR_ID_END - 1);
            cx_add_stx_port(s);
            cx_add_stx_end(s);

            while (aggr_mgmt_port_members_get(s->isid, aggr_no, &group, 1) == VTSS_RC_OK) {
                aggr_no = group.aggr_no;
                cx_add_attr_start(s, CX_TAG_ENTRY);
                cx_add_attr_ulong(s, "aggr_id", mgmt_aggr_no2id(aggr_no));
                cx_add_attr_txt(s, "port", mgmt_iport_list2txt(group.entry.member, buf));
                CX_RC(cx_add_attr_end(s, CX_TAG_ENTRY));
            }
            cx_add_tag_line(s, CX_TAG_AGGR_TABLE, 1);
        }

#ifdef VTSS_SW_OPTION_LACP
        CX_RC(cx_add_port_table(s, s->isid, CX_TAG_PORT_TABLE, cx_lacp_match, cx_lacp_print));
#endif /* VTSS_SW_OPTION_LACP */
        CX_RC(cx_add_tag_line(s, CX_TAG_AGGR, 1));

        break;
    default:
        T_E("Unknown command");
        return VTSS_RC_ERROR;
        break;
    } /* End of switch */

    return VTSS_RC_OK;
}

/* Register the info in to the cx_module_table */
CX_MODULE_TAB_ENTRY(
    VTSS_MODULE_ID_AGGR,
    aggr_cx_tag_table,
    sizeof(aggr_cx_set_state_t),
    0,
    NULL,                    /* init function       */
    aggr_cx_gen_func,        /* Generation fucntion */
    aggr_cx_parse_func       /* parse fucntion      */
);

