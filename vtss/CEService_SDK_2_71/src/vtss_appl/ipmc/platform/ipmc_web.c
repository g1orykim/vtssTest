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
#endif

#include "ipmc.h"
#include "ipmc_api.h"


#define VTSS_TRACE_MODULE_ID    VTSS_MODULE_ID_IPMC
#define IPMC_WEB_BUF_LEN        512

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

static cyg_int32 handler_config_ipmc(CYG_HTTPD_STATE *p)
{
    vtss_isid_t                     sid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    port_iter_t                     pit;
    vtss_port_no_t                  iport;
    vtss_uport_no_t                 uport;
    int                             ct = 0;
    BOOL                            ipmc_mode, unreg_flood, leave_proxy, ipmc_proxy;
    ipmc_prefix_t                   pfx;
    ipmc_conf_router_port_t         ipmc_rports, ipmc_rports_new;
    ipmc_conf_fast_leave_port_t     fast_leave_ports, fast_leave_ports_new;
    ipmc_conf_throttling_max_no_t   throttling_max_no, throttling_max_no_new;
    char                            var_router_port[16], var_fast_leave_port[20], ip_buf[IPV6_ADDR_IBUF_MAX_LEN];
    int                             var_value;
    ipmc_ip_version_t               ipmc_version = IPMC_IP_VERSION_IGMP;
    const char                      *var_string;
#ifdef VTSS_SW_OPTION_SMB_IPMC
    char                            var_throttling_port[20];
    int                             ipmc_smb = 1, var_value2;
#else
    int                             ipmc_smb = 0;
#endif /* VTSS_SW_OPTION_SMB_IPMC */

    if (redirectUnmanagedOrInvalid(p, sid)) { /* Redirect unmanaged/invalid access to handler */
        return -1;
    }

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_IPMC)) {
        return -1;
    }
#endif

    if (p->method == CYG_HTTPD_METHOD_POST) {
        int errors = 0;
        size_t nlen;

        (void) cyg_httpd_form_varable_int(p, "ipmc_version", &var_value);
        ipmc_version = var_value;
        if (ipmc_mgmt_get_mode(&ipmc_mode, ipmc_version) != VTSS_OK) {
            T_W("Get IPMC Snooping Mode fail");
        }
        if (cyg_httpd_form_varable_string(p, "ipmc_mode", &nlen)) {
            if (ipmc_mode != TRUE) {
                ipmc_mode = TRUE;
                if (ipmc_mgmt_set_mode(&ipmc_mode, ipmc_version) != VTSS_OK) {
                    T_W("SET IPMC Snooping Mode fail");
                }
            }
        } else {
            if (ipmc_mode == TRUE) {
                ipmc_mode = FALSE;
                if (ipmc_mgmt_set_mode(&ipmc_mode, ipmc_version) != VTSS_OK) {
                    T_W("SET IPMC Snooping Mode fail");
                }
            }
        }

        if (ipmc_mgmt_get_unreg_flood(&unreg_flood, ipmc_version) != VTSS_OK) {
            T_W("Get IPMC Unregister flood Mode fail");
        }
        if ( cyg_httpd_form_varable_string(p, "unreg_ipmc", &nlen) ) {
            if (unreg_flood != TRUE) {
                unreg_flood = TRUE;
                if (ipmc_mgmt_set_unreg_flood(&unreg_flood, ipmc_version) != VTSS_OK) {
                    T_W("SET IPMC Unregister flood Mode fail");
                }
            }
        } else {
            if (unreg_flood == TRUE) {
                unreg_flood = FALSE;
                if (ipmc_mgmt_set_unreg_flood(&unreg_flood, ipmc_version) != VTSS_OK) {
                    T_W("SET IPMC Unregister flood Mode fail");
                }
            }
        }

        if (ipmc_mgmt_get_ssm_range(ipmc_version, &pfx) != VTSS_OK) {
            T_W("Get IPMC SSM Range fail");
        }

        if (ipmc_mgmt_get_leave_proxy(&leave_proxy, ipmc_version) != VTSS_OK) {
            T_W("Get IPMC Leave Proxy Mode fail");
        }

        if (ipmc_mgmt_get_proxy(&ipmc_proxy, ipmc_version) != VTSS_OK) {
            T_W("Get IPMC Proxy Mode fail");
        }

#ifdef VTSS_SW_OPTION_SMB_IPMC
        if (ipmc_version == IPMC_IP_VERSION_IGMP) {
            pfx.len = VTSS_IPMC_SSM4_RANGE_LEN;
            (void) cyg_httpd_form_varable_ipv4(p, "ssm4_range_prefix", &pfx.addr.value.prefix);
            if (cyg_httpd_form_varable_int(p, "ssm4_range_length", &var_value)) {
                pfx.len = var_value;
            }
        } else {
            pfx.len = VTSS_IPMC_SSM6_RANGE_LEN;
            (void) cyg_httpd_form_varable_ipv6(p, "ssm6_range_prefix", &pfx.addr.array.prefix);
            if (cyg_httpd_form_varable_int(p, "ssm6_range_length", &var_value)) {
                pfx.len = var_value;
            }
        }
        if (ipmc_mgmt_set_ssm_range(ipmc_version, &pfx) != VTSS_OK) {
            T_W("SET IPMC SSM Range fail");
        }

        if ( cyg_httpd_form_varable_string(p, "leave_proxy", &nlen) ) {
            if (leave_proxy != TRUE) {
                leave_proxy = TRUE;
                if (ipmc_mgmt_set_leave_proxy(&leave_proxy, ipmc_version) != VTSS_OK) {
                    T_W("SET IPMC Leave Proxy Mode fail");
                }
            }
        } else {
            if (leave_proxy == TRUE) {
                leave_proxy = FALSE;
                if (ipmc_mgmt_set_leave_proxy(&leave_proxy, ipmc_version) != VTSS_OK) {
                    T_W("SET IPMC Leave Proxy Mode fail");
                }
            }
        }
        if ( cyg_httpd_form_varable_string(p, "proxy", &nlen) ) {
            if (ipmc_proxy != TRUE) {
                ipmc_proxy = TRUE;
                if (ipmc_mgmt_set_proxy(&ipmc_proxy, ipmc_version) != VTSS_OK) {
                    T_W("SET IPMC Proxy Mode fail");
                }
            }
        } else {
            if (ipmc_proxy == TRUE) {
                ipmc_proxy = FALSE;
                if (ipmc_mgmt_set_proxy(&ipmc_proxy, ipmc_version) != VTSS_OK) {
                    T_W("SET IPMC Proxy Mode fail");
                }
            }
        }
#endif /* VTSS_SW_OPTION_SMB_IPMC */

        if (ipmc_mgmt_get_static_router_port(sid, &ipmc_rports, ipmc_version) != VTSS_OK) {
            T_W("Get IPMC Router ports fail");
        }
        if (ipmc_mgmt_get_fast_leave_port(sid, &fast_leave_ports, ipmc_version) != VTSS_OK) {
            T_W("Get IPMC Fast Leave ports fail");
        }
        if (ipmc_mgmt_get_throttling_max_count(sid, &throttling_max_no, ipmc_version) != VTSS_OK) {
            T_W("Get IPMC Port Throttling Max Number fail");
        }

        memset(&ipmc_rports_new, 0x0, sizeof(ipmc_rports_new));
        memset(&fast_leave_ports_new, 0x0, sizeof(fast_leave_ports_new));
        memset(&throttling_max_no_new, 0x0, sizeof(throttling_max_no_new));
        (void) port_iter_init(&pit, NULL, sid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
        while (port_iter_getnext(&pit)) {
            iport = pit.iport;

            uport = iport2uport(iport);
            memset(var_router_port, 0x0, sizeof(var_router_port));
            sprintf(var_router_port, "router_port_%d", uport);
            memset(var_fast_leave_port, 0x0, sizeof(var_fast_leave_port));
            sprintf(var_fast_leave_port, "fast_leave_port_%d", uport);

            if (cyg_httpd_form_varable_string(p, var_router_port, &nlen)) {
                ipmc_rports_new.ports[iport] = TRUE;
            } else {
                ipmc_rports_new.ports[iport] = FALSE;
            }

            if (cyg_httpd_form_varable_string(p, var_fast_leave_port, &nlen)) {
                fast_leave_ports_new.ports[iport] = TRUE;
            } else {
                fast_leave_ports_new.ports[iport] = FALSE;
            }

#ifdef VTSS_SW_OPTION_SMB_IPMC
            memset(var_throttling_port, 0x0, sizeof(var_throttling_port));
            sprintf(var_throttling_port, "throttling_port_%d", uport);
            if (cyg_httpd_form_varable_int(p, var_throttling_port, &var_value2)) {
                throttling_max_no_new.ports[iport] = var_value2;
            }
#endif /* VTSS_SW_OPTION_SMB_IPMC */
        }
        if (memcmp(&ipmc_rports, &ipmc_rports_new, sizeof(ipmc_rports)) != 0) {
            if (ipmc_mgmt_set_router_port(sid, &ipmc_rports_new, ipmc_version) != VTSS_OK) {
                T_W("Set IPMC Router ports fail");
            }
        }
        if (memcmp(&fast_leave_ports, &fast_leave_ports_new, sizeof(fast_leave_ports)) != 0) {
            if (ipmc_mgmt_set_fast_leave_port(sid, &fast_leave_ports_new, ipmc_version) != VTSS_OK) {
                T_W("Set IPMC Fast Leave ports fail");
            }
        }
        if (memcmp(&throttling_max_no, &throttling_max_no_new, sizeof(throttling_max_no)) != 0) {
            if (ipmc_mgmt_set_throttling_max_count(sid, &throttling_max_no_new, ipmc_version) != VTSS_OK) {
                T_W("Set IPMC Port throttling fail");
            }
        }

        if (ipmc_version == IPMC_IP_VERSION_IGMP) {
            redirect(p, errors ? STACK_ERR_URL : "/ipmc_igmps.htm");
        } else {
            redirect(p, errors ? STACK_ERR_URL : "/ipmc_mldsnp.htm");
        }
    } else {
        /* CYG_HTTPD_METHOD_GET (+HEAD) */

        // Get ipmc_version
        if ((var_string = cyg_httpd_form_varable_find(p, "ipmc_version")) != NULL) {
            ipmc_version = atoi(var_string);
        }

        (void)cyg_httpd_start_chunked("html");

        if (ipmc_mgmt_get_mode(&ipmc_mode, ipmc_version) == VTSS_OK &&
            ipmc_mgmt_get_unreg_flood(&unreg_flood, ipmc_version) == VTSS_OK &&
            ipmc_mgmt_get_ssm_range(ipmc_version, &pfx) == VTSS_OK &&
            ipmc_mgmt_get_leave_proxy(&leave_proxy, ipmc_version) == VTSS_OK &&
            ipmc_mgmt_get_proxy(&ipmc_proxy, ipmc_version) == VTSS_OK &&
            ipmc_mgmt_get_static_router_port(sid, &ipmc_rports, ipmc_version) == VTSS_OK &&
            ipmc_mgmt_get_fast_leave_port(sid, &fast_leave_ports, ipmc_version) == VTSS_OK &&
            ipmc_mgmt_get_throttling_max_count(sid, &throttling_max_no, ipmc_version) == VTSS_OK) {

            /* Format: [ipmc_smb]/[ipmc_mode]/[unreg_ipmc]/[ssm6_range_prefix]/[ssm6_range_length]/[leave_proxy]/[proxy]
                       ,[port_no]/[router_port]/[fast_leave]/[throttling]|...
            */
            memset(&ip_buf[0], 0x0, sizeof(ip_buf));
            if (ipmc_version == IPMC_IP_VERSION_IGMP) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer),
                              "%d/%u/%u/%s/%u/%u/%u,",
                              ipmc_smb,
                              ipmc_mode,
                              unreg_flood,
                              misc_ipv4_txt(pfx.addr.value.prefix, ip_buf),
                              pfx.len,
                              leave_proxy,
                              ipmc_proxy);
            } else {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer),
                              "%d/%u/%u/%s/%u/%u/%u,",
                              ipmc_smb,
                              ipmc_mode,
                              unreg_flood,
                              misc_ipv6_txt(&pfx.addr.array.prefix, ip_buf),
                              pfx.len,
                              leave_proxy,
                              ipmc_proxy);
            }
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);

            (void) port_iter_init(&pit, NULL, sid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
            while (port_iter_getnext(&pit)) {
                iport = pit.iport;

                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/%u/%u/%u|",
                              iport2uport(iport), ipmc_rports.ports[iport], fast_leave_ports.ports[iport], throttling_max_no.ports[iport]);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            }
        }

        (void)cyg_httpd_end_chunked();
    }
    return -1; // Do not further search the file system.
}

static cyg_int32 handler_config_ipmc_vlan(CYG_HTTPD_STATE *p)
{
    int                             ct = 0;
    BOOL                            vlan_state, vlan_state_tmp, vlan_querier, vlan_querier_tmp;
    vtss_vid_t                      vid = 0;
    char                            var_vlan[32];
    size_t                          nlen, nlen2;
    ipmc_ip_version_t               ipmc_version = IPMC_IP_VERSION_IGMP;
    const char                      *var_string;
    vtss_isid_t                     sid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    ipmc_prot_intf_entry_param_t    vid_entry, vid_entry_new;
#ifdef VTSS_SW_OPTION_SMB_IPMC
    int                             ipmc_smb = 1;
#else
    int                             ipmc_smb = 0;
#endif /* VTSS_SW_OPTION_SMB_IPMC */

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_IPMC)) {
        return -1;
    }
#endif

    if (p->method == CYG_HTTPD_METHOD_POST) {
        const char              *err_buf_ptr;
        vtss_rc                 rc;
        int                     cntr, var_value, errors = 0;
        ipmc_operation_action_t op;
        BOOL                    op_flag;

        (void) cyg_httpd_form_varable_int(p, "ipmc_version", &var_value);
        ipmc_version = var_value;

        /* Process Existed Entries First */
        for (cntr = 0; cntr < IPMC_VLAN_MAX; cntr++) {
            memset(var_vlan, 0x0, sizeof(var_vlan));
            sprintf(var_vlan, "mvid_ipmc_vlan_%d", cntr);
            if (!cyg_httpd_form_varable_int(p, var_vlan, &var_value) || !var_value) {
                continue;
            }

            op_flag = FALSE;
            vid = (vtss_vid_t)(var_value & 0xFFFF);
            if (ipmc_mgmt_get_intf_state_querier(TRUE, &vid, &vlan_state, &vlan_querier, FALSE, ipmc_version) == VTSS_OK) {
                op_flag = TRUE;
            }
            vid = (vtss_vid_t)(var_value & 0xFFFF);

            op = IPMC_OP_SET;
            memset(var_vlan, 0x0, sizeof(var_vlan));
            sprintf(var_vlan, "delete_ipmc_vlan_%u", vid);
            if (cyg_httpd_form_varable_find(p, var_vlan)) {
                /* "delete" checked */
                if (op_flag) {
                    rc = ipmc_mgmt_set_intf_state_querier(IPMC_OP_DEL, vid, &vlan_state_tmp, &vlan_querier_tmp, ipmc_version);
                    if (rc != VTSS_OK) {
                        err_buf_ptr = "Set IPMC Interface State/Querier Failed!!!";

                        send_custom_error(p, "IPMC Interface Configuration Error", err_buf_ptr, strlen(err_buf_ptr));
                        return -1;
                    }
                }

                continue;
            } else {
                if (op_flag) {
                    op = IPMC_OP_UPD;
                } else {
                    op = IPMC_OP_ADD;
                }
            }

            memset(var_vlan, 0x0, sizeof(var_vlan));
            sprintf(var_vlan, "vlan_mode_%d", vid);
            (void) cyg_httpd_form_varable_string(p, var_vlan, &nlen);
            memset(var_vlan, 0x0, sizeof(var_vlan));
            sprintf(var_vlan, "vlan_query_%d", vid);
            (void) cyg_httpd_form_varable_string(p, var_vlan, &nlen2);
            vlan_state_tmp = nlen ? TRUE : FALSE;
            vlan_querier_tmp = nlen2 ? TRUE : FALSE;

            op_flag = FALSE;
            rc = VTSS_RC_ERROR;
            if (op == IPMC_OP_ADD) {
                rc = ipmc_mgmt_set_intf_state_querier(IPMC_OP_ADD, vid, &vlan_state_tmp, &vlan_querier_tmp, ipmc_version);
                op_flag = TRUE;
            } else {
                if ((vlan_state_tmp != vlan_state) || (vlan_querier_tmp != vlan_querier)) {
                    rc = ipmc_mgmt_set_intf_state_querier(IPMC_OP_UPD, vid, &vlan_state_tmp, &vlan_querier_tmp, ipmc_version);
                    op_flag = TRUE;
                }
            }

            if (op_flag) {
                if (rc != VTSS_OK) {
                    if (rc == IPMC_ERROR_TABLE_IS_FULL) {
                        err_buf_ptr = "IPMC Interface State/Querier Table Full!!!";
                    } else {
                        err_buf_ptr = "Set IPMC Interface State/Querier Failed!!!";
                    }

                    send_custom_error(p, "IPMC Interface Configuration Error", err_buf_ptr, strlen(err_buf_ptr));
                    return -1;
                }

                VTSS_OS_MSLEEP(1000); // Wait for processing querier state
            }

            if (ipmc_mgmt_get_intf_info(sid, vid, &vid_entry, ipmc_version) == VTSS_OK) {
                memcpy(&vid_entry_new, &vid_entry, sizeof(ipmc_prot_intf_entry_param_t));

                if (ipmc_version == IPMC_IP_VERSION_IGMP) {
                    memset(var_vlan, 0x0, sizeof(var_vlan));
                    sprintf(var_vlan, "vlan_qradr_%d", vid);
                    if (!cyg_httpd_form_varable_ipv4(p, var_vlan, &vid_entry_new.querier.QuerierAdrs4)) {
                        err_buf_ptr = "Invalid Querier Address!!!";
                        send_custom_error(p, "IPMC Interface Configuration Error", err_buf_ptr, strlen(err_buf_ptr));
                        return -1;
                    }
                }

#ifdef VTSS_SW_OPTION_SMB_IPMC
                memset(var_vlan, 0x0, sizeof(var_vlan));
                sprintf(var_vlan, "vlan_compat_%d", vid);
                (void) cyg_httpd_form_varable_int(p, var_vlan, &var_value);
                vid_entry_new.cfg_compatibility = var_value;
                if ((ipmc_version == IPMC_IP_VERSION_MLD) &&
                    (vid_entry_new.cfg_compatibility == VTSS_IPMC_COMPAT_MODE_OLD)) {
                    err_buf_ptr = "Invalid MLD Compatibility!!!";
                    send_custom_error(p, "IPMC Interface Configuration Error", err_buf_ptr, strlen(err_buf_ptr));
                    return -1;
                }
                memset(var_vlan, 0x0, sizeof(var_vlan));
                sprintf(var_vlan, "vlan_pri_%d", vid);
                (void) cyg_httpd_form_varable_int(p, var_vlan, &var_value);
                vid_entry_new.priority = (var_value & IPMC_PARAM_PRIORITY_MASK);
                memset(var_vlan, 0x0, sizeof(var_vlan));
                sprintf(var_vlan, "vlan_rv_%d", vid);
                (void) cyg_httpd_form_varable_long_int(p, var_vlan, &vid_entry_new.querier.RobustVari);
                memset(var_vlan, 0x0, sizeof(var_vlan));
                sprintf(var_vlan, "vlan_qi_%d", vid);
                (void) cyg_httpd_form_varable_long_int(p, var_vlan, &vid_entry_new.querier.QueryIntvl);
                memset(var_vlan, 0x0, sizeof(var_vlan));
                sprintf(var_vlan, "vlan_qri_%d", vid);
                (void) cyg_httpd_form_varable_long_int(p, var_vlan, &vid_entry_new.querier.MaxResTime);
                memset(var_vlan, 0x0, sizeof(var_vlan));
                sprintf(var_vlan, "vlan_llqi_%d", vid);
                (void) cyg_httpd_form_varable_long_int(p, var_vlan, &vid_entry_new.querier.LastQryItv);
                memset(var_vlan, 0x0, sizeof(var_vlan));
                sprintf(var_vlan, "vlan_uri_%d", vid);
                (void) cyg_httpd_form_varable_long_int(p, var_vlan, &vid_entry_new.querier.UnsolicitR);
#endif /* VTSS_SW_OPTION_SMB_IPMC */

                if (memcmp(&vid_entry_new, &vid_entry, sizeof(ipmc_prot_intf_entry_param_t))) {
                    if (ipmc_mgmt_set_intf_info(sid, &vid_entry_new, ipmc_version) != VTSS_OK) {
                        err_buf_ptr = "Set IPMC Interface Parameter Failed!!!";
                        send_custom_error(p, "IPMC Interface Configuration Error", err_buf_ptr, strlen(err_buf_ptr));
                        return -1;
                    }
                }
            }
        }

        /* Process Created Entries Next */
        for (cntr = 1; cntr <= IPMC_VLAN_MAX; cntr++) {
            memset(var_vlan, 0x0, sizeof(var_vlan));
            sprintf(var_vlan, "new_mvid_ipmc_vlan_%d", cntr);
            if (!cyg_httpd_form_varable_int(p, var_vlan, &var_value) || !var_value) {
                continue;
            }

            op = IPMC_OP_ADD;
            vid = (vtss_vid_t)(var_value & 0xFFFF);
            if (ipmc_mgmt_get_intf_state_querier(TRUE, &vid, &vlan_state, &vlan_querier, FALSE, ipmc_version) == VTSS_OK) {
                op = IPMC_OP_UPD;
            }
            vid = (vtss_vid_t)(var_value & 0xFFFF);

            memset(var_vlan, 0x0, sizeof(var_vlan));
            sprintf(var_vlan, "new_vmode_ipmc_vlan_%d", cntr);
            (void) cyg_httpd_form_varable_string(p, var_vlan, &nlen);
            memset(var_vlan, 0x0, sizeof(var_vlan));
            sprintf(var_vlan, "new_vqry_ipmc_vlan_%d", cntr);
            (void) cyg_httpd_form_varable_string(p, var_vlan, &nlen2);
            vlan_state_tmp = nlen ? TRUE : FALSE;
            vlan_querier_tmp = nlen2 ? TRUE : FALSE;

            op_flag = FALSE;
            rc = VTSS_RC_ERROR;
            if (op == IPMC_OP_ADD) {
                rc = ipmc_mgmt_set_intf_state_querier(IPMC_OP_ADD, vid, &vlan_state_tmp, &vlan_querier_tmp, ipmc_version);
                op_flag = TRUE;
            } else {
                if ((vlan_state_tmp != vlan_state) || (vlan_querier_tmp != vlan_querier)) {
                    rc = ipmc_mgmt_set_intf_state_querier(IPMC_OP_UPD, vid, &vlan_state_tmp, &vlan_querier_tmp, ipmc_version);
                    op_flag = TRUE;
                }
            }

            if (op_flag) {
                if (rc != VTSS_OK) {
                    if (rc == IPMC_ERROR_TABLE_IS_FULL) {
                        err_buf_ptr = "IPMC Interface State/Querier Table Full!!!";
                    } else {
                        err_buf_ptr = "Set IPMC Interface State/Querier Failed!!!";
                    }

                    send_custom_error(p, "IPMC Interface Configuration Error", err_buf_ptr, strlen(err_buf_ptr));
                    return -1;
                }

                VTSS_OS_MSLEEP(1000); // Wait for processing querier state
            }

            if (ipmc_mgmt_get_intf_info(sid, vid, &vid_entry, ipmc_version) == VTSS_OK) {
                memcpy(&vid_entry_new, &vid_entry, sizeof(ipmc_prot_intf_entry_param_t));

                if (ipmc_version == IPMC_IP_VERSION_IGMP) {
                    memset(var_vlan, 0x0, sizeof(var_vlan));
                    sprintf(var_vlan, "new_vqradr_ipmc_vlan_%d", cntr);
                    if (!cyg_httpd_form_varable_ipv4(p, var_vlan, &vid_entry_new.querier.QuerierAdrs4)) {
                        err_buf_ptr = "Invalid Querier Address!!!";
                        send_custom_error(p, "IPMC Interface Configuration Error", err_buf_ptr, strlen(err_buf_ptr));
                        return -1;
                    }
                }

#ifdef VTSS_SW_OPTION_SMB_IPMC
                memset(var_vlan, 0x0, sizeof(var_vlan));
                sprintf(var_vlan, "new_vcompat_ipmc_vlan_%d", cntr);
                (void) cyg_httpd_form_varable_int(p, var_vlan, &var_value);
                vid_entry_new.cfg_compatibility = var_value;
                if ((ipmc_version == IPMC_IP_VERSION_MLD) &&
                    (vid_entry_new.cfg_compatibility == VTSS_IPMC_COMPAT_MODE_OLD)) {
                    err_buf_ptr = "Invalid MLD Compatibility!!!";
                    send_custom_error(p, "IPMC Interface Configuration Error", err_buf_ptr, strlen(err_buf_ptr));
                    return -1;
                }
                memset(var_vlan, 0x0, sizeof(var_vlan));
                sprintf(var_vlan, "new_vpri_ipmc_vlan_%d", cntr);
                (void) cyg_httpd_form_varable_int(p, var_vlan, &var_value);
                vid_entry_new.priority = (var_value & IPMC_PARAM_PRIORITY_MASK);

                memset(var_vlan, 0x0, sizeof(var_vlan));
                sprintf(var_vlan, "new_vrv_ipmc_vlan_%d", cntr);
                (void) cyg_httpd_form_varable_long_int(p, var_vlan, &vid_entry_new.querier.RobustVari);
                memset(var_vlan, 0x0, sizeof(var_vlan));
                sprintf(var_vlan, "new_vqi_ipmc_vlan_%d", cntr);
                (void) cyg_httpd_form_varable_long_int(p, var_vlan, &vid_entry_new.querier.QueryIntvl);
                memset(var_vlan, 0x0, sizeof(var_vlan));
                sprintf(var_vlan, "new_vqri_ipmc_vlan_%d", cntr);
                (void) cyg_httpd_form_varable_long_int(p, var_vlan, &vid_entry_new.querier.MaxResTime);
                memset(var_vlan, 0x0, sizeof(var_vlan));
                sprintf(var_vlan, "new_vllqi_ipmc_vlan_%d", cntr);
                (void) cyg_httpd_form_varable_long_int(p, var_vlan, &vid_entry_new.querier.LastQryItv);
                memset(var_vlan, 0x0, sizeof(var_vlan));
                sprintf(var_vlan, "new_vuri_ipmc_vlan_%d", cntr);
                (void) cyg_httpd_form_varable_long_int(p, var_vlan, &vid_entry_new.querier.UnsolicitR);
#endif /* VTSS_SW_OPTION_SMB_IPMC */

                if (memcmp(&vid_entry_new, &vid_entry, sizeof(vid_entry))) {
                    if (ipmc_mgmt_set_intf_info(sid, &vid_entry_new, ipmc_version) != VTSS_OK) {
                        err_buf_ptr = "Set IPMC Interface Parameter Failed!!!";
                        send_custom_error(p, "IPMC Interface Configuration Error", err_buf_ptr, strlen(err_buf_ptr));
                        return -1;
                    }
                }
            }
        }

        if (ipmc_version == IPMC_IP_VERSION_IGMP) {
            redirect(p, errors ? STACK_ERR_URL : "/ipmc_igmps_vlan.htm");
        } else {
            redirect(p, errors ? STACK_ERR_URL : "/ipmc_mldsnp_vlan.htm");
        }
    } else {
        int         entry_cnt, num_of_entries, dyn_get_next_entry;
        vtss_vid_t  start_vid;
        /* CYG_HTTPD_METHOD_GET (+HEAD) */

        // Get ipmc_version
        if ((var_string = cyg_httpd_form_varable_find(p, "ipmc_version")) != NULL) {
            ipmc_version = atoi(var_string);
        }

        // Format: [start_vid],[num_of_entries],[ipmc_smb],[vid]/[vlan_mode]/[vlan_query]|...
        start_vid = 1;
        entry_cnt = num_of_entries = dyn_get_next_entry = 0;
        (void)cyg_httpd_start_chunked("html");

        // Get start vid
        if ((var_string = cyg_httpd_form_varable_find(p, "DynStartVid")) != NULL) {
            start_vid = atoi(var_string);
        }

        // Get number of entries per page
        if ((var_string = cyg_httpd_form_varable_find(p, "DynNumberOfEntries")) != NULL) {
            num_of_entries = atoi(var_string);
        }
        if (num_of_entries <= 0 || num_of_entries > IPMC_NO_OF_SUPPORTED_SRCLIST) {
            num_of_entries = 20;
        }

        // Get or GetNext
        if ((var_string = cyg_httpd_form_varable_find(p, "GetNextEntry")) != NULL) {
            dyn_get_next_entry = atoi(var_string);
        }
        if (dyn_get_next_entry && start_vid < 4095) {
            start_vid++;
        }

        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d,%d,%d,",
                      start_vid, num_of_entries, ipmc_smb);
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);

        while (ipmc_mgmt_get_intf_state_querier(TRUE, &vid, &vlan_state, &vlan_querier, TRUE, ipmc_version) == VTSS_OK) {
            if (vid < start_vid) {
                continue;
            }
            if (entry_cnt < num_of_entries) {
                entry_cnt++;
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/%u/%u",
                              vid, vlan_state, vlan_querier);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                if (ipmc_mgmt_get_intf_info(sid, vid, &vid_entry, ipmc_version) == VTSS_OK) {
                    if (ipmc_version == IPMC_IP_VERSION_IGMP) {
                        i8  ip_buf[IPV6_ADDR_IBUF_MAX_LEN];

                        memset(ip_buf, 0x0, sizeof(ip_buf));
                        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%s",
                                      misc_ipv4_txt(vid_entry.querier.QuerierAdrs4, ip_buf));
                        (void)cyg_httpd_write_chunked(p->outbuffer, ct);
                    }

#ifdef VTSS_SW_OPTION_SMB_IPMC
                    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%d/%u/%u/%u/%u/%u/%u",
                                  vid_entry.cfg_compatibility,
                                  vid_entry.querier.RobustVari,
                                  vid_entry.querier.QueryIntvl,
                                  vid_entry.querier.MaxResTime,
                                  vid_entry.querier.LastQryItv,
                                  vid_entry.querier.UnsolicitR,
                                  vid_entry.priority);
                    (void)cyg_httpd_write_chunked(p->outbuffer, ct);
#endif /* #ifdef VTSS_SW_OPTION_SMB_IPMC */
                } else {
                    if (ipmc_version == IPMC_IP_VERSION_IGMP) {
                        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/0.0.0.0");
                        (void)cyg_httpd_write_chunked(p->outbuffer, ct);
                    }
                }

                (void)cyg_httpd_write_chunked("|", 1);
            } else {
                break;
            }
        }

        if (entry_cnt == 0) { /* No entry existing */
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "NoEntries");
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        }

        (void)cyg_httpd_end_chunked();
    }
    return -1; // Do not further search the file system.
}

#ifdef VTSS_SW_OPTION_SMB_IPMC
static cyg_int32 handler_config_ipmc_filtering(CYG_HTTPD_STATE *p)
{
    vtss_isid_t                         sid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    port_iter_t                         pit;
    vtss_port_no_t                      iport;
    ipmc_ip_version_t                   ipmc_version = IPMC_IP_VERSION_IGMP;
    const char                          *var_string;
    ipmc_conf_port_group_filtering_t    ipmc_web_pg_filtering;
    ipmc_lib_profile_mem_t              *pfm;
    ipmc_lib_grp_fltr_profile_t         *fltr_profile;

    /* Redirect unmanaged/invalid access to handler */
    if (!p || redirectUnmanagedOrInvalid(p, sid) ||
        !IPMC_MEM_PROFILE_MTAKE(pfm)) {
        return -1;
    }

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_IPMC)) {
        IPMC_MEM_PROFILE_MGIVE(pfm);
        return -1;
    }
#endif

    fltr_profile = &pfm->profile;
    if (p->method == CYG_HTTPD_METHOD_POST) {
        char    str_name[VTSS_IPMC_NAME_MAX_LEN], search_str[65];;
        int     var_value, str_len, errors = 0;
        size_t  var_len;

        (void) cyg_httpd_form_varable_int(p, "ipmc_version", &var_value);
        ipmc_version = var_value;
        (void) port_iter_init(&pit, NULL, sid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
        while (port_iter_getnext(&pit)) {
            iport = pit.iport;

            memset(search_str, 0x0, sizeof(search_str));
            sprintf(search_str, "ref_port_filter_profile_%u", pit.uport);
            var_string = cyg_httpd_form_varable_string(p, search_str, &var_len);
            memset(str_name, 0x0, sizeof(str_name));
            if (var_len > 0) {
                (void) cgi_unescape(var_string, str_name, var_len, sizeof(str_name));
            }
            str_len = strlen(str_name);

            if (str_len) {
                memset(fltr_profile, 0x0, sizeof(ipmc_lib_grp_fltr_profile_t));
                strncpy(fltr_profile->data.name, str_name, str_len);
                if (ipmc_lib_mgmt_fltr_profile_get(fltr_profile, TRUE) != VTSS_OK) {
                    continue;
                }
            }

            memset(&ipmc_web_pg_filtering, 0x0, sizeof(ipmc_conf_port_group_filtering_t));
            ipmc_web_pg_filtering.port_no = iport;
            if (ipmc_mgmt_get_port_group_filtering(sid, &ipmc_web_pg_filtering, ipmc_version) == VTSS_OK) {
                if (str_len) {
                    memset(ipmc_web_pg_filtering.addr.profile.name, 0x0, sizeof(ipmc_web_pg_filtering.addr.profile.name));
                    strncpy(ipmc_web_pg_filtering.addr.profile.name, str_name, str_len);
                    (void) ipmc_mgmt_set_port_group_filtering(sid, &ipmc_web_pg_filtering, ipmc_version);
                } else {
                    (void) ipmc_mgmt_del_port_group_filtering(sid, &ipmc_web_pg_filtering, ipmc_version);
                }
            } else {
                if (str_len) {
                    memset(&ipmc_web_pg_filtering, 0x0, sizeof(ipmc_conf_port_group_filtering_t));
                    ipmc_web_pg_filtering.port_no = iport;
                    strncpy(ipmc_web_pg_filtering.addr.profile.name, str_name, str_len);
                    (void) ipmc_mgmt_set_port_group_filtering(sid, &ipmc_web_pg_filtering, ipmc_version);
                }
            }
        }

        if (ipmc_version == IPMC_IP_VERSION_IGMP) {
            redirect(p, errors ? STACK_ERR_URL : "/ipmc_igmps_filtering.htm");
        } else {
            redirect(p, errors ? STACK_ERR_URL : "/ipmc_mldsnp_filtering.htm");
        }
    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        int                 ct, cntr;
        ipmc_lib_profile_t  *data;
        char                encoded_string[3 * VTSS_IPMC_NAME_MAX_LEN];

        // Get ipmc_version
        if ((var_string = cyg_httpd_form_varable_find(p, "ipmc_version")) != NULL) {
            ipmc_version = atoi(var_string);
        }

        (void)cyg_httpd_start_chunked("html");

        // Format: [profile_1]|...|[profile_n];[profile]|...;

        cntr = 0;
        memset(fltr_profile, 0x0, sizeof(ipmc_lib_grp_fltr_profile_t));
        while (ipmc_lib_mgmt_fltr_profile_get_next(fltr_profile, TRUE) == VTSS_OK) {
            data = &fltr_profile->data;
            memset(encoded_string, 0x0, sizeof(encoded_string));
            ct = cgi_escape(data->name, encoded_string);

            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s%s",
                          cntr ? "|" : "",
                          encoded_string);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);

            cntr++;
        }

        cntr = 0;
        (void) port_iter_init(&pit, NULL, sid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
        while (port_iter_getnext(&pit)) {
            iport = pit.iport;

            if (port_isid_port_no_is_stack(sid, iport)) {
                continue;
            }

            memset(&ipmc_web_pg_filtering, 0x0, sizeof(ipmc_conf_port_group_filtering_t));
            ipmc_web_pg_filtering.port_no = iport;
            if (ipmc_mgmt_get_port_group_filtering(sid, &ipmc_web_pg_filtering, ipmc_version) == VTSS_OK) {
                memset(encoded_string, 0x0, sizeof(encoded_string));
                ct = cgi_escape(ipmc_web_pg_filtering.addr.profile.name, encoded_string);

                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s%s",
                              cntr ? "|" : ";",
                              encoded_string);
            } else {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s", cntr ? "|" : ";");
            }
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);

            cntr++;
        }

        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), ";");
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        (void)cyg_httpd_end_chunked();
    }

    IPMC_MEM_PROFILE_MGIVE(pfm);
    return -1; // Do not further search the file system.
}
#endif /* VTSS_SW_OPTION_SMB_IPMC */

static cyg_int32 handler_stat_ipmc_status(CYG_HTTPD_STATE *p)
{
    vtss_isid_t                     sid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    port_iter_t                     pit;
    vtss_port_no_t                  iport;
    int                             ct = 0;
    uchar                           vlan_state, vlan_querier;
    ushort                          vid;
    ipmc_prot_intf_entry_param_t    vid_entry;
    ipmc_conf_router_port_t         ipmc_rports;
    ipmc_dynamic_router_port_t      ipmc_drports;
    int                             rport_status;
    ipmc_intf_query_host_version_t  vlan_version_entry;
    ipmc_ip_version_t               ipmc_version = 1;
    BOOL                            ipmc_mode;
    const char                      *var_string;

    if (redirectUnmanagedOrInvalid(p, sid)) { /* Redirect unmanaged/invalid access to handler */
        return -1;
    }

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_STAT_TYPE, VTSS_MODULE_ID_IPMC)) {
        return -1;
    }
#endif

    // Get ipmc_version
    if ((var_string = cyg_httpd_form_varable_find(p, "ipmc_version")) != NULL) {
        ipmc_version = atoi(var_string);
    }

    /* get form data
       Format: [vid],[querier_ver],[host_ver],[querier_status],[querier_transmitted],[received_v1_reports],[received_v2_reports],[received_v1_leave]
               | [port_no],[status]/[port_no],[status]/...
       status 0: None     1: Static      2: Dynamic      3: Both
    */
    (void)cyg_httpd_start_chunked("html");

    if ((cyg_httpd_form_varable_find(p, "clear") != NULL)) { /* Clear? */
        if (ipmc_mgmt_clear_stat_counter(sid, ipmc_version, VTSS_IPMC_VID_ALL) != VTSS_OK) {
            T_W("Clear %s Snooping counters fail", ipmc_lib_version_txt(ipmc_version, IPMC_TXT_CASE_UPPER));
        } else {
            /* Wait for processing statistics */
            VTSS_OS_MSLEEP(100);
        }
    }

    /* Normal read */
    if (ipmc_mgmt_get_mode(&ipmc_mode, ipmc_version) != VTSS_OK) {
        ipmc_mode = FALSE;
    }
    vid = 0;
    while (ipmc_mgmt_get_intf_state_querier(FALSE, &vid, &vlan_state, &vlan_querier, TRUE, ipmc_version) == VTSS_OK) {
        /* Don't show disabled interfaces, since no meaningful running info */
        if (!vlan_state) {
            continue;
        }

        memset(&vid_entry, 0x0, sizeof(ipmc_prot_intf_entry_param_t));
        if (ipmc_mgmt_get_intf_info(sid, vid, &vid_entry, ipmc_version) != VTSS_OK) {
            continue;
        }
        vlan_version_entry.vid = vid;
        if (ipmc_mgmt_get_intf_version(sid, &vlan_version_entry, ipmc_version) != VTSS_OK) {
            vlan_version_entry.query_version = VTSS_IPMC_VERSION_DEFAULT;
            vlan_version_entry.host_version = VTSS_IPMC_VERSION_DEFAULT;
        }
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u,", vid);
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        if (vlan_version_entry.query_version == VTSS_IPMC_VERSION_DEFAULT) {
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s,", ipmc_version == IPMC_IP_VERSION_IGMP ? "v3" : "v2");
        } else {
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "v%u,",
                          ipmc_version == IPMC_IP_VERSION_IGMP ? vlan_version_entry.query_version : vlan_version_entry.query_version - VTSS_IPMC_VERSION1);
        }
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        if (vlan_version_entry.host_version == VTSS_IPMC_VERSION_DEFAULT) {
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s,", ipmc_version == IPMC_IP_VERSION_IGMP ? "v3" : "v2");
        } else {
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "v%u,",
                          ipmc_version == IPMC_IP_VERSION_IGMP ? vlan_version_entry.host_version : vlan_version_entry.host_version - VTSS_IPMC_VERSION1);
        }
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        if (ipmc_mode && vlan_state) {
            if (vid_entry.querier.state != IPMC_QUERIER_IDLE) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u,%u,",
                              IPMC_QUERIER_ACTIVE,
                              vid_entry.querier.ipmc_queries_sent);
            } else {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u,%u,",
                              IPMC_QUERIER_IDLE,
                              vid_entry.querier.ipmc_queries_sent);
            }
        } else {
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u,%u,",
                          IPMC_QUERIER_INIT,
                          vid_entry.querier.ipmc_queries_sent);
        }
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        if (ipmc_version == IPMC_IP_VERSION_IGMP) {
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u,%u,%u,%u,%u",
                          vid_entry.stats.igmp_queries,
                          vid_entry.stats.igmp_v1_membership_join,
                          vid_entry.stats.igmp_v2_membership_join,
                          vid_entry.stats.igmp_v3_membership_join,
                          vid_entry.stats.igmp_v2_membership_leave);
        } else {
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u,%u,%u,%u",
                          vid_entry.stats.mld_queries,
                          vid_entry.stats.mld_v1_membership_report,
                          vid_entry.stats.mld_v2_membership_report,
                          vid_entry.stats.mld_v1_membership_done);
        }
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);

        (void)cyg_httpd_write_chunked("/", 1);
    }

    (void)cyg_httpd_write_chunked("|", 1);
    (void) ipmc_mgmt_get_static_router_port(sid, &ipmc_rports, ipmc_version);
    (void) ipmc_mgmt_get_dynamic_router_ports(sid, &ipmc_drports, ipmc_version);
    (void) port_iter_init(&pit, NULL, sid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
    while (port_iter_getnext(&pit)) {
        iport = pit.iport;

        if ((ipmc_rports.ports[iport] == 0) && (ipmc_drports.ports[iport] == 0)) {
            rport_status = 0;
        } else if ((ipmc_rports.ports[iport] == 1) && (ipmc_drports.ports[iport] == 0)) {
            rport_status = 1;
        } else if ((ipmc_rports.ports[iport] == 0) && (ipmc_drports.ports[iport] == 1)) {
            rport_status = 2;
        } else if ((ipmc_rports.ports[iport] == 1) && (ipmc_drports.ports[iport] == 1)) {
            rport_status = 3;
        } else {
            rport_status = 0;
        }
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u,%d", iport2uport(iport), rport_status);
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        (void)cyg_httpd_write_chunked("/", 1);
    }
    (void)cyg_httpd_end_chunked();

    return -1; // Do not further search the file system.
}

#ifdef VTSS_SW_OPTION_SMB_IPMC
static cyg_int32 handler_stat_ipmc_sfm_info(CYG_HTTPD_STATE *p)
{
    vtss_isid_t                         sid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    int                                 ct = 0;
    ulong                               num_of_entries = 0;
    int                                 dyn_get_next_entry = 0;
    ipmc_prot_intf_group_entry_t        intf_group;
    ipmc_prot_group_srclist_t           group_srclist_entry;
    uchar                               dummy;
    ushort                              vid = 0, start_vid = 1;
    int                                 mode;
    port_iter_t                         pit;
    ulong                               i, print_cnt = 0;
    ipmc_sfm_srclist_t                  v3_src;
    char                                ip_buf1[IPV6_ADDR_IBUF_MAX_LEN];
    char                                ip_buf2[IPV6_ADDR_IBUF_MAX_LEN];
    vtss_ipv4_t                         start_group_addr = 0;
    vtss_ipv6_t                         start_group_addr_ipv6;
    BOOL                                amir, smir, has_do_not_fwd_src = FALSE;
    int                                 var_value;
    ipmc_ip_version_t                   ipmc_version = 1;
    const char                          *var_string;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_STAT_TYPE, VTSS_MODULE_ID_IPMC)) {
        return -1;
    }
#endif

    if (p->method == CYG_HTTPD_METHOD_POST) {
        (void) cyg_httpd_form_varable_int(p, "ipmc_version", &var_value);
        ipmc_version = var_value;
        if (ipmc_version == IPMC_IP_VERSION_IGMP) {
            redirect(p, "/ipmc_igmps_v2info.htm");
        } else {
            redirect(p, "/ipmc_mldsnp_v2info.htm");
        }
    } else {
        // Get ipmc_version
        if ((var_string = cyg_httpd_form_varable_find(p, "ipmc_version")) != NULL) {
            ipmc_version = atoi(var_string);
        }

        (void)cyg_httpd_start_chunked("html");

        //Format: <start_vid>/<start_group>/<num_of_entries>|[vid]/[group]/[port]/[mode]/[source_addr]/[type]/[in_hw]|...

        // Get number of entries per page
        if ((var_string = cyg_httpd_form_varable_find(p, "DynNumberOfEntries")) != NULL) {
            num_of_entries = atoi(var_string);
        }
        if (num_of_entries <= 0 || num_of_entries > 99) {
            num_of_entries = 20;
        }

        // Get start vid
        if ((var_string = cyg_httpd_form_varable_find(p, "DynStartVid")) != NULL) {
            start_vid = atoi(var_string);
        }

        // Get start group address
        memset(&start_group_addr_ipv6, 0x0, sizeof(vtss_ipv6_t));
        var_string = cyg_httpd_form_varable_find(p, "DynStartGroup");
        if (var_string != NULL) {
            if (ipmc_version == IPMC_IP_VERSION_IGMP) {
                (void) cyg_httpd_form_varable_ipv4(p, "DynStartGroup", &start_group_addr);
            } else {
                (void) cyg_httpd_form_varable_ipv6(p, "DynStartGroup", &start_group_addr_ipv6);
            }
        }

        // Get or GetNext
        if ((var_string = cyg_httpd_form_varable_find(p, "GetNextEntry")) != NULL) {
            dyn_get_next_entry = atoi(var_string);
        }

        if (!dyn_get_next_entry) { /* Not get next button */
            memset(&ip_buf1[0], 0x0, sizeof(ip_buf1));
            if (ipmc_version == IPMC_IP_VERSION_IGMP) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d/%s/%d|",
                              start_vid,
                              misc_ipv4_txt(start_group_addr, ip_buf1),
                              num_of_entries);
            } else {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d/%s/%d|",
                              start_vid,
                              misc_ipv6_txt(&start_group_addr_ipv6, ip_buf1),
                              num_of_entries);
            }
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        }

        // start to getting data
        while (ipmc_mgmt_get_intf_state_querier(FALSE, &vid, &dummy, &dummy, TRUE, ipmc_version) == VTSS_OK) {
            if (vid < start_vid) {
                continue;
            }

            memset(&intf_group, 0x0, sizeof(ipmc_prot_intf_group_entry_t));
            intf_group.vid = vid;
            if (ipmc_version == IPMC_IP_VERSION_IGMP) {
                if (dyn_get_next_entry) {
                    var_value = ntohl(start_group_addr);
                } else {
                    var_value = ntohl(start_group_addr - 1);
                }
                memcpy(&intf_group.group_addr.addr[12], &var_value, sizeof(start_group_addr));
            } else {
                memcpy(&intf_group.group_addr, &start_group_addr_ipv6, sizeof(start_group_addr_ipv6));
                if (!dyn_get_next_entry) {
                    i = 16;
                    do {
                        i--;
                        intf_group.group_addr.addr[i] -= 1;
                    } while (intf_group.group_addr.addr[i] == 0xFF && i != 0);
                }
            }

            while (ipmc_mgmt_get_next_intf_group_info(sid, vid, &intf_group, ipmc_version) == VTSS_OK) {
                if (++print_cnt > num_of_entries) {
                    break;
                }

                if (dyn_get_next_entry && (print_cnt == 1)) { /* Only for GetNext button */
                    memset(&ip_buf1[0], 0x0, sizeof(ip_buf1));
                    if (ipmc_version == IPMC_IP_VERSION_IGMP) {
                        ulong group_addr;
                        memcpy((uchar *)&group_addr, &intf_group.group_addr.addr[12], sizeof(ulong));
                        group_addr = htonl(group_addr);
                        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d/%s/%d|",
                                      vid,
                                      misc_ipv4_txt(group_addr, ip_buf1),
                                      num_of_entries);
                    } else {
                        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d/%s/%d|",
                                      vid,
                                      misc_ipv6_txt(&intf_group.group_addr, ip_buf1),
                                      num_of_entries);
                    }
                    (void)cyg_httpd_write_chunked(p->outbuffer, ct);
                }

                amir = intf_group.db.asm_in_hw;
                (void) port_iter_init(&pit, NULL, sid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
                while (port_iter_getnext(&pit)) {
                    i = pit.iport;
                    /* we hide stacking port that we get the same result from iCLI */
                    if (port_isid_port_no_is_stack(sid, i)) {
                        continue;
                    }

                    if (IPMC_LIB_GRP_PORT_DO_SFM(&intf_group.db, i)) {
                        if (IPMC_LIB_GRP_PORT_SFM_EX(&intf_group.db, i)) {
                            mode = 0;
                            has_do_not_fwd_src = FALSE;
                            memset(&group_srclist_entry, 0x0, sizeof(ipmc_prot_group_srclist_t));
                            group_srclist_entry.type = FALSE;
                            while (ipmc_mgmt_get_next_group_srclist_info(
                                       sid, ipmc_version, vid,
                                       &intf_group.group_addr,
                                       &group_srclist_entry) == VTSS_OK) {
                                if (!group_srclist_entry.cntr) {
                                    break;
                                }

                                memcpy(&v3_src, &group_srclist_entry.srclist, sizeof(ipmc_sfm_srclist_t));
                                if (VTSS_PORT_BF_GET(v3_src.port_mask, i) == FALSE) {
                                    continue;
                                }

                                if (has_do_not_fwd_src == FALSE) {
                                    has_do_not_fwd_src = TRUE;
                                }

                                if ((dyn_get_next_entry && memcmp(&intf_group.group_addr, &start_group_addr_ipv6, sizeof(start_group_addr_ipv6)) > 0) || (!dyn_get_next_entry)) {
                                    memset(&ip_buf1[0], 0x0, sizeof(ip_buf1));
                                    memset(&ip_buf2[0], 0x0, sizeof(ip_buf2));
                                    smir = v3_src.sfm_in_hw;
                                    if (ipmc_version == IPMC_IP_VERSION_IGMP) {
                                        ulong group_addr, src_addr;
                                        memcpy((uchar *)&group_addr, &intf_group.group_addr.addr[12], sizeof(ulong));
                                        group_addr = htonl(group_addr);
                                        memcpy((uchar *)&src_addr, &v3_src.src_ip.addr[12], sizeof(ulong));
                                        src_addr = htonl(src_addr);
                                        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "|%d/%s/%d/%d/%s/0/%u|",
                                                      vid,
                                                      misc_ipv4_txt(group_addr, ip_buf1),
                                                      iport2uport(i),
                                                      mode,
                                                      misc_ipv4_txt(src_addr, ip_buf2),
                                                      (amir && smir));
                                    } else {
                                        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "|%d/%s/%d/%d/%s/0/%u|",
                                                      vid,
                                                      misc_ipv6_txt(&intf_group.group_addr, ip_buf1),
                                                      iport2uport(i),
                                                      mode,
                                                      misc_ipv6_txt(&v3_src.src_ip, ip_buf2),
                                                      (amir && smir));
                                    }
                                    (void)cyg_httpd_write_chunked(p->outbuffer, ct);
                                }
                            }
                            if (has_do_not_fwd_src == FALSE) { /* ???->Original logic is strange --- Charles */
                                if (IPMC_LIB_GRP_PORT_SFM_EX(&intf_group.db, i)) {
                                    memset(&ip_buf1[0], 0x0, sizeof(ip_buf1));
                                    if (ipmc_version == IPMC_IP_VERSION_IGMP) {
                                        ulong group_addr;
                                        memcpy((uchar *)&group_addr, &intf_group.group_addr.addr[12], sizeof(ulong));
                                        group_addr = htonl(group_addr);
                                        if ((dyn_get_next_entry && group_addr > start_group_addr) || (!dyn_get_next_entry)) {
                                            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d/%s/%d/%d/None/0/%u|",
                                                          vid,
                                                          misc_ipv4_txt(group_addr, ip_buf1),
                                                          iport2uport(i),
                                                          mode,
                                                          amir);
                                            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
                                        }
                                    } else {
                                        if ((dyn_get_next_entry && memcmp(&intf_group.group_addr, &start_group_addr_ipv6, sizeof(start_group_addr_ipv6)) > 0) || (!dyn_get_next_entry)) {
                                            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d/%s/%d/%d/None/0/%u|",
                                                          vid,
                                                          misc_ipv6_txt(&intf_group.group_addr, ip_buf1),
                                                          iport2uport(i),
                                                          mode,
                                                          amir);
                                            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
                                        }
                                    }
                                }
                            }
                        } else {
                            mode = 1;
                            memset(&group_srclist_entry, 0x0, sizeof(ipmc_prot_group_srclist_t));
                            group_srclist_entry.type = TRUE;
                            while (ipmc_mgmt_get_next_group_srclist_info(
                                       sid, ipmc_version, vid,
                                       &intf_group.group_addr,
                                       &group_srclist_entry) == VTSS_OK) {
                                if (!group_srclist_entry.cntr) {
                                    break;
                                }

                                memcpy(&v3_src, &group_srclist_entry.srclist, sizeof(ipmc_sfm_srclist_t));
                                if (VTSS_PORT_BF_GET(v3_src.port_mask, i) == FALSE) {
                                    continue;
                                }

                                if (ipmc_version == IPMC_IP_VERSION_IGMP) {
                                    var_value = (memcmp(&intf_group.group_addr.addr[12], &start_group_addr, sizeof(start_group_addr)) > 0);
                                } else {
                                    var_value = (memcmp(&intf_group.group_addr, &start_group_addr_ipv6, sizeof(start_group_addr_ipv6)) > 0);
                                }
                                if ((dyn_get_next_entry && var_value) || (!dyn_get_next_entry)) {
                                    memset(&ip_buf1[0], 0x0, sizeof(ip_buf1));
                                    memset(&ip_buf2[0], 0x0, sizeof(ip_buf2));
                                    smir = v3_src.sfm_in_hw;
                                    if (ipmc_version == IPMC_IP_VERSION_IGMP) {
                                        ulong group_addr, src_addr;
                                        memcpy((uchar *)&group_addr, &intf_group.group_addr.addr[12], sizeof(ulong));
                                        group_addr = htonl(group_addr);
                                        memcpy((uchar *)&src_addr, &v3_src.src_ip.addr[12], sizeof(ulong));
                                        src_addr = htonl(src_addr);
                                        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d/%s/%d/%d/%s/1/%u|",
                                                      vid,
                                                      misc_ipv4_txt(group_addr, ip_buf1),
                                                      iport2uport(i),
                                                      mode,
                                                      misc_ipv4_txt(src_addr, ip_buf2),
                                                      (amir && smir));
                                    } else {
                                        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d/%s/%d/%d/%s/1/%u|",
                                                      vid,
                                                      misc_ipv6_txt(&intf_group.group_addr, ip_buf1),
                                                      iport2uport(i),
                                                      mode,
                                                      misc_ipv6_txt(&v3_src.src_ip, ip_buf2),
                                                      (amir && smir));
                                    }
                                    (void)cyg_httpd_write_chunked(p->outbuffer, ct);
                                }
                            }
                        }
                    }
                }
            }

            if (print_cnt > num_of_entries) {
                break;
            }
        }

        if (print_cnt == 0) { /* No entry existing */
            if (dyn_get_next_entry) {
                memset(&ip_buf1[0], 0x0, sizeof(ip_buf1));
                if (ipmc_version == IPMC_IP_VERSION_IGMP) {
                    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d/%s/%d|",
                                  start_vid,
                                  misc_ipv4_txt(start_group_addr, ip_buf1),
                                  num_of_entries);
                } else {
                    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d/%s/%d|",
                                  start_vid,
                                  misc_ipv6_txt(&start_group_addr_ipv6, ip_buf1),
                                  num_of_entries);
                }
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            }
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "NoEntries");
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        }

        (void)cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}
#endif /* VTSS_SW_OPTION_SMB_IPMC */

static cyg_int32 handler_stat_ipmc_groups_info(CYG_HTTPD_STATE *p)
{
    vtss_isid_t                         sid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    int                                 ct = 0;
    ipmc_prot_intf_group_entry_t        vid_entry_group;
    char                                ip_buf[IPV6_ADDR_IBUF_MAX_LEN];
    uchar                               dummy;
    ushort                              vid = 0, start_vid = 1;
    int                                 entry_cnt = 0;
    int                                 num_of_entries = 0;
    u32                                 portCnt;
    vtss_ipv4_t                         start_group_addr = 0;
    vtss_ipv6_t                         start_group_addr_ipv6;
    port_iter_t                         pit;
    BOOL                                port_selected;
    int                                 dyn_get_next_entry = 0, i;
    int                                 var_value;
    ipmc_ip_version_t                   ipmc_version = 1;
    const char                          *var_string;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_STAT_TYPE, VTSS_MODULE_ID_IPMC)) {
        return -1;
    }
#endif

    if (p->method == CYG_HTTPD_METHOD_POST) {
        (void) cyg_httpd_form_varable_int(p, "ipmc_version", &var_value);
        ipmc_version = var_value;
        if (ipmc_version == IPMC_IP_VERSION_IGMP) {
            redirect(p, "/ipmc_igmps_groups_info.htm");
        } else {
            redirect(p, "/ipmc_mldsnp_groups_info.htm");
        }
    } else { /* CYG_HTTPD_METHOD_GET (+HEAD) */
        // Get ipmc_version
        if ((var_string = cyg_httpd_form_varable_find(p, "ipmc_version")) != NULL) {
            ipmc_version = atoi(var_string);
        }

        (void)cyg_httpd_start_chunked("html");

        // Get number of entries per page
        if ((var_string = cyg_httpd_form_varable_find(p, "DynNumberOfEntries")) != NULL) {
            num_of_entries = atoi(var_string);
        }
        if (num_of_entries <= 0 || num_of_entries > 99) {
            num_of_entries = 20;
        }

        // Get start vid
        if ((var_string = cyg_httpd_form_varable_find(p, "DynStartVid")) != NULL) {
            start_vid = atoi(var_string);
        }

        // Get start group address
        memset(&start_group_addr_ipv6, 0, sizeof(start_group_addr_ipv6));
        if (cyg_httpd_form_varable_find(p, "DynStartGroup") != NULL) {
            if (ipmc_version == IPMC_IP_VERSION_IGMP) {
                (void) cyg_httpd_form_varable_ipv4(p, "DynStartGroup", &start_group_addr);
            } else {
                (void) cyg_httpd_form_varable_ipv6(p, "DynStartGroup", &start_group_addr_ipv6);
            }
        }

        // Get or GetNext
        if ((var_string = cyg_httpd_form_varable_find(p, "GetNextEntry")) != NULL) {
            dyn_get_next_entry = atoi(var_string);
        }

        /*
            Format:
            <startVid>|<startGroup>|<NumberOfEntries>|<portCnt>|[vid],[groups],[port1_is_member],[port2_is_member],[port3_is_member],.../...
        */
        portCnt = 0;
        (void) port_iter_init(&pit, NULL, sid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
        while (port_iter_getnext(&pit)) {
            ++portCnt;
        }
        memset(&ip_buf[0], 0x0, sizeof(ip_buf));
        if (!dyn_get_next_entry) { /* Not get next button */
            if (ipmc_version == IPMC_IP_VERSION_IGMP) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d|%s|%d|%u|",
                              start_vid,
                              misc_ipv4_txt(start_group_addr, ip_buf),
                              num_of_entries,
                              portCnt);
            } else {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d|%s|%d|%u|",
                              start_vid,
                              misc_ipv6_txt(&start_group_addr_ipv6, ip_buf),
                              num_of_entries,
                              portCnt);
            }
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        }

        while (ipmc_mgmt_get_intf_state_querier(FALSE, &vid, &dummy, &dummy, TRUE, ipmc_version) == VTSS_OK) {
            if (vid < start_vid) {
                continue;
            }
            memset(&vid_entry_group, 0, sizeof(vid_entry_group));
            vid_entry_group.vid = start_vid;
            if (ipmc_version == IPMC_IP_VERSION_IGMP) {
                if (dyn_get_next_entry) {
                    var_value = ntohl(start_group_addr);
                } else {
                    var_value = ntohl(start_group_addr - 1);
                }
                memcpy(&vid_entry_group.group_addr.addr[12], &var_value, sizeof(start_group_addr));
            } else {
                memcpy(&vid_entry_group.group_addr, &start_group_addr_ipv6, sizeof(start_group_addr_ipv6));
                if (!dyn_get_next_entry) {
                    i = 16;
                    do {
                        i--;
                        vid_entry_group.group_addr.addr[i] -= 1;
                    } while (vid_entry_group.group_addr.addr[i] == 0xFF && i != 0);
                }
            }
            while (ipmc_mgmt_get_next_intf_group_info(sid, vid, &vid_entry_group, ipmc_version) == VTSS_OK) {
                if (++entry_cnt > num_of_entries) {
                    break;
                }

                if (dyn_get_next_entry && (entry_cnt == 1)) { /* Only for GetNext button */
                    memset(&ip_buf[0], 0x0, sizeof(ip_buf));
                    if (ipmc_version == IPMC_IP_VERSION_IGMP) {
                        ulong group_addr;
                        memcpy((uchar *)&group_addr, &vid_entry_group.group_addr.addr[12], sizeof(ulong));
                        group_addr = htonl(group_addr);
                        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d|%s|%d|%u|",
                                      vid,
                                      misc_ipv4_txt(group_addr, ip_buf),
                                      num_of_entries,
                                      portCnt);
                    } else {
                        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d|%s|%d|%u|",
                                      vid,
                                      misc_ipv6_txt(&vid_entry_group.group_addr, ip_buf),
                                      num_of_entries,
                                      portCnt);
                    }
                    (void)cyg_httpd_write_chunked(p->outbuffer, ct);
                }

                memset(&ip_buf[0], 0x0, sizeof(ip_buf));
                if (ipmc_version == IPMC_IP_VERSION_IGMP) {
                    ulong group_addr;
                    memcpy((uchar *)&group_addr, &vid_entry_group.group_addr.addr[12], sizeof(ulong));
                    group_addr = htonl(group_addr);
                    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u,%s",
                                  vid,
                                  misc_ipv4_txt(group_addr, ip_buf));
                } else {
                    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u,%s",
                                  vid,
                                  misc_ipv6_txt(&vid_entry_group.group_addr, ip_buf));
                }
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                (void) port_iter_init(&pit, NULL, sid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
                while (port_iter_getnext(&pit)) {
                    /* we hide stacking port that we get the same result from iCLI */
                    if (port_isid_port_no_is_stack(sid, pit.iport)) {
                        port_selected = FALSE;
                    } else {
                        port_selected = VTSS_PORT_BF_GET(vid_entry_group.db.port_mask, pit.iport);
                    }
                    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), ",%u", port_selected);
                    (void)cyg_httpd_write_chunked(p->outbuffer, ct);
                }
                (void)cyg_httpd_write_chunked("/", 1);
            }

            if (entry_cnt > num_of_entries) {
                break;
            }
        }

        if (entry_cnt == 0) { /* No entry existing */
            if (dyn_get_next_entry) {
                memset(&ip_buf[0], 0x0, sizeof(ip_buf));
                if (ipmc_version == IPMC_IP_VERSION_IGMP) {
                    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d|%s|%d|%u|",
                                  start_vid,
                                  misc_ipv4_txt(start_group_addr, ip_buf),
                                  num_of_entries,
                                  portCnt);
                } else {
                    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d|%s|%d|%u|",
                                  start_vid,
                                  misc_ipv6_txt(&start_group_addr_ipv6, ip_buf),
                                  num_of_entries,
                                  portCnt);
                }
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            }

            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "NoEntries");
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        }

        (void)cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}


/****************************************************************************/
/*  Module JS lib config routine                                            */
/****************************************************************************/

static size_t ipmc_lib_config_js(char **base_ptr, char **cur_ptr, size_t *length)
{
    char buff[IPMC_WEB_BUF_LEN];
    (void) snprintf(buff, IPMC_WEB_BUF_LEN,
                    "var configIpmcVLANsMax = %d;\n",
                    IPMC_VLAN_MAX);
    return webCommonBufferHandler(base_ptr, cur_ptr, length, buff);
}

/****************************************************************************/
/*  JS lib config table entry                                               */
/****************************************************************************/

web_lib_config_js_tab_entry(ipmc_lib_config_js);

/****************************************************************************/
/*  HTTPD Table Entries                                                     */
/****************************************************************************/

CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_ipmc, "/config/ipmc", handler_config_ipmc);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_ipmc_vlan, "/config/ipmc_vlan", handler_config_ipmc_vlan);
#ifdef VTSS_SW_OPTION_SMB_IPMC
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_ipmc_filtering, "/config/ipmc_filtering", handler_config_ipmc_filtering);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_stat_ipmc_sfm_info, "/stat/ipmc_v2info", handler_stat_ipmc_sfm_info);
#endif /* VTSS_SW_OPTION_SMB_IPMC */
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_stat_ipmc_status, "/stat/ipmc_status", handler_stat_ipmc_status);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_stat_ipmc_groups_info, "/stat/ipmc_groups_info", handler_stat_ipmc_groups_info);

/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
