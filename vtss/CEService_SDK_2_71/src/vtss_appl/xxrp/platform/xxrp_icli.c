/*

 Vitesse Switch Software.

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

#include "xxrp_api.h"
#include "icli_api.h"
#include "../base/src/vtss_gvrp.h"
#include "../base/src/vtss_garp.h"
#include "xxrp_icli.h"
#include <vtss_xxrp_callout.h>


#define VTSS_TRACE_MODULE_ID  VTSS_MODULE_ID_XXRP



static void f_header(void)
{
    printf("             |<-------- State of: ------->||<--- Timer [cs]: -->|\n");
    printf("Sw Port VLan  Applicant Registrar LeaveAll  txPDU leave leaveall  GIP-Context\n");
}

u32 leaveall_state;


static void f(u16 usid, u16 isid, u16 iport, u16 port, unsigned int vlan)
{
    int rc;

    int port_no;
    struct garp_gid_instance *gid;
    int leave;
    u32 txPDU_timeout;
    int gip_context;

    port_no = L2PORT2PORT(isid, iport);

    rc = vtss_gvrp_gid(port_no, vlan, &gid);
    if (rc) {
        return;
    }

    printf("%2hu %4hu %4u  ", usid, port, vlan);
    printf("%6s %9s %11s",
           applicant_state_name(gid->applicant_state),
           registrar_state_name(gid->registrar_state),
           leaveall_state_name(leaveall_state));

    leave = vtss_gvrp_leave_timeout(gid);

    // --- If not in txPDU queue, then print dash
    if (IS_GID_IN_TXPDU_QUEUE(gid)) {
        txPDU_timeout = vtss_gvrp_txPDU_timeout(gid);
        printf("%7u", txPDU_timeout);
    }  else {
        printf("     - ");
    }

    // --- If not in leave queue, then print dash
    if ((leave == -1)) {
        printf("   -   ");
    } else {
        printf(" %5d ", leave);
    }

    printf("%6u", vtss_gvrp_leaveall_timeout(port_no));

    gip_context = vtss_gvrp_gip_context(gid);

    if (gip_context < 0) {
        printf("     -\n");
    } else {
        printf("     %1d\n", gip_context);
    }


    vtss_gvrp_gid_done(&gid);
}



void gvrp_protocol_state(icli_stack_port_range_t *plist, icli_unsigned_range_t *v_vlan_list)
{
    u32 i;
    int rc;

    u16 usid, isid;

    if (!plist || !v_vlan_list) {
        T_E("plist=%p,  v_vlan_list=%p", plist, v_vlan_list);
        return;
    }

    f_header();

    GVRP_CRIT_ENTER();

    //  Loop over switch, port, vlans
    for (i = 0; i < plist->cnt; ++i) {

        usid = plist->switch_range[i].usid;
        isid = plist->switch_range[i].isid;

        u16 portBegin = plist->switch_range[i].begin_port;
        u16 iportBegin = plist->switch_range[i].begin_iport;
        u16 k;

        for (k = 0; k < plist->switch_range[i].port_cnt; ++k) {
            u32 j;
            int port_no;
            struct garp_participant *p;

            port_no = L2PORT2PORT(isid, k + iportBegin);

            rc = vtss_gvrp_participant(port_no, &p);
            if (rc) {
                T_N("GVRP protocol-state.1: port_no=%d do not exist", port_no);
                continue;
            }
            leaveall_state = p->leaveall_state;

            for (j = 0; j < v_vlan_list->cnt; ++j) {

                unsigned int vlan_min = v_vlan_list->range[j].min;
                unsigned int vlan_max = v_vlan_list->range[j].max;
                unsigned int vlan;

                for (vlan = vlan_min; vlan <= vlan_max; ++vlan) {

                    f(usid, isid, k + iportBegin, k + portBegin, vlan);

                }
            }
        }
    }

    GVRP_CRIT_EXIT();
    return;
}


void gvrp_global_enable(int enable, int max_vlans)
{
    int rc = 0;

    T_N("GVRP global enable.1, enable=%d max_vlans=%d", enable, max_vlans);

    if (enable) {

        rc = vtss_gvrp_construct(-1, max_vlans);
        rc = xxrp_mgmt_global_enabled_set(VTSS_GARP_APPL_GVRP, 1);
    } else {
        rc = xxrp_mgmt_global_enabled_set(VTSS_GARP_APPL_GVRP, 0);
        GVRP_CRIT_ENTER();
        vtss_gvrp_destruct(FALSE);
        GVRP_CRIT_EXIT();
    }

    if (rc) {
        T_N("GVRP global enable.2 failed, rc=%d", rc);
        return;
    }

    T_N("GVRP global enable.2: Exit OK");
}



static void funktor_portvlan(icli_stack_port_range_t *plist,
                             icli_unsigned_range_t *v_vlan_list,
                             vtss_rc (*F)(int, u16))
{
    u32 i;
    vtss_rc rc;

    //  Loop over switch, port, vlans
    for (i = 0; i < plist->cnt; ++i) {

        u16 isid = plist->switch_range[i].switch_id;

        u16 portBegin = plist->switch_range[i].begin_iport;
        u16 portEnd = portBegin + plist->switch_range[i].port_cnt;
        u16 iport;

        for (iport = portBegin; iport < portEnd; ++iport) {

            u32 j;
            int port_no;

            port_no = L2PORT2PORT(isid, iport);

            for (j = 0; j < v_vlan_list->cnt; ++j) {

                unsigned int vlan_min = v_vlan_list->range[j].min;
                unsigned int vlan_max = v_vlan_list->range[j].max;
                unsigned int vlan;

                for (vlan = vlan_min; vlan <= vlan_max; ++vlan) {

                    rc = F(port_no, vlan);
                    if (rc) {
                        T_E("rc=%d", rc);
                    }
                }
            }
        }
    }

    return;
}

static void funktor_port(icli_stack_port_range_t *plist,
                         void (*F)(vtss_isid_t, vtss_port_no_t, int), int enable)
{
    u32 i;

    //  Loop over switch, port, vlans
    for (i = 0; i < plist->cnt; ++i) {

        u16 portBegin = plist->switch_range[i].begin_iport;
        u16 portEnd = portBegin + plist->switch_range[i].port_cnt;
        u16 iport;

        for (iport = portBegin; iport < portEnd; ++iport) {
            F(plist->switch_range[i].isid, (vtss_port_no_t)iport, enable);
        }
    }

    return;
}



void gvrp_join_request(icli_stack_port_range_t *plist, icli_unsigned_range_t *v_vlan_list)
{
    GVRP_CRIT_ENTER();
    funktor_portvlan(plist, v_vlan_list, vtss_gvrp_join_request);
    GVRP_CRIT_EXIT();
}



void gvrp_leave_request(icli_stack_port_range_t *plist, icli_unsigned_range_t *v_vlan_list)
{
    GVRP_CRIT_ENTER();
    funktor_portvlan(plist, v_vlan_list, vtss_gvrp_leave_request);
    GVRP_CRIT_EXIT();
}

#if 0
static void port_enable(int port, int enable)
{
    vtss_isid_t isid = 1;
    vtss_port_no_t iport = port;
    vtss_rc rc;


    rc = xxrp_mgmt_enabled_set(isid, iport, VTSS_GARP_APPL_GVRP, enable);
    T_N("port_enable(%d, %d) rc=%d", port, enable, rc);
}
#endif

static void port_enable(vtss_isid_t isid, vtss_port_no_t iport, int enable)
{
    vtss_rc rc;

    rc = xxrp_mgmt_enabled_set(isid, iport, VTSS_GARP_APPL_GVRP, enable);
    if (rc) {
        T_E("rc=%d", rc);
    }

    T_N("port_enable(%d, %d) rc=%d", iport, enable, rc);
}

void gvrp_port_enable(icli_stack_port_range_t *plist, int enable)
{
    funktor_port(plist, port_enable, enable);
}
