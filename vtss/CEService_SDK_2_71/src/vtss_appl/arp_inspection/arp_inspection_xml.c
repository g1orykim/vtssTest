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
#include "arp_inspection_api.h"
#include "misc_api.h"
#include "mgmt_api.h"
#include "port_api.h"
#include "topo_api.h"
#include "vlan_api.h"

static const cx_kw_t cx_kw_log_type[] = {
    { "none",       ARP_INSPECTION_LOG_NONE },
    { "deny",       ARP_INSPECTION_LOG_DENY },
    { "permit",     ARP_INSPECTION_LOG_PERMIT },
    { "all",        ARP_INSPECTION_LOG_ALL },
    { NULL,       0 }
};

/* Tag IDs */
enum {
    /* Module tags */
    CX_TAG_ARP_INSPECTION,

    /* Group tags */
    CX_TAG_PORT_TABLE,
    CX_TAG_PORT_VLAN_TABLE,
    CX_TAG_PORT_LOG_TABLE,
    CX_TAG_ARP_INSPECTION_TABLE,
    CX_TAG_ARP_INSPECTION_VLAN_TABLE,

    /* Parameter tags */
    CX_TAG_ENTRY,
    CX_TAG_MODE,

    /* Last entry */
    CX_TAG_NONE
};

/* Tag table */
static cx_tag_entry_t arp_inspection_cx_tag_table[CX_TAG_NONE + 1] = {
    [CX_TAG_ARP_INSPECTION] = {
        .name  = "arp_inspection",
        .descr = "Arp Inspection",
        .type = CX_TAG_TYPE_MODULE
    },
    [CX_TAG_PORT_TABLE] = {
        .name  = "port_table",
        .descr = "",
        .type = CX_TAG_TYPE_GROUP
    },
    [CX_TAG_PORT_VLAN_TABLE] = {
        .name  = "port_vlan_table",
        .descr = "",
        .type = CX_TAG_TYPE_GROUP
    },
    [CX_TAG_PORT_LOG_TABLE] = {
        .name  = "port_log_table",
        .descr = "",
        .type = CX_TAG_TYPE_GROUP
    },
    [CX_TAG_ARP_INSPECTION_TABLE] = {
        .name  = "arp_inspection_table",
        .descr = "",
        .type = CX_TAG_TYPE_GROUP
    },
    [CX_TAG_ARP_INSPECTION_VLAN_TABLE] = {
        .name  = "arp_inspection_vlan_table",
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

/* Specific set state structure */
typedef struct {
    int entry_count;
} arp_inspection_cx_set_state_t;

/* Add log type attribute */
vtss_rc cx_add_attr_log_type(cx_get_state_t *s, const char *name, arp_inspection_log_type_t type)
{
    return cx_add_attr_kw(s, name, cx_kw_log_type, type);
}

/* Parse log type parameter */
vtss_rc cx_parse_log_type(cx_set_state_t *s, const char *name, arp_inspection_log_type_t *val, BOOL force)
{
    ulong value;

    CX_RC(cx_parse_kw(s, name, cx_kw_log_type, &value, force));
    *val = value;
    T_D("name: %s, val: %d", name, *val);
    return VTSS_OK;
}

/* Parse log type parameter for VLAN */
static arp_inspection_log_type_t cx_parse_log_type_for_vlan(u8 flags)
{
    if ((flags & ARP_INSPECTION_VLAN_LOG_DENY) && (flags & ARP_INSPECTION_VLAN_LOG_PERMIT)) {
        return ARP_INSPECTION_LOG_ALL;
    } else if (flags & ARP_INSPECTION_VLAN_LOG_DENY) {
        return ARP_INSPECTION_LOG_DENY;
    } else if (flags & ARP_INSPECTION_VLAN_LOG_PERMIT) {
        return ARP_INSPECTION_LOG_PERMIT;
    } else {
        return ARP_INSPECTION_LOG_NONE;
    }
}

/* Add log type syntax */
vtss_rc cx_add_stx_log_type(cx_get_state_t *s, const char *name)
{
    return cx_add_stx_kw(s, name, cx_kw_log_type);
}

static BOOL cx_arp_inspection_match(const cx_table_context_t *context, ulong port_a, ulong port_b)
{
    arp_inspection_port_mode_conf_t     arp_inspection_port_mode;

    return (arp_inspection_mgmt_conf_port_mode_get(context->isid, &arp_inspection_port_mode) == VTSS_OK &&
            arp_inspection_port_mode.mode[port_a] == arp_inspection_port_mode.mode[port_b]);
}

static BOOL cx_arp_inspection_match_vlan_mode(const cx_table_context_t *context, ulong port_a, ulong port_b)
{
    arp_inspection_port_mode_conf_t     arp_inspection_port_mode;

    return (arp_inspection_mgmt_conf_port_mode_get(context->isid, &arp_inspection_port_mode) == VTSS_OK &&
            arp_inspection_port_mode.check_VLAN[port_a] == arp_inspection_port_mode.check_VLAN[port_b]);
}

static BOOL cx_arp_inspection_match_port_log(const cx_table_context_t *context, ulong port_a, ulong port_b)
{
    arp_inspection_port_mode_conf_t     arp_inspection_port_mode;

    return (arp_inspection_mgmt_conf_port_mode_get(context->isid, &arp_inspection_port_mode) == VTSS_OK &&
            arp_inspection_port_mode.log_type[port_a] == arp_inspection_port_mode.log_type[port_b]);
}

static vtss_rc cx_arp_inspection_print(cx_get_state_t *s, const cx_table_context_t *context, ulong port_no, char *ports)
{
    arp_inspection_port_mode_conf_t arp_inspection_port_mode;

    if (ports == NULL) {
        /* Syntax */
        CX_RC(cx_add_stx_start(s));
        CX_RC(cx_add_stx_port(s));
        CX_RC(cx_add_stx_bool(s, "port_mode"));
        return cx_add_stx_end(s);
    }

    CX_RC(arp_inspection_mgmt_conf_port_mode_get(context->isid, &arp_inspection_port_mode));
    CX_RC(cx_add_port_start(s, CX_TAG_ENTRY, ports));
    CX_RC(cx_add_attr_bool(s, "port_mode", arp_inspection_port_mode.mode[port_no]));
    return cx_add_port_end(s, CX_TAG_ENTRY);
}

static vtss_rc cx_arp_inspection_print_vlan_mode(cx_get_state_t *s, const cx_table_context_t *context, ulong port_no, char *ports)
{
    arp_inspection_port_mode_conf_t arp_inspection_port_mode;

    if (ports == NULL) {
        /* Syntax */
        CX_RC(cx_add_stx_start(s));
        CX_RC(cx_add_stx_port(s));
        CX_RC(cx_add_stx_bool(s, "check_vlan"));
        return cx_add_stx_end(s);
    }

    CX_RC(arp_inspection_mgmt_conf_port_mode_get(context->isid, &arp_inspection_port_mode));
    CX_RC(cx_add_port_start(s, CX_TAG_ENTRY, ports));
    CX_RC(cx_add_attr_bool(s, "check_vlan", arp_inspection_port_mode.check_VLAN[port_no]));
    return cx_add_port_end(s, CX_TAG_ENTRY);
}

static vtss_rc cx_arp_inspection_print_port_log(cx_get_state_t *s, const cx_table_context_t *context, ulong port_no, char *ports)
{
    arp_inspection_port_mode_conf_t arp_inspection_port_mode;

    if (ports == NULL) {
        /* Syntax */
        CX_RC(cx_add_stx_start(s));
        CX_RC(cx_add_stx_port(s));
        CX_RC(cx_add_stx_log_type(s, "log_type"));
        return cx_add_stx_end(s);
    }

    CX_RC(arp_inspection_mgmt_conf_port_mode_get(context->isid, &arp_inspection_port_mode));
    CX_RC(cx_add_port_start(s, CX_TAG_ENTRY, ports));
    CX_RC(cx_add_attr_log_type(s, "log_type", arp_inspection_port_mode.log_type[port_no]));
    return cx_add_port_end(s, CX_TAG_ENTRY);
}

static vtss_rc arp_inspection_cx_parse_func(cx_set_state_t *s)
{
    switch (s->cmd) {
    case CX_PARSE_CMD_PARM: {
        BOOL                            port_list[VTSS_PORT_ARRAY_SIZE + 1];
        BOOL                            mode_temp;
        BOOL                            global;
        BOOL                            vlan_state = TRUE;
        arp_inspection_entry_t          entry;
        arp_inspection_cx_set_state_t   *arp_inspection_state = s->mod_state;
        arp_inspection_vlan_mode_conf_t vlan_mode_conf;
        arp_inspection_log_type_t       log_type = 0;
        u32                             mode;
        u32                             port_count;
        long                            sid_temp, vid_temp;

        global = (s->isid == VTSS_ISID_GLOBAL);
        memset(&entry, 0, sizeof(arp_inspection_entry_t));

        switch (s->id) {
        case CX_TAG_MODE:

            if (global && cx_parse_val_bool(s, &mode_temp, 1) == VTSS_OK) {
                mode = mode_temp;
                if (s->apply) {
                    CX_RC(arp_inspection_mgmt_conf_mode_set(&mode));
                }
            } else {
                s->ignored = 1;
            }

            break;
        case CX_TAG_ARP_INSPECTION_TABLE:
            if (global) {
                arp_inspection_state->entry_count = 0;
                if (s->apply) {
                    CX_RC(arp_inspection_mgmt_conf_all_static_entry_del());
                }
            } else {
                s->ignored = 1;
            }
            break;
        case CX_TAG_ARP_INSPECTION_VLAN_TABLE:
            if (global) {
                if (s->apply) {
                    // set all entries to disabled mode
                    CX_RC(arp_inspection_mgmt_conf_vlan_entry_del());
                }
            } else {
                s->ignored = 1;
            }
            break;
        case CX_TAG_PORT_TABLE:
        case CX_TAG_PORT_VLAN_TABLE:
        case CX_TAG_PORT_LOG_TABLE:
            break;
        case CX_TAG_ENTRY:
            port_count = s->port_count;
            if (s->group == CX_TAG_ARP_INSPECTION_TABLE) {
                for (; s->rc == VTSS_OK && cx_parse_attr(s) == VTSS_OK; s->p = s->next) {
                    if (cx_parse_long(s, "sid", &sid_temp, VTSS_USID_START, VTSS_USID_END - 1) == VTSS_OK) {
                        entry.isid = (vtss_isid_t) sid_temp;
                        entry.isid = topo_usid2isid(entry.isid);
                    } else if (cx_parse_ulong(s, "port", &entry.port_no, 1, port_count) == VTSS_OK) {
                        entry.port_no = uport2iport(entry.port_no);
                    } else if (cx_parse_long(s, "vid", &vid_temp, VLAN_ID_MIN, VLAN_ID_MAX) == VTSS_OK) {
                        entry.vid = (vtss_vid_t) vid_temp;
                    } else if (cx_parse_mac(s, "mac_addr",
                                            entry.mac) == VTSS_OK) {
                        ;

                    } else if (cx_parse_ipv4(s, "ip_addr", &entry.assigned_ip, NULL, 0) == VTSS_OK) {
                        ;
                    } else {
                        CX_RC(cx_parm_unknown(s));
                    }
                }

                arp_inspection_state->entry_count++;
                if (arp_inspection_state->entry_count > ARP_INSPECTION_MAX_ENTRY_CNT) {
                    sprintf(s->msg, "The maximum entry count is %d", ARP_INSPECTION_MAX_ENTRY_CNT);
                    s->rc = CONF_XML_ERROR_FILE_PARM;
                    break;
                }

                if (s->apply) {
                    entry.type = ARP_INSPECTION_STATIC_TYPE;
                    entry.valid = TRUE;
                    CX_RC(arp_inspection_mgmt_conf_static_entry_set(&entry));
                }
            } else if (s->group == CX_TAG_ARP_INSPECTION_VLAN_TABLE) {
                for (; s->rc == VTSS_OK && cx_parse_attr(s) == VTSS_OK; s->p = s->next) {
                    if (cx_parse_long(s, "vid", &vid_temp, VLAN_ID_MIN, VLAN_ID_MAX) == VTSS_OK) {
                    } else if (cx_parse_bool(s, "state", &vlan_state, 1) == VTSS_OK) {
                    } else if (cx_parse_log_type(s, "log_type", &log_type, 1) == VTSS_OK) {
                    } else {
                        CX_RC(cx_parm_unknown(s));
                    }
                }

                if (s->apply) {
                    // init the value
                    vlan_mode_conf.flags = 0;

                    if (vlan_state) {
                        vlan_mode_conf.flags |= ARP_INSPECTION_VLAN_MODE;
                    } else {
                        vlan_mode_conf.flags &= ~ARP_INSPECTION_VLAN_MODE;
                    }

                    switch (log_type) {
                    case ARP_INSPECTION_LOG_NONE:
                        vlan_mode_conf.flags &= ~ARP_INSPECTION_VLAN_LOG_DENY;
                        vlan_mode_conf.flags &= ~ARP_INSPECTION_VLAN_LOG_PERMIT;
                        break;
                    case ARP_INSPECTION_LOG_DENY:
                        vlan_mode_conf.flags |= ARP_INSPECTION_VLAN_LOG_DENY;
                        vlan_mode_conf.flags &= ~ARP_INSPECTION_VLAN_LOG_PERMIT;
                        break;
                    case ARP_INSPECTION_LOG_PERMIT:
                        vlan_mode_conf.flags &= ~ARP_INSPECTION_VLAN_LOG_DENY;
                        vlan_mode_conf.flags |= ARP_INSPECTION_VLAN_LOG_PERMIT;
                        break;
                    case ARP_INSPECTION_LOG_ALL:
                        vlan_mode_conf.flags |= ARP_INSPECTION_VLAN_LOG_DENY;
                        vlan_mode_conf.flags |= ARP_INSPECTION_VLAN_LOG_PERMIT;
                        break;
                    default:
                        vlan_mode_conf.flags &= ~ARP_INSPECTION_VLAN_LOG_DENY;
                        vlan_mode_conf.flags &= ~ARP_INSPECTION_VLAN_LOG_PERMIT;
                        break;
                    }

                    CX_RC(arp_inspection_mgmt_conf_vlan_mode_set(vid_temp, &vlan_mode_conf));
                    CX_RC(arp_inspection_mgmt_conf_vlan_mode_save());
                }
            } else if (s->group == CX_TAG_PORT_TABLE &&
                       cx_parse_ports(s, port_list, 1) == VTSS_OK) {
                /* Handle port table in switch section */
                arp_inspection_port_mode_conf_t arp_inspection_port_mode;
                BOOL                            port_mode;

                s->p = s->next;
                for (; s->rc == VTSS_OK && cx_parse_attr(s) == VTSS_OK; s->p = s->next) {
                    if (!cx_parse_bool(s, "port_mode", &port_mode, 1) == VTSS_OK) {
                        CX_RC(cx_parm_unknown(s));
                    }
                } /* for loop */

                if (s->apply && arp_inspection_mgmt_conf_port_mode_get(s->isid, &arp_inspection_port_mode) == VTSS_OK) {

                    port_iter_t     pit;

                    (void) port_iter_init(&pit, NULL, s->isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
                    while (port_iter_getnext(&pit)) {
                        if (!port_list[iport2uport(pit.iport)]) {
                            continue;
                        }
                        arp_inspection_port_mode.mode[pit.iport] = port_mode;
                    }
                    CX_RC(arp_inspection_mgmt_conf_port_mode_set(s->isid, &arp_inspection_port_mode));
                }
            } else if (s->group == CX_TAG_PORT_VLAN_TABLE &&
                       cx_parse_ports(s, port_list, 1) == VTSS_OK) {
                /* Handle port table in switch section */
                arp_inspection_port_mode_conf_t arp_inspection_port_mode;
                BOOL                            port_mode;

                s->p = s->next;
                for (; s->rc == VTSS_OK && cx_parse_attr(s) == VTSS_OK; s->p = s->next) {
                    if (!cx_parse_bool(s, "check_vlan", &port_mode, 1) == VTSS_OK) {
                        CX_RC(cx_parm_unknown(s));
                    }
                } /* for loop */

                if (s->apply && arp_inspection_mgmt_conf_port_mode_get(s->isid, &arp_inspection_port_mode) == VTSS_OK) {

                    port_iter_t     pit;

                    (void) port_iter_init(&pit, NULL, s->isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
                    while (port_iter_getnext(&pit)) {
                        if (!port_list[iport2uport(pit.iport)]) {
                            continue;
                        }
                        arp_inspection_port_mode.check_VLAN[pit.iport] = port_mode;
                    }
                    CX_RC(arp_inspection_mgmt_conf_port_mode_set(s->isid, &arp_inspection_port_mode));
                }
            } else if (s->group == CX_TAG_PORT_LOG_TABLE &&
                       cx_parse_ports(s, port_list, 1) == VTSS_OK) {
                /* Handle port table in switch section */
                arp_inspection_port_mode_conf_t arp_inspection_port_mode;
                //arp_inspection_log_type_t       log_type;

                s->p = s->next;
                for (; s->rc == VTSS_OK && cx_parse_attr(s) == VTSS_OK; s->p = s->next) {
                    if (!cx_parse_log_type(s, "log_type", &log_type, 1) == VTSS_OK) {
                        CX_RC(cx_parm_unknown(s));
                    }
                } /* for loop */

                if (s->apply && arp_inspection_mgmt_conf_port_mode_get(s->isid, &arp_inspection_port_mode) == VTSS_OK) {

                    port_iter_t     pit;

                    (void) port_iter_init(&pit, NULL, s->isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
                    while (port_iter_getnext(&pit)) {
                        if (!port_list[iport2uport(pit.iport)]) {
                            continue;
                        }
                        arp_inspection_port_mode.log_type[pit.iport] = log_type;
                    }
                    CX_RC(arp_inspection_mgmt_conf_port_mode_set(s->isid, &arp_inspection_port_mode));
                }
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

static vtss_rc arp_inspection_cx_gen_func(cx_get_state_t *s)
{
    char           buf[128];

    switch (s->cmd) {
    case CX_GEN_CMD_GLOBAL:
        /* Global - ARP INSPECTION */
        T_D("global - arp inspection");
        CX_RC(cx_add_tag_line(s, CX_TAG_ARP_INSPECTION, 0));
        {
            arp_inspection_entry_t          entry;
            arp_inspection_vlan_mode_conf_t vlan_mode_conf;
            u32                             mode, idx;

            CX_RC(arp_inspection_mgmt_conf_mode_get(&mode));
            CX_RC(cx_add_val_bool(s, CX_TAG_MODE, mode));

            CX_RC(cx_add_tag_line(s, CX_TAG_ARP_INSPECTION_TABLE, 0));

            /* Entry syntax */
            CX_RC(cx_add_stx_start(s));
            CX_RC(cx_add_stx_ulong(s, "sid", VTSS_USID_START, VTSS_USID_END - 1));
            CX_RC(cx_add_stx_ulong(s, "port", 1, s->port_count));
            CX_RC(cx_add_stx_ulong(s, "vid", VLAN_ID_MIN, VLAN_ID_MAX));
            CX_RC(cx_add_attr_txt(s, "mac_addr", "'xx-xx-xx-xx-xx-xx' or 'xx.xx.xx.xx.xx.xx' or 'xxxxxxxxxxxx' (x is a hexadecimal digit))"));
            CX_RC(cx_add_stx_ipv4(s, "ip_addr"));
            CX_RC(cx_add_stx_end(s));

            if (arp_inspection_mgmt_conf_static_entry_get(&entry, FALSE) == VTSS_OK) {
                CX_RC(cx_add_attr_start(s, CX_TAG_ENTRY));
                CX_RC(cx_add_attr_ulong(s, "sid", topo_isid2usid(entry.isid)));
                CX_RC(cx_add_attr_ulong(s, "port", iport2uport(entry.port_no)));
                CX_RC(cx_add_attr_ulong(s, "vid", entry.vid));
                CX_RC(cx_add_attr_txt(s, "mac_addr", misc_mac_txt(entry.mac, buf)));
                CX_RC(cx_add_attr_ipv4(s, "ip_addr", entry.assigned_ip));
                CX_RC(cx_add_attr_end(s, CX_TAG_ENTRY));
                while (arp_inspection_mgmt_conf_static_entry_get(&entry, TRUE) == VTSS_OK) {
                    CX_RC(cx_add_attr_start(s, CX_TAG_ENTRY));
                    CX_RC(cx_add_attr_ulong(s, "sid", topo_isid2usid(entry.isid)));
                    CX_RC(cx_add_attr_ulong(s, "port", iport2uport(entry.port_no)));
                    CX_RC(cx_add_attr_ulong(s, "vid", entry.vid));
                    CX_RC(cx_add_attr_txt(s, "mac_addr", misc_mac_txt(entry.mac, buf)));
                    CX_RC(cx_add_attr_ipv4(s, "ip_addr", entry.assigned_ip));
                    CX_RC(cx_add_attr_end(s, CX_TAG_ENTRY));
                }
            }
            CX_RC(cx_add_tag_line(s, CX_TAG_ARP_INSPECTION_TABLE, 1));

            CX_RC(cx_add_tag_line(s, CX_TAG_ARP_INSPECTION_VLAN_TABLE, 0));

            /* Entry syntax */
            CX_RC(cx_add_stx_start(s));
            CX_RC(cx_add_stx_ulong(s, "vid", VLAN_ID_MIN, VLAN_ID_MAX));
            CX_RC(cx_add_stx_bool(s, "state"));
            CX_RC(cx_add_stx_log_type(s, "log_type"));
            CX_RC(cx_add_stx_end(s));

            for (idx = VLAN_ID_MIN; idx <= VLAN_ID_MAX; idx++) {
                // get configuration
                if (arp_inspection_mgmt_conf_vlan_mode_get(idx, &vlan_mode_conf, FALSE) != VTSS_OK) {
                    continue;
                }

                if (vlan_mode_conf.flags & ARP_INSPECTION_VLAN_MODE) {
                    CX_RC(cx_add_attr_start(s, CX_TAG_ENTRY));
                    CX_RC(cx_add_attr_ulong(s, "vid", idx));
                    CX_RC(cx_add_attr_bool(s, "state", vlan_mode_conf.flags & ARP_INSPECTION_VLAN_MODE));
                    CX_RC(cx_add_attr_log_type(s, "log_type", cx_parse_log_type_for_vlan(vlan_mode_conf.flags)));
                    CX_RC(cx_add_attr_end(s, CX_TAG_ENTRY));
                }
            }

            CX_RC(cx_add_tag_line(s, CX_TAG_ARP_INSPECTION_VLAN_TABLE, 1));
        }
        CX_RC(cx_add_tag_line(s, CX_TAG_ARP_INSPECTION, 1));
        break;
    case CX_GEN_CMD_SWITCH:
        /* Switch - ARP Inspection */
        T_D("switch - arp_inspection");
        CX_RC(cx_add_tag_line(s, CX_TAG_ARP_INSPECTION, 0));
        CX_RC(cx_add_port_table(s, s->isid, CX_TAG_PORT_TABLE, cx_arp_inspection_match, cx_arp_inspection_print));
        CX_RC(cx_add_port_table(s, s->isid, CX_TAG_PORT_VLAN_TABLE, cx_arp_inspection_match_vlan_mode, cx_arp_inspection_print_vlan_mode));
        CX_RC(cx_add_port_table(s, s->isid, CX_TAG_PORT_LOG_TABLE, cx_arp_inspection_match_port_log, cx_arp_inspection_print_port_log));
        CX_RC(cx_add_tag_line(s, CX_TAG_ARP_INSPECTION, 1));
        break;
    default:
        T_E("Unknown command");
        return VTSS_RC_ERROR;
    } /* End of Switch */

    return VTSS_OK;
}

/* Register the info in to the cx_module_table */
CX_MODULE_TAB_ENTRY(
    VTSS_MODULE_ID_ARP_INSPECTION,
    arp_inspection_cx_tag_table,
    sizeof(arp_inspection_cx_set_state_t),
    0,
    NULL,                             /* init function       */
    arp_inspection_cx_gen_func,       /* Generation fucntion */
    arp_inspection_cx_parse_func      /* parse fucntion      */
);

