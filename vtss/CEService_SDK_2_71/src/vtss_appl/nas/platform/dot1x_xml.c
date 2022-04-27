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
#include "dot1x_api.h"
#include "port_api.h" /* For PORT_NO_IS_STACK() */

#include "conf_xml_api.h"
#include "conf_xml_trace_def.h"
#include "dot1x_xml.h"
#include "misc_api.h"

/* Tag IDs */
enum {
    /* Module tags */
    CX_TAG_DOT1X,
    CX_TAG_NAS,

    /* Group tags */
    CX_TAG_PORT_TABLE,

    /* Parameter tags */
    CX_TAG_ENTRY,
    CX_TAG_MODE,
    CX_TAG_REAUTH,
    CX_TAG_PERIOD,
    CX_TAG_TIMEOUT,
    CX_TAG_AGETIME,
    CX_TAG_HOLDTIME,
#ifdef VTSS_SW_OPTION_NAS_GUEST_VLAN
    CX_TAG_GUEST_VLAN_ENA,
    CX_TAG_GUEST_VID,
    CX_TAG_REAUTH_MAX,
    CX_TAG_GUEST_VLAN_ALLOW_EAPOL,
#endif
#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS
    CX_TAG_BACKEND_QOS,
#endif
#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_VLAN
    CX_TAG_BACKEND_VLAN,
#endif

    /* Last entry */
    CX_TAG_NONE
};

/* Tag table */
static cx_tag_entry_t dot1x_cx_tag_table[CX_TAG_NONE + 1] = {
    [CX_TAG_DOT1X] = {
        .name  = "dot1x",
        .descr = "IEEE 802.1X authentication",
        .type  = CX_TAG_TYPE_MODULE
    },
    // When reading, accept both the "nas" and the "dot1x" tag
    // When writing, only use the "nas" tag
    [CX_TAG_NAS] = {
        .name  = "nas",
        .descr = "Network Access Server (hereunder IEEE 802.1X)",
        .type  = CX_TAG_TYPE_MODULE
    },
    [CX_TAG_PORT_TABLE] = {
        .name  = "port_table",
        .descr = "",
        .type  = CX_TAG_TYPE_GROUP
    },
    [CX_TAG_ENTRY] = {
        .name  = "entry",
        .descr = "",
        .type  = CX_TAG_TYPE_PARM
    },
    [CX_TAG_MODE] = {
        .name  = "mode",
        .descr = "",
        .type  = CX_TAG_TYPE_PARM
    },
    [CX_TAG_REAUTH] = {
        .name  = "reauth",
        .descr = "",
        .type  = CX_TAG_TYPE_PARM
    },
    [CX_TAG_PERIOD] = {
        .name  = "period",
        .descr = "",
        .type  = CX_TAG_TYPE_PARM
    },
    [CX_TAG_TIMEOUT] = {
        .name  = "timeout",
        .descr = "",
        .type  = CX_TAG_TYPE_PARM
    },
    [CX_TAG_AGETIME] = {
        .name  = "agetime",
        .descr = "",
        .type  = CX_TAG_TYPE_PARM
    },
    [CX_TAG_HOLDTIME] = {
        .name  = "holdtime",
        .descr = "",
        .type  = CX_TAG_TYPE_PARM
    },
#ifdef VTSS_SW_OPTION_NAS_GUEST_VLAN
    [CX_TAG_GUEST_VLAN_ENA] = {
        .name  = "guest_vlan",
        .descr = "",
        .type  = CX_TAG_TYPE_PARM
    },
    [CX_TAG_GUEST_VID] = {
        .name  = "guest_vid",
        .descr = "",
        .type  = CX_TAG_TYPE_PARM
    },
    [CX_TAG_REAUTH_MAX] = {
        .name  = "reauth_max",
        .descr = "",
        .type  = CX_TAG_TYPE_PARM
    },
    [CX_TAG_GUEST_VLAN_ALLOW_EAPOL] = {
        .name  = "guest_vlan_allow_eapol",
        .descr = "",
        .type  = CX_TAG_TYPE_PARM
    },
#endif
#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS
    [CX_TAG_BACKEND_QOS] = {
        .name  = "backend_qos",
        .descr = "",
        .type  = CX_TAG_TYPE_PARM
    },
#endif
#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_VLAN
    [CX_TAG_BACKEND_VLAN] = {
        .name  = "backend_vlan",
        .descr = "",
        .type  = CX_TAG_TYPE_PARM
    },
#endif

    /* Last entry */
    [CX_TAG_NONE] = {
        .name  = "",
        .descr = "",
        .type  = CX_TAG_TYPE_NONE
    }
};

/* Keyword for 802.1X state */
static const cx_kw_t cx_kw_dot1x_state[] = {
    {"auto",         NAS_PORT_CONTROL_AUTO},
    {"authorized",   NAS_PORT_CONTROL_FORCE_AUTHORIZED},
    {"unauthorized", NAS_PORT_CONTROL_FORCE_UNAUTHORIZED},
#ifdef VTSS_SW_OPTION_NAS_MAC_BASED
    {"macbased",     NAS_PORT_CONTROL_MAC_BASED},
#endif /* VTSS_SW_OPTION_NAS_MAC_BASED */
#ifdef VTSS_SW_OPTION_NAS_DOT1X_SINGLE
    {"single",       NAS_PORT_CONTROL_DOT1X_SINGLE},
#endif
#ifdef VTSS_SW_OPTION_NAS_DOT1X_MULTI
    {"multi",        NAS_PORT_CONTROL_DOT1X_MULTI},
#endif
    {NULL,           0}
};

/****************************************************************************/
/****************************************************************************/
static BOOL cx_dot1x_match(const cx_table_context_t *context, ulong port_a, ulong port_b)
{
    dot1x_switch_cfg_t conf;

    return (dot1x_mgmt_switch_cfg_get(context->isid, &conf) == VTSS_RC_OK &&
            memcmp(&conf.port_cfg[port_a - VTSS_PORT_NO_START], &conf.port_cfg[port_b - VTSS_PORT_NO_START], sizeof(dot1x_port_cfg_t)) == 0);
}

/****************************************************************************/
/****************************************************************************/
static vtss_rc cx_dot1x_print(cx_get_state_t *s, const cx_table_context_t *context, ulong port_no, char *ports)
{
    dot1x_switch_cfg_t conf;
    dot1x_port_cfg_t *port_cfg;

    if (ports == NULL) {
        /* Syntax */
        CX_RC(cx_add_stx_start(s));
        CX_RC(cx_add_stx_port(s));
        CX_RC(cx_add_stx_kw(s, "state", cx_kw_dot1x_state));
#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS
        CX_RC(cx_add_stx_bool(s, "backend_qos"));
#endif
#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_VLAN
        CX_RC(cx_add_stx_bool(s, "backend_vlan"));
#endif
#ifdef VTSS_SW_OPTION_NAS_GUEST_VLAN
        CX_RC(cx_add_stx_bool(s, "guest_vlan"));
#endif
        return cx_add_stx_end(s);
    }

    CX_RC(dot1x_mgmt_switch_cfg_get(context->isid, &conf));
    port_cfg = &conf.port_cfg[port_no - VTSS_PORT_NO_START];
    CX_RC(cx_add_port_start(s, CX_TAG_ENTRY, ports));
    CX_RC(cx_add_attr_kw(s, "state", cx_kw_dot1x_state, port_cfg->admin_state));
#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS
    CX_RC(cx_add_attr_bool(s, "backend_qos",  port_cfg->qos_backend_assignment_enabled));
#endif
#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_VLAN
    CX_RC(cx_add_attr_bool(s, "backend_vlan", port_cfg->vlan_backend_assignment_enabled));
#endif
#ifdef VTSS_SW_OPTION_NAS_GUEST_VLAN
    CX_RC(cx_add_attr_bool(s, "guest_vlan",   port_cfg->guest_vlan_enabled));
#endif
    return cx_add_port_end(s, CX_TAG_ENTRY);
}

/****************************************************************************/
/****************************************************************************/
static vtss_rc dot1x_cx_parse_func(cx_set_state_t *s)
{
    BOOL                  port_list[VTSS_PORT_ARRAY_SIZE + 1];
    vtss_port_no_t        port_idx;
    dot1x_cx_set_state_t *dot1x_state = s->mod_state;

    switch (s->cmd) {
    case CX_PARSE_CMD_PARM: {
        ulong          val;
        BOOL           global;
        dot1x_glbl_cfg_t conf;

        global = (s->isid == VTSS_ISID_GLOBAL);

        // Accept both NAS and DOT1X as a keyword when reading.
        if (s->mod_tag != CX_TAG_NAS && s->mod_tag != CX_TAG_DOT1X) {
            s->ignored = 1;
            break;
        }

        if (global && s->apply && dot1x_mgmt_glbl_cfg_get(&conf) != VTSS_RC_OK) {
            break;
        }

        switch (s->id) {
        case CX_TAG_MODE:
            if (global) {
                CX_RC(cx_parse_val_bool(s, &conf.enabled, 1));
            } else {
                s->ignored = 1;
            }
            break;

        case CX_TAG_REAUTH:
            if (global) {
                CX_RC(cx_parse_val_bool(s, &conf.reauth_enabled, 1));
            } else {
                s->ignored = 1;
            }
            break;

        case CX_TAG_PERIOD:
            if (global && cx_parse_val_ulong(s, &val, DOT1X_REAUTH_PERIOD_SECS_MIN, DOT1X_REAUTH_PERIOD_SECS_MAX) == VTSS_RC_OK) {
                conf.reauth_period_secs = val;
            } else {
                s->ignored = 1;
            }
            break;

        case CX_TAG_TIMEOUT:
            if (global && cx_parse_val_ulong(s, &val, DOT1X_EAPOL_TIMEOUT_SECS_MIN, DOT1X_EAPOL_TIMEOUT_SECS_MAX) == VTSS_RC_OK) {
                conf.eapol_timeout_secs = val;
            } else {
                s->ignored = 1;
            }
            break;

#ifdef NAS_USES_PSEC
        case CX_TAG_AGETIME:
            if (global && cx_parse_val_ulong(s, &val, NAS_PSEC_AGING_PERIOD_SECS_MIN, NAS_PSEC_AGING_PERIOD_SECS_MAX) == VTSS_RC_OK) {
                conf.psec_aging_period_secs = val;
            } else {
                s->ignored = 1;
            }
            break;

        case CX_TAG_HOLDTIME:
            if (global && cx_parse_val_ulong(s, &val, NAS_PSEC_HOLD_TIME_SECS_MIN, NAS_PSEC_HOLD_TIME_SECS_MAX) == VTSS_RC_OK) {
                conf.psec_hold_time_secs = val;
            } else {
                s->ignored = 1;
            }
            break;
#endif /* NAS_USES_PSEC */

#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS
        case CX_TAG_BACKEND_QOS:
            if (global) {
                CX_RC(cx_parse_val_bool(s, &conf.qos_backend_assignment_enabled, 1));
            } else {
                s->ignored = 1;
            }
            break;
#endif /* VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS */

#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_VLAN
        case CX_TAG_BACKEND_VLAN:
            if (global) {
                CX_RC(cx_parse_val_bool(s, &conf.vlan_backend_assignment_enabled, 1));
            } else {
                s->ignored = 1;
            }
            break;
#endif /* VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_VLAN */

#ifdef VTSS_SW_OPTION_NAS_GUEST_VLAN
        case CX_TAG_GUEST_VLAN_ENA:
            if (global) {
                CX_RC(cx_parse_val_bool(s, &conf.guest_vlan_enabled, 1));
            } else {
                s->ignored = 1;
            }
            break;

        case CX_TAG_GUEST_VID:
            if (global && cx_parse_val_ulong(s, &val, 1, 4095) == VTSS_RC_OK) {
                conf.guest_vid = val;
            } else {
                s->ignored = 1;
            }
            break;

        case CX_TAG_REAUTH_MAX:
            if (global && cx_parse_val_ulong(s, &val, 1, 255) == VTSS_RC_OK) {
                conf.reauth_max = val;
            } else {
                s->ignored = 1;
            }
            break;

        case CX_TAG_GUEST_VLAN_ALLOW_EAPOL:
            if (global) {
                CX_RC(cx_parse_val_bool(s, &conf.guest_vlan_allow_eapols, 1));
            } else {
                s->ignored = 1;
            }
            break;

#endif /* VTSS_SW_OPTION_NAS_GUEST_VLAN */
        case CX_TAG_PORT_TABLE:
            if (global) {
                s->ignored = 1;
            }
            break;
        case CX_TAG_ENTRY:
            T_N("NAS CX_TAG_ENTRY");
            if (!global && s->group == CX_TAG_PORT_TABLE && cx_parse_ports(s, port_list, 1) == VTSS_RC_OK) {
                dot1x_port_cfg_t port_cfg;
                BOOL             state_seen        = FALSE;
#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS
                BOOL             backend_qos_seen  = FALSE;
#endif
#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_VLAN
                BOOL             backend_vlan_seen = FALSE;
#endif
#ifdef VTSS_SW_OPTION_NAS_GUEST_VLAN
                BOOL             guest_vlan_seen   = FALSE;
#endif

                // Satisfy compiler (i.e. avoid warning that port_cfg.admin_state may be used uninitialized).
                port_cfg.admin_state = NAS_PORT_CONTROL_FORCE_AUTHORIZED;

                s->p = s->next;
                for (; s->rc == VTSS_RC_OK && cx_parse_attr(s) == VTSS_RC_OK; s->p = s->next) {
                    if (cx_parse_kw(s, "state", cx_kw_dot1x_state, &val, 1) == VTSS_RC_OK) {
                        dot1x_state->line_dot1x = s->line;
                        state_seen = TRUE;
                        port_cfg.admin_state = val;
#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS
                    } else if (cx_parse_bool(s, "backend_qos", &port_cfg.qos_backend_assignment_enabled, 1) == VTSS_RC_OK) {
                        backend_qos_seen = TRUE;
#endif
#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_VLAN
                    } else if (cx_parse_bool(s, "backend_vlan", &port_cfg.vlan_backend_assignment_enabled, 1) == VTSS_RC_OK) {
                        backend_vlan_seen = TRUE;
#endif
#ifdef VTSS_SW_OPTION_NAS_GUEST_VLAN
                    } else if (cx_parse_bool(s, "guest_vlan", &port_cfg.guest_vlan_enabled, 1) == VTSS_RC_OK) {
                        guest_vlan_seen = TRUE;
#endif
                    } else {
                        CX_RC(cx_parm_unknown(s));
                    }
                }
                for (port_idx = VTSS_PORT_NO_START; port_idx < VTSS_PORT_NO_END; port_idx++) {
                    if (port_list[iport2uport(port_idx)]) {
                        if (state_seen) {
                            dot1x_state->dot1x.port_cfg[port_idx - VTSS_PORT_NO_START].admin_state = port_cfg.admin_state;
                        }
#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS
                        if (backend_qos_seen) {
                            dot1x_state->dot1x.port_cfg[port_idx - VTSS_PORT_NO_START].qos_backend_assignment_enabled = port_cfg.qos_backend_assignment_enabled;
                        }
#endif
#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_VLAN
                        if (backend_vlan_seen) {
                            dot1x_state->dot1x.port_cfg[port_idx - VTSS_PORT_NO_START].vlan_backend_assignment_enabled = port_cfg.vlan_backend_assignment_enabled;
                        }
#endif
#ifdef VTSS_SW_OPTION_NAS_GUEST_VLAN
                        if (guest_vlan_seen) {
                            dot1x_state->dot1x.port_cfg[port_idx - VTSS_PORT_NO_START].guest_vlan_enabled = port_cfg.guest_vlan_enabled;
                        }
#endif
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
        if (global && s->apply) {
            CX_RC(dot1x_mgmt_glbl_cfg_set(&conf));
        }
        break;
        } /* CX_PARSE_CMD_PARM */

    case CX_PARSE_CMD_GLOBAL:
        break;

    case CX_PARSE_CMD_SWITCH:
        if (s->init) {
            /* Initialize state */
            dot1x_state->line_dot1x.number = 0;
            /* NAS state */
            CX_RC(dot1x_mgmt_switch_cfg_get(s->isid, &dot1x_state->dot1x));
        } else if (s->apply) {
            /* Disable 802.1X before setting the new state. */
            vtss_rc            rc;
            dot1x_switch_cfg_t switch_conf;
            dot1x_port_cfg_t   *conf;
            BOOL               chgd = FALSE;

            rc = dot1x_mgmt_switch_cfg_get(s->isid, &switch_conf);
            for (port_idx = VTSS_PORT_NO_START; port_idx < VTSS_PORT_NO_END; port_idx++) {
                conf = &switch_conf.port_cfg[port_idx - VTSS_PORT_NO_START];
                if (rc == VTSS_RC_OK && !PORT_NO_IS_STACK(port_idx) && conf->admin_state != NAS_PORT_CONTROL_FORCE_AUTHORIZED) {
                    conf->admin_state = NAS_PORT_CONTROL_FORCE_AUTHORIZED;
                    chgd = TRUE;
                }
            }
            if (chgd) {
                CX_RC(dot1x_mgmt_switch_cfg_set(s->isid, &switch_conf));
            }

            /* Set new NAS state */
            CX_RC(dot1x_mgmt_switch_cfg_set(s->isid, &dot1x_state->dot1x));
        }
        break;

    default:
        break;
    }

    return s->rc;
}

/****************************************************************************/
/****************************************************************************/
static vtss_rc dot1x_cx_gen_func(cx_get_state_t *s)
{
    switch (s->cmd) {
    case CX_GEN_CMD_GLOBAL:
        /* Global - 802.1X */
        // When writing, use NAS (when reading, we accept both NAS and DOT1X)
        CX_RC(cx_add_tag_line(s, CX_TAG_NAS, 0));
        {
            dot1x_glbl_cfg_t conf;

            if (dot1x_mgmt_glbl_cfg_get(&conf) == VTSS_RC_OK) {
                CX_RC(cx_add_val_bool (s, CX_TAG_MODE, conf.enabled));
                CX_RC(cx_add_val_bool (s, CX_TAG_REAUTH, conf.reauth_enabled));
                CX_RC(cx_add_val_ulong(s, CX_TAG_PERIOD, conf.reauth_period_secs, DOT1X_REAUTH_PERIOD_SECS_MIN, DOT1X_REAUTH_PERIOD_SECS_MAX));
                CX_RC(cx_add_val_ulong(s, CX_TAG_TIMEOUT, conf.eapol_timeout_secs, DOT1X_EAPOL_TIMEOUT_SECS_MIN, DOT1X_EAPOL_TIMEOUT_SECS_MAX));
#ifdef NAS_USES_PSEC
                CX_RC(cx_add_val_ulong(s, CX_TAG_AGETIME, conf.psec_aging_period_secs, NAS_PSEC_AGING_PERIOD_SECS_MIN, NAS_PSEC_AGING_PERIOD_SECS_MAX));
                CX_RC(cx_add_val_ulong(s, CX_TAG_HOLDTIME, conf.psec_hold_time_secs, NAS_PSEC_HOLD_TIME_SECS_MIN, NAS_PSEC_HOLD_TIME_SECS_MAX));
#endif /* NAS_USES_PSEC */
#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS
                CX_RC(cx_add_val_bool (s, CX_TAG_BACKEND_QOS, conf.qos_backend_assignment_enabled));
#endif
#ifdef VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_VLAN
                CX_RC(cx_add_val_bool (s, CX_TAG_BACKEND_VLAN, conf.vlan_backend_assignment_enabled));
#endif
#ifdef VTSS_SW_OPTION_NAS_GUEST_VLAN
                CX_RC(cx_add_val_bool (s, CX_TAG_GUEST_VLAN_ENA, conf.guest_vlan_enabled));
                CX_RC(cx_add_val_ulong(s, CX_TAG_GUEST_VID, conf.guest_vid, 1, 4095));
                CX_RC(cx_add_val_ulong(s, CX_TAG_REAUTH_MAX, conf.reauth_max, 1, 255));
                CX_RC(cx_add_val_bool (s, CX_TAG_GUEST_VLAN_ALLOW_EAPOL, conf.guest_vlan_allow_eapols));
#endif
            }
        }
        CX_RC(cx_add_tag_line(s, CX_TAG_NAS, 1));
        break;

    case CX_GEN_CMD_SWITCH:
        /* Switch - NAS */
        CX_RC(cx_add_tag_line(s, CX_TAG_NAS, 0));
        CX_RC(cx_add_port_table(s, s->isid, CX_TAG_PORT_TABLE, cx_dot1x_match, cx_dot1x_print));
        CX_RC(cx_add_tag_line(s, CX_TAG_NAS, 1));
        break;

    default:
        T_E("Unknown command");
        return VTSS_RC_ERROR;
    } /* End of Switch */

    return VTSS_RC_OK;
}

/* Register the info in to the cx_module_table */
CX_MODULE_TAB_ENTRY(VTSS_MODULE_ID_DOT1X, dot1x_cx_tag_table,
                    sizeof(dot1x_cx_set_state_t), 0,
                    NULL,                    /* init function       */
                    dot1x_cx_gen_func,       /* Generation fucntion */
                    dot1x_cx_parse_func);    /* parse fucntion      */
