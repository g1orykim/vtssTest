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
#include "led_pow_reduc_api.h"
#include "conf_xml_api.h"
#include "conf_xml_trace_def.h"
#include "led_pow_reduc_custom_api.h"
#include "mgmt_api.h"
#include "misc_api.h"
/* Tag IDs */
enum {
    /* Module tags */
    CX_TAG_LED_POW_REDUC,

    /* Group tags */
    CX_TAG_TIMER_TABLE,

    CX_TAG_ENTRY,
    CX_TAG_LED_POW_REDUC_ON_AT_ERR,
    CX_TAG_LED_POW_REDUC_MAINTENANCE_TIME,


    /* Last entry */
    CX_TAG_NONE
};

/* Tag table */
static cx_tag_entry_t led_pow_reduc_cx_tag_table[CX_TAG_NONE + 1] = {
    [CX_TAG_LED_POW_REDUC] = {
        .name  = "LED_POW_REDUC",
        .descr = "Energy Efficient Ethernet",
        .type = CX_TAG_TYPE_MODULE
    },
    [CX_TAG_ENTRY] = {
        .name  = "entry",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_LED_POW_REDUC_ON_AT_ERR] = {
        .name  = "on_at_err",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_LED_POW_REDUC_MAINTENANCE_TIME] = {
        .name  = "maintenance_time",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_TIMER_TABLE] = {
        .name  = "timer_table",
        .descr = "",
        .type = CX_TAG_TYPE_GROUP
    },

    /* Last entry */
    [CX_TAG_NONE] = {
        .name  = "",
        .descr = "",
        .type = CX_TAG_TYPE_NONE
    }
};


/* LED_POW_REDUC specific set state structure */
typedef struct {
    cx_line_t  line_led_pow_reduc;  /* LED_POW_REDUC line */
} led_pow_reduc_cx_set_state_t;


static BOOL cx_led_pow_reduc_match(const cx_table_context_t *context, ulong timer_a, ulong timer_b)
{
    led_pow_reduc_local_conf_t conf;
    led_pow_reduc_mgmt_get_switch_conf(&conf);
    T_DG(VTSS_TRACE_GRP_LED_POW_REDUC, "conf.glbl_conf.led_timer[%u]:%d, conf.glbl_conf.led_timer[%u]:%d", timer_a, conf.glbl_conf.led_timer_intensity[timer_a], timer_b, conf.glbl_conf.led_timer_intensity[timer_b]);
    T_NG(VTSS_TRACE_GRP_LED_POW_REDUC, "conf.glbl_conf.led_timer_intensity[%u]:%d, conf.glbl_conf.led_timer_intensity[%u]:%d", timer_a, conf.glbl_conf.led_timer_intensity[timer_a], timer_b, conf.glbl_conf.led_timer_intensity[timer_b]);
    return conf.glbl_conf.led_timer_intensity[timer_a]  == conf.glbl_conf.led_timer_intensity[timer_b];
}

static vtss_rc cx_led_pow_reduc_print(cx_get_state_t *s, const cx_table_context_t *context, ulong led_timer, char *txt)
{
    led_pow_reduc_local_conf_t conf;
    char buf[80];

    if (txt == NULL) {
        // Syntax
        CX_RC(cx_add_stx_start(s));
        CX_RC(cx_add_attr_txt(s, "time", cx_list_txt(buf, LED_POW_REDUC_TIMERS_MIN, LED_POW_REDUC_TIMERS_MAX)));
        CX_RC(cx_add_stx_ulong(s, "intensity", LED_POW_REDUC_INTENSITY_MIN, LED_POW_REDUC_INTENSITY_MAX));
        return cx_add_stx_end(s);
    }

    led_pow_reduc_mgmt_get_switch_conf(&conf);

    // Only add timers used
    T_IG(VTSS_TRACE_GRP_LED_POW_REDUC, "led_timer intensity:%u", conf.glbl_conf.led_timer_intensity[led_timer]);
    CX_RC(cx_add_attr_start(s, CX_TAG_ENTRY));
    CX_RC(cx_add_attr_txt(s, "time", txt));
    CX_RC(cx_add_attr_ulong(s, "intensity", conf.glbl_conf.led_timer_intensity[led_timer]));


    CX_RC(cx_add_attr_end(s, CX_TAG_ENTRY));
    return VTSS_OK;
}



static vtss_rc led_pow_reduc_cx_parse_func(cx_set_state_t *s)
{
    BOOL           global;
    ulong          val;
    BOOL           timer_list[LED_POW_REDUC_TIMERS_CNT];
    led_pow_reduc_local_conf_t conf;
    u16            timer_idx;
    u32            intensity = 0;
    u32            timer_time = 0;
    char           buf[64];
    BOOL           found_intensity = FALSE;
    switch (s->cmd) {
    case CX_PARSE_CMD_PARM:
        global = (s->isid == VTSS_ISID_GLOBAL);

        if (s->mod_tag == CX_TAG_LED_POW_REDUC) {

            if (s->apply) {
                led_pow_reduc_mgmt_get_switch_conf(&conf);
            }

            switch (s->id) {
            case CX_TAG_TIMER_TABLE:
                if (!global) {
                    s->ignored = 1;
                }
                break;
            case CX_TAG_ENTRY:
                CX_RC(cx_parse_txt(s, "time", buf, sizeof(buf)));
                if (global && s->group == CX_TAG_TIMER_TABLE &&
                    mgmt_txt2list(buf, &timer_list[0], LED_POW_REDUC_TIMERS_MIN, LED_POW_REDUC_TIMERS_MAX, 0) == VTSS_OK) {


                    s->p = s->next;
                    for (; s->rc == VTSS_OK && cx_parse_attr(s) == VTSS_OK; s->p = s->next) {
                        if (cx_parse_ulong(s, "intensity", &intensity, LED_POW_REDUC_INTENSITY_MIN, LED_POW_REDUC_INTENSITY_MAX) == VTSS_OK) {
                            found_intensity = TRUE;
                        }
                    }

                    T_IG(VTSS_TRACE_GRP_LED_POW_REDUC, "found_intensity:%d, timer_time:%u, intensity:%u, s->apply:%d",
                         found_intensity, timer_time, intensity, s->apply);

                    for (timer_idx = LED_POW_REDUC_TIMERS_MIN; timer_idx <= LED_POW_REDUC_TIMERS_MAX; timer_idx++) {
                        if (s->apply && timer_list[timer_idx]) {
                            T_IG(VTSS_TRACE_GRP_LED_POW_REDUC, "found_intensity:%d, timer_time:%u, intensity:%u, timer_idx:%d",
                                 found_intensity, timer_time, intensity, timer_idx);

                            if (found_intensity) {
                                conf.glbl_conf.led_timer_intensity[timer_idx] = (u8) intensity;
                            }
                        }
                    }
                } else {

                    s->ignored = 1;
                }
                break;
            case CX_TAG_LED_POW_REDUC_ON_AT_ERR:
                if (global) {
                    CX_RC(cx_parse_val_bool(s, &conf.glbl_conf.on_at_err, 1));
                } else {
                    s->ignored = 1;
                }
                break;

            case CX_TAG_LED_POW_REDUC_MAINTENANCE_TIME:
                if (global && cx_parse_val_ulong(s, &val, LED_POW_REDUC_MAINTENANCE_TIME_MIN, LED_POW_REDUC_MAINTENANCE_TIME_MAX) == VTSS_OK) {
                    conf.glbl_conf.maintenance_time = val;
                } else {
                    s->ignored = 1;
                }
                break;
            default:
                s->ignored = 1;
                break;
            }

            if (s->apply) {
                if (global) {
                    if (led_pow_reduc_mgmt_set_switch_conf(&conf) != VTSS_OK) {
                        T_W("Could not set LED_POW_REDUC configuration");
                    }
                } else {
                }
            }
            break;
        } // CX_PARSE_CMD_PARM
    default:
        break;
    }
    return s->rc;
//    return VTSS_OK;
}

static BOOL cx_skip_none(ulong n)
{
    return 0;
}

static vtss_rc led_pow_reduc_cx_gen_func(cx_get_state_t *s)
{
    led_pow_reduc_local_conf_t conf;
    led_pow_reduc_mgmt_get_switch_conf(&conf);

    switch (s->cmd) {
    case CX_GEN_CMD_GLOBAL:
        /* Global - LED_POW_REDUC */
        CX_RC(cx_add_tag_line(s, CX_TAG_LED_POW_REDUC, 0));

        CX_RC(cx_add_val_bool(s, CX_TAG_LED_POW_REDUC_ON_AT_ERR, conf.glbl_conf.on_at_err));
        CX_RC(cx_add_val_ulong(s, CX_TAG_LED_POW_REDUC_MAINTENANCE_TIME, conf.glbl_conf.maintenance_time,
                               LED_POW_REDUC_MAINTENANCE_TIME_MIN,
                               LED_POW_REDUC_MAINTENANCE_TIME_MAX));

        CX_RC(cx_add_tag_line(s, CX_TAG_TIMER_TABLE, 0));
        CX_RC(cx_add_table(s, NULL,
                           LED_POW_REDUC_TIMERS_MIN, LED_POW_REDUC_TIMERS_MAX,
                           cx_skip_none, cx_led_pow_reduc_match, cx_led_pow_reduc_print, FALSE));
        CX_RC(cx_add_tag_line(s, CX_TAG_TIMER_TABLE, 1));


        CX_RC(cx_add_tag_line(s, CX_TAG_LED_POW_REDUC, 1));

        break;
    case CX_GEN_CMD_SWITCH:
        /* Switch - LED_POW_REDUC */
        break;
    default:
        T_E("Unknown command");
        return VTSS_RC_ERROR;
    } /* End of Switch */

    return VTSS_OK;
}

/* Register the info in to the cx_module_table */
CX_MODULE_TAB_ENTRY(
    VTSS_MODULE_ID_LED_POW_REDUC,
    led_pow_reduc_cx_tag_table,
    sizeof(led_pow_reduc_cx_set_state_t),
    0,
    NULL,                  /* Init function       */
    led_pow_reduc_cx_gen_func,       /* Generation function */
    led_pow_reduc_cx_parse_func      /* Parse function      */
);
