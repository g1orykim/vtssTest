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
#include "daylight_saving_api.h"

#include "conf_xml_api.h"
#include "conf_xml_trace_def.h"

/* Tag IDs */
enum {
    /* Module tags */
    CX_TAG_TIME,

    /* Parameter tags */
    CX_TAG_DST_MODE,
    CX_TAG_DST_START_TIME,
    CX_TAG_DST_END_TIME,
    CX_TAG_DST_OFFSET,
    CX_TAG_ACRONYM,
    CX_TAG_TIMEZONE,

    /* Last entry */
    CX_TAG_NONE
};

/* Tag table */
static cx_tag_entry_t time_cx_tag_table[CX_TAG_NONE + 1] = {
    [CX_TAG_TIME] = {
        .name  = "time",
        .descr = "Time settings",
        .type = CX_TAG_TYPE_MODULE
    },
    [CX_TAG_TIMEZONE] = {
        .name  = "timezone",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_ACRONYM] = {
        .name  = "acronym",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_DST_MODE] = {
        .name  = "daylight_saving_mode",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_DST_START_TIME] = {
        .name  = "start_time",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_DST_END_TIME] = {
        .name  = "end_time",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_DST_OFFSET] = {
        .name  = "offset",
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

static vtss_rc time_cx_parse_func(cx_set_state_t *s)
{
    switch (s->cmd) {
    case CX_PARSE_CMD_PARM: {
        time_conf_t     conf;
        long            tz;
        long            mode;
        long            offset;
        int             rc;
        char            buf[256];
        BOOL            global;

        global = (s->isid == VTSS_ISID_GLOBAL);

        T_D("id = %u, apply = %d, global = %d, isid = %u", s->id, s->apply, global, s->isid );

        if (!global) {
            s->ignored = 1;
            break;
        }

        if (s->apply && time_dst_get_config(&conf) != VTSS_OK) {
            break;
        }

        switch (s->id) {
        case CX_TAG_DST_MODE:
            if (cx_parse_val_long(s, &mode, 0, 2) == VTSS_OK) {
                conf.dst_mode = mode;
                T_N("mode = %lu", mode);
            }
            break;
        case CX_TAG_DST_START_TIME: {
            /*  syntax:
              *  week/day/month/date/year/hour/minute
              *  x/x/xx/xx/xxxx/xx/xx
              */
            int _week = 0, _day = 0, _month = 0, _date = 0, _year = 0, _hour = 0, _minute = 0;
            if (cx_parse_val_txt(s, buf, 24) == VTSS_OK) {
                if ((rc = sscanf(buf, "%d/%d/%d/%d/%d/%d/%d", &_week, &_day, &_month, &_date, &_year, &_hour, &_minute)) != 7) {
                    T_E("parse error, rc = %d, buf = '%s'", rc, buf);
                }
            }

            conf.dst_start_time.week = _week;
            conf.dst_start_time.day = _day;
            conf.dst_start_time.month = _month;
            conf.dst_start_time.date = _date;
            conf.dst_start_time.year = _year;
            conf.dst_start_time.hour = _hour;
            conf.dst_start_time.minute = _minute;
            T_N("%d/%d/%d/%d/%d/%d/%d", _week, _day, _month, _date, _year, _hour, _minute);
        }
        break;
        case CX_TAG_DST_END_TIME: {
            /*  syntax:
              *  week/day/month/date/year/hour/minute
              *  x/x/xx/xx/xxxx/xx/xx
              */
            int _week = 0, _day = 0, _month = 0, _date = 0, _year = 0, _hour = 0, _minute = 0;
            if (cx_parse_val_txt(s, buf, 24) == VTSS_OK) {
                if ((rc = sscanf(buf, "%d/%d/%d/%d/%d/%d/%d", &_week, &_day, &_month, &_date, &_year, &_hour, &_minute)) != 7) {
                    T_E("parse error, rc = %d, buf = '%s'", rc, buf);
                }
            }

            conf.dst_end_time.week = _week;
            conf.dst_end_time.day = _day;
            conf.dst_end_time.month = _month;
            conf.dst_end_time.date = _date;
            conf.dst_end_time.year = _year;
            conf.dst_end_time.hour = _hour;
            conf.dst_end_time.minute = _minute;
            T_N("%d/%d/%d/%d/%d/%d/%d", _week, _day, _month, _date, _year, _hour, _minute);
        }
        break;
        case CX_TAG_DST_OFFSET:
            if (cx_parse_val_long(s, &offset, 1, 1440) == VTSS_OK) {
                conf.dst_offset = offset;
            } else {
                conf.dst_offset = 1;
            }
            T_N("offset = %u", conf.dst_offset);
            break;
        case CX_TAG_TIMEZONE:
            if (cx_parse_val_long(s, &tz, -7200, 7201) == VTSS_OK) {
                conf.tz_offset = tz / 10;
                conf.tz = tz;
                T_N("tz = %lu", tz);
            }
            break;
        case CX_TAG_ACRONYM:
            if (cx_parse_val_txt(s, buf, 17) == VTSS_OK) {
                strcpy(conf.tz_acronym, buf);
            }
            break;
        default:
            s->ignored = 1;
            break;
        }
        if (s->apply) {
            CX_RC(time_dst_set_config(&conf));
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

static vtss_rc time_cx_gen_func(cx_get_state_t *s)
{
    char           buf[128];
    char           buf_time[128];

    switch (s->cmd) {
    case CX_GEN_CMD_GLOBAL:
        /* Global - Time */
        T_D("global - time");
        CX_RC(cx_add_tag_line(s, CX_TAG_TIME, 0));
        {
            time_conf_t conf;

            if (time_dst_get_config(&conf) == VTSS_OK) {
                CX_RC(cx_add_val_long(s, CX_TAG_TIMEZONE, conf.tz, "-7200-7201"));
                T_N("tz = %u", conf.tz);

                sprintf(buf, "0-16 characters");
                CX_RC(cx_add_val_txt(s, CX_TAG_ACRONYM, conf.tz_acronym, buf));

                sprintf(buf, "0:disabled, 1:recurring, 2:non-recurring");
                CX_RC(cx_add_val_long(s, CX_TAG_DST_MODE, conf.dst_mode, buf));
                T_N("dst_mode = %u", conf.dst_mode);

                sprintf(buf_time, "%u/%u/%u/%u/%u/%u/%u", conf.dst_start_time.week, conf.dst_start_time.day, conf.dst_start_time.month, conf.dst_start_time.date, conf.dst_start_time.year, conf.dst_start_time.hour, conf.dst_start_time.minute);
                sprintf(buf, "week/day/month/date/year/hour/minute");
                CX_RC(cx_add_val_txt(s, CX_TAG_DST_START_TIME, buf_time, buf));
                T_N("start time = %s", buf_time);

                sprintf(buf_time, "%u/%u/%u/%u/%u/%u/%u", conf.dst_end_time.week, conf.dst_end_time.day, conf.dst_end_time.month, conf.dst_end_time.date, conf.dst_end_time.year, conf.dst_end_time.hour, conf.dst_end_time.minute);
                sprintf(buf, "week/day/month/date/year/hour/minute");
                CX_RC(cx_add_val_txt(s, CX_TAG_DST_END_TIME, buf_time, buf));
                T_N("end time = %s", buf_time);

                CX_RC(cx_add_val_long(s, CX_TAG_DST_OFFSET, conf.dst_offset, "1-1440"));
                T_N("dst_offset = %u", conf.dst_offset);
            }
        }
        CX_RC(cx_add_tag_line(s, CX_TAG_TIME, 1));
        break;
    case CX_GEN_CMD_SWITCH:
        break;
    default:
        T_E("Unknown command");
        return VTSS_RC_ERROR;
    } /* End of switch */

    return VTSS_OK;
}

/* Register the info in to the cx_module_table */
CX_MODULE_TAB_ENTRY(
    VTSS_MODULE_ID_DAYLIGHT_SAVING,
    time_cx_tag_table,
    0,
    0,
    NULL,                       /* init function */
    time_cx_gen_func,           /* Generation fucntion */
    time_cx_parse_func          /* parse fucntion */
);

