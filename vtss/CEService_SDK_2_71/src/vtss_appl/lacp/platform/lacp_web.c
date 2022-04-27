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
#include "lacp_api.h"
#include "mgmt_api.h"
#ifdef VTSS_SW_OPTION_PRIV_LVL
#include "vtss_privilege_api.h"
#include "vtss_privilege_web_api.h"
#endif
#if VTSS_SWITCH_STACKABLE
#include "topo_api.h"
#include "msg_api.h"
#endif /* VTSS_SWITCH_STACKABLE */

/****************************************************************************/
/*  Web Handler  functions                                                  */
/****************************************************************************/

static cyg_int32 handler_config_lacp_ports(CYG_HTTPD_STATE *p)
{
    vtss_isid_t              isid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    vtss_uport_no_t          uport;
    int                      ct;
    vtss_lacp_port_config_t  conf;
    char                     *err = NULL;
    vtss_rc                  rc;
    port_iter_t              pit;

    if (redirectUnmanagedOrInvalid(p, isid)) { /* Redirect unmanaged/invalid access to handler */
        return -1;
    }

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_LACP)) {
        return -1;
    }
#endif

    if (p->method == CYG_HTTPD_METHOD_POST) {
        int val;
        (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT);
        while (port_iter_getnext(&pit)) {
            if (lacp_mgmt_port_conf_get(isid, pit.iport, &conf) != VTSS_RC_OK) {
                continue;
            }

            vtss_lacp_port_config_t new, *old = &conf;
            uport = iport2uport(pit.iport);
            new = *old;
            new.enable_lacp = cyg_httpd_form_variable_check_fmt(p, "enable_%d", uport);
            if (cyg_httpd_form_variable_int_fmt(p, &val, "keyauto_%d", uport) && !val) {
                if (cyg_httpd_form_variable_int_fmt(p, &val, "key_%d", uport)) {
                    new.port_key = val;
                }
            } else {
                new.port_key = VTSS_LACP_AUTOKEY;
            }

            if (cyg_httpd_form_variable_int_fmt(p, &val, "role_%d", uport)) {
                new.active_or_passive = val;
            }

            if (cyg_httpd_form_variable_int_fmt(p, &val, "timeout_%d", uport)) {
                new.xmit_mode = val;
            }

            if (cyg_httpd_form_variable_int_fmt(p, &val, "prio_%d", uport)) {
                new.port_prio = val;
            }

            if (memcmp(&new, old, sizeof(*old)) != 0) {
                *old = new;
                if ((rc = lacp_mgmt_port_conf_set(isid, pit.iport, &conf)) != VTSS_RC_OK) {
                    if (rc == LACP_ERROR_STATIC_AGGR_ENABLED) {
                        err = "LACP and Static aggregation can not both be enabled on the same ports";
                    } else if (rc == LACP_ERROR_DOT1X_ENABLED) {
                        err = "LACP cannot be enabed on ports whose 802.1X Admin State is not Authorized";
                    } else {
                        err = "Unspecified error.";
                    }
                }
            }
        }

        if (err != NULL) {
            send_custom_error(p, "LACP Error", err, strlen(err));
        } else {
            redirect(p, "/lacp_port_config.htm");
        }

    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        cyg_httpd_start_chunked("html");
        (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT);
        while (port_iter_getnext(&pit)) {
            if (lacp_mgmt_port_conf_get(isid, pit.iport, &conf) != VTSS_RC_OK) {
                continue;
            }

            const vtss_lacp_port_config_t *pp = &conf;
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/%d/%d/%d/%d/%d|",
                          iport2uport(pit.iport),
                          pp->enable_lacp,
                          pp->port_key,
                          pp->active_or_passive,
                          pp->xmit_mode,
                          pp->port_prio);
            cyg_httpd_write_chunked(p->outbuffer, ct);
        }
        cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}

static cyg_int32 handler_stat_lacp_port_status(CYG_HTTPD_STATE *p)
{
    vtss_isid_t                  isid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    vtss_port_no_t               iport;
    vtss_uport_no_t              uport;
    int                          ct;
    vtss_lacp_aggregatorstatus_t aggr;
    char                         buf[32];
    vtss_lacp_portstatus_t       pp;
    const char                   *aggr_id = NULL;
    u32                          port_count = port_isid_port_count(isid);
    BOOL                         found_aggr;
    aggr_mgmt_group_no_t         aggr_no;
    int                          search_aid, return_aid;
    BOOL                         first_search = 1;

    if (redirectUnmanagedOrInvalid(p, isid)) { /* Redirect unmanaged/invalid access to handler */
        return -1;
    }

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_STAT_TYPE, VTSS_MODULE_ID_LACP)) {
        return -1;
    }
#endif

    cyg_httpd_start_chunked("html");

    for (iport = VTSS_PORT_NO_START; iport < port_count; iport++) {
        if (port_isid_port_no_is_stack(isid, iport) || lacp_mgmt_port_status_get(L2PORT2PORT(isid, iport), &pp) != VTSS_RC_OK) {
            continue;
        }

        uport = iport2uport(iport);
        found_aggr = 0;
        first_search = 1;
        while (aggr_mgmt_lacp_id_get_next(first_search ? NULL : &search_aid,  &return_aid) == VTSS_RC_OK) {
            search_aid = return_aid;
            first_search = 0;
            if (lacp_mgmt_aggr_status_get(return_aid, &aggr)) {
                if (aggr.port_list[L2PORT2PORT(isid, iport) - VTSS_PORT_NO_START]) {
                    found_aggr = 1;
                    break;
                }
            }
        }

        if (!found_aggr) {
            if (pp.port_state == LACP_PORT_STANDBY) {

                ct = snprintf(p->outbuffer, sizeof(p->outbuffer),
                              "%u/%s/%d/-/-/-/-|",
                              uport,
                              "Standby",
                              pp.port_enabled ? pp.actor_oper_port_key : 0);

            } else {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer),
                              "%u/%s/%d/-/-/-/-|",
                              uport,
                              pp.port_enabled ? "Yes" : " No",
                              pp.port_enabled ? pp.actor_oper_port_key : 0);
            }
        } else {
            aggr_no = lacp_to_aggr_id(pp.actor_port_aggregator_identifier);
            if ((pp.port_state == LACP_PORT_NOT_ACTIVE) || !AGGR_MGMT_GROUP_IS_AGGR(aggr_no)) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer),
                              "%u/%s/%d/-/-/-/-|",
                              uport,
                              pp.port_enabled ? "Yes" : " No",
                              pp.port_enabled ? pp.actor_oper_port_key : 0);
            } else {
                if (AGGR_MGMT_GROUP_IS_LAG(lacp_to_aggr_id(pp.actor_port_aggregator_identifier))) {
                    aggr_id = l2port2str(L2LLAG2PORT(isid, lacp_to_aggr_id(aggr.aggrid) - AGGR_MGMT_GROUP_NO_START));
                } else {
                    aggr_id = l2port2str(L2GLAG2PORT(lacp_to_aggr_id(aggr.aggrid) - AGGR_MGMT_GLAG_START + VTSS_GLAG_NO_START));
                }
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer),
                              "%u/%s/%d/%s/%s/%d/%d|",
                              uport,
                              pp.port_enabled ? "Yes" : " No",
                              pp.actor_oper_port_key,
                              aggr_id,
                              misc_mac_txt(aggr.partner_oper_system.macaddr, buf),
                              pp.partner_oper_port_number,
                              pp.partner_oper_port_priority);
            }
        }

        cyg_httpd_write_chunked(p->outbuffer, ct);

    }

    cyg_httpd_write_chunked("|", 1); /* Must return something - <empty> is stack error! */
    cyg_httpd_end_chunked();

    return -1; // Do not further search the file system.
}

static cyg_int32 handler_stat_lacp_sys_status(CYG_HTTPD_STATE *p)
{
    vtss_isid_t isid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    int                          ct;
    vtss_lacp_aggregatorstatus_t aggr;
    char                         buf[32];
    char                         portbuf[100];
    l2_port_no_t                 l2port;
    int                          first;
    vtss_port_no_t               iport;
    vtss_isid_t                  isid_id;
    const char                   *aggr_id;
    int                          search_aid, return_aid;
    BOOL                         first_search = 1;

    if (redirectUnmanagedOrInvalid(p, isid)) { /* Redirect unmanaged/invalid access to handler */
        return -1;
    }

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_STAT_TYPE, VTSS_MODULE_ID_LACP)) {
        return -1;
    }
#endif

    cyg_httpd_start_chunked("html");

    while (aggr_mgmt_lacp_id_get_next(first_search ? NULL : &search_aid,  &return_aid) == VTSS_RC_OK) {
        search_aid = return_aid;
        first_search = 0;
        if (lacp_mgmt_aggr_status_get(return_aid, &aggr)) {
            memset(portbuf, 0, sizeof(portbuf));
            first = 1;
#if VTSS_SWITCH_STACKABLE
            vtss_usid_t usid;
            vtss_isid_t isid;
            u32         port_count;
            for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) {
                isid = topo_usid2isid(usid);
                port_count = port_isid_port_count(isid);
                if (msg_switch_exists(isid)) {
                    for (iport = VTSS_PORT_NO_START; iport < port_count; iport++) {
                        l2port = L2PORT2PORT(isid, iport) - VTSS_PORT_NO_START;
                        if (aggr.port_list[l2port]) {
                            sprintf(portbuf, "%s%s%s", portbuf, first ? "" : ",", l2port2str(l2port));
                            if (first) {
                                l2port2port(l2port, &isid_id, &iport);
                            }
                            first = 0;
                        }
                    }
                }
            }
#else /* VTSS_SWITCH_STACKABLE */
            for (l2port = 0; l2port < VTSS_LACP_MAX_PORTS; l2port++) {
                if (aggr.port_list[l2port]) {
                    sprintf(portbuf, "%s%s%s", portbuf, first ? "" : ",", l2port2str(l2port));
                    if (first) {
                        l2port2port(l2port, &isid_id, &iport);
                    }
                    first = 0;
                }
            }
#endif /* VTSS_SWITCH_STACKABLE */

            if (AGGR_MGMT_GROUP_IS_LAG(lacp_to_aggr_id(aggr.aggrid))) {
                aggr_id = l2port2str(L2LLAG2PORT(isid_id, lacp_to_aggr_id(aggr.aggrid) - AGGR_MGMT_GROUP_NO_START));
            } else {
                aggr_id = l2port2str(L2GLAG2PORT(lacp_to_aggr_id(aggr.aggrid) - AGGR_MGMT_GLAG_START + VTSS_GLAG_NO_START));
            }

            ct = snprintf(p->outbuffer, sizeof(p->outbuffer),
                          "%s/%s/%d/%d/%s/%s|",
                          aggr_id,
                          misc_mac_txt(aggr.partner_oper_system.macaddr, buf),
                          aggr.partner_oper_key,
                          aggr.partner_oper_system_priority,
                          misc_time2interval(aggr.secs_since_last_change),
                          portbuf);

            cyg_httpd_write_chunked(p->outbuffer, ct);
        }
    }

    cyg_httpd_write_chunked("|", 1); /* Must return something - <empty> is stack error! */
    cyg_httpd_end_chunked();

    return -1; // Do not further search the file system.
}

static cyg_int32 handler_stat_lacp_statistics(CYG_HTTPD_STATE *p)
{
    vtss_isid_t isid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    int                    ct;
    vtss_port_no_t         iport;
    vtss_lacp_portstatus_t pp;
    u32                    port_count = port_isid_port_count(isid);

    if (redirectUnmanagedOrInvalid(p, isid)) { /* Redirect unmanaged/invalid access to handler */
        return -1;
    }

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_STAT_TYPE, VTSS_MODULE_ID_LACP)) {
        return -1;
    }
#endif

    cyg_httpd_start_chunked("html");

    for (iport = VTSS_PORT_NO_START; iport < port_count; iport++) {
        if (port_isid_port_no_is_stack(isid, iport)) {
            continue;
        }

        if (var_clear[0]) {    /* Clear? */
            lacp_mgmt_statistics_clear(L2PORT2PORT(isid, iport));
        }

        (void)lacp_mgmt_port_status_get(L2PORT2PORT(isid, iport), &pp);
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer),
                      "%u/%lu/%lu/%lu/%lu|",
                      iport2uport(iport),
                      pp.port_stats.lacp_frame_recvs,
                      pp.port_stats.lacp_frame_xmits,
                      pp.port_stats.illegal_frame_recvs,
                      pp.port_stats.unknown_frame_recvs);
        cyg_httpd_write_chunked(p->outbuffer, ct);
    }

    cyg_httpd_write_chunked("|", 1); /* Must return something - <empty> is stack error! */
    cyg_httpd_end_chunked();

    return -1; // Do not further search the file system.
}

/****************************************************************************/
/*  HTTPD Table Entries                                                     */
/****************************************************************************/

CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_lacp_ports, "/config/lacp_ports", handler_config_lacp_ports);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_stat_lacp_port_status, "/stat/lacp_port_status", handler_stat_lacp_port_status);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_stat_lacp_sys_status, "/stat/lacp_sys_status", handler_stat_lacp_sys_status);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_stat_lacp_statistics, "/stat/lacp_statistics", handler_stat_lacp_statistics);

/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
