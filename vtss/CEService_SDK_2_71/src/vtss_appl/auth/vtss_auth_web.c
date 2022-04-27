/*

   Vitesse Switch API software.

 Copyright (c) 2002-2014 Vitesse Semiconductor Corporation "Vitesse". All
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
#include "vtss_auth_api.h"
#include "mgmt_api.h"
#ifdef VTSS_SW_OPTION_PRIV_LVL
#include "vtss_privilege_api.h"
#include "vtss_privilege_web_api.h"
#endif

#define AUTH_WEB_BUF_LEN 512

/****************************************************************************/
/*  Web Handler  functions                                                  */
/****************************************************************************/

#if defined(VTSS_SW_OPTION_RADIUS) || defined(VTSS_SW_OPTION_TACPLUS)
static void auth_web_host_cb(vtss_auth_proto_t proto, const void *const contxt, const vtss_auth_host_conf_t *const conf, int number)
{
    vtss_auth_host_conf_t *c = (vtss_auth_host_conf_t *)contxt;
    c[number] = *conf;
}
#endif /* defined(VTSS_SW_OPTION_RADIUS) || defined(VTSS_SW_OPTION_TACPLUS) */

#ifdef VTSS_SW_OPTION_RADIUS
cyg_int32 handler_config_auth_radius(CYG_HTTPD_STATE *p)
{
    vtss_auth_global_host_conf_t gc;
    vtss_auth_radius_conf_t      rac;
    char                         buf[INET6_ADDRSTRLEN];
    int                          cnt, i;
    vtss_rc                      rc;
    const char                   *str;
    ulong                        val = 0;
    size_t                       len;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_SECURITY)) {
        return -1;
    }
#endif

    if (p->method == CYG_HTTPD_METHOD_POST) {

        if ((rc = vtss_auth_mgmt_global_host_conf_get(VTSS_AUTH_PROTO_RADIUS, &gc)) != VTSS_OK) {
            const char *err = error_txt(rc);
            send_custom_error(p, "Authentication Error", err, strlen(err));
            return -1; // Do not further search the file system.
        }

        if (cyg_httpd_form_varable_long_int(p, "timeout", &val)) {
            gc.timeout = val;
        }
        if (cyg_httpd_form_varable_long_int(p, "retransmit", &val)) {
            gc.retransmit = val;
        }
        if (cyg_httpd_form_varable_long_int(p, "deadtime", &val)) {
            gc.deadtime = val;
        }
        if ((str = cyg_httpd_form_varable_string(p, "key", &len))) {
            (void) cgi_unescape(str, gc.key, len, sizeof(gc.key)); // Key needs to be cgi_unescaped (e.g. %20 -> ' ').
        }

        if ((rc = vtss_auth_mgmt_global_host_conf_set(VTSS_AUTH_PROTO_RADIUS, &gc)) != VTSS_OK) {
            const char *err = error_txt(rc);
            send_custom_error(p, "Authentication Error", err, strlen(err));
            return -1; // Do not further search the file system.
        }

        if ((rc = vtss_auth_mgmt_radius_conf_get(&rac)) != VTSS_OK) {
            const char *err = error_txt(rc);
            send_custom_error(p, "Authentication Error", err, strlen(err));
            return -1; // Do not further search the file system.
        }

        if ((str = cyg_httpd_form_varable_string(p, "nas-ip-address", &len))) {
            if (len) {
                (void) cgi_unescape(str, buf, len, sizeof(buf));
                if (mgmt_txt2ipv4(buf, &rac.nas_ip_address, NULL, FALSE) == VTSS_OK) { // This has already been verified in web page
                    rac.nas_ip_address_enable = TRUE;
                }
            } else {
                rac.nas_ip_address_enable = FALSE;
            }
        }
        if ((str = cyg_httpd_form_varable_string(p, "nas-ipv6-address", &len))) {
            if (len) {
                (void) cgi_unescape(str, buf, len, sizeof(buf));
                if (mgmt_txt2ipv6(buf, &rac.nas_ipv6_address) == VTSS_OK) { // This has already been verified in web page
                    rac.nas_ipv6_address_enable = TRUE;
                }
            } else {
                rac.nas_ipv6_address_enable = FALSE;
            }
        }
        if ((str = cyg_httpd_form_varable_string(p, "nas-identifier", &len))) {
            (void) cgi_unescape(str, rac.nas_identifier, len, sizeof(rac.nas_identifier));
        }

        if ((rc = vtss_auth_mgmt_radius_conf_set(&rac)) != VTSS_OK) {
            const char *err = error_txt(rc);
            send_custom_error(p, "Authentication Error", err, strlen(err));
            return -1; // Do not further search the file system.
        }

        // host_config
        for (i = 0; i < VTSS_AUTH_NUMBER_OF_SERVERS; i++ ) {
            vtss_auth_host_conf_t hc = {{0}};
            if ((str = cyg_httpd_form_variable_str_fmt(p, &len, "host_%d", i))) {
                (void) cgi_unescape(str, hc.host, len, sizeof(hc.host)); // Host needs to be cgi_unescaped (e.g. %20 -> ' ').

                if (cyg_httpd_form_variable_long_int_fmt(p, &val, "auth_port_%d", i)) {
                    hc.auth_port = val;
                }
                if (cyg_httpd_form_variable_long_int_fmt(p, &val, "acct_port_%d", i)) {
                    hc.acct_port = val;
                }
                if (cyg_httpd_form_variable_long_int_fmt(p, &val, "timeout_%d", i)) {
                    hc.timeout = val;
                }
                if (cyg_httpd_form_variable_long_int_fmt(p, &val, "retransmit_%d", i)) {
                    hc.retransmit = val;
                }
                if ((str = cyg_httpd_form_variable_str_fmt(p, &len, "key_%d", i))) {
                    (void) cgi_unescape(str, hc.key, len, sizeof(hc.key)); // Key needs to be cgi_unescaped (e.g. %20 -> ' ').
                }
                if (cyg_httpd_form_variable_check_fmt(p, "delete_%d", i)) {
                    rc = vtss_auth_mgmt_host_del(VTSS_AUTH_PROTO_RADIUS, &hc);
                } else {
                    rc = vtss_auth_mgmt_host_add(VTSS_AUTH_PROTO_RADIUS, &hc);
                }
                if (rc != VTSS_OK) {
                    const char *err = error_txt(rc);
                    send_custom_error(p, "Authentication Error", err, strlen(err));
                    return -1; // Do not further search the file system.
                }
            }
        }
        redirect(p, "/auth_radius_config.htm");
    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        int                   server_cnt;
        vtss_auth_host_conf_t hc[VTSS_AUTH_NUMBER_OF_SERVERS] = {{{0}}};
        char                  encoded_host[3 * VTSS_AUTH_HOST_LEN];
        char                  encoded_key[3 * VTSS_AUTH_KEY_LEN];
        char                  encoded_ipv4[3 * INET_ADDRSTRLEN];
        char                  encoded_ipv6[3 * INET6_ADDRSTRLEN];
        /*
          # Configuration format:
          #
          # <timeout>#<retransmit>#<deadtime>#<key>#<nas_ip_address>#<nas_ipv6_address>#<nas_identifier>#<host_config>
          #
          # timeout          :== unsigned int # global timeout
          # retransmit       :== unsigned int # global retransmit
          # deadtime         :== unsigned int # global deadtime
          # key              :== string       # global key
          # nas_ip_address   :== string       # global nas-ip-address (Attribute 4)
          # nas_ipv6_address :== string       # global nas-ipv6-address (Attribute 95)
          # nas_identifier   :== string       # global nas-ip-address (Attribute 32)
          #
          # host_config :== <host 0>/<host 1>/...<host n>
          #   host x := <hostname>|<auth_port>|<acct_port>|<timeout>|<retransmit>|<key>
          #     hostname   :== string
          #     auth_port  :== 0..0xffff
          #     acct_port  :== 0..0xffff
          #     timeout    :== unsigned int
          #     retransmit :== unsigned int
          #     key        :== string
          #
        */
        cyg_httpd_start_chunked("html");
        if ((vtss_auth_mgmt_global_host_conf_get(VTSS_AUTH_PROTO_RADIUS, &gc) == VTSS_OK) &&
            (vtss_auth_mgmt_radius_conf_get(&rac) == VTSS_OK)) {
            // timeout, retransmit, deadtime, key, nas_ip_address, nas_ipv6_address and nas_identifier
            (void) cgi_escape(gc.key, encoded_key);
            (void) cgi_escape(rac.nas_ip_address_enable ? misc_ipv4_txt(rac.nas_ip_address, buf) : "", encoded_ipv4);
            (void) cgi_escape(rac.nas_ipv6_address_enable ? misc_ipv6_txt(&rac.nas_ipv6_address, buf) : "", encoded_ipv6);
            (void) cgi_escape(rac.nas_identifier, encoded_host); // <- Using encoded_host here
            cnt = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u#%u#%u#%s#%s#%s#%s",
                           gc.timeout,
                           gc.retransmit,
                           gc.deadtime,
                           encoded_key,
                           encoded_ipv4,
                           encoded_ipv6,
                           encoded_host);
            cyg_httpd_write_chunked(p->outbuffer, cnt);

            // host_config
            if (vtss_auth_mgmt_host_iterate(VTSS_AUTH_PROTO_RADIUS, &hc[0], auth_web_host_cb, &server_cnt) == VTSS_OK) {
                for (i = 0; i < server_cnt; i++) {
                    (void) cgi_escape(hc[i].host, encoded_host);
                    (void) cgi_escape(hc[i].key, encoded_key);

                    cnt = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s%s|%u|%u|%u|%u|%s",
                                   (i == 0) ? "#" : "/",
                                   encoded_host,
                                   hc[i].auth_port,
                                   hc[i].acct_port,
                                   hc[i].timeout,
                                   hc[i].retransmit,
                                   encoded_key);
                    cyg_httpd_write_chunked(p->outbuffer, cnt);
                }
            }
        }
        cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}
#endif /* VTSS_SW_OPTION_RADIUS */

#ifdef VTSS_SW_OPTION_TACPLUS
cyg_int32 handler_config_auth_tacacs(CYG_HTTPD_STATE *p)
{
    vtss_auth_global_host_conf_t gc;
    int                          cnt, i;
    vtss_rc                      rc;
    const char                   *str;
    ulong                        val = 0;
    size_t                       len;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_SECURITY)) {
        return -1;
    }
#endif

    if (p->method == CYG_HTTPD_METHOD_POST) {

        if ((rc = vtss_auth_mgmt_global_host_conf_get(VTSS_AUTH_PROTO_TACACS, &gc)) != VTSS_OK) {
            const char *err = error_txt(rc);
            send_custom_error(p, "Authentication Error", err, strlen(err));
            return -1; // Do not further search the file system.
        }

        if (cyg_httpd_form_varable_long_int(p, "timeout", &val)) {
            gc.timeout = val;
        }
        if (cyg_httpd_form_varable_long_int(p, "deadtime", &val)) {
            gc.deadtime = val;
        }
        if ((str = cyg_httpd_form_varable_string(p, "key", &len))) {
            (void) cgi_unescape(str, gc.key, len, sizeof(gc.key)); // Key needs to be cgi_unescaped (e.g. %20 -> ' ').
        }

        if ((rc = vtss_auth_mgmt_global_host_conf_set(VTSS_AUTH_PROTO_TACACS, &gc)) != VTSS_OK) {
            const char *err = error_txt(rc);
            send_custom_error(p, "Authentication Error", err, strlen(err));
            return -1; // Do not further search the file system.
        }

        // host_config
        for (i = 0; i < VTSS_AUTH_NUMBER_OF_SERVERS; i++ ) {
            vtss_auth_host_conf_t hc = {{0}};
            if ((str = cyg_httpd_form_variable_str_fmt(p, &len, "host_%d", i))) {
                (void) cgi_unescape(str, hc.host, len, sizeof(hc.host)); // Host needs to be cgi_unescaped (e.g. %20 -> ' ').

                if (cyg_httpd_form_variable_long_int_fmt(p, &val, "port_%d", i)) {
                    hc.auth_port = val;
                }
                if (cyg_httpd_form_variable_long_int_fmt(p, &val, "timeout_%d", i)) {
                    hc.timeout = val;
                }
                if ((str = cyg_httpd_form_variable_str_fmt(p, &len, "key_%d", i))) {
                    (void) cgi_unescape(str, hc.key, len, sizeof(hc.key)); // Key needs to be cgi_unescaped (e.g. %20 -> ' ').
                }
                if (cyg_httpd_form_variable_check_fmt(p, "delete_%d", i)) {
                    rc = vtss_auth_mgmt_host_del(VTSS_AUTH_PROTO_TACACS, &hc);
                } else {
                    rc = vtss_auth_mgmt_host_add(VTSS_AUTH_PROTO_TACACS, &hc);
                }
                if (rc != VTSS_OK) {
                    const char *err = error_txt(rc);
                    send_custom_error(p, "Authentication Error", err, strlen(err));
                    return -1; // Do not further search the file system.
                }
            }
        }
        redirect(p, "/auth_tacacs_config.htm");
    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        int                   server_cnt;
        vtss_auth_host_conf_t hc[VTSS_AUTH_NUMBER_OF_SERVERS] = {{{0}}};
        char                  encoded_host[3 * VTSS_AUTH_HOST_LEN];
        char                  encoded_key[3 * VTSS_AUTH_HOST_LEN];
        /*
          # Configuration format:
          #
          # <timeout>#<deadtime>#<key>#<host_config>
          #
          # timeout    :== unsigned int # global timeout
          # deadtime   :== unsigned int # global deadtime
          # key        :== string       # global key
          #
          # host_config :== <host 0>/<host 1>/...<host n>
          #   host x := <hostname>|<port>|<timeout>|<key>
          #     hostname   :== string
          #     port       :== 0..0xffff
          #     timeout    :== unsigned int
          #     key        :== string
          #
        */
        cyg_httpd_start_chunked("html");
        if (vtss_auth_mgmt_global_host_conf_get(VTSS_AUTH_PROTO_TACACS, &gc) == VTSS_OK) {
            // timeout, retransmit, deadtime and key
            (void) cgi_escape(gc.key, encoded_key);
            cnt = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u#%u#%s",
                           gc.timeout,
                           gc.deadtime,
                           encoded_key);
            cyg_httpd_write_chunked(p->outbuffer, cnt);

            // host_config
            if (vtss_auth_mgmt_host_iterate(VTSS_AUTH_PROTO_TACACS, &hc[0], auth_web_host_cb, &server_cnt) == VTSS_OK) {
                for (i = 0; i < server_cnt; i++) {
                    (void) cgi_escape(hc[i].host, encoded_host);
                    (void) cgi_escape(hc[i].key, encoded_key);

                    cnt = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s%s|%u|%u|%s",
                                   (i == 0) ? "#" : "/",
                                   encoded_host,
                                   hc[i].auth_port,
                                   hc[i].timeout,
                                   encoded_key);
                    cyg_httpd_write_chunked(p->outbuffer, cnt);
                }
            }
        }
        cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}
#endif /* VTSS_SW_OPTION_TACPLUS */

cyg_int32 handler_config_auth_method(CYG_HTTPD_STATE *p)
{
    int                    cnt, i, j, var_value;
    vtss_rc                rc;
    vtss_auth_agent_conf_t ac;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_SECURITY)) {
        return -1;
    }
#endif

    if (p->method == CYG_HTTPD_METHOD_POST) {
        // client config
        for (i = 0; i < VTSS_AUTH_AGENT_LAST; i++ ) {
            if ((rc = vtss_auth_mgmt_agent_conf_get((vtss_auth_agent_t)i, &ac)) != VTSS_OK) {
                const char *err = error_txt(rc);
                send_custom_error(p, "Authentication Error", err, strlen(err));
                return -1; // Do not further search the file system.
            }
            for (j = 0; j < VTSS_AUTH_METHOD_LAST; j++ ) {
                // client_method_%d_%d: First %d is client, second is method
                if (cyg_httpd_form_variable_int_fmt(p, &var_value, "client_method_%d_%d", i, j)) {
                    ac.method[j] = var_value;
                }

            }
            if ((rc = vtss_auth_mgmt_agent_conf_set(i, &ac)) != VTSS_OK) {
                const char *err = error_txt(rc);
                send_custom_error(p, "Authentication Error", err, strlen(err));
                return -1; // Do not further search the file system.
            }
        }
        redirect(p, "/auth_method_config.htm");
    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        int method_cnt, client_cnt = 0;
        /*
          # Configuration format:
          #
          # <clients>#<method_name>#<method_num>
          #
          # clients         :== <client 0>/<client 1>/...<client n>
          #   client x      :== <client_name>|<client_num>|<methods>
          #     client_name :== "console" or "telnet" or "ssh" or "http"
          #     client_num  :== 0..3 # the corresponding value for client_name
          #     methods     :== <method 0>,<method 1>,...<method n> # List of configured methods. E.g {3,1,0} ~ {tacacs, local, no}
          #       method x  :== 0..3 # the method value
          #
          # method_name :== <name 0>/<name 1>/...<name n>
          #   name x    :== "no" or "local" or "radius" or "tacacs"
          #
          # method_num  :== <num 0>/<num 1>/...<num n>
          #   num x     :== 0..3 # the corresponding value for method_name
          #
        */
        cyg_httpd_start_chunked("html");
#ifdef VTSS_AUTH_ENABLE_CONSOLE
        if (vtss_auth_mgmt_agent_conf_get(VTSS_AUTH_AGENT_CONSOLE, &ac) == VTSS_OK) {
            cnt = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s%s|%d|",
                           client_cnt++ ? "/" : "",
                           vtss_auth_agent_names[VTSS_AUTH_AGENT_CONSOLE],
                           VTSS_AUTH_AGENT_CONSOLE);
            cyg_httpd_write_chunked(p->outbuffer, cnt);
            method_cnt = 0;
            for (i = 0; i < VTSS_AUTH_METHOD_LAST; i++) {
                cnt = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s%d",
                               method_cnt++ ? "," : "",
                               ac.method[i]);
                cyg_httpd_write_chunked(p->outbuffer, cnt);
            }
        }
#endif

#ifdef VTSS_SW_OPTION_CLI_TELNET
        if (vtss_auth_mgmt_agent_conf_get(VTSS_AUTH_AGENT_TELNET, &ac) == VTSS_OK) {
            cnt = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s%s|%d|",
                           client_cnt++ ? "/" : "",
                           vtss_auth_agent_names[VTSS_AUTH_AGENT_TELNET],
                           VTSS_AUTH_AGENT_TELNET);
            cyg_httpd_write_chunked(p->outbuffer, cnt);
            method_cnt = 0;
            for (i = 0; i < VTSS_AUTH_METHOD_LAST; i++) {
                cnt = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s%d",
                               method_cnt++ ? "," : "",
                               ac.method[i]);
                cyg_httpd_write_chunked(p->outbuffer, cnt);
            }
        }
#endif

#ifdef VTSS_SW_OPTION_SSH
        if (vtss_auth_mgmt_agent_conf_get(VTSS_AUTH_AGENT_SSH, &ac) == VTSS_OK) {
            cnt = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s%s|%d|",
                           client_cnt++ ? "/" : "",
                           vtss_auth_agent_names[VTSS_AUTH_AGENT_SSH],
                           VTSS_AUTH_AGENT_SSH);
            cyg_httpd_write_chunked(p->outbuffer, cnt);
            method_cnt = 0;
            for (i = 0; i < VTSS_AUTH_METHOD_LAST; i++) {
                cnt = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s%d",
                               method_cnt++ ? "," : "",
                               ac.method[i]);
                cyg_httpd_write_chunked(p->outbuffer, cnt);
            }
        }
#endif

#ifdef VTSS_SW_OPTION_WEB
        if (vtss_auth_mgmt_agent_conf_get(VTSS_AUTH_AGENT_HTTP, &ac) == VTSS_OK) {
            cnt = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s%s|%d|",
                           client_cnt++ ? "/" : "",
                           vtss_auth_agent_names[VTSS_AUTH_AGENT_HTTP],
                           VTSS_AUTH_AGENT_HTTP);
            cyg_httpd_write_chunked(p->outbuffer, cnt);
            method_cnt = 0;
            for (i = 0; i < VTSS_AUTH_METHOD_LAST; i++) {
                cnt = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s%d",
                               method_cnt++ ? "," : "",
                               ac.method[i]);
                cyg_httpd_write_chunked(p->outbuffer, cnt);
            }
        }
#endif

        cnt = snprintf(p->outbuffer, sizeof(p->outbuffer), "#");
        cyg_httpd_write_chunked(p->outbuffer, cnt);

        // method_name
        for (i = 0; i < VTSS_AUTH_METHOD_LAST; i++ ) {
#ifndef VTSS_SW_OPTION_RADIUS
            if (i == VTSS_AUTH_METHOD_RADIUS) {
                continue;
            }
#endif
#ifndef VTSS_SW_OPTION_TACPLUS
            if (i == VTSS_AUTH_METHOD_TACACS) {
                continue;
            }
#endif
            cnt = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s%s",
                           (i == 0) ? "" : "/",
                           vtss_auth_method_names[i]);
            cyg_httpd_write_chunked(p->outbuffer, cnt);
        }

        cnt = snprintf(p->outbuffer, sizeof(p->outbuffer), "#");
        cyg_httpd_write_chunked(p->outbuffer, cnt);

        // method_num
        for (i = 0; i < VTSS_AUTH_METHOD_LAST; i++ ) {
#ifndef VTSS_SW_OPTION_RADIUS
            if (i == VTSS_AUTH_METHOD_RADIUS) {
                continue;
            }
#endif
#ifndef VTSS_SW_OPTION_TACPLUS
            if (i == VTSS_AUTH_METHOD_TACACS) {
                continue;
            }
#endif
            cnt = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s%d",
                           (i == 0) ? "" : "/",
                           i);
            cyg_httpd_write_chunked(p->outbuffer, cnt);
        }
        cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}

/****************************************************************************/
/*  Module JS lib config routine                                            */
/****************************************************************************/

static size_t auth_lib_config_js(char **base_ptr, char **cur_ptr, size_t *length)
{
    char buff[AUTH_WEB_BUF_LEN];
    (void) snprintf(buff, AUTH_WEB_BUF_LEN,
                    "var configAuthServerCnt = %d;\n"
                    "var configAuthHostLen = %d;\n"
                    "var configAuthKeyLen = %d;\n"
                    "var configAuthRadiusAuthPortDef = %d;\n"
                    "var configAuthRadiusAcctPortDef = %d;\n"
                    "var configAuthTacacsPortDef = %d;\n"
                    "var configAuthTimeoutDef = %d;\n"
                    "var configAuthTimeoutMin = %d;\n"
                    "var configAuthTimeoutMax = %d;\n"
                    "var configAuthRetransmitDef = %d;\n"
                    "var configAuthRetransmitMin = %d;\n"
                    "var configAuthRetransmitMax = %d;\n"
                    "var configAuthDeadtimeDef = %d;\n"
                    "var configAuthDeadtimeMin = %d;\n"
                    "var configAuthDeadtimeMax = %d;\n"
                    ,
                    VTSS_AUTH_NUMBER_OF_SERVERS,
                    VTSS_AUTH_HOST_LEN - 1,
                    VTSS_AUTH_KEY_LEN - 1,
                    VTSS_AUTH_RADIUS_AUTH_PORT_DEFAULT,
                    VTSS_AUTH_RADIUS_ACCT_PORT_DEFAULT,
                    VTSS_AUTH_TACACS_PORT_DEFAULT,
                    VTSS_AUTH_TIMEOUT_DEFAULT,
                    VTSS_AUTH_TIMEOUT_MIN,
                    VTSS_AUTH_TIMEOUT_MAX,
                    VTSS_AUTH_RETRANSMIT_DEFAULT,
                    VTSS_AUTH_RETRANSMIT_MIN,
                    VTSS_AUTH_RETRANSMIT_MAX,
                    VTSS_AUTH_DEADTIME_DEFAULT,
                    VTSS_AUTH_DEADTIME_MIN,
                    VTSS_AUTH_DEADTIME_MAX);
    return webCommonBufferHandler(base_ptr, cur_ptr, length, buff);
}

/****************************************************************************/
/*  JS lib config table entry                                               */
/****************************************************************************/

web_lib_config_js_tab_entry(auth_lib_config_js);

#if !defined(VTSS_SW_OPTION_RADIUS) && !defined(VTSS_SW_OPTION_DOT1X_ACCT) && !defined(VTSS_SW_OPTION_TACPLUS)
/****************************************************************************/
/*  Module Filter CSS routine                                               */
/****************************************************************************/
static size_t auth_lib_filter_css(char **base_ptr, char **cur_ptr, size_t *length)
{
    char buff[AUTH_WEB_BUF_LEN];
    (void) snprintf(buff, AUTH_WEB_BUF_LEN, ".AUTH_SERVER { display: none; }\r\n");
    return webCommonBufferHandler(base_ptr, cur_ptr, length, buff);
}

/****************************************************************************/
/*  Filter CSS table entry                                                  */
/****************************************************************************/
web_lib_filter_css_tab_entry(auth_lib_filter_css);
#endif /* !defined(VTSS_SW_OPTION_RADIUS) && !defined(VTSS_SW_OPTION_DOT1X_ACCT) && !defined(VTSS_SW_OPTION_TACPLUS) */

/****************************************************************************/
/*  HTTPD Table Entries                                                     */
/****************************************************************************/

#ifdef VTSS_SW_OPTION_RADIUS
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_auth_radius, "/config/auth_radius_config", handler_config_auth_radius);
#endif /* VTSS_SW_OPTION_RADIUS */

#ifdef VTSS_SW_OPTION_TACPLUS
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_auth_tacacs, "/config/auth_tacacs_config", handler_config_auth_tacacs);
#endif /* VTSS_SW_OPTION_TACPLUS */

CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_auth_method, "/config/auth_method_config", handler_config_auth_method);

/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
