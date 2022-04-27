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
#include "pvlan_api.h"

#include "conf_xml_api.h"
#include "conf_xml_trace_def.h"
#include "misc_api.h"
#include "mgmt_api.h"
#include "port_api.h"

/* Tag IDs */
enum {
    /* Module tags */
    CX_TAG_PVLAN,

    /* Group tags */
    CX_TAG_PORT_TABLE,
    CX_TAG_PVLAN_TABLE,

    /* Parameter tags */
    CX_TAG_ENTRY,

    /* Last entry */
    CX_TAG_NONE
};

/* Tag table */
static cx_tag_entry_t pvlan_cx_tag_table[CX_TAG_NONE + 1] = {
    [CX_TAG_PVLAN] = {
        .name  = "pvlan",
        .descr = "Private VLAN",
        .type = CX_TAG_TYPE_MODULE
    },
    [CX_TAG_PORT_TABLE] = {
        .name  = "port_table",
        .descr = "",
        .type = CX_TAG_TYPE_GROUP
    },
    [CX_TAG_PVLAN_TABLE] = {
        .name  = "pvlan_table",
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

static BOOL cx_pvlan_match(const cx_table_context_t *context, ulong port_a, ulong port_b)
{
    BOOL member[VTSS_PORT_ARRAY_SIZE];

    return (pvlan_mgmt_isolate_conf_get(context->isid, member) == VTSS_OK &&
            member[port_a] == member[port_b]);
}

static vtss_rc cx_pvlan_print(cx_get_state_t *s, const cx_table_context_t *context, ulong port_no, char *ports)
{
    BOOL member[VTSS_PORT_ARRAY_SIZE];

    if (ports == NULL) {
        /* Syntax */
        CX_RC(cx_add_stx_start(s));
        CX_RC(cx_add_stx_port(s));
        CX_RC(cx_add_stx_bool(s, "isolate"));
        return cx_add_stx_end(s);
    }

    CX_RC(pvlan_mgmt_isolate_conf_get(context->isid, member));
    CX_RC(cx_add_port_start(s, CX_TAG_ENTRY, ports));
    CX_RC(cx_add_attr_bool(s, "isolate", member[port_no]));
    return cx_add_port_end(s, CX_TAG_ENTRY);
}

static vtss_rc pvlan_cx_parse_func(cx_set_state_t *s)
{
    switch (s->cmd) {
    case CX_PARSE_CMD_PARM: {
        BOOL           port_list[VTSS_PORT_ARRAY_SIZE + 1];
#if defined(PVLAN_SRC_MASK_ENA)
        ulong          val;
#endif /* PVLAN_SRC_MASK_ENA */
        BOOL           global;
        port_iter_t    pit;

        global = (s->isid == VTSS_ISID_GLOBAL);
        if (global) {
            s->ignored = 1;
            break;
        }

        switch (s->id) {
        case CX_TAG_PORT_TABLE:
            break;
        case CX_TAG_PVLAN_TABLE:
#if defined(PVLAN_SRC_MASK_ENA)
            if (s->apply) {
                /* Flush PVLAN table */
                pvlan_mgmt_entry_t conf;
                BOOL    first = 1;

                conf.privatevid = 0;
                while ((first && pvlan_mgmt_pvlan_get(s->isid, conf.privatevid, &conf, 0) == VTSS_OK) ||
                       (pvlan_mgmt_pvlan_get(s->isid, conf.privatevid, &conf, 1) == VTSS_OK)) {
                    first = 0;
                    memset(conf.ports, 0, sizeof(conf.ports));
                    CX_RC(pvlan_mgmt_pvlan_add(s->isid, &conf));
                }
            }
#endif /* PVLAN_SRC_MASK_ENA */
            break;
        case CX_TAG_ENTRY:
            if (s->group == CX_TAG_PORT_TABLE && cx_parse_ports(s, port_list, 1) == VTSS_OK) {
                s->p = s->next;
                BOOL member[VTSS_PORT_ARRAY_SIZE], isolate;
                if (cx_parse_bool(s, "isolate", &isolate, 1) != VTSS_OK) {
                    CX_RC(cx_parm_found_error(s, "isolate"));
                }

                if (s->apply) {
                    if (pvlan_mgmt_isolate_conf_get(s->isid, member) != VTSS_OK) {
                        break;
                    }
                    (void)port_iter_init(&pit, NULL, s->isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
                    while (port_iter_getnext(&pit)) {
                        if (port_list[pit.uport]) {
                            member[pit.iport] = isolate;
                        }
                    }
                    CX_RC(pvlan_mgmt_isolate_conf_set(s->isid, member));
                }
            } else if (s->group == CX_TAG_PVLAN_TABLE) {
#if defined(PVLAN_SRC_MASK_ENA)
                pvlan_mgmt_entry_t conf;

                if (cx_parse_ulong(s, "pvlan_id", &val, 1, VTSS_PVLANS) != VTSS_OK) {
                    cx_parm_found_error(s, "pvlan_id");
                } else {
                    conf.privatevid = val - 1;
                    s->p = s->next;
                    if (cx_parse_ports(s, port_list, 1) == VTSS_OK && s->apply) {
                        (void)port_iter_init(&pit, NULL, s->isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
                        while (port_iter_getnext(&pit)) {
                            conf.ports[pit.iport] = port_list[pit.uport];
                        }
                        CX_RC(pvlan_mgmt_pvlan_add(s->isid, &conf));
                    }
                }
#endif /* PVLAN_SRC_MASK_ENA */
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

static vtss_rc pvlan_cx_gen_func(cx_get_state_t *s)
{
#if defined(PVLAN_SRC_MASK_ENA)
    char buf[MGMT_PORT_BUF_SIZE];
#endif /* PVLAN_SRC_MASK_ENA */

    switch (s->cmd) {
    case CX_GEN_CMD_GLOBAL:
        break;
    case CX_GEN_CMD_SWITCH:
        /* Switch - PVLAN */
        T_D("switch - pvlan");
        CX_RC(cx_add_tag_line(s, CX_TAG_PVLAN, 0));
        CX_RC(cx_add_port_table(s, s->isid, CX_TAG_PORT_TABLE, cx_pvlan_match, cx_pvlan_print));
#if defined(PVLAN_SRC_MASK_ENA)
        {
            pvlan_mgmt_entry_t conf;
            BOOL    first = 1;

            CX_RC(cx_add_tag_line(s, CX_TAG_PVLAN_TABLE, 0));

            /* Entry syntax */
            CX_RC(cx_add_stx_start(s));
            CX_RC(cx_add_stx_ulong(s, "pvlan_id", 1, VTSS_PVLANS));
            CX_RC(cx_add_stx_port(s));
            CX_RC(cx_add_stx_end(s));

            conf.privatevid = 0;
            while ((first && pvlan_mgmt_pvlan_get(s->isid, conf.privatevid, &conf, 0) == VTSS_OK) ||
                   (pvlan_mgmt_pvlan_get(s->isid, conf.privatevid, &conf, 1) == VTSS_OK)) {
                first = 0;
                CX_RC(cx_add_attr_start(s, CX_TAG_ENTRY));
                CX_RC(cx_add_attr_ulong(s, "pvlan_id", (conf.privatevid + 1)));
                CX_RC(cx_add_attr_txt(s, "port", mgmt_iport_list2txt(conf.ports, buf)));
                CX_RC(cx_add_attr_end(s, CX_TAG_ENTRY));
            }
            CX_RC(cx_add_tag_line(s, CX_TAG_PVLAN_TABLE, 1));
        }
#endif /* PVLAN_SRC_MASK_ENA */

        CX_RC(cx_add_tag_line(s, CX_TAG_PVLAN, 1));
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
    VTSS_MODULE_ID_PVLAN,
    pvlan_cx_tag_table,
    0,
    0,
    NULL,                /* init function       */
    pvlan_cx_gen_func,     /* Generation fucntion */
    pvlan_cx_parse_func    /* parse fucntion      */
);

