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
#include "port_api.h"
#include "mgmt_api.h"

#ifdef VTSS_SW_OPTION_PRIV_LVL
#include "vtss_privilege_api.h"
#include "vtss_privilege_web_api.h"
#endif /* VTSS_SW_OPTION_PRIV_LVL */

#include "ipmc_lib.h"


#define VTSS_TRACE_MODULE_ID    VTSS_MODULE_ID_IPMC_LIB
#define IPMC_LIB_WEB_BUF_LEN    512

/* =================
 * Trace definitions
 * -------------- */
#include <vtss_module_id.h>
#include <vtss_trace_lvl_api.h>
#include <vtss_trace_api.h>
/* ============== */

/****************************************************************************/
/*  Web Handler  functions                                                  */
/****************************************************************************/
static cyg_int32 handler_config_ipmclib_entry(CYG_HTTPD_STATE *p)
{
    vtss_isid_t                 sid = web_retrieve_request_sid(p);  /* Includes USID = ISID */
    size_t                      var_len;
    int                         cntr, str_len;
    char                        str_name[VTSS_IPMC_NAME_MAX_LEN];
    const char                  *var_string;

    vtss_ipv4_t                 adrs4;
    ipmc_lib_grp_fltr_entry_t   fltr_entry;

    /* Redirect unmanaged/invalid access to handler */
    if (!p || redirectUnmanagedOrInvalid(p, sid)) {
        return -1;
    }

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_IPMC_LIB)) {
        return -1;
    }
#endif /* VTSS_SW_OPTION_PRIV_LVL */

    if (p->method == CYG_HTTPD_METHOD_POST) {
        int                     errors = 0;
        BOOL                    entryFound;
        char                    search_str[65];
        ipmc_ip_version_t       version;
        vtss_ipv6_t             adrs6;
        ipmc_operation_action_t op;

        /* Process Existed Entries First */
        for (cntr = 0; cntr < IPMC_LIB_FLTR_ENTRY_MAX_CNT; cntr++) {
            str_len = 0;
            memset(search_str, 0x0, sizeof(search_str));
            sprintf(search_str, "idx_ipmcpf_entry_%d", cntr);
            var_string = cyg_httpd_form_varable_string(p, search_str, &var_len);
            memset(str_name, 0x0, sizeof(str_name));
            if (var_len > 0) {
                (void) cgi_unescape(var_string, str_name, var_len, sizeof(str_name));
            } else {
                break;
            }

            if ((str_len = strlen(str_name)) == 0) {
                continue;
            }

            memset(&fltr_entry, 0x0, sizeof(ipmc_lib_grp_fltr_entry_t));
            strncpy(fltr_entry.name, str_name, str_len);
            entryFound = (ipmc_lib_mgmt_fltr_entry_get(&fltr_entry, TRUE) == VTSS_OK);

            op = IPMC_OP_SET;
            memset(search_str, 0x0, sizeof(search_str));
            sprintf(search_str, "delete_ipmcpf_entry_%d", cntr);
            if (cyg_httpd_form_varable_find(p, search_str)) {
                /* "delete" checked */
                if (entryFound) {
                    op = IPMC_OP_DEL;
                } else {
                    continue;
                }
            } else {
                if (entryFound) {
                    op = IPMC_OP_UPD;
                } else {
                    op = IPMC_OP_ADD;
                }
            }

            if (op != IPMC_OP_DEL) {
                memset(search_str, 0x0, sizeof(search_str));
                sprintf(search_str, "bgn_ipmcpf_entry_%d", cntr);
                if (!cyg_httpd_form_varable_ipv6(p, search_str, &adrs6)) {
                    if (!cyg_httpd_form_varable_ipv4(p, search_str, &adrs4)) {
                        continue;
                    } else {
                        adrs4 = htonl(adrs4);
                        IPMC_LIB_ADRS_4TO6_SET(adrs4, fltr_entry.grp_bgn);
                    }
                } else {
                    IPMC_LIB_ADRS_CPY(&fltr_entry.grp_bgn, &adrs6);
                }

                memset(search_str, 0x0, sizeof(search_str));
                sprintf(search_str, "end_ipmcpf_entry_%d", cntr);
                if (!cyg_httpd_form_varable_ipv6(p, search_str, &adrs6)) {
                    if (!cyg_httpd_form_varable_ipv4(p, search_str, &adrs4)) {
                        continue;
                    } else {
                        adrs4 = htonl(adrs4);
                        IPMC_LIB_ADRS_4TO6_SET(adrs4, fltr_entry.grp_end);
                    }
                } else {
                    IPMC_LIB_ADRS_CPY(&fltr_entry.grp_end, &adrs6);
                }
            }

            if (ipmc_lib_mgmt_fltr_entry_set(op, &fltr_entry) != VTSS_OK) {
                T_W("Failure in setting IPMC Profile Entry %s", fltr_entry.name);
            }
        }

        /* Process Created Entries Next */
        for (cntr = 1; cntr <= IPMC_LIB_FLTR_ENTRY_MAX_CNT; cntr++) {
            str_len = 0;
            memset(search_str, 0x0, sizeof(search_str));
            sprintf(search_str, "new_idx_ipmcpf_entry_%d", cntr);
            var_string = cyg_httpd_form_varable_string(p, search_str, &var_len);
            memset(str_name, 0x0, sizeof(str_name));
            if (var_len > 0) {
                (void) cgi_unescape(var_string, str_name, var_len, sizeof(str_name));
            } else {
                break;
            }

            if ((str_len = strlen(str_name)) == 0) {
                continue;
            }

            memset(&fltr_entry, 0x0, sizeof(ipmc_lib_grp_fltr_entry_t));
            strncpy(fltr_entry.name, str_name, str_len);
            entryFound = (ipmc_lib_mgmt_fltr_entry_get(&fltr_entry, TRUE) == VTSS_OK);

            op = IPMC_OP_SET;
            if (entryFound) {
                op = IPMC_OP_UPD;
            } else {
                op = IPMC_OP_ADD;
            }

            version = IPMC_IP_VERSION_INIT;
            memset(search_str, 0x0, sizeof(search_str));
            sprintf(search_str, "new_bgn_ipmcpf_entry_%d", cntr);
            if (!cyg_httpd_form_varable_ipv6(p, search_str, &adrs6)) {
                if (!cyg_httpd_form_varable_ipv4(p, search_str, &adrs4)) {
                    continue;
                } else {
                    adrs4 = htonl(adrs4);
                    IPMC_LIB_ADRS_4TO6_SET(adrs4, fltr_entry.grp_bgn);
                    version = IPMC_IP_VERSION_IGMP;
                }
            } else {
                IPMC_LIB_ADRS_CPY(&fltr_entry.grp_bgn, &adrs6);
                version = IPMC_IP_VERSION_MLD;
            }

            memset(search_str, 0x0, sizeof(search_str));
            sprintf(search_str, "new_end_ipmcpf_entry_%d", cntr);
            if (!cyg_httpd_form_varable_ipv6(p, search_str, &adrs6)) {
                if (!cyg_httpd_form_varable_ipv4(p, search_str, &adrs4)) {
                    continue;
                } else {
                    if (version != IPMC_IP_VERSION_IGMP) {
                        continue;
                    }

                    adrs4 = htonl(adrs4);
                    IPMC_LIB_ADRS_4TO6_SET(adrs4, fltr_entry.grp_end);
                }
            } else {
                if (version != IPMC_IP_VERSION_MLD) {
                    continue;
                }

                IPMC_LIB_ADRS_CPY(&fltr_entry.grp_end, &adrs6);
            }

            fltr_entry.version = version;
            if (ipmc_lib_mgmt_fltr_entry_set(op, &fltr_entry) != VTSS_OK) {
                T_W("Failure in setting IPMC Profile Entry %s", fltr_entry.name);
            }
        }

        cntr = 0;
        if ((var_string = cyg_httpd_form_varable_find(p, "NumberOfEntries")) != NULL) {
            cntr = atoi(var_string);
        }
        memset(search_str, 0x0, sizeof(search_str));
        sprintf(search_str, "/ipmc_lib_entry_table.htm?&DynDisplayNum=%d&DynChannelGrp=", cntr);
        redirect(p, errors ? STACK_ERR_URL : search_str);
    } else {
        int         ct;
        vtss_ipv4_t boundary;
        int         display_cnt;
        char        bgn_buf[IPV6_ADDR_IBUF_MAX_LEN], end_buf[IPV6_ADDR_IBUF_MAX_LEN];
        char        encoded_string[3 * VTSS_IPMC_NAME_MAX_LEN];

        cntr = display_cnt = 0;
        if ((var_string = cyg_httpd_form_varable_find(p, "DynDisplayNum")) != NULL) {
            display_cnt = atoi(var_string);
        }
        var_string = cyg_httpd_form_varable_string(p, "DynChannelGrp", &var_len);
        memset(str_name, 0x0, sizeof(str_name));
        if (var_len > 0) {
            (void) cgi_unescape(var_string, str_name, var_len, sizeof(str_name));
        }
        str_len = strlen(str_name);

        /* CYG_HTTPD_METHOD_GET (+HEAD) */
        /*
            Format:
            [channel_name]/[start_addr]/[end_addr]|...|
        */
        (void)cyg_httpd_start_chunked("html");

        memset(&fltr_entry, 0x0, sizeof(ipmc_lib_grp_fltr_entry_t));
        if (str_len && (str_len < VTSS_IPMC_NAME_MAX_LEN)) {
            strncpy(fltr_entry.name, str_name, str_len);
        }
        while (ipmc_lib_mgmt_fltr_entry_get_next(&fltr_entry, TRUE) == VTSS_OK) {
            memset(encoded_string, 0x0, sizeof(encoded_string));
            ct = cgi_escape(fltr_entry.name, encoded_string);

            memset(bgn_buf, 0x0, sizeof(bgn_buf));
            memset(end_buf, 0x0, sizeof(end_buf));
            if (fltr_entry.version == IPMC_IP_VERSION_IGMP) {
                IPMC_LIB_ADRS_6TO4_SET(fltr_entry.grp_bgn, adrs4);
                IPMC_LIB_ADRS_6TO4_SET(fltr_entry.grp_end, boundary);
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s%s/%s/%s",
                              cntr ? "|" : "",
                              encoded_string,
                              misc_ipv4_txt(htonl(adrs4), bgn_buf),
                              misc_ipv4_txt(htonl(boundary), end_buf));
            } else {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s%s/%s/%s",
                              cntr ? "|" : "",
                              encoded_string,
                              misc_ipv6_txt(&fltr_entry.grp_bgn, bgn_buf),
                              misc_ipv6_txt(&fltr_entry.grp_end, end_buf));
            }
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);

            cntr++;
            if (display_cnt && (cntr >= display_cnt)) {
                break;
            }
        }

        if (!cntr) {
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "NoEntries|");
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        } else {
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "|");
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        }

        (void)cyg_httpd_end_chunked();
    }

    return -1;  // Do not further search the file system.
}

static cyg_int32 handler_config_ipmclib_profile(CYG_HTTPD_STATE *p)
{
    vtss_isid_t                 sid = web_retrieve_request_sid(p);  /* Includes USID = ISID */
    int                         ct, cntr, str_len;
    const char                  *var_string;

    BOOL                        state;
    ipmc_lib_profile_mem_t      *pfm;
    ipmc_lib_grp_fltr_profile_t *fltr_profile;
    ipmc_lib_profile_t          *data;

    /* Redirect unmanaged/invalid access to handler */
    if (!p || redirectUnmanagedOrInvalid(p, sid) ||
        !IPMC_MEM_PROFILE_MTAKE(pfm)) {
        return -1;
    }

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_IPMC_LIB)) {
        IPMC_MEM_PROFILE_MGIVE(pfm);
        return -1;
    }
#endif /* VTSS_SW_OPTION_PRIV_LVL */

    fltr_profile = &pfm->profile;
    if (p->method == CYG_HTTPD_METHOD_POST) {
        int                     errors = 0;
        size_t                  var_len;
        char                    str_name[VTSS_IPMC_NAME_MAX_LEN];
        BOOL                    entryFound;
        ipmc_operation_action_t op;
        char                    search_str[65];

        if (ipmc_lib_mgmt_profile_state_get(&state) == VTSS_OK) {
            if (cyg_httpd_form_varable_int(p, "profile_mode", &ct)) {
                state = ct ? TRUE : FALSE;
                if (ipmc_lib_mgmt_profile_state_set(state) != VTSS_OK) {
                    T_W("Set IPMC Profile Global Mode fail");
                }
            }
        }

        /* Process Existed Entries First */
        for (cntr = 0; cntr < IPMC_LIB_FLTR_PROFILE_MAX_CNT; cntr++) {
            str_len = 0;
            memset(search_str, 0x0, sizeof(search_str));
            sprintf(search_str, "idx_ipmcpf_profile_%d", cntr);
            var_string = cyg_httpd_form_varable_string(p, search_str, &var_len);
            memset(str_name, 0x0, sizeof(str_name));
            if (var_len > 0) {
                (void) cgi_unescape(var_string, str_name, var_len, sizeof(str_name));
            } else {
                break;
            }

            if ((str_len = strlen(str_name)) == 0) {
                continue;
            }

            memset(fltr_profile, 0x0, sizeof(ipmc_lib_grp_fltr_profile_t));
            data = &fltr_profile->data;
            strncpy(data->name, str_name, str_len);
            entryFound = (ipmc_lib_mgmt_fltr_profile_get(fltr_profile, TRUE) == VTSS_OK);

            op = IPMC_OP_SET;
            memset(search_str, 0x0, sizeof(search_str));
            sprintf(search_str, "delete_ipmcpf_profile_%d", cntr);
            if (cyg_httpd_form_varable_find(p, search_str)) {
                /* "delete" checked */
                if (entryFound) {
                    op = IPMC_OP_DEL;
                } else {
                    continue;
                }
            } else {
                if (entryFound) {
                    op = IPMC_OP_UPD;
                } else {
                    op = IPMC_OP_ADD;
                }
            }

            if (op != IPMC_OP_DEL) {
                memset(search_str, 0x0, sizeof(search_str));
                sprintf(search_str, "desc_ipmcpf_profile_%d", cntr);
                var_string = cyg_httpd_form_varable_string(p, search_str, &var_len);
                memset(data->desc, 0x0, sizeof(data->desc));
                if (var_len > 0) {
                    (void) cgi_unescape(var_string, data->desc, var_len, sizeof(data->desc));
                }
            }

            if (ipmc_lib_mgmt_fltr_profile_set(op, fltr_profile) != VTSS_OK) {
                T_W("Failure in setting IPMC Profile Entry %s", data->name);
            }
        }

        /* Process Created Entries Next */
        for (cntr = 1; cntr <= IPMC_LIB_FLTR_PROFILE_MAX_CNT; cntr++) {
            str_len = 0;
            memset(search_str, 0x0, sizeof(search_str));
            sprintf(search_str, "new_idx_ipmcpf_profile_%d", cntr);
            var_string = cyg_httpd_form_varable_string(p, search_str, &var_len);
            memset(str_name, 0x0, sizeof(str_name));
            if (var_len > 0) {
                (void) cgi_unescape(var_string, str_name, var_len, sizeof(str_name));
            } else {
                break;
            }

            if ((str_len = strlen(str_name)) == 0) {
                continue;
            }

            memset(fltr_profile, 0x0, sizeof(ipmc_lib_grp_fltr_profile_t));
            data = &fltr_profile->data;
            strncpy(data->name, str_name, str_len);
            entryFound = (ipmc_lib_mgmt_fltr_profile_get(fltr_profile, TRUE) == VTSS_OK);

            op = IPMC_OP_SET;
            if (entryFound) {
                op = IPMC_OP_UPD;
            } else {
                op = IPMC_OP_ADD;
            }

            memset(search_str, 0x0, sizeof(search_str));
            sprintf(search_str, "new_desc_ipmcpf_profile_%d", cntr);
            var_string = cyg_httpd_form_varable_string(p, search_str, &var_len);
            memset(data->desc, 0x0, sizeof(data->desc));
            if (var_len > 0) {
                (void) cgi_unescape(var_string, data->desc, var_len, sizeof(data->desc));
            }

            if (ipmc_lib_mgmt_fltr_profile_set(op, fltr_profile) != VTSS_OK) {
                T_W("Failure in setting IPMC Profile Entry %s", data->name);
            }
        }

        redirect(p, errors ? STACK_ERR_URL : "/ipmc_lib_profile_table.htm");
    } else {
        char    encoded_string_name[3 * VTSS_IPMC_NAME_MAX_LEN], encoded_string_desc[3 * VTSS_IPMC_DESC_MAX_LEN];
        /* CYG_HTTPD_METHOD_GET (+HEAD) */
        /*
            Format:
            [state];[profile_name]/[description]|...;
        */
        (void)cyg_httpd_start_chunked("html");

        if (ipmc_lib_mgmt_profile_state_get(&state) != VTSS_OK) {
            T_W("Failure in GET profile state");
            state = FALSE;
        }
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u;", state);
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);

        cntr = 0;
        memset(fltr_profile, 0x0, sizeof(ipmc_lib_grp_fltr_profile_t));
        while (ipmc_lib_mgmt_fltr_profile_get_next(fltr_profile, TRUE) == VTSS_OK) {
            data = &fltr_profile->data;

            memset(encoded_string_name, 0x0, sizeof(encoded_string_name));
            ct = cgi_escape(data->name, encoded_string_name);
            memset(encoded_string_desc, 0x0, sizeof(encoded_string_desc));
            ct = cgi_escape(data->desc, encoded_string_desc);

            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s%s/%s",
                          cntr ? "|" : "",
                          encoded_string_name,
                          encoded_string_desc);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);

            cntr++;
        }

        if (!cntr) {
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "NoEntries;");
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        } else {
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), ";");
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        }

        (void)cyg_httpd_end_chunked();
    }

    IPMC_MEM_PROFILE_MGIVE(pfm);
    return -1;  // Do not further search the file system.
}

static cyg_int32 handler_config_ipmclib_rule(CYG_HTTPD_STATE *p)
{
    vtss_isid_t                 sid = web_retrieve_request_sid(p);  /* Includes USID = ISID */
    BOOL                        entryFound;
    size_t                      var_len;
    int                         ct, cntr, str_len;
    char                        profile_name[VTSS_IPMC_NAME_MAX_LEN];
    char                        encoded_string[(4 * VTSS_IPMC_NAME_MAX_LEN) + 1];
    const char                  *var_string;

    vtss_ipv4_t                 adrs4, bdry4;
    ipmc_lib_profile_mem_t      *pfm;
    ipmc_lib_grp_fltr_profile_t *fltr_profile;
    ipmc_lib_profile_t          *data;
    ipmc_lib_rule_t             fltr_rule;
    ipmc_lib_grp_fltr_entry_t   fltr_entry;

    /* Redirect unmanaged/invalid access to handler */
    if (!p || redirectUnmanagedOrInvalid(p, sid) ||
        !IPMC_MEM_PROFILE_MTAKE(pfm)) {
        return -1;
    }

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_IPMC_LIB)) {
        IPMC_MEM_PROFILE_MGIVE(pfm);
        return -1;
    }
#endif /* VTSS_SW_OPTION_PRIV_LVL */

    fltr_profile = &pfm->profile;
    if (p->method == CYG_HTTPD_METHOD_POST) {
        u32     pdx;
        int     errors = 0;
        char    search_str[(3 * VTSS_IPMC_NAME_MAX_LEN) + VTSS_IPMC_DESC_MAX_LEN];
        char    str_name[VTSS_IPMC_NAME_MAX_LEN];

        var_string = cyg_httpd_form_varable_string(p, "pdx", &var_len);
        memset(profile_name, 0x0, sizeof(profile_name));
        if (var_len > 0) {
            (void) cgi_unescape(var_string, profile_name, var_len, sizeof(profile_name));
        }
        str_len = strlen(profile_name);

        memset(fltr_profile, 0x0, sizeof(ipmc_lib_grp_fltr_profile_t));
        data = &fltr_profile->data;
        if (str_len && (str_len < VTSS_IPMC_NAME_MAX_LEN)) {
            strncpy(data->name, profile_name, str_len);
        }

        if (ipmc_lib_mgmt_fltr_profile_get(fltr_profile, TRUE) == VTSS_OK) {
            memset(search_str, 0x0, sizeof(search_str));
            memcpy(search_str, data->desc, sizeof(data->desc));
            if (ipmc_lib_mgmt_fltr_profile_set(IPMC_OP_DEL, fltr_profile) != VTSS_OK) {
                T_W("Failure in deleting IPMC Profile Entry %s", data->name);
                memset(search_str, 0x0, sizeof(search_str));
                sprintf(search_str, "Failure in deleting IPMC Profile Entry %s", data->name);
                send_custom_error(p, "IPMC Profile Rule Configuration Error", search_str, strlen(search_str));

                IPMC_MEM_PROFILE_MGIVE(pfm);
                return -1;  // Do not further search the file system.
            } else {
                memset(fltr_profile, 0x0, sizeof(ipmc_lib_grp_fltr_profile_t));
                strncpy(data->name, profile_name, str_len);
                memcpy(data->desc, search_str, sizeof(data->desc));
                if (ipmc_lib_mgmt_fltr_profile_set(IPMC_OP_ADD, fltr_profile) != VTSS_OK) {
                    T_W("Failure in adding IPMC Profile Entry %s", data->name);
                    sprintf(search_str, "Failure in adding IPMC Profile Entry %s", data->name);
                    send_custom_error(p, "IPMC Profile Rule Configuration Error", search_str, strlen(search_str));

                    IPMC_MEM_PROFILE_MGIVE(pfm);
                    return -1;  // Do not further search the file system.
                } else {
                    if (ipmc_lib_mgmt_fltr_profile_get(fltr_profile, TRUE) != VTSS_OK) {
                        T_W("Failure in getting IPMC Profile Entry %s", data->name);
                        sprintf(search_str, "Failure in getting IPMC Profile Entry %s", data->name);
                        send_custom_error(p, "IPMC Profile Rule Configuration Error", search_str, strlen(search_str));

                        IPMC_MEM_PROFILE_MGIVE(pfm);
                        return -1;  // Do not further search the file system.
                    }
                }
            }

            pdx = data->index;
            for (cntr = 1; cntr <= IPMC_LIB_FLTR_ENTRY_MAX_CNT; cntr++) {
                str_len = 0;
                memset(search_str, 0x0, sizeof(search_str));
                sprintf(search_str, "edx_ipmcpf_rule_%d", cntr);
                var_string = cyg_httpd_form_varable_string(p, search_str, &var_len);
                memset(str_name, 0x0, sizeof(str_name));
                if (var_len > 0) {
                    (void) cgi_unescape(var_string, str_name, var_len, sizeof(str_name));
                } else {
                    break;
                }

                if ((str_len = strlen(str_name)) == 0) {
                    continue;
                }

                memset(&fltr_entry, 0x0, sizeof(ipmc_lib_grp_fltr_entry_t));
                strncpy(fltr_entry.name, str_name, str_len);
                entryFound = (ipmc_lib_mgmt_fltr_entry_get(&fltr_entry, TRUE) == VTSS_OK);
                if (!entryFound) {
                    continue;
                }

                memset(&fltr_rule, 0x0, sizeof(ipmc_lib_rule_t));
                fltr_rule.idx = IPMC_LIB_FLTR_RULE_IDX_INIT;
                fltr_rule.entry_index = fltr_entry.index;
                fltr_rule.next_rule_idx = IPMC_LIB_FLTR_RULE_IDX_INIT;

                memset(search_str, 0x0, sizeof(search_str));
                sprintf(search_str, "action_ipmcpf_rule_%d", cntr);
                if (cyg_httpd_form_varable_int(p, search_str, &ct)) {
                    fltr_rule.action = ct;
                }
                memset(search_str, 0x0, sizeof(search_str));
                sprintf(search_str, "log_ipmcpf_rule_%d", cntr);
                if (cyg_httpd_form_varable_int(p, search_str, &ct)) {
                    fltr_rule.log = ct ? TRUE : FALSE;
                }

                if (ipmc_lib_mgmt_fltr_profile_rule_set(IPMC_OP_ADD, pdx, &fltr_rule) != VTSS_OK) {
                    T_W("Failure in setting IPMC Profile Rule %s", fltr_entry.name);
                }
            }
        }

        memset(search_str, 0x0, sizeof(search_str));
        memset(encoded_string, 0x0, sizeof(encoded_string));
        ct = cgi_text_str_to_ascii_str(profile_name, encoded_string, strlen(profile_name), sizeof(encoded_string));
        if (ct < 0) {
            errors = -1;
        } else {
            sprintf(search_str, "/ipmc_lib_rule_table.htm?CurSidV=%d&DoPdxOp=1&DynBgnPdx=%s",
                    sid, encoded_string);
        }
        redirect(p, errors ? STACK_ERR_URL : search_str);
    } else {
        char    bgn_buf[IPV6_ADDR_IBUF_MAX_LEN], end_buf[IPV6_ADDR_IBUF_MAX_LEN];

        var_string = cyg_httpd_form_varable_string(p, "DynBgnPdx", &var_len);
        memset(profile_name, 0x0, sizeof(profile_name));
        if (var_len > 0) {
            (void) cgi_ascii_str_to_text_str(var_string, profile_name, var_len, sizeof(profile_name));
        }
        str_len = strlen(profile_name);

        /* CYG_HTTPD_METHOD_GET (+HEAD) */
        /*
            Format:
            [profile_name];[entry_1]/[bgn]/[end]|...|[entry_n]/[bgn]/[end];[entry_name]/[action]/[log]|...;
        */
        (void)cyg_httpd_start_chunked("html");

        memset(fltr_profile, 0x0, sizeof(ipmc_lib_grp_fltr_profile_t));
        data = &fltr_profile->data;
        if (str_len && (str_len < VTSS_IPMC_NAME_MAX_LEN)) {
            strncpy(data->name, profile_name, str_len);
        }

        if (ipmc_lib_mgmt_fltr_profile_get(fltr_profile, TRUE) == VTSS_OK) {
            memset(encoded_string, 0x0, sizeof(encoded_string));
            ct = cgi_escape(data->name, encoded_string);
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s", encoded_string);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);

            cntr = 0;
            memset(&fltr_entry, 0x0, sizeof(ipmc_lib_grp_fltr_entry_t));
            while (ipmc_lib_mgmt_fltr_entry_get_next(&fltr_entry, TRUE) == VTSS_OK) {
                memset(encoded_string, 0x0, sizeof(encoded_string));
                ct = cgi_escape(fltr_entry.name, encoded_string);

                memset(bgn_buf, 0x0, sizeof(bgn_buf));
                memset(end_buf, 0x0, sizeof(end_buf));
                if (fltr_entry.version == IPMC_IP_VERSION_IGMP) {
                    IPMC_LIB_ADRS_6TO4_SET(fltr_entry.grp_bgn, adrs4);
                    IPMC_LIB_ADRS_6TO4_SET(fltr_entry.grp_end, bdry4);
                    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s%s/%s/%s",
                                  cntr ? "|" : ";",
                                  encoded_string,
                                  misc_ipv4_txt(htonl(adrs4), bgn_buf),
                                  misc_ipv4_txt(htonl(bdry4), end_buf));
                } else {
                    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s%s/%s/%s",
                                  cntr ? "|" : ";",
                                  encoded_string,
                                  misc_ipv6_txt(&fltr_entry.grp_bgn, bgn_buf),
                                  misc_ipv6_txt(&fltr_entry.grp_end, end_buf));
                }
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                cntr++;
            }
            if (!cntr) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), ";NoEntries");
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            }

            cntr = 0;
            if (ipmc_lib_mgmt_fltr_profile_rule_get_first(data->index, &fltr_rule) == VTSS_OK) {
                do {
                    fltr_entry.index = fltr_rule.entry_index;
                    if (ipmc_lib_mgmt_fltr_entry_get(&fltr_entry, FALSE) == VTSS_OK) {
                        memset(encoded_string, 0x0, sizeof(encoded_string));
                        ct = cgi_escape(fltr_entry.name, encoded_string);

                        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s%s/%d/%u",
                                      cntr ? "|" : ";",
                                      encoded_string,
                                      fltr_rule.action,
                                      fltr_rule.log);
                        (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                        cntr++;
                    }
                } while (ipmc_lib_mgmt_fltr_profile_rule_get_next(data->index, &fltr_rule) == VTSS_OK);
            }
            if (!cntr) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), ";NoEntries;");
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            } else {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), ";");
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            }
        } else {
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "INVALID;NoEntries;NoEntries;");
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        }

        (void)cyg_httpd_end_chunked();
    }

    IPMC_MEM_PROFILE_MGIVE(pfm);
    return -1;  // Do not further search the file system.
}

/****************************************************************************/
/*  Module JS lib config routine                                            */
/****************************************************************************/

static size_t ipmclib_lib_config_js(char **base_ptr, char **cur_ptr, size_t *length)
{
    char    buff[IPMC_LIB_WEB_BUF_LEN];
    (void) snprintf(buff, IPMC_LIB_WEB_BUF_LEN,
                    "var cfgIpmcLibPfeMax = %d;\n"
                    "var cfgIpmcLibPftMax = %d;\n"
                    "var cfgIpmcLibPfrMax = %d;\n"
                    "var cfgIpmcLibChrAscMin = %d;\n"
                    "var cfgIpmcLibChrAscMax = %d;\n"
                    "var cfgIpmcLibChrAscSpc = %d;\n",
                    IPMC_LIB_FLTR_ENTRY_MAX_CNT,
                    IPMC_LIB_FLTR_PROFILE_MAX_CNT,
                    IPMC_LIB_FLTR_ENTRY_MAX_CNT,
                    IPMC_LIB_CHAR_ASCII_MIN,
                    IPMC_LIB_CHAR_ASCII_MAX,
                    IPMC_LIB_CHAR_ASCII_SPACE);

    return webCommonBufferHandler(base_ptr, cur_ptr, length, buff);
}

/****************************************************************************/
/*  JS lib config table entry                                               */
/****************************************************************************/

web_lib_config_js_tab_entry(ipmclib_lib_config_js);

/****************************************************************************/
/*  HTTPD Table Entries                                                     */
/****************************************************************************/

CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_ipmclib_entry, "/config/ipmclib_entry", handler_config_ipmclib_entry);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_ipmclib_profile, "/config/ipmclib_profile", handler_config_ipmclib_profile);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_ipmclib_rule, "/config/ipmclib_rule", handler_config_ipmclib_rule);

/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
