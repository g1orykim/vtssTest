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

/*
******************************************************************************

    Include files

******************************************************************************
*/
#include "icfg_api.h"
#include "vtss_auth_api.h"
#include "vtss_auth_icfg.h"
#include "misc_api.h"
#if defined(VTSS_SW_OPTION_RADIUS)
#include <netinet/in.h>
#endif /* defined(VTSS_SW_OPTION_RADIUS) */

/*
******************************************************************************

    Constant and Macro definition

******************************************************************************
*/
#undef IC_RC
#define IC_RC ICLI_VTSS_RC

/*
******************************************************************************

    Data structure type definition

******************************************************************************
*/

/*
******************************************************************************

    Static Function

******************************************************************************
*/

#if defined(VTSS_AUTH_ENABLE_CONSOLE) || defined(VTSS_SW_OPTION_CLI_TELNET) || defined(VTSS_SW_OPTION_SSH) || defined(VTSS_SW_OPTION_WEB)
static vtss_rc VTSS_AUTH_ICFG_agent_print(const vtss_icfg_query_request_t *req, vtss_icfg_query_result_t *result, vtss_auth_agent_t agent)
{
    vtss_auth_agent_conf_t conf;
    vtss_auth_method_t     method;
    int                    i;

    IC_RC(vtss_auth_mgmt_agent_conf_get(agent, &conf));
    if (req->all_defaults || memcmp(&conf, &vtss_auth_agent_conf_default, sizeof(conf))) {
        if (conf.method[0] == VTSS_AUTH_METHOD_NONE) {
            IC_RC(vtss_icfg_printf(result, "no aaa authentication login %s\n", vtss_auth_agent_names[agent]));
        } else {
            IC_RC(vtss_icfg_printf(result, "aaa authentication login %s", vtss_auth_agent_names[agent]));
            for (i = 0; i < VTSS_AUTH_METHOD_LAST; i++) {
                method = conf.method[i];
                if (method == VTSS_AUTH_METHOD_NONE) {
                    break;
                }
                IC_RC(vtss_icfg_printf(result, " %s", vtss_auth_method_names[method]));
            }
            IC_RC(vtss_icfg_printf(result, "\n"));
        }
    }
    return VTSS_OK;
}

static vtss_rc VTSS_AUTH_ICFG_agent_conf(const vtss_icfg_query_request_t *req, vtss_icfg_query_result_t *result)
{
#ifdef VTSS_AUTH_ENABLE_CONSOLE
    IC_RC(VTSS_AUTH_ICFG_agent_print(req, result, VTSS_AUTH_AGENT_CONSOLE));
#endif
#ifdef VTSS_SW_OPTION_CLI_TELNET
    IC_RC(VTSS_AUTH_ICFG_agent_print(req, result, VTSS_AUTH_AGENT_TELNET));
#endif
#ifdef VTSS_SW_OPTION_SSH
    IC_RC(VTSS_AUTH_ICFG_agent_print(req, result, VTSS_AUTH_AGENT_SSH));
#endif
#ifdef VTSS_SW_OPTION_WEB
    IC_RC(VTSS_AUTH_ICFG_agent_print(req, result, VTSS_AUTH_AGENT_HTTP));
#endif
    return VTSS_OK;
}
#endif /* defined(VTSS_AUTH_ENABLE_CONSOLE) || defined(VTSS_SW_OPTION_CLI_TELNET) || defined(VTSS_SW_OPTION_SSH) || defined(VTSS_SW_OPTION_WEB) */

#if defined(VTSS_SW_OPTION_RADIUS)
static vtss_rc VTSS_AUTH_ICFG_radius_print(const vtss_icfg_query_request_t *req, vtss_icfg_query_result_t *result, const char *pname)
{
    vtss_auth_radius_conf_t conf;
    char                    buf[INET6_ADDRSTRLEN];

    IC_RC(vtss_auth_mgmt_radius_conf_get(&conf));
    if (req->all_defaults || (conf.nas_ip_address_enable)) {
        if (conf.nas_ip_address_enable) {
            IC_RC(vtss_icfg_printf(result, "%s-server attribute 4 %s\n", pname, misc_ipv4_txt(conf.nas_ip_address, buf)));
        } else {
            IC_RC(vtss_icfg_printf(result, "no %s-server attribute 4\n", pname));
        }
    }
#ifdef VTSS_SW_OPTION_IPV6
    if (req->all_defaults || (conf.nas_ipv6_address_enable)) {
        if (conf.nas_ipv6_address_enable) {
            IC_RC(vtss_icfg_printf(result, "%s-server attribute 95 %s\n", pname, misc_ipv6_txt(&conf.nas_ipv6_address, buf)));
        } else {
            IC_RC(vtss_icfg_printf(result, "no %s-server attribute 95\n", pname));
        }
    }
#endif /* VTSS_SW_OPTION_IPV6 */
    if (req->all_defaults || (conf.nas_identifier[0])) {
        if (conf.nas_identifier[0]) {
            IC_RC(vtss_icfg_printf(result, "%s-server attribute 32 %s\n", pname, conf.nas_identifier));
        } else {
            IC_RC(vtss_icfg_printf(result, "no %s-server attribute 32\n", pname));
        }
    }
    return VTSS_OK;
}
#endif /* defined(VTSS_SW_OPTION_RADIUS) */

#if defined(VTSS_SW_OPTION_RADIUS) || defined(VTSS_SW_OPTION_TACPLUS)
static vtss_rc VTSS_AUTH_ICFG_global_host_print(const vtss_icfg_query_request_t *req, vtss_icfg_query_result_t *result,
                                                vtss_auth_proto_t proto, const char *pname)
{
    vtss_auth_global_host_conf_t conf;

    IC_RC(vtss_auth_mgmt_global_host_conf_get(proto, &conf));
    if (req->all_defaults || (conf.timeout != VTSS_AUTH_TIMEOUT_DEFAULT)) {
        if (conf.timeout == VTSS_AUTH_TIMEOUT_DEFAULT) {
            IC_RC(vtss_icfg_printf(result, "no %s-server timeout\n", pname));
        } else {
            IC_RC(vtss_icfg_printf(result, "%s-server timeout %u\n", pname, conf.timeout));
        }
    }
    if ((proto == VTSS_AUTH_PROTO_RADIUS) && (req->all_defaults || (conf.retransmit != VTSS_AUTH_RETRANSMIT_DEFAULT))) {
        if (conf.retransmit == VTSS_AUTH_RETRANSMIT_DEFAULT) {
            IC_RC(vtss_icfg_printf(result, "no %s-server retransmit\n", pname));
        } else {
            IC_RC(vtss_icfg_printf(result, "%s-server retransmit %u\n", pname, conf.retransmit));
        }
    }
    if (req->all_defaults || (conf.deadtime != VTSS_AUTH_DEADTIME_DEFAULT)) {
        if (conf.deadtime == VTSS_AUTH_DEADTIME_DEFAULT) {
            IC_RC(vtss_icfg_printf(result, "no %s-server deadtime\n", pname));
        } else {
            IC_RC(vtss_icfg_printf(result, "%s-server deadtime %u\n", pname, conf.deadtime));
        }
    }
    if (req->all_defaults || (conf.key[0])) {
        if (conf.key[0] == 0) {
            IC_RC(vtss_icfg_printf(result, "no %s-server key\n", pname));
        } else {
            IC_RC(vtss_icfg_printf(result, "%s-server key %s\n", pname, conf.key));
        }
    }
    return VTSS_OK;
}

static void VTSS_AUTH_ICFG_host_cb(vtss_auth_proto_t proto, const void *const contxt, const vtss_auth_host_conf_t *const conf, int number)
{
    vtss_auth_host_conf_t *c = (vtss_auth_host_conf_t *)contxt;
    c[number] = *conf;
}

static vtss_rc VTSS_AUTH_ICFG_host_print(const vtss_icfg_query_request_t *req, vtss_icfg_query_result_t *result,
                                         vtss_auth_proto_t proto, const char *pname)
{
    vtss_auth_host_conf_t hc[VTSS_AUTH_NUMBER_OF_SERVERS] = {{{0}}};
    int i, cnt;

    if (vtss_auth_mgmt_host_iterate(proto, &hc[0], VTSS_AUTH_ICFG_host_cb, &cnt) == VTSS_OK) {
        for (i = 0; i < cnt; i++) {
            IC_RC(vtss_icfg_printf(result, "%s-server host %s", pname, hc[i].host));
            if (proto == VTSS_AUTH_PROTO_RADIUS) {
                if (req->all_defaults || (hc[i].auth_port != VTSS_AUTH_RADIUS_AUTH_PORT_DEFAULT)) {
                    IC_RC(vtss_icfg_printf(result, " auth-port %u", hc[i].auth_port));
                }
                if (req->all_defaults || (hc[i].acct_port != VTSS_AUTH_RADIUS_ACCT_PORT_DEFAULT)) {
                    IC_RC(vtss_icfg_printf(result, " acct-port %u", hc[i].acct_port));
                }
            } else {
                if (req->all_defaults || (hc[i].auth_port != VTSS_AUTH_TACACS_PORT_DEFAULT)) {
                    IC_RC(vtss_icfg_printf(result, " port %u", hc[i].auth_port));
                }
            }
            if (hc[i].timeout) {
                IC_RC(vtss_icfg_printf(result, " timeout %u", hc[i].timeout));
            }
            if ((proto == VTSS_AUTH_PROTO_RADIUS) && (hc[i].retransmit)) {
                IC_RC(vtss_icfg_printf(result, " retransmit %u", hc[i].retransmit));
            }
            if ((hc[i].key[0])) {
                IC_RC(vtss_icfg_printf(result, " key %s", hc[i].key));
            }
            IC_RC(vtss_icfg_printf(result, "\n"));
        }
    }
    return VTSS_OK;
}
#endif /* defined(VTSS_SW_OPTION_RADIUS) || defined(VTSS_SW_OPTION_TACPLUS) */

#if defined(VTSS_SW_OPTION_RADIUS)
static vtss_rc VTSS_AUTH_ICFG_radius_conf(const vtss_icfg_query_request_t *req, vtss_icfg_query_result_t *result)
{
    IC_RC(VTSS_AUTH_ICFG_global_host_print(req, result, VTSS_AUTH_PROTO_RADIUS, "radius"));
    IC_RC(VTSS_AUTH_ICFG_radius_print(req, result, "radius"));
    IC_RC(VTSS_AUTH_ICFG_host_print(req, result, VTSS_AUTH_PROTO_RADIUS, "radius"));
    return VTSS_OK;
}
#endif /* defined(VTSS_SW_OPTION_RADIUS) */

#if defined(VTSS_SW_OPTION_TACPLUS)
static vtss_rc VTSS_AUTH_ICFG_tacacs_conf(const vtss_icfg_query_request_t *req, vtss_icfg_query_result_t *result)
{
    IC_RC(VTSS_AUTH_ICFG_global_host_print(req, result, VTSS_AUTH_PROTO_TACACS, "tacacs"));
    IC_RC(VTSS_AUTH_ICFG_host_print(req, result, VTSS_AUTH_PROTO_TACACS, "tacacs"));
    return VTSS_OK;
}
#endif /* defined(VTSS_SW_OPTION_TACPLUS) */

/*
******************************************************************************

    Public functions

******************************************************************************
*/

/* Initialization function */
vtss_rc vtss_auth_icfg_init(void)
{
#if defined(VTSS_AUTH_ENABLE_CONSOLE) || defined(VTSS_SW_OPTION_CLI_TELNET) || defined(VTSS_SW_OPTION_SSH) || defined(VTSS_SW_OPTION_WEB)
    IC_RC(vtss_icfg_query_register(VTSS_ICFG_AUTH_AGENT_CONF, "auth", VTSS_AUTH_ICFG_agent_conf));
#endif /* defined(VTSS_AUTH_ENABLE_CONSOLE) || defined(VTSS_SW_OPTION_CLI_TELNET) || defined(VTSS_SW_OPTION_SSH) || defined(VTSS_SW_OPTION_WEB) */

#if defined(VTSS_SW_OPTION_RADIUS)
    IC_RC(vtss_icfg_query_register(VTSS_ICFG_AUTH_RADIUS_CONF, "auth", VTSS_AUTH_ICFG_radius_conf));
#endif /* defined(VTSS_SW_OPTION_RADIUS) */

#if defined(VTSS_SW_OPTION_TACPLUS)
    IC_RC(vtss_icfg_query_register(VTSS_ICFG_AUTH_TACACS_CONF, "auth", VTSS_AUTH_ICFG_tacacs_conf));
#endif /* defined(VTSS_SW_OPTION_TACPLUS) */
    return VTSS_OK;
}
