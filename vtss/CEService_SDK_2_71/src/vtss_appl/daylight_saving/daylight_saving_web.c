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

#include "web_api.h"
//#include "syslog_api.h"
//#include "sysutil_api.h"
#include "daylight_saving_api.h"
#ifdef VTSS_SW_OPTION_PRIV_LVL
#include "vtss_privilege_api.h"
#include "vtss_privilege_web_api.h"
#endif

/* =================
 * Trace definitions
 * -------------- */
#include <vtss_module_id.h>
#include <vtss_trace_lvl_api.h>
#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_WEB
#include <vtss_trace_api.h>
/* ============== */

#define DST_WEB_BUF_LEN 512

/****************************************************************************/
/*  Web Handler functions                                                  */
/****************************************************************************/

static cyg_int32 handler_config_time_dst(CYG_HTTPD_STATE *p)
{
    int             ct, var_value;
    time_conf_t     conf, newconf;
    u32             default_offset;

    const char    *var_string;
    size_t        len;
    int           tz;
    char          encoded_string[3 * 16];

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_SYSTEM)) {
        return -1;
    }
#endif

    /* should get these values from management APIs */
    memset(conf.tz_acronym, 0x0, 16);
    (void) time_dst_get_config(&conf);

    if (p->method == CYG_HTTPD_METHOD_POST) {
        newconf = conf;

        /******************** timezone configuration ********************/
        //time_zone
        if (cyg_httpd_form_varable_int(p, "time_zone", &tz)) {
            newconf.tz = tz;
            newconf.tz_offset = tz / 10;
        }

        //acronym
        var_string = cyg_httpd_form_varable_string(p, "acronym", &len);
        if (len > 0) {
            (void) cgi_unescape(var_string, newconf.tz_acronym, len, sizeof(newconf.tz_acronym));
        } else {
            strcpy(newconf.tz_acronym, "");
            newconf.tz_acronym[0] = '\0';
        }

        /******************** daylight saving time configuration ********************/
        // mode
        if (cyg_httpd_form_varable_int(p, "mode", &var_value)) {
            newconf.dst_mode = var_value;
        }
        // week_s
        if (cyg_httpd_form_varable_int(p, "week_s", &var_value)) {
            newconf.dst_start_time.week = var_value;
        }
        // day_s
        if (cyg_httpd_form_varable_int(p, "day_s", &var_value)) {
            newconf.dst_start_time.day = var_value;
        }
        // month_s
        if (cyg_httpd_form_varable_int(p, "month_s", &var_value)) {
            newconf.dst_start_time.month = var_value;
        }
        // date_s
        if (cyg_httpd_form_varable_int(p, "date_s", &var_value)) {
            newconf.dst_start_time.date = var_value;
        }
        // year_s
        if (cyg_httpd_form_varable_int(p, "year_s", &var_value)) {
            newconf.dst_start_time.year = var_value;
        }
        // hours_s
        if (cyg_httpd_form_varable_int(p, "hours_s", &var_value)) {
            newconf.dst_start_time.hour = var_value;
        }
        // minutes_s
        if (cyg_httpd_form_varable_int(p, "minutes_s", &var_value)) {
            newconf.dst_start_time.minute = var_value;
        }
        // week_e
        if (cyg_httpd_form_varable_int(p, "week_e", &var_value)) {
            newconf.dst_end_time.week = var_value;
        }
        // day_e
        if (cyg_httpd_form_varable_int(p, "day_e", &var_value)) {
            newconf.dst_end_time.day = var_value;
        }
        // month_e
        if (cyg_httpd_form_varable_int(p, "month_e", &var_value)) {
            newconf.dst_end_time.month = var_value;
        }
        // date_e
        if (cyg_httpd_form_varable_int(p, "date_e", &var_value)) {
            newconf.dst_end_time.date = var_value;
        }
        // year_e
        if (cyg_httpd_form_varable_int(p, "year_e", &var_value)) {
            newconf.dst_end_time.year = var_value;
        }
        // hours_e
        if (cyg_httpd_form_varable_int(p, "hours_e", &var_value)) {
            newconf.dst_end_time.hour = var_value;
        }
        // minutes_e
        if (cyg_httpd_form_varable_int(p, "minutes_e", &var_value)) {
            newconf.dst_end_time.minute = var_value;
        }
        // offset
        if (cyg_httpd_form_varable_int(p, "offset", &var_value)) {
            newconf.dst_offset = var_value;
        }

        if (memcmp(&newconf, &conf, sizeof(newconf)) != 0) {
            vtss_rc rc;
            if ((rc = time_dst_set_config(&newconf)) < 0) {
                T_D("time_dst_set_config: failed rc = %d", rc);
            }
        }
        redirect(p, "/daylight_saving_config.htm");
    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        (void)cyg_httpd_start_chunked("html");

        /* get form data
           Format: <time_zone>,
        */

        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d,", conf.tz);
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);

        /* get form data
           Format: <acronym>
        */

        if (strlen(conf.tz_acronym)) {
            ct = cgi_escape(conf.tz_acronym, encoded_string);
            (void)cyg_httpd_write_chunked(encoded_string, ct);
        }

        /* get form data
           format: mode, week_s, day_s, month_s, date_s, year_s, hours_s, minutes_s,
                   week_e, day_e, month_e, date_e, year_e, hours_e, minutes_e, offset
         */

        if (conf.dst_offset < 1) {
            default_offset = 1;
        } else {
            default_offset = conf.dst_offset;
        }

        ct = snprintf(p->outbuffer, sizeof(p->outbuffer),
                      ", %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %u",
                      conf.dst_mode,
                      conf.dst_start_time.week,
                      conf.dst_start_time.day,
                      conf.dst_start_time.month,
                      conf.dst_start_time.date,
                      conf.dst_start_time.year,
                      conf.dst_start_time.hour,
                      conf.dst_start_time.minute,
                      conf.dst_end_time.week,
                      conf.dst_end_time.day,
                      conf.dst_end_time.month,
                      conf.dst_end_time.date,
                      conf.dst_end_time.year,
                      conf.dst_end_time.hour,
                      conf.dst_end_time.minute,
                      default_offset
                     );
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);

        (void)cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}

/****************************************************************************/
/*  Module Filter CSS routine                                               */
/****************************************************************************/
static size_t dst_lib_filter_css(char **base_ptr, char **cur_ptr, size_t *length)
{
    char buff[DST_WEB_BUF_LEN];
    (void) snprintf(buff, DST_WEB_BUF_LEN,
                    ".dst_tz_offset_display { display: none; }\r\n"
                   );
    return webCommonBufferHandler(base_ptr, cur_ptr, length, buff);
}

/****************************************************************************/
/*  Filter CSS table entry                                                  */
/****************************************************************************/

web_lib_filter_css_tab_entry(dst_lib_filter_css);

/****************************************************************************/
/*  HTTPD Table Entries                                                     */
/****************************************************************************/

CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_time_dst, "/config/time_dst", handler_config_time_dst);

/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
