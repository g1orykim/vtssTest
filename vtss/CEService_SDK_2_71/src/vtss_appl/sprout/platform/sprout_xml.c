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
#include "topo_api.h"
#include "misc_api.h"
#include "port_api.h"

#include "conf_xml_api.h"
#include "conf_xml_trace_def.h"

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_TOPO


enum {
    
    CX_TAG_STACK,

    
    CX_TAG_MST_PRIO,
    CX_TAG_STACKING,
    CX_TAG_PORT_0,
    CX_TAG_PORT_1,

    
    CX_TAG_NONE
};


static cx_tag_entry_t sprout_cx_tag_table[CX_TAG_NONE + 1] = {
    [CX_TAG_STACK] = {
        .name  = "stack",
        .descr = "Stack setup",
        .type = CX_TAG_TYPE_MODULE
    },

    [CX_TAG_MST_PRIO] = {
        .name  = "prio",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },

    [CX_TAG_STACKING] = {
        .name  = "stacking",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },

    [CX_TAG_PORT_0] = {
        .name  = "port_0",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },

    [CX_TAG_PORT_1] = {
        .name  = "port_1",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },

    
    [CX_TAG_NONE] = {
        .name  = "",
        .descr = "",
        .type = CX_TAG_TYPE_NONE
    }
};



typedef struct {
    cx_line_t      line_no;  
    stack_config_t conf;
} stack_cx_set_state_t;


static BOOL validate_stack_port(vtss_isid_t isid, vtss_port_no_t port_no)
{
    vtss_rc               rc;
    port_isid_port_info_t info;

    if ((rc = port_isid_port_info_get(isid, port_no, &info)) == VTSS_RC_OK && (info.cap & PORT_CAP_STACKING)) {
        return TRUE;
    }
    T_W("Isid %d port %d - skipping invalid stack port", isid, port_no);
    return FALSE;
}


static vtss_rc sprout_cx_parse_func(cx_set_state_t *s)
{
    stack_cx_set_state_t *state = s->mod_state;

    switch (s->cmd) {
    case CX_PARSE_CMD_PARM: {
        ulong val;
        BOOL  bval, global = (s->isid == VTSS_ISID_GLOBAL);
        switch (s->id) {
        case CX_TAG_MST_PRIO:
            
            if (!global && cx_parse_val_ulong(s, &val, TOPO_PARM_MST_ELECT_PRIO_MIN, TOPO_PARM_MST_ELECT_PRIO_MAX) == VTSS_RC_OK) {
                if (s->apply && vtss_stacking_enabled()) {
                    T_D("Setting master prio to %u", val);
                    if ((topo_parm_set(s->isid, TOPO_PARM_MST_ELECT_PRIO, val)) == VTSS_RC_OK) {
                        s->master_elect = 1;
                    } else {
                        T_E("Master priority wasn't set correctly");
                    }
                }
            } else {
                s->ignored = 1;
            }
            break;
        case CX_TAG_STACKING:
            if (!global && cx_parse_val_bool(s, &bval, 1) == VTSS_RC_OK) {
                state->conf.stacking = bval;
            } else {
                s->ignored = 1;
            }
            break;
        case CX_TAG_PORT_0:
            if (!global && cx_parse_val_ulong(s, &val, iport2uport(VTSS_PORT_NO_START), iport2uport(VTSS_PORT_NO_END)) == VTSS_RC_OK) {
                if (validate_stack_port(s->isid, val)) {
                    state->conf.port_0 = val;
                } else {
                    T_E("Invalid stack port (%u) for switch #%u", val, topo_isid2usid(s->isid));
                    s->rc = CONF_XML_ERROR_FILE_PARM;
                }
                state->conf.port_0 = val;
            } else {
                s->ignored = 1;
            }
            break;
        case CX_TAG_PORT_1:
            if (!global && cx_parse_val_ulong(s, &val, iport2uport(VTSS_PORT_NO_START), iport2uport(VTSS_PORT_NO_END)) == VTSS_RC_OK) {
                if (validate_stack_port(s->isid, val)) {
                    state->conf.port_1 = val;
                } else {
                    T_E("Invalid stack port (%u) for switch #%u", val, topo_isid2usid(s->isid));
                    s->rc = CONF_XML_ERROR_FILE_PARM;
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
        } 
    case CX_PARSE_CMD_GLOBAL:
        break;
    case CX_PARSE_CMD_SWITCH:
        if (s->init) {
            BOOL dirty; 
            s->rc = topo_stack_config_get(s->isid, &state->conf, &dirty);
        } else if (s->apply) {
            if ((s->rc = topo_stack_config_set(s->isid, &state->conf)) != VTSS_RC_OK) {
                T_E("Stack config: %s", error_txt(s->rc));
            }
        }
        break;
    default:
        break;
    }

    return s->rc;
}

static vtss_rc sprout_cx_gen_func(cx_get_state_t *s)
{
    switch (s->cmd) {
    case CX_GEN_CMD_GLOBAL:
        break;
    case CX_GEN_CMD_SWITCH:
        
        CX_RC(cx_add_tag_line(s, CX_TAG_STACK, 0));
        {
            topo_switch_list_t *tsl_p;
            topo_switch_t      *ts_p;
            vtss_usid_t         usid = topo_isid2usid(s->isid);

            if ((tsl_p = VTSS_MALLOC(sizeof(topo_switch_list_t))) == NULL) {
                T_W("VTSS_MALLOC() failed, size=%zu", sizeof(topo_switch_list_t));
                return VTSS_UNSPECIFIED_ERROR;
            } else {
                if (topo_switch_list_get(tsl_p) == VTSS_RC_OK) {
                    int i;
                    for (i = 0; i < ARRSZ(tsl_p->ts); i++) {
                        if (!tsl_p->ts[i].vld) {
                            break;
                        }

                        ts_p = &tsl_p->ts[i];
                        if (ts_p->usid == usid) {
                            vtss_rc rc;
                            T_I("Prio = %d, usid = %d, isid = %d", ts_p->mst_elect_prio, usid, s->isid);
                            rc = cx_add_val_ulong(s, CX_TAG_MST_PRIO, ts_p->mst_elect_prio, TOPO_PARM_MST_ELECT_PRIO_MIN, TOPO_PARM_MST_ELECT_PRIO_MAX);
                            if (rc != VTSS_RC_OK) {
                                VTSS_FREE(tsl_p);
                                return rc;
                            }
                        }
                    }
                }
            }
            VTSS_FREE(tsl_p);
        }
        {
            stack_config_t conf;
            BOOL           dirty;
            vtss_rc        rc;
            if ((rc = topo_stack_config_get(s->isid, &conf, &dirty)) == VTSS_RC_OK) {
                CX_RC(cx_add_val_bool(s, CX_TAG_STACKING, conf.stacking));
                CX_RC(cx_add_val_ulong(s, CX_TAG_PORT_0, conf.port_0, iport2uport(VTSS_PORT_NO_START), iport2uport(VTSS_PORT_NO_END - 1)));
                CX_RC(cx_add_val_ulong(s, CX_TAG_PORT_1, conf.port_1, iport2uport(VTSS_PORT_NO_START), iport2uport(VTSS_PORT_NO_END - 1)));
            } else {
                T_W("Config get failed: %s", error_txt(rc));
            }

        }
        CX_RC(cx_add_tag_line(s, CX_TAG_STACK, 1));
        break;
    default:
        T_E("Unknown command");
        return VTSS_RC_ERROR;
        break;
    } 

    return VTSS_RC_OK;
}


CX_MODULE_TAB_ENTRY(VTSS_MODULE_ID_SPROUT, sprout_cx_tag_table,
                    sizeof(stack_cx_set_state_t), 0,
                    NULL,                    
                    sprout_cx_gen_func,       
                    sprout_cx_parse_func);    

