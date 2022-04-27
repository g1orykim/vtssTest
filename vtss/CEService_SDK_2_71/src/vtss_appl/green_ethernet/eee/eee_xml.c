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
#include "eee_api.h"
#include "conf_xml_api.h"
#include "conf_xml_trace_def.h"
#include "mgmt_api.h"
#include "port_api.h"

/* Tag IDs */
enum {
    /* Module tags */
    CX_TAG_EEE,

    /* Group tags */
    CX_TAG_PORT_TABLE,

    CX_TAG_ENTRY,

    CX_TAG_EEE_OPTIMIZED_FOR_POWER,

    /* Last entry */
    CX_TAG_NONE
};

/* Tag table */
static cx_tag_entry_t eee_cx_tag_table[CX_TAG_NONE + 1] = {
    [CX_TAG_EEE] = {
        .name  = "EEE",
        .descr = "Energy Efficient Ethernet",
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
    [CX_TAG_EEE_OPTIMIZED_FOR_POWER] = {
        .name  = "optimized_for_power",
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

/* EEE specific set state structure */
typedef struct {
    cx_line_t  line_eee;  /* EEE line */
} eee_cx_set_state_t;

/* Keyword for EEE mode */
static const cx_kw_t cx_kw_eee_mode[] = {
    { "enabled",  TRUE },
    { "disabled", FALSE },
    { NULL,       0 }
};

static BOOL cx_eee_match(const cx_table_context_t *context, ulong port_a, ulong port_b)
{
    eee_switch_conf_t conf;

    (void)eee_mgmt_switch_conf_get(context->isid, &conf);
    return conf.port[port_a].eee_ena  == conf.port[port_b].eee_ena
#if EEE_FAST_QUEUES_CNT > 0
           && conf.port[port_a].eee_fast_queues  == conf.port[port_b].eee_fast_queues
#endif
           ;
}

static vtss_rc cx_eee_print(cx_get_state_t *s, const cx_table_context_t *context, ulong port_no, char *ports)
{
    eee_switch_conf_t conf;
#if EEE_FAST_QUEUES_CNT > 0
    char buf[80];
    BOOL bool_list[32];
#endif

    if (ports == NULL) {
        /* Syntax */
        (void) cx_add_stx_start(s);
        (void) cx_add_stx_port(s);
        (void) cx_add_stx_kw(s, "mode", cx_kw_eee_mode);
#if EEE_FAST_QUEUES_CNT > 0
        (void) cx_add_attr_txt(s, "urgent_queues", cx_list_txt(buf, EEE_FAST_QUEUES_MIN, EEE_FAST_QUEUES_MAX));
#endif
        return cx_add_stx_end(s);
    }


    (void)eee_mgmt_switch_conf_get(context->isid, &conf);

    (void) cx_add_port_start(s, CX_TAG_ENTRY, ports);
    (void) cx_add_attr_kw(s, "mode", cx_kw_eee_mode, conf.port[port_no].eee_ena);

#if EEE_FAST_QUEUES_CNT > 0
    ulong fast_qu = (u32) conf.port[port_no].eee_fast_queues; // Type conversions
    (void) mgmt_ulong2bool_list(fast_qu, &bool_list[0]); // Convert to array
    (void) mgmt_list2txt(&bool_list[0], 0, EEE_FAST_QUEUES_CNT - 1, buf); // Convert to txt
    (void) cx_add_attr_txt(s, "urgent_queues", &buf[0]);
#endif
    return cx_add_port_end(s, CX_TAG_ENTRY);
}



static vtss_rc eee_cx_parse_func(cx_set_state_t *s)
{
    vtss_rc rc;
    switch (s->cmd) {
    case CX_PARSE_CMD_PARM: {
        BOOL               port_list[VTSS_PORT_ARRAY_SIZE + 1];
        ulong              val;
        BOOL               global;
        eee_switch_conf_t  conf;
        eee_switch_global_conf_t  global_conf;
        eee_switch_state_t switch_state;

        global = (s->isid == VTSS_ISID_GLOBAL);

        (void)eee_mgmt_switch_global_conf_get(&global_conf);


        if (s->mod_tag == CX_TAG_EEE) {
            (void)eee_mgmt_switch_state_get(s->isid, &switch_state);

            if (s->apply) {
                (void)eee_mgmt_switch_conf_get(s->isid, &conf);
            }

            switch (s->id) {
            case CX_TAG_PORT_TABLE:
                if (global) {
                    s->ignored = 1;
                }
                break;
            case CX_TAG_ENTRY: {
                port_iter_t pit;

                if (!global && s->group == CX_TAG_PORT_TABLE &&
                    cx_parse_ports(s, port_list, 1) == VTSS_OK) {
                    uchar state = 0;
                    BOOL mode = FALSE;
#if EEE_FAST_QUEUES_CNT > 0
                    BOOL urgent_qu_found = FALSE;
                    BOOL urgent_qu_list[EEE_FAST_QUEUES_CNT];
                    char buf[256];
#endif
                    s->p = s->next;
                    for (; s->rc == VTSS_OK && cx_parse_attr(s) == VTSS_OK; s->p = s->next) {
                        if (cx_parse_kw(s, "mode", cx_kw_eee_mode, &val, 1) == VTSS_OK) {
                            mode = 1;
                            state = val;
#if EEE_FAST_QUEUES_CNT > 0
                        } else if (cx_parse_txt(s, "urgent_queues", buf, sizeof(buf)) == VTSS_OK) {
                            if (mgmt_txt2list(buf, urgent_qu_list, 0, EEE_FAST_QUEUES_CNT, 0) == VTSS_OK) {
                                urgent_qu_found = TRUE;
                            } else {
                                T_W("Problems converting to list");
                            }
#endif
                        } else {
                            (void) cx_parm_unknown(s);
                        }
                    }

                    /* Port setup */
                    (void)port_iter_init(&pit, NULL, s->isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
                    while (port_iter_getnext(&pit)) {
                        if (s->apply && port_list[pit.uport] && switch_state.port[pit.iport].eee_capable) {
#if EEE_FAST_QUEUES_CNT > 0
                            T_NG_PORT(VTSS_TRACE_GRP_EEE, pit.iport, "mode %d, urgent_qu_found = %d", mode , urgent_qu_found);
#endif
                            if (mode) {
                                conf.port[pit.iport].eee_ena = state;
                            }
#if EEE_FAST_QUEUES_CNT > 0
                            if (urgent_qu_found) {
                                conf.port[pit.iport].eee_fast_queues = (uchar) mgmt_bool_list2ulong(&urgent_qu_list[EEE_FAST_QUEUES_MIN]);
                                T_DG_PORT(VTSS_TRACE_GRP_EEE, pit.iport, "conf.port[%lu].eee_fast_queues = %d", pit.iport, conf.port[pit.iport].eee_fast_queues);
                            }
#endif
                        }
                    }
                } else {
                    s->ignored = 1;
                }
                break;
            }

            case CX_TAG_EEE_OPTIMIZED_FOR_POWER:
                if (global) {
                    CX_RC(cx_parse_val_bool(s, &global_conf.optimized_for_power, TRUE));
                } else {
                    s->ignored = 1;
                }
                break;


            default:
                s->ignored = 1;
                break;
            }

            if (s->apply) {
                if (global) {
                    T_IG(VTSS_TRACE_GRP_EEE, "optimized_for_power:%d", global_conf.optimized_for_power);
                    if ((rc = eee_mgmt_switch_global_conf_set(&global_conf)) != VTSS_OK) {
                        T_W("Could not set EEE configuration : %s", eee_error_txt(rc));
                    }
                    T_IG(VTSS_TRACE_GRP_EEE, "optimized_for_power:%d", global_conf.optimized_for_power);
                } else {
                    if (eee_mgmt_switch_conf_set(s->isid, &conf) != VTSS_OK) {
                        T_W("Could not set EEE configuration");
                    }
                }
            }
            break;
        } /* CX_TAG_EEE */
        } /* CX_PARSE_CMD_PARM */
    case CX_PARSE_CMD_GLOBAL: {
        eee_cx_set_state_t *eee_state = s->mod_state;
        eee_switch_conf_t eee_conf;

        if (s->init) {
            eee_state->line_eee.number = 0;
            (void)eee_mgmt_switch_conf_get(s->isid, &eee_conf);
        } else if (s->apply) {
            (void)eee_mgmt_switch_conf_get(s->isid, &eee_conf);
            (void)eee_mgmt_switch_conf_set(s->isid, &eee_conf);
        }
        break;
    }
    case CX_PARSE_CMD_SWITCH:
        break;
    default:
        break;
    }
    return s->rc;
}

static vtss_rc eee_cx_gen_func(cx_get_state_t *s)
{
    eee_switch_global_conf_t eee_global_conf;
    switch (s->cmd) {
    case CX_GEN_CMD_GLOBAL:
        CX_RC(cx_add_tag_line(s, CX_TAG_EEE, 0)); // Header
        /* Global - EEE */
        (void)eee_mgmt_switch_global_conf_get(&eee_global_conf); // Since this is the same for all switches we can simply get the configuration frm the local switch.
        T_IG(VTSS_TRACE_GRP_EEE, "optimized_for_power %d, isid:%d;", eee_global_conf.optimized_for_power, s->isid);
        CX_RC(cx_add_val_bool(s, CX_TAG_EEE_OPTIMIZED_FOR_POWER, eee_global_conf.optimized_for_power));
        CX_RC(cx_add_tag_line(s, CX_TAG_EEE, 1)); // End header
        break;
    case CX_GEN_CMD_SWITCH:
        /* Switch - EEE */
        (void) cx_add_tag_line(s, CX_TAG_EEE, 0);
        CX_RC(cx_add_port_table(s, s->isid, CX_TAG_PORT_TABLE, cx_eee_match, cx_eee_print));
        CX_RC(cx_add_tag_line(s, CX_TAG_EEE, 1));
        break;
    default:
        T_E("Unknown command");
        return VTSS_RC_ERROR;
    } /* End of Switch */

    return VTSS_OK;
}

/* Register the info in to the cx_module_table */
CX_MODULE_TAB_ENTRY(
    VTSS_MODULE_ID_EEE,
    eee_cx_tag_table,
    sizeof(eee_cx_set_state_t),
    0,
    NULL,                  /* Init function       */
    eee_cx_gen_func,       /* Generation function */
    eee_cx_parse_func      /* Parse function      */
);
