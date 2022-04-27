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
#include "access_mgmt_api.h"
#include "ip2_legacy.h"
#ifdef VTSS_SW_OPTION_PRIV_LVL
#include "vtss_privilege_api.h"
#include "vtss_privilege_web_api.h"
#endif

#define ACCESS_MGMT_WEB_BUF_LEN 512

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

static cyg_int32 handler_config_access_mgmt(CYG_HTTPD_STATE *p)
{
    int                 ct;
    access_mgmt_conf_t  conf, newconf;
    size_t              len;
    int                 var_value, idx, access_id;
    char                str_buff1[40], str_buff2[40];
#if defined(VTSS_SW_OPTION_IPV6)
    int                 ipv6_supported = 1;
#else
    int                 ipv6_supported = 0;
#endif /* VTSS_SW_OPTION_IPV6 */
#if defined(VTSS_SW_OPTION_SNMP)
    int                 snmp_supported = 1;
#else
    int                 snmp_supported = 0;
#endif /* VTSS_SW_OPTION_SNMP */
#if defined(VTSS_SW_OPTION_CLI_TELNET) || defined(VTSS_SW_OPTION_SSH)
    int                 telnet_supported = 1;
#else
    int                 telnet_supported = 0;
#endif /* VTSS_SW_OPTION_CLI_TELNET || VTSS_SW_OPTION_SSH */

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_SECURITY)) {
        return -1;
    }
#endif

    if (p->method == CYG_HTTPD_METHOD_POST) {
        if (access_mgmt_conf_get(&conf) != VTSS_OK) {
            T_W("access_mgmt_conf_get(): failed");
            redirect(p, "/access_mgmt.htm");
            return -1;
        }

        /* store form data */
        newconf = conf;

        /* mode */
        if (cyg_httpd_form_varable_int(p, "mode", &var_value)) {
            newconf.mode = var_value;
        }

        /* delete entry */
        for (access_id = ACCESS_MGMT_ACCESS_ID_START; access_id < ACCESS_MGMT_MAX_ENTRIES + ACCESS_MGMT_ACCESS_ID_START; access_id++) {
            if (cyg_httpd_form_variable_str_fmt(p, &len, "del_%d", access_id) && len > 0) {
                /* delete the entry */
                if (newconf.entry[access_id].valid) {
                    if (newconf.entry_num > 0) {
                        newconf.entry_num--;
                    }
                    memset(&newconf.entry[access_id], 0x0, sizeof(newconf.entry[access_id]));
                }
            } else {
                /* modify the entry */
                //vid
                sprintf(str_buff1, "vid_%d", access_id);
                if (cyg_httpd_form_varable_int(p, str_buff1, &var_value)) {
                    newconf.entry[access_id].vid = (vtss_vid_t) var_value;
                }

                //start_ipaddr
                sprintf(str_buff1, "start_ipaddr_%d", access_id);
                if (cyg_httpd_form_varable_ipv4(p, str_buff1, &newconf.entry[access_id].start_ip) == FALSE) {
#ifdef VTSS_SW_OPTION_IPV6
                    if (web_parse_ipv6(p, str_buff1, &newconf.entry[access_id].start_ipv6) == VTSS_OK) {
                        newconf.entry[access_id].start_ip = 0;
                        newconf.entry[access_id].end_ip = 0;
                        newconf.entry[access_id].entry_type = ACCESS_MGMT_ENTRY_TYPE_IPV6;
                    }
#endif /* VTSS_SW_OPTION_IPV6 */
                }

                //end_ipaddr
                sprintf(str_buff1, "end_ipaddr_%d", access_id);
                if (cyg_httpd_form_varable_ipv4(p, str_buff1, &newconf.entry[access_id].end_ip) == FALSE) {
#ifdef VTSS_SW_OPTION_IPV6
                    (void) web_parse_ipv6(p, str_buff1, &newconf.entry[access_id].end_ipv6);
#endif /* VTSS_SW_OPTION_IPV6 */
                }

                //swap start_ipaddr and end_ipaddr
                if (newconf.entry[access_id].entry_type == ACCESS_MGMT_ENTRY_TYPE_IPV4) {
                    if (newconf.entry[access_id].start_ip > newconf.entry[access_id].end_ip) {
                        ulong temp_ip = newconf.entry[access_id].start_ip;
                        newconf.entry[access_id].start_ip = newconf.entry[access_id].end_ip;
                        newconf.entry[access_id].end_ip = temp_ip;
                    }
                }
#ifdef VTSS_SW_OPTION_IPV6
                else if (memcmp(&newconf.entry[access_id].start_ipv6, &newconf.entry[access_id].end_ipv6, sizeof(vtss_ipv6_t)) > 0) {
                    vtss_ipv6_t temp_ipv6addr = newconf.entry[access_id].start_ipv6;
                    newconf.entry[access_id].start_ipv6 = newconf.entry[access_id].end_ipv6;
                    newconf.entry[access_id].end_ipv6 = temp_ipv6addr;
                }
#endif /* VTSS_SW_OPTION_IPV6 */

                //web
                if (cyg_httpd_form_variable_str_fmt(p, &len, "web_%d", access_id) && len > 0) {
                    newconf.entry[access_id].service_type |= ACCESS_MGMT_SERVICES_TYPE_WEB;
                } else {
                    newconf.entry[access_id].service_type &= (~ACCESS_MGMT_SERVICES_TYPE_WEB);
                }

#if defined(VTSS_SW_OPTION_SNMP)
                //snmp
                if (cyg_httpd_form_variable_str_fmt(p, &len, "snmp_%d", access_id) && len > 0) {
                    newconf.entry[access_id].service_type |= ACCESS_MGMT_SERVICES_TYPE_SNMP;
                } else {
                    newconf.entry[access_id].service_type &= (~ACCESS_MGMT_SERVICES_TYPE_SNMP);
                }
#endif /* VTSS_SW_OPTION_SNMP */

#if defined(VTSS_SW_OPTION_CLI_TELNET) || defined(VTSS_SW_OPTION_SSH)
                //telnet
                if (cyg_httpd_form_variable_str_fmt(p, &len, "telnet_%d", access_id) && len > 0) {
                    newconf.entry[access_id].service_type |= ACCESS_MGMT_SERVICES_TYPE_TELNET;
                } else {
                    newconf.entry[access_id].service_type &= (~ACCESS_MGMT_SERVICES_TYPE_TELNET);
                }
#endif /* VTSS_SW_OPTION_CLI_TELNET || VTSS_SW_OPTION_SSH */
            }
        }

        /* Add new entry */
        for (idx = ACCESS_MGMT_ACCESS_ID_START; idx < ACCESS_MGMT_MAX_ENTRIES + ACCESS_MGMT_ACCESS_ID_START; idx++) {
            //check if has new entry
            sprintf(str_buff1, "new_start_ipaddr_%d", idx);
            (void) cyg_httpd_form_varable_string(p, str_buff1, &len);
            if (len <= 0) {
                continue;
            }

            //find available entry
            access_id = ACCESS_MGMT_ACCESS_ID_START;
            while (newconf.entry[access_id].valid) {
                access_id++;
            }
            if (access_id >= (ACCESS_MGMT_MAX_ENTRIES + ACCESS_MGMT_ACCESS_ID_START)) {
                break;
            }

            //new_vid
            sprintf(str_buff1, "new_vid_%d", idx);
            if (cyg_httpd_form_varable_int(p, str_buff1, &var_value)) {
                newconf.entry[access_id].vid = (vtss_vid_t) var_value;
            }

            //new_start_ipaddr
            sprintf(str_buff1, "new_start_ipaddr_%d", idx);
            if (cyg_httpd_form_varable_ipv4(p, str_buff1, &newconf.entry[access_id].start_ip) == FALSE) {
#ifdef VTSS_SW_OPTION_IPV6
                if (web_parse_ipv6(p, str_buff1, &newconf.entry[access_id].start_ipv6) == VTSS_OK) {
                    newconf.entry[access_id].start_ip = 0;
                    newconf.entry[access_id].end_ip = 0;
                    newconf.entry[access_id].entry_type = ACCESS_MGMT_ENTRY_TYPE_IPV6;
                }
#endif /* VTSS_SW_OPTION_IPV6 */
            } else {
                newconf.entry[access_id].entry_type = ACCESS_MGMT_ENTRY_TYPE_IPV4;
            }

            //new_end_ipaddr
            sprintf(str_buff1, "new_end_ipaddr_%d", idx);
            if (cyg_httpd_form_varable_ipv4(p, str_buff1, &newconf.entry[access_id].end_ip) == FALSE) {
#ifdef VTSS_SW_OPTION_IPV6
                (void) web_parse_ipv6(p, str_buff1, &newconf.entry[access_id].end_ipv6);
#endif /* VTSS_SW_OPTION_IPV6 */
            }

            //swap new_start_ipaddr and new_end_ipaddr
            if (newconf.entry[access_id].entry_type == ACCESS_MGMT_ENTRY_TYPE_IPV4) {
                if (newconf.entry[access_id].start_ip > newconf.entry[access_id].end_ip) {
                    ulong temp_ip = newconf.entry[access_id].start_ip;
                    newconf.entry[access_id].start_ip = newconf.entry[access_id].end_ip;
                    newconf.entry[access_id].end_ip = temp_ip;
                }
            }
#ifdef VTSS_SW_OPTION_IPV6
            else if (memcmp(&newconf.entry[access_id].start_ipv6, &newconf.entry[access_id].end_ipv6, sizeof(vtss_ipv6_t))) {
                vtss_ipv6_t temp_ipv6addr = newconf.entry[access_id].start_ipv6;
                newconf.entry[access_id].start_ipv6 = newconf.entry[access_id].end_ipv6;
                newconf.entry[access_id].end_ipv6 = temp_ipv6addr;
            }
#endif /* VTSS_SW_OPTION_IPV6 */

            //new_web
            if (cyg_httpd_form_variable_str_fmt(p, &len, "new_web_%d", idx) && len > 0) {
                newconf.entry[access_id].service_type |= ACCESS_MGMT_SERVICES_TYPE_WEB;
            } else {
                newconf.entry[access_id].service_type &= (~ACCESS_MGMT_SERVICES_TYPE_WEB);
            }

            //new_snmp
            if (cyg_httpd_form_variable_str_fmt(p, &len, "new_snmp_%d", idx) && len > 0) {
                newconf.entry[access_id].service_type |= ACCESS_MGMT_SERVICES_TYPE_SNMP;
            } else {
                newconf.entry[access_id].service_type &= (~ACCESS_MGMT_SERVICES_TYPE_SNMP);
            }

            //new_telnet
            if (cyg_httpd_form_variable_str_fmt(p, &len, "new_telnet_%d", idx) && len > 0) {
                newconf.entry[access_id].service_type |= ACCESS_MGMT_SERVICES_TYPE_TELNET;
            } else {
                newconf.entry[access_id].service_type &= (~ACCESS_MGMT_SERVICES_TYPE_TELNET);
            }

            if (!newconf.entry[access_id].valid) {
                newconf.entry_num++;
                newconf.entry[access_id].valid = 1;
            }
        }

        if (memcmp(&newconf, &conf, sizeof(newconf)) != 0) {
            if (access_mgmt_conf_set(&newconf) != VTSS_OK) {
                T_D("access_mgmt_conf_set(): failed");
            }
        }

        redirect(p, "/access_mgmt.htm");
    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        (void)cyg_httpd_start_chunked("html");

        if (access_mgmt_conf_get(&conf) != VTSS_OK) {
            T_W("access_mgmt_conf_get(): failed");
            (void)cyg_httpd_end_chunked();
            return -1;
        }

        /* get form data
           Format: [ipv6_supported],[snmp_supported],[telnet_supported],[mode],[max_enetries_num],[access_id]/[vid]/[start_ipaddr]/[end_ipaddr]/[web]/[snmp]/[telnet]|...
        */
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d,%d,%d,%u,%u,", ipv6_supported, snmp_supported, telnet_supported, conf.mode, ACCESS_MGMT_MAX_ENTRIES);
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        for (access_id = ACCESS_MGMT_ACCESS_ID_START; access_id < ACCESS_MGMT_MAX_ENTRIES + ACCESS_MGMT_ACCESS_ID_START; access_id++) {
            if (conf.entry[access_id].valid) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d/",
                              access_id);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d/",
                              conf.entry[access_id].vid);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s/%s/%d/%d/%d|",
#ifdef VTSS_SW_OPTION_IPV6
                              conf.entry[access_id].entry_type == ACCESS_MGMT_ENTRY_TYPE_IPV4 ? misc_ipv4_txt(conf.entry[access_id].start_ip, str_buff1) : misc_ipv6_txt(&conf.entry[access_id].start_ipv6, str_buff1),
                              conf.entry[access_id].entry_type == ACCESS_MGMT_ENTRY_TYPE_IPV4 ? misc_ipv4_txt(conf.entry[access_id].end_ip, str_buff2) : misc_ipv6_txt(&conf.entry[access_id].end_ipv6, str_buff2),
#else
                              misc_ipv4_txt(conf.entry[access_id].start_ip, str_buff1),
                              misc_ipv4_txt(conf.entry[access_id].end_ip, str_buff2),
#endif /* VTSS_SW_OPTION_IPV6 */
                              conf.entry[access_id].service_type & ACCESS_MGMT_SERVICES_TYPE_WEB ? 1 : 0,
                              conf.entry[access_id].service_type & ACCESS_MGMT_SERVICES_TYPE_SNMP ? 1 : 0,
                              conf.entry[access_id].service_type & ACCESS_MGMT_SERVICES_TYPE_TELNET ? 1 : 0);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            }
        }
        (void)cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}

static cyg_int32 handler_stat_access_mgmt_statistics(CYG_HTTPD_STATE *p)
{
    int                 ct;
    access_mgmt_stats_t stats;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_STAT_TYPE, VTSS_MODULE_ID_SECURITY)) {
        return -1;
    }
#endif

    (void)cyg_httpd_start_chunked("html");

    if ((cyg_httpd_form_varable_find(p, "clear") != NULL)) { /* Clear? */
        access_mgmt_stats_clear();
    }

    access_mgmt_stats_get(&stats);

    //Format: [interface]/[interface_receive_cnt]/[interface_allow_cnt]/[interface_discard_cnt]|...
    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s/%u/%u/%u",
                  "HTTP",   stats.http_receive_cnt,      stats.http_receive_cnt      - stats.http_discard_cnt,   stats.http_discard_cnt);
    (void)cyg_httpd_write_chunked(p->outbuffer, ct);
#if defined(VTSS_SW_OPTION_HTTPS)
    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "|%s/%u/%u/%u",
                  "HTTPS",  stats.https_receive_cnt,     stats.https_receive_cnt     - stats.https_discard_cnt,  stats.https_discard_cnt);
    (void)cyg_httpd_write_chunked(p->outbuffer, ct);
#endif /* VTSS_SW_OPTION_HTTPS */
#if defined(VTSS_SW_OPTION_SNMP)
    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "|%s/%u/%u/%u",
                  "SNMP",   stats.snmp_receive_cnt,      stats.snmp_receive_cnt      - stats.snmp_discard_cnt,   stats.snmp_discard_cnt);
    (void)cyg_httpd_write_chunked(p->outbuffer, ct);
#endif /* VTSS_SW_OPTION_SNMP */
#if defined(VTSS_SW_OPTION_CLI_TELNET)
    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "|%s/%u/%u/%u",
                  "TELNET", stats.telnet_receive_cnt,    stats.telnet_receive_cnt    - stats.telnet_discard_cnt, stats.telnet_discard_cnt);
    (void)cyg_httpd_write_chunked(p->outbuffer, ct);
#endif /* VTSS_SW_OPTION_CLI_TELNET */
#if defined(VTSS_SW_OPTION_SSH)
    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "|%s/%u/%u/%u",
                  "SSH",    stats.ssh_receive_cnt,       stats.ssh_receive_cnt       - stats.ssh_discard_cnt,    stats.ssh_discard_cnt);
    (void)cyg_httpd_write_chunked(p->outbuffer, ct);
#endif /* VTSS_SW_OPTION_SSH */
    (void)cyg_httpd_end_chunked();

    return -1; // Do not further search the file system.
}

/****************************************************************************/
/*  lib_config                                                              */
/****************************************************************************/

static size_t access_mgmt_lib_config_js(char **base_ptr, char **cur_ptr, size_t *length)
{
    char buff[ACCESS_MGMT_WEB_BUF_LEN];
    (void) snprintf(buff, ACCESS_MGMT_WEB_BUF_LEN,
                    "var configAccessMgmtMax = %d;\n", ACCESS_MGMT_MAX_ENTRIES);
    return webCommonBufferHandler(base_ptr, cur_ptr, length, buff);
}

web_lib_config_js_tab_entry(access_mgmt_lib_config_js);

/****************************************************************************/
/*  HTTPD Table Entries                                                     */
/****************************************************************************/

CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_access_mgmt, "/config/access_mgmt", handler_config_access_mgmt);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_stat_access_mgmt_statistics, "/stat/access_mgmt_statistics", handler_stat_access_mgmt_statistics);

/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
