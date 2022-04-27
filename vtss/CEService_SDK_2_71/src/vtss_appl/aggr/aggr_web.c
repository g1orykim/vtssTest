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
#ifdef VTSS_SW_OPTION_LACP
#include "l2proto_api.h"
#endif /* VTSS_SW_OPTION_LACP */
#include "aggr_api.h"
#include "port_api.h"
#include <network.h>
#include <pkgconf/system.h>
#include <pkgconf/net.h>
#include <time.h>
#include <arpa/inet.h>
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

// #define aggr_mgmt_port_members_add(a,b,c) aggr_mgmt_port_members_add(b,c)
// #define aggr_mgmt_group_del(a,b)        aggr_mgmt_group_del(b)
// #define aggr_mgmt_aggr_mode_set(a,b)    aggr_mgmt_aggr_mode_set(b)
// #define aggr_mgmt_port_members_get(a,b,c,d) aggr_mgmt_port_members_get(b,c,d)
// #define aggr_mgmt_aggr_mode_get(a,b)    aggr_mgmt_aggr_mode_get(b)
static aggr_mgmt_group_no_t web_aggr_id2no(aggr_mgmt_group_no_t aggr_id)
{
    if (vtss_switch_stackable()) {
#if defined(VTSS_FEATURE_VSTAX_V2)
        return aggr_id;
#else
        return (aggr_id > 2 ?
                (AGGR_MGMT_GROUP_NO_START + aggr_id - VTSS_GLAGS - 1) :
                (AGGR_MGMT_GLAG_START + aggr_id - 1) );
#endif
    } else {
        return (AGGR_MGMT_GROUP_NO_START + aggr_id - 1);
    }

}

/****************************************************************************/
/*  Web Handler  functions                                                  */
/****************************************************************************/
static cyg_int32 handler_config_aggregation(CYG_HTTPD_STATE *p)
{
    vtss_isid_t              sid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    vtss_port_no_t           iport;
    aggr_mgmt_group_no_t     aggrgroup;
    aggr_mgmt_group_member_t aggrMember;
    aggr_mgmt_group_t        groupMember;
    int                      ct;
    vtss_rc                  rc;
    unsigned int             aggr_count = AGGR_LAG_CNT;
    BOOL                     aggr[AGGR_LAG_CNT + 1][VTSS_PORTS] = {{0}}; //Estax34 should have 13 LAGs, reserved one for STACK ports
    u32                      port_count = port_isid_port_count(sid);
#if 0
    /* aggr[][] structure like following, each cell mapping to web related radio buttom */
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, /*Normal Group */
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, /* Group 1 :GLAG */
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, /* Group 2 :GLAG */
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, /* Group 3 :LLAG */
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, /* Group 4 :LLAG */
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, /* Group 5 :LLAG */
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} /* Group 14 :LLAG */
#endif
    vtss_aggr_mode_t mode;
    int glag_count[2] = {0, 0};

    if (redirectUnmanagedOrInvalid(p, sid)) { /* Redirect unmanaged/invalid access to handler */
        return -1;
    }

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_AGGR)) {
        return -1;
    }
#endif
    if (AGGR_UPLINK == AGGR_UPLINK_NONE) {
        aggr_count++;
    }
    if (p->method == CYG_HTTPD_METHOD_POST) {
        int errors = 0, tmpport;
        char var_aggr[12], var_id[16];
        size_t nlen;
        const char *id;
        aggr_mgmt_group_member_t grp;
        aggr_mgmt_group_no_t aggr_id;

        for (iport = 0; iport < port_count; iport++) { // for each port, find it's group from web POST arguments
            sprintf(var_aggr, "aggr_port_%d", iport);
            id = cyg_httpd_form_varable_string(p, var_aggr, &nlen);
            for (aggrgroup = 0; aggrgroup < aggr_count; aggrgroup++) {
                sprintf(var_id, "aggr_%u_port_%d", aggrgroup, iport);
                if (nlen == strlen(var_id) && memcmp(id, var_id, nlen) == 0) {
                    aggr[aggrgroup][iport] = 1;
                } else {
                    aggr[aggrgroup][iport] = 0;
                }
            }
        }

        for (aggrgroup = 1; aggrgroup < aggr_count; aggrgroup++) { // Group 0 is Normal group, real group from 1
            for (iport = 0; iport < port_count; iport++) {
                if ( aggr[aggrgroup][iport] == 1 ) { // this group has members, add the group
                    memcpy(&groupMember.member[VTSS_PORT_NO_START], aggr[aggrgroup], VTSS_PORTS * sizeof(BOOL));

                    /* If the port is a member in other groups it must be removed  */
                    for (tmpport = 0; tmpport < port_count; tmpport++) {
                        if (groupMember.member[tmpport]) {
                            if ((aggr_id = aggr_mgmt_get_port_aggr_id(sid, tmpport)) > 0 && (aggr_id != aggrgroup)) {
                                if ((rc = aggr_mgmt_port_members_get(sid, aggr_id, &grp, 0)) != VTSS_RC_OK) {
                                    send_custom_error(p, "Aggregation Error", aggr_error_txt(rc), strlen(aggr_error_txt(rc)));
                                }
                                grp.entry.member[tmpport] = 0;  // Delete the member

                                if ((rc = aggr_mgmt_port_members_add(sid, aggr_id, &grp.entry)) != VTSS_RC_OK) {
                                    send_custom_error(p, "Aggregation Error", aggr_error_txt(rc), strlen(aggr_error_txt(rc)));
                                }
                            }
                        }
                    }

                    if ((rc = aggr_mgmt_port_members_add(sid, (web_aggr_id2no(aggrgroup)), &groupMember)) < 0) {
                        send_custom_error(p, "Aggregation Error", aggr_error_txt(rc), strlen(aggr_error_txt(rc)));
                        return -1;
                    }
                    break;
                }
                if (iport == (port_count - 1) ) { // This group has no member
                    if ((rc = aggr_mgmt_group_del(sid, web_aggr_id2no(aggrgroup))) < 0) {
                        if (aggr_mgmt_group_del(sid, web_aggr_id2no(aggrgroup)) == AGGR_ERROR_GROUP_IN_USE) {
                            continue;
                        }
                        T_D("aggr_mgmt_group_del(%u,%u): failed rc = %d", sid, aggrgroup, rc);
                        errors++; /* Probably stack error */
                    }
                }
            }
        }

        memset(&mode, 0, sizeof(vtss_aggr_mode_t));  // Gnats 5498, Jack 20061129
        if (cyg_httpd_form_varable_string(p, "src_mac", &nlen)) { // if get token "src_mac", it must be checked on browser
            mode.smac_enable = 1;
        } else {
            mode.smac_enable = 0;
        }
        if (cyg_httpd_form_varable_string(p, "det_mac", &nlen)) {
            mode.dmac_enable = 1;
        } else {
            mode.dmac_enable = 0;
        }
        if (cyg_httpd_form_varable_string(p, "mode_ip", &nlen)) {
            mode.sip_dip_enable = 1;
        } else {
            mode.sip_dip_enable = 0;
        }
        if (cyg_httpd_form_varable_string(p, "mode_port", &nlen)) {
            mode.sport_dport_enable = 1;
        } else {
            mode.sport_dport_enable = 0;
        }
        if ((rc = aggr_mgmt_aggr_mode_set(&mode)) < 0) {
            T_D("aggr_mgmt_aggr_mode_set(%u):failed rc = %d", sid, rc);
            errors++; /* Probably stack error */
        }
        redirect(p, errors ? STACK_ERR_URL : "/aggregation.htm");
    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        for (aggrgroup = 1; aggrgroup < aggr_count; aggrgroup++) { // Get each group members
            if (aggr_mgmt_port_members_get(sid, web_aggr_id2no(aggrgroup), &aggrMember, 0) == VTSS_RC_OK) {
                for (iport = VTSS_PORT_NO_START ; iport < port_count ; iport++) {
                    aggr[aggrgroup][iport - VTSS_PORT_NO_START] = aggrMember.entry.member[iport];

                }
            } else {
                for (iport = VTSS_PORT_NO_START ; iport < VTSS_PORT_NO_END ; iport++) {
                    aggr[aggrgroup][iport - VTSS_PORT_NO_START] = 0;
                }
            }
        }

        for (iport = VTSS_PORT_NO_START; iport < port_count; iport++) {
            aggr[0][iport - VTSS_PORT_NO_START] = 1; // default belong to Normal group
            for (aggrgroup = 1; aggrgroup < aggr_count; aggrgroup++) {
                if (aggr[aggrgroup][iport - VTSS_PORT_NO_START] == 1) {
                    aggr[0][iport - VTSS_PORT_NO_START] = 0; // if this port belongs to any other aggr group, remove it from the Normal group.
                    //break;
                }
            }
        }

        if (aggr_mgmt_aggr_mode_get(&mode) != VTSS_RC_OK) {
            T_E("aggr_mgmt_aggr_mode_get fail!!");
        }
        cyg_httpd_start_chunked("html");

        for (aggrgroup = 0 ; aggrgroup < aggr_count; aggrgroup++) {   // send out the whole aggr[][] structure to client.
            for (iport = 0 ; iport < port_count; iport++) { // VTSS_PORTS - 2 for "Do not send stack port 25,26"
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), iport < ( VTSS_PORTS - 1) ? "%u/" : "%u", aggr[aggrgroup][iport]);
                cyg_httpd_write_chunked(p->outbuffer, ct);
            }
            //ct = snprintf(p->outbuffer, sizeof(p->outbuffer),"%s", aggrgroup == (aggr_count-1) ? "" : "|");
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "|");
            cyg_httpd_write_chunked(p->outbuffer, ct);
        }
#if 0
// L28 code, to be removed
        /* calculate the current member count of glag1 and glag2(not include selected switch itself)*/
        (void)switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID);
        while (switch_iter_getnext(&sit)) {
            if (sit.isid == sid) {
                continue;    /* exclude the selected switch for easier Java implementation */
            }
            for (aggrgroup = 1; aggrgroup <= VTSS_GLAGS; aggrgroup++) { // Get each group members
                if ( aggr_mgmt_port_members_get(sit.isid, web_aggr_id2no(aggrgroup), &aggrMember, 0) == VTSS_RC_OK ) {
                    for (iport = VTSS_PORT_NO_START; iport < port_count; iport++) {
                        if (aggrMember.entry.member[iport]) {
                            glag_count[aggrgroup - 1]++;
                        }
                    }
                }
            }
        }
#endif

        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/%u/%u/%u/%u/%u/%u",
                      mode.smac_enable, mode.dmac_enable, mode.sip_dip_enable, mode.sport_dport_enable, 0, glag_count[0], glag_count[1]);
        cyg_httpd_write_chunked(p->outbuffer, ct);

        cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}

/****************************************************************************/
/*  HTTPD Table Entries                                                     */
/****************************************************************************/

CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_aggregation, "/config/aggregation", handler_config_aggregation);

/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
