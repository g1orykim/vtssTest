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
#include "vtss_ntp_api.h"
#ifdef VTSS_SW_OPTION_PRIV_LVL
#include "vtss_privilege_api.h"
#include "vtss_privilege_web_api.h"
#endif

#define NTP_WEB_BUF_LEN 512

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

static cyg_int32 handler_config_ntp(CYG_HTTPD_STATE *p)
{
    int         ct;
    ntp_conf_t  conf, newconf;
    const char  *var_string;
    size_t      len;
    int         var_value, idx, i;
    char        host_buf[INET6_ADDRSTRLEN];
#ifdef VTSS_SW_OPTION_IPV6
    char        str_buff1[40], str_buff2[40], str_buff3[40], str_buff4[40], str_buff5[40];
#else
    char        str_buff1[40];
#endif

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_IP)) {
        return -1;
    }
#endif

    if (p->method == CYG_HTTPD_METHOD_POST) {
        /* store form data */
        if (ntp_mgmt_conf_get(&conf) == VTSS_OK) {
            newconf = conf;

            /* ntp_mode */
            if (cyg_httpd_form_varable_int(p, "ntp_mode", &var_value)) {
                newconf.mode_enabled = var_value;
            }
#if 0
            /* Do not set Interval */
            /* ntp_polling_interval */
            if (cyg_httpd_form_varable_int(p, "ntp_polling_interval", &var_value)) {
                newconf.ntp_interval = var_value;
            }
#endif
            /* ntp_server */
            for (idx = 0; idx < NTP_MAX_SERVER_COUNT; idx++) {
                sprintf((char *)str_buff1, "ntp_server%d", idx + 1);
#ifdef VTSS_SW_OPTION_IPV6
                if (cyg_httpd_form_varable_ipv6(p, str_buff1, &newconf.server[idx].ipv6_addr)) {
                    memset(newconf.server[idx].ip_host_string, 0, sizeof(newconf.server[idx].ip_host_string));
                    newconf.server[idx].ip_type = NTP_IP_TYPE_IPV6;
                    continue;
                }
#endif
                var_string = cyg_httpd_form_varable_string(p, str_buff1, &len);
                for (i = 0; i < INET6_ADDRSTRLEN; i++) {
                    if (*var_string != '&') {
                        host_buf[i] = *var_string;
                        var_string ++;
                    } else {
                        host_buf[i] = '\0';
                        break;
                    }
                }
                strcpy(newconf.server[idx].ip_host_string, host_buf);
#ifdef VTSS_SW_OPTION_IPV6
                memset(&newconf.server[idx].ipv6_addr, 0, sizeof(newconf.server[idx].ipv6_addr));
#endif
                newconf.server[idx].ip_type = NTP_IP_TYPE_IPV4;
            }

            if (memcmp(&newconf, &conf, sizeof(newconf)) != 0) {
                T_D("Calling ntp_mgmt_conf_set()");
                if (ntp_mgmt_conf_set(&newconf) != VTSS_OK) {
                    T_E("ntp_mgmt_conf_set(): failed");
                }
            }
        }
        redirect(p, "/ntp.htm");
    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        (void)cyg_httpd_start_chunked("html");

        /* get form data
           Format: [ipv6_supported]/[ntp_mode]/[ntp_polling_interval]/[ntp_server1]/[ntp_server2]/[ntp_server3]/[ntp_server4]/[ntp_server5]
        */
        if (ntp_mgmt_conf_get(&conf) == VTSS_OK) {

#ifdef VTSS_SW_OPTION_IPV6
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "1/%u/%u/%s/%s/%s/%s/%s",
                          conf.mode_enabled,
                          conf.interval_min,
                          conf.server[0].ip_type == NTP_IP_TYPE_IPV4 ? (char *)conf.server[0].ip_host_string : misc_ipv6_txt(&conf.server[0].ipv6_addr, str_buff1),
                          conf.server[1].ip_type == NTP_IP_TYPE_IPV4 ? (char *)conf.server[1].ip_host_string : misc_ipv6_txt(&conf.server[1].ipv6_addr, str_buff2),
                          conf.server[2].ip_type == NTP_IP_TYPE_IPV4 ? (char *)conf.server[2].ip_host_string : misc_ipv6_txt(&conf.server[2].ipv6_addr, str_buff3),
                          conf.server[3].ip_type == NTP_IP_TYPE_IPV4 ? (char *)conf.server[3].ip_host_string : misc_ipv6_txt(&conf.server[3].ipv6_addr, str_buff4),
                          conf.server[4].ip_type == NTP_IP_TYPE_IPV4 ? (char *)conf.server[4].ip_host_string : misc_ipv6_txt(&conf.server[4].ipv6_addr, str_buff5));
#else
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "0/%u/%u/%s/%s/%s/%s/%s",
                          conf.mode_enabled,
                          conf.interval_min,
                          conf.server[0].ip_host_string,
                          conf.server[1].ip_host_string,
                          conf.server[2].ip_host_string,
                          conf.server[3].ip_host_string,
                          conf.server[4].ip_host_string);
#endif

            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        }
        (void)cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}
/****************************************************************************/
/*  Module Config JS lib routine                                            */
/****************************************************************************/

static size_t ntp_lib_config_js(char **base_ptr, char **cur_ptr, size_t *length)
{
    const char *buff =
        "function isNtpSupported() {\n"
        " return true;\n"
        "}\n\n";
    return webCommonBufferHandler(base_ptr, cur_ptr, length, buff);
}

/****************************************************************************/
/*  Config JS lib table entry                                               */
/****************************************************************************/
web_lib_config_js_tab_entry(ntp_lib_config_js);

/****************************************************************************/
/*  Module Filter CSS routine                                               */
/****************************************************************************/
static size_t ntp_lib_filter_css(char **base_ptr, char **cur_ptr, size_t *length)
{
    return webCommonBufferHandler(base_ptr, cur_ptr, length, ".NTP_CTRL { display: none; }\r\n");
}
/****************************************************************************/
/*  Filter CSS table entry                                                  */
/****************************************************************************/
web_lib_filter_css_tab_entry(ntp_lib_filter_css);

/****************************************************************************/
/*  HTTPD Table Entries                                                     */
/****************************************************************************/

CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_ntp, "/config/ntp", handler_config_ntp);

/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
