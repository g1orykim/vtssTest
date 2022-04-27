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
#include "port_api.h"
#include "mgmt_api.h"

#ifdef VTSS_SW_OPTION_PRIV_LVL
#include "vtss_privilege_api.h"
#include "vtss_privilege_web_api.h"
#endif /* VTSS_SW_OPTION_PRIV_LVL */

#include "mvr.h"
#include "mvr_api.h"


#define MVR_WEB_BUF_LEN 512

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

static cyg_int32 handler_config_mvr(CYG_HTTPD_STATE *p)
{
    vtss_isid_t                 sid = web_retrieve_request_sid(p);  /* Includes USID = ISID */
    vtss_rc                     rc;
    int                         ct, cntr, errors;
    port_iter_t                 pit;
    vtss_port_no_t              iport;

    BOOL                        mvr_mode;
    mvr_mgmt_interface_t        mvrif;
    mvr_conf_intf_entry_t       *intf;
    mvr_conf_port_role_t        *role;
    mvr_conf_fast_leave_t       fastleave;
    ipmc_lib_profile_mem_t      *pfm;
    ipmc_lib_grp_fltr_profile_t *fltr_profile;
    ipmc_lib_profile_t          *data;

    /* Redirect unmanaged/invalid access to handler */
    if (redirectUnmanagedOrInvalid(p, sid) ||
        !IPMC_MEM_PROFILE_MTAKE(pfm)) {
        return -1;
    }

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_MVR)) {
        IPMC_MEM_PROFILE_MGIVE(pfm);
        return -1;
    }
#endif /* VTSS_SW_OPTION_PRIV_LVL */

    errors = 0;
    fltr_profile = &pfm->profile;
    data = &fltr_profile->data;
    if (p->method == CYG_HTTPD_METHOD_POST) {
        int                     var_value;
        vtss_uport_no_t         uport;
        ipmc_operation_action_t op;
        BOOL                    entryFound, roleChanged;
        size_t                  mvr_name_len;
        char                    search_str[64];
        const char              *var_string;

        if (mvr_mgmt_get_mode(&mvr_mode) == VTSS_OK) {
            if (cyg_httpd_form_varable_int(p, "mvr_mode", &var_value)) {
                mvr_mode = var_value ? TRUE : FALSE;
                if (mvr_mgmt_set_mode(&mvr_mode) != VTSS_OK) {
                    T_D("SET MVR Mode fail");
                }
            }
        }

        /* Process Existed Entries First */
        for (cntr = 0; cntr < MVR_VLAN_MAX; cntr++) {
            memset(search_str, 0x0, sizeof(search_str));
            sprintf(search_str, "mvid_mvr_vlan_%d", cntr);
            if (!cyg_httpd_form_varable_int(p, search_str, &var_value) || !var_value) {
                continue;
            }

            entryFound = FALSE;
            memset(&mvrif, 0x0, sizeof(mvr_mgmt_interface_t));
            mvrif.vid = (u16)(var_value & 0xFFFF);
            if (mvr_mgmt_get_intf_entry(sid, &mvrif) == VTSS_OK) {
                entryFound = TRUE;
            }
            mvrif.vid = (u16)(var_value & 0xFFFF);
            intf = &mvrif.intf;
            role = &mvrif.role;

            op = IPMC_OP_SET;
            memset(search_str, 0x0, sizeof(search_str));
            sprintf(search_str, "delete_mvr_vlan_%u", mvrif.vid);
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

                    intf->querier4_address = MVR_CONF_DEF_INTF_ADRS4;
                    intf->mode = MVR_CONF_DEF_INTF_MODE;
                    intf->vtag = MVR_CONF_DEF_INTF_VTAG;
                    intf->priority = MVR_CONF_DEF_INTF_PRIO;
                    intf->last_listener_query_interval = MVR_CONF_DEF_INTF_LLQI;
                    intf->profile_index = MVR_CONF_DEF_INTF_PROFILE;
                }
            }

            roleChanged = FALSE;
            if (op != IPMC_OP_DEL) {
                memset(search_str, 0x0, sizeof(search_str));
                sprintf(search_str, "name_mvr_vlan_%d", mvrif.vid);
                var_string = cyg_httpd_form_varable_string(p, search_str, &mvr_name_len);
                memset(intf->name, 0x0, sizeof(intf->name));
                if (mvr_name_len > 0) {
                    (void) cgi_unescape(var_string, intf->name, mvr_name_len, sizeof(intf->name));
                }

                memset(search_str, 0x0, sizeof(search_str));
                sprintf(search_str, "adrs_mvr_vlan_%d", mvrif.vid);
                if (!cyg_httpd_form_varable_ipv4(p, search_str, &intf->querier4_address)) {
                    memset(search_str, 0x0, sizeof(search_str));
                    sprintf(search_str, "Invalid IGMP Address (used for MVR VLAN %u)!!!", mvrif.vid);
                    send_custom_error(p, "MVR Interface Configuration Error", search_str, strlen(search_str));

                    IPMC_MEM_PROFILE_MGIVE(pfm);
                    return -1;  // Do not further search the file system.
                }

                intf->profile_index = MVR_CONF_DEF_INTF_PROFILE;
                memset(search_str, 0x0, sizeof(search_str));
                sprintf(search_str, "ref_profile_mvr_vlan_%d", mvrif.vid);
                var_string = cyg_httpd_form_varable_string(p, search_str, &mvr_name_len);
                memset(fltr_profile, 0x0, sizeof(ipmc_lib_grp_fltr_profile_t));
                if (mvr_name_len > 0) {
                    (void) cgi_unescape(var_string, data->name, mvr_name_len, sizeof(data->name));
                }
                if (ipmc_lib_mgmt_fltr_profile_get(fltr_profile, TRUE) == VTSS_OK) {
                    intf->profile_index = data->index;
                }

                memset(search_str, 0x0, sizeof(search_str));
                sprintf(search_str, "vmode_mvr_vlan_%d", mvrif.vid);
                if (cyg_httpd_form_varable_int(p, search_str, &var_value)) {
                    intf->mode = var_value;
                }

                memset(search_str, 0x0, sizeof(search_str));
                sprintf(search_str, "vtag_mvr_vlan_%d", mvrif.vid);
                if (cyg_httpd_form_varable_int(p, search_str, &var_value)) {
                    intf->vtag = var_value;
                }

                memset(search_str, 0x0, sizeof(search_str));
                sprintf(search_str, "vpri_mvr_vlan_%d", mvrif.vid);
                if (cyg_httpd_form_varable_int(p, search_str, &var_value)) {
                    intf->priority = var_value;
                }

                memset(search_str, 0x0, sizeof(search_str));
                sprintf(search_str, "vllqi_mvr_vlan_%d", mvrif.vid);
                if (cyg_httpd_form_varable_int(p, search_str, &var_value)) {
                    intf->last_listener_query_interval = var_value;
                }

                (void) port_iter_init(&pit, NULL, sid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
                while (port_iter_getnext(&pit)) {
                    iport = pit.iport;

                    if (port_isid_port_no_is_stack(sid, iport)) {
                        continue;
                    }

                    memset(search_str, 0x0, sizeof(search_str));
                    sprintf(search_str, "prole_%d_mvr_vlan_%d", iport, cntr);
                    if (cyg_httpd_form_varable_int(p, search_str, &var_value)) {
                        if (role->ports[iport] != var_value) {
                            roleChanged = TRUE;
                        }

                        role->ports[iport] = var_value;
                    }
                }
            }

            rc = VTSS_OK;
            if (roleChanged) {
                if ((rc = mvr_mgmt_set_intf_entry(VTSS_ISID_GLOBAL, op, &mvrif)) == VTSS_OK) {
                    rc = mvr_mgmt_set_intf_entry(sid, IPMC_OP_SET, &mvrif);
                }
            } else {
                rc = mvr_mgmt_set_intf_entry(VTSS_ISID_GLOBAL, op, &mvrif);
            }
            if (rc != VTSS_OK) {
                T_D("Failure in SET MVR VLAN VID %u", mvrif.vid);

                memset(search_str, 0x0, sizeof(search_str));
                if (rc == IPMC_ERROR_ENTRY_OVERLAPPED) {
                    sprintf(search_str, "Expected profile (used for MVR VLAN %u) has overlapped addresses used in other MVR VLAN", mvrif.vid);
                } else {
                    if (rc == IPMC_ERROR_ENTRY_NOT_FOUND) {
                        sprintf(search_str, "Expected profile (used for MVR VLAN %u) does not exist", mvrif.vid);
                    } else {
                        sprintf(search_str, "Failure in SET MVR VLAN VID %u", mvrif.vid);
                    }
                }
                send_custom_error(p, "MVR Interface Configuration Error", search_str, strlen(search_str));

                IPMC_MEM_PROFILE_MGIVE(pfm);
                return -1;  // Do not further search the file system.
            }
        }

        /* Process Created Entries Next */
        for (cntr = 1; cntr <= MVR_VLAN_MAX; cntr++) {
            memset(search_str, 0x0, sizeof(search_str));
            sprintf(search_str, "new_mvid_mvr_vlan_%d", cntr);
            if (!cyg_httpd_form_varable_int(p, search_str, &var_value) || !var_value) {
                continue;
            }

            op = IPMC_OP_ADD;
            memset(&mvrif, 0x0, sizeof(mvr_mgmt_interface_t));
            mvrif.vid = (u16)(var_value & 0xFFFF);
            intf = &mvrif.intf;
            role = &mvrif.role;
            intf->querier4_address = MVR_CONF_DEF_INTF_ADRS4;
            intf->mode = MVR_CONF_DEF_INTF_MODE;
            intf->vtag = MVR_CONF_DEF_INTF_VTAG;
            intf->priority = MVR_CONF_DEF_INTF_PRIO;
            intf->last_listener_query_interval = MVR_CONF_DEF_INTF_LLQI;
            if (mvr_mgmt_get_intf_entry(sid, &mvrif) == VTSS_OK) {
                op = IPMC_OP_UPD;
            }
            mvrif.vid = (u16)(var_value & 0xFFFF);

            memset(search_str, 0x0, sizeof(search_str));
            sprintf(search_str, "new_name_mvr_vlan_%d", cntr);
            var_string = cyg_httpd_form_varable_string(p, search_str, &mvr_name_len);
            memset(intf->name, 0x0, sizeof(intf->name));
            if (mvr_name_len > 0) {
                (void) cgi_unescape(var_string, intf->name, mvr_name_len, sizeof(intf->name));
            }

            memset(search_str, 0x0, sizeof(search_str));
            sprintf(search_str, "new_adrs_mvr_vlan_%d", cntr);
            if (!cyg_httpd_form_varable_ipv4(p, search_str, &intf->querier4_address)) {
                memset(search_str, 0x0, sizeof(search_str));
                sprintf(search_str, "Invalid IGMP Address (used for MVR VLAN %u)!!!", mvrif.vid);
                send_custom_error(p, "MVR Interface Configuration Error", search_str, strlen(search_str));

                IPMC_MEM_PROFILE_MGIVE(pfm);
                return -1;  // Do not further search the file system.
            }

            if (cyg_httpd_form_varable_int(p, search_str, &var_value)) {
                intf->mode = var_value;
            }

            intf->profile_index = MVR_CONF_DEF_INTF_PROFILE;
            memset(search_str, 0x0, sizeof(search_str));
            sprintf(search_str, "new_profile_mvr_vlan_%d", cntr);
            var_string = cyg_httpd_form_varable_string(p, search_str, &mvr_name_len);
            memset(fltr_profile, 0x0, sizeof(ipmc_lib_grp_fltr_profile_t));
            if (mvr_name_len > 0) {
                (void) cgi_unescape(var_string, data->name, mvr_name_len, sizeof(data->name));
            }
            if (ipmc_lib_mgmt_fltr_profile_get(fltr_profile, TRUE) == VTSS_OK) {
                intf->profile_index = data->index;
            }

            memset(search_str, 0x0, sizeof(search_str));
            sprintf(search_str, "new_vmode_mvr_vlan_%d", cntr);
            if (cyg_httpd_form_varable_int(p, search_str, &var_value)) {
                intf->mode = var_value;
            }

            memset(search_str, 0x0, sizeof(search_str));
            sprintf(search_str, "new_vtag_mvr_vlan_%d", cntr);
            if (cyg_httpd_form_varable_int(p, search_str, &var_value)) {
                intf->vtag = var_value;
            }

            memset(search_str, 0x0, sizeof(search_str));
            sprintf(search_str, "new_vpri_mvr_vlan_%d", cntr);
            if (cyg_httpd_form_varable_int(p, search_str, &var_value)) {
                intf->priority = var_value;
            }

            memset(search_str, 0x0, sizeof(search_str));
            sprintf(search_str, "new_vllqi_mvr_vlan_%d", cntr);
            if (cyg_httpd_form_varable_int(p, search_str, &var_value)) {
                intf->last_listener_query_interval = var_value;
            }

            roleChanged = FALSE;
            (void) port_iter_init(&pit, NULL, sid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
            while (port_iter_getnext(&pit)) {
                iport = pit.iport;

                if (port_isid_port_no_is_stack(sid, iport)) {
                    continue;
                }

//                uport = iport2uport(iport);
                memset(search_str, 0x0, sizeof(search_str));
                sprintf(search_str, "new_prole_%d_mvr_vlan_%d", iport, cntr);
                if (cyg_httpd_form_varable_int(p, search_str, &var_value)) {
                    if (role->ports[iport] != var_value) {
                        roleChanged = TRUE;
                    }

                    role->ports[iport] = var_value;
                }
            }

            rc = VTSS_OK;
            if (roleChanged) {
                if ((rc = mvr_mgmt_set_intf_entry(VTSS_ISID_GLOBAL, op, &mvrif)) == VTSS_OK) {
                    rc = mvr_mgmt_set_intf_entry(sid, IPMC_OP_SET, &mvrif);
                }
            } else {
                rc = mvr_mgmt_set_intf_entry(VTSS_ISID_GLOBAL, op, &mvrif);
            }
            if (rc != VTSS_OK) {
                T_D("Failure in SET MVR VLAN VID %u", mvrif.vid);

                memset(search_str, 0x0, sizeof(search_str));
                if (rc == IPMC_ERROR_ENTRY_OVERLAPPED) {
                    sprintf(search_str, "Expected profile (used for MVR VLAN %u) has overlapped addresses used in other MVR VLAN", mvrif.vid);
                } else {
                    if (rc == IPMC_ERROR_ENTRY_NOT_FOUND) {
                        sprintf(search_str, "Expected profile (used for MVR VLAN %u) does not exist", mvrif.vid);
                    } else {
                        sprintf(search_str, "Failure in SET MVR VLAN VID %u", mvrif.vid);
                    }
                }
                send_custom_error(p, "MVR Interface Configuration Error", search_str, strlen(search_str));

                IPMC_MEM_PROFILE_MGIVE(pfm);
                return -1;  // Do not further search the file system.
            }
        }

        memset(&fastleave, 0x0, sizeof(mvr_conf_fast_leave_t));
        if (mvr_mgmt_get_fast_leave_port(sid, &fastleave) == VTSS_OK) {
            (void) port_iter_init(&pit, NULL, sid, PORT_ITER_SORT_ORDER_UPORT, PORT_ITER_FLAGS_ALL);
            while (port_iter_getnext(&pit)) {
                uport = pit.uport;
                iport = pit.iport;

                if (port_isid_port_no_is_stack(sid, iport)) {
                    continue;
                }

                memset(search_str, 0x0, sizeof(search_str));
                sprintf(search_str, "fastleave_port_%d", uport);
                if (cyg_httpd_form_varable_int(p, search_str, &var_value)) {
                    VTSS_PORT_BF_SET(fastleave.ports, iport, var_value);
                }
            }

            if (mvr_mgmt_set_fast_leave_port(sid, &fastleave) != VTSS_OK) {
                T_D("Set MVR fast leave port fail");
            }
        }

        redirect(p, errors ? STACK_ERR_URL : "/mvr.htm");
    } else {
        i8  encoded_string_intf[3 * MVR_NAME_MAX_LEN], encoded_string_data[3 * VTSS_IPMC_NAME_MAX_LEN];
        i8  ip_buf[IPV6_ADDR_IBUF_MAX_LEN];

        /* CYG_HTTPD_METHOD_GET (+HEAD) */
        /*
            Format:
            [mvr_mode];[channel_conflict];[profile_1]|...|[profile_n];[pmode]|...
            ;[vid]/[name]/[vmode]/[vtag]/[vpri]/[vllqi]/[profile]/[igmp_adrs],[prole]/...|...
        */
        (void)cyg_httpd_start_chunked("html");

        if (mvr_mgmt_get_mode(&mvr_mode) != VTSS_OK) {
            mvr_mode = FALSE;
        }
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u", mvr_mode);
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);

        if ((rc = mvr_mgmt_validate_intf_channel()) != VTSS_OK) {
            if (rc == IPMC_ERROR_ENTRY_OVERLAPPED) {
                errors = 1;
            } else {
                if (rc == IPMC_ERROR_ENTRY_NOT_FOUND) {
                    errors = 2;
                } else {
                    errors = 3;
                }
            }
        }
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), ";%d", errors);
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);

        cntr = 0;
        memset(fltr_profile, 0x0, sizeof(ipmc_lib_grp_fltr_profile_t));
        while (ipmc_lib_mgmt_fltr_profile_get_next(fltr_profile, TRUE) == VTSS_OK) {
            memset(encoded_string_data, 0x0, sizeof(encoded_string_data));
            ct = cgi_escape(data->name, encoded_string_data);

            ct = snprintf(p->outbuffer, sizeof(p->outbuffer),
                          cntr ? "|%s" : ";%s",
                          encoded_string_data);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);

            cntr++;
        }

        if (!cntr) {
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), ";");
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        }

        memset(&fastleave, 0x0, sizeof(mvr_conf_fast_leave_t));
        (void) mvr_mgmt_get_fast_leave_port(sid, &fastleave);
        (void) port_iter_init(&pit, NULL, sid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
        while (port_iter_getnext(&pit)) {
            iport = pit.iport;

            if (!port_isid_port_no_is_stack(sid, iport)) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer),
                              (iport == VTSS_PORT_NO_START) ? ";%u" : "|%u",
                              VTSS_PORT_BF_GET(fastleave.ports, iport));
            } else {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), (iport == VTSS_PORT_NO_START) ? ";-1" : "|-1");
            }

            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        }

        cntr = 0;
        memset(&mvrif, 0x0, sizeof(mvr_mgmt_interface_t));
        while (mvr_mgmt_get_next_intf_entry(sid, &mvrif) == VTSS_OK) {
            intf = &mvrif.intf;

            memset(fltr_profile, 0x0, sizeof(ipmc_lib_grp_fltr_profile_t));
            data->index = intf->profile_index;
            if (ipmc_lib_mgmt_fltr_profile_get(fltr_profile, FALSE) != VTSS_OK) {
                memset(fltr_profile, 0x0, sizeof(ipmc_lib_grp_fltr_profile_t));
            }

            memset(encoded_string_intf, 0x0, sizeof(encoded_string_intf));
            ct = cgi_escape(intf->name, encoded_string_intf);
            memset(encoded_string_data, 0x0, sizeof(encoded_string_data));
            ct = cgi_escape(data->name, encoded_string_data);
            memset(ip_buf, 0x0, sizeof(ip_buf));
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer),
                          (cntr > 0) ? "|%u/%s/%d/%d/%u/%u/%s/%s" : ";%u/%s/%d/%d/%u/%u/%s/%s",
                          intf->vid,
                          encoded_string_intf,
                          intf->mode,
                          intf->vtag,
                          intf->priority,
                          intf->last_listener_query_interval,
                          encoded_string_data,
                          misc_ipv4_txt(intf->querier4_address, ip_buf));
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);

            role = &mvrif.role;
            (void) port_iter_init(&pit, NULL, sid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
            while (port_iter_getnext(&pit)) {
                iport = pit.iport;

                if (!port_isid_port_no_is_stack(sid, iport)) {
                    ct = snprintf(p->outbuffer, sizeof(p->outbuffer),
                                  (iport == VTSS_PORT_NO_START) ? ",%d" : "/%d",
                                  role->ports[iport]);
                } else {
                    ct = snprintf(p->outbuffer, sizeof(p->outbuffer),
                                  (iport == VTSS_PORT_NO_START) ? ",%d" : "/%d",
                                  MVR_PORT_ROLE_STACK);
                }

                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            }

            cntr++;
        }

        if (!cntr) {
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), ";NoEntries");
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        }

        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), ";");
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        (void)cyg_httpd_end_chunked();
    }

    IPMC_MEM_PROFILE_MGIVE(pfm);
    return -1;  // Do not further search the file system.
}

static cyg_int32 handler_stat_mvr_status(CYG_HTTPD_STATE *p)
{
    vtss_isid_t                     sid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    int                             ct, cntr;
    ipmc_prot_intf_entry_param_t    intf_param;
    ipmc_ip_version_t               version;
    u16                             mvid;
    BOOL                            flag, mvr_mode;

    /* Redirect unmanaged/invalid access to handler */
    if (redirectUnmanagedOrInvalid(p, sid)) {
        return -1;
    }

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_MVR)) {
        return -1;
    }
#endif /* VTSS_SW_OPTION_PRIV_LVL */

    /* Clear? */
    if ((cyg_httpd_form_varable_find(p, "clear") != NULL)) {
        if (mvr_mgmt_clear_stat_counter(sid, VTSS_IPMC_VID_ALL) != VTSS_OK) {
            T_D("Clear MVR counters fail");
        } else {
            /* Wait for processing statistics */
            VTSS_OS_MSLEEP(100);
        }
    }

    /* Normal read */
    if (mvr_mgmt_get_mode(&mvr_mode) != VTSS_OK) {
        mvr_mode = FALSE;
    }

    /*
        Format:
        [vid],[name],[querier_status];
        [rx_igmp_query],[tx_igmp_query],
        [rx_igmpv1_joins],[rx_igmpv2_joins],[rx_igmpv3_joins],[rx_igmpv2_leaves];
        [rx_mld_query],[tx_mld_query],
        [rx_mldv1_reports],[rx_mldv2_reports],[rx_mldv1_dones]|...
    */
    (void)cyg_httpd_start_chunked("html");

    mvid = 0;
    flag = FALSE;
    cntr = 0;
    version = IPMC_IP_VERSION_ALL;
    memset(&intf_param, 0x0, sizeof(ipmc_prot_intf_entry_param_t));
    while (mvr_mgmt_get_next_intf_info(sid, &version, &intf_param) == VTSS_OK) {
        if (!flag && (mvid != intf_param.vid)) {
            flag = TRUE;
        }

        if (flag) {
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer),
                          "%s%u,%s,%d",
                          (cntr > 0) ? "|" : "",
                          intf_param.vid,
                          "",
                          mvr_mode ? intf_param.querier.state : IPMC_QUERIER_INIT);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        }

        if (version == IPMC_IP_VERSION_IGMP) {
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer),
                          ";%u,%u,%u,%u,%u,%u",
                          intf_param.stats.igmp_queries,
                          intf_param.querier.ipmc_queries_sent,
                          intf_param.stats.igmp_v1_membership_join,
                          intf_param.stats.igmp_v2_membership_join,
                          intf_param.stats.igmp_v3_membership_join,
                          intf_param.stats.igmp_v2_membership_leave);
        } else {
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer),
                          ";%u,%u,%u,%u,%u",
                          intf_param.stats.mld_queries,
                          intf_param.querier.ipmc_queries_sent,
                          intf_param.stats.mld_v1_membership_report,
                          intf_param.stats.mld_v2_membership_report,
                          intf_param.stats.mld_v1_membership_done);
        }
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);

        if (flag) {
            mvid = intf_param.vid;
            flag = FALSE;
        }

        cntr++;
    }

    if (!cntr) {
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "NoEntries|");
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);
    }

    (void)cyg_httpd_end_chunked();

    return -1; // Do not further search the file system.
}

static cyg_int32 handler_stat_mvr_groups_info(CYG_HTTPD_STATE *p)
{
    vtss_isid_t                         sid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    int                                 ct = 0;
    ipmc_prot_intf_group_entry_t        intf_group;
    char                                ip_buf[IPV6_ADDR_IBUF_MAX_LEN];
    u16                                 vid = 0, start_vid = 1;
    int                                 entry_cnt, dyn_get_next_entry;
    int                                 num_of_entries = 0;
    u32                                 portCnt;
    vtss_ipv4_t                         start_group_addr = 0;
    vtss_ipv6_t                         start_group_addr_ipv6;
    port_iter_t                         pit;
    vtss_port_no_t                      iport;
    BOOL                                port_selected;
    int                                 var_value;
    ipmc_ip_version_t                   ipmc_version;
    const char                          *var_string;

    /* Redirect unmanaged/invalid access to handler */
    if (redirectUnmanagedOrInvalid(p, sid)) {
        return -1;
    }

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_MVR)) {
        return -1;
    }
#endif /* VTSS_SW_OPTION_PRIV_LVL */

    /*
        Format:
        <startVid>|<startGroup>|<NumberOfEntries>|<portCnt>|[vid],[groups],[p1_is_mbr],[p2_is_mbr],[p3_is_mbr],.../...
    */
    (void)cyg_httpd_start_chunked("html");

    // Get number of entries per page
    if ((var_string = cyg_httpd_form_varable_find(p, "DynNumberOfEntries")) != NULL) {
        num_of_entries = atoi(var_string);
    }
    if ((num_of_entries < 1) || (num_of_entries > 99)) {
        num_of_entries = 20;
    }

    memset(&intf_group, 0x0, sizeof(ipmc_prot_intf_group_entry_t));

    // Get start vid
    if ((var_string = cyg_httpd_form_varable_find(p, "DynStartVid")) != NULL) {
        intf_group.vid = vid = start_vid = atoi(var_string);
    }

    // Get start group address
    if (!cyg_httpd_form_varable_ipv6(p, "DynStartGroup", &start_group_addr_ipv6)) {
        if (!cyg_httpd_form_varable_ipv4(p, "DynStartGroup", &start_group_addr)) {
            ipmc_version = IPMC_IP_VERSION_ALL;
            memset(&start_group_addr_ipv6, 0x0, sizeof(vtss_ipv6_t));
            start_group_addr = 0;
        } else {
            ipmc_version = IPMC_IP_VERSION_IGMP;
        }
    } else {
        ipmc_version = IPMC_IP_VERSION_MLD;
    }

    // Get or GetNext
    dyn_get_next_entry = 0;
    if ((var_string = cyg_httpd_form_varable_find(p, "GetNextEntry")) != NULL) {
        dyn_get_next_entry = atoi(var_string);
    }

    memset(&ip_buf[0], 0x0, sizeof(ip_buf));
    portCnt = port_isid_port_count(sid);
    if (!dyn_get_next_entry) { /* Not get next button */
        if (ipmc_version == IPMC_IP_VERSION_IGMP) {
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d|%s|%d|%u",
                          (start_vid > 0) ? start_vid : 1,
                          misc_ipv4_txt(start_group_addr, ip_buf),
                          num_of_entries,
                          portCnt);
        } else {
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d|%s|%d|%u",
                          (start_vid > 0) ? start_vid : 1,
                          misc_ipv6_txt(&start_group_addr_ipv6, ip_buf),
                          num_of_entries,
                          portCnt);
        }
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);
    } else {
        if (ipmc_version == IPMC_IP_VERSION_IGMP) {
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d|%s|%d|%u",
                          (start_vid > 0) ? start_vid : 1,
                          misc_ipv4_txt(start_group_addr, ip_buf),
                          num_of_entries,
                          portCnt);
        } else if (ipmc_version == IPMC_IP_VERSION_MLD) {
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d|%s|%d|%u",
                          (start_vid > 0) ? start_vid : 1,
                          misc_ipv6_txt(&start_group_addr_ipv6, ip_buf),
                          num_of_entries,
                          portCnt);
        } else {
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d|%s|%d|%u",
                          (start_vid > 0) ? start_vid : 1,
                          "",
                          num_of_entries,
                          portCnt);
        }
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);
    }

    if (ipmc_version == IPMC_IP_VERSION_IGMP) {
        var_value = ntohl(start_group_addr);
        memcpy(&intf_group.group_addr.addr[12], &var_value, sizeof(u32));
    } else {
        memcpy(&intf_group.group_addr, &start_group_addr_ipv6, sizeof(vtss_ipv6_t));
    }
    --intf_group.group_addr.addr[15];

    entry_cnt = 0;
    while (mvr_mgmt_get_next_intf_group(sid, &ipmc_version, &vid, &intf_group) == VTSS_OK) {
        if (vid < start_vid) {
            continue;
        }

        if (ipmc_version == IPMC_IP_VERSION_IGMP) {
            u32 group_addr;

            memcpy((uchar *)&group_addr, &intf_group.group_addr.addr[12], sizeof(u32));
            group_addr = htonl(group_addr);
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s%u,%s",
                          (entry_cnt > 0) ? "/" : "|",
                          vid,
                          misc_ipv4_txt(group_addr, ip_buf));
        } else {
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s%u,%s",
                          (entry_cnt > 0) ? "/" : "|",
                          vid,
                          misc_ipv6_txt(&intf_group.group_addr, ip_buf));
        }
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);

        (void) port_iter_init(&pit, NULL, sid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
        while (port_iter_getnext(&pit)) {
            iport = pit.iport;

            /* we hide stacking port that we get the same result from iCLI */
            if (port_isid_port_no_is_stack(sid, iport)) {
                port_selected = FALSE;
            } else {
                port_selected = VTSS_PORT_BF_GET(intf_group.db.port_mask, iport);
            }
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), ",%u", port_selected);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        }

        if (++entry_cnt >= num_of_entries) {
            break;
        }
    }

    if (entry_cnt == 0) { /* No entry existing */
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "|NoEntries");
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);
    }

    (void)cyg_httpd_end_chunked();

    return -1; // Do not further search the file system.
}

static cyg_int32 handler_stat_mvr_groups_sfm(CYG_HTTPD_STATE *p)
{
    vtss_isid_t                         sid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    int                                 ct = 0;
    u32                                 entry_cnt, num_of_entries;
    int                                 dyn_get_next_entry;
    ipmc_prot_intf_group_entry_t        intf_group;
    ipmc_prot_group_srclist_t           group_srclist_entry;
    ipmc_sfm_srclist_t                  sfm_src;
    u16                                 vid = 0, start_vid = 1;
    port_iter_t                         pit;
    vtss_port_no_t                      iport;
    vtss_uport_no_t                     uport;
    char                                ip_buf[IPV6_ADDR_IBUF_MAX_LEN];
    int                                 idx;
    u32                                 start_group_addr = 0;
    vtss_ipv6_t                         start_group_addr_ipv6;
    BOOL                                amir, smir;
    ipmc_ip_version_t                   ipmc_version;
    const char                          *var_string;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_MVR)) {
        return -1;
    }
#endif /* VTSS_SW_OPTION_PRIV_LVL */

    /*
        Format:
        <start_vid>|<start_group>|<num_of_entries>;[vid]/[group]/[port]/[mode]/[source_addr]/[type]/[in_hw]|...
    */
    (void)cyg_httpd_start_chunked("html");

    // Get number of entries per page
    num_of_entries = 0;
    if ((var_string = cyg_httpd_form_varable_find(p, "DynNumberOfEntries")) != NULL) {
        num_of_entries = atoi(var_string);
    }
    if ((num_of_entries < 1) || (num_of_entries > 99)) {
        num_of_entries = 20;
    }

    memset(&intf_group, 0x0, sizeof(ipmc_prot_intf_group_entry_t));

    // Get start vid
    if ((var_string = cyg_httpd_form_varable_find(p, "DynStartVid")) != NULL) {
        intf_group.vid = vid = start_vid = atoi(var_string);
    }

    // Get start group address
    if (!cyg_httpd_form_varable_ipv6(p, "DynStartGroup", &start_group_addr_ipv6)) {
        if (!cyg_httpd_form_varable_ipv4(p, "DynStartGroup", &start_group_addr)) {
            ipmc_version = IPMC_IP_VERSION_ALL;
            memset(&start_group_addr_ipv6, 0x0, sizeof(vtss_ipv6_t));
            start_group_addr = 0;
        } else {
            ipmc_version = IPMC_IP_VERSION_IGMP;
        }
    } else {
        ipmc_version = IPMC_IP_VERSION_MLD;
    }

    // Get or GetNext
    dyn_get_next_entry = 0;
    if ((var_string = cyg_httpd_form_varable_find(p, "GetNextEntry")) != NULL) {
        dyn_get_next_entry = atoi(var_string);
    }

    memset(&ip_buf[0], 0x0, sizeof(ip_buf));
    if (!dyn_get_next_entry) { /* Not get next button */
        if (ipmc_version == IPMC_IP_VERSION_IGMP) {
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d|%s|%u",
                          (start_vid > 0) ? start_vid : 1,
                          misc_ipv4_txt(start_group_addr, ip_buf),
                          num_of_entries);
        } else {
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d|%s|%u",
                          (start_vid > 0) ? start_vid : 1,
                          misc_ipv6_txt(&start_group_addr_ipv6, ip_buf),
                          num_of_entries);
        }
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);
    } else {
        if (ipmc_version == IPMC_IP_VERSION_IGMP) {
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d|%s|%u",
                          (start_vid > 0) ? start_vid : 1,
                          misc_ipv4_txt(start_group_addr, ip_buf),
                          num_of_entries);
        } else if (ipmc_version == IPMC_IP_VERSION_MLD) {
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d|%s|%u",
                          (start_vid > 0) ? start_vid : 1,
                          misc_ipv6_txt(&start_group_addr_ipv6, ip_buf),
                          num_of_entries);
        } else {
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d|%s|%u",
                          (start_vid > 0) ? start_vid : 1,
                          "",
                          num_of_entries);
        }
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);
    }

    if (ipmc_version == IPMC_IP_VERSION_IGMP) {
        idx = ntohl(start_group_addr);
        memcpy(&intf_group.group_addr.addr[12], &idx, sizeof(u32));
    } else {
        memcpy(&intf_group.group_addr, &start_group_addr_ipv6, sizeof(vtss_ipv6_t));
    }
    for (idx = 15; idx >= 0; idx--) {
        if (intf_group.group_addr.addr[idx]) {
            --intf_group.group_addr.addr[idx];
            break;
        }
    }

    entry_cnt = 0;
    while (mvr_mgmt_get_next_intf_group(sid, &ipmc_version, &vid, &intf_group) == VTSS_OK) {
        if (entry_cnt >= num_of_entries) {
            break;
        }
        if (vid < start_vid) {
            continue;
        }

        amir = intf_group.db.asm_in_hw;
        (void) port_iter_init(&pit, NULL, sid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
        while (port_iter_getnext(&pit)) {
            iport = pit.iport;
            /* we hide stacking port that we get the same result from iCLI */
            if (port_isid_port_no_is_stack(sid, iport)) {
                continue;
            }

            if (IPMC_LIB_GRP_PORT_DO_SFM(&intf_group.db, iport)) {
                BOOL    sfm_found = FALSE;
                u32     ip4grp, ip4src;
                char    src_buf[IPV6_ADDR_IBUF_MAX_LEN];

                uport = iport2uport(iport);

                /* SIP/Allow */
                memset(&group_srclist_entry, 0x0, sizeof(ipmc_prot_group_srclist_t));
                group_srclist_entry.type = TRUE;    /* ALLOW */
                while (mvr_mgmt_get_next_group_srclist(
                           sid, ipmc_version, intf_group.vid,
                           &intf_group.group_addr,
                           &group_srclist_entry) == VTSS_OK) {
                    if (!group_srclist_entry.cntr) {
                        break;
                    }

                    memcpy(&sfm_src, &group_srclist_entry.srclist, sizeof(ipmc_sfm_srclist_t));
                    if (VTSS_PORT_BF_GET(sfm_src.port_mask, iport) == FALSE) {
                        continue;
                    }

                    sfm_found = TRUE;
                    smir = sfm_src.sfm_in_hw;
                    /* ;[vid]/[group]/[port]/[mode]/[source_addr]/[type]/[in_hw]|... */
                    if (ipmc_version == IPMC_IP_VERSION_IGMP) {
                        memcpy((uchar *)&ip4grp, &intf_group.group_addr.addr[12], sizeof(u32));
                        memcpy((uchar *)&ip4src, &sfm_src.src_ip.addr[12], sizeof(u32));
                        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s%u/%s/%d/%u/%s/%u/%u",
                                      (entry_cnt > 0) ? "|" : ";",
                                      vid,
                                      misc_ipv4_txt(htonl(ip4grp), ip_buf),
                                      uport,
                                      VTSS_PORT_BF_GET(intf_group.db.ipmc_sf_port_mode, iport),
                                      misc_ipv4_txt(htonl(ip4src), src_buf),
                                      TRUE,
                                      (amir && smir));
                    } else {
                        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s%u/%s/%d/%u/%s/%u/%u",
                                      (entry_cnt > 0) ? "|" : ";",
                                      vid,
                                      misc_ipv6_txt(&intf_group.group_addr, ip_buf),
                                      uport,
                                      VTSS_PORT_BF_GET(intf_group.db.ipmc_sf_port_mode, iport),
                                      misc_ipv6_txt(&sfm_src.src_ip, src_buf),
                                      TRUE,
                                      (amir && smir));
                    }
                    (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                    if (++entry_cnt >= num_of_entries) {
                        break;
                    }
                }

                if (entry_cnt >= num_of_entries) {
                    break;
                }

                /* SIP/Deny */
                memset(&group_srclist_entry, 0x0, sizeof(ipmc_prot_group_srclist_t));
                group_srclist_entry.type = FALSE;   /* DENY */
                while (mvr_mgmt_get_next_group_srclist(
                           sid, ipmc_version, intf_group.vid,
                           &intf_group.group_addr,
                           &group_srclist_entry) == VTSS_OK) {
                    if (!group_srclist_entry.cntr) {
                        break;
                    }

                    memcpy(&sfm_src, &group_srclist_entry.srclist, sizeof(ipmc_sfm_srclist_t));
                    if (VTSS_PORT_BF_GET(sfm_src.port_mask, iport) == FALSE) {
                        continue;
                    }

                    sfm_found = TRUE;
                    smir = sfm_src.sfm_in_hw;
                    /* ;[vid]/[group]/[port]/[mode]/[source_addr]/[type]/[in_hw]|... */
                    if (ipmc_version == IPMC_IP_VERSION_IGMP) {
                        memcpy((uchar *)&ip4grp, &intf_group.group_addr.addr[12], sizeof(u32));
                        memcpy((uchar *)&ip4src, &sfm_src.src_ip.addr[12], sizeof(u32));
                        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s%u/%s/%d/%u/%s/%u/%u",
                                      (entry_cnt > 0) ? "|" : ";",
                                      vid,
                                      misc_ipv4_txt(htonl(ip4grp), ip_buf),
                                      uport,
                                      VTSS_PORT_BF_GET(intf_group.db.ipmc_sf_port_mode, iport),
                                      misc_ipv4_txt(htonl(ip4src), src_buf),
                                      FALSE,
                                      (amir && smir));
                    } else {
                        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s%u/%s/%d/%u/%s/%u/%u",
                                      (entry_cnt > 0) ? "|" : ";",
                                      vid,
                                      misc_ipv6_txt(&intf_group.group_addr, ip_buf),
                                      uport,
                                      VTSS_PORT_BF_GET(intf_group.db.ipmc_sf_port_mode, iport),
                                      misc_ipv6_txt(&sfm_src.src_ip, src_buf),
                                      FALSE,
                                      (amir && smir));
                    }
                    (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                    if (++entry_cnt >= num_of_entries) {
                        break;
                    }
                }

                if (entry_cnt >= num_of_entries) {
                    break;
                }

                if (!sfm_found) {
                    /* ;[vid]/[group]/[port]/[mode]/[source_addr]/[type]/[in_hw]|... */
                    if (ipmc_version == IPMC_IP_VERSION_IGMP) {
                        memcpy((uchar *)&ip4grp, &intf_group.group_addr.addr[12], sizeof(u32));
                        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s%u/%s/%d/%u/%s/%u/%u",
                                      (entry_cnt > 0) ? "|" : ";",
                                      vid,
                                      misc_ipv4_txt(htonl(ip4grp), ip_buf),
                                      uport,
                                      VTSS_PORT_BF_GET(intf_group.db.ipmc_sf_port_mode, iport),
                                      "",
                                      FALSE,
                                      amir);
                    } else {
                        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s%u/%s/%d/%u/%s/%u/%u",
                                      (entry_cnt > 0) ? "|" : ";",
                                      vid,
                                      misc_ipv6_txt(&intf_group.group_addr, ip_buf),
                                      uport,
                                      VTSS_PORT_BF_GET(intf_group.db.ipmc_sf_port_mode, iport),
                                      "",
                                      FALSE,
                                      amir);
                    }
                    (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                    if (++entry_cnt >= num_of_entries) {
                        break;
                    }
                }
            }
        }
    }

    if (entry_cnt == 0) { /* No entry existing */
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), ";NoEntries");
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);
    }

    (void)cyg_httpd_end_chunked();

    return -1; // Do not further search the file system.
}


/****************************************************************************/
/*  Module JS lib config routine                                            */
/****************************************************************************/

static size_t mvr_lib_config_js(char **base_ptr, char **cur_ptr, size_t *length)
{
    char    buff[MVR_WEB_BUF_LEN];
    (void) snprintf(buff, MVR_WEB_BUF_LEN,
                    "var configMvrVlanMax = %d;\n"
                    "var configMvrVlanNameLen = %d;\n"
                    "var configMvrRoleStack = %d;\n"
                    "var configMvrCharAsciiMin = %d;\n"
                    "var configMvrCharAsciiMax = %d;\n",
                    MVR_VLAN_MAX,
                    (MVR_NAME_MAX_LEN - 1),
                    MVR_PORT_ROLE_STACK,
                    IPMC_LIB_CHAR_ASCII_MIN,
                    IPMC_LIB_CHAR_ASCII_MAX);
    return webCommonBufferHandler(base_ptr, cur_ptr, length, buff);
}

/****************************************************************************/
/*  JS lib config table entry                                               */
/****************************************************************************/

web_lib_config_js_tab_entry(mvr_lib_config_js);

/****************************************************************************/
/*  HTTPD Table Entries                                                     */
/****************************************************************************/

CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_mvr, "/config/mvr", handler_config_mvr);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_stat_mvr_status, "/stat/mvr_status", handler_stat_mvr_status);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_stat_mvr_groups_info, "/stat/mvr_groups_info", handler_stat_mvr_groups_info);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_stat_mvr_groups_sfm, "/stat/mvr_groups_sfm", handler_stat_mvr_groups_sfm);

/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
