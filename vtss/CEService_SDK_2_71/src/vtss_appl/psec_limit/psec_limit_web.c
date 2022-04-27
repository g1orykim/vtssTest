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
#ifdef VTSS_SW_OPTION_PSEC_LIMIT
#include "psec_limit_api.h" /* For psec_limit_mgmt_XXX_cfg_get()               */
#endif
#include "psec_api.h"         /* Interface to the module that this file supports */
#include "psec.h"             /* For semi-public PSEC functions                  */
#include "msg_api.h"          /* For msg_abstime_get()                           */
#ifdef VTSS_SW_OPTION_PRIV_LVL
#include "vtss_privilege_api.h"
#include "vtss_privilege_web_api.h"
#endif

#define PSEC_LIMIT_WEB_BUF_LEN 512

/* =================
 * Trace definitions
 * -------------- */
#include <vtss_module_id.h>
#include <vtss_trace_lvl_api.h>
#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_WEB
#include <vtss_trace_api.h>
/* ============== */

/*lint -sem(handler_config_psec_limit_reopen, thread_protected) Has unprotected access to var_uport */

static cyg_int32 handler_config_psec_limit(CYG_HTTPD_STATE *p)
{
    vtss_isid_t             isid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    psec_limit_glbl_cfg_t   glbl_cfg;
    psec_limit_switch_cfg_t switch_cfg;
    psec_switch_status_t    switch_status;
    vtss_rc                 rc;
    int                     cnt, an_integer, errors = 0;
    const char              *err_buf_ptr;
    port_iter_t             pit;

    if (redirectUnmanagedOrInvalid(p, isid)) {
        /* Redirect unmanaged/invalid access to handler */
        return -1;
    }

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_SECURITY)) {
        return -1;
    }
#endif

    if ((rc = psec_limit_mgmt_glbl_cfg_get(&glbl_cfg)) != VTSS_OK) {
        errors++;
    }
    if (p->method == CYG_HTTPD_METHOD_POST) {
        if (!errors) {
            // glbl_ena: select
            if (cyg_httpd_form_varable_int(p, "glbl_ena", &an_integer) && (an_integer == FALSE || an_integer == TRUE)) {
                glbl_cfg.enabled = an_integer;
            } else {
                errors++;
            }

            // aging_enabled: Checkbox
            glbl_cfg.enable_aging = cyg_httpd_form_varable_find(p, "aging_enabled") ? TRUE : FALSE; // Returns non-NULL if checked

            // aging_period: Integer. Only overwrite if enabled.
            if (glbl_cfg.enable_aging) {
                if (cyg_httpd_form_varable_int(p, "aging_period", &an_integer)) {
                    glbl_cfg.aging_period_secs = an_integer;
                } else {
                    errors++;
                }
            }

            if (!errors) {
                if ((rc = psec_limit_mgmt_glbl_cfg_set(&glbl_cfg)) != VTSS_OK) {
                    T_D("psec_limit_mgmt_glbl_cfg_set() failed");
                    errors++;
                }
            }

            // Gotta get the current switch config because we may only
            // overwrite some of the per-port parameters.
            if (psec_limit_mgmt_switch_cfg_get(isid, &switch_cfg) != VTSS_OK) {
                errors++;
            }

            // Port config.
            (void)port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
            while (!errors && port_iter_getnext(&pit)) {
                char                  var_name[16];
                psec_limit_port_cfg_t *port_cfg = &switch_cfg.port_cfg[pit.iport - VTSS_PORT_NO_START];

                // ena_%d: Int
                sprintf(var_name, "ena_%u", pit.uport);
                if (cyg_httpd_form_varable_int(p, var_name, &an_integer)) {
                    port_cfg->enabled = an_integer;
                } else {
                    errors++;
                }

                if (port_cfg->enabled) {
                    // limit_%d: Int
                    sprintf(var_name, "limit_%u", pit.uport);
                    if (cyg_httpd_form_varable_int(p, var_name, &an_integer)) {
                        port_cfg->limit = an_integer;
                    } else {
                        errors++;
                    }

                    // action_%d: Select
                    sprintf(var_name, "action_%u", pit.uport);
                    if (cyg_httpd_form_varable_int(p, var_name, &an_integer)) {
                        port_cfg->action = an_integer;
                    } else {
                        errors++;
                    }
                }
            }

            if (!errors) {
                if ((rc = psec_limit_mgmt_switch_cfg_set(isid, &switch_cfg)) != VTSS_OK) {
                    T_D("psec_limit_mgmt_switch_cfg_set() failed");
                    errors++;
                }
            }

            if (errors) {
                // There are two types of errors: Those where a form variable was invalid,
                // and those where a psec_limit_mgmt_XXX() function failed.
                // In the first case, we redirect to the STACK_ERR_URL page, and in the
                // second, we redirect to a custom error page.
                if (rc == VTSS_OK) {
                    redirect(p, STACK_ERR_URL);
                } else {
                    err_buf_ptr = error_txt(rc);
                    send_custom_error(p, "Port Security Control Error", err_buf_ptr, strlen(err_buf_ptr));
                }
            } else {
                // No errors. Update with the current settings.
                redirect(p, "/psec_limit.htm");
            }
        }
    } else { /* CYG_HTTPD_METHOD_GET (+HEAD) */
        (void)cyg_httpd_start_chunked("html");

        // Notice that the current configuration comes from the PSEC LIMIT module, whereas
        // PSEC LIMIT has no idea of the current status, so we gotta get that from the
        // underlying PSEC module.
        if (psec_limit_mgmt_switch_cfg_get(isid, &switch_cfg) != VTSS_OK || psec_mgmt_switch_status_get(isid, &switch_status) != VTSS_OK) {
            errors++;
        }

        if (!errors) {
            // GlblEna/AgingEna/AgingPeriod/MaxLimit#[PortConfig]
            // Format of [PortConfig]
            // PortNumber_1/PortEna_1/Limit_1/Action_1/State_1#PortNumber_2/PortEna_2/Limit_2/Action_2/State_2#...#PortNumber_N/PortEna_N/Limit_N/Action_N/State_N
            cnt = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d/%d/%u/%d#",
                           glbl_cfg.enabled,
                           glbl_cfg.enable_aging,
                           glbl_cfg.aging_period_secs,
                           PSEC_LIMIT_LIMIT_MAX);
            cyg_httpd_write_chunked(p->outbuffer, cnt);

            (void)port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
            while (port_iter_getnext(&pit)) {
                int                   state;
                psec_port_status_t    *port_status = &switch_status.port_status[pit.iport - VTSS_PORT_NO_START];
                psec_limit_port_cfg_t *port_cfg = &switch_cfg.port_cfg[pit.iport - VTSS_PORT_NO_START];

                // Gotta combine the values from the PSEC module with the values from the PSEC LIMIT module.
                // The final value will be encoded as follows:
                //   0 = Disabled      (PSEC LIMIT disabled)
                //   1 = Ready         (PSEC LIMIT enabled, and port is not shut down, and limit is not reached).
                //   2 = Limit Reached (PSEC LIMIT enabled, port is not shutdown, but the limit is reached. This can only be reached if Action = None or Trap).
                //   3 = Shutdown      (PSEC LIMIT enabled, port is shutdown. This can only be reached if Action = Shutdown or Trap & Shutdown).
                // The PSEC module just returns its two flags Limit Reached and Shutdown. If PSEC LIMIT is not enabled, it should not be possible
                // to get a TRUE value out of these, since only the PSEC LIMIT module can have the PSEC module set these flags.
                // To determine whether to return Disabled or Ready, we need the current configuration of the PSEC LIMIT module.
                if (glbl_cfg.enabled && port_cfg->enabled) {
                    if (port_status->shutdown) {
                        state = 3; // Shutdown
                    } else if (port_status->limit_reached) {
                        state = 2; // Limit reached
                    } else {
                        state = 1; // Ready
                    }
                } else {
                    if (port_status->limit_reached || port_status->shutdown) {
                        // As said, it shouldn't be possible to have a port in its limit_reached or shutdown state
                        // if PSEC LIMIT Control is not enabled on that port.
                        T_E("Internal error.");
                    }
                    state = 0; // Disabled
                }

                if (!pit.first) {
                    cyg_httpd_write_chunked("#", 1);
                }

                // [PortConfig]: PortNumber_1/PortEna_1/Limit_1/Action_1/State_1#PortNumber_2/PortEna_2/Limit_2/Action_2/State_2#...#PortNumber_N/PortEna_N/Limit_N/Action_N/State_N
                cnt = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d/%d/%u/%d/%d",
                               pit.uport,
                               port_cfg->enabled,
                               port_cfg->limit,
                               port_cfg->action,
                               state);
                cyg_httpd_write_chunked(p->outbuffer, cnt);
            }
        }
        cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}

static cyg_int32 handler_config_psec_limit_reopen(CYG_HTTPD_STATE *p)
{
    vtss_isid_t    isid = web_retrieve_request_sid(p);
    vtss_port_no_t iport;
    int            errors = 0;

    if (redirectUnmanagedOrInvalid(p, isid)) {
        /* Redirect unmanaged/invalid access to handler */
        return -1;
    }

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_SECURITY)) {
        return -1;
    }
#endif

    if (p->method == CYG_HTTPD_METHOD_GET) {
        (void)cyg_httpd_start_chunked("html");
        // The Reopen option is implemented as a CYG_HTTPD_METHOD_GET method.
        // The URL-encoded property is auto-interpreted and stored in var_uport.

        // port=%d
        iport = uport2iport(atoi(var_uport));
        if (iport >= port_isid_port_count(isid) + VTSS_PORT_NO_START || port_isid_port_no_is_stack(isid, iport)) {
            errors++;
        }

        // Do the reopening
        if (!errors && psec_mgmt_reopen_port(PSEC_USER_PSEC_LIMIT, isid, iport) != VTSS_OK) {
            errors++;
        }

        if (errors) {
            T_W("Error in URL");
        }

        cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}

/****************************************************************************/
/*  Module JS lib config routine                                            */
/****************************************************************************/
static size_t psec_limit_lib_config_js(char **base_ptr, char **cur_ptr, size_t *length)
{
    char buff[PSEC_LIMIT_WEB_BUF_LEN];
    (void) snprintf(buff, PSEC_LIMIT_WEB_BUF_LEN, "var configPsecLimitLimitMax = %d;\n", PSEC_LIMIT_LIMIT_MAX);
    return webCommonBufferHandler(base_ptr, cur_ptr, length, buff);
}

/****************************************************************************/
/*  JS lib config table entry                                               */
/****************************************************************************/
web_lib_config_js_tab_entry(psec_limit_lib_config_js);

/****************************************************************************/
/*  HTTPD Table Entries                                                     */
/****************************************************************************/
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_psec_limit,        "/config/psec_limit",        handler_config_psec_limit);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_psec_limit_reopen, "/config/psec_limit_reopen", handler_config_psec_limit_reopen);

/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
