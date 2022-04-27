/*

 Vitesse Switch API software.

 Copyright (c) 2002-2012 Vitesse Semiconductor Corporation "Vitesse". All
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
#include "misc_api.h"
#include "port_api.h"   /* For VTSS_FRONT_PORT_COUNT */
#include "mvr.h"
#include "mvr_api.h"


/* Tag IDs */
enum {
    /* Module tags */
    CX_TAG_MVR,

    /* Group tags */
    CX_TAG_VLAN_TABLE,
    CX_TAG_PORT_TABLE,

    /* Parameter tags */
    CX_TAG_MODE,
    CX_TAG_ENTRY,

    /* Last entry */
    CX_TAG_NONE
};

/* Tag table */
static cx_tag_entry_t mvr_cx_tag_table[CX_TAG_NONE + 1] = {
    [CX_TAG_MVR] = {
        .name  = "mvr",
        .descr = "Multicast VLAN Registration",
        .type = CX_TAG_TYPE_MODULE
    },
    [CX_TAG_VLAN_TABLE] = {
        .name  = "vlan_table",
        .descr = "",
        .type = CX_TAG_TYPE_GROUP
    },
    [CX_TAG_PORT_TABLE] = {
        .name  = "port_table",
        .descr = "",
        .type = CX_TAG_TYPE_GROUP
    },
    [CX_TAG_MODE] = {
        .name  = "mode",
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

/* Keyword for MVR VLAN mode */
static const cx_kw_t cx_kw_mvr_vlan_mode[] = {
    { "dynamic",    MVR_INTF_MODE_DYNA },
    { "compatible", MVR_INTF_MODE_COMP },
    { NULL,       0 }
};

/* Keyword for MVR VLAN Tag */
static const cx_kw_t cx_kw_mvr_vlan_vtag[] = {
    { "untagged",   IPMC_INTF_UNTAG },
    { "tagged",     IPMC_INTF_TAGED },
    { NULL,       0 }
};

/* Keyword for MVR port mode */
static const cx_kw_t cx_kw_mvr_port_role[] = {
    { "inactive",   MVR_PORT_ROLE_INACT },
    { "source",     MVR_PORT_ROLE_SOURC },
    { "receiver",   MVR_PORT_ROLE_RECVR },
    { NULL,       0 }
};

static BOOL cx_mvr_port_roles_match(const cx_table_context_t *context, ulong port_a, ulong port_b)
{
    mvr_mgmt_interface_t    mvrif;
    mvr_conf_port_role_t    *q = &mvrif.role;

    mvrif.vid = *(u16 *)context->custom;
    return ((mvr_mgmt_get_intf_entry(context->isid, &mvrif) == VTSS_OK) &&
            (q->ports[port_a] == q->ports[port_b]));
}

static vtss_rc cx_mvr_port_roles_print(cx_get_state_t *s, const cx_table_context_t *context, ulong port_no, char *ports)
{
    mvr_mgmt_interface_t    mvrif;
    mvr_conf_port_role_t    *q;
    u16                     vid;

    if (ports == NULL) {
        /* Syntax */
        CX_RC(cx_add_stx_start(s));
        CX_RC(cx_add_stx_port(s));
        CX_RC(cx_add_stx_ulong(s, "mvid", MVR_VID_MIN, MVR_VID_MAX));
        CX_RC(cx_add_stx_kw(s, "role", cx_kw_mvr_port_role));
        return cx_add_stx_end(s);
    }

    mvrif.vid = vid = *(u16 *)context->custom;
    CX_RC(mvr_mgmt_get_intf_entry(context->isid, &mvrif));

    q = &mvrif.role;
    CX_RC(cx_add_port_start(s, CX_TAG_ENTRY, ports));
    CX_RC(cx_add_attr_ulong(s, "mvid", vid));
    CX_RC(cx_add_attr_kw(s, "role", cx_kw_mvr_port_role, q->ports[port_no]));
    return cx_add_port_end(s, CX_TAG_ENTRY);
}

static BOOL cx_mvr_fast_leave_match(const cx_table_context_t *context, ulong port_a, ulong port_b)
{
    mvr_conf_fast_leave_t   fast_leave;

    return ((mvr_mgmt_get_fast_leave_port(context->isid, &fast_leave) == VTSS_OK) &&
            (VTSS_PORT_BF_GET(fast_leave.ports, port_a) == VTSS_PORT_BF_GET(fast_leave.ports, port_b)));
}

static vtss_rc cx_mvr_fast_leave_print(cx_get_state_t *s, const cx_table_context_t *context, ulong port_no, char *ports)
{
    mvr_conf_fast_leave_t   fast_leave;

    if (ports == NULL) {
        /* Syntax */
        CX_RC(cx_add_stx_start(s));
        CX_RC(cx_add_stx_port(s));
        CX_RC(cx_add_stx_bool(s, "immediate"));
        return cx_add_stx_end(s);
    }

    CX_RC(mvr_mgmt_get_fast_leave_port(context->isid, &fast_leave));

    CX_RC(cx_add_port_start(s, CX_TAG_ENTRY, ports));
    CX_RC(cx_add_attr_bool(s, "immediate", VTSS_PORT_BF_GET(fast_leave.ports, port_no)));
    return cx_add_port_end(s, CX_TAG_ENTRY);
}

static vtss_rc cx_parse_mvr_name(cx_set_state_t *s, char *name, char *mvr_name)
{
    u32     name_len, idx;
    BOOL    valid_flag;

    CX_RC(cx_parse_txt(s, name, mvr_name, MVR_NAME_MAX_LEN - 1));

    if ((name_len = strlen(mvr_name)) != 0) {
        valid_flag = FALSE;
        for (idx = 0; idx < name_len; idx++) {
            if ((mvr_name[idx] < 48) || (mvr_name[idx] > 122)) {
                valid_flag = FALSE;
                break;
            } else {
                if (((mvr_name[idx] > 64) && (mvr_name[idx] < 91)) ||
                    (mvr_name[idx] > 96)) {
                    valid_flag = TRUE;
                } else {
                    if (mvr_name[idx] > 57) {
                        valid_flag = FALSE;
                        break;
                    }
                }
            }
        }
    } else {
        valid_flag = TRUE;
    }

    if (!valid_flag) {
        CX_RC(cx_parm_invalid(s));
    }

    return s->rc;
}

static vtss_rc mvr_cx_parse_func(cx_set_state_t *s)
{
    port_iter_t             pit;
    u32                     val;
    BOOL                    global, state, port_list[VTSS_PORT_ARRAY_SIZE + 1];
    mvr_mgmt_interface_t    mvrif;
    mvr_conf_intf_entry_t   *p;
    mvr_conf_port_role_t    *q;
    mvr_conf_fast_leave_t   fastleave;

    switch ( s->cmd ) {
    case CX_PARSE_CMD_PARM:
        global = (s->isid == VTSS_ISID_GLOBAL);

        switch ( s->id ) {
        case CX_TAG_MODE:
            if (global) {
                if (s->apply && (cx_parse_val_bool(s, &state, TRUE) == VTSS_OK)) {
                    CX_RC(mvr_mgmt_set_mode(&state));
                }
            } else {
                s->ignored = TRUE;
            }

            break;
        case CX_TAG_VLAN_TABLE:
            if (global) {
                if (s->apply) {
                    memset(&mvrif, 0x0, sizeof(mvr_mgmt_interface_t));
                    while (mvr_mgmt_get_next_intf_entry(s->isid, &mvrif)  == VTSS_OK) {
                        CX_RC(mvr_mgmt_set_intf_entry(VTSS_ISID_GLOBAL, IPMC_OP_DEL, &mvrif));
                    }
                }
            } else {
                s->ignored = TRUE;
            }

            break;
        case CX_TAG_PORT_TABLE:
            if (global) {
                s->ignored = TRUE;
            }

            break;
        case CX_TAG_ENTRY:
            if (global && (cx_parse_ulong(s, "vid", &val, MVR_VID_MIN, MVR_VID_MAX) == VTSS_OK)) {
                if (s->group == CX_TAG_VLAN_TABLE) {
                    i8                      pf_name[VTSS_IPMC_NAME_MAX_LEN];
                    u32                     priority = MVR_CONF_DEF_INTF_PRIO;
                    u32                     mode = MVR_CONF_DEF_INTF_MODE;
                    u32                     vtag = MVR_CONF_DEF_INTF_VTAG;
                    ipmc_operation_action_t action = IPMC_OP_SET;

                    memset(pf_name, 0x0, sizeof(pf_name));
                    memset(&mvrif, 0x0, sizeof(mvr_mgmt_interface_t));
                    mvrif.vid = (u16)(val & 0xFFFF);
                    p = &mvrif.intf;
                    if (mvr_mgmt_get_intf_entry(s->isid, &mvrif)  == VTSS_OK) {
                        action = IPMC_OP_UPD;
                    } else {
                        action = IPMC_OP_ADD;
                    }

                    s->p = s->next;
                    for (; (s->rc == VTSS_OK) && (cx_parse_attr(s) == VTSS_OK); s->p = s->next) {
                        if ((cx_parse_mvr_name(s, "name", p->name) != VTSS_OK) &&
                            (cx_parse_ipv4(s, "igmp_address", &p->querier4_address, NULL, FALSE) != VTSS_OK) &&
                            (cx_parse_kw(s, "mode", cx_kw_mvr_vlan_mode, &mode, TRUE) != VTSS_OK) &&
                            (cx_parse_kw(s, "vtag", cx_kw_mvr_vlan_vtag, &vtag, TRUE) != VTSS_OK) &&
                            (cx_parse_ulong(s, "priority", &priority, 0, 7) != VTSS_OK) &&
                            (cx_parse_ulong(s, "llqi", &p->last_listener_query_interval, 0, 31744) != VTSS_OK) &&
                            (cx_parse_mvr_name(s, "profile", pf_name) != VTSS_OK)) {
                            CX_RC(cx_parm_unknown(s));
                        }
                    }

                    if (s->apply) {
                        u32                     pdx;
                        int                     str_len = strlen(pf_name);
                        ipmc_lib_profile_mem_t  *pfm;

                        pdx = 0;
                        if (str_len && IPMC_MEM_PROFILE_MTAKE(pfm)) {
                            ipmc_lib_grp_fltr_profile_t *fltr_profile;

                            fltr_profile = &pfm->profile;
                            memset(fltr_profile->data.name, 0x0, sizeof(fltr_profile->data.name));
                            strncpy(fltr_profile->data.name, pf_name, str_len);
                            if (ipmc_lib_mgmt_fltr_profile_get(fltr_profile, TRUE) == VTSS_OK) {
                                pdx = fltr_profile->data.index;
                            }
                            IPMC_MEM_PROFILE_MGIVE(pfm);
                        }

                        p->mode = mode;
                        p->vtag = vtag;
                        p->priority = (u8)(priority & 0xFF);
                        p->profile_index = pdx;
                        CX_RC(mvr_mgmt_set_intf_entry(VTSS_ISID_GLOBAL, action, &mvrif));
                    }
                }
            } else if (!global && (s->group == CX_TAG_PORT_TABLE) &&
                       (cx_parse_ports(s, port_list, TRUE) == VTSS_OK)) {
                u32     role = MVR_CONF_DEF_PORT_ROLE;
                BOOL    flag, config_role = FALSE, config_fast = FALSE;

                val = 0x0;
                s->p = s->next;
                for (; (s->rc == VTSS_OK) && (cx_parse_attr(s) == VTSS_OK); s->p = s->next) {
                    if (cx_parse_bool(s, "immediate", &state, TRUE) == VTSS_OK) {
                        config_fast = TRUE;
                    }

                    if (cx_parse_ulong(s, "mvid", &val, MVR_VID_MIN, MVR_VID_MAX) == VTSS_OK) {
                        config_role = TRUE;
                    }

                    if (cx_parse_kw(s, "role", cx_kw_mvr_port_role, &role, TRUE) == VTSS_OK) {
                        config_role = TRUE;
                    }
                }

                if (config_role && (val == 0x0)) {
                    config_role = FALSE;
                }

                if (!config_role && !config_fast) {
                    CX_RC(cx_parm_unknown(s));
                }

                if (s->apply) {
                    if (config_role) {
                        mvrif.vid = val;
                        if (mvr_mgmt_get_intf_entry(s->isid, &mvrif)  == VTSS_OK) {
                            q = &mvrif.role;
                            flag = FALSE;

                            (void) port_iter_init(&pit, NULL, s->isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
                            while (port_iter_getnext(&pit)) {
                                if (port_list[pit.uport]) {
                                    flag = TRUE;
                                    q->ports[pit.iport] = role;
                                }
                            }

                            if (flag) {
                                CX_RC(mvr_mgmt_set_intf_entry(s->isid, IPMC_OP_SET, &mvrif));
                            }
                        }
                    }

                    if (config_fast && (mvr_mgmt_get_fast_leave_port(s->isid, &fastleave) == VTSS_OK )) {
                        flag = FALSE;

                        (void) port_iter_init(&pit, NULL, s->isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
                        while (port_iter_getnext(&pit)) {
                            if (port_list[pit.uport]) {
                                flag = TRUE;
                                VTSS_PORT_BF_SET(fastleave.ports, pit.iport, state);
                            }
                        }

                        if (flag) {
                            CX_RC(mvr_mgmt_set_fast_leave_port(s->isid, &fastleave));
                        }
                    }
                }
            } else {
                s->ignored = TRUE;
            }

            break;
        default:
            s->ignored = TRUE;

            break;
        }

        break;
    case CX_PARSE_CMD_GLOBAL:

        break;
    case CX_PARSE_CMD_SWITCH:

        break;
    default:

        break;
    }

    return s->rc;
}

static vtss_rc mvr_cx_gen_func(cx_get_state_t *s)
{
    BOOL                        state;
    mvr_mgmt_interface_t        mvrif;
    mvr_conf_intf_entry_t       *p;
    ipmc_lib_profile_mem_t      *pfm;
    ipmc_lib_grp_fltr_profile_t *fltr_profile;
    i8                          profile_name[VTSS_IPMC_NAME_MAX_LEN];

    switch ( s->cmd ) {
    case CX_GEN_CMD_GLOBAL:
        /* Global - MVR */
        T_D("global - mvr");
        CX_RC(cx_add_tag_line(s, CX_TAG_MVR, 0));
        if (mvr_mgmt_get_mode(&state) == VTSS_OK) {
            CX_RC(cx_add_val_bool(s, CX_TAG_MODE, state));
        }

        CX_RC(cx_add_tag_line(s, CX_TAG_VLAN_TABLE, 0));
        CX_RC(cx_add_stx_start(s));
        CX_RC(cx_add_stx_ulong(s, "vid", MVR_VID_MIN, MVR_VID_MAX));
        CX_RC(cx_add_attr_txt(s, "name", "MVR VLAN Name"));
        CX_RC(cx_add_attr_txt(s, "igmp_address", "IPv4 Unicast Address"));
        CX_RC(cx_add_stx_kw(s, "mode", cx_kw_mvr_vlan_mode));
        CX_RC(cx_add_stx_kw(s, "vtag", cx_kw_mvr_vlan_vtag));
        CX_RC(cx_add_stx_ulong(s, "priority", 0, 7));
        CX_RC(cx_add_stx_ulong(s, "llqi", 0, 31744));
        CX_RC(cx_add_attr_txt(s, "profile", "Name of associated profile used for channel"));
        CX_RC(cx_add_stx_end(s));
        memset(&mvrif, 0x0, sizeof(mvr_mgmt_interface_t));
        while (mvr_mgmt_get_next_intf_entry(VTSS_ISID_GLOBAL, &mvrif) == VTSS_OK) {
            p = &mvrif.intf;

            CX_RC(cx_add_attr_start(s, CX_TAG_ENTRY));
            CX_RC(cx_add_attr_ulong(s, "vid", p->vid));
            CX_RC(cx_add_attr_txt(s, "name", p->name));
            CX_RC(cx_add_attr_ipv4(s, "igmp_address", p->querier4_address));
            CX_RC(cx_add_attr_kw(s, "mode", cx_kw_mvr_vlan_mode, p->mode));
            CX_RC(cx_add_attr_kw(s, "vtag", cx_kw_mvr_vlan_vtag, p->vtag));
            CX_RC(cx_add_attr_ulong(s, "priority", p->priority));
            CX_RC(cx_add_attr_ulong(s, "llqi", p->last_listener_query_interval));
            memset(profile_name, 0x0, sizeof(profile_name));
            if (p->profile_index && IPMC_MEM_PROFILE_MTAKE(pfm)) {
                fltr_profile = &pfm->profile;
                fltr_profile->data.index = p->profile_index;
                if ((ipmc_lib_mgmt_fltr_profile_get(fltr_profile, FALSE) == VTSS_OK) &&
                    strlen(fltr_profile->data.name)) {
                    memcpy(profile_name, fltr_profile->data.name, sizeof(profile_name));
                }

                IPMC_MEM_PROFILE_MGIVE(pfm);
            }
            CX_RC(cx_add_attr_txt(s, "profile", profile_name));
            CX_RC(cx_add_attr_end(s, CX_TAG_ENTRY));

        }
        CX_RC(cx_add_tag_line(s, CX_TAG_VLAN_TABLE, 1));

        CX_RC(cx_add_tag_line(s, CX_TAG_MVR, 1));

        break;
    case CX_GEN_CMD_SWITCH:
        /* Switch - MVR */
        T_D("switch - mvr");
        CX_RC(cx_add_tag_line(s, CX_TAG_MVR, 0));
        memset(&mvrif, 0x0, sizeof(mvr_mgmt_interface_t));
        while (mvr_mgmt_get_next_intf_entry(s->isid, &mvrif) == VTSS_OK) {
            CX_RC(cx_add_port_table_ex(s, s->isid, &mvrif.vid, CX_TAG_PORT_TABLE, cx_mvr_port_roles_match, cx_mvr_port_roles_print));
        }
        CX_RC(cx_add_port_table(s, s->isid, CX_TAG_PORT_TABLE, cx_mvr_fast_leave_match, cx_mvr_fast_leave_print));
        CX_RC(cx_add_tag_line(s, CX_TAG_MVR, 1));

        break;
    default:
        T_E("Unknown command");
        return VTSS_RC_ERROR;
    }

    return VTSS_OK;
}

/* Register the info in to the cx_module_table */
CX_MODULE_TAB_ENTRY(
    VTSS_MODULE_ID_MVR,
    mvr_cx_tag_table,
    0,
    0,
    NULL,                  /* init function       */
    mvr_cx_gen_func,       /* Generation fucntion */
    mvr_cx_parse_func      /* parse fucntion      */
);

