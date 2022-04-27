/*

 Vitesse Switch Software.

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
#include "ip2_api.h"
#include "ip2_snmp.h"
#include "ip2_utils.h"
#include "ip2_trace.h"
#include "critd_api.h"

#ifdef VTSS_SW_OPTION_SNMP

#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_IP2
#define __GRP VTSS_TRACE_IP2_GRP_SNMP
#define E(_fmt, ...) T(__GRP, VTSS_TRACE_LVL_ERROR, _fmt, ##__VA_ARGS__)
#define W(_fmt, ...) T(__GRP, VTSS_TRACE_LVL_WARNING, _fmt, ##__VA_ARGS__)
#define I(_fmt, ...) T(__GRP, VTSS_TRACE_LVL_INFO, _fmt, ##__VA_ARGS__)
#define D(_fmt, ...) T(__GRP, VTSS_TRACE_LVL_DEBUG, _fmt, ##__VA_ARGS__)
#define N(_fmt, ...) T(__GRP, VTSS_TRACE_LVL_NOISE, _fmt, ##__VA_ARGS__)
#define R(_fmt, ...) T(__GRP, VTSS_TRACE_LVL_RACKET, _fmt, ##__VA_ARGS__)

static critd_t ip2_crit;

#if (VTSS_TRACE_ENABLED)
#  define IP2_CRIT_ENTER()                                                  \
      critd_enter(&ip2_crit, VTSS_TRACE_IP2_GRP_CRIT, VTSS_TRACE_LVL_NOISE, \
                  __FILE__, __LINE__)
#  define IP2_CRIT_EXIT()                                                   \
      critd_exit(&ip2_crit, VTSS_TRACE_IP2_GRP_CRIT, VTSS_TRACE_LVL_NOISE,  \
                 __FILE__, __LINE__)

#  define IP2_CRIT_ASSERT_LOCKED()                 \
    critd_assert_locked(&ip2_crit,                 \
                        TRACE_GRP_CRIT,            \
                        __FILE__, __LINE__)
#else
#  define IP2_CRIT_ENTER() critd_enter(&ip2_crit)
#  define IP2_CRIT_EXIT()  critd_exit( &ip2_crit)
#  define IP2_CRIT_ASSERT_LOCKED() critd_assert_locked(&ip2_crit)
#endif /* VTSS_TRACE_ENABLED */

#define IP2_CRIT_RETURN(T, X) \
do {                          \
    T __val = (X);            \
    IP2_CRIT_EXIT();          \
    return __val;             \
} while(0)

#define IP2_CRIT_RETURN_RC(X)    \
    IP2_CRIT_RETURN(vtss_rc, X)

#define IP2_CRIT_RETURN_VOID()   \
    IP2_CRIT_EXIT();             \
    return
typedef struct {
    bool   active;

    bool has_ipv4;
    vtss_ipv4_network_t ipv4;
    cyg_tick_count_t ipv4_created;
    cyg_tick_count_t ipv4_changed;

    bool has_ipv6;
    vtss_ipv6_network_t ipv6;
    cyg_tick_count_t ipv6_created;
    cyg_tick_count_t ipv6_changed;
} if_snmp_status_t;

cyg_tick_count_t IP2_if_table_changed = 0;

// TODO, use a map instead
static if_snmp_status_t IP2_status[4096];

void SNMP_if_change_callback(vtss_if_id_vlan_t if_id)
{
    vtss_if_status_t st;
    bool exists;

    IP2_CRIT_ENTER();
    exists = (vtss_ip2_if_status_get_first(VTSS_IF_STATUS_TYPE_ANY, if_id,
                                           &st) == VTSS_RC_OK);

    if ((!IP2_status[if_id].active) && exists) {
        D("vlan created %u", if_id);
        IP2_status[if_id].active = TRUE;
        IP2_if_table_changed = cyg_current_time();
    }

    if (IP2_status[if_id].active && (!exists)) {
        D("vlan deleted %u", if_id);
        memset(&IP2_status[if_id], 0, sizeof(if_snmp_status_t));
        IP2_if_table_changed = cyg_current_time();
        IP2_CRIT_RETURN_VOID();
    }

    I("SNMP_if_change_callback vlan=%u", if_id);
    if (vtss_ip2_if_status_get_first(VTSS_IF_STATUS_TYPE_IPV4, if_id,
                                     &st) == VTSS_RC_OK ) {

        if (!IP2_status[if_id].has_ipv4) {
            // IPv4 address created
            D("ipv4 address created %u "VTSS_IPV4N_FORMAT, if_id,
              VTSS_IPV4N_ARG(st.u.ipv4.net));
            IP2_status[if_id].ipv4_created = cyg_current_time();
            IP2_status[if_id].ipv4_changed = cyg_current_time();

        } else if (vtss_ipv4_network_equal(&IP2_status[if_id].ipv4,
                                           &st.u.ipv4.net)) {
            // IPv4 address changed
            D("ipv4 address changed %u "VTSS_IPV4N_FORMAT, if_id,
              VTSS_IPV4N_ARG(st.u.ipv4.net));
            IP2_status[if_id].ipv4_changed = cyg_current_time();
        }

        IP2_status[if_id].ipv4 = st.u.ipv4.net;
        IP2_status[if_id].has_ipv4 = TRUE;

    } else {
        if (IP2_status[if_id].has_ipv4) {
            D("ipv4 address deleted %u ", if_id);
        }
        IP2_status[if_id].has_ipv4 = FALSE;
    }


    if (vtss_ip2_if_status_get_first(VTSS_IF_STATUS_TYPE_IPV6,
                                     if_id,
                                     &st) == VTSS_RC_OK ) {

        if (!IP2_status[if_id].has_ipv6) {
            D("ipv6 address created %u "VTSS_IPV6N_FORMAT, if_id,
              VTSS_IPV6N_ARG(st.u.ipv6.net));
            IP2_status[if_id].ipv6_created = cyg_current_time();
            IP2_status[if_id].ipv6_changed = cyg_current_time();

        } else if (vtss_ipv6_network_equal(&IP2_status[if_id].ipv6,
                                           &st.u.ipv6.net)) {
            D("ipv6 address changed %u "VTSS_IPV6N_FORMAT, if_id,
              VTSS_IPV6N_ARG(st.u.ipv6.net));
            IP2_status[if_id].ipv6_changed = cyg_current_time();
        }

        IP2_status[if_id].ipv6 = st.u.ipv6.net;
        IP2_status[if_id].has_ipv6 = TRUE;

    } else {
        if (IP2_status[if_id].has_ipv6) {
            D("ipv6 address deleted %u ", if_id);
        }
        IP2_status[if_id].has_ipv6 = false;
    }

    IP2_CRIT_RETURN_VOID();
}

vtss_rc vtss_ip2_snmp_init()
{
    vtss_rc rc;
    vtss_if_id_vlan_t i;

    critd_init(&ip2_crit, "ip2-snmp.crit",
               VTSS_MODULE_ID_IP2, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);

    I("vtss_ip2_snmp_init");

    memset(IP2_status, 0, sizeof(IP2_status));
    rc = vtss_ip2_if_callback_add(SNMP_if_change_callback);

    if (rc != VTSS_RC_OK) {
        E("Failed to registerer callback");
    }

    IP2_CRIT_EXIT();

    // be sure to have an updated state
    if (rc == VTSS_RC_OK) {
        for (i = 0; i < 4096; ++i) {
            SNMP_if_change_callback(i);
        }
    }

    return rc;
}

void vtss_ip2_snmp_signal_global_changes()
{
    IP2_CRIT_ENTER();
    IP2_if_table_changed = cyg_current_time();
    IP2_CRIT_EXIT();
}

vtss_rc vtss_ip2_interfaces_last_change(u64 *time)
{
    IP2_CRIT_ENTER();
    D("vtss_ip2_interfaces_last_change");
    *time = IP2_if_table_changed / ECOS_TICKS_PER_SEC;
    IP2_CRIT_RETURN_RC(VTSS_RC_OK);
}

vtss_rc vtss_ip2_address_created_ipv4(vtss_if_id_vlan_t if_id, u64 *time)
{
    vtss_rc rc = VTSS_RC_ERROR;

    IP2_CRIT_ENTER();
    D("vtss_ip2_address_created_ipv4 vlan=%u", if_id);
    if (IP2_status[if_id].has_ipv4) {
        *time = IP2_status[if_id].ipv4_created / ECOS_TICKS_PER_SEC;
        rc = VTSS_RC_OK;
    }
    IP2_CRIT_RETURN_RC(rc);
}

vtss_rc vtss_ip2_address_changed_ipv4(vtss_if_id_vlan_t if_id, u64 *time)
{
    vtss_rc rc = VTSS_RC_ERROR;

    IP2_CRIT_ENTER();
    D("vtss_ip2_address_changed_ipv4 vlan=%u", if_id);
    if (IP2_status[if_id].has_ipv4) {
        *time = IP2_status[if_id].ipv4_changed / ECOS_TICKS_PER_SEC;
        rc = VTSS_RC_OK;
    }
    IP2_CRIT_RETURN_RC(rc);
}

vtss_rc vtss_ip2_address_created_ipv6(vtss_if_id_vlan_t if_id, u64 *time)
{
    vtss_rc rc = VTSS_RC_ERROR;

    IP2_CRIT_ENTER();
    D("vtss_ip2_address_created_ipv6 vlan=%u", if_id);
    if (IP2_status[if_id].has_ipv6) {
        *time = IP2_status[if_id].ipv6_created / ECOS_TICKS_PER_SEC;
        rc = VTSS_RC_OK;
    }
    IP2_CRIT_RETURN_RC(rc);
}

vtss_rc vtss_ip2_address_changed_ipv6(vtss_if_id_vlan_t if_id, u64 *time)
{
    vtss_rc rc = VTSS_RC_ERROR;

    IP2_CRIT_ENTER();
    D("vtss_ip2_address_changed_ipv6 vlan=%u", if_id);
    if (IP2_status[if_id].has_ipv6) {
        *time = IP2_status[if_id].ipv6_changed / ECOS_TICKS_PER_SEC;
        rc = VTSS_RC_OK;
    }
    IP2_CRIT_RETURN_RC(rc);
}

#endif /* VTSS_SW_OPTION_SNMP */

