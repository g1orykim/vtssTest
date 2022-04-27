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
#include "vtss_snmp_api.h"
#ifdef VTSS_SW_OPTION_PRIV_LVL
#include "vtss_privilege_api.h"
#include "vtss_privilege_web_api.h"
#endif

#include "port_api.h"
#include "mgmt_api.h"
/* =================
 * Trace definitions
 * -------------- */
#include <vtss_module_id.h>
#include <vtss_trace_lvl_api.h>
#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_WEB
#include <vtss_trace_api.h>
/* ============== */

//#include "rmon_api.h"
//#include "rfc1213_mib2.h"

cyg_bool
cyg_httpd_form_varable_oid(CYG_HTTPD_STATE *p, const char *name, ulong *oidSubTree,  ulong *oid_len, uchar *oid_mask, ulong *oid_mask_len)
{
    size_t     len;
    const char *value;
    char       *value_char;
    int        num = 0, dot_flag = FALSE;
    u32        i, mask = 0x80, maskpos = 0;

    if ((value = cyg_httpd_form_varable_string(p, name, &len)) && len > 0) {
        value_char = (char *)value;
        *oid_len = *oid_mask_len = 0;

        //check if OID format .x.x.x
        for (i = 0; i < len; i++) {
            if (((value_char[i] != '.') && (value_char[i] != '*')) &&
                (value_char[i] < '0' || value_char[i] > '9')) {
                return FALSE;
            }
            if (value_char[i] == '*') {
                if (i == 0 || value_char[i - 1] != '.') {
                    return FALSE;
                }
            }
            if (value_char[i] == '.') {
                if (dot_flag) { //double dot
                    return FALSE;
                }
                dot_flag = TRUE;
                num++;
                if (num > 128) {
                    return FALSE;
                }
            } else {
                dot_flag = FALSE;
            }
        }
        *oid_mask_len = *oid_len = num;

        /* convert OID string (RFC1447)
           Each bit of this bit mask corresponds to the (8*i - 7)-th
           sub-identifier, and the least significant bit of the i-th
           octet of this octet string corresponding to the (8*i)-th
           sub-identifier, where i is in the range 1 through 16. */
        for (i = 0; i < *oid_len; i++) {
            if (!memcmp(value_char, ".*", 2)) {
                oidSubTree[i] = 0;
                oid_mask[maskpos] &= (~mask);
                value_char = value_char + 2;
            } else {
                oid_mask[maskpos] |= mask;
                (void) sscanf(value_char++, ".%d", &oidSubTree[i]);
            }

            if (i == *oid_len - 1) {
                break; //last OID node
            }
            while (*value_char != '.') {
                value_char++;
            }

            if (mask == 1) {
                mask = 0x80;
                maskpos++;
            } else {
                mask >>= 1;
            }
        }
        return TRUE;
    }

    return FALSE;
}

/****************************************************************************/
/*  Web Handler  functions                                                  */
/****************************************************************************/

static cyg_int32 handler_config_snmp(CYG_HTTPD_STATE *p)
{
    int         ct;
    snmp_conf_t snmp_conf, newconf;
    int         var_value;
    const char  *var_string;
    size_t      len = 64 * sizeof(char);
    uchar       str_buff[16];
    char        host_buf[INET6_ADDRSTRLEN];
    int         i;
#ifdef VTSS_SW_OPTION_IPV6
    int         ipv6_supported = 1;
    char        ipv6_str_buff[40];
#else
    int         ipv6_supported = 0;
#endif /* VTSS_SW_OPTION_IPV6 */
#ifdef SNMP_SUPPORT_V3
    snmpv3_users_conf_t user_conf;
    ulong       idx;
    ulong       pval;
#endif /* SNMP_SUPPORT_V3 */

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_SNMP)) {
        return -1;
    }
#endif

    if (p->method == CYG_HTTPD_METHOD_POST) {
        /* store form data */
        if (snmp_mgmt_snmp_conf_get(&snmp_conf) != VTSS_OK) {
            redirect(p, "/snmp.htm");;
            return -1;
        }
        newconf = snmp_conf;

        if (cyg_httpd_form_varable_int(p, "snmp_mode", &var_value)) {
            newconf.mode = var_value;
        }
        if (cyg_httpd_form_varable_int(p, "snmp_version", &var_value)) {
            newconf.version = var_value;
        }
        /* Remove Port and Trap Port parameters (always use port 161/162)
        if (cyg_httpd_form_varable_int(p, "snmp_port", &var_value)) {
            newconf.port = var_value;
        } */

        var_string = cyg_httpd_form_varable_string(p, "snmp_read_community", &len);
        if (len > 0) {
            (void) cgi_unescape(var_string, newconf.read_community, len, sizeof(newconf.read_community));
        } else {
            strcpy(newconf.read_community, "");
        }

        var_string = cyg_httpd_form_varable_string(p, "snmp_write_community", &len);
        if (len > 0) {
            (void) cgi_unescape(var_string, newconf.write_community, len, sizeof(newconf.write_community));
        } else {
            strcpy(newconf.write_community, "");
        }

#ifdef SNMP_SUPPORT_V3
        var_string = cyg_httpd_form_varable_string(p, "snmpv3_engineid", &len);
        if (len > 0) {
            for (idx = 0; idx < len; idx = idx + 2) {
                memcpy(str_buff, var_string + idx, 2);
                str_buff[2] = '\0';
                if (cyg_httpd_str_to_hex((const char *) str_buff, &pval) == FALSE) {
                    continue;
                }
                newconf.engineid[idx / 2] = (uchar)pval;
            }
            newconf.engineid_len = len / 2;
        }
#endif /* SNMP_SUPPORT_V3 */

        if (cyg_httpd_form_varable_int(p, "trap_mode", &var_value)) {
            newconf.trap_mode = var_value;
        }
        if (cyg_httpd_form_varable_int(p, "trap_version", &var_value)) {
            newconf.trap_version = var_value;
        }
        /* Remove Port and Trap Port parameters (always use port 161/162)
        if (cyg_httpd_form_varable_int(p, "trap_port", &var_value)) {
            newconf.trap_port = var_value;
        } */
        var_string = cyg_httpd_form_varable_string(p, "trap_community", &len);
        if (len > 0) {
            (void) cgi_unescape(var_string, newconf.trap_community, len, sizeof(newconf.trap_community));
        } else {
            strcpy(newconf.trap_community, "");
        }
        //cyg_httpd_form_varable_ipv4(p, "trap_dip", &newconf.trap_dip);
        var_string = cyg_httpd_form_varable_string(p, "trap_dip", &len);
        for (i = 0; i < INET6_ADDRSTRLEN; i++) {
            if (*var_string != '&') {
                host_buf[i] = *var_string;
                var_string ++;
            } else {
                host_buf[i] = '\0';
                break;
            }
        }
        strcpy(newconf.trap_dip_string, host_buf);

#ifdef VTSS_SW_OPTION_IPV6
        (void)cyg_httpd_form_varable_ipv6(p, "trap_dipv6", &newconf.trap_dipv6);
#endif /* VTSS_SW_OPTION_IPV6 */
        if (cyg_httpd_form_varable_int(p, "trap_authen_fail", &var_value)) {
            newconf.trap_authen_fail = var_value;
        }
        if (cyg_httpd_form_varable_int(p, "trap_linkup_linkdown", &var_value)) {
            newconf.trap_linkup_linkdown = var_value;
        }
        if (cyg_httpd_form_varable_int(p, "trap_inform_mode", &var_value)) {
            newconf.trap_inform_mode = var_value;
        }
        if (cyg_httpd_form_varable_int(p, "trap_inform_timeout", &var_value)) {
            newconf.trap_inform_timeout = var_value;
        }
        if (cyg_httpd_form_varable_int(p, "trap_inform_retries", &var_value)) {
            newconf.trap_inform_retries = var_value;
        }

#ifdef SNMP_SUPPORT_V3
        if (cyg_httpd_form_varable_int(p, "trap_probe_security_engineid", &var_value)) {
            newconf.trap_probe_security_engineid = var_value;
        }
        if (newconf.trap_probe_security_engineid == SNMP_MGMT_DISABLED) {
            var_string = cyg_httpd_form_varable_string(p, "trap_security_engineid", &len);
            if (len > 0) {
                for (idx = 0; idx < len; idx = idx + 2) {
                    memcpy(str_buff, var_string + idx, 2);
                    str_buff[2] = '\0';
                    if (cyg_httpd_str_to_hex((const char *) str_buff, &pval) == FALSE) {
                        continue;
                    }
                    newconf.trap_security_engineid[idx / 2] = (uchar)pval;
                }
                newconf.trap_security_engineid_len = len / 2;
            }
        }
        var_string = cyg_httpd_form_varable_string(p, "trap_security_name", &len);
        if (len > 0) {
            (void) cgi_unescape(var_string, newconf.trap_security_name, len, sizeof(newconf.trap_security_name));
        }
#endif /* SNMP_SUPPORT_V3 */

        if (memcmp(&newconf, &snmp_conf, sizeof(newconf)) != 0) {
            T_D("Calling snmp_mgmt_snmp_conf_set()");
            if (snmp_mgmt_snmp_conf_set(&newconf) < 0) {
                T_E("snmp_mgmt_snmp_conf_set(): failed");
            }
        }

#ifdef SNMP_SUPPORT_V3
        /* waiting for trap probe reply */
        if (snmp_conf.trap_version == SNMP_SUPPORT_V3 &&
            newconf.mode && strcmp(newconf.trap_dip_string, "") &&
            newconf.trap_mode && newconf.trap_mode != snmp_conf.trap_mode &&
            newconf.trap_probe_security_engineid) {
            VTSS_OS_MSLEEP(1000);
        }
#endif /* SNMP_SUPPORT_V3 */

        redirect(p, "/snmp.htm");
    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        char encoded_string[3 * SNMP_MGMT_MAX_COMMUNITY_LEN];

        (void)cyg_httpd_start_chunked("html");

        /* get form data
           Format: [ipv6_supported],[v3_supported],[trap_security_name1]|[trap_security_name2]|...,
                   [snmp_mode]/[snmp_version]/[snmp_read_community]/[snmp_write_community]/[snmpv3_engineid]|
                   [trap_mode]/[trap_version]/[trap_community]/[trap_dip]/[trap_dipv6]/[trap_authen_fail]/[trap_linkup_linkdown]/[trap_inform_mode]/[trap_inform_timeout]/[trap_inform_retries]/[trap_probe_security_engineid]/[trap_security_engineid]/[trap_security_name]
        */
        if (snmp_mgmt_snmp_conf_get(&snmp_conf) != VTSS_OK) {
            (void)cyg_httpd_end_chunked();
            return -1;
        }

#ifdef SNMP_SUPPORT_V3
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d,1,%s|", ipv6_supported, SNMPV3_NONAME);
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        strcpy(user_conf.user_name, SNMPV3_CONF_ACESS_GETFIRST);
        while (snmpv3_mgmt_users_conf_get(&user_conf, TRUE) == VTSS_OK) {
            if (user_conf.status != SNMP_MGMT_ROW_ACTIVE) {
                continue;
            }
            if (memcmp(snmp_conf.trap_security_engineid, user_conf.engineid, snmp_conf.trap_security_engineid_len > user_conf.engineid_len ? snmp_conf.trap_security_engineid_len : user_conf.engineid_len)) {
                continue;
            }
            if (cgi_escape(user_conf.user_name, encoded_string) == 0) {
                continue;
            }
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s|", encoded_string);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        }
#else
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d,0,", ipv6_supported);
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);
#endif /* SNMP_SUPPORT_V3 */

        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), ",%u/%u/",
                      snmp_conf.mode,
                      snmp_conf.version);
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);

        if (strlen(snmp_conf.read_community)) {
            ct = cgi_escape(snmp_conf.read_community, encoded_string);
            (void)cyg_httpd_write_chunked(encoded_string, ct);
        }
        (void)cyg_httpd_write_chunked("/", 1);

        if (strlen(snmp_conf.write_community)) {
            ct = cgi_escape(snmp_conf.write_community, encoded_string);
            (void)cyg_httpd_write_chunked(encoded_string, ct);
        }

#ifdef SNMP_SUPPORT_V3
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%s",
                      misc_engineid2str(snmp_conf.engineid, snmp_conf.engineid_len));
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);
#endif /* SNMP_SUPPORT_V3 */

        (void) cgi_escape(snmp_conf.trap_community, encoded_string);
        // p->outbuffer is 2KBytes, so plenty of room.

        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "|%u/%u/%s/%s/%s/%u/%u/%u/%u/%u",
                      snmp_conf.trap_mode,
                      snmp_conf.trap_version,
                      encoded_string,
                      snmp_conf.trap_dip_string,
#ifdef VTSS_SW_OPTION_IPV6
                      misc_ipv6_txt(&snmp_conf.trap_dipv6, ipv6_str_buff),
#else
                      "0:0:0:0:0:0:0:0",
#endif /* VTSS_SW_OPTION_IPV6 */
                      snmp_conf.trap_authen_fail,
                      snmp_conf.trap_linkup_linkdown,
                      snmp_conf.trap_inform_mode,
                      snmp_conf.trap_inform_timeout,
                      snmp_conf.trap_inform_retries);

        (void)cyg_httpd_write_chunked(p->outbuffer, ct);

#ifdef SNMP_SUPPORT_V3
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%d/%s",
                      snmp_conf.trap_probe_security_engineid,
                      misc_engineid2str(snmp_conf.trap_security_engineid, snmp_conf.trap_security_engineid_len));
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);

        user_conf.engineid_len = snmp_conf.trap_security_engineid_len;
        memcpy(user_conf.engineid, snmp_conf.trap_security_engineid, snmp_conf.trap_security_engineid_len);
        strcpy(user_conf.user_name, snmp_conf.trap_security_name);
        (void) cgi_escape(snmp_conf.trap_security_name, encoded_string);
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%s",
                      (snmpv3_mgmt_users_conf_get(&user_conf, FALSE) == VTSS_OK && user_conf.status == SNMP_MGMT_ROW_ACTIVE) ? encoded_string : SNMPV3_NONAME);
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);
#endif /* SNMP_SUPPORT_V3 */

        (void)cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}

static cyg_int32 handler_config_trap(CYG_HTTPD_STATE *p)
{
    int                 ct;
    char                ch;
    BOOL         change_flag = FALSE;
    vtss_trap_entry_t   entry;
    vtss_trap_conf_t    *conf = &entry.trap_conf;
    BOOL                global_mode;
    int                 int_tmp;
    int                 count = 0;
    u32                 max_digest_length = (((3 * TRAP_MAX_NAME_LEN ) / 3 + (((3 * TRAP_MAX_NAME_LEN ) % 3) ? 1 : 0)) * 4);
    char                digest[max_digest_length + 1];
    char                encoded_string[3 * max_digest_length + 1], search_str[3 * max_digest_length + 7 + 1];

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_SNMP)) {
        return -1;
    }
#endif

    if (p->method == CYG_HTTPD_METHOD_POST) {

        memset(&entry, 0, sizeof(entry));
        while (trap_mgmt_conf_get_next(&entry) == VTSS_OK) {
            // delete the entries if the user checked

            digest[0] = '\0';
            (void) cyg_httpd_base64_encode(digest, entry.trap_conf_name, strlen(entry.trap_conf_name));

            encoded_string[0] = '\0';
            (void) cgi_escape(digest, encoded_string);

            T_D("encode_string = %s", encoded_string);
            sprintf(search_str, "delete_%s", encoded_string);

            if (cyg_httpd_form_varable_find(p, search_str)) { /* "delete" if checked */
                // set configuration
                change_flag = TRUE;
                entry.valid = FALSE;
                (void) trap_mgmt_conf_set(&entry);
            }

            if (change_flag) {
                // save configuration
            }
        }

        sprintf(search_str, "trap_mode");
        (void) cyg_httpd_form_varable_int(p, search_str, &int_tmp);
        (void) trap_mgmt_mode_set((BOOL)int_tmp);

        redirect(p, "/trap.htm");

    } else {

        //Format: <global_mode>,<max_entries_num>,<name>/<enable>/<version>/<dip>/<dport>|...,<remained_entries>

        (void) trap_mgmt_mode_get(&global_mode);

        (void)cyg_httpd_start_chunked("html");

        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d,%d", global_mode, VTSS_TRAP_CONF_MAX);
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);

        memset(&entry, 0, sizeof(entry));
        while (trap_mgmt_conf_get_next(&entry) == VTSS_OK) {
            if (count++ == 0) {
                ch = ',';
            } else {
                ch = '|';
            }

            encoded_string[0] = '\0';
            (void) cgi_escape(entry.trap_conf_name, encoded_string);

#ifdef VTSS_SW_OPTION_IPV6
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%c%s/%d/%u/%s/%u", ch, encoded_string, conf->enable, conf->trap_version,
                          (!conf->dip.ipv6_flag) ? conf->dip.addr.ipv4_str : misc_ipv6_txt(&conf->dip.addr.ipv6, search_str),
                          conf->trap_port);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
#else
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%c%s/%d/%u/%s/%u", ch, encoded_string, conf->enable, conf->trap_version,
                          conf->dip.addr.ipv4_str,
                          conf->trap_port);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
#endif /* VTSS_SW_OPTION_IPV6 */

        }
        if ( 0 == count) {
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), ",");
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        }

        (void)cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}

static void check_intfEventAll( vtss_trap_event_t    *event, int *linkup_all, int *linkdown_all, int *lldp_all)
{
    switch_iter_t   sit;
    port_iter_t     pit;
    int trap_linkup_tmp = -1, trap_linkdown_tmp = -1, trap_lldp_tmp = -1;
    int linkup_all_tmp = 1, linkdown_all_tmp = 1, lldp_all_tmp = 1;

    (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_USID);
    while (switch_iter_getnext(&sit)) {
        (void) port_iter_init(&pit, NULL, sit.isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
        while (port_iter_getnext(&pit)) {
            if (trap_linkup_tmp == -1) {
                trap_linkup_tmp = event->interface.trap_linkup[sit.isid][pit.iport];
                trap_linkdown_tmp = event->interface.trap_linkdown[sit.isid][pit.iport];
                trap_lldp_tmp = event->interface.trap_lldp[sit.isid][pit.iport];
                continue;
            }
            if ( linkup_all_tmp != 0 && trap_linkup_tmp != (int) event->interface.trap_linkup[sit.isid][pit.iport] ) {
                linkup_all_tmp = 0;
            }
            if ( linkdown_all_tmp != 0 && trap_linkdown_tmp != (int) event->interface.trap_linkdown[sit.isid][pit.iport] ) {
                linkdown_all_tmp = 0;
            }
            if ( lldp_all_tmp != 0 && trap_lldp_tmp != (int ) event->interface.trap_lldp[sit.isid][pit.iport] ) {
                lldp_all_tmp = 0;
            }
            if (linkup_all_tmp == 0 && linkdown_all_tmp == 0 && lldp_all_tmp == 0 ) {
                goto loop_end;
            }
        }
    }
loop_end:
    *linkup_all = !linkup_all_tmp ? 0 : !trap_linkup_tmp ? 1 : 2;
    *linkdown_all = !linkdown_all_tmp ? 0 : !trap_linkdown_tmp ? 1 : 2;
    *lldp_all = !lldp_all_tmp ? 0 : !trap_lldp_tmp ? 1 : 2;
}

static cyg_int32 handler_config_trap_detailed(CYG_HTTPD_STATE *p)
{
    int linkup_all, linkdown_all, lldp_all;
    int         ct;
    vtss_trap_entry_t trap_entry, trap_entry_tmp;
    int         var_value;
    const char  *var_string;
    size_t      len = 64 * sizeof(char);
    char        str_buff[40];
    char        host_buf[INET6_ADDRSTRLEN];
    int         i;
    char        trap_conf_name[TRAP_MAX_NAME_LEN + 1];
    vtss_trap_conf_t    *conf = &trap_entry.trap_conf;
    vtss_trap_event_t    *event = &trap_entry.trap_event;
#ifdef VTSS_SW_OPTION_IPV6
    int         ipv6_supported = 1;
#else
    int         ipv6_supported = 0;
#endif /* VTSS_SW_OPTION_IPV6 */
#ifdef SNMP_SUPPORT_V3
    snmpv3_users_conf_t user_conf;
    ulong       idx;
    ulong       pval;
#endif /* SNMP_SUPPORT_V3 */
    vtss_isid_t             sid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    port_iter_t       pit;
    switch_iter_t     sit;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_SNMP)) {
        return -1;
    }
#endif

    var_string = cyg_httpd_form_varable_string(p, "conf_name", &len);

    (void) cyg_httpd_base64_decode(trap_conf_name, (char *)var_string, len);

    if (p->method == CYG_HTTPD_METHOD_POST) {

        /*
           trap_conf_name, trap_mode, trap_version, trap_community, trap_dip, trap_dport, trap_inform_mode, trap_inform_timeout,
                trap_inform_retries, trap_probe_security_engineid, trap_security_engineid, trap_security_name,
                linkup_radio, linkdown_radio, lldp_radio, linkup_<port>, linkdwon_<port>, lldp_<port>
        */

        var_string = cyg_httpd_form_varable_string(p, "trap_conf_name", &len);
        if (len > 0) {
            if (cgi_unescape(var_string, trap_entry_tmp.trap_conf_name, len, sizeof(trap_entry_tmp.trap_conf_name)) == FALSE) {
                redirect(p, "/trap.htm");
                return -1;
            }
        }

        if ( VTSS_RC_OK != trap_mgmt_conf_get(&trap_entry_tmp)) {
            trap_mgmt_conf_default_get(&trap_entry_tmp);
        }

        memcpy(&trap_entry, &trap_entry_tmp, sizeof(trap_entry_tmp));

        if (cyg_httpd_form_varable_int(p, "trap_mode", &var_value)) {
            conf->enable = var_value;
        }
        if (cyg_httpd_form_varable_int(p, "trap_version", &var_value)) {
            conf->trap_version = var_value;
        }
        /* Remove Port and Trap Port parameters (always use port 161/162)
        if (cyg_httpd_form_varable_int(p, "trap_port", &var_value)) {
            newconf->trap_port = var_value;
        } */
        var_string = cyg_httpd_form_varable_string(p, "trap_community", &len);
        if (len > 0) {
            (void) cgi_unescape(var_string, conf->trap_community, len, sizeof(conf->trap_community));
        } else {
            strcpy(conf->trap_community, "");
        }
        var_string = cyg_httpd_form_varable_string(p, "trap_dip", &len);
        for (i = 0; i < INET6_ADDRSTRLEN; i++) {
            if (*var_string != '&') {
                host_buf[i] = *var_string;
                var_string ++;
            } else {
                host_buf[i] = '\0';
                break;
            }
        }
#ifdef VTSS_SW_OPTION_IPV6
        if ( VTSS_OK == mgmt_txt2ipv6(host_buf, &conf->dip.addr.ipv6)) {
            conf->dip.ipv6_flag = TRUE;
        } else {
            conf->dip.ipv6_flag = FALSE;
            strcpy(conf->dip.addr.ipv4_str, host_buf);
        }
#else
        conf->dip.ipv6_flag = FALSE;
        strcpy(conf->dip.addr.ipv4_str, host_buf);
#endif /* VTSS_SW_OPTION_IPV6 */

        if (cyg_httpd_form_varable_int(p, "trap_dport", &var_value)) {
            conf->trap_port = var_value;
        }

        if (cyg_httpd_form_varable_int(p, "trap_inform_mode", &var_value)) {
            conf->trap_inform_mode = var_value;
        }
        if (cyg_httpd_form_varable_int(p, "trap_inform_timeout", &var_value)) {
            conf->trap_inform_timeout = var_value;
        }
        if (cyg_httpd_form_varable_int(p, "trap_inform_retries", &var_value)) {
            conf->trap_inform_retries = var_value;
        }



#ifdef SNMP_SUPPORT_V3
        if (cyg_httpd_form_varable_int(p, "trap_probe_security_engineid", &var_value)) {
            conf->trap_probe_engineid = var_value;
        }
        if (conf->trap_probe_engineid == SNMP_MGMT_DISABLED) {
            var_string = cyg_httpd_form_varable_string(p, "trap_security_engineid", &len);
            if (len > 0) {
                for (idx = 0; idx < len; idx = idx + 2) {
                    memcpy(str_buff, var_string + idx, 2);
                    str_buff[2] = '\0';
                    if (cyg_httpd_str_to_hex((const char *) str_buff, &pval) == FALSE) {
                        continue;
                    }
                    conf->trap_engineid[idx / 2] = (uchar)pval;
                }
                conf->trap_engineid_len = len / 2;
            }
        }
        var_string = cyg_httpd_form_varable_string(p, "trap_security_name", &len);
        if (len > 0) {
            (void) cgi_unescape(var_string, conf->trap_security_name, len, sizeof(conf->trap_security_name));
        }
#endif /* SNMP_SUPPORT_V3 */

        /*
                linkup_radio, linkdown_radio, lldp_radio, warm_start, cold_start, authentication_fail, stp, rmon
        */

        (void) cyg_httpd_form_varable_int(p, "linkup_radio", &linkup_all);
        (void) (cyg_httpd_form_varable_int(p, "linkdown_radio", &linkdown_all));
        (void) (cyg_httpd_form_varable_int(p, "lldp_radio", &lldp_all));

        event->system.warm_start = (cyg_httpd_form_varable_string(p, "warm_start", &len) ? 1 : 0);
        event->system.cold_start = (cyg_httpd_form_varable_string(p, "cold_start", &len) ? 1 : 0);
        event->aaa.trap_authen_fail = (cyg_httpd_form_varable_string(p, "authentication_fail", &len) ? 1 : 0);
        event->sw.stp = (cyg_httpd_form_varable_string(p, "stp", &len) ? 1 : 0);
        event->sw.rmon = (cyg_httpd_form_varable_string(p, "rmon", &len) ? 1 : 0);

        if ( 0 == linkup_all || 0 == linkdown_all || 0 == lldp_all) {
            if (0 == linkup_all) {
                memset(event->interface.trap_linkup[sid], 0, sizeof(event->interface.trap_linkup[sid]));
            }
            if (0 == linkup_all) {
                memset(event->interface.trap_linkdown[sid], 0, sizeof(event->interface.trap_linkdown[sid]));
            }
            if (0 == linkup_all) {
                memset(event->interface.trap_lldp[sid], 0, sizeof(event->interface.trap_lldp[sid]));
            }

            (void) port_iter_init(&pit, NULL, sid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
            while (port_iter_getnext(&pit)) {
                if (linkup_all == 0 ) {
                    sprintf(str_buff, "linkup_%u", pit.uport);
                    event->interface.trap_linkup[sid][pit.iport] = (cyg_httpd_form_varable_string(p, str_buff, &len) ? 1 : 0);
                }
                if (linkdown_all == 0 ) {
                    sprintf(str_buff, "linkdown_%u", pit.uport);
                    event->interface.trap_linkdown[sid][pit.iport] = (cyg_httpd_form_varable_string(p, str_buff, &len) ? 1 : 0);
                }
                if (lldp_all == 0) {
                    sprintf(str_buff, "lldp_%u", pit.uport);
                    event->interface.trap_lldp[sid][pit.iport] = (cyg_httpd_form_varable_string(p, str_buff, &len) ? 1 : 0);
                }

            }
        }

        (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_USID);
        while (switch_iter_getnext(&sit)) {
            if (linkup_all != 0) {
                memset(event->interface.trap_linkup[sit.isid], linkup_all == 1 ? 0 : 1, sizeof(event->interface.trap_linkup[sit.isid]));
            }
            if (linkdown_all != 0) {
                memset(event->interface.trap_linkdown[sit.isid], linkdown_all == 1 ? 0 : 1, sizeof(event->interface.trap_linkdown[sit.isid]));
            }
            if (lldp_all != 0) {
                memset(event->interface.trap_lldp[sit.isid], lldp_all == 1 ? 0 : 1, sizeof(event->interface.trap_lldp[sit.isid]));
            }
        }


        trap_entry.valid = TRUE;
        if (trap_mgmt_conf_set(&trap_entry) < 0) {
            T_D("snmp_mgmt_snmp_conf_set(): failed");
        }

        redirect(p, "/trap.htm");
    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        char encoded_string[3 * SNMP_MGMT_MAX_COMMUNITY_LEN + 1];

        (void)cyg_httpd_start_chunked("html");

        /* get form data
           Format: [ipv6_supported],[v3_supported],[trap_conf_name1]|[trap_conf_name2]|...,
                   [trap_mode]/[trap_version]/[trap_community]/[trap_dip]/[trap_inform_mode]/[trap_inform_timeout]/[trap_inform_retries]/[trap_probe_security_engineid]/[trap_security_engineid]/[trap_security_name],
                   [trap_security_name1]|,
                   [link_up]/[link_down]/[lldp]
        */

#ifdef SNMP_SUPPORT_V3
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d,1,", ipv6_supported);
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);
#else
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d,0,", ipv6_supported);
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);
#endif /* SNMP_SUPPORT_V3 */

        memset(&trap_entry, 0, sizeof(trap_entry));
        i = 0;
        while (VTSS_RC_OK == trap_mgmt_conf_get_next(&trap_entry)) {
            encoded_string[0] = '\0';
            (void) cgi_escape(trap_entry.trap_conf_name, encoded_string);
            if ( i++ == 0 ) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s", encoded_string);
            } else {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "|%s", encoded_string);
            }
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        }

        strcpy(trap_entry.trap_conf_name, trap_conf_name);
        if (trap_entry.trap_conf_name[0] == 0 ) {
            trap_mgmt_conf_default_get(&trap_entry);
        } else if (trap_mgmt_conf_get(&trap_entry) != VTSS_OK) {
            (void)cyg_httpd_end_chunked();
            return -1;
        }

        encoded_string[0] = '\0';
        (void) cgi_escape(trap_entry.trap_conf_name, encoded_string);
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), ",%s/", encoded_string);
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);

        encoded_string[0] = '\0';
        (void) cgi_escape(conf->trap_community, encoded_string);

#ifdef VTSS_SW_OPTION_IPV6
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d/%u/%s/%s/%d/%u/%u/%u",
                      conf->enable,
                      conf->trap_version,
                      encoded_string,
                      ( FALSE == conf->dip.ipv6_flag) ? conf->dip.addr.ipv4_str : misc_ipv6_txt(&conf->dip.addr.ipv6, str_buff),
                      conf->trap_port,
                      conf->trap_inform_mode,
                      conf->trap_inform_timeout,
                      conf->trap_inform_retries);
#else
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d/%u/%s/%s/%d/%u/%u/%u",
                      conf->enable,
                      conf->trap_version,
                      encoded_string,
                      conf->dip.addr.ipv4_str,
                      conf->trap_port,
                      conf->trap_inform_mode,
                      conf->trap_inform_timeout,
                      conf->trap_inform_retries);

#endif /* VTSS_SW_OPTION_IPV6 */

        (void)cyg_httpd_write_chunked(p->outbuffer, ct);

#ifdef SNMP_SUPPORT_V3
        /*remove for testing */
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%d/%s",
                      conf->trap_probe_engineid, misc_engineid2str(conf->trap_engineid, conf->trap_engineid_len));
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);


        if ( 0 == conf->trap_engineid_len) {
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%s", SNMPV3_NONAME);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        } else {
            user_conf.engineid_len = conf->trap_engineid_len;
            memcpy(user_conf.engineid, conf->trap_engineid, conf->trap_engineid_len);
            strcpy(user_conf.user_name, conf->trap_security_name);
            (void) cgi_escape(conf->trap_security_name, encoded_string);
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%s",
                          (snmpv3_mgmt_users_conf_get(&user_conf, FALSE) == VTSS_OK && user_conf.status == SNMP_MGMT_ROW_ACTIVE) ? encoded_string : SNMPV3_NONAME);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        }

        strcpy(user_conf.user_name, SNMPV3_CONF_ACESS_GETFIRST);

        i = 0;

        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%s", SNMPV3_NONAME);
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        while (snmpv3_mgmt_users_conf_get(&user_conf, TRUE) == VTSS_OK) {
            if (user_conf.status != SNMP_MGMT_ROW_ACTIVE) {
                continue;
            }
            if (memcmp(conf->trap_engineid, user_conf.engineid, conf->trap_engineid_len > user_conf.engineid_len ? conf->trap_engineid_len : user_conf.engineid_len)) {
                continue;
            }
            if (cgi_escape(user_conf.user_name, encoded_string) == 0) {
                continue;
            }
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "|%s", encoded_string);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        }
        if ( 0 == i) {
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/");
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);

        }

#endif /* SNMP_SUPPORT_V3 */

        check_intfEventAll(event, &linkup_all, &linkdown_all, &lldp_all);
        /* [warm_start]/[cold_start]/[auth_fail]/[stp]/[rmon]/[link_up_all]/[link_down_all]/[lldp_all],
                    [port1]/[link_up]/[link_down]/[lldp]|[port2]/[link_up]/[link_down]/[lldp]|...
                        [link_up_all]:
                                        0: spec
                                        1: none
                                        2: all

                 */
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), ",%d/%d/%d/%d/%d/%d/%d/%d",
                      event->system.warm_start, event->system.cold_start, event->aaa.trap_authen_fail, event->sw.stp, event->sw.rmon,
                      linkup_all, linkdown_all, lldp_all);
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        i = 0;
        (void) port_iter_init(&pit, NULL, sid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
        while (port_iter_getnext(&pit)) {
            if (i++ == 0) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), ",%u/%d/%d/%d",
                              pit.uport, event->interface.trap_linkup[sid][pit.iport], event->interface.trap_linkdown[sid][pit.iport],
                              event->interface.trap_lldp[sid][pit.iport]);
            } else {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "|%u/%d/%d/%d",
                              pit.uport, event->interface.trap_linkup[sid][pit.iport], event->interface.trap_linkdown[sid][pit.iport],
                              event->interface.trap_lldp[sid][pit.iport]);

            }
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);

        }

        (void)cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}

#ifdef SNMP_SUPPORT_V3
static cyg_int32 handler_config_snmpv3_communities(CYG_HTTPD_STATE *p)
{
    int                       ct;
    snmpv3_communities_conf_t conf, newconf;
    ulong                     idx = 0, change_flag, del_flag;
    const char                *var_string;
    size_t                    len = 64 * sizeof(char);
    char                      buf[64], ip_buf[16], ip_mask_buf[16];
    vtss_rc                   rc;
    char                      encoded_string[3 * 64];

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_SNMP)) {
        return -1;
    }
#endif

    if (p->method == CYG_HTTPD_METHOD_POST) {
        /* store form data */
        strcpy(conf.community, SNMPV3_CONF_ACESS_GETFIRST);
        while (snmpv3_mgmt_communities_conf_get(&conf, TRUE) == VTSS_OK) {
            if (conf.status != SNMP_MGMT_ROW_ACTIVE) {
                continue;
            }
            newconf = conf;
            change_flag = del_flag = 0;

            /* delete entry */
            sprintf(buf, "del_%s", conf.community);
            if (cgi_escape(buf, encoded_string) == 0) {
                continue;
            }
            (void)cyg_httpd_form_varable_string(p, encoded_string, &len);
            if (len > 0) {
                newconf.valid = 0;
                change_flag = del_flag = 1;
            } else {
                //sip
                sprintf(buf, "sip_%s", conf.community);
                (void)cyg_httpd_form_varable_ipv4(p, buf, &newconf.sip);
                if (newconf.sip != conf.sip) {
                    change_flag = 1;
                }

                //sip_mask
                sprintf(buf, "sip_mask_%s", conf.community);
                (void)cyg_httpd_form_varable_ipv4(p, buf, &newconf.sip_mask);
                if (newconf.sip_mask != conf.sip_mask) {
                    change_flag = 1;
                }
            }

            if (change_flag) {
                T_D("Calling snmpv3_communities_users_conf_set(%s)", newconf.community);
                if (snmpv3_mgmt_communities_conf_set(&newconf) < 0) {
                    T_E("snmpv3_mgmt_communities_conf_set(%s): failed", newconf.community);
                }
                if (del_flag) {
                    strcpy(conf.community, SNMPV3_CONF_ACESS_GETFIRST);
                }
            }
        }

        /* new entry */
        idx = 1;
        sprintf(buf, "new_community_%d", idx);
        while ((var_string = cyg_httpd_form_varable_string(p, buf, &len)) && len > 0) {
            memset(&newconf, 0x0, sizeof(newconf));
            //community
            if (cgi_unescape(var_string, newconf.community, len, sizeof(newconf.community)) == FALSE) {
                continue;
            }

            //sip
            sprintf(buf, "new_sip_%d", idx);
            (void)cyg_httpd_form_varable_ipv4(p, buf, &newconf.sip);

            //sip_mask
            sprintf(buf, "new_sip_mask_%d", idx);
            (void)cyg_httpd_form_varable_ipv4(p, buf, &newconf.sip_mask);

            newconf.valid = 1;
            newconf.storage_type = SNMP_MGMT_STORAGE_PERMANENT;
            newconf.status = SNMP_MGMT_ROW_ACTIVE;
            T_D("Calling snmpv3_communities_users_conf_set(%s)", newconf.community);
            if ((rc = snmpv3_mgmt_communities_conf_set(&newconf)) < 0) {
                if (rc == SNMPV3_ERROR_COMMUNITIES_TABLE_FULL) {
                    char *err = "SNMPv3 communities table is full";
                    send_custom_error(p, err, err, strlen(err));
                    return -1; // Do not further search the file system.
                } else if (rc != VTSS_OK) {
                    T_E("snmpv3_mgmt_communities_conf_set(%s): failed", newconf.community);
                }
            }
            idx++;
            sprintf(buf, "new_community_%d", idx);
        }

        redirect(p, "/snmpv3_communities.htm");
    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        (void)cyg_httpd_start_chunked("html");

        /* get form data
           Format: <community_name>/<sip>/<sip_mask>,...
        */
        strcpy(conf.community, SNMPV3_CONF_ACESS_GETFIRST);
        while (snmpv3_mgmt_communities_conf_get(&conf, TRUE) == VTSS_OK) {
            if (conf.status != SNMP_MGMT_ROW_ACTIVE) {
                continue;
            }
            if (cgi_escape(conf.community, encoded_string) == 0) {
                continue;
            }
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s/%s/%s|",
                          encoded_string,
                          misc_ipv4_txt(conf.sip, ip_buf),
                          misc_ipv4_txt(conf.sip_mask, ip_mask_buf));
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        }

        (void)cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}

static cyg_int32 handler_config_snmpv3_users(CYG_HTTPD_STATE *p)
{
    int                 ct;
    int                 var_value;
    snmpv3_users_conf_t conf, newconf;
    ulong               idx = 0, idx2 = 0, change_flag, del_flag;
    const char          *var_string;
    size_t              len = 64 * sizeof(char);
    char                buf[128], buf1[4];
    vtss_rc             rc;
    char                encoded_string[3 * 64];
    ulong               pval;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_SNMP)) {
        return -1;
    }
#endif

    if (p->method == CYG_HTTPD_METHOD_POST) {
        /* store form data */
        strcpy(conf.user_name, SNMPV3_CONF_ACESS_GETFIRST);
        while (snmpv3_mgmt_users_conf_get(&conf, TRUE) == VTSS_OK) {
            if (conf.status != SNMP_MGMT_ROW_ACTIVE) {
                continue;
            }
            newconf = conf;
            change_flag = del_flag = 0;

            /* delete entry */
            sprintf(buf, "del_%s%s", misc_engineid2str(conf.engineid, conf.engineid_len), conf.user_name);
            if (cgi_escape(buf, encoded_string) == 0) {
                continue;
            }
            (void)cyg_httpd_form_varable_string(p, encoded_string, &len);
            if (len > 0) {
                newconf.valid = 0;
                change_flag = del_flag = 1;
            } else {
                //auth_password
                sprintf(buf, "auth_pd_%s%s", misc_engineid2str(conf.engineid, conf.engineid_len), conf.user_name);
                if (cgi_escape(buf, encoded_string) == 0) {
                    continue;
                }
                var_string = cyg_httpd_form_varable_string(p, encoded_string, &len);
                if (len > 0) {
                    if (cgi_unescape(var_string, newconf.auth_password, len, sizeof(newconf.auth_password)) == FALSE) {
                        continue;
                    }
                    if (strcmp(conf.auth_password, newconf.auth_password)) {
                        change_flag = 1;
                    }
                }
                //priv_password
                sprintf(buf, "priv_pd_%s%s", misc_engineid2str(conf.engineid, conf.engineid_len), conf.user_name);
                if (cgi_escape(buf, encoded_string) == 0) {
                    continue;
                }
                var_string = cyg_httpd_form_varable_string(p, encoded_string, &len);
                if (len > 0) {
                    if (cgi_unescape(var_string, newconf.priv_password, len, sizeof(newconf.priv_password)) == FALSE) {
                        continue;
                    }
                    if (strcmp(conf.priv_password, newconf.priv_password)) {
                        change_flag = 1;
                    }
                }
            }

            if (change_flag) {
                T_D("Calling snmpv3_mgmt_users_conf_set(%s)", newconf.user_name);
                if (snmpv3_mgmt_users_conf_set(&newconf) < 0) {
                    T_E("snmpv3_mgmt_users_conf_set(%s): failed", newconf.user_name);
                }
                if (del_flag) {
                    strcpy(conf.user_name, SNMPV3_CONF_ACESS_GETFIRST);
                }
            }
        }

        /* new entry */
        idx = 1;
        sprintf(buf, "new_engineid_%d", idx);
        while ((var_string = cyg_httpd_form_varable_string(p, buf, &len)) && len > 0) {
            memset(&newconf, 0x0, sizeof(newconf));
            //engine_id
            for (idx2 = 0; idx2 < len; idx2 = idx2 + 2) {
                memcpy(buf1, var_string + idx2, 2);
                buf1[2] = '\0';
                if (cyg_httpd_str_to_hex((const char *) buf1, &pval) == FALSE) {
                    continue;
                }
                newconf.engineid[idx2 / 2] = (uchar)pval;
            }
            newconf.engineid_len = len / 2;

            //user_name
            sprintf(buf, "new_user_%d", idx);
            var_string = cyg_httpd_form_varable_string(p, buf, &len);
            if (len > 0) {
                if (cgi_unescape(var_string, newconf.user_name, len, sizeof(newconf.user_name)) == FALSE) {
                    continue;
                }
            }

            //security_level
            sprintf(buf, "new_level_%d", idx);
            if (cyg_httpd_form_varable_int(p, buf, &var_value)) {
                newconf.security_level = var_value;
            }

            if (newconf.security_level != SNMP_MGMT_SEC_LEVEL_NOAUTH) {
                //auth_proto
                sprintf(buf, "new_auth_%d", idx);
                if (cyg_httpd_form_varable_int(p, buf, &var_value)) {
                    newconf.auth_protocol = var_value;
                }

                //auth_password
                sprintf(buf, "new_auth_pd_%d", idx);
                var_string = cyg_httpd_form_varable_string(p, buf, &len);
                if (len > 0) {
                    if (cgi_unescape(var_string, newconf.auth_password, len, sizeof(newconf.auth_password)) == FALSE) {
                        continue;
                    }
                }
            }

            if (newconf.security_level == SNMP_MGMT_SEC_LEVEL_AUTHPRIV) {
                //priv_proto
                sprintf(buf, "new_priv_%d", idx);
                if (cyg_httpd_form_varable_int(p, buf, &var_value)) {
                    newconf.priv_protocol = var_value;
                }

                //priv_password
                sprintf(buf, "new_priv_pd_%d", idx);
                var_string = cyg_httpd_form_varable_string(p, buf, &len);
                if (len > 0) {
                    if (cgi_unescape(var_string, newconf.priv_password, len, sizeof(newconf.priv_password)) == FALSE) {
                        continue;
                    }
                }
            }

            newconf.valid = 1;
            newconf.storage_type = SNMP_MGMT_STORAGE_PERMANENT;
            newconf.status = SNMP_MGMT_ROW_ACTIVE;
            T_D("Calling snmpv3_mgmt_users_conf_set(%s)", newconf.user_name);
            if ((rc = snmpv3_mgmt_users_conf_set(&newconf)) < 0) {
                if (rc == SNMPV3_ERROR_USERS_TABLE_FULL) {
                    char *err = "SNMPv3 users table is full";
                    send_custom_error(p, err, err, strlen(err));
                    return -1; // Do not further search the file system.
                } else if (rc != VTSS_OK) {
                    T_E("snmpv3_mgmt_users_conf_set(%s): failed", newconf.user_name);
                }
            }
            idx++;
            sprintf(buf, "new_engineid_%d", idx);
        }

        redirect(p, "/snmpv3_users.htm");
    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        (void)cyg_httpd_start_chunked("html");

        /* get form data
           Format: <engineid>/<user_name>/<group_name>/<level>/<auth_proto>/<auth_pd>/<privacy_proto>/<privacy_pd>,...
        */
        strcpy(conf.user_name, SNMPV3_CONF_ACESS_GETFIRST);
        while (snmpv3_mgmt_users_conf_get(&conf, TRUE) == VTSS_OK) {
            if (conf.status != SNMP_MGMT_ROW_ACTIVE) {
                continue;
            }
            if (cgi_escape(conf.user_name, encoded_string) == 0) {
                continue;
            }
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s/%s/%d/",
                          misc_engineid2str(conf.engineid, conf.engineid_len),
                          encoded_string,
                          conf.security_level);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);

            // auth_password, auth_protocol
            encoded_string[0] = '\0';
            if (conf.auth_password[0] != '\0' && cgi_escape(conf.auth_password, encoded_string) == 0) {
                continue;
            }
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d/%s/",
                          conf.auth_protocol,
                          encoded_string);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);

            // priv_password, priv_protocol
            encoded_string[0] = '\0';
            if (conf.priv_password[0] != '\0' && cgi_escape(conf.priv_password, encoded_string) == 0) {
                continue;
            }
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d/%s|",
                          conf.priv_protocol,
                          encoded_string);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        }

        (void)cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}

static cyg_int32 handler_config_snmpv3_groups(CYG_HTTPD_STATE *p)
{
    int                       ct;
    int                       var_value;
    snmp_conf_t               snmp_conf;
    snmpv3_communities_conf_t community_conf;
    snmpv3_users_conf_t       user_conf;
    snmpv3_groups_conf_t      conf, newconf;
    ulong                     idx = 0, change_flag, del_flag;
    const char                *var_string;
    size_t                    len = 64 * sizeof(char);
    char                      buf[64];
    vtss_rc                   rc;
    char                      encoded_string[3 * 64];

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_SNMP)) {
        return -1;
    }
#endif

    if (p->method == CYG_HTTPD_METHOD_POST) {
        /* store form data */
        strcpy(conf.security_name, SNMPV3_CONF_ACESS_GETFIRST);
        while (snmpv3_mgmt_groups_conf_get(&conf, TRUE) == VTSS_OK) {
            if (conf.status != SNMP_MGMT_ROW_ACTIVE) {
                continue;
            }
            newconf = conf;
            change_flag = del_flag = 0;

            /* delete entry */
            sprintf(buf, "del_%d%s", conf.security_model, conf.security_name);
            if (cgi_escape(buf, encoded_string) == 0) {
                continue;
            }
            (void)cyg_httpd_form_varable_string(p, encoded_string, &len);
            if (len > 0) {
                newconf.valid = 0;
                change_flag = del_flag = 1;
            } else {
                //group_name
                sprintf(buf, "group_%d%s", conf.security_model, conf.security_name);
                var_string = cyg_httpd_form_varable_string(p, buf, &len);
                if (len > 0) {
                    if (cgi_unescape(var_string, newconf.group_name, len, sizeof(newconf.group_name)) == FALSE) {
                        continue;
                    }
                    if (strcmp(newconf.group_name, conf.group_name)) {
                        change_flag = 1;
                    }
                }
            }

            if (change_flag) {
                T_D("Calling snmpv3_mgmt_groups_conf_set(%d, %s)", newconf.security_model, newconf.security_name);
                if (snmpv3_mgmt_groups_conf_set(&newconf) < 0) {
                    T_E("snmpv3_mgmt_groups_conf_set(%d, %s): failed", newconf.security_model, newconf.security_name);
                }
                if (del_flag) {
                    strcpy(conf.security_name, SNMPV3_CONF_ACESS_GETFIRST);
                }
            }
        }

        /* new entry */
        idx = 1;
        sprintf(buf, "new_group_%d", idx);
        while ((var_string = cyg_httpd_form_varable_string(p, buf, &len)) && len > 0) {
            memset(&newconf, 0x0, sizeof(newconf));
            //group_name
            if (cgi_unescape(var_string, newconf.group_name, len, sizeof(newconf.group_name)) == FALSE) {
                continue;
            }

            //security_model
            sprintf(buf, "new_model_%d", idx);
            if (cyg_httpd_form_varable_int(p, buf, &var_value)) {
                newconf.security_model = var_value;
            }

            //security_name
            sprintf(buf, "new_security_%d", idx);
            var_string = cyg_httpd_form_varable_string(p, buf, &len);
            if (len > 0) {
                if (cgi_unescape(var_string, newconf.security_name, len, sizeof(newconf.security_name)) == FALSE) {
                    continue;
                }
            }

            newconf.valid = 1;
            newconf.storage_type = SNMP_MGMT_STORAGE_PERMANENT;
            newconf.status = SNMP_MGMT_ROW_ACTIVE;
            T_D("Calling snmpv3_mgmt_groups_conf_set(%d, %s)", newconf.security_model, newconf.security_name);
            if ((rc = snmpv3_mgmt_groups_conf_set(&newconf)) < 0) {
                if (rc == SNMPV3_ERROR_GROUPS_TABLE_FULL) {
                    char *err = "SNMPv3 groups table is full";
                    send_custom_error(p, err, err, strlen(err));
                    return -1; // Do not further search the file system.
                } else if (rc != VTSS_OK) {
                    T_E("snmpv3_mgmt_groups_conf_set(%d, %s): failed", newconf.security_model, newconf.security_name);
                }
            }
            idx++;
            sprintf(buf, "new_group_%d", idx);
        }

        redirect(p, "/snmpv3_groups.htm");
    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        (void)cyg_httpd_start_chunked("html");

        /* get form data
           <community1>|<community2>|...,<user1>|<user2>|...,<model>/<security_name>/<group_name>,...
        */
        if (snmp_mgmt_snmp_conf_get(&snmp_conf) != VTSS_OK) {
            (void)cyg_httpd_end_chunked();
            return -1;
        }

        strcpy(community_conf.community, SNMPV3_CONF_ACESS_GETFIRST);
        while (snmpv3_mgmt_communities_conf_get(&community_conf, TRUE) == VTSS_OK) {
            if (community_conf.status != SNMP_MGMT_ROW_ACTIVE) {
                continue;
            }
            if (cgi_escape(community_conf.community, encoded_string) == 0) {
                continue;
            }
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s|", encoded_string);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        }
        (void)cyg_httpd_write_chunked(",", 1);
        strcpy(user_conf.user_name, SNMPV3_CONF_ACESS_GETFIRST);
        while (snmpv3_mgmt_users_conf_get(&user_conf, TRUE) == VTSS_OK) {
            if (user_conf.status != SNMP_MGMT_ROW_ACTIVE) {
                continue;
            }
            if (memcmp(snmp_conf.engineid, user_conf.engineid, snmp_conf.engineid_len > user_conf.engineid_len ? snmp_conf.engineid_len : user_conf.engineid_len)) {
                continue;
            }
            if (cgi_escape(user_conf.user_name, encoded_string) == 0) {
                continue;
            }
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s|", encoded_string);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        }
        (void)cyg_httpd_write_chunked(",", 1);
        strcpy(conf.security_name, SNMPV3_CONF_ACESS_GETFIRST);
        while (snmpv3_mgmt_groups_conf_get(&conf, TRUE) == VTSS_OK) {
            if (conf.status != SNMP_MGMT_ROW_ACTIVE) {
                continue;
            }
            if (cgi_escape(conf.security_name, encoded_string) == 0) {
                continue;
            }
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d/%s/",
                          conf.security_model,
                          encoded_string);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            if (cgi_escape(conf.group_name, encoded_string) == 0) {
                continue;
            }
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s|",
                          encoded_string);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        }

        (void)cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}

static cyg_int32 handler_config_snmpv3_views(CYG_HTTPD_STATE *p)
{
    int                 ct;
    int                 var_value;
    snmpv3_views_conf_t conf, newconf;
    ulong               idx = 0, change_flag, del_flag;
    const char          *var_string;
    size_t              len = 64 * sizeof(char);
    char                buf[64];
    vtss_rc             rc;
    char                encoded_string[3 * 64];

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_SNMP)) {
        return -1;
    }
#endif

    if (p->method == CYG_HTTPD_METHOD_POST) {
        /* store form data */
        strcpy(conf.view_name, SNMPV3_CONF_ACESS_GETFIRST);
        while (snmpv3_mgmt_views_conf_get(&conf, TRUE) == VTSS_OK) {
            if (conf.status != SNMP_MGMT_ROW_ACTIVE) {
                continue;
            }
            newconf = conf;
            change_flag = del_flag = 0;

            /* delete entry */
            sprintf(buf, "del_%s%s", conf.view_name, misc_oid2str(conf.subtree, conf.subtree_len, conf.subtree_mask, conf.subtree_mask_len));
            if (cgi_escape(buf, encoded_string) == 0) {
                continue;
            }
            (void)cyg_httpd_form_varable_string(p, encoded_string, &len);
            if (len > 0) {
                newconf.valid = 0;
                change_flag = del_flag = 1;
            } else {
                //view_type
                sprintf(buf, "type_%s%s", conf.view_name, misc_oid2str(conf.subtree, conf.subtree_len, conf.subtree_mask, conf.subtree_mask_len));
                if (cyg_httpd_form_varable_int(p, buf, &var_value)) {
                    newconf.view_type = var_value;
                    if (newconf.view_type != conf.view_type) {
                        change_flag = 1;
                    }
                }
            }

            if (change_flag) {
                T_D("Calling snmpv3_mgmt_views_conf_set(%s)", newconf.view_name);
                if (snmpv3_mgmt_views_conf_set(&newconf) < 0) {
                    T_E("snmpv3_mgmt_views_conf_set(%s): failed", newconf.view_name);
                }
                if (del_flag) {
                    strcpy(conf.view_name, SNMPV3_CONF_ACESS_GETFIRST);
                }
            }
        }

        /* new entry */
        idx = 1;
        sprintf(buf, "new_view_%d", idx);
        while ((var_string = cyg_httpd_form_varable_string(p, buf, &len)) && len > 0) {
            memset(&newconf, 0x0, sizeof(newconf));
            //view_name
            if (cgi_unescape(var_string, newconf.view_name, len, sizeof(newconf.view_name)) == FALSE) {
                continue;
            }

            //view_type
            sprintf(buf, "new_type_%d", idx);
            if (cyg_httpd_form_varable_int(p, buf, &var_value)) {
                newconf.view_type = var_value;
            }

            //subtree
            sprintf(buf, "new_subtree_%d", idx);
            if (cyg_httpd_form_varable_oid(p, buf, newconf.subtree, &newconf.subtree_len, newconf.subtree_mask, &newconf.subtree_mask_len) == FALSE) {
                continue;
            }

            newconf.valid = 1;
            newconf.storage_type = SNMP_MGMT_STORAGE_PERMANENT;
            newconf.status = SNMP_MGMT_ROW_ACTIVE;
            T_D("Calling snmpv3_mgmt_views_conf_set(%s)", newconf.view_name);
            if ((rc = snmpv3_mgmt_views_conf_set(&newconf)) < 0) {
                if (rc == SNMPV3_ERROR_VIEWS_TABLE_FULL) {
                    char *err = "SNMPv3 views table is full";
                    send_custom_error(p, err, err, strlen(err));
                    return -1; // Do not further search the file system.
                } else if (rc == SNMP_ERROR_ENGINE_FAIL) {
                    char err[512];
                    sprintf(err, "OID %s is not supported",
                            misc_oid2str(newconf.subtree, newconf.subtree_len, newconf.subtree_mask, newconf.subtree_len));
                    send_custom_error(p, err, err, strlen(err));
                    return -1; // Do not further search the file system.
                } else if (rc != VTSS_OK) {
                    T_E("snmpv3_mgmt_views_conf_set(%s): failed", newconf.view_name);
                }
            }
            idx++;
            sprintf(buf, "new_view_%d", idx);
        }

        redirect(p, "/snmpv3_views.htm");
    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        (void)cyg_httpd_start_chunked("html");

        /* get form data
           Format: <view_name>/<view_type>/<subtree>,...
        */
        strcpy(conf.view_name, SNMPV3_CONF_ACESS_GETFIRST);
        while (snmpv3_mgmt_views_conf_get(&conf, TRUE) == VTSS_OK) {
            if (conf.status != SNMP_MGMT_ROW_ACTIVE) {
                continue;
            }
            if (cgi_escape(conf.view_name, encoded_string) == 0) {
                continue;
            }
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s/%d/%s|",
                          encoded_string,
                          conf.view_type,
                          misc_oid2str(conf.subtree, conf.subtree_len, conf.subtree_mask, conf.subtree_mask_len));
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        }

        (void)cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}

static cyg_int32 handler_config_snmpv3_accesses(CYG_HTTPD_STATE *p)
{
    int                    ct;
    int                    var_value;
    snmpv3_groups_conf_t   group_conf;
    snmpv3_views_conf_t    view_conf;
    snmpv3_accesses_conf_t conf, newconf;
    ulong                  idx = 0, change_flag, del_flag;
    const char             *var_string;
    size_t                 len = 64 * sizeof(char);
    char                   buf[64];
    vtss_rc                rc;
    char                   encoded_string[3 * 64];

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_SNMP)) {
        return -1;
    }
#endif

    if (p->method == CYG_HTTPD_METHOD_POST) {
        /* store form data */
        strcpy(conf.group_name, SNMPV3_CONF_ACESS_GETFIRST);
        while (snmpv3_mgmt_accesses_conf_get(&conf, TRUE) == VTSS_OK) {
            if (conf.status != SNMP_MGMT_ROW_ACTIVE) {
                continue;
            }
            newconf = conf;
            change_flag = del_flag = 0;

            /* delete entry */
            sprintf(buf, "del_%s%d%d", conf.group_name, newconf.security_model, newconf.security_level);
            if (cgi_escape(buf, encoded_string) == 0) {
                continue;
            }
            (void)cyg_httpd_form_varable_string(p, encoded_string, &len);
            if (len > 0) {
                newconf.valid = 0;
                change_flag = del_flag = 1;
            } else {
                //read_view_name
                sprintf(buf, "read_%s%d%d", conf.group_name, newconf.security_model, newconf.security_level);
                var_string = cyg_httpd_form_varable_string(p, buf, &len);
                if (len > 0) {
                    if (cgi_unescape(var_string, newconf.read_view_name, len, sizeof(newconf.read_view_name)) == FALSE) {
                        continue;
                    }
                    if (strcmp(newconf.read_view_name, conf.read_view_name)) {
                        change_flag = 1;
                    }
                }

                //write_view_name
                sprintf(buf, "write_%s%d%d", conf.group_name, newconf.security_model, newconf.security_level);
                var_string = cyg_httpd_form_varable_string(p, buf, &len);
                if (len > 0) {
                    if (cgi_unescape(var_string, newconf.write_view_name, len, sizeof(newconf.write_view_name)) == FALSE) {
                        continue;
                    }
                    if (strcmp(newconf.write_view_name, conf.write_view_name)) {
                        change_flag = 1;
                    }
                }
            }

            if (change_flag) {
                T_D("Calling snmpv3_mgmt_accesses_conf_set(%s, %d, %d)", newconf.group_name, newconf.security_model, newconf.security_level);
                if (snmpv3_mgmt_accesses_conf_set(&newconf) < 0) {
                    T_E("snmpv3_mgmt_accesses_conf_set(%s, %d, %d): failed", newconf.group_name, newconf.security_model, newconf.security_level);
                }
                if (del_flag) {
                    strcpy(conf.group_name, SNMPV3_CONF_ACESS_GETFIRST);
                }
            }
        }

        /* new entry */
        idx = 1;
        sprintf(buf, "new_group_%d", idx);
        while ((var_string = cyg_httpd_form_varable_string(p, buf, &len)) && len > 0) {
            memset(&newconf, 0x0, sizeof(newconf));
            //user_name
            if (cgi_unescape(var_string, newconf.group_name, len, sizeof(newconf.group_name)) == FALSE) {
                continue;
            }

            //security_model
            sprintf(buf, "new_model_%d", idx);
            if (cyg_httpd_form_varable_int(p, buf, &var_value)) {
                newconf.security_model = var_value;
            }

            //security_level
            sprintf(buf, "new_level_%d", idx);
            if (cyg_httpd_form_varable_int(p, buf, &var_value)) {
                newconf.security_level = var_value;
            }

            //context_match
            newconf.context_match = SNMPV3_MGMT_CONTEX_MATCH_EXACT;

            //read_view_name
            sprintf(buf, "new_read_%d", idx);
            var_string = cyg_httpd_form_varable_string(p, buf, &len);
            if (len > 0) {
                if (cgi_unescape(var_string, newconf.read_view_name, len, sizeof(newconf.read_view_name)) == FALSE) {
                    continue;
                }
            }

            //write_view_name
            sprintf(buf, "new_write_%d", idx);
            var_string = cyg_httpd_form_varable_string(p, buf, &len);
            if (len > 0) {
                if (cgi_unescape(var_string, newconf.write_view_name, len, sizeof(newconf.write_view_name)) == FALSE) {
                    continue;
                }
            }

            //notify_view_name
            /* SNMP/eCos package don't support notify yet
            sprintf(buf, "new_notify_%d", idx);
            var_string = cyg_httpd_form_varable_string(p, buf, &len);
            if (len > 0) {
                if (cgi_unescape(var_string, newconf.notify_view_name, len, sizeof(newconf.notify_view_name)) == FALSE) {
                    continue;
                    }
            } */
            strcpy(newconf.notify_view_name, SNMPV3_NONAME);

            newconf.valid = 1;
            newconf.storage_type = SNMP_MGMT_STORAGE_PERMANENT;
            newconf.status = SNMP_MGMT_ROW_ACTIVE;
            T_D("Calling snmpv3_mgmt_accesses_conf_set(%s, %d, %d)", newconf.group_name, newconf.security_model, newconf.security_level);
            if ((rc = snmpv3_mgmt_accesses_conf_set(&newconf)) < 0) {
                if (rc == SNMPV3_ERROR_ACCESSES_TABLE_FULL) {
                    char *err = "SNMPv3 accesses table is full";
                    send_custom_error(p, err, err, strlen(err));
                    return -1; // Do not further search the file system.
                } else if (rc != VTSS_OK) {
                    T_E("snmpv3_mgmt_accesses_conf_set(%s, %d, %d): failed", newconf.group_name, newconf.security_model, newconf.security_level);
                }
            }
            idx++;
            sprintf(buf, "new_group_%d", idx);
        }

        redirect(p, "/snmpv3_accesses.htm");
    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        (void)cyg_httpd_start_chunked("html");

        /* get form data
           Format: <grouop_name1>|<grouop_name2>|...,<view_name1>|<view_name2>|...,<group_name>/<modle>/<level>/<read_view_name>/<write_view_name>/<notify_view_name>,...
        */
        strcpy(group_conf.security_name, SNMPV3_CONF_ACESS_GETFIRST);
        while (snmpv3_mgmt_groups_conf_get(&group_conf, TRUE) == VTSS_OK) {
            if (group_conf.status != SNMP_MGMT_ROW_ACTIVE) {
                continue;
            }
            if (cgi_escape(group_conf.group_name, encoded_string) == 0) {
                continue;
            }
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s|", encoded_string);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        }
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), ",None|");
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        strcpy(view_conf.view_name, SNMPV3_CONF_ACESS_GETFIRST);
        while (snmpv3_mgmt_views_conf_get(&view_conf, TRUE) == VTSS_OK) {
            if (view_conf.status != SNMP_MGMT_ROW_ACTIVE) {
                continue;
            }
            if (cgi_escape(view_conf.view_name, encoded_string) == 0) {
                continue;
            }
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s|", encoded_string);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        }
        (void)cyg_httpd_write_chunked(",", 1);
        strcpy(conf.group_name, SNMPV3_CONF_ACESS_GETFIRST);
        while (snmpv3_mgmt_accesses_conf_get(&conf, TRUE) == VTSS_OK) {
            if (conf.status != SNMP_MGMT_ROW_ACTIVE) {
                continue;
            }
            if (cgi_escape(conf.group_name, encoded_string) == 0) {
                continue;
            }
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s/%d/%d/",
                          encoded_string,
                          conf.security_model,
                          conf.security_level);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            if (cgi_escape(conf.read_view_name, encoded_string) == 0) {
                continue;
            }
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s/",
                          encoded_string);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            if (cgi_escape(conf.write_view_name, encoded_string) == 0) {
                continue;
            }
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s/",
                          encoded_string);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            if (cgi_escape(conf.notify_view_name, encoded_string) == 0) {
                continue;
            }
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s|",
                          encoded_string);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        }

        (void)cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}
#endif /* SNMP_SUPPORT_V3 */

#ifndef SNMP_SUPPORT_V3
/****************************************************************************/
/*  Module Filter CSS routine                                               */
/****************************************************************************/
static size_t snmp_lib_filter_css(char **base_ptr, char **cur_ptr, size_t *length)
{
    return webCommonBufferHandler(base_ptr, cur_ptr, length, ".SNMPV3_CTRL { display: none; }\r\n");
}

/****************************************************************************/
/*  Filter CSS table entry                                                  */
/****************************************************************************/
web_lib_filter_css_tab_entry(snmp_lib_filter_css);
#endif /* SNMP_SUPPORT_V3 */

/****************************************************************************/
/*  HTTPD Table Entries                                                     */
/****************************************************************************/

CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_snmp, "/config/snmp", handler_config_snmp);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_trap, "/config/trap", handler_config_trap);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_trap_detailed, "/config/trap_detailed", handler_config_trap_detailed);
#ifdef SNMP_SUPPORT_V3
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_snmpv3_communities, "/config/snmpv3_communities", handler_config_snmpv3_communities);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_snmpv3_users, "/config/snmpv3_users", handler_config_snmpv3_users);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_snmpv3_groups, "/config/snmpv3_groups", handler_config_snmpv3_groups);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_snmpv3_views, "/config/snmpv3_views", handler_config_snmpv3_views);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_snmpv3_accesses, "/config/snmpv3_accesses", handler_config_snmpv3_accesses);
#endif /* SNMP_SUPPORT_V3 */


/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/

