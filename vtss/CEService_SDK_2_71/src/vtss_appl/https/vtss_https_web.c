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
#include "web_api.h"
#include "vtss_https_api.h"
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

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_HTTPS

/****************************************************************************/
/*  Web Handler  functions                                                  */
/****************************************************************************/

static cyg_int32 handler_config_https(CYG_HTTPD_STATE *p)
{
    int          ct;
    https_conf_t *https_conf, *newconf;
    int          var_value;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_SECURITY)) {
        return -1;
    }
#endif

    if ((https_conf = VTSS_MALLOC(sizeof(https_conf_t))) == NULL) {
        return -1;
    }
    if ((newconf = VTSS_MALLOC(sizeof(https_conf_t))) == NULL) {
        VTSS_FREE(https_conf);
        return -1;
    }

    if (p->method == CYG_HTTPD_METHOD_POST) {
        /* store form data */
        if (https_mgmt_conf_get(https_conf) == VTSS_OK) {
            *newconf = *https_conf;

            //https_mode
            if (cyg_httpd_form_varable_int(p, "https_mode", &var_value)) {
                newconf->mode = var_value;
            }

#if HTTPS_MGMT_SUPPORTED_REDIRECT
            //https_redirect
            if (newconf->mode == HTTPS_MGMT_DISABLED) {
                newconf->redirect = HTTPS_MGMT_DISABLED;
            } else if (cyg_httpd_form_varable_int(p, "https_redirect", &var_value)) {
                newconf->redirect = var_value;
            }
#endif /* HTTPS_MGMT_SUPPORTED_REDIRECT */

            if (memcmp(newconf, https_conf, sizeof(*newconf)) != 0) {
                T_D("Calling https_mgmt_conf_set()");
                if (https_mgmt_conf_set(newconf) < 0) {
                    T_E("https_mgmt_conf_set(): failed");
                }
            }
        }

        redirect(p, "/https_config.htm");
    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        (void)cyg_httpd_start_chunked("html");

        /* get form data
           Format: [https_mode]/[https_redirect]
        */
        if (https_mgmt_conf_get(https_conf) == VTSS_OK) {
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d/%d",
                          https_conf->mode,
                          https_conf->redirect);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        }
        (void)cyg_httpd_end_chunked();
    }

    VTSS_FREE(newconf);
    VTSS_FREE(https_conf);

    return -1; // Do not further search the file system.
}

/****************************************************************************/
/*  HTTPD Table Entries                                                     */
/****************************************************************************/

CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_https, "/config/https", handler_config_https);

/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
