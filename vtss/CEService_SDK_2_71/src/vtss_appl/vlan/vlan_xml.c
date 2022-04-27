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
#include "vlan_api.h"

#include "conf_xml_api.h"
#include "conf_xml_trace_def.h"
#include "misc_api.h"
#include "mgmt_api.h"
#include "port_api.h"

/* Tag IDs */
enum {
    /* Module tags */
    CX_TAG_VLAN,

    /* Group tags */
    CX_TAG_PORT_TABLE,
    CX_TAG_VLAN_TABLE,

    /* Parameter tags */
    CX_TAG_ENTRY,
#if defined(VTSS_FEATURE_VLAN_PORT_V2)
    CX_TAG_TPID,
#endif

    /* Last entry */
    CX_TAG_NONE
};

/* Tag table */
static cx_tag_entry_t vlan_cx_tag_table[CX_TAG_NONE + 1] = {
    [CX_TAG_VLAN] = {
        .name  = "vlan",
        .descr = "Virtual LAN",
        .type = CX_TAG_TYPE_MODULE
    },
    [CX_TAG_PORT_TABLE] = {
        .name  = "port_table",
        .descr = "",
        .type = CX_TAG_TYPE_GROUP
    },
    [CX_TAG_VLAN_TABLE] = {
        .name  = "vlan_table",
        .descr = "",
        .type = CX_TAG_TYPE_GROUP
    },
    [CX_TAG_ENTRY] = {
        .name  = "entry",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
#if defined(VTSS_FEATURE_VLAN_PORT_V2)
    [CX_TAG_TPID] = {
        .name  = "tpid",
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

/* Keyword for frame type */
static const cx_kw_t cx_kw_frame[] = {
    { "all",    VTSS_VLAN_FRAME_ALL },
    { "tagged", VTSS_VLAN_FRAME_TAGGED },
#if defined(VTSS_FEATURE_VLAN_PORT_V2)
    { "untagged", VTSS_VLAN_FRAME_UNTAGGED },
#endif
    { NULL,     0 }
};

/* Keyword for port type */
static const cx_kw_t cx_kw_port[] = {
    { "unaware",    VLAN_PORT_TYPE_UNAWARE },
    { "c-port", VLAN_PORT_TYPE_C },
#if defined(VTSS_FEATURE_VLAN_PORT_V2)
    { "s-port", VLAN_PORT_TYPE_S },
    { "s-custom-port", VLAN_PORT_TYPE_S_CUSTOM },
#endif
    { NULL,     0 }
};

/* Keyword for tx tag */
static const cx_kw_t cx_kw_tx[] = {
    { "untag_pvid",    VLAN_TX_TAG_TYPE_UNTAG_THIS },
    { "tag_all", VLAN_TX_TAG_TYPE_TAG_ALL },
    { "untag_all", VLAN_TX_TAG_TYPE_UNTAG_ALL },
    { NULL,     0 }
};

/* VLAN specific set state structure */
typedef struct {
    u8  vlans_updated[VTSS_BF_SIZE(VTSS_VIDS)];
    u32 vlan_cnt;
} vlan_cx_set_state_t;

static BOOL cx_vlan_match(const cx_table_context_t *context, ulong port_a, ulong port_b)
{
    vlan_port_conf_t conf_a, conf_b;

    return (vlan_mgmt_port_conf_get(context->isid, port_a, &conf_a, VLAN_USER_STATIC) == VTSS_RC_OK &&
            vlan_mgmt_port_conf_get(context->isid, port_b, &conf_b, VLAN_USER_STATIC) == VTSS_RC_OK &&
            memcmp(&conf_a, &conf_b, sizeof(conf_a)) == 0);
}

static vtss_rc cx_vlan_print(cx_get_state_t *s, const cx_table_context_t *context, ulong port_no, char *ports)
{
    vlan_port_conf_t conf;

    if (ports == NULL) {
        /* Syntax */
        CX_RC(cx_add_stx_start(s));
        CX_RC(cx_add_stx_port(s));
        CX_RC(cx_add_stx_ulong(s, "pvid", VLAN_ID_MIN, VLAN_ID_MAX));
        CX_RC(cx_add_stx_kw(s, "frame_type", cx_kw_frame));
#ifdef VTSS_SW_OPTION_VLAN_INGRESS_FILTERING
        CX_RC(cx_add_stx_bool(s, "ingress_filter"));
#endif  /* VTSS_SW_OPTION_VLAN_INGRESS_FILTERING */
        CX_RC(cx_add_stx_kw(s, "port_type", cx_kw_port));
        CX_RC(cx_add_stx_kw(s, "tx_tag", cx_kw_tx));
        return cx_add_stx_end(s);
    }

    (void)vlan_mgmt_port_conf_get(context->isid, port_no, &conf, VLAN_USER_STATIC);
    CX_RC(cx_add_port_start(s, CX_TAG_ENTRY, ports));
    CX_RC(cx_add_attr_ulong(s, "pvid", conf.pvid));
    CX_RC(cx_add_attr_kw(s, "frame_type", cx_kw_frame, conf.frame_type));
#ifdef VTSS_SW_OPTION_VLAN_INGRESS_FILTERING
    CX_RC(cx_add_attr_bool(s, "ingress_filter", conf.ingress_filter));
#endif  /* VTSS_SW_OPTION_VLAN_INGRESS_FILTERING */
    CX_RC(cx_add_attr_kw(s, "port_type", cx_kw_port, conf.port_type));
    CX_RC(cx_add_attr_kw(s, "tx_tag", cx_kw_tx, conf.tx_tag_type));
    return cx_add_port_end(s, CX_TAG_ENTRY);
}

#ifdef VTSS_SW_OPTION_VLAN_NAMING
/* Parse VLAN name string */
static vtss_rc cx_parse_vlan_name(cx_set_state_t *s, char *name, char *vlan_name)
{
    uint idx;

    CX_RC(cx_parse_txt(s, name, vlan_name, VLAN_NAME_MAX_LEN - 1));

    for (idx = 0; idx < strlen(vlan_name); idx++) {
        if (vlan_name[idx] < 33 || vlan_name[idx] > 126) {
            CX_RC(cx_parm_invalid(s));
            break;
        }
    }

    return s->rc;
}
#endif

static vtss_rc vlan_cx_parse_func(cx_set_state_t *s)
{
    vlan_cx_set_state_t *vlan_state = s->mod_state;

    switch (s->cmd) {
    case CX_PARSE_CMD_PARM: {
        vtss_port_no_t port_idx;
        BOOL           port_list[VTSS_PORT_ARRAY_SIZE + 1];
        BOOL           forbidden_port_list[VTSS_PORT_ARRAY_SIZE + 1];
        ulong          val;
        BOOL           global;
        vtss_vid_t     vid;

        global = (s->isid == VTSS_ISID_GLOBAL);
        if (global) {
#if defined(VTSS_FEATURE_VLAN_PORT_V2)
            if (s->id != CX_TAG_TPID) {
#endif
                s->ignored = 1;
                break;
#if defined(VTSS_FEATURE_VLAN_PORT_V2)
            } else {
                if (cx_parse_val_ulong(s, &val, 0x600, 0xFFFF) == VTSS_RC_OK && s->apply) {
                    CX_RC(vlan_mgmt_s_custom_etype_set((vtss_etype_t)val));
                }
                return s->rc;
            }
#endif
        }

        switch (s->id) {
        case CX_TAG_PORT_TABLE:
            break;

        case CX_TAG_VLAN_TABLE:
            break;

        case CX_TAG_ENTRY:
            if (s->group == CX_TAG_PORT_TABLE && cx_parse_ports(s, port_list, 1) == VTSS_RC_OK) {
                vlan_port_conf_t new_port_conf, port_conf;
                BOOL             pvid = 0, frame = 0, in_filter = 0;
                BOOL             ptype = 0, tx_tag = 0;

                s->p = s->next;
                for (; s->rc == VTSS_RC_OK && cx_parse_attr(s) == VTSS_RC_OK; s->p = s->next) {
                    if (cx_parse_ulong_word(s, "pvid", &val, VLAN_ID_MIN, VLAN_ID_MAX, "none", 0) == VTSS_RC_OK) {
                        pvid = 1;
                        new_port_conf.pvid = val;
                        if (val == 0) {
                            // Allow deprecated "none" keyword, which sets PVID to 1 and causes port to tag all frames.
                            new_port_conf.pvid = VLAN_ID_DEFAULT;
                            new_port_conf.tx_tag_type = VLAN_TX_TAG_TYPE_TAG_ALL;
                            tx_tag = 1;
                        }
                    } else if (cx_parse_kw(s, "frame_type", cx_kw_frame, &val, 1) == VTSS_RC_OK) {
                        frame = 1;
                        new_port_conf.frame_type = val;
                    } else if (cx_parse_bool(s, "ingress_filter", &new_port_conf.ingress_filter, 1) == VTSS_RC_OK) {
#ifdef VTSS_SW_OPTION_VLAN_INGRESS_FILTERING
                        in_filter = 1;
#else
                        /* NB: Accept syntax, but skip using data */
                        in_filter = 0;
#endif  /* VTSS_SW_OPTION_VLAN_INGRESS_FILTERING */
                    } else if (cx_parse_kw(s, "port_type", cx_kw_port, &val, 1) == VTSS_RC_OK) {
                        ptype = 1;
                        new_port_conf.port_type = val;
                    } else if (cx_parse_kw(s, "tx_tag", cx_kw_tx, &val, 1) == VTSS_RC_OK) {
                        tx_tag = 1;
                        new_port_conf.tx_tag_type = val;
                    } else {
                        CX_RC(cx_parm_unknown(s));
                    }
                }

                for (port_idx = VTSS_PORT_NO_START; s->apply && port_idx < s->port_count; port_idx++) {
                    if (port_list[iport2uport(port_idx)] && vlan_mgmt_port_conf_get(s->isid, port_idx, &port_conf, VLAN_USER_STATIC) == VTSS_RC_OK) {
                        vtss_rc rc;

                        if (pvid) {
                            port_conf.pvid = new_port_conf.pvid;
                            port_conf.untagged_vid = new_port_conf.pvid;
                            port_conf.tx_tag_type = VLAN_TX_TAG_TYPE_UNTAG_THIS;
                        }

                        if (frame) {
                            port_conf.frame_type = new_port_conf.frame_type;
                        }

#ifdef VTSS_SW_OPTION_VLAN_INGRESS_FILTERING
                        if (in_filter) {
                            port_conf.ingress_filter = new_port_conf.ingress_filter;
                        }
#endif  /* VTSS_SW_OPTION_VLAN_INGRESS_FILTERING */

                        if (ptype) {
                            port_conf.port_type = new_port_conf.port_type;
                        }

                        if (tx_tag) {
                            port_conf.tx_tag_type = new_port_conf.tx_tag_type;
                        }

                        if ((rc = vlan_mgmt_port_conf_set(s->isid, port_idx, &port_conf, VLAN_USER_STATIC)) != VTSS_RC_OK) {
                            T_E("vlan_mgmt_port_conf_set() failed with %s", error_txt(rc));
                        }
                    }
                }
            } else if (s->group == CX_TAG_VLAN_TABLE && cx_parse_vid(s, &vid, 1) == VTSS_RC_OK) {
                BOOL              vid_set = FALSE;
                vlan_mgmt_entry_t new_vlan_conf;
#ifdef VTSS_SW_OPTION_VLAN_NAMING
                BOOL              has_vlan_name = FALSE;
                char              vlan_name[VLAN_NAME_MAX_LEN];
#endif
                memset(&new_vlan_conf, 0, sizeof(new_vlan_conf));

                new_vlan_conf.vid = vid;
                s->p = s->next;

#ifdef VTSS_SW_OPTION_VLAN_NAMING
                if (cx_parse_vlan_name(s, "vlan_name", vlan_name) == VTSS_RC_OK) {
                    has_vlan_name = TRUE;
                }
                s->p = s->next;
#endif

                memset(port_list,           0, sizeof(port_list));
                memset(forbidden_port_list, 0, sizeof(forbidden_port_list));

                if (cx_parse_ports(s, port_list, FALSE) == VTSS_RC_OK) {
                    // Convert from xml port to api port
                    for (port_idx = VTSS_PORT_NO_START; port_idx < VTSS_PORT_NO_END; port_idx++) {
                        new_vlan_conf.ports[port_idx] = port_list[iport2uport(port_idx)];
                    }
#ifdef VTSS_SW_OPTION_VLAN_NAMING
                    if (has_vlan_name == FALSE) {
                        CX_RC(cx_parm_found_error(s, "vlan_name"));
                    }
#endif
                    // Apply configuration
                    vid_set = TRUE;
                }
                s->p = s->next;

                if (cx_parse_ports_name(s, forbidden_port_list, FALSE, "Forbidden_port") == VTSS_RC_OK) {
                    for (port_idx = VTSS_PORT_NO_START; port_idx < VTSS_PORT_NO_END; port_idx++) {
                        if (new_vlan_conf.ports[port_idx] != 1) {
                            if (forbidden_port_list[iport2uport(port_idx)]) {
                                new_vlan_conf.ports[port_idx] = VLAN_FORBIDDEN_PORT;
                            }
                            // Apply configuration
                            vid_set = TRUE;
                        } else if (forbidden_port_list[iport2uport(port_idx)]) { /* not legal configuration */
                            sprintf(s->msg, "Ports cannot be a member of both VLAN include membership and VLAN forbidden membership");
                            s->rc = CONF_XML_ERROR_FILE_PARM;
                            vid_set = FALSE;
                            //CX_RC(cx_parm_found_error(s, "Forbidden_port"));
                        }
                    }
#ifdef VTSS_SW_OPTION_VLAN_NAMING
                    if (has_vlan_name == FALSE) {
                        CX_RC(cx_parm_found_error(s, "vlan_name"));
                    }
#endif
                }

                if (!vid_set) {
                    // VLAN will be deleted later (in CX_PARSE_CMD_SWITCH section)
                    break;
                }

                VTSS_BF_SET(vlan_state->vlans_updated, vid, TRUE);

                if (s->apply) {
                    vlan_mgmt_entry_t old_vlan_conf;
                    BOOL              vlan_exists, add_vlan = FALSE;

#ifdef VTSS_SW_OPTION_VLAN_NAMING
                    if (has_vlan_name) {
                        (void)vlan_mgmt_name_set(vid, vlan_name);
                    }
#endif

                    if (vlan_state->vlan_cnt++ == 0) {
                        // We have to call vlan_bulk_update_begin() here, upon the first VLAN change,
                        // because the CX_PARSE_CMD_SWITCH case below can't detect the right state because
                        // of the silly way that the s->init and s->apply flags are updated by conf_xml.c
                        vlan_bulk_update_begin();
                    }
                    // Gotta check if either VLAN or forbidden ports on a VLAN is already configured on the port,
                    // and if so, check that the new conf is different, and if so, check whether just applying the
                    // new conf will give rise to VLAN/Forbidden VLAN ports conflicts, and if so, remove the old VLANs
                    // before re-applying the new.
                    memset(&old_vlan_conf, 0, sizeof(old_vlan_conf));
                    vlan_exists = vlan_mgmt_vlan_get(s->isid, vid, &old_vlan_conf, FALSE, VLAN_USER_STATIC) == VTSS_RC_OK;

                    add_vlan = vid_set && ((vlan_exists && memcmp(&new_vlan_conf.ports[0], &old_vlan_conf.ports[0], sizeof(new_vlan_conf.ports)) != 0) || !vlan_exists);

                    if (add_vlan) {
                        // Prevent port-clash
                        T_DG(VTSS_TRACE_GRP_VLAN, "VID=%d: Deleting forbidden to prevent port-clash", vid);
                        (void)vlan_mgmt_vlan_del(s->isid, vid, VLAN_USER_STATIC);
                        (void)vlan_mgmt_vlan_del(s->isid, vid, VLAN_USER_FORBIDDEN);
                        T_DG(VTSS_TRACE_GRP_VLAN, "VID=%d: Adding VLAN", vid);
                        (void)vlan_mgmt_vlan_add(s->isid, &new_vlan_conf, VLAN_USER_STATIC);
                    }
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
        if (s->init) {
            memset(vlan_state->vlans_updated, 0, sizeof(vlan_state->vlans_updated));
            vlan_state->vlan_cnt = 0;
        } else if (s->apply) {
            vlan_mgmt_entry_t vlan_conf;

            if (vlan_state->vlan_cnt) {
                // Gotta go through the vlan_state->vlans_updated array and delete all VLANs
                // that are not changed by this XML file.
                vlan_conf.vid = VTSS_VID_NULL;
                while (vlan_mgmt_vlan_get(s->isid, vlan_conf.vid, &vlan_conf, TRUE, VLAN_USER_STATIC) == VTSS_RC_OK) {
                    if (VTSS_BF_GET(vlan_state->vlans_updated, vlan_conf.vid) == FALSE) {
                        T_DG(VTSS_TRACE_GRP_VLAN, "VID=%d: No line for VLAN. Deleting", vlan_conf.vid);
                        (void)vlan_mgmt_vlan_del(s->isid, vlan_conf.vid, VLAN_USER_STATIC);
                        (void)vlan_mgmt_vlan_del(s->isid, vlan_conf.vid, VLAN_USER_FORBIDDEN);
                    }
                }

                // Time to let bulk VLAN update listeners get a view of the new VLAN membership.
                vlan_bulk_update_end();
            }
        }
        break;

    default:
        break;
    }

    return s->rc;
}

static vtss_rc vlan_cx_gen_func(cx_get_state_t *s)
{
    char           buf[MGMT_PORT_BUF_SIZE];
    vtss_vid_t     prev_vid = 0;
    BOOL           mports[VTSS_PORT_ARRAY_SIZE], fports[VTSS_PORT_ARRAY_SIZE];
    BOOL           mports_exist = FALSE, fports_exist = FALSE;
    vtss_port_no_t iport;
#if defined(VTSS_FEATURE_VLAN_PORT_V2)
    vtss_etype_t   tpid;
#endif

    switch (s->cmd) {
    case CX_GEN_CMD_GLOBAL:
#if defined(VTSS_FEATURE_VLAN_PORT_V2)
        CX_RC(cx_add_tag_line(s, CX_TAG_VLAN, 0));
        if (vlan_mgmt_s_custom_etype_get(&tpid) == VTSS_RC_OK) {
            CX_RC(cx_add_val_ulong(s, CX_TAG_TPID, tpid, 0x600, 0xFFFFFFFF));
        }
        CX_RC(cx_add_tag_line(s, CX_TAG_VLAN, 1));
#endif
        break;
    case CX_GEN_CMD_SWITCH:
        /* Switch - VLAN */
        T_D("switch - vlan");
        CX_RC(cx_add_tag_line(s, CX_TAG_VLAN, 0));
        CX_RC(cx_add_port_table(s, s->isid, CX_TAG_PORT_TABLE, cx_vlan_match, cx_vlan_print));
        {
            vlan_mgmt_entry_t conf;
#ifdef VTSS_SW_OPTION_VLAN_NAMING
            i8                help[100];
#endif
#if VTSS_SWITCH_STACKABLE
            port_isid_info_t  pinfo;
#endif

            CX_RC(cx_add_tag_line(s, CX_TAG_VLAN_TABLE, 0));

            /* Entry syntax */
            CX_RC(cx_add_stx_start(s));
            CX_RC(cx_add_stx_ulong(s, "vid", VLAN_ID_MIN, VLAN_ID_MAX));
#ifdef VTSS_SW_OPTION_VLAN_NAMING
            sprintf(help, "0-%u characters", VLAN_NAME_MAX_LEN - 1);
            CX_RC(cx_add_attr_txt(s, "vlan_name", help));
#endif
            sprintf(help, "none or 1-4095 (list allowed, e.g. 1,3-5)");
            CX_RC(cx_add_attr_txt(s, "port", help));
            sprintf(help, "none or 1-4095 (list allowed, e.g. 1,3-5)");
            CX_RC(cx_add_attr_txt(s, "Forbidden_port", help));
            CX_RC(cx_add_stx_end(s));

            prev_vid = VTSS_VID_NULL;
            while (((vlan_mgmt_vlan_get(s->isid, prev_vid, &conf, TRUE, VLAN_USER_STATIC)) == VTSS_RC_OK)) {
                prev_vid = conf.vid;
                CX_RC(cx_add_attr_start(s, CX_TAG_ENTRY));
                CX_RC(cx_add_attr_ulong(s, "vid", conf.vid));
                for (iport = VTSS_PORT_NO_START; iport < VTSS_PORT_NO_END; iport++) {
                    mports[iport] = (conf.ports[iport] == 1) ? TRUE : FALSE;
                    fports[iport] = (conf.ports[iport] == VLAN_FORBIDDEN_PORT) ? TRUE : FALSE;
                    if (mports[iport]) {
                        mports_exist = TRUE;
                    }
                    if (fports[iport]) {
                        fports_exist = TRUE;
                    }
                }
#if VTSS_SWITCH_STACKABLE
                if (port_isid_info_get(s->isid, &pinfo) == VTSS_RC_OK) {
                    conf.ports[pinfo.stack_port_0] = 0;
                    conf.ports[pinfo.stack_port_1] = 0;
                }
#endif
#ifdef VTSS_SW_OPTION_VLAN_NAMING
                CX_RC(cx_add_attr_txt(s, "vlan_name", conf.vlan_name));
#endif
                if (mports_exist) {
                    CX_RC(cx_add_attr_txt(s, "port", mgmt_iport_list2txt(mports, buf)));
                } else {
                    CX_RC(cx_add_attr_txt(s, "port", "none"));
                }
                if (fports_exist) {
                    CX_RC(cx_add_attr_txt(s, "Forbidden_port", mgmt_iport_list2txt(fports, buf)));
                } else {
                    CX_RC(cx_add_attr_txt(s, "Forbidden_port", "none"));
                }
                CX_RC(cx_add_attr_end(s, CX_TAG_ENTRY));
            }
            CX_RC(cx_add_tag_line(s, CX_TAG_VLAN_TABLE, 1));
        }
        CX_RC(cx_add_tag_line(s, CX_TAG_VLAN, 1));
        break;
    default:
        T_E("Unknown command");
        return VTSS_RC_ERROR;
    }

    return VTSS_RC_OK;
}

/* Register the info in to the cx_module_table */
CX_MODULE_TAB_ENTRY(
    VTSS_MODULE_ID_VLAN,
    vlan_cx_tag_table,
    sizeof(vlan_cx_set_state_t),
    0,
    NULL,                   /* init function       */
    vlan_cx_gen_func,       /* Generation fucntion */
    vlan_cx_parse_func      /* parse fucntion      */
);


