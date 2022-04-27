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
#include "port_api.h"
#include "dhcp_snooping_api.h"
#include "conf_xml_api.h"
#include "conf_xml_trace_def.h"
#include "misc_api.h"

/* Tag IDs */
enum {
    /* Module tags */
    CX_TAG_DHCP_SNOOPING,

    /* Group tags */
    CX_TAG_PORT_TABLE,

    /* Parameter tags */
    CX_TAG_ENTRY,
    CX_TAG_MODE,

    /* Last entry */
    CX_TAG_NONE
};

/* Tag table */
static cx_tag_entry_t dhcp_snooping_cx_tag_table[CX_TAG_NONE + 1] = {
    [CX_TAG_DHCP_SNOOPING] = {
        .name  = "dhcp_snooping",
        .descr = "DHCP Snooping",
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

    /* Last entry */
    [CX_TAG_NONE] = {
        .name  = "",
        .descr = "",
        .type = CX_TAG_TYPE_NONE
    }
};

/* Keyword for DHCP snooping mode */
static const cx_kw_t cx_kw_dhcp_snooping_port_mode[] = {
    { "trusted",    DHCP_SNOOPING_PORT_MODE_TRUSTED },
    { "untrusted",  DHCP_SNOOPING_PORT_MODE_UNTRUSTED },
    { NULL,         0 }
};

static BOOL cx_dhcp_snooping_match(const cx_table_context_t *context, ulong port_a, ulong port_b)
{
    dhcp_snooping_port_conf_t conf;

    CX_RC(dhcp_snooping_mgmt_port_conf_get(context->isid, &conf));
    return (conf.port_mode[port_a] == conf.port_mode[port_b]);
}

static vtss_rc cx_dhcp_snooping_print(cx_get_state_t *s, const cx_table_context_t *context, ulong port_no, char *ports)
{
    dhcp_snooping_port_conf_t conf;

    if (ports == NULL) {
        /* Syntax */
        CX_RC(cx_add_stx_start(s));
        CX_RC(cx_add_stx_port(s));
        CX_RC(cx_add_stx_kw(s, "mode", cx_kw_dhcp_snooping_port_mode));
        return cx_add_stx_end(s);
    }

    CX_RC(dhcp_snooping_mgmt_port_conf_get(context->isid, &conf));
    CX_RC(cx_add_port_start(s, CX_TAG_ENTRY, ports));
    CX_RC(cx_add_attr_kw(s, "mode", cx_kw_dhcp_snooping_port_mode, conf.port_mode[port_no]));
    return cx_add_port_end(s, CX_TAG_ENTRY);
}

static vtss_rc dhcp_snooping_cx_parse_func(cx_set_state_t *s)
{
    switch (s->cmd) {
    case CX_PARSE_CMD_PARM: {
        port_iter_t    pit;
        dhcp_snooping_conf_t    conf;
        BOOL                    port_list[VTSS_PORT_ARRAY_SIZE + 1];
        BOOL                    global;
        BOOL                    mode;

        global = (s->isid == VTSS_ISID_GLOBAL);
        switch (s->id) {
        case CX_TAG_MODE:
            if (!global) {
                s->ignored = 1;
                break;
            }
            CX_RC(cx_parse_val_bool(s, &mode, 1));
            if (s->apply && dhcp_snooping_mgmt_conf_get(&conf) == VTSS_OK) {
                conf.snooping_mode = mode;
                CX_RC(dhcp_snooping_mgmt_conf_set(&conf));
            }
            break;
        case CX_TAG_PORT_TABLE:
            if (global) {
                s->ignored = 1;
            }
            break;
        case CX_TAG_ENTRY:
            if (global) {
                s->ignored = 1;
                break;
            }
            if (s->group == CX_TAG_PORT_TABLE &&
                cx_parse_ports(s, port_list, 1) == VTSS_OK) {
                /* Handle port table in switch section */
                dhcp_snooping_port_conf_t   port_conf;
                ulong                       port_mode;

                s->p = s->next;
                for (; s->rc == VTSS_OK && cx_parse_attr(s) == VTSS_OK; s->p = s->next) {
                    if (!cx_parse_kw(s, "mode", cx_kw_dhcp_snooping_port_mode, &port_mode, 1) == VTSS_OK) {
                        CX_RC(cx_parm_unknown(s));
                    }
                } /* for loop */

                if (s->apply && dhcp_snooping_mgmt_port_conf_get(s->isid, &port_conf) == VTSS_OK) {
                    (void) port_iter_init(&pit, NULL, s->isid, PORT_ITER_SORT_ORDER_UPORT, PORT_ITER_FLAGS_NORMAL);
                    while (port_iter_getnext(&pit)) {
                        if (!port_list[pit.uport]) {
                            continue;
                        }
                        port_conf.port_mode[pit.iport] = port_mode;
                    }
                    CX_RC(dhcp_snooping_mgmt_port_conf_set(s->isid, &port_conf));
                }
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

static vtss_rc dhcp_snooping_cx_gen_func(cx_get_state_t *s)
{
    switch (s->cmd) {
    case CX_GEN_CMD_GLOBAL:
        /* Global - dhcp_snooping */
        T_D("global - dhcp_snooping");
        CX_RC(cx_add_tag_line(s, CX_TAG_DHCP_SNOOPING, 0));
        {
            dhcp_snooping_conf_t conf;

            if (dhcp_snooping_mgmt_conf_get(&conf) == VTSS_OK) {
                CX_RC(cx_add_val_bool(s, CX_TAG_MODE, conf.snooping_mode));
            }
        }
        CX_RC(cx_add_tag_line(s, CX_TAG_DHCP_SNOOPING, 1));
        break;
    case CX_GEN_CMD_SWITCH:
        /* Switch - DHCP Snooping */
        T_D("switch - dhcp_snooping");
        CX_RC(cx_add_tag_line(s, CX_TAG_DHCP_SNOOPING, 0));
        CX_RC(cx_add_port_table(s, s->isid, CX_TAG_PORT_TABLE, cx_dhcp_snooping_match, cx_dhcp_snooping_print));
        CX_RC(cx_add_tag_line(s, CX_TAG_DHCP_SNOOPING, 1));

        break;
    default:
        T_E("Unknown command");
        return VTSS_RC_ERROR;
    } /* End of Switch */

    return VTSS_OK;
}
/* Register the info in to the cx_module_table */
CX_MODULE_TAB_ENTRY(VTSS_MODULE_ID_DHCP_SNOOPING,
                    dhcp_snooping_cx_tag_table,
                    0, 0,
                    NULL,                            /* init function       */
                    dhcp_snooping_cx_gen_func,       /* Generation fucntion */
                    dhcp_snooping_cx_parse_func);    /* parse fucntion      */
