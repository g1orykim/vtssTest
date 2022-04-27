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

#ifdef VTSS_SW_OPTION_IP2
#ifndef VTSS_SW_OPTION_NTP
#ifdef VTSS_SW_OPTION_SNTP

#ifdef VTSS_SW_OPTION_SYSUTIL
#include "sysutil_api.h"
#endif

#include "web_api.h"
#include "vtss_sntp_api.h"
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

static cyg_int32 handler_config_sntp(CYG_HTTPD_STATE *p)
{
    int         ct;
    sntp_conf_t conf, newconf;
    const char  *var_string;
    size_t      len;
    int         var_value, idx, i;
    char        host_buf[SNTP_ADDRSTRLEN];
    char        str_buff1[40];

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_SYSTEM)) {
        return -1;
    }
#endif

    if (p->method == CYG_HTTPD_METHOD_POST) {
        /* store form data */
        if (sntp_config_get(&conf) == VTSS_OK) {
            newconf = conf;

            /* ntp_mode */
            if (cyg_httpd_form_varable_int(p, "sntp_mode", &var_value)) {
                newconf.mode = var_value;
            }

            /* ntp_server */
            for (idx = 0; idx < SNTP_MAX_SERVER_COUNT; idx++) {
                sprintf((char *)str_buff1, "sntp_server%d", idx + 1);
#ifdef VTSS_SW_OPTION_IPV6
                if (cyg_httpd_form_varable_ipv6(p, str_buff1, &newconf.ipv6_addr)) {
                    memset(newconf.sntp_server, 0, sizeof(newconf.sntp_server));
                    newconf.ip_type = SNTP_IP_TYPE_IPV6;
                    continue;
                }
#endif
                var_string = cyg_httpd_form_varable_string(p, str_buff1, &len);
                for (i = 0; i < SNTP_ADDRSTRLEN; i++) {
                    if (*var_string != '&') {
                        host_buf[i] = *var_string;
                        var_string ++;
                    } else {
                        host_buf[i] = '\0';
                        break;
                    }
                }
                strcpy(newconf.sntp_server, host_buf);
#ifdef VTSS_SW_OPTION_IPV6
                memset(&newconf.ipv6_addr, 0, sizeof(newconf.ipv6_addr));
#endif
                newconf.ip_type = SNTP_IP_TYPE_IPV4;
            }

            if (memcmp(&newconf, &conf, sizeof(newconf)) != 0) {
                T_D("Calling sntp_config_set()");
                if (sntp_config_set(&newconf) != VTSS_OK) {
                    T_E("sntp_config_set(): failed");
                }
            }
        }
        redirect(p, "/sntp.htm");
    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        (void)cyg_httpd_start_chunked("html");

        /* get form data
           Format: [ipv6_supported]/[ntp_mode]/[ntp_server1]
        */
        if (sntp_config_get(&conf) == VTSS_OK) {

#if defined(VTSS_SW_OPTION_IPV6) && defined(VTSS_SW_OPTION_NTP)
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "1/%lu/%s",
                          (long unsigned int)conf.mode,
                          conf.ip_type == NTP_IP_TYPE_IPV4 ? (char *)conf.sntp_server : misc_ipv6_txt(&conf.ipv6_addr, str_buff1));
#else
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "0/%lu/%s",
                          (long unsigned int)conf.mode,
                          conf.sntp_server);
#endif

            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        }
        (void)cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}

#if 0
/****************************************************************************/
/*  Module Config JS lib routine                                            */
/****************************************************************************/

static size_t sntp_lib_config_js(char **base_ptr, char **cur_ptr, size_t *length)
{
    const char *buff =
        "function isSntpSupported() {\n"
        " return true;\n"
        "}\n\n";
    return webCommonBufferHandler(base_ptr, cur_ptr, length, buff);
}

/****************************************************************************/
/*  Config JS lib table entry                                               */
/****************************************************************************/
web_lib_config_js_tab_entry(sntp_lib_config_js);

/****************************************************************************/
/*  Module Filter CSS routine                                               */
/****************************************************************************/
static size_t ntp_lib_filter_css(char **base_ptr, char **cur_ptr, size_t *length)
{
    return webCommonBufferHandler(base_ptr, cur_ptr, length, ".SNTP_CTRL { display: none; }\r\n");
}
/****************************************************************************/
/*  Filter CSS table entry                                                  */
/****************************************************************************/
web_lib_filter_css_tab_entry(ntp_lib_filter_css);
#endif

/****************************************************************************/
/*  HTTPD Table Entries                                                     */
/****************************************************************************/

CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_sntp, "/config/sntp", handler_config_sntp);

#endif
#endif
#endif
/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
