/*

 Vitesse Switch API software.

 Copyright (c) 2002-2011 Vitesse Semiconductor Corporation "Vitesse". All
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
#include "port_custom_api.h"

#include "conf_xml_api.h"
#include "conf_xml_trace_def.h"
#include "misc_api.h"

/* Tag IDs */
enum {
    /* Module tags */
    CX_TAG_PORT,

    /* Group tags */
    CX_TAG_PORT_TABLE,

    /* Parameter tags */
    CX_TAG_ENTRY,

    /* Last entry */
    CX_TAG_NONE
};

/* Tag table */
static cx_tag_entry_t port_cx_tag_table[CX_TAG_NONE + 1] =
{
    [CX_TAG_PORT] = {
        .name  = "port",
        .descr = "Port control",
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

    /* Last entry */
    [CX_TAG_NONE] = {
        .name  = "",
        .descr = "",
        .type = CX_TAG_TYPE_NONE
    }
};

#ifdef VTSS_SW_OPTION_PHY_POWER_CONTROL
/* Keyword for port power control */
static const cx_kw_t cx_kw_port_power[] = {
    { "enabled",  VTSS_PHY_POWER_ENABLED },
    { "disabled", VTSS_PHY_POWER_NOMINAL },
    { "actiphy",  VTSS_PHY_POWER_ACTIPHY },
    { "dynamic",  VTSS_PHY_POWER_DYNAMIC },
    { NULL, 0 }
};
#endif /* VTSS_SW_OPTION */

/* Port mode */
typedef struct {
    port_cap_t        cap;
    vtss_port_speed_t speed;
    BOOL              fdx;
} cx_port_mode_t;

/* Mapping between capability and speed/fdx */
static const cx_port_mode_t cx_port_mode[] = {
    { PORT_CAP_AUTONEG,  VTSS_SPEED_UNDEFINED, 0},
    { PORT_CAP_10M_HDX,  VTSS_SPEED_10M,       0},
    { PORT_CAP_10M_FDX,  VTSS_SPEED_10M,       1},
    { PORT_CAP_100M_HDX, VTSS_SPEED_100M,      0},
    { PORT_CAP_100M_FDX, VTSS_SPEED_100M,      1},
    { PORT_CAP_1G_FDX,   VTSS_SPEED_1G,        1},
    { PORT_CAP_2_5G_FDX, VTSS_SPEED_2500M,     1},
    { PORT_CAP_5G_FDX,   VTSS_SPEED_5G,        1},
    { PORT_CAP_10G_FDX,  VTSS_SPEED_10G,       1},
    { PORT_CAP_NONE,     VTSS_SPEED_UNDEFINED, 0}
};

/* Maximum number of port modes (one extra mode to support both 1000fdx and 1Gfdx */
#define PORT_CX_MODE_MAX (sizeof(cx_port_mode)/sizeof(cx_port_mode_t)+1)

static vtss_rc port_cx_mode_get(port_cap_t cap, cx_kw_t *kw)
{
    if (cap & PORT_CAP_AUTONEG) {
        kw->name = "auto";
        kw->val = PORT_CAP_AUTONEG;
        kw++;
    }
    if (cap & PORT_CAP_10M_HDX) {
        kw->name = "10hdx";
        kw->val = PORT_CAP_10M_HDX;
        kw++;
    }
    if (cap & PORT_CAP_10M_FDX) {
        kw->name = "10fdx";
        kw->val = PORT_CAP_10M_FDX;
        kw++;
    }
    if (cap & PORT_CAP_100M_HDX) {
        kw->name = "100hdx";
        kw->val = PORT_CAP_100M_HDX;
        kw++;
    }
    if (cap & PORT_CAP_100M_FDX) {
        kw->name = "100fdx";
        kw->val = PORT_CAP_100M_FDX;
        kw++;
    }
    if (cap & PORT_CAP_1G_FDX) {
        /* Support both '1000fdx' and '1Gfdx' */
        kw->name = "1000fdx";
        kw->val = PORT_CAP_1G_FDX;
        kw++;
        kw->name = "1Gfdx";
        kw->val = PORT_CAP_1G_FDX;
        kw++;
    }
    if (cap & PORT_CAP_2_5G_FDX) {
        kw->name = "2500fdx";
        kw->val = PORT_CAP_2_5G_FDX;
        kw++;
    }
    if (cap & PORT_CAP_5G_FDX) {
        kw->name = "5Gfdx";
        kw->val = PORT_CAP_5G_FDX;
        kw++;
    }
    if (cap & PORT_CAP_10G_FDX) {
        kw->name = "10Gfdx";
        kw->val = PORT_CAP_10G_FDX;
        kw++;
    }

    /* End of list */
    kw->name = NULL;

    return VTSS_OK;
}

static BOOL cx_port_match(const cx_table_context_t *context, ulong port_a, ulong port_b)
{
    port_conf_t conf_a, conf_b;

    return (port_mgmt_conf_get(context->isid, port_a, &conf_a) == VTSS_OK &&
            port_mgmt_conf_get(context->isid, port_b, &conf_b) == VTSS_OK &&
            conf_a.enable == conf_b.enable &&
            conf_a.autoneg == conf_b.autoneg &&
            (conf_a.autoneg || (conf_a.speed == conf_b.speed && conf_a.fdx == conf_b.fdx)) &&
            conf_a.flow_control == conf_b.flow_control &&
            conf_a.max_length == conf_b.max_length &&
#ifdef VTSS_SW_OPTION_PHY_POWER_CONTROL
            conf_a.power_mode == conf_b.power_mode &&
#endif /* VTSS_SW_OPTION_PHY_POWER_CONTROL */
            conf_a.exc_col_cont == conf_b.exc_col_cont);
}

static vtss_rc cx_port_print(cx_get_state_t *s, const cx_table_context_t *context, ulong port_no, char *ports)
{
    port_conf_t      conf;
    cx_port_mode_t   *mode;
    cx_kw_t          port_mode_table[PORT_CX_MODE_MAX];
    port_isid_info_t info;
    BOOL             fc;
    
    /* Determine if flow control is supported */
    if (port_isid_info_get(s->isid, &info) != VTSS_OK ||
        port_cx_mode_get(info.cap, port_mode_table) != VTSS_OK) {
        s->error = 1;
        return CONF_XML_ERROR_GEN; 
    }
    fc = ((info.cap & PORT_CAP_FLOW_CTRL) ? 1 : 0);
    
    if (ports == NULL) {
        /* Syntax */
        CX_RC(cx_add_stx_start(s));
        CX_RC(cx_add_stx_port(s));
        CX_RC(cx_add_stx_bool(s, "state"));
        CX_RC(cx_add_stx_kw(s, "mode", port_mode_table));
        if (fc)
            CX_RC(cx_add_stx_bool(s, "flow_control"));
        CX_RC(cx_add_stx_ulong(s, "length", VTSS_MAX_FRAME_LENGTH_STANDARD, VTSS_MAX_FRAME_LENGTH_MAX));
        CX_RC(cx_add_stx_bool(s, "excessive_restart"));
#ifdef VTSS_SW_OPTION_PHY_POWER_CONTROL
        CX_RC(cx_add_stx_kw(s, "power", cx_kw_port_power));
#endif /* VTSS_SW_OPTION_PHY_POWER_CONTROL */
        return cx_add_stx_end(s);
    }

    CX_RC(port_mgmt_conf_get(context->isid, port_no, &conf));
    CX_RC(cx_add_port_start(s, CX_TAG_ENTRY, ports));
    CX_RC(cx_add_attr_bool(s, "state", conf.enable));
    for (mode = (cx_port_mode_t *)cx_port_mode; mode->cap != PORT_CAP_NONE; mode++)
        if ((conf.autoneg && mode->cap == PORT_CAP_AUTONEG) ||
            (conf.speed == mode->speed && conf.fdx == mode->fdx))
            break;
    CX_RC(cx_add_attr_kw(s, "mode", port_mode_table, mode->cap));
    if (fc)
        CX_RC(cx_add_attr_bool(s, "flow_control", conf.flow_control));
    CX_RC(cx_add_attr_ulong(s, "length", conf.max_length));
    CX_RC(cx_add_attr_bool(s, "excessive_restart", conf.exc_col_cont));
#ifdef VTSS_SW_OPTION_PHY_POWER_CONTROL
    CX_RC(cx_add_attr_kw(s, "power", cx_kw_port_power, conf.power_mode));
#endif /* VTSS_SW_OPTION_PHY_POWER_CONTROL */
    return cx_add_port_end(s, CX_TAG_ENTRY);
}

static vtss_rc port_cx_parm_error(cx_set_state_t *s, const char *name, vtss_port_no_t iport)
{
    char buf[256];

    sprintf(buf, "Parameter '%s' illegal for port %u", name, iport2uport(iport));
    return cx_parm_error(s, buf);
}

static vtss_rc port_cx_parse_func(cx_set_state_t *s)
{
    vtss_port_no_t        iport;
    BOOL                  port_list[VTSS_PORT_ARRAY_SIZE+1];
    ulong                 val;
    BOOL                  global;
    port_conf_t           conf, new;
    cx_port_mode_t        *mode;
    BOOL                  state = 0, aneg = 0, fc = 0, max = 0, exc = 0;
    port_cap_t            new_cap = 0;
#ifdef VTSS_SW_OPTION_PHY_POWER_CONTROL
    BOOL                  power = 0;
#endif /* VTSS_SW_OPTION_PHY_POWER_CONTROL */
    port_isid_port_info_t info;
    cx_kw_t               port_mode_table[PORT_CX_MODE_MAX];
    port_isid_info_t      isid_info;
    
    if (s->cmd != CX_PARSE_CMD_PARM) {
        return s->rc;
    }

    global = (s->isid == VTSS_ISID_GLOBAL);
    if (global) {
        s->ignored = 1;
        return s->rc;
    }
    
    switch (s->id) {
    case CX_TAG_PORT_TABLE:
        break;
    case CX_TAG_ENTRY:
        if (s->group == CX_TAG_PORT_TABLE &&
            cx_parse_ports(s, port_list, 1) == VTSS_OK) {
            
            /* Build port mode table for ISID */
            if (port_isid_info_get(s->isid, &isid_info) != VTSS_OK ||
                port_cx_mode_get(isid_info.cap, port_mode_table) != VTSS_OK) {
                sprintf(s->msg, "Unknown capabilities for ISID %u", s->isid);
                s->rc = CONF_XML_ERROR_GEN;
                break;
            }

            /* Handle port table in switch section */
            s->p = s->next;
            for (; s->rc == VTSS_OK && cx_parse_attr(s) == VTSS_OK; s->p = s->next) {
                if (cx_parse_bool(s, "state", &new.enable, 1) == VTSS_OK)
                    state = 1;
                else if (cx_parse_kw(s, "mode", port_mode_table, &new_cap, 1) == VTSS_OK) {
                    aneg = 1;
                    new.autoneg = (new_cap == PORT_CAP_AUTONEG);
                    for (mode = (cx_port_mode_t *)cx_port_mode; mode->cap != PORT_CAP_NONE; mode++)
                        if (mode->cap == new_cap) {
                            new.speed = mode->speed;
                            new.fdx = mode->fdx;
                        }
                } else if (cx_parse_bool(s, "flow_control", &new.flow_control,
                                         1) == VTSS_OK)
                    fc = 1;
                else if (cx_parse_ulong(s, "length", &val, VTSS_MAX_FRAME_LENGTH_STANDARD,
                                        VTSS_MAX_FRAME_LENGTH_MAX) == VTSS_OK) {
                    max = 1;
                    new.max_length = val;
                } else if (cx_parse_bool(s, "excessive_restart",
                                         &new.exc_col_cont, 1) == VTSS_OK) {
                    exc = 1;
#ifdef VTSS_SW_OPTION_PHY_POWER_CONTROL
                } else if (cx_parse_kw(s, "power", cx_kw_port_power, &val, 1) == VTSS_OK) {
                    power = 1;
                    new.power_mode = val;
#endif /* VTSS_SW_OPTION_PHY_POWER_CONTROL */
                } else
                    return cx_parm_unknown(s);
            } /* for loop */
            for (iport = VTSS_PORT_NO_START; iport < VTSS_PORT_NO_END; iport++) {
                if (!port_list[iport2uport(iport)])
                    continue;
                if (s->apply && port_mgmt_conf_get(s->isid, iport, &conf) == VTSS_OK) {
                    if (state)
                        conf.enable = new.enable;
                    if (aneg) {
                        conf.autoneg = new.autoneg;
                        if (!new.autoneg) {
                            conf.speed = new.speed;
                            conf.fdx = new.fdx;
                        }
                    }
                    if (fc)
                        conf.flow_control = new.flow_control;
                    if (max)
                        conf.max_length = new.max_length;
                    if (exc)
                        conf.exc_col_cont = new.exc_col_cont;
#ifdef VTSS_SW_OPTION_PHY_POWER_CONTROL
                    if (power)
                        conf.power_mode = new.power_mode;
#endif /* VTSS_SW_OPTION_PHY_POWER_CONTROL */
                    (void)port_mgmt_conf_set(s->isid, iport, &conf); /* Ignore errors */
                } else if (port_isid_port_info_get(s->isid, iport, &info) != VTSS_OK) {
                    sprintf(s->msg, "Unknown capabilities for port %u", iport2uport(iport));
                    s->rc = CONF_XML_ERROR_GEN;
                    break;
                } else {
                    /* Check port capability */
                    if (aneg && (new_cap & info.cap) == 0) {
                        return port_cx_parm_error(s, "mode", iport);
                    }
                    if (fc && new.flow_control && (info.cap & PORT_CAP_FLOW_CTRL) == 0) {
                        return port_cx_parm_error(s, "flow_control", iport);
                    }
#ifdef VTSS_SW_OPTION_PHY_POWER_CONTROL
                    if (power && new.power_mode != VTSS_PHY_POWER_NOMINAL && 
                        (info.cap & PORT_CAP_1G_PHY) == 0) {
                        return port_cx_parm_error(s, "power", iport);
                    }
#endif /* VTSS_SW_OPTION_PHY_POWER_CONTROL */
                }
            }
        } else
            s->ignored = 1;
        break;
    default:
        s->ignored = 1;
        break;
    }
    return s->rc;
}
static vtss_rc port_cx_gen_func(cx_get_state_t *s)
{
    if (s->cmd != CX_GEN_CMD_SWITCH) {
        return VTSS_OK;
    }

    /* Switch - Port */
    T_D("switch - port");
    CX_RC(cx_add_tag_line(s, CX_TAG_PORT, 0));
    CX_RC(cx_add_port_table(s, s->isid, CX_TAG_PORT_TABLE,
                            cx_port_match, cx_port_print));
    CX_RC(cx_add_tag_line(s, CX_TAG_PORT, 1));

    return VTSS_OK;
}
/* Register the info in to the cx_module_table */
CX_MODULE_TAB_ENTRY(VTSS_MODULE_ID_PORT, 
                    port_cx_tag_table,
                    0, 
                    0,
                    NULL,                /* Init function       */
                    port_cx_gen_func,    /* Generation function */
                    port_cx_parse_func); /* Parse function      */
