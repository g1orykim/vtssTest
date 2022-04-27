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
#include "mirror_api.h"

#include "conf_xml_api.h"
#include "conf_xml_trace_def.h"
#include "misc_api.h"
#include "port_api.h" /* Reqd for VTSS_FRONT_PORT_COUNT */

#if VTSS_SWITCH_STACKABLE
#include "topo_api.h"
#endif


/* Tag IDs */
enum {
    /* Module tags */
    CX_TAG_MIRROR,

    /* Group tags */
    CX_TAG_PORT_TABLE,

    /* Parameter tags */
    CX_TAG_ENTRY,
    CX_TAG_MIRROR_PORT,
    CX_TAG_MIRROR_SID,
#ifdef VTSS_FEATURE_MIRROR_CPU
    CX_TAG_MIRROR_CPU,
#endif
    /* Last entry */
    CX_TAG_NONE
};

/* Tag table */
static cx_tag_entry_t mirror_cx_tag_table[CX_TAG_NONE + 1] = {
    [CX_TAG_MIRROR] = {
        .name  = "mirror",
        .descr = "Port mirroring",
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
    [CX_TAG_MIRROR_PORT] = {
        .name  = "mirror_port",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_MIRROR_SID] = {
        .name  = "sid",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
#ifdef VTSS_FEATURE_MIRROR_CPU
    [CX_TAG_MIRROR_CPU] = {
        .name  = "cpu",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
#endif
    /* Last entry */
    [CX_TAG_NONE] = {
        .name  = "",
        .descr = "",
        .type = CX_TAG_TYPE_NONE
    }
};

/* Keyword for mirror mode (rx, tx) */
static const cx_kw_t cx_kw_mirror[] = {
    { "enabled",  CX_DUAL_VAL(1, 1) },
    { "disabled", CX_DUAL_VAL(0, 0) },
    { "rx",       CX_DUAL_VAL(1, 0) },
    { "tx",       CX_DUAL_VAL(0, 1) },
    { NULL,     0 }
};

static BOOL cx_mirror_match(const cx_table_context_t *context, ulong port_a, ulong port_b)
{

    mirror_switch_conf_t switch_conf;

    (void) mirror_mgmt_switch_conf_get(context->isid, &switch_conf);

    return (switch_conf.src_enable[port_a] == switch_conf.src_enable[port_b] &&
            switch_conf.dst_enable[port_a] == switch_conf.dst_enable[port_b]);
}

static vtss_rc cx_mirror_print(cx_get_state_t *s, const cx_table_context_t *context, ulong port_no, char *ports)
{
    mirror_switch_conf_t conf;

    if (ports == NULL) {
        /* Syntax */
        (void) cx_add_stx_start(s);
        (void) cx_add_stx_port(s);
        (void) cx_add_stx_kw(s, "mode", cx_kw_mirror);
        return cx_add_stx_end(s);
    }

    (void) mirror_mgmt_switch_conf_get(context->isid, &conf);

    (void) cx_add_port_start(s, CX_TAG_ENTRY, ports);
    (void) cx_add_attr_kw(s, "mode", cx_kw_mirror,
                          CX_DUAL_VAL(conf.src_enable[port_no], conf.dst_enable[port_no]));
    return cx_add_port_end(s, CX_TAG_ENTRY);
}

static vtss_rc mirror_cx_parse_func(cx_set_state_t *s)
{
    switch (s->cmd) {
    case CX_PARSE_CMD_PARM: {
        vtss_port_no_t port_idx;
        BOOL           port_list[VTSS_PORT_ARRAY_SIZE + 1];
        ulong          val;
        BOOL           global;
        mirror_conf_t  conf;

        global = (s->isid == VTSS_ISID_GLOBAL);
        if (global && s->apply) {
            mirror_mgmt_conf_get(&conf);
        }

        T_DG(VTSS_TRACE_GRP_MIRROR, "id = %d", (int)s->id);
        switch (s->id) {
        case CX_TAG_MIRROR_PORT:
            T_DG(VTSS_TRACE_GRP_MIRROR, "CX_TAG_MIRROR_PORT");
            if (global && cx_parse_ulong_dis(s, "val", &val, 1,
                                             VTSS_FRONT_PORT_COUNT, 0) == VTSS_OK) {
                conf.dst_port  = uport2iport(val);
            } else {
                s->ignored = 1;
            }
            break;

        case CX_TAG_MIRROR_SID:
            T_DG(VTSS_TRACE_GRP_MIRROR, "CX_TAG_MIRROR_SID");
#if VTSS_SWITCH_STACKABLE
            if (global && cx_parse_val_ulong(s, &val, VTSS_USID_START, VTSS_USID_END) == VTSS_OK) {
                conf.mirror_switch  = topo_usid2isid(val);
            } else {
                s->ignored = 1;
            }
#endif /* VTSS_SWITCH_STACKABLE */
            break;
#ifdef VTSS_FEATURE_MIRROR_CPU
        case CX_TAG_MIRROR_CPU:
            if (!global &&  cx_parse_kw(s, "cpu_mode", cx_kw_mirror, &val, 1) == VTSS_OK) {
                if (s->apply) {
                    mirror_switch_conf_t local_conf;
                    mirror_mgmt_switch_conf_get(s->isid, &local_conf); // Get current configuration

                    local_conf.cpu_src_enable = CX_DUAL_V1(val);
                    local_conf.cpu_dst_enable = CX_DUAL_V2(val);

                    mirror_mgmt_switch_conf_set(s->isid, &local_conf);
                }
            } else {
                s->ignored = 1;
            }
            break;
#endif
        case CX_TAG_PORT_TABLE:
            T_DG(VTSS_TRACE_GRP_MIRROR, "CX_TAG_PORT_TABLE");
            if (global) {
                s->ignored = 1;
            }
            break;

        case CX_TAG_ENTRY:
            if (!global && s->group == CX_TAG_PORT_TABLE &&
                cx_parse_ports(s, port_list, 1) == VTSS_OK) {
                mirror_switch_conf_t conf;
                BOOL                 src_ena = 0;
                BOOL                 dst_ena = 0;
                BOOL                 mode = 0;

                T_DG(VTSS_TRACE_GRP_MIRROR, "Update mode");
                s->p = s->next;
                for (; s->rc == VTSS_OK && cx_parse_attr(s) == VTSS_OK; s->p = s->next) {
                    if (cx_parse_kw(s, "mode", cx_kw_mirror, &val, 1) == VTSS_OK) {
                        mode = 1;
                        src_ena = CX_DUAL_V1(val);
                        dst_ena = CX_DUAL_V2(val);
                    }
                }
                if (s->apply && mode) {
                    /* Port setup */
                    (void) mirror_mgmt_switch_conf_get(s->isid, &conf);
                    for (port_idx = VTSS_PORT_NO_START; port_idx < VTSS_PORT_NO_END; port_idx++)
                        if (port_list[iport2uport(port_idx)]) {
                            T_DG(VTSS_TRACE_GRP_MIRROR, "setting enable/disable");
                            conf.src_enable[port_idx] = src_ena;
                            conf.dst_enable[port_idx] = dst_ena;
                        }
                    mirror_mgmt_switch_conf_set(s->isid, &conf);
                }
            } else {
                s->ignored = 1;
            }
            break;
        default:
            T_DG(VTSS_TRACE_GRP_MIRROR, "Mirror ignored");
            s->ignored = 1;
            break;
        }
        if (global && s->apply) {
            T_DG(VTSS_TRACE_GRP_MIRROR, "Mirror appling, sid = %d", conf.mirror_switch);
            mirror_mgmt_conf_set(&conf);
        }

        T_NG(VTSS_TRACE_GRP_MIRROR, "ignored = %d", s->ignored);
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

static vtss_rc  mirror_cx_gen_func(cx_get_state_t *s)
{
    char           buf[128];
    switch (s->cmd) {
    case CX_GEN_CMD_GLOBAL:

        /* Global - MIRROR */
        T_DG(VTSS_TRACE_GRP_MIRROR, "global - mirror");
        cx_add_tag_line(s, CX_TAG_MIRROR, 0);
        {
            char          stx[32];
            mirror_conf_t conf;
            mirror_mgmt_conf_get(&conf);

#if VTSS_SWITCH_STACKABLE
            cx_add_val_ulong(s, CX_TAG_MIRROR_SID,
                             topo_isid2usid(conf.mirror_switch),
                             VTSS_USID_START, VTSS_USID_CNT);
#endif /* VTSS_SWITCH_STACKABLE */
            if (conf.dst_port == VTSS_PORT_NO_NONE) {
                strcpy(buf, "disabled");
            } else {
                sprintf(buf, "%u", iport2uport(conf.dst_port));
            }
            sprintf(stx, "disabled or %u-%u", 1, VTSS_FRONT_PORT_COUNT);
            cx_add_val_txt(s, CX_TAG_MIRROR_PORT, buf, stx);
        }
        CX_RC(cx_add_tag_line(s, CX_TAG_MIRROR, 1));
        break;
    case CX_GEN_CMD_SWITCH:
        /* Switch - MIRROR */
        T_D("switch - mirror");
        cx_add_tag_line(s, CX_TAG_MIRROR, 0);
        CX_RC(cx_add_port_table(s, s->isid, CX_TAG_PORT_TABLE, cx_mirror_match, cx_mirror_print));
#ifdef VTSS_FEATURE_MIRROR_CPU
        mirror_switch_conf_t switch_conf;

        (void) mirror_mgmt_switch_conf_get(s->isid, &switch_conf);

        // Added CPU mode comment
        CX_RC(cx_add_stx_start(s));
        CX_RC(cx_add_stx_kw(s, "cpu_mode", cx_kw_mirror));
        CX_RC(cx_add_stx_end(s));

        // Add CPU mode
        CX_RC(cx_add_attr_start(s, CX_TAG_MIRROR_CPU));
        CX_RC(cx_add_attr_kw(s, "cpu_mode", cx_kw_mirror,
                             CX_DUAL_VAL(switch_conf.cpu_src_enable, switch_conf.cpu_dst_enable)));
        cx_add_attr_end(s, CX_TAG_MIRROR_CPU);
#endif

        CX_RC(cx_add_tag_line(s, CX_TAG_MIRROR, 1));
        break;
    default:
        T_E("Unknown command");
        return VTSS_RC_ERROR;
        break;
    } /* End of Switch */

    return VTSS_OK;
}

/* Register the info in to the cx_module_table */
CX_MODULE_TAB_ENTRY(
    VTSS_MODULE_ID_MIRROR,
    mirror_cx_tag_table,
    0,
    0,
    NULL,                   /* init function       */
    mirror_cx_gen_func,     /* Generation fucntion */
    mirror_cx_parse_func    /* parse fucntion        */
);

