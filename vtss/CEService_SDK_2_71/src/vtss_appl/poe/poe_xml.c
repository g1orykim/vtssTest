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
#include "poe_api.h"
#include "poe_custom_api.h"

#include "conf_xml_api.h"
#include "conf_xml_trace_def.h"
#include "misc_api.h"

/* Tag IDs */
enum {
    /* Module tags */
    CX_TAG_POE,

    /* Group tags */
    CX_TAG_PORT_TABLE,

    /* Parameter tags */
    CX_TAG_ENTRY,
    CX_TAG_POE_MGMT,
    CX_TAG_POE_PRIMARY_POWER_SUPPLY,
    CX_TAG_POE_BACKUP_POWER_SUPPLY,

    /* Last entry */
    CX_TAG_NONE
};

/* Tag table */
static cx_tag_entry_t poe_cx_tag_table[CX_TAG_NONE + 1] = {
    [CX_TAG_POE] = {
        .name  = "PoE",
        .descr = "PoE",
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
    [CX_TAG_POE_MGMT] = {
        .name  = "poe_mgmt",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_POE_PRIMARY_POWER_SUPPLY] = {
        .name  = "primary_power_supply",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_POE_BACKUP_POWER_SUPPLY] = {
        .name  = "backup_power_supply",
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

/* Keyword for poe power*/
static const cx_kw_t cx_kw_poe_mgmt[] = {
    { "class_reserved", CLASS_RESERVED },
    { "class_consump", CLASS_CONSUMP },
    { "allocated_reserved", ALLOCATED_RESERVED },
    { "allocated_consump", ALLOCATED_CONSUMP },
    { "lldpmed_reserved", LLDPMED_RESERVED },
    { "lldpmed_consump", LLDPMED_CONSUMP },
    { NULL,     0 }
};

static const cx_kw_t cx_kw_poe_prio[] = {
    { "low",  LOW },
    { "high", HIGH },
    { "critical", CRITICAL },
    { NULL,       0 }
};

static const cx_kw_t cx_kw_poe_mode[] = {
    { "disabled", POE_MODE_POE_DISABLED },
    { "poe_af",   POE_MODE_POE },
    { "poe+",     POE_MODE_POE_PLUS},
    { NULL,       0 }
};

static BOOL cx_poe_match(const cx_table_context_t *context, ulong port_a, ulong port_b)
{
    poe_local_conf_t poe_local_conf;
    poe_mgmt_get_local_config(&poe_local_conf, context->isid);
    return (poe_local_conf.poe_mode[port_a] == poe_local_conf.poe_mode[port_b] &&
            poe_local_conf.priority[port_a] == poe_local_conf.priority[port_b] &&
            poe_local_conf.max_port_power[port_a] == poe_local_conf.max_port_power[port_b]);
}

static vtss_rc cx_poe_print(cx_get_state_t *s, const cx_table_context_t *context, ulong port_no, char *ports)
{

    poe_local_conf_t poe_local_conf;
    poe_mgmt_get_local_config(&poe_local_conf, context->isid);


    if (ports == NULL) {
        /* Syntax */
        VTSS_RC(cx_add_stx_start(s));
        VTSS_RC(cx_add_stx_port(s));
        VTSS_RC(cx_add_attr_txt(s, "poe_mode", "disabled,poe_af or poe+"));
        VTSS_RC(cx_add_attr_txt(s, "prio", "low, high or critical"));
        VTSS_RC(cx_add_attr_txt(s, "max_power", "Power in deciWatts. 0-300"));
        return cx_add_stx_end(s);
    }

    VTSS_RC(cx_add_port_start(s, CX_TAG_ENTRY, ports));
    VTSS_RC(cx_add_attr_kw(s, "poe_mode", cx_kw_poe_mode, poe_local_conf.poe_mode[port_no]));
    VTSS_RC(cx_add_attr_kw(s, "prio", cx_kw_poe_prio, poe_local_conf.priority[port_no]));
    VTSS_RC(cx_add_attr_long(s, "max_power", poe_local_conf.max_port_power[port_no]));
    return cx_add_port_end(s, CX_TAG_ENTRY);
}

static vtss_rc poe_cx_parse_func(cx_set_state_t *s)
{
    switch (s->cmd) {
    case CX_PARSE_CMD_PARM: {
        vtss_port_no_t port_idx;
        BOOL           port_list[VTSS_PORT_ARRAY_SIZE + 1];
        ulong          val;
        BOOL           global;
        poe_local_conf_t poe_local_conf;

        global = (s->isid == VTSS_ISID_GLOBAL);

        T_DG(VTSS_TRACE_GRP_POE, "PoE input");
        if (!global && s->apply) {
            poe_mgmt_get_local_config(&poe_local_conf, s->isid);
        }

        switch (s->id) {
        case CX_TAG_POE_BACKUP_POWER_SUPPLY :
            T_DG(VTSS_TRACE_GRP_POE, "CX_TAG_POE_BACKUP_POWER_SUPPLY");
            if (!global && cx_parse_ulong_dis(s, "val", &val, poe_custom_get_power_supply_min(),
                                              POE_POWER_SUPPLY_MAX, 0) == VTSS_OK) {
                poe_local_conf.backup_power_supply = val;
            } else {
                s->ignored = 1;
            }

            break;
        case CX_TAG_POE_PRIMARY_POWER_SUPPLY:
            T_DG(VTSS_TRACE_GRP_POE, "CX_TAG_POE_PRIMARY_POWER_SUPPLY, POE_MAX_POWER_SUPPLY = %d", POE_POWER_SUPPLY_MAX);
            if (!global && cx_parse_ulong_dis(s, "val", &val, poe_custom_get_power_supply_min(),
                                              POE_POWER_SUPPLY_MAX, 0) == VTSS_OK) {
                poe_local_conf.primary_power_supply = val;
            } else {
                s->ignored = 1;
            }
            break;

        case CX_TAG_POE_MGMT:
            T_DG(VTSS_TRACE_GRP_POE, "CX_TAG_POE_MGMT");
            if (global && (cx_parse_kw(s, "poe_mgmt", cx_kw_poe_mgmt, &val, 1) == VTSS_OK)) {
                if (s->apply) {
                    poe_master_conf_t poe_master_conf;
                    poe_mgmt_get_master_config(&poe_master_conf);

                    poe_master_conf.power_mgmt_mode = val;
                    poe_mgmt_set_master_config(&poe_master_conf);
                }
            } else {
                s->ignored = 1;
            }
            break;

        case CX_TAG_PORT_TABLE:
            T_DG(VTSS_TRACE_GRP_MIRROR, "CX_TAG_PORT_TABLE");
            if (global) {
                s->ignored = 1;
            }
            break;

        case CX_TAG_ENTRY:
            switch (s->group) {
            case  CX_TAG_PORT_TABLE :
                if (cx_parse_ports(s, port_list, 1) == VTSS_OK) {

                    BOOL                 poe_mode = 0;
                    BOOL                 mode_update = 0;
                    poe_priority_t       prio = LOW;
                    BOOL                 prio_update = 0;
                    int                  power = 0;
                    BOOL                 power_update = 0;

                    T_DG(VTSS_TRACE_GRP_POE, "Update mode");
                    s->p = s->next;
                    for (; s->rc == VTSS_OK && cx_parse_attr(s) == VTSS_OK; s->p = s->next) {
                        if (cx_parse_kw(s, "poe_mode", cx_kw_poe_mode, &val, 1) == VTSS_OK) {
                            mode_update = 1;
                            poe_mode = val;
                        }

                        if (cx_parse_kw(s, "prio", cx_kw_poe_prio, &val, 1) == VTSS_OK) {
                            prio_update = 1;
                            prio = val;
                        }

                        for (port_idx = VTSS_PORT_NO_START; port_idx < VTSS_PORT_NO_END; port_idx++) {
                            if (port_list[iport2uport(port_idx)]) {
                                if (cx_parse_ulong_dis(s, "max_power", &val, 0, poe_custom_get_port_power_max(port_idx), 0) == VTSS_OK) {
                                    power_update = 1;
                                    power = val;
                                } else {
                                    power_update = 0; // Signal update fail.
                                    break; // At least one of the ports are outside the range. Stop the for loop.
                                }
                            }
                        }
                    }

                    if (s->apply) {
                        for (port_idx = VTSS_PORT_NO_START; port_idx < VTSS_PORT_NO_END; port_idx++) {
                            if (port_list[iport2uport(port_idx)]) {
                                T_DG(VTSS_TRACE_GRP_POE, "setting enable/disable for port %u", port_idx);
                                if (mode_update) {
                                    poe_local_conf.poe_mode[port_idx] = poe_mode;
                                }


                                if (prio_update) {
                                    poe_local_conf.priority[port_idx] = prio;
                                }

                                if (power_update) {
                                    poe_local_conf.max_port_power[port_idx] = power;
                                }
                            }
                        }
                    }
                } else {
                    s->ignored = 1;
                }
                break;

            default :
                T_WG(VTSS_TRACE_GRP_POE, "Unknown group");
                break;
            }
            break;

        default:
            T_DG(VTSS_TRACE_GRP_POE, "Unknowned ID - PoE ignored");
            s->ignored = 1;
            break;
        }

        if (!global && s->apply) {
            T_DG(VTSS_TRACE_GRP_POE, "PoE  apply, isid = %d", s->isid);
            poe_mgmt_set_local_config(&poe_local_conf, s->isid);
        }
        T_NG(VTSS_TRACE_GRP_POE, "ignored = %d", s->ignored);

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

static vtss_rc poe_cx_gen_func(cx_get_state_t *s)
{
    switch (s->cmd) {
    case CX_GEN_CMD_GLOBAL:
        //////////////////
        /* Global - PoE */
        //////////////////
        T_D("global - poe");
        VTSS_RC(cx_add_tag_line(s, CX_TAG_POE, 0));

        // Get current configuration
        poe_local_conf_t poe_local_conf;
        poe_master_conf_t poe_master_conf;

        poe_mgmt_get_local_config(&poe_local_conf, VTSS_ISID_LOCAL);
        poe_mgmt_get_master_config(&poe_master_conf);

        // Help text
        VTSS_RC(cx_add_stx_start(s));
        VTSS_RC(cx_add_stx_kw(s, "poe_mgmt", cx_kw_poe_mgmt));
        VTSS_RC(cx_add_stx_end(s));

        VTSS_RC(cx_add_attr_start(s, CX_TAG_POE_MGMT));
        VTSS_RC(cx_add_attr_kw(s, "poe_mgmt", cx_kw_poe_mgmt,
                               poe_master_conf.power_mgmt_mode));
        VTSS_RC(cx_add_attr_end(s, CX_TAG_POE_MGMT));

        // Do the magic
        VTSS_RC(cx_add_tag_line(s, CX_TAG_POE, 1));
        break;
    case CX_GEN_CMD_SWITCH:
        /* Switch - PoE */
        T_D("switch - PoE");
        poe_mgmt_get_local_config(&poe_local_conf, s->isid); // Configuration

        VTSS_RC(cx_add_tag_line(s, CX_TAG_POE, 0));
        VTSS_RC(cx_add_val_ulong(s, CX_TAG_POE_PRIMARY_POWER_SUPPLY,
                                 poe_local_conf.primary_power_supply, 0,
                                 POE_POWER_SUPPLY_MAX));
        VTSS_RC(cx_add_val_ulong(s, CX_TAG_POE_BACKUP_POWER_SUPPLY,
                                 poe_local_conf.backup_power_supply, 0,
                                 poe_custom_get_power_supply_min()));

        CX_RC(cx_add_port_table(s, s->isid, CX_TAG_PORT_TABLE, cx_poe_match, cx_poe_print));
        CX_RC(cx_add_tag_line(s, CX_TAG_POE, 1));
        break;
    default:
        T_E("Unknown command");
        return VTSS_RC_ERROR;
    } /* End of Switch */

    return VTSS_OK;
}
/* Register the info in to the cx_module_table */
CX_MODULE_TAB_ENTRY(VTSS_MODULE_ID_POE, poe_cx_tag_table,
                    0, 0,
                    NULL,                  /* init function       */
                    poe_cx_gen_func,       /* Generation fucntion */
                    poe_cx_parse_func);    /* parse fucntion      */
