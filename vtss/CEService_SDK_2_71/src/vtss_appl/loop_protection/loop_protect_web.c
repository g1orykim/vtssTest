/*

   Vitesse Switch API software.

 Copyright (c) 2002-2012 Vitesse Semiconductor Corporation "Vitesse". All
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
#include "port_api.h"
#include "loop_protect_api.h"
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

static cyg_int32 handler_config_loop_protect(CYG_HTTPD_STATE *p)
{
    vtss_isid_t sid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    int ct, var;
    port_iter_t pit;
    loop_protect_conf_t conf, newconf;

    if(redirectUnmanagedOrInvalid(p, sid)) /* Redirect unmanaged/invalid access to handler */
        return -1;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_LOOP_PROTECT)) {
        return -1;
    }
#endif

    if (p->method == CYG_HTTPD_METHOD_POST) {
        int errors = 0;

        if (loop_protect_conf_get(&conf) < 0) {
            errors++;   /* Probably stack error */
        } else {
            vtss_rc rc;
            newconf = conf;

            if (cyg_httpd_form_varable_int(p, "gbl_enable", &var))
                newconf.enabled = var;
            if (cyg_httpd_form_varable_int(p, "txtime", &var))
                newconf.transmission_time = var;
            if (cyg_httpd_form_varable_int(p, "shuttime", &var))
                newconf.shutdown_time = var;

            if ((rc = loop_protect_conf_set(&newconf)) < 0) {
                T_D("%s: failed rc = %d", __FUNCTION__, rc);
                errors++; /* Probably stack error */
            }

            /* Ports */
            (void) port_iter_init(&pit, NULL, sid,
                                  PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
            while (port_iter_getnext(&pit)) {
                loop_protect_port_conf_t pconf;
                int i = pit.iport, uport = pit.uport;
                if((rc = loop_protect_conf_port_get(sid, i, &pconf)) == VTSS_OK) {
                    pconf.enabled = cyg_httpd_form_variable_check_fmt(p, "enable_%d", uport);
                    if(cyg_httpd_form_variable_int_fmt(p, &var, "action_%d", uport))
                        pconf.action = var;
                    if(cyg_httpd_form_variable_int_fmt(p, &var, "txmode_%d", uport))
                        pconf.transmit = var;
                    if((rc = loop_protect_conf_port_set(sid, i, &pconf)) != VTSS_OK) {
                        T_W("rc = %d", rc);
                        errors++;
                    }
                } else {
                    T_W("rc = %d", rc);
                    errors++;
                }
            }
        }
        redirect(p, errors ? STACK_ERR_URL : "/loop_config.htm");
    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        cyg_httpd_start_chunked("html");

        /* should get these values from management APIs */
        if (loop_protect_conf_get(&conf) == VTSS_OK) {
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d,%d,%d,",
                          conf.enabled, conf.transmission_time, conf.shutdown_time);
            cyg_httpd_write_chunked(p->outbuffer, ct);
        }

        /* Ports */
        (void) port_iter_init(&pit, NULL, sid,
                              PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
        while (port_iter_getnext(&pit)) {
            loop_protect_port_conf_t pconf;
            int i = pit.iport, uport = pit.uport;
            if(loop_protect_conf_port_get(sid, i, &pconf) == VTSS_OK) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d/%d/%d/%d|",
                              uport,
                              pconf.enabled,
                              pconf.action,
                              pconf.transmit);
                cyg_httpd_write_chunked(p->outbuffer, ct);
            }
        }
        cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}

CYG_HTTPD_HANDLER_TABLE_ENTRY(cb_handler_config_loop_protect, "/config/loop_config", handler_config_loop_protect);

static cyg_int32 handler_status_loop_protect(CYG_HTTPD_STATE *p)
{
    vtss_isid_t sid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    int ct;
    port_iter_t pit;
    loop_protect_conf_t conf;

    if(redirectUnmanagedOrInvalid(p, sid) ||
       loop_protect_conf_get(&conf) != VTSS_OK) /* Redirect unmanaged/invalid access to handler */
        return -1;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_LOOP_PROTECT)) {
        return -1;
    }
#endif

    cyg_httpd_start_chunked("html");
    (void) port_iter_init(&pit, NULL, sid,
                          PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
    while (conf.enabled && port_iter_getnext(&pit)) {
        loop_protect_port_conf_t pconf;
        int i = pit.iport;
        if(loop_protect_conf_port_get(sid, i, &pconf) == VTSS_OK) {
            loop_protect_port_info_t linfo;
            port_status_t pinfo;
            if(pconf.enabled &&
               loop_protect_port_info_get(sid, i, &linfo) == VTSS_OK) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d/%s/%s/%u/%s/%s/%s|",
                              pit.uport,
                              loop_protect_action2string(pconf.action),
                              pconf.transmit ? "Enabled" : "Disabled",
                              linfo.loops,
                              linfo.disabled ? "Disabled" : 
                              ((port_mgmt_status_get(sid, i, &pinfo) == VTSS_OK) && pinfo.status.link) ? "Up" : "Down",
                              linfo.loop_detect ? "Loop" : "-",
                              linfo.loops ? misc_time2str(linfo.last_loop) : "-");
                cyg_httpd_write_chunked(p->outbuffer, ct);
            }
        }
    }
    cyg_httpd_write_chunked("|", 1); /* Must return something - <empty> is stack error! */
    cyg_httpd_end_chunked();

    return -1; // Do not further search the file system.
}

CYG_HTTPD_HANDLER_TABLE_ENTRY(cb_handler_status_loop_protect, "/stat/loop_status", handler_status_loop_protect);
