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
#include "sysutil_api.h"
#include "conf_api.h"
#include "msg_api.h"
#include "control_api.h"
#include "vtss_api_if_api.h" /* For vtss_api_chipid() */
#if VTSS_SWITCH_STACKABLE
#include "topo_api.h"
#endif
#ifdef VTSS_SW_OPTION_PRIV_LVL
#include "vtss_privilege_api.h"
#include "vtss_privilege_web_api.h"
#endif

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_SYSTEM

/* =================
 * Trace definitions
 * -------------- */
#include <vtss_module_id.h>
#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_WEB
#include <vtss_trace_api.h>
/* ============== */

/****************************************************************************/
/*  Web Handler  functions                                                  */
/****************************************************************************/

static cyg_int32 handler_status_cpuload(CYG_HTTPD_STATE *p)
{
#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_STAT_TYPE, VTSS_MODULE_ID_SYSTEM)) {
        (void)diag_printf("No access\n");
        return -1;
    }
#endif

    cyg_uint32 average_point1s, average_1s, average_10s;
    average_point1s = average_1s = average_10s = 100;
    (void) control_sys_get_cpuload(&average_point1s, &average_1s, &average_10s);
  
    (void)cyg_httpd_start_chunked("html");
    int ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u|%u|%u",
                      average_point1s, average_1s, average_10s);
    (void)cyg_httpd_write_chunked(p->outbuffer, ct);
    (void)cyg_httpd_end_chunked();

    return -1; // Do not further search the file system.
}

#ifndef VTSS_SW_OPTION_USERS
static cyg_int32 handler_config_passwd(CYG_HTTPD_STATE *p)
{
    const char  *pass1, *pass2;
    char        unescaped_pass[VTSS_SYS_PASSWD_LEN];
    size_t      len1, len2;
    vtss_rc     rc;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_MISC)) {
        return -1;
    }
#endif

    if (p->method == CYG_HTTPD_METHOD_POST) {
        const char *oldpass_entered = cyg_httpd_form_varable_string(p, "oldpass", &len1);
        const char *oldpass_real    = system_get_passwd();
        rc = cgi_unescape(oldpass_entered, unescaped_pass, len1, sizeof(unescaped_pass));
        oldpass_entered = unescaped_pass;
        len1 = strlen(oldpass_entered);
        // Special case when len1 is 0 (in that case strncmp() always returns 0).
        if (rc != TRUE || len1 != strlen(oldpass_real) || strncmp(oldpass_entered, oldpass_real, strlen(oldpass_real) > len1 ? strlen(oldpass_real) : len1) != 0) {
            static const char *err_str = "The old password is incorrect. New password is not set.";
            send_custom_error(p, "Password Error", err_str, strlen(err_str));
            return -1;
        }
        pass1 = cyg_httpd_form_varable_string(p, "pass1", &len1);
        pass2 = cyg_httpd_form_varable_string(p, "pass2", &len2);
        if (pass1 && pass2 &&   /* Safeguard - this is *also* checked by javascript */
                len1 == len2 &&
                strncmp(pass1, pass2, len1) == 0 &&
                cgi_unescape(pass1, unescaped_pass, len1, sizeof(unescaped_pass))) {
            T_D("Password changed");
            (void) system_set_passwd(unescaped_pass);
        } else {
            T_W("Password check failed, password left unchanged");
        }
        redirect(p, "/passwd_config.htm");
    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        (void)cyg_httpd_start_chunked("html");
        (void)cyg_httpd_end_chunked();
    }
    return -1; // Do not further search the file system.
}
#endif /* VTSS_SW_OPTION_USERS */

static cyg_int32 handler_stat_sys(CYG_HTTPD_STATE *p)
{
    int           ct = 0;
    conf_board_t  conf;
    system_conf_t sys_conf;
    char          encoded_string[3 * VTSS_SYS_STRING_LEN];
    char          chipid_str[20];
    char          sep[] = "/";

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_STAT_TYPE, VTSS_MODULE_ID_SYSTEM)) {
        return -1;
    }
#endif

    cyg_int32 cursecs = cyg_current_time() / CYGNUM_HAL_RTC_DENOMINATOR;
    time_t t = time(NULL);

    /* get form data */
    (void)cyg_httpd_start_chunked("html");

    // sysInfo[0-2]
    (void)conf_mgmt_board_get(&conf);
    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%02x-%02x-%02x-%02x-%02x-%02x%c%s%c%s%c",
                  conf.mac_address[0],
                  conf.mac_address[1],
                  conf.mac_address[2],
                  conf.mac_address[3],
                  conf.mac_address[4],
                  conf.mac_address[5],
                  sep[0],
                  misc_time2interval(cursecs),
                  sep[0],
                  misc_time2str(t),
                  sep[0]);
    (void)cyg_httpd_write_chunked(p->outbuffer, ct);

    // sysInfo[3]
    ct = cgi_escape(misc_software_version_txt(), encoded_string);
    if (ct) {
        (void)cyg_httpd_write_chunked(encoded_string, ct);
    }
    (void)cyg_httpd_write_chunked(sep, 1);

    // sysInfo[4]
    ct = cgi_escape(misc_software_date_txt(), encoded_string);
    if (ct) {
        (void)cyg_httpd_write_chunked(encoded_string, ct);
    }
    (void)cyg_httpd_write_chunked(sep, 1);

    // sysInfo[5]
    sprintf(chipid_str, "VSC%x", vtss_api_chipid());
    ct = cgi_escape(chipid_str, encoded_string);
    if (ct) {
        (void)cyg_httpd_write_chunked(encoded_string, ct);
    }
    (void)cyg_httpd_write_chunked(sep, 1);

    (void)system_get_config(&sys_conf);

    // sysInfo[6]
    ct = cgi_escape(sys_conf.sys_contact, encoded_string);
    if (ct) {
        (void)cyg_httpd_write_chunked(encoded_string, ct);
    }
    (void)cyg_httpd_write_chunked(sep, 1);

    // sysInfo[7]
    ct = cgi_escape(sys_conf.sys_name, encoded_string);
    if (ct) {
        (void)cyg_httpd_write_chunked(encoded_string, ct);
    }
    (void)cyg_httpd_write_chunked(sep, 1);

    // sysInfo[8]
    ct = cgi_escape(sys_conf.sys_location, encoded_string);
    if (ct) {
        (void)cyg_httpd_write_chunked(encoded_string, ct);
    }
    (void)cyg_httpd_write_chunked(sep, 1);

    // sysInfo[9]
    ct = cgi_escape(misc_software_code_revision_txt(), encoded_string);
    if (ct) {
        (void)cyg_httpd_write_chunked(encoded_string, ct);
    }
    (void)cyg_httpd_write_chunked(sep, 1);

#if VTSS_SWITCH_STACKABLE
    if (msg_switch_is_master()) {
        // sysInfo[10]
        topo_switch_list_t *tsl_p = NULL;
        /* Lint thinks that sizeof(topo_switch_list_t) is -1923825720 bytes (yes, negative). Help it */
        /*lint -e{422, 433} */
        if ((tsl_p = VTSS_MALLOC(sizeof(topo_switch_list_t))) && topo_switch_list_get(tsl_p) == VTSS_OK) {
            msg_switch_info_t info;
            char              enc_ver_str[3 * sizeof(info.version_string)];
            char              enc_chipid_str[3 * sizeof(chipid_str)];
            ulong i;
            for (i = 0; i < ARRSZ(tsl_p->ts); i++) {
                topo_switch_t *ts_p = &tsl_p->ts[i];
                if (ts_p->vld && ts_p->present) {
                    if (VTSS_ISID_LEGAL(ts_p->isid) && msg_switch_info_get(ts_p->isid, &info) == VTSS_RC_OK) {
                        sprintf(chipid_str, "VSC%x", info.info.api_inst_id);
                    } else {
                        // An ISID may be illegal (0) if a new 17th switch has arrived in the stack.
                        strcpy(info.version_string, "N/A");
                        strcpy(chipid_str, "N/A");
                    }
                    (void)cgi_escape(info.version_string, enc_ver_str);
                    (void)cgi_escape(chipid_str, enc_chipid_str);
                    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d|%s|%s,", ts_p->usid, enc_chipid_str, enc_ver_str);
                    (void)cyg_httpd_write_chunked(p->outbuffer, ct);
                }
            }
        }
        if (tsl_p) {
            VTSS_FREE(tsl_p);
        }
    }
#endif /* VTSS_SWITCH_STACKABLE */

    (void)cyg_httpd_end_chunked();

    return -1; // Do not further search the file system.
}

static cyg_int32 handler_stat_logout(CYG_HTTPD_STATE *p)
{
    int is_ssl, sess_id;

    if (p->method == CYG_HTTPD_METHOD_GET) {
        // We use "seid" to save HTTP session ID and "sesslid" to save HTTPS session ID 
        if (cyg_httpd_form_varable_int(p, "type", &is_ssl)) {
            if (cyg_httpd_form_varable_int(p, "sessid", &sess_id)) {
                cyg_http_release_session(is_ssl,sess_id);
            }
        }
        (void)cyg_httpd_start_chunked("html");
        (void)cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}

/****************************************************************************/
/*  Module Filter CSS routine                                               */
/****************************************************************************/
static size_t SYS_lib_filter_css(char **base_ptr, char **cur_ptr, size_t *length)
{
    // Do not display help text of Code Revision line in sys.htm if the length
    // of the Code Revision string is 0.
    if (strlen(misc_software_code_revision_txt()) == 0) {
        return webCommonBufferHandler(base_ptr, cur_ptr, length, ".SYS_CODE_REV {display:none;}\r\n");
    } else {
        return 0;
    }
}

/****************************************************************************/
/*  Filter CSS table entry                                                  */
/****************************************************************************/
web_lib_filter_css_tab_entry(SYS_lib_filter_css);

/****************************************************************************/
/*  HTTPD Table Entries                                                     */
/****************************************************************************/
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_status_cpuload, "/stat/cpuload", handler_status_cpuload);
#ifndef VTSS_SW_OPTION_USERS
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_passwd, "/config/passwd", handler_config_passwd);
#endif /* VTSS_SW_OPTION_USERS */
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_sys, "/stat/sys", handler_stat_sys);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_stat_logout, "/stat/logout", handler_stat_logout);

/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
