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

 $Id$
 $Revision$

*/

#include "main.h"
#include "conf_api.h"
#include "msg_api.h"
#include "critd_api.h"
#include "time.h"
#include "misc_api.h"

#include "daylight_saving_api.h"
#include "daylight_saving.h"

#if defined(VTSS_SW_OPTION_SYSUTIL)
#include "sysutil_api.h"
#endif /* VTSS_SW_OPTION_SYSUTIL */

#ifdef VTSS_SW_OPTION_ICFG
#include "daylight_saving_icfg.h"
#endif


#if 0
#include "main.h"
#include "conf_api.h"
#include "msg_api.h"
#include "critd_api.h"
#include "vtss_ecos_mutex_api.h"
#include "sysutil_api.h"
#include "time.h"
#ifdef VTSS_SW_OPTION_LLDP
#include "lldp_api.h"
#endif /* VTSS_SW_OPTION_LLDP */
#include "sysutil.h"
#ifdef VTSS_SW_OPTION_AUTH
#include "vtss_auth_api.h"
#endif /* VTSS_SW_OPTION_AUTH */
#ifdef VTSS_SW_OPTION_VCLI
#include "cli.h"
void system_cli_req_init(void);
#endif
#endif

/****************************************************************************/
/*  Global variables                                                        */
/****************************************************************************/

/* Global structure */
static time_global_t dst_global;
static time_t dst_start_utc_time;
static time_t dst_end_utc_time;

#if (VTSS_TRACE_ENABLED)
/* Trace registration. Initialized by time_dst_init() */
static vtss_trace_reg_t trace_reg = {
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "time_dst",
    .descr     = "time (configuration)"
};

static vtss_trace_grp_t trace_grps[TRACE_GRP_CNT] = {
    [VTSS_TRACE_GRP_DEFAULT] = {
        .name      = "default",
        .descr     = "Default",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [TRACE_GRP_CRIT] = {
        .name      = "crit",
        .descr     = "Critical regions ",
        .lvl       = VTSS_TRACE_LVL_ERROR,
        .timestamp = 1,
    }
};

#define TIME_DST_CRIT_ENTER() critd_enter(&dst_global.crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define TIME_DST_CRIT_EXIT()  critd_exit( &dst_global.crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#else
#define TIME_DST_CRIT_ENTER() critd_enter(&dst_global.crit)
#define TIME_DST_CRIT_EXIT()  critd_exit( &dst_global.crit)
#endif /* VTSS_TRACE_ENABLED */

#define SECSPERMIN          60
#define MINSPERHOUR         60
#define HOURSPERDAY         24
#define DAYSPERWEEK         7
#define DAYSPERNYEAR        365
#define DAYSPERLYEAR        366
#define SECSPERHOUR         (SECSPERMIN * MINSPERHOUR)
#define SECSPERDAY          ((u32) SECSPERHOUR * HOURSPERDAY)
#define MONSPERYEAR         12
#define EPOCH_YEAR          1970

#define IS_LEAP(y) (((y) % 4) == 0 && (((y) % 100) != 0 || ((y) % 400) == 0))

static const int dst_mon_lengths[2][MONSPERYEAR] = {
    { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
    { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
};

static const int dst_year_lengths[2] = {
    DAYSPERNYEAR, DAYSPERLYEAR
};

typedef struct {
    uchar   month;
    uchar   date;
    ushort  year;
    uchar   hour;
    uchar   minute;
    u32     dst_offset; /* 1 - 1440 minutes */
    int     tz_offset;  /* +- 720 minutes   */
} time_dst_non_recurring_utc_t;

typedef struct {
    ushort  year;
    uchar   month;
    uchar   week;
    ushort  day;
    uchar   hour;
    uchar   minute;
    u32     dst_offset; /* 1 - 1440 minutes */
    int     tz_offset;  /* +- 720 minutes   */
} time_dst_recurring_utc_t;

/****************************************************************************/
/*  Various local functions                                                 */
/****************************************************************************/
/* set timezone offset on system timezone offset to update the value on old parameter */
static vtss_rc _time_dst_set_tz_offset(int tz_off)
{
    vtss_rc       rc = VTSS_OK;
    system_conf_t conf;

    if ((rc = system_get_config(&conf)) != VTSS_OK) {
        return rc;
    }

    conf.tz_off = tz_off;
    rc = system_set_config(&conf);

    return rc;
}

/* Determine if daylight saving configuration has changed */
static int _time_dst_conf_changed(time_conf_t *old, time_conf_t *new)
{
    return (new->dst_mode != old->dst_mode ||
            memcmp(&new->dst_start_time, &old->dst_start_time, sizeof(time_dst_cfg_t)) ||
            memcmp(&new->dst_end_time, &old->dst_end_time, sizeof(time_dst_cfg_t)) ||
            new->dst_offset != old->dst_offset ||
            strcmp(new->tz_acronym, old->tz_acronym) ||
            new->tz_offset != old->tz_offset ||
            new->tz != old->tz);
}

/* daylight saving time, Set system defaults */
static void _time_dst_default_set(time_conf_t *conf)
{
    conf->dst_mode = TIME_DST_DISABLED;
    memset(&conf->dst_start_time, 0, sizeof(time_dst_cfg_t));
    memset(&conf->dst_end_time, 0, sizeof(time_dst_cfg_t));
    conf->dst_offset = 1;
    memset(conf->tz_acronym, 0x0, 16);
    //conf->tz_acronym[0] = '\0';
    conf->tz_offset = 0x0;
    conf->tz = 0;
}

/* daylight saving time, none recurring convert */
static u32 _time_dst_non_recurring_utc_convert(time_dst_non_recurring_utc_t *cfg)
{
    /*
     * The function receives the year, month, date, hour, minute, time zone offset and
     * DST offset as input and calculate the UTC time as output.When calculating
     * the start time of DST, the Standard Time is used so dst_offset must be 0.
     * When calculating the end time of DST, the DST time is used so the dst_offset
     * must be the configured value, non-zero.
     */

    time_t     value = 0;
    u16     year, month, leapyear;

    /* Don't use trace api T_D/E/W here becasue it thos api might
     * call to this function to get time and it will cause infinit loop
     */

    //printf("cfg->year: %u\n", cfg->year);
    //printf("cfg->month: %u\n", cfg->month);
    //printf("cfg->date: %u\n", cfg->date);
    //printf("cfg->hour: %u\n", cfg->hour);
    //printf("cfg->minute: %u\n", cfg->minute);
    //printf("cfg->tz_offset: %u\n", cfg->tz_offset);
    //printf("cfg->dst_offset: %u\n", cfg->dst_offset);

    /* year */
    for (year = EPOCH_YEAR; year < cfg->year; ++year) {
        leapyear = IS_LEAP(year);
        value += dst_year_lengths[leapyear] * SECSPERDAY;
    }

    //printf("year: %d\n",value);

    /* month */
    for (month = 0; month < (cfg->month - 1); ++month) {
        leapyear = IS_LEAP(cfg->year);
        value += dst_mon_lengths[leapyear][month] * SECSPERDAY;
    }
    //printf("month: %d\n",value);

    /* date */
    value += (cfg->date - 1) * SECSPERDAY;
    //printf("date: %d\n",value);

    /* hour */
    value += (cfg->hour) * SECSPERHOUR;
    //printf("hour: %d\n",value);

    /* minute */
    value += (cfg->minute) * SECSPERMIN;
    //printf("minute: %d\n",value);

    /* time zone. For example, the time zone is +8:00 in TW so when
     * it is 08:00 am in TW, it is 00:00 am UTC time. We need to
     * subtract the tz_offset to get the real UTC time.
     */
    value -= (cfg->tz_offset) * SECSPERMIN;
    //printf("timezone: %d\n",value);

    /* dst offset. For example, if the dst offset is 60 minutes in TW,
     * it means the time is adjusted to 60 minutes forward. When it is
     * 09:00 am in TW, it is 00:00 am UTC time. We need to subrtact
     * the dst offset to get the real UTC time.
     */
    value -= (cfg->dst_offset) * SECSPERMIN;
    //printf("dst_offset: %d\n",value);

    return value;
}

/* daylight saving time, recurring convert */
static u32 _time_dst_recurring_utc_convert(time_dst_recurring_utc_t *cfg)
{
    int     leapyear;
    time_t  value = 0;
    int     i;
    u16     year;
    int     d, m1, yy0, yy1, yy2, dow;

    /* Don't use trace api T_D/E/W here becasue it thos api might
     * call to this function to get time and it will cause infinit loop
     */

    //printf("cfg->year: %u\n", cfg->year);
    //printf("cfg->month: %u\n", cfg->month);
    //printf("cfg->week: %u\n", cfg->week);
    //printf("cfg->day: %u\n", cfg->day);
    //printf("cfg->hour: %u\n", cfg->hour);
    //printf("cfg->minute: %u\n", cfg->minute);
    //printf("cfg->tz_offset: %u\n", cfg->tz_offset);
    //printf("cfg->dst_offset: %u\n", cfg->dst_offset);

    /*
    ** Mm.n.d - nth "dth day" of month m.
    */
    /* year */
    for (year = EPOCH_YEAR; year < cfg->year; ++year) {
        leapyear = IS_LEAP(year);
        value += dst_year_lengths[leapyear] * SECSPERDAY;

    }
    //printf("year: %d\n",value);

    leapyear = IS_LEAP(year);

    for (i = 0; i < cfg->month - 1; ++i) {
        value += dst_mon_lengths[leapyear][i] * SECSPERDAY;
    }
    //printf("month: %d\n",value);

    /*
    ** Use Zeller's Congruence to get day-of-week of first day of
    ** month.
    */
    m1 = (cfg->month + 9) % 12 + 1;
    yy0 = (cfg->month <= 2) ? (cfg->year - 1) : cfg->year;
    yy1 = yy0 / 100;
    yy2 = yy0 % 100;
    dow = ((26 * m1 - 2) / 10 +
           1 + yy2 + yy2 / 4 + yy1 / 4 - 2 * yy1) % 7;
    if (dow < 0) {
        dow += DAYSPERWEEK;
    }

    /*
    ** "dow" is the day-of-week of the first day of the month. Get
    ** the day-of-month (zero-origin) of the first "dow" day of the
    ** month.
    */
    d = cfg->day - dow;
    if (d < 0) {
        d += DAYSPERWEEK;
    }
    for (i = 1; i < cfg->week; ++i) {
        if (d + DAYSPERWEEK >=
            dst_mon_lengths[leapyear][cfg->month - 1]) {
            break;
        }
        d += DAYSPERWEEK;
    }

    /*
    ** "d" is the day-of-month (zero-origin) of the day we want.
    */
    value += d * SECSPERDAY;
    //printf("day: %d\n",value);

    /* hour */
    value += (cfg->hour) * SECSPERHOUR;
    //printf("hour: %d\n",value);

    /* minute */
    value += (cfg->minute) * SECSPERMIN;
    //printf("minute: %d\n",value);

    /* time zone. For example, the time zone is +8:00 in TW so when
     * it is 08:00 am in TW, it is 00:00 am UTC time. We need to
     * subtract the tz_offset to get the real UTC time.
     */
    value -= (cfg->tz_offset) * SECSPERMIN;
    //printf("timezone: %d\n",value);

    /* dst offset. For example, if the dst offset is 60 minutes in TW,
     * it means the time is adjusted to 60 minutes forward. When it is
     * 09:00 am in TW, it is 00:00 am UTC time. We need to subrtact
     * the dst offset to get the real UTC time.
     */
    value -= (cfg->dst_offset) * SECSPERMIN;
    //printf("dst_offset: %d\n",value);

    return value;
}

/* daylight saving time, recurring time set */
static void _time_dst_recurring_time_set(time_conf_t *conf)
{
    /*
     * The function receives the year, month, week, day, hour, minute, time zone offset and
     * DST offset as input and calculate the UTC time as output.When calculating
     * the start time of DST, the Standard Time is used so dst_offset must be 0.
     * When calculating the end time of DST, the DST time is used so the dst_offset
     * must be the configured value, non-zero.
     */

    /* Don't use trace api T_D/E/W here becasue it thos api might
     * call to this function to get time and it will cause infinit loop
     */

    time_dst_recurring_utc_t      utc1_cfg;
    time_t     current_time = time(NULL);
    struct tm  *timeinfo_p;
    u16        year_to_cal;

    timeinfo_p = localtime(&current_time);
    year_to_cal = timeinfo_p->tm_year + 1900;

    /* calculate the end time of DST */
    utc1_cfg.year = year_to_cal;
    utc1_cfg.month = conf->dst_end_time.month;
    utc1_cfg.week = conf->dst_end_time.week;
    utc1_cfg.day = conf->dst_end_time.day;
    utc1_cfg.hour = conf->dst_end_time.hour;
    utc1_cfg.minute = conf->dst_end_time.minute;
    utc1_cfg.tz_offset = conf->tz_offset;
    utc1_cfg.dst_offset = conf->dst_offset; /* calculate DST time */

    dst_end_utc_time = _time_dst_recurring_utc_convert(&utc1_cfg);
    if (current_time > dst_end_utc_time) {
        /* The daylight saving of this year is over and to calculate
         * the duration of next year directly.
         */
        year_to_cal++;
        utc1_cfg.year = year_to_cal;
        dst_end_utc_time = _time_dst_recurring_utc_convert(&utc1_cfg);
    }

    /* calculate the start time of DST */
    utc1_cfg.month = conf->dst_start_time.month;
    utc1_cfg.week = conf->dst_start_time.week;
    utc1_cfg.day = conf->dst_start_time.day;
    utc1_cfg.hour = conf->dst_start_time.hour;
    utc1_cfg.minute = conf->dst_start_time.minute;
    utc1_cfg.tz_offset = conf->tz_offset;
    utc1_cfg.dst_offset = 0; /* calculate Standard time */
    dst_start_utc_time = _time_dst_recurring_utc_convert(&utc1_cfg);
    //printf("start time:%d, end time:%d\n",dst_start_utc_time, dst_end_utc_time);
}

/****************************************************************************/
/*  Various global functions                                                */
/****************************************************************************/

/* daylight saving time, time set */
static void time_dst_time_set(time_conf_t *conf)
{
    time_dst_non_recurring_utc_t  utc_cfg;

    TIME_DST_CRIT_ENTER();
    if (conf->dst_mode == TIME_DST_DISABLED) {
        dst_start_utc_time = 0;
        dst_end_utc_time = 0;
    } else if (conf->dst_mode == TIME_DST_NON_RECURRING) {
        // calculate strat time (UTC)
        utc_cfg.year = conf->dst_start_time.year;
        utc_cfg.month = conf->dst_start_time.month;
        utc_cfg.date = conf->dst_start_time.date;
        utc_cfg.hour = conf->dst_start_time.hour;
        utc_cfg.minute = conf->dst_start_time.minute;
        utc_cfg.dst_offset = 0; /* calculate standard time */
        utc_cfg.tz_offset = conf->tz_offset;
        dst_start_utc_time = _time_dst_non_recurring_utc_convert(&utc_cfg);
        // calculate end time (UTC)
        utc_cfg.year = conf->dst_end_time.year;
        utc_cfg.month = conf->dst_end_time.month;
        utc_cfg.date = conf->dst_end_time.date;
        utc_cfg.hour = conf->dst_end_time.hour;
        utc_cfg.minute = conf->dst_end_time.minute;
        utc_cfg.dst_offset = conf->dst_offset; /* calculate DST time */
        utc_cfg.tz_offset = conf->tz_offset;
        dst_end_utc_time = _time_dst_non_recurring_utc_convert(&utc_cfg);
        //printf("start time:%d, end time:%d\n",dst_start_utc_time, dst_end_utc_time);
    } else if (conf->dst_mode == TIME_DST_RECURRING) {
        _time_dst_recurring_time_set(conf);
    }
    TIME_DST_CRIT_EXIT();

    return;
}

/* daylight saving time, get daylight saving time offset */
u32 time_dst_get_offset(void)
{
    u32     dst_off = 0;
    time_t  current_time = time(NULL);

    /* This is is not protected by TIME_DST_CRIT_ENTER/EXIT, since this would create
       a deadlock for trace statements inside critical regions of this module */

    if (dst_global.system_conf.dst_mode == TIME_DST_DISABLED) {
        dst_off = 0;
    } else if (dst_global.system_conf.dst_mode == TIME_DST_NON_RECURRING) {
        if (current_time >=  dst_start_utc_time && current_time <= dst_end_utc_time) {
            dst_off = dst_global.system_conf.dst_offset;
        } else {
            dst_off = 0;
        }
    } else if (dst_global.system_conf.dst_mode == TIME_DST_RECURRING) {
        if (current_time >  dst_end_utc_time) {
            _time_dst_recurring_time_set(&dst_global.system_conf);
        }
        if (current_time >=  dst_start_utc_time && current_time <= dst_end_utc_time) {
            dst_off = dst_global.system_conf.dst_offset;
        } else {
            dst_off = 0;
        }
    }

    return dst_off;
}

/* Get daylight saving configuration */
vtss_rc time_dst_get_config(time_conf_t *conf)
{
    T_D("enter");

    TIME_DST_CRIT_ENTER();
    *conf = dst_global.system_conf;
    TIME_DST_CRIT_EXIT();

    T_D("exit");

    return VTSS_OK;
}

/* Set daylight saving configuration */
vtss_rc time_dst_set_config(time_conf_t *conf)
{
    vtss_rc rc          = VTSS_OK;
    int     dst_changed = 0;

    T_D("enter");

    TIME_DST_CRIT_ENTER();
    if (msg_switch_is_master()) {
        dst_changed = _time_dst_conf_changed(&dst_global.system_conf, conf);
        dst_global.system_conf = *conf;
    } else {
        T_W("not master");
        rc = TIME_DST_ERROR_STACK_STATE;
    }
    TIME_DST_CRIT_EXIT();

    if (dst_changed) {
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
        /* Save changed configuration */
        conf_blk_id_t       blk_id  = CONF_BLK_DAYLIGHT_SAVING;
        time_dst_conf_blk_t *conf_blk;
        if ((conf_blk = conf_sec_open(CONF_SEC_GLOBAL, blk_id, NULL)) == NULL) {
            T_E("failed to open configuration table");
        } else {
            conf_blk->conf = *conf;
            conf_sec_close(CONF_SEC_GLOBAL, blk_id);
        }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */

        /* Apply daylight saving time to system */
        time_dst_time_set(conf);

        /* Apply tz offset to old parameter (sysutil) */
        rc = _time_dst_set_tz_offset(conf->tz_offset);
    }

    T_D("exit");

    return rc;
}

/* update timezone offset from xml module, for backward compatible issue */
vtss_rc time_dst_update_tz_offset(int tz_off)
{
    vtss_rc       rc = VTSS_OK;
    time_conf_t   conf;

    if ((rc = time_dst_get_config(&conf)) != VTSS_OK) {
        return rc;
    }

    conf.tz_offset = tz_off;
    conf.tz = (tz_off * 10);
    rc = time_dst_set_config(&conf);

    return rc;
}

/****************************************************************************/
/*  Initialization functions                                                */
/****************************************************************************/

/* Read/create system switch configuration */
static vtss_rc time_dst_conf_read_switch(vtss_isid_t isid_add)
{
    BOOL                    do_create;
    ulong                   size;
    vtss_isid_t             isid;
    time_dst_conf_blk_t     *conf_blk_p;
    conf_blk_id_t           blk_id      = CONF_BLK_DAYLIGHT_SAVING;
    ulong                   blk_version = TIME_DST_CONF_BLK_VERSION;
    vtss_rc                 rc = VTSS_OK;

    T_D("enter, isid_add: %d", isid_add);

    if (misc_conf_read_use()) {
        /* read configuration */
        if ((conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, &size)) == NULL ||
            size != sizeof(*conf_blk_p)) {
            T_W("conf_sec_open failed or size mismatch, creating defaults");
            conf_blk_p = conf_sec_create(CONF_SEC_GLOBAL, blk_id, sizeof(*conf_blk_p));
            do_create = 1;
        } else if (conf_blk_p->version != blk_version) {
            T_W("version mismatch, creating defaults");
            do_create = 1;
        } else {
            do_create = (isid_add != VTSS_ISID_GLOBAL);
        }
    } else {
        conf_blk_p = NULL;
        do_create  = TRUE;
    }

    for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
        if (isid_add != VTSS_ISID_GLOBAL && isid_add != isid) {
            continue;
        }

        TIME_DST_CRIT_ENTER();
        if (do_create) {
            /* Use default values */
            _time_dst_default_set(&dst_global.system_conf);
            if (conf_blk_p != NULL) {
                conf_blk_p->conf = dst_global.system_conf;
            }
        } else {
            /* Use new configuration */
            if (conf_blk_p != NULL) {  // Quiet lint
                dst_global.system_conf = conf_blk_p->conf;
            }
        }

        TIME_DST_CRIT_EXIT();
    }

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (conf_blk_p == NULL) {
        T_W("failed to open system table");
    } else {
        conf_blk_p->version = blk_version;
        conf_sec_close(CONF_SEC_GLOBAL, blk_id);
    }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */

    T_D("exit");
    return rc;
}

/* Read/create system stack configuration */
static vtss_rc time_dst_conf_read_stack(BOOL create)
{
    int                     changed;
    BOOL                    do_create;
    ulong                   size;
    time_conf_t             *old_conf_p, new_conf;
    time_dst_conf_blk_t     *conf_blk_p;
    conf_blk_id_t           blk_id      = CONF_BLK_DAYLIGHT_SAVING;
    ulong                   blk_version = TIME_DST_CONF_BLK_VERSION;
    vtss_rc                 rc = VTSS_OK;

    T_D("enter, create: %d", create);

    if (misc_conf_read_use()) {
        /* Read/create configuration */
        if ((conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, &size)) == NULL ||
            size != sizeof(*conf_blk_p)) {
            T_W("conf_sec_open failed or size mismatch, creating defaults");
            conf_blk_p = conf_sec_create(CONF_SEC_GLOBAL, blk_id, sizeof(*conf_blk_p));
            do_create = 1;
        } else if (conf_blk_p->version != blk_version) {
            T_W("version mismatch, creating defaults");
            do_create = 1;
        } else {
            do_create = create;
        }
    } else {
        conf_blk_p = NULL;
        do_create  = TRUE;
    }

    changed = 0;
    TIME_DST_CRIT_ENTER();
    if (do_create) {
        /* Use default values */
        _time_dst_default_set(&new_conf);
        if (conf_blk_p != NULL) {
            conf_blk_p->conf = new_conf;
        }
    } else {
        /* Use new configuration */
        if (conf_blk_p != NULL) {
            new_conf = conf_blk_p->conf;
        }
    }
    old_conf_p = &dst_global.system_conf;
    if (_time_dst_conf_changed(old_conf_p, &new_conf)) {
        changed = 1;
    }

    dst_global.system_conf = new_conf;
    TIME_DST_CRIT_EXIT();

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (conf_blk_p == NULL) {
        T_W("failed to open configuration table");
    } else {
        conf_blk_p->version = blk_version;
        conf_sec_close(CONF_SEC_GLOBAL, blk_id);
    }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */

    if (changed) {
        /* Apply daylight saving time to system */
        time_dst_time_set(&new_conf);

        /* Apply tz offset to old parameter (sysutil) */
        rc = _time_dst_set_tz_offset(new_conf.tz_offset);
    }

    T_D("exit");

    return rc;
}

/* Module start */
static void time_dst_start(BOOL init)
{
    time_conf_t       *conf_p;

    T_D("enter, init: %d", init);

    if (init) {
        /* Initialize configuration */
        conf_p = &dst_global.system_conf;
        _time_dst_default_set(conf_p);

        /* initial DST variables */
        dst_start_utc_time = 0;
        dst_end_utc_time = 0;

        /* Create semaphore for critical regions */
        critd_init(&dst_global.crit, "dst_global.crit", VTSS_MODULE_ID_DAYLIGHT_SAVING, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
        TIME_DST_CRIT_EXIT();

    } else {
    }

    T_D("exit");
}

/* Initialize module */
vtss_rc time_dst_init(vtss_init_data_t *data)
{
    vtss_rc     rc = VTSS_OK;
    vtss_isid_t isid = data->isid;

    if (data->cmd == INIT_CMD_INIT) {
        /* Initialize and register trace ressources */
        VTSS_TRACE_REG_INIT(&trace_reg, trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&trace_reg);
    }

    T_D("enter, cmd: %d, isid: %u, flags: 0x%x", data->cmd, data->isid, data->flags);

    switch (data->cmd) {
    case INIT_CMD_INIT:
        T_D("INIT");
        time_dst_start(1);
#ifdef VTSS_SW_OPTION_ICFG
        if ((rc = vtss_daylight_saving_icfg_init()) != VTSS_OK) {
            T_D("Calling vtss_daylight_saving_icfg_init() failed rc = %s", error_txt(rc));
        }
#endif
        break;
    case INIT_CMD_START:
        T_D("START");
        time_dst_start(0);
        break;
    case INIT_CMD_CONF_DEF:
        T_D("CONF_DEF, isid: %d", isid);
        if (isid == VTSS_ISID_LOCAL) {
            /* Reset local configuration */
        } else if (isid == VTSS_ISID_GLOBAL) {
            /* Reset stack configuration */
            rc = time_dst_conf_read_stack(1);
        } else if (VTSS_ISID_LEGAL(isid)) {
            /* Reset switch configuration */
            rc = time_dst_conf_read_switch(isid);
        }
        break;
    case INIT_CMD_MASTER_UP:
        T_D("MASTER_UP");
        /* Read stack and switch configuration */
        if ((rc = time_dst_conf_read_stack(0)) != VTSS_OK) {
            return rc;
        }
        rc = time_dst_conf_read_switch(VTSS_ISID_GLOBAL);
        break;
    case INIT_CMD_MASTER_DOWN:
        T_D("MASTER_DOWN");
        break;
    case INIT_CMD_SWITCH_ADD:
        T_D("SWITCH_ADD, isid: %d", isid);
        break;
    case INIT_CMD_SWITCH_DEL:
        T_D("SWITCH_DEL, isid: %d", isid);
        break;
    default:
        break;
    }

    T_D("exit");

    return rc;
}

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/

