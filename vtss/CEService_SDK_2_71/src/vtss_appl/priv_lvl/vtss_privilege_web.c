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

#include "vtss_privilege_web_api.h"

/* =================
 * Trace definitions
 * -------------- */
#include <vtss_module_id.h>
#include <vtss_trace_lvl_api.h>
#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_WEB
#include <vtss_trace_api.h>
/* ============== */

int web_process_priv_lvl(CYG_HTTPD_STATE *p, vtss_priv_lvl_type_t type, vtss_module_id_t id)
{
    vtss_rc rc1, rc2;
    int current_priv_lvl = cyg_httpd_current_privilege_level();

    if (type == VTSS_PRIV_LVL_CONFIG_TYPE) {
        rc1 = vtss_priv_is_allowed_crw(id, current_priv_lvl);
    } else {
        rc1 = vtss_priv_is_allowed_srw(id, current_priv_lvl);
    }

    if (p->method == CYG_HTTPD_METHOD_POST) {
        if (!rc1) {
            redirect(p, "/insuf_priv_lvl.htm");
            return -1;
        }
    } else {
        /* If the web page only allows read-only, add tag "X-ReadOnly: true" in HTML response header.
           If the web page doesn¡¦t allow any action, add tag "X-ReadOnly: null" in HTML response header.
           If the web page allows read-write, don¡¦t need inserts the tag. */
        if (type == VTSS_PRIV_LVL_CONFIG_TYPE) {
            rc2 = vtss_priv_is_allowed_cro(id, current_priv_lvl);
        } else {
            rc2 = vtss_priv_is_allowed_sro(id, current_priv_lvl);
        }

        if (!rc1 && rc2) {
            cyg_httpd_set_xreadonly_tag(1);    //Readonly privilege level
        } else if (!rc1 && !rc2) {
            cyg_httpd_set_xreadonly_tag(0);    //Insufficient privilege level
        }
    }
    return 0;
}

/****************************************************************************/
/*  Web Handler  functions                                                  */
/****************************************************************************/

static cyg_int32 handler_config_priv_lvl(CYG_HTTPD_STATE *p)
{
    int                 ct;
    vtss_priv_conf_t    conf, newconf;
    int                 var_value;
    char                buf[VTSS_PRIV_LVL_NAME_LEN_MAX];
    char                encoded_string[3 * VTSS_PRIV_LVL_NAME_LEN_MAX];
    char                priv_group_name[VTSS_PRIV_LVL_NAME_LEN_MAX] = "";
    vtss_module_id_t    module_id;
    users_conf_t        users_conf;

    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_MISC)) {
        return -1;
    }

    if (p->method == CYG_HTTPD_METHOD_POST) {
        if (vtss_priv_mgmt_conf_get(&conf) == VTSS_OK) {
            /* store form data */
            newconf = conf;

            while (vtss_privilege_group_name_get(priv_group_name, &module_id, TRUE)) {
                //cro
                sprintf(buf, "cro_%s", vtss_module_names[module_id]);
                (void) cgi_escape(buf, encoded_string);
                if (cyg_httpd_form_varable_int(p, encoded_string, &var_value)) {
                    newconf.privilege_level[module_id].cro = var_value;
                }

                //crw
                sprintf(buf, "crw_%s", vtss_module_names[module_id]);
                (void) cgi_escape(buf, encoded_string);
                if (cyg_httpd_form_varable_int(p, encoded_string, &var_value)) {
                    newconf.privilege_level[module_id].crw = var_value;
                }

                //sro
                sprintf(buf, "sro_%s", vtss_module_names[module_id]);
                (void) cgi_escape(buf, encoded_string);
                if (cyg_httpd_form_varable_int(p, encoded_string, &var_value)) {
                    newconf.privilege_level[module_id].sro = var_value;
                }

                //srw
                sprintf(buf, "srw_%s", vtss_module_names[module_id]);
                (void) cgi_escape(buf, encoded_string);
                if (cyg_httpd_form_varable_int(p, encoded_string, &var_value)) {
                    newconf.privilege_level[module_id].srw = var_value;
                }
            }

            if (memcmp(&newconf, &conf, sizeof(newconf)) != 0) {
                T_D("Calling vtss_priv_mgmt_conf_set()");
                if (vtss_priv_mgmt_conf_set(&newconf) < 0) {
                    T_E("vtss_priv_mgmt_conf_set(): failed");
                }
            }
        }
        redirect(p, "/priv_lvl.htm");
    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        (void)cyg_httpd_start_chunked("html");

        /* get form data
           Format: [admin_priv_level],[group_name]/[cro]/[crw]/[sro]/[srw]|...
        */
        strcpy(users_conf.username, VTSS_SYS_ADMIN_NAME);
        if (vtss_priv_mgmt_conf_get(&conf) == VTSS_OK &&
            vtss_users_mgmt_conf_get(&users_conf, FALSE) == VTSS_OK) {
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d,|", users_conf.privilege_level);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);

            while (vtss_privilege_group_name_get(priv_group_name, &module_id, TRUE)) {
                if (cgi_escape((char *)vtss_module_names[module_id], encoded_string) == 0) {
                    continue;
                }
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s/%d/%d/%d/%d/|",
                              encoded_string,
                              conf.privilege_level[module_id].cro,
                              conf.privilege_level[module_id].crw,
                              conf.privilege_level[module_id].sro,
                              conf.privilege_level[module_id].srw);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            };
        }
        (void)cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}

/****************************************************************************/
/*  HTTPD Table Entries                                                     */
/****************************************************************************/

CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_priv_lvl, "/config/priv_lvl", handler_config_priv_lvl);

/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
