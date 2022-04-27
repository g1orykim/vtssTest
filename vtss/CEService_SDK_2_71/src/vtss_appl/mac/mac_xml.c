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
#include "mac_api.h"

#include "conf_xml_api.h"
#include "conf_xml_trace_def.h"
#include "misc_api.h"
#include "mgmt_api.h"
#include "port_api.h"
#include "vlan_api.h"

/* Tag IDs */
enum {
    /* Module tags */
    CX_TAG_MAC,

    /* Group tags */
    CX_TAG_PORT_TABLE,
    CX_TAG_MAC_TABLE,

    /* Parameter tags */
    CX_TAG_ENTRY,
    CX_TAG_AGE,

    /* Last entry */
    CX_TAG_NONE
};

/* Tag table */
static cx_tag_entry_t mac_cx_tag_table[CX_TAG_NONE + 1] = {
    [CX_TAG_MAC] = {
        .name  = "mac",
        .descr = "MAC address table",
        .type = CX_TAG_TYPE_MODULE
    },
    [CX_TAG_PORT_TABLE] = {
        .name  = "port_table",
        .descr = "",
        .type = CX_TAG_TYPE_GROUP
    },
    [CX_TAG_MAC_TABLE] = {
        .name  = "mac_table",
        .descr = "",
        .type = CX_TAG_TYPE_GROUP
    },
    [CX_TAG_ENTRY] = {
        .name  = "entry",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_AGE] = {
        .name  = "age",
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

/* Keyword for learn mode (automatic, discard) */
static const cx_kw_t cx_kw_learn[] = {
    { "auto",     CX_DUAL_VAL(1, 0) },
    { "secure",   CX_DUAL_VAL(0, 1) },
    { "disabled", CX_DUAL_VAL(0, 0) },
    { NULL,     0 }
};

static BOOL cx_mac_match(const cx_table_context_t *context, ulong port_a, ulong port_b)
{
    vtss_learn_mode_t mode_a, mode_b;
    BOOL              dummy_bool;

    mac_mgmt_learn_mode_get(context->isid, port_a, &mode_a, &dummy_bool);
    mac_mgmt_learn_mode_get(context->isid, port_b, &mode_b, &dummy_bool);
    return (memcmp(&mode_a, &mode_b, sizeof(mode_a)) == 0);
}

static vtss_rc cx_mac_print(cx_get_state_t *s, const cx_table_context_t *context, ulong port_no, char *ports)
{
    vtss_learn_mode_t mode;
    BOOL              dummy_bool;

    if (ports == NULL) {
        /* Syntax */
        cx_add_stx_start(s);
        cx_add_stx_port(s);
        cx_add_stx_kw(s, "learn_mode", cx_kw_learn);
        return cx_add_stx_end(s);
    }

    mac_mgmt_learn_mode_get(context->isid, port_no, &mode, &dummy_bool);
    cx_add_port_start(s, CX_TAG_ENTRY, ports);
    cx_add_attr_kw(s, "learn_mode", cx_kw_learn, CX_DUAL_VAL(mode.automatic, mode.discard));
    return cx_add_port_end(s, CX_TAG_ENTRY);
}

static vtss_rc mac_cx_parse_func(cx_set_state_t *s)
{
    switch (s->cmd) {
    case CX_PARSE_CMD_PARM: {
        vtss_port_no_t port_idx;
        BOOL           port_list[VTSS_PORT_ARRAY_SIZE + 1];
        ulong          val;
        BOOL           global;

        global = (s->isid == VTSS_ISID_GLOBAL);

        switch (s->id) {
        case CX_TAG_AGE:
            if (global) {
                if (cx_parse_val_ulong(s, &val, 0, MAC_AGE_TIME_MAX) == VTSS_OK) {
                    if (val != 0 && val < MAC_AGE_TIME_MIN) {
                        cx_parm_invalid(s);
                    } else if (s->apply) {
                        mac_age_conf_t conf;
                        conf.mac_age_time = val;
                        mac_mgmt_age_time_set(&conf);
                    }
                } else {
                    sprintf(s->msg, "Parameter 'val' must be a integer in range "vtss_xstr(MAC_AGE_TIME_MIN)"-"vtss_xstr(MAC_AGE_TIME_MAX)" or 0 (disabled)");
                }
            } else {
                s->ignored = 1;
            }
            break;
        case CX_TAG_PORT_TABLE:
            if (global) {
                s->ignored = 1;
            }
            break;
        case CX_TAG_MAC_TABLE:
            if (global) {
                s->ignored = 1;
            } else if (s->apply) {
                /* Flush MAC table */
                mac_mgmt_addr_entry_t mac_entry;
                vtss_vid_mac_t        vid_mac;

                memset(&vid_mac, 0, sizeof(vid_mac));
                while (mac_mgmt_static_get_next(s->isid,
                                                &vid_mac, &mac_entry, 1, 0) == VTSS_OK) {
                    mac_mgmt_table_del(s->isid, &mac_entry.vid_mac, 0);
                }
            }
            break;
        case CX_TAG_ENTRY:
            if (!global && s->group == CX_TAG_PORT_TABLE &&
                cx_parse_ports(s, port_list, 1) == VTSS_OK) {
                s->p = s->next;
                if (cx_parse_kw(s, "learn_mode", cx_kw_learn, &val, 1) == VTSS_OK) {
                    vtss_learn_mode_t mode;

                    mode.cpu = 0;
                    mode.automatic = CX_DUAL_V1(val);
                    mode.discard = CX_DUAL_V2(val);
                    for (port_idx = VTSS_PORT_NO_START; port_idx < VTSS_PORT_NO_END; port_idx++)
                        if (s->apply && port_list[iport2uport(port_idx)]) {
                            mac_mgmt_learn_mode_set(s->isid, port_idx, &mode);
                        }
                } else {
                    cx_parm_unknown(s);
                }
            } else if (!global && s->group == CX_TAG_MAC_TABLE) {
                mac_mgmt_addr_entry_t mac_entry;
                BOOL                  mac = 0, port = 0;

                memset(&mac_entry, 0, sizeof(mac_entry));
                mac_entry.vid_mac.vid = VLAN_ID_DEFAULT;
                for (; s->rc == VTSS_OK && cx_parse_attr(s) == VTSS_OK; s->p = s->next) {
                    if (cx_parse_mac(s, "mac_addr",
                                     mac_entry.vid_mac.mac.addr) == VTSS_OK) {
                        mac = 1;
                    } else if (cx_parse_ports(s, port_list, 0) == VTSS_OK) {
                        port = 1;
                    } else if (cx_parse_vid(s, &mac_entry.vid_mac.vid, 0) != VTSS_OK) {
                        cx_parm_unknown(s);
                    }
                }
                if (s->apply && mac && port) {
                    for (port_idx = VTSS_PORT_NO_START; port_idx < VTSS_PORT_NO_END; port_idx++) {
                        mac_entry.destination[port_idx] = port_list[iport2uport(port_idx)];
                    }
                    mac_mgmt_table_add(s->isid, &mac_entry);
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

static vtss_rc mac_cx_gen_func(cx_get_state_t *s)
{
    char buf[MGMT_PORT_BUF_SIZE];

    switch (s->cmd) {
    case CX_GEN_CMD_GLOBAL:
        /* Global - MAC */
        T_D("global - mac");
        cx_add_tag_line(s, CX_TAG_MAC, 0);
        {
            mac_age_conf_t conf;

            if (mac_mgmt_age_time_get(&conf) == VTSS_OK) {
                sprintf(buf, "%u-%u or 0 (disabled)", MAC_AGE_TIME_MIN, MAC_AGE_TIME_MAX);
                cx_add_val_ulong_stx(s, CX_TAG_AGE, conf.mac_age_time, buf);
            }
        }
        CX_RC(cx_add_tag_line(s, CX_TAG_MAC, 1));
        break;
    case CX_GEN_CMD_SWITCH:
        /* Switch - MAC */
        T_D("switch - mac");
        cx_add_tag_line(s, CX_TAG_MAC, 0);
        CX_RC(cx_add_port_table(s, s->isid, CX_TAG_PORT_TABLE, cx_mac_match, cx_mac_print));
        {
            mac_mgmt_addr_entry_t mac_entry;
            vtss_vid_mac_t        vid_mac;

            cx_add_tag_line(s, CX_TAG_MAC_TABLE, 0);

            /* Entry syntax */
            cx_add_stx_start(s);
            cx_add_stx_ulong(s, "vid", VLAN_ID_MIN, VLAN_ID_MAX);
            cx_add_attr_txt(s, "mac_addr", "'xx-xx-xx-xx-xx-xx' or 'xx.xx.xx.xx.xx.xx' or 'xxxxxxxxxxxx' (x is a hexadecimal digit)");
            cx_add_stx_port(s);
            cx_add_stx_end(s);

            memset(&vid_mac, 0, sizeof(vid_mac));
            while (mac_mgmt_static_get_next(s->isid, &vid_mac, &mac_entry, 1, 0) == VTSS_OK) {
                vid_mac = mac_entry.vid_mac;
#if VTSS_SWITCH_STACKABLE
                mac_entry.destination[PORT_NO_STACK_0] = 0;
                mac_entry.destination[PORT_NO_STACK_1] = 0;
#endif
                cx_add_attr_start(s, CX_TAG_ENTRY);
                cx_add_attr_ulong(s, "vid", vid_mac.vid);
                cx_add_attr_txt(s, "mac_addr", misc_mac_txt(vid_mac.mac.addr, buf));
                cx_add_attr_txt(s, "port", mgmt_iport_list2txt(mac_entry.destination, buf));
                CX_RC(cx_add_attr_end(s, CX_TAG_ENTRY));
            }
            cx_add_tag_line(s, CX_TAG_MAC_TABLE, 1);
        }
        CX_RC(cx_add_tag_line(s, CX_TAG_MAC, 1));
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
    VTSS_MODULE_ID_MAC,
    mac_cx_tag_table,
    0,
    0,
    NULL,                  /* init function       */
    mac_cx_gen_func,       /* Generation fucntion */
    mac_cx_parse_func      /* parse fucntion      */
);

