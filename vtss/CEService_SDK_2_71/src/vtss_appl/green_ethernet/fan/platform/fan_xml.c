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
#include "fan_api.h"
#include "conf_xml_api.h"
#include "conf_xml_trace_def.h"
#include "mgmt_api.h"
#include "misc_api.h"
/* Tag IDs */
enum {
    /* Module tags */
    CX_TAG_FAN,

    /* Group tags */
    CX_TAG_PORT_TABLE,

    CX_TAG_T_ON,
    CX_TAG_T_MAX,

    /* Last entry */
    CX_TAG_NONE
};

/* Tag table */
static cx_tag_entry_t fan_cx_tag_table[CX_TAG_NONE + 1] = {
    [CX_TAG_FAN] = {
        .name  = "FAN",
        .descr = "Energy Efficient Ethernet",
        .type = CX_TAG_TYPE_MODULE
    },
    [CX_TAG_PORT_TABLE] = {
        .name  = "port_table",
        .descr = "",
        .type = CX_TAG_TYPE_GROUP
    },
    [CX_TAG_T_ON] = {
        .name  = "t_on",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_T_MAX] = {
        .name  = "t_max",
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


/* FAN specific set state structure */
typedef struct {
    cx_line_t  line_fan;  /* FAN line */
    i8 t_on;
    i8 t_max;
} fan_cx_set_state_t;

static vtss_rc fan_cx_parse_func(cx_set_state_t *s)
{
    switch (s->cmd) {
    case CX_PARSE_CMD_PARM: {
        BOOL           global;
        long          val;
        fan_cx_set_state_t *fan_state = s->mod_state;
        global = (s->isid == VTSS_ISID_GLOBAL);

        if (s->mod_tag == CX_TAG_FAN) {
            switch (s->id) {
            case CX_TAG_PORT_TABLE:
                if (global) {
                    s->ignored = 1;
                }
                break;

            case CX_TAG_T_ON:
                if (global && cx_parse_val_long(s, &val, FAN_TEMP_MIN, FAN_TEMP_MAX) == VTSS_OK) {
                    fan_state->t_on = val;
                }
                break;

            case CX_TAG_T_MAX:
                if (global && cx_parse_val_long(s, &val, FAN_TEMP_MIN, FAN_TEMP_MAX) == VTSS_OK) {
                    fan_state->t_max = val;
                }
                break;

            default:
                s->ignored = 1;
                break;
            }
            break;

        } /* CX_TAG_FAN */
    }

    case CX_PARSE_CMD_GLOBAL:
        break;
    case CX_PARSE_CMD_SWITCH: {
        fan_cx_set_state_t *fan_state = s->mod_state;
        fan_local_conf_t   fan_conf;
        fan_mgmt_get_switch_conf(&fan_conf);
        if (s->init) {
            fan_state->line_fan.number = 0;
            fan_state->t_on = fan_conf.glbl_conf.t_on;
            fan_state->t_max = fan_conf.glbl_conf.t_max;

        } else if (s->apply) {
            fan_conf.glbl_conf.t_on = fan_state->t_on;
            fan_conf.glbl_conf.t_max = fan_state->t_max;
            CX_RC(fan_mgmt_set_switch_conf(&fan_conf));
        } else {
            if (fan_state->t_max < fan_state->t_on) {
                CX_RC(cx_parm_error(s, "Fan t_max must be higher then t_on"));
            }
        }
        break;
    }
    default:
        break;
    }
    return s->rc;
}


static vtss_rc fan_cx_gen_func(cx_get_state_t *s)
{
    fan_local_conf_t     fan_conf;
    char string_buf[100];
    switch (s->cmd) {
    case CX_GEN_CMD_GLOBAL:
        break;
    case CX_GEN_CMD_SWITCH:
        /* Switch - FAN */
        CX_RC(cx_add_tag_line(s, CX_TAG_FAN, 0));
        /* Global - FAN */
        fan_mgmt_get_switch_conf(&fan_conf);
        sprintf(string_buf, "%d-%d", FAN_TEMP_MIN, FAN_TEMP_MAX);
        CX_RC(cx_add_val_long(s, CX_TAG_T_ON,  fan_conf.glbl_conf.t_on, &string_buf[0]));
        CX_RC(cx_add_val_long(s, CX_TAG_T_MAX,  fan_conf.glbl_conf.t_max, &string_buf[0]));
        CX_RC(cx_add_tag_line(s, CX_TAG_FAN, 1));

        break;
    default:
        T_E("Unknown command");
        return VTSS_RC_ERROR;
    } /* End of Switch */

    return VTSS_OK;
}

/* Register the info in to the cx_module_table */
CX_MODULE_TAB_ENTRY(
    VTSS_MODULE_ID_FAN,
    fan_cx_tag_table,
    sizeof(fan_cx_set_state_t),
    0,
    NULL,                  /* Init function       */
    fan_cx_gen_func,       /* Generation function */
    fan_cx_parse_func      /* Parse function      */
);
