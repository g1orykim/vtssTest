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
#include "vtss_users_api.h"
#ifdef VTSS_SW_OPTION_PRIV_LVL
#include "vtss_privilege_api.h"
#include "vtss_privilege_web_api.h"
#endif

#define USERS_WEB_BUF_LEN 512

/* =================
 * Trace definitions
 * -------------- */
#include <vtss_module_id.h>
#include <vtss_trace_lvl_api.h>
#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_WEB
#include <vtss_trace_api.h>
/* ============== */

/****************************************************************************/
/*  Web Handler  functions                                                  */
/****************************************************************************/

static cyg_int32 handler_stat_users(CYG_HTTPD_STATE *p)
{
    int             ct, i = 0, max_num = VTSS_USERS_NUMBER_OF_USERS;
    users_conf_t    conf;
    char            encoded_string[3 * VTSS_SYS_USERNAME_LEN];

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_STAT_TYPE, VTSS_MODULE_ID_MISC)) {
        return -1;
    }
#endif

    (void)cyg_httpd_start_chunked("html");

    /* get form data
       Format: [max_num]|[user_idx],[username],[priv_level]|...
    */
    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d|", max_num);
    (void)cyg_httpd_write_chunked(p->outbuffer, ct);

    memset(&conf, 0x0, sizeof(conf));
    while (vtss_users_mgmt_conf_get(&conf, 1) == VTSS_OK) {
        if (cgi_escape(conf.username, encoded_string) == 0) {
            continue;
        }
#ifdef VTSS_SW_OPTION_PRIV_LVL
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d,%s,%d|", ++i, encoded_string, conf.privilege_level);
#else
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d,%s,0|", ++i, encoded_string);
#endif
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);
    }

    (void)cyg_httpd_end_chunked();
    return -1; // Do not further search the file system.
}

static cyg_int32 handler_config_user_config(CYG_HTTPD_STATE *p)
{
    int             ct;
    users_conf_t    conf;
    int             var_value, i = 0;
    const char      *var_string, *pass1, *pass2;
    char            unescaped_pass[VTSS_SYS_PASSWD_LEN];
    char            encoded_string[3 * VTSS_SYS_USERNAME_LEN];
    size_t          len, len1, len2;
    vtss_rc         rc;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    vtss_priv_conf_t priv_conf;
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_MISC)) {
        return -1;
    }
#endif

    memset(&conf, 0x0, sizeof(conf));

    if (p->method == CYG_HTTPD_METHOD_POST) {
        /* store form data */
        //username
        var_string = cyg_httpd_form_varable_string(p, "username", &len);
        if (len > 0) {
            if (cgi_unescape(var_string, conf.username, len, sizeof(conf.username)) == FALSE) {
                redirect(p, "/users.htm");
                return -1;
            }
        }
        if (cyg_httpd_form_varable_int(p, "delete", &var_value)) {
            //delete
            if (!strcmp(conf.username, VTSS_SYS_ADMIN_NAME)) {
                redirect(p, "/users.htm");
                return -1;
            } else {
                (void) vtss_users_mgmt_conf_del(conf.username);
            }
        } else {
            // Get user entry first if it existing
            (void) vtss_users_mgmt_conf_get(&conf, FALSE);

            //password
            pass1 = cyg_httpd_form_varable_string(p, "password1", &len1);
            pass2 = cyg_httpd_form_varable_string(p, "password2", &len2);
            if (pass1 && pass2 &&   /* Safeguard - this is *also* checked by javascript */
                len1 == len2 &&
                strncmp(pass1, pass2, len1) == 0 &&
                cgi_unescape(pass1, unescaped_pass, len1, sizeof(unescaped_pass))) {
                strcpy(conf.password, unescaped_pass);
            }

#ifdef VTSS_SW_OPTION_PRIV_LVL
            //priv_level
            if (cyg_httpd_form_varable_int(p, "priv_level", &var_value)) {
                conf.privilege_level = var_value;
            }
#endif

            conf.valid = 1;
            if (vtss_users_mgmt_conf_set(&conf) != VTSS_OK) {
                T_E("vtss_users_mgmt_conf_set(): failed");
            }
        }

        redirect(p, "/users.htm");
    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        (void)cyg_httpd_start_chunked("html");

        /* get form data
           Format: [user_idx],[username],[password],[priv_level],[misc_priv_level]
        */
        if ((var_string = cyg_httpd_form_varable_find(p, "user")) != NULL) {
            var_value = atoi(var_string);
            while ((rc = vtss_users_mgmt_conf_get(&conf, 1)) == VTSS_OK) {
                i++;
                if (i == var_value) {
                    break;
                }
            }
            if (rc != VTSS_OK || cgi_escape(conf.username, encoded_string) == 0) {
                (void)cyg_httpd_end_chunked();
                return -1;
            }
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d,%s,", i, encoded_string);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            (void) cgi_escape(conf.password, encoded_string);
#ifdef VTSS_SW_OPTION_PRIV_LVL
            if (vtss_priv_mgmt_conf_get(&priv_conf) != VTSS_OK) {
                T_W("vtss_priv_mgmt_conf_get() failed");
                (void)cyg_httpd_end_chunked();
                return -1;
            }
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s,%d,%d", encoded_string, conf.privilege_level, priv_conf.privilege_level[VTSS_MODULE_ID_MISC].crw);
#else
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s,0", encoded_string);
#endif
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        } else {
#ifdef VTSS_SW_OPTION_PRIV_LVL
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "-1,,,1,15");
#else
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "-1,,,0,0");
#endif
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        }
        (void)cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}

/****************************************************************************/
/*  Module JS lib config routine                                            */
/****************************************************************************/

static size_t users_lib_config_js(char **base_ptr, char **cur_ptr, size_t *length)
{
    char buff[USERS_WEB_BUF_LEN];
    (void) snprintf(buff, USERS_WEB_BUF_LEN,
                    "var configUsernameMaxLen = %d;\n"
                    "var configPasswordMaxLen = %d;\n"
                    , VTSS_SYS_USERNAME_LEN - 1
                    , VTSS_SYS_PASSWD_LEN - 1);
    return webCommonBufferHandler(base_ptr, cur_ptr, length, buff);
}

/****************************************************************************/
/*  JS lib config table entry                                               */
/****************************************************************************/

web_lib_config_js_tab_entry(users_lib_config_js);

/****************************************************************************/
/*  HTTPD Table Entries                                                     */
/****************************************************************************/
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_stat_users, "/stat/users", handler_stat_users);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_user_config, "/config/user_config", handler_config_user_config);

/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
