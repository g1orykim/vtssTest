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
#include "ip_source_guard_api.h"
#include "conf_xml_api.h"
#include "conf_xml_trace_def.h"
#include "misc_api.h"
#include "mgmt_api.h"
#include "port_api.h"
#include "topo_api.h"
#include "vlan_api.h"

/* Tag IDs */
enum {
    /* Module tags */
    CX_TAG_IP_SOURCE_GUARD,

    /* Group tags */
    CX_TAG_PORT_TABLE,
    CX_TAG_IP_SOURCE_GUARD_TABLE,

    /* Parameter tags */
    CX_TAG_ENTRY,
    CX_TAG_MODE,

    /* Last entry */
    CX_TAG_NONE
};

/* Tag table */
static cx_tag_entry_t ip_guard_cx_tag_table[CX_TAG_NONE + 1] = {
    [CX_TAG_IP_SOURCE_GUARD] = {
        .name  = "ip_guard",
        .descr = "IP Source Guard",
        .type = CX_TAG_TYPE_MODULE
    },
    [CX_TAG_PORT_TABLE] = {
        .name  = "port_table",
        .descr = "",
        .type = CX_TAG_TYPE_GROUP
    },
    [CX_TAG_IP_SOURCE_GUARD_TABLE] = {
        .name  = "ip_source_guard_table",
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

/* Keyword for max dynamic clients */
static const cx_kw_t cx_kw_ip_source_guard_dyna_entry_cnt[] = {
    { "0",          0 },
    { "1",          1 },
    { "2",          2 },
    { "Unlimited",  IP_SOURCE_GUARD_DYNAMIC_UNLIMITED },
    { NULL,         0 }
};

/* Specific set state structure */
typedef struct {
    int entry_count;
} ip_source_guard_cx_set_state_t;

static BOOL cx_ip_source_guard_match(const cx_table_context_t *context, ulong port_a, ulong port_b)
{
    ip_source_guard_port_mode_conf_t            ip_source_guard_port_mode;
    ip_source_guard_port_dynamic_entry_conf_t   ip_source_guard_dynamic_entry;

    return (ip_source_guard_mgmt_conf_get_port_mode(context->isid, &ip_source_guard_port_mode) == VTSS_OK &&
            ip_source_guard_mgmt_conf_get_port_dynamic_entry_cnt(context->isid, &ip_source_guard_dynamic_entry) == VTSS_OK &&
            ip_source_guard_port_mode.mode[port_a] == ip_source_guard_port_mode.mode[port_b] &&
            ip_source_guard_dynamic_entry.entry_cnt[port_a] == ip_source_guard_dynamic_entry.entry_cnt[port_b]);
}

static vtss_rc cx_ip_source_guard_print(cx_get_state_t *s, const cx_table_context_t *context, ulong port_no, char *ports)
{
    ip_source_guard_port_mode_conf_t            ip_source_guard_port_mode;
    ip_source_guard_port_dynamic_entry_conf_t   ip_source_guard_dynamic_entry;

    if (ports == NULL) {
        /* Syntax */
        CX_RC(cx_add_stx_start(s));
        CX_RC(cx_add_stx_port(s));
        CX_RC(cx_add_stx_bool(s, "port_mode"));
        CX_RC(cx_add_stx_kw(s, "entry_cnt", cx_kw_ip_source_guard_dyna_entry_cnt));
        return cx_add_stx_end(s);
    }

    CX_RC(ip_source_guard_mgmt_conf_get_port_mode(context->isid, &ip_source_guard_port_mode));
    CX_RC(ip_source_guard_mgmt_conf_get_port_dynamic_entry_cnt(context->isid, &ip_source_guard_dynamic_entry));
    CX_RC(cx_add_port_start(s, CX_TAG_ENTRY, ports));
    CX_RC(cx_add_attr_bool(s, "port_mode", ip_source_guard_port_mode.mode[port_no]));
    CX_RC(cx_add_attr_kw(s, "entry_cnt", cx_kw_ip_source_guard_dyna_entry_cnt, ip_source_guard_dynamic_entry.entry_cnt[port_no]));
    return cx_add_port_end(s, CX_TAG_ENTRY);
}

static vtss_rc ip_source_guard_cx_parse_func(cx_set_state_t *s)
{
    switch (s->cmd) {
    case CX_PARSE_CMD_PARM: {
        BOOL                            port_list[VTSS_PORT_ARRAY_SIZE + 1];
        BOOL                            global;
        BOOL                            mode_temp;
        ip_source_guard_entry_t         entry;
        ulong                           mode;
        long                            sid_temp, vid_temp;
        ip_source_guard_cx_set_state_t  *ip_source_guard_state = s->mod_state;
        u32                             port_count;

        global = (s->isid == VTSS_ISID_GLOBAL);
        memset(&entry, 0, sizeof(ip_source_guard_entry_t));

        switch (s->id) {
        case CX_TAG_MODE:

            if (global && cx_parse_val_bool(s, &mode_temp, 1) == VTSS_OK) {
                mode = mode_temp;
                if (s->apply) {
                    CX_RC(ip_source_guard_mgmt_conf_set_mode(mode));
                }
            } else {
                s->ignored = 1;
            }

            break;
        case CX_TAG_IP_SOURCE_GUARD_TABLE:
            if (global) {
                ip_source_guard_state->entry_count = 0;
                if (s->apply) {
                    CX_RC(ip_source_guard_mgmt_conf_del_all_static_entry());
                }
            } else {
                s->ignored = 1;
            }
            break;
        case CX_TAG_PORT_TABLE:
            break;
        case CX_TAG_ENTRY:
            port_count = s->port_count;
            if (s->group == CX_TAG_IP_SOURCE_GUARD_TABLE) {
                for (; s->rc == VTSS_OK && cx_parse_attr(s) == VTSS_OK; s->p = s->next) {
                    if (cx_parse_long(s, "sid", &sid_temp, VTSS_USID_START, VTSS_USID_END) == VTSS_OK) {
                        entry.isid = (vtss_isid_t) sid_temp;
                        entry.isid = topo_usid2isid(entry.isid);
                    } else if (cx_parse_ulong(s, "port", &entry.port_no, 1, port_count) == VTSS_OK) {
                        entry.port_no = uport2iport(entry.port_no);
                    } else if (cx_parse_long(s, "vid", &vid_temp, VLAN_ID_MIN, VLAN_ID_MAX) == VTSS_OK) {
                        entry.vid = (vtss_vid_t) vid_temp;
                    } else if (cx_parse_ipv4(s, "ip_addr", &entry.assigned_ip, NULL, 0) == VTSS_OK) {
                        ;
#if defined(VTSS_FEATURE_ACL_V2)
                    } else if (cx_parse_mac(s, "mac_addr", entry.assigned_mac) == VTSS_OK) {
#else
                    } else if (cx_parse_ipv4(s, "ip_mask", &entry.ip_mask, NULL, 1) == VTSS_OK) {
                        ;
#endif /* VTSS_FEATURE_ACL_V2 */
                    } else {
                        CX_RC(cx_parm_unknown(s));
                    }
                }

                ip_source_guard_state->entry_count++;
                if (ip_source_guard_state->entry_count > IP_SOURCE_GUARD_MAX_ENTRY_CNT) {
                    sprintf(s->msg, "The maximum entry count is %d", IP_SOURCE_GUARD_MAX_ENTRY_CNT);
                    s->rc = CONF_XML_ERROR_FILE_PARM;
                    break;
                }

                if (s->apply) {
                    entry.type = IP_SOURCE_GUARD_STATIC_TYPE;
                    entry.valid = 1;
                    CX_RC(ip_source_guard_mgmt_conf_set_static_entry(&entry));
                }
            } else if (!global && s->group == CX_TAG_PORT_TABLE &&
                       cx_parse_ports(s, port_list, 1) == VTSS_OK) {
                ip_source_guard_port_mode_conf_t            ip_source_guard_port_mode;
                ip_source_guard_port_dynamic_entry_conf_t   ip_source_guard_dynamic_entry;
                ulong                                       m_found = 0, d_found = 0, entry_cnt = IP_SOURCE_GUARD_DYNAMIC_UNLIMITED;
                BOOL                                        port_mode;

                s->p = s->next;
                for (; s->rc == VTSS_OK && cx_parse_attr(s) == VTSS_OK; s->p = s->next) {
                    if (cx_parse_bool(s, "port_mode", &port_mode, 1) == VTSS_OK) {
                        m_found = 1;
                    } else if (cx_parse_kw(s, "entry_cnt", cx_kw_ip_source_guard_dyna_entry_cnt, &entry_cnt, 1) == VTSS_OK) {
                        d_found = 1;
                    } else {
                        CX_RC(cx_parm_unknown(s));
                    }
                }
                if (s->apply &&
                    ip_source_guard_mgmt_conf_get_port_mode(s->isid, &ip_source_guard_port_mode) == VTSS_OK &&
                    ip_source_guard_mgmt_conf_get_port_dynamic_entry_cnt(s->isid, &ip_source_guard_dynamic_entry) == VTSS_OK) {

                    port_iter_t     pit;

                    (void) port_iter_init(&pit, NULL, s->isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
                    while (port_iter_getnext(&pit)) {
                        if (port_list[iport2uport(pit.iport)]) {
                            if (m_found) {
                                ip_source_guard_port_mode.mode[pit.iport] = port_mode;
                            }
                            if (d_found) {
                                ip_source_guard_dynamic_entry.entry_cnt[pit.iport] = entry_cnt;
                            }
                        }
                    }
                    CX_RC(ip_source_guard_mgmt_conf_set_port_mode(s->isid, &ip_source_guard_port_mode));
                    CX_RC(ip_source_guard_mgmt_conf_set_port_dynamic_entry_cnt(s->isid, &ip_source_guard_dynamic_entry));
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

static vtss_rc ip_source_guard_cx_gen_func(cx_get_state_t *s)
{
#if defined(VTSS_FEATURE_ACL_V2)
    char           buf[128];
#endif /* VTSS_FEATURE_ACL_V2 */

    switch (s->cmd) {
    case CX_GEN_CMD_GLOBAL:
        /* Global - IP Source Guard */
        T_D("global - ip source guard");
        CX_RC(cx_add_tag_line(s, CX_TAG_IP_SOURCE_GUARD, 0));
        {
            ip_source_guard_entry_t entry;
            ulong                   mode;

            CX_RC(ip_source_guard_mgmt_conf_get_mode(&mode));
            CX_RC(cx_add_val_bool(s, CX_TAG_MODE, mode));
            CX_RC(cx_add_tag_line(s, CX_TAG_IP_SOURCE_GUARD_TABLE, 0));

            /* Entry syntax */
            CX_RC(cx_add_stx_start(s));
            CX_RC(cx_add_stx_ulong(s, "sid", VTSS_USID_START, VTSS_USID_END - 1));
            CX_RC(cx_add_stx_ulong(s, "port", 1, s->port_count));
            CX_RC(cx_add_stx_ulong(s, "vid", VLAN_ID_MIN, VLAN_ID_MAX));
            CX_RC(cx_add_stx_ipv4(s, "ip_addr"));
#if defined(VTSS_FEATURE_ACL_V2)
            CX_RC(cx_add_attr_txt(s, "mac_addr", "'xx-xx-xx-xx-xx-xx' or 'xx.xx.xx.xx.xx.xx' or 'xxxxxxxxxxxx' (x is a hexadecimal digit)"));
#else
            CX_RC(cx_add_stx_ipv4(s, "ip_mask"));
#endif /* VTSS_FEATURE_ACL_V2 */
            CX_RC(cx_add_stx_end(s));

            if (ip_source_guard_mgmt_conf_get_first_static_entry(&entry) == VTSS_OK) {
                CX_RC(cx_add_attr_start(s, CX_TAG_ENTRY));
                CX_RC(cx_add_attr_ulong(s, "sid", topo_isid2usid(entry.isid)));
                CX_RC(cx_add_attr_ulong(s, "port", iport2uport(entry.port_no)));
                CX_RC(cx_add_attr_ulong(s, "vid", entry.vid));
                CX_RC(cx_add_attr_ipv4(s, "ip_addr", entry.assigned_ip));
#if defined(VTSS_FEATURE_ACL_V2)
                CX_RC(cx_add_attr_txt(s, "mac_addr", misc_mac_txt(entry.assigned_mac, buf)));
#else
                CX_RC(cx_add_attr_ipv4(s, "ip_mask", entry.ip_mask));
#endif /* VTSS_FEATURE_ACL_V2 */
                CX_RC(cx_add_attr_end(s, CX_TAG_ENTRY));
                while (ip_source_guard_mgmt_conf_get_next_static_entry(&entry) == VTSS_OK) {
                    CX_RC(cx_add_attr_start(s, CX_TAG_ENTRY));
                    CX_RC(cx_add_attr_ulong(s, "sid", topo_isid2usid(entry.isid)));
                    CX_RC(cx_add_attr_ulong(s, "port", iport2uport(entry.port_no)));
                    CX_RC(cx_add_attr_ulong(s, "vid", entry.vid));
                    CX_RC(cx_add_attr_ipv4(s, "ip_addr", entry.assigned_ip));
#if defined(VTSS_FEATURE_ACL_V2)
                    CX_RC(cx_add_attr_txt(s, "mac_addr", misc_mac_txt(entry.assigned_mac, buf)));
#else
                    CX_RC(cx_add_attr_ipv4(s, "ip_mask", entry.ip_mask));
#endif /* VTSS_FEATURE_ACL_V2 */
                    CX_RC(cx_add_attr_end(s, CX_TAG_ENTRY));
                }
            }

            CX_RC(cx_add_tag_line(s, CX_TAG_IP_SOURCE_GUARD_TABLE, 1));
        }
        CX_RC(cx_add_tag_line(s, CX_TAG_IP_SOURCE_GUARD, 1));
        break;
    case CX_GEN_CMD_SWITCH:
        /* Switch - IP Source Guard */
        T_D("switch - ip_source_guard");
        CX_RC(cx_add_tag_line(s, CX_TAG_IP_SOURCE_GUARD, 0));
        CX_RC(cx_add_port_table(s, s->isid, CX_TAG_PORT_TABLE,
                                cx_ip_source_guard_match,
                                cx_ip_source_guard_print));
        CX_RC(cx_add_tag_line(s, CX_TAG_IP_SOURCE_GUARD, 1));
        break;
    default:
        T_E("Unknown command");
        return VTSS_RC_ERROR;
    } /* End of Switch */

    return VTSS_OK;
}
/* Register the info in to the cx_module_table */
CX_MODULE_TAB_ENTRY(VTSS_MODULE_ID_IP_SOURCE_GUARD, ip_guard_cx_tag_table,
                    sizeof(ip_source_guard_cx_set_state_t), 0,
                    NULL,                   /* init function       */
                    ip_source_guard_cx_gen_func,       /* Generation fucntion */
                    ip_source_guard_cx_parse_func);    /* parse fucntion      */
