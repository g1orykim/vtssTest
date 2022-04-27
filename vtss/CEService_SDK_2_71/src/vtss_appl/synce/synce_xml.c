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
#include "synce.h"

#include "conf_xml_api.h"
#include "conf_xml_trace_def.h"

/* Tag IDs */
enum {
    /* Module tags */
    CX_TAG_SYNCE,

    /* Group tags */
    CX_TAG_PORT_TABLE,
    CX_TAG_GROUP_TABLE,

    /* Parameter tags */
    CX_TAG_SYNCE_SELECTION,
    CX_TAG_SYNCE_SSM_OVERWRITE,
    CX_TAG_SYNCE_MANUAL_CLK_SRC,
    CX_TAG_SYNCE_WTR,
    CX_TAG_SYNCE_SSM_UNLOCK,
    CX_TAG_SYNCE_NOMINATE,
    CX_TAG_SYNCE_NOMINATED,
    CX_TAG_ENTRY,

    /* Last entry */
    CX_TAG_NONE
};

/* Tag table */
static cx_tag_entry_t synce_cx_tag_table[CX_TAG_NONE + 1] =
{
    [CX_TAG_SYNCE] = {
        .name  = "SyncE",
        .descr = "SyncE",
        .type = CX_TAG_TYPE_MODULE
    },
    [CX_TAG_PORT_TABLE] = {
        .name  = "port_table",
        .descr = "",
        .type = CX_TAG_TYPE_GROUP
    },
    [CX_TAG_GROUP_TABLE] = {
        .name  = "group_table",
        .descr = "",
        .type = CX_TAG_TYPE_GROUP
    },
    [CX_TAG_SYNCE_SELECTION] = {
        .name  = "clock_selection",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_SYNCE_SSM_OVERWRITE] = {
        .name  = "ssm_overwrite",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_SYNCE_MANUAL_CLK_SRC] = {
        .name  = "manual_clk_src",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_SYNCE_WTR] = {
        .name  = "wtr",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_SYNCE_SSM_UNLOCK] = {
        .name  = "ssm_unlock",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_SYNCE_NOMINATE] = {
        .name  = "nominate",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_SYNCE_NOMINATED] = {
        .name  = "nominated",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
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

/* Keyword for synce ssm*/
static cx_kw_t cx_kw_synce_ssm[] = {
    { "ql_none",QL_NONE },
    { "ql_prc", QL_PRC  },
    { "ql_ssua", QL_SSUA},
    { "ql_ssub", QL_SSUB},
    { "ql_eec2", QL_EEC2},
    { "ql_eec1", QL_EEC1},
    { "ql_dnu",  QL_DNU},
    { NULL,     0 }
};

static cx_kw_t cx_kw_synce_ssm_unlock[] = {
    { "ql_none",QL_NONE },
    { "ql_prc", QL_PRC  },
    { "ql_ssua", QL_SSUA},
    { "ql_ssub", QL_SSUB},
    { "ql_eec2", QL_EEC2},
    { "ql_eec1", QL_EEC1},
    { NULL,     0 }
};

/* Keyword for synce selection*/
static const cx_kw_t cx_kw_synce_selection[] = {
    { "manual", SYNCE_MGMT_SELECTOR_MODE_MANUEL },
    { "manual_to_selected",SYNCE_MGMT_SELECTOR_MODE_MANUEL_TO_SELECTED},
    { "nonrevertive",SYNCE_MGMT_SELECTOR_MODE_AUTOMATIC_NONREVERTIVE},
    { "revertive",     SYNCE_MGMT_SELECTOR_MODE_AUTOMATIC_REVERTIVE},
    { NULL,     0 }
};

/* Synce specific set state structure */
typedef struct {
    int synce_count;
} synce_cx_set_state_t;

static BOOL cx_synce_match(const cx_table_context_t *context, ulong port_a, ulong port_b)
{

    synce_mgmt_port_conf_blk_t   port_config;
    synce_mgmt_port_config_get(&port_config);

    return (port_config.ssm_enabled[port_a] == port_config.ssm_enabled[port_b]);
}

static vtss_rc cx_synce_print(cx_get_state_t *s, const cx_table_context_t *context, ulong port_no, char *ports)
{

    synce_mgmt_port_conf_blk_t   port_config;
    synce_mgmt_port_config_get(&port_config);

    if (ports == NULL) {
        /* Syntax */
        cx_add_stx_start(s);
        cx_add_stx_port(s);
        cx_add_stx_bool(s, "ssm");
        return cx_add_stx_end(s);
    }

    cx_add_port_start(s, CX_TAG_ENTRY, ports);
    cx_add_attr_bool(s, "ssm",port_config.ssm_enabled[port_no]);
    return cx_add_port_end(s, CX_TAG_ENTRY);
}

static vtss_rc synce_cx_parse_func(cx_set_state_t *s)
{
    switch (s->cmd) {
    case CX_PARSE_CMD_PARM:
    {
        char           buf[256];
        vtss_port_no_t port_idx;
        BOOL           port_list[VTSS_PORT_ARRAY_SIZE+1];
        int            i;
        ulong          val;
        BOOL           global;
        vtss_priv_conf_t conf;
        T_NG(VTSS_TRACE_GRP_SYNCE,"CX_TAG_SYNCE");
        vtss_rc   rc;
        synce_mgmt_port_conf_blk_t   port_config;
        synce_mgmt_clock_conf_blk_t  clock_config;
        synce_cx_set_state_t *synce_state = s->mod_state;

        global = (s->isid == VTSS_ISID_GLOBAL);

        if ((rc = synce_mgmt_clock_config_get(&clock_config)) != VTSS_OK)
            T_EG(VTSS_TRACE_GRP_SYNCE,"%s\n", error_txt(rc));


        T_NG(VTSS_TRACE_GRP_SYNCE,"Synce input");
        if (!global && s->apply)
            synce_mgmt_port_config_get(&port_config);

        switch (s->id) {


        case CX_TAG_SYNCE_SELECTION:
            if (global && (cx_parse_kw(s, "clock_selection", cx_kw_synce_selection, &val, 1) == VTSS_OK)) {
                clock_config.selection_mode = val;
            } else {
                s->ignored = 1;
            }
            break;

        case CX_TAG_SYNCE_SSM_OVERWRITE:
            if (global && (cx_parse_kw(s, "ssm_overwrite", cx_kw_synce_ssm, &val, 1) == VTSS_OK)) {
                clock_config.ssm_overwrite[0] = val;
            } else {
                s->ignored = 1;
            }

            break;


        case CX_TAG_SYNCE_WTR:
            T_NG(VTSS_TRACE_GRP_SYNCE,"CX_TAG_SYNCE_WTR, group id = %d",s->id);
            if (global && cx_parse_ulong_dis(s, "val", &val, 0, SYNCE_WTR_MAX, 0) == VTSS_OK)
                clock_config.wtr_time = val;
            else
                s->ignored = 1;
            break;

        case CX_TAG_SYNCE_MANUAL_CLK_SRC:
            if (global && cx_parse_ulong_dis(s, "val", &val, 0, clock_my_input_max -1, 0) == VTSS_OK)
                clock_config.source = val;
            else
                s->ignored = 1;
            break;


        case CX_TAG_SYNCE_SSM_UNLOCK:

            T_NG(VTSS_TRACE_GRP_SYNCE,"CX_TAG_SYNCE_SSM_UNLOCK");
            if (global && (cx_parse_kw(s, "ssm_unlock", cx_kw_synce_ssm_unlock, &val, 1) == VTSS_OK)) {
                clock_config.ssm_unlocked = val;
            } else {
                s->ignored = 1;
            }
            break;


        case CX_TAG_ENTRY:
            T_NG(VTSS_TRACE_GRP_SYNCE,"CX_TAG_ENTRY, id = %d, group = %d",s->id,s->group);
            switch (s->group) {
            case  CX_TAG_PORT_TABLE :
                T_NG(VTSS_TRACE_GRP_SYNCE,"CX_TAG_PORT_TABLE group = %d",s->group);
                if (cx_parse_ports(s, port_list, 1) == VTSS_OK) {

                    BOOL                 ssm = 0;
                    BOOL                 ssm_update = 0;

                    s->p = s->next;
                    for (; s->rc == VTSS_OK && cx_parse_attr(s) == VTSS_OK; s->p = s->next) {
                        if (cx_parse_bool(s, "ssm", &ssm, 1) == VTSS_OK) {
                            ssm_update = 1;
                        }

                    }

                    if (s->apply) {
                        for (port_idx = VTSS_PORT_NO_START; port_idx < VTSS_PORT_NO_END; port_idx++) {
                            if (port_list[iport2uport(port_idx)]) {
                                T_DG_PORT(VTSS_TRACE_GRP_SYNCE,port_idx,"setting enable/disable");
                                if (ssm_update) {
                                    synce_mgmt_ssm_set(port_idx,ssm);
                                }
                            }
                        }

                    }
                } else {
                    s->ignored = 1;
                }
                break;
            case CX_TAG_GROUP_TABLE:
                if (1) {
                    T_NG(VTSS_TRACE_GRP_SYNCE,"CX_TAG_GROUP_TABLE");
                    int                  port = clock_config.port[synce_state->synce_count]; // Set to a port it is currently configured to.
                    BOOL                 port_update = 0;
                    BOOL                 nominated = 0;
                    BOOL                 nominated_update = 0;
                    synce_mgmt_quality_level_t ssm_overwrite = clock_config.ssm_overwrite[synce_state->synce_count]; // Set ssm_overwrite to the current configuration.
                    BOOL                 ssm_overwrite_update = 0;
                    uint                 clk_prio = clock_config.priority[synce_state->synce_count]; // Set to a prio it is currently configured to.
                    BOOL                 clk_prio_update = 0;

                    for (; s->rc == VTSS_OK && cx_parse_attr(s) == VTSS_OK; s->p = s->next) {
                        if (cx_parse_bool(s, "nominated", &nominated, 1) == VTSS_OK) {
                            nominated_update = 1;
                        }

                        if (cx_parse_ulong_dis(s, "port", &val, 0,SYNCE_PORT_MAX, 0) == VTSS_OK) {
                            port_update = 1;
                            port = val;
                        }

                        if (cx_parse_ulong_dis(s, "prio", &val, 0,synce_my_priority_max -1, 0) == VTSS_OK) {
                            clk_prio_update = 1;
                            clk_prio = val;
                        }

                        if (cx_parse_kw(s, "ssm_overwrite", cx_kw_synce_ssm, &val, 1) == VTSS_OK) {
                            ssm_overwrite_update = 1;
                            ssm_overwrite = val;
                        } else {

                        }
                        T_NG(VTSS_TRACE_GRP_SYNCE,"CX_TAG_GROUP_TABLE, ssm_overwrite = %d, port = %d, nominated = %d",ssm_overwrite,port,nominated);
                    }


                    if (s->apply) {
                        if (synce_state->synce_count < synce_my_nominated_max) {
                            if (clk_prio_update) {
                                if ((rc = synce_mgmt_nominated_priority_set(synce_state->synce_count,clk_prio)) != VTSS_OK) {
                                    s->ignored=1;
                                    T_EG(VTSS_TRACE_GRP_SYNCE,"%s\n", error_txt(rc));
                                }
                            }
                            if (nominated_update || ssm_overwrite_update || port_update) {
                                if (nominated == 0) {
                                    port = 0; // Setting port = 0 means denominate the clock source.
                                }

                                T_DG(VTSS_TRACE_GRP_SYNCE,"ssm_overwrite = %d, port = %d , clk_src = %d",ssm_overwrite,port,synce_state->synce_count);
                                if ((rc = synce_mgmt_nominated_source_set(synce_state->synce_count, nominated, port, SYNCE_MGMT_SELECTOR_SLAVE, ssm_overwrite)) != VTSS_OK) {
                                    s->ignored=1;
                                    T_EG(VTSS_TRACE_GRP_SYNCE,"%s\n", error_txt(rc));
                                }
                            }
                        }
                    }
                }

                T_NG(VTSS_TRACE_GRP_SYNCE,"CX_TAG_GROUP_TABLE group = %d, synce_state->synce_count = %d",s->group,synce_state->synce_count);
                synce_state->synce_count++;
                break;
            default :
                T_WG(VTSS_TRACE_GRP_SYNCE,"Unknown group  ( in entry )");
                break;
            }


            break;

        case CX_TAG_GROUP_TABLE:
            T_DG(VTSS_TRACE_GRP_SYNCE,"CX_TAG_GROUP_TABLE flush");
            /* Flush group table */
            synce_state->synce_count = 0;
            break;


        case CX_TAG_PORT_TABLE :
            if (global)
                s->ignored = 1;

            T_DG(VTSS_TRACE_GRP_SYNCE,"CX_TAG_PORT_TABLE group = %d",s->group);
            break;

        default :
            T_WG(VTSS_TRACE_GRP_SYNCE,"SyncE Unknown group id = %d",s->id);
            break;
        }


        if (global && s->apply) {
            T_NG(VTSS_TRACE_GRP_SYNCE,"SYNCE  apply, isid = %d",s->isid);
            synce_mgmt_nominated_selection_mode_set(clock_config.selection_mode,
                                                    clock_config.source,
                                                    clock_config.wtr_time,
                                                    clock_config.ssm_unlocked);
        }


        T_NG(VTSS_TRACE_GRP_SYNCE,"ignored = %d",s->ignored);

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

static vtss_rc synce_cx_gen_func(cx_set_state_t *s)
{
    switch (s->cmd) {
        case CX_GEN_CMD_GLOBAL:

            //////////////////
            /* Global - SYNCE */
            //////////////////
            T_DG(VTSS_TRACE_GRP_SYNCE,"global - synce");
            cx_add_tag_line(s, CX_TAG_SYNCE, 0);

            // Get current configuration
            synce_mgmt_port_conf_blk_t   port_config;
            synce_mgmt_port_config_get(&port_config);

            vtss_rc   rc;
            synce_mgmt_clock_conf_blk_t     clock_config;
            if ((rc = synce_mgmt_clock_config_get(&clock_config)) != VTSS_OK)
                T_EG(VTSS_TRACE_GRP_SYNCE,"%s\n", error_txt(rc));
            // Clock selection Help text + Clock selection Configuration
            cx_add_stx_start(s);
            cx_add_stx_kw(s, "clock_selection", cx_kw_synce_selection);
            cx_add_stx_end(s);
            cx_add_attr_start(s,CX_TAG_SYNCE_SELECTION);
            cx_add_attr_kw(s, "clock_selection", cx_kw_synce_selection,clock_config.selection_mode);
            cx_add_attr_end(s,CX_TAG_SYNCE_SELECTION);

            cx_add_val_ulong(s, CX_TAG_SYNCE_MANUAL_CLK_SRC, clock_config.source, 0, clock_my_input_max -1);


            // SSM unlock
            cx_add_stx_start(s);
            cx_add_stx_kw(s, "ssm_unlock", cx_kw_synce_ssm_unlock);
            cx_add_stx_end(s);
            cx_add_attr_start(s,CX_TAG_SYNCE_SSM_UNLOCK);
            cx_add_attr_kw(s, "ssm_unlock", cx_kw_synce_ssm_unlock,clock_config.ssm_unlocked);
            cx_add_attr_end(s,CX_TAG_SYNCE_SSM_UNLOCK);

            // WTR
            cx_add_val_ulong(s, CX_TAG_SYNCE_WTR, clock_config.wtr_time, 0,SYNCE_WTR_MAX);


            /* nominate */
            cx_add_stx_start(s);
            cx_add_stx_ulong(s, "clk_src", 0, synce_my_nominated_max-1);
            cx_add_stx_bool(s, "nominated");
            cx_add_stx_ulong(s, "port",SYNCE_PORT_MIN , SYNCE_PORT_MAX);// help text for port
            cx_add_stx_kw(s, "ssm_overwrite", cx_kw_synce_ssm);        // help text for ssm_overwrite
            cx_add_stx_ulong(s, "prio", 0, synce_my_priority_max-1);
            cx_add_stx_end(s);

            cx_add_tag_line(s, CX_TAG_GROUP_TABLE, 0);
            int clk_src;
            for (clk_src = 0; clk_src < synce_my_nominated_max; clk_src++) {
                //        cx_add_attr_start(s, CX_TAG_SYNCE_NOMINATE);
                cx_add_attr_start(s, CX_TAG_ENTRY);
                cx_add_attr_ulong(s, "clk_src", clk_src);

                // nominated
                cx_add_attr_bool(s, "nominated", clock_config.nominated[clk_src]);

                // port
                cx_add_attr_ulong(s, "port", clock_config.port[clk_src]);

                // SSM overwrite
                cx_add_attr_kw(s, "ssm_overwrite", cx_kw_synce_ssm,clock_config.ssm_overwrite[clk_src]);


                // Priority
                cx_add_attr_ulong(s, "prio",clock_config.priority[clk_src]);

                cx_add_attr_end(s, CX_TAG_ENTRY);
                //        cx_add_attr_end(s,CX_TAG_SYNCE_NOMINATE);
            }
            CX_RC(cx_add_tag_line(s, CX_TAG_GROUP_TABLE, 1));

            // Do the magic
            CX_RC(cx_add_tag_line(s, CX_TAG_SYNCE, 1));
            break;
        case CX_GEN_CMD_SWITCH:
            T_DG(VTSS_TRACE_GRP_SYNCE,"switch - SyncE");
            cx_add_tag_line(s, CX_TAG_SYNCE, 0);
            CX_RC(cx_add_port_table(s, isid, CX_TAG_PORT_TABLE, cx_synce_match, cx_synce_print));
            CX_RC(cx_add_tag_line(s, CX_TAG_SYNCE, 1));
            break;
        default:
            T_E("Unknown command");
            return VTSS_RC_ERROR;
            break;
    } /* End of Switch */

    return VTSS_OK;
}
/* Register the info in to the cx_module_table */
VTSS_CX_MODULE_TAB_ENTRY(VTSS_MODULE_ID_SYNCE, synce_cx_tag_table,
                         sizeof(synce_cx_set_state_t), 0,
                         NULL,                    /* init function       */
                         synce_cx_gen_func,       /* Generation fucntion */
                         synce_cx_parse_func);    /* parse fucntion      */
