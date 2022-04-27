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
#include "vtss_ssh_api.h"
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

/****************************************************************************/
/*  Web Handler  functions                                                  */
/****************************************************************************/

static cyg_int32 handler_config_ssh(CYG_HTTPD_STATE *p)
{
    int        ct;
    ssh_conf_t ssh_conf, newconf;
    int        var_value;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_SECURITY)) {
        return -1;
    }
#endif

    if (p->method == CYG_HTTPD_METHOD_POST) {
        /* store form data */
        if (ssh_mgmt_conf_get(&ssh_conf) == VTSS_OK) {
            newconf = ssh_conf;

            //ssh_mode
            if (cyg_httpd_form_varable_int(p, "ssh_mode", &var_value)) {
                newconf.mode = var_value;
            }

            if (memcmp(&newconf, &ssh_conf, sizeof(newconf)) != 0) {
                T_D("Calling ssh_mgmt_conf_set()");
                if (ssh_mgmt_conf_set(&newconf) < 0) {
                    T_E("ssh_mgmt_conf_set(): failed");
                }
            }
        }

        redirect(p, "/ssh_config.htm");
    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        (void)cyg_httpd_start_chunked("html");

        /* get form data
           Format: [ssh_mode]
        */
        if (ssh_mgmt_conf_get(&ssh_conf) == VTSS_OK) {
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d", ssh_conf.mode);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        }
        (void)cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}

/****************************************************************************/
/*  HTTPD Table Entries                                                     */
/****************************************************************************/

CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_ssh, "/config/ssh", handler_config_ssh);

/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
