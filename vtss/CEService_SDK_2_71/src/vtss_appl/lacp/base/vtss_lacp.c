/*

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

/**
 * This file contains an implementation of the 802.3ad link aggregation
 * protocol Clause 43 a.k.a LACP - Link Aggregation Control Protocol.
 *
 * It is written generic to be adapted into different operating environment.
 * The file vtss_lacp.h defines the API between this protocol module and
 * the operating environment.
 */

#define VTSS_LACP_PROTOCOL 1    /* This is the protocol module */

#include "vtss_lacp.h"
#ifdef VTSS_LACP_NOT_WANTED
#if defined(__CX51__) || defined(__C51__)
const char *vtss_lacp_coseg = "This creates the CO segment of vtss_lacp";
#endif /* __CX51__ || __C51__ */
#else
#include "vtss_lacp_private.h"

#ifndef VTSS_LACP_TRACE
#include <stdio.h>
#define VTSS_LACP_TRACE(lvl, fmt, ...)     printf(fmt, ##__VA_ARGS__)
#endif /* VTSS_LACP_TRACE */

/* The *only* global variable */
vtss_lacp_system_vars_t vtss_lacp_vars;

static void finish_work(void);

#define LACP_IS_UNINITED (!LACP->initialized)
#define LACP_INIT       do { if (LACP_IS_UNINITED) lacp_sw_init(); finish_work(); } while (0);

static const char *vtss_lacp_str_portstate(vtss_lacp_portstate_t pst)
{
    static char buf[128];

    buf[0] = '\0';
    if (pst & VTSS_LACP_PORTSTATE_LACP_ACTIVITY) {
        strcat(buf, "activity ");
    }
    if (pst & VTSS_LACP_PORTSTATE_LACP_TIMEOUT) {
        strcat(buf, "timeout ");
    }
    if (pst & VTSS_LACP_PORTSTATE_AGGREGATION) {
        strcat(buf, "aggregation ");
    }
    if (pst & VTSS_LACP_PORTSTATE_SYNCHRONIZATION) {
        strcat(buf, "synchronization ");
    }
    if (pst & VTSS_LACP_PORTSTATE_COLLECTING) {
        strcat(buf, "collecting ");
    }
    if (pst & VTSS_LACP_PORTSTATE_DISTRIBUTING) {
        strcat(buf, "distributing ");
    }
    if (pst & VTSS_LACP_PORTSTATE_DEFAULTED) {
        strcat(buf, "defaulted ");
    }
    if (pst & VTSS_LACP_PORTSTATE_EXPIRED) {
        strcat(buf, "expired ");
    }
    return buf;
}

static const char *vtss_lacp_str_sm(vtss_lacp_sm_t sm)
{
    static char buf[128];

    buf[0] = '\0';
    if (sm & VTSS_LACP_PORT_BEGIN) {
        strcat(buf, "begin ");
    }
    if (sm & VTSS_LACP_PORT_LACP_ENABLED) {
        strcat(buf, "lacp_enabled ");
    }
    if (sm & VTSS_LACP_PORT_ACTOR_CHURN) {
        strcat(buf, "actor_churn ");
    }
    if (sm & VTSS_LACP_PORT_PARTNER_CHURN) {
        strcat(buf, "partner_churn ");
    }
    if (sm & VTSS_LACP_PORT_READY) {
        strcat(buf, "ready ");
    }
    if (sm & VTSS_LACP_PORT_READY_N) {
        strcat(buf, "ready_n ");
    }
    if (sm & VTSS_LACP_PORT_MATCHED) {
        strcat(buf, "matched ");
    }
    if (sm & VTSS_LACP_PORT_STANDBY) {
        strcat(buf, "standby ");
    }
    if (sm & VTSS_LACP_PORT_SELECTED) {
        strcat(buf, "selected ");
    }
    if (sm & VTSS_LACP_PORT_MOVED) {
        strcat(buf, "moved ");
    }
    return buf;
}
#ifndef VTSS_LACP_NDEBUG
#endif /* !VTSS_LACP_NDEBUG */

/**
 * __ad_timer_to_ticks - convert a given timer type to vtss_os ticks
 * @timer_type: which timer to operate
 * @par: timer parameter. see below
 *
 * If @timer_type is %current_while_timer, @par indicates long/short timer.
 * If @timer_type is %periodic_timer, @par is one of %FAST_PERIODIC_TIME,
 *                          %SLOW_PERIODIC_TIME.
 */
static vtss_lacp_tcount_t timer2ticks(vtss_lacp_timer_id_t timer_type, vtss_lacp_time_interval_t par)
{
    vtss_lacp_tcount_t retval = 0; /* to silence the compiler */

    switch (timer_type) {
    case VTSS_LACP_TID_CURRENT_WHILE_TIMER : /* for rx machine usage */
        if (par) { /* for short or long timeout */
            retval = VTSS_LACP_SHORT_TIMEOUT_TIME * VTSS_LACP_TICKS_PER_SEC;    /* short timeout */
        } else {
            retval = VTSS_LACP_LONG_TIMEOUT_TIME * VTSS_LACP_TICKS_PER_SEC;    /* long timeout */
        }
        break;
    case VTSS_LACP_TID_ACTOR_CHURN_TIMER : /* for local churn machine */
        retval = VTSS_LACP_CHURN_DETECTION_TIME * VTSS_LACP_TICKS_PER_SEC;
        break;
    case VTSS_LACP_TID_PERIODIC_TIMER : /* for periodic machine */
        retval = par * VTSS_LACP_TICKS_PER_SEC; /* long timeout */
        break;
    case VTSS_LACP_TID_PARTNER_CHURN_TIMER : /* for remote churn machine */
        retval = VTSS_LACP_CHURN_DETECTION_TIME * VTSS_LACP_TICKS_PER_SEC;
        break;
    case VTSS_LACP_TID_WAIT_WHILE_TIMER : /* for selection machine */
        retval = VTSS_LACP_AGGREGATE_WAIT_TIME * VTSS_LACP_TICKS_PER_SEC;
        break;
    }
    return retval;
}

#ifdef VTSS_LACP_USE_MARKER
static vtss_lacp_tcount_t colldelay2ticks(vtss_lacp_port_vars_t *pp)
{
    vtss_lacp_tcount_t r;

    r = pp->collmaxdelay / 10000;
    if (r == 0) {
        return 1;
    }
    if (r > VTSS_LACP_COLLMAXDELAY_MAX) {
        return VTSS_LACP_COLLMAXDELAY_MAX;
    }
    return r;
}
#endif /* VTSS_LACP_USE_MARKER */

static void fill_lacpheader(vtss_lacp_frame_header_t *pdu, vtss_lacp_port_vars_t *pp, vtss_common_octet_t subtype)
{
    VTSS_COMMON_MACADDR_ASSIGN(pdu->dst_mac, VTSS_LACP_LACPMAC);
    VTSS_COMMON_MACADDR_ASSIGN(pdu->src_mac, pp->port_macaddr.macaddr);
    VTSS_COMMON_UNALIGNED_PUT_2B(pdu->eth_type, HOST2NETS(VTSS_LACP_ETHTYPE));
    pdu->subtype = subtype;
    pdu->version = VTSS_LACP_VERSION_NO;
}

#ifdef VTSS_LACP_USE_MARKER
static int markerpdu_send(vtss_lacp_port_vars_t *pp)
{
    vtss_lacp_markerpdu_t *markerpdu;
    vtss_common_bufref_t bufref;

    markerpdu = vtss_os_alloc_xmit(pp->actor_port_number + VTSS_PORT_NO_START - 1, sizeof(vtss_lacp_markerpdu_t), &bufref);
    if (markerpdu == NULL) {
        return VTSS_COMMON_CC_GENERR;
    }

    /* Fill out constant fields in the PDU buffer */
    fill_lacpheader((vtss_lacp_frame_header_t *)markerpdu->frame_header, pp, VTSS_LACP_SUBTYPE_MARK);
    markerpdu->tvl_type_marker_info = VTSS_LACP_TVLTYPE_MARKER_INFO;
    markerpdu->tvl_length_marker_info = VTSS_LACP_TVLLEN_MARKER_INFO;
    VTSS_COMMON_UNALIGNED_PUT_2B(markerpdu->requester_port, HOST2NETS(pp->actor_port_number));
    VTSS_COMMON_MACADDR_ASSIGN(markerpdu->requester_system_macaddr, LACP->system_config.system_id.macaddr);
    markerpdu->requester_transaction_id[0] = ++pp->marker_transid;
    markerpdu->requester_transaction_id[1] = 0;
    markerpdu->requester_transaction_id[2] = 0;
    markerpdu->requester_transaction_id[3] = 0;
    markerpdu->tvl_type_terminator = VTSS_LACP_TVLTYPE_TERMINATOR;
    markerpdu->tvl_length_terminator = VTSS_LACP_TVLLEN_TERMINATOR;
    VTSS_LACP_TRACE(VTSS_LACP_TRLVL_DEBUG, ("Sent MARKERPDU (req id 0x%x) on port %u\n",
                                            (unsigned)markerpdu->requester_transaction_id[0],
                                            (unsigned)pp->actor_port_number));
    pp->stats.markreq_frame_xmits++;
    return vtss_os_xmit(pp->actor_port_number + VTSS_PORT_NO_START - 1, markerpdu, sizeof(vtss_lacp_markerpdu_t), bufref);
}
#endif /* VTSS_LACP_USE_MARKER */

static void disable_collecting_distributing(vtss_lacp_port_vars_t *pp)
{
    if (pp->hw_aggregator) {
        VTSS_LACP_TRACE(VTSS_LACP_TRLVL_DEBUG, ("Disable hwaggr %u port %u\n",
                                                (unsigned)pp->hw_aggregator->aggregator_identifier,
                                                (unsigned)pp->actor_port_number));
#ifdef VTSS_LACP_USE_MARKER
        if (pp->mark_reply_timer_counter == 0) {
            pp->hw_aggregator->last_change = LACP->ticks_since_start;
            vtss_os_clear_hwaggr(pp->hw_aggregator->aggregator_identifier + VTSS_PORT_NO_START - 1, pp->actor_port_number + VTSS_PORT_NO_START - 1);
        } else {
            VTSS_LACP_TRACE(VTSS_LACP_TRLVL_DEBUG, ("Disable hwaggr %u port %u canceling outstanding enable\n",
                                                    (unsigned)pp->hw_aggregator->aggregator_identifier,
                                                    (unsigned)pp->actor_port_number));
            pp->mark_reply_timer_counter = 0; /* Stop timer */
        }
        pp->marker_transid++; /* Invalidate old transid */
#else
        pp->hw_aggregator->last_change = LACP->ticks_since_start;
        vtss_os_clear_hwaggr(pp->hw_aggregator->aggregator_identifier + VTSS_PORT_NO_START - 1, pp->actor_port_number + VTSS_PORT_NO_START - 1);
#endif /* VTSS_LACP_USE_MARKER */
        pp->hw_aggregator = NULL;
    }
}

#ifdef VTSS_LACP_USE_MARKER
static void got_mark_reply_or_tmo(vtss_lacp_port_vars_t *pp, vtss_common_bool_t got_reply)
{
#ifndef VTSS_LACP_NDEBUG
    static const char *because[] = { "tmo", "reply", "forced" };
#endif /* !VTSS_LACP_NDEBUG */

    if (got_reply || (pp->mark_reply_timer_counter && --pp->mark_reply_timer_counter == 0)) {
        VTSS_LACP_TRACE(got_reply == VTSS_COMMON_BOOL_TRUE ? VTSS_LACP_TRLVL_DEBUG : VTSS_LACP_TRLVL_WARNING,
                        ("Delayed enable hwaggr %u port %u: %s\n",
                         (unsigned)pp->hw_aggregator->aggregator_identifier,
                         (unsigned)pp->actor_port_number,
                         because[got_reply]));
        pp->mark_reply_timer_counter = 0;
        pp->hw_aggregator->last_change = LACP->ticks_since_start;
        vtss_os_set_hwaggr(pp->hw_aggregator->aggregator_identifier + VTSS_PORT_NO_START - 1, pp->actor_port_number + VTSS_PORT_NO_START - 1);
        vtss_os_set_fwdstate(pp->actor_port_number + VTSS_PORT_NO_START - 1, VTSS_COMMON_FWDSTATE_ENABLED);
    }
}
#endif /* VTSS_LACP_USE_MARKER */

static void enable_collecting_distributing(vtss_lacp_port_vars_t *pp)
{
    VTSS_LACP_ASSERT(pp->hw_aggregator == NULL);
    pp->hw_aggregator = pp->aggregator;
#ifdef VTSS_LACP_USE_MARKER
    if (pp->mark_reply_timer_counter == 0) {
        VTSS_LACP_TRACE(VTSS_LACP_TRLVL_DEBUG, ("Enable hwaggr %u port %u timer started\n",
                                                (unsigned)pp->hw_aggregator->aggregator_identifier,
                                                (unsigned)pp->actor_port_number));
        vtss_os_set_fwdstate(pp->actor_port_number + VTSS_PORT_NO_START - 1, VTSS_COMMON_FWDSTATE_DISABLED); /* Silence while we switch over */
        if (markerpdu_send(pp) == VTSS_COMMON_CC_OK) {
            pp->mark_reply_timer_counter = colldelay2ticks(pp);
        } else { /* Could not transmit buffer - so fake an answer */
            got_mark_reply_or_tmo(pp, 2);
        }
    }
#else
    pp->aggregator->last_change = LACP->ticks_since_start;
    vtss_os_set_hwaggr(pp->hw_aggregator->aggregator_identifier + VTSS_PORT_NO_START - 1, pp->actor_port_number + VTSS_PORT_NO_START - 1);
#endif /* VTSS_LACP_USE_MARKER */
}

static void start_partner_churn(vtss_lacp_port_vars_t *pp)
{
    pp->sm_partner_churn_state = VTSS_LACP_CHURNSTATE_MONITOR;
    pp->sm_pc_timer_counter = timer2ticks(VTSS_LACP_TID_PARTNER_CHURN_TIMER, 0);
}

static void stop_partner_churn(vtss_lacp_port_vars_t *pp)
{
    pp->sm_partner_churn_state = VTSS_LACP_CHURNSTATE_NONE;
    pp->sm_pc_timer_counter = 0;
}

static void record_default(vtss_lacp_port_vars_t *pp)
{
    // record the partner admin parameters
    pp->partner_oper_port_number = pp->partner_admin_port_number;
    pp->partner_oper_port_priority = pp->partner_admin_port_priority;
    pp->partner_oper_system = pp->partner_admin_system;
    pp->partner_oper_system_priority = pp->partner_admin_system_priority;
    pp->partner_oper_key = pp->partner_admin_key;
    pp->partner_oper_port_state = pp->partner_admin_port_state;

    // set actor_oper_port_state.defaulted to true
    pp->actor_oper_port_state |= VTSS_LACP_PORTSTATE_DEFAULTED;
}

static void rx_update(const vtss_lacp_lacpdu_t *lacpdu,
                      vtss_lacp_port_vars_t *pp)
{
#define actorinfo   ((const vtss_lacp_info_t *)lacpdu->actor_info)
#define partnerinfo ((const vtss_lacp_info_t *)lacpdu->partner_info)

    if (lacpdu->tvl_length_partner != VTSS_LACP_TVLLEN_PARTNER_INFO ||
        lacpdu->tvl_length_actor != VTSS_LACP_TVLLEN_ACTOR_INFO ||
        lacpdu->tvl_length_collector != VTSS_LACP_TVLLEN_COLLECTOR) {
        record_default(pp);
    } else if (UNAL_NET2HOSTS(actorinfo->port) != pp->partner_oper_port_number ||
               UNAL_NET2HOSTS(actorinfo->port_priority) != pp->partner_oper_port_priority ||
               VTSS_COMMON_MACADDR_CMP(actorinfo->system_macaddr, pp->partner_oper_system.macaddr) ||
               UNAL_NET2HOSTS(actorinfo->system_priority) != pp->partner_oper_system_priority ||
               UNAL_NET2HOSTS(actorinfo->key) != pp->partner_oper_key ||
               (actorinfo->state & VTSS_LACP_PORTSTATE_AGGREGATION) != (pp->partner_oper_port_state & VTSS_LACP_PORTSTATE_AGGREGATION)) {
        /* update the state machine Selected variable */

        pp->sm_vars &= ~VTSS_LACP_PORT_SELECTED;
        pp->sm_vars &= ~VTSS_LACP_PORT_STANDBY;
        /* record the new parameter values for the partner operational */
        pp->partner_oper_port_number = UNAL_NET2HOSTS(actorinfo->port);
        pp->partner_oper_port_priority = UNAL_NET2HOSTS(actorinfo->port_priority);
        VTSS_COMMON_MACADDR_ASSIGN(pp->partner_oper_system.macaddr, actorinfo->system_macaddr);
        pp->partner_oper_system_priority = UNAL_NET2HOSTS(actorinfo->system_priority);
        pp->partner_oper_key = UNAL_NET2HOSTS(actorinfo->key);
    }


    /* check if any parameter is different */
    if (vtss_os_translate_port(UNAL_NET2HOSTS(partnerinfo->port) + VTSS_PORT_NO_START - 1, 0) != pp->actor_port_number ||
        UNAL_NET2HOSTS(partnerinfo->port_priority) != pp->port_config.port_prio ||
#if defined(HP_PROCURVE_HW)
        VTSS_COMMON_MACADDR_CMP(partnerinfo->system_macaddr, pp->port_macaddr.macaddr) ||
#else
//        VTSS_COMMON_MACADDR_CMP(partnerinfo->system_macaddr, pp->port_macaddr.macaddr) ||
        VTSS_COMMON_MACADDR_CMP(partnerinfo->system_macaddr, LACP->system_config.system_id.macaddr) ||
#endif
        UNAL_NET2HOSTS(partnerinfo->system_priority) != LACP->system_config.system_prio ||
        UNAL_NET2HOSTS(partnerinfo->key) != pp->actor_oper_port_key ||
        (partnerinfo->state & (VTSS_LACP_PORTSTATE_LACP_ACTIVITY |
                               VTSS_LACP_PORTSTATE_LACP_TIMEOUT |
                               VTSS_LACP_PORTSTATE_SYNCHRONIZATION |
                               VTSS_LACP_PORTSTATE_AGGREGATION)) !=
        (pp->partner_oper_port_state & (VTSS_LACP_PORTSTATE_LACP_ACTIVITY |
                                        VTSS_LACP_PORTSTATE_LACP_TIMEOUT |
                                        VTSS_LACP_PORTSTATE_SYNCHRONIZATION |
                                        VTSS_LACP_PORTSTATE_AGGREGATION))) {
        pp->ntt = VTSS_COMMON_BOOL_TRUE;
    }

    /* zero partener's last states */
    pp->partner_oper_port_state = (actorinfo->state & (VTSS_LACP_PORTSTATE_LACP_ACTIVITY |
                                                       VTSS_LACP_PORTSTATE_LACP_TIMEOUT |
                                                       VTSS_LACP_PORTSTATE_AGGREGATION |
                                                       VTSS_LACP_PORTSTATE_SYNCHRONIZATION |
                                                       VTSS_LACP_PORTSTATE_COLLECTING |
                                                       VTSS_LACP_PORTSTATE_DISTRIBUTING |
                                                       VTSS_LACP_PORTSTATE_DEFAULTED |
                                                       VTSS_LACP_PORTSTATE_EXPIRED));

    /* set actor_oper_port_state.defaulted to FALSE */
    pp->actor_oper_port_state &= ~VTSS_LACP_PORTSTATE_DEFAULTED;

    /* set the partner sync. to on if the partner is sync. and the port is matched */
    if ((pp->sm_vars & VTSS_LACP_PORT_MATCHED) && (actorinfo->state & VTSS_LACP_PORTSTATE_SYNCHRONIZATION)) {
        pp->partner_oper_port_state |= VTSS_LACP_PORTSTATE_SYNCHRONIZATION;
        stop_partner_churn(pp);
    } else {
        pp->partner_oper_port_state &= ~VTSS_LACP_PORTSTATE_SYNCHRONIZATION;
    }

    /* check if all parameters are alike */
    if ((vtss_os_translate_port(UNAL_NET2HOSTS(partnerinfo->port) + VTSS_PORT_NO_START - 1, 0) == pp->actor_port_number &&
         UNAL_NET2HOSTS(partnerinfo->port_priority) == pp->port_config.port_prio &&
         !VTSS_COMMON_MACADDR_CMP(partnerinfo->system_macaddr, LACP->system_config.system_id.macaddr) &&
         UNAL_NET2HOSTS(partnerinfo->system_priority) == LACP->system_config.system_prio &&
         UNAL_NET2HOSTS(partnerinfo->key) == pp->actor_oper_port_key &&
         (partnerinfo->state & VTSS_LACP_PORTSTATE_AGGREGATION) == (pp->actor_oper_port_state & VTSS_LACP_PORTSTATE_AGGREGATION)) ||
        /* or this is individual link(aggregation == FALSE) */
        ((actorinfo->state & VTSS_LACP_PORTSTATE_AGGREGATION) == 0)) {
        /* update the state machine Matched variable */
        pp->sm_vars |= VTSS_LACP_PORT_MATCHED;
    } else {
#if 0
        vtss_printf("No MATCH: lacpdu partner port %u actor port %u\n",
                    (unsigned)NET2HOSTS(partnerinfo->port),
                    (unsigned)pp->actor_port_number);
        vtss_printf("NO MATCH: lacpdu partner port prio %u actor port prio %u\n",
                    (unsigned)NET2HOSTS(partnerinfo->port_priority),
                    (unsigned)pp->port_config.port_prio);
        vtss_printf("NO MATCH: lacpdu partner MAC %s",
                    vtss_common_str_macaddr(&partnerinfo->system_macaddr));
        vtss_printf(" actor MAC %s\n",
                    vtss_common_str_macaddr(&LACP->system_config.system_id));
        vtss_printf("NO MATCH: lacpdu partner sys prio %u actor sys prio %u\n",
                    (unsigned)NET2HOSTS(partnerinfo->system_priority),
                    (unsigned)LACP->system_config.system_prio);
        vtss_printf("NO MATCH: lacpdu partner key %u actor key %u\n",
                    (unsigned)NET2HOSTS(partnerinfo->key),
                    (unsigned)pp->actor_oper_port_key);
        vtss_printf("NO MATCH: lacpdu partner state: 0x%x %s\n",
                    (unsigned)partnerinfo->state,
                    vtss_lacp_str_portstate(partnerinfo->state));
        vtss_printf("          actor state: 0x%x %s\n",
                    (unsigned)pp->actor_oper_port_state,
                    vtss_lacp_str_portstate(pp->actor_oper_port_state));
        vtss_printf("          lacpdu actor state: 0x%x %s\n",
                    (unsigned)lacpdu->actorinfo.state,
                    vtss_lacp_str_portstate(actorinfo->state));
        vtss_common_dump_frame((const vtss_common_octet_t VTSS_COMMON_PTR_ATTRIB *)lacpdu, sizeof(*lacpdu));
#endif
        pp->sm_vars &= ~VTSS_LACP_PORT_MATCHED;
    }

    pp->collmaxdelay = UNAL_NET2HOSTS(lacpdu->collector_max_delay);
    if (pp->collmaxdelay)
        VTSS_LACP_TRACE(VTSS_LACP_TRLVL_NOISE, ("port %u: coll max delay = %u\n",
                                                (unsigned)pp->actor_port_number,
                                                (unsigned)pp->collmaxdelay));
#undef partnerinfo
#undef actorinfo
}

/**
 * update_default_selected - update a port's Selected variable from Partner
 * @port: the port we're looking at
 *
 * This function updates the value of the selected variable, using the partner
 * administrative parameter values. The administrative values are compared with
 * the corresponding operational parameter values for the partner. If one or
 * more of the comparisons shows that the administrative value(s) differ from
 * the current operational values, then Selected is set to FALSE and
 * actor_oper_port_state.synchronization is set to OUT_OF_SYNC. Otherwise,
 * Selected remains unchanged.
 */
static void update_default_selected(vtss_lacp_port_vars_t *pp)
{
    /* check if any parameter is different */
    if (pp->partner_admin_port_number != pp->partner_oper_port_number ||
        pp->partner_admin_port_priority != pp->partner_oper_port_priority ||
        VTSS_COMMON_MACADDR_CMP(pp->partner_admin_system.macaddr, pp->partner_oper_system.macaddr) ||
        pp->partner_admin_system_priority != pp->partner_oper_system_priority ||
        pp->partner_admin_key != pp->partner_oper_key ||
        (pp->partner_admin_port_state & VTSS_LACP_PORTSTATE_AGGREGATION) != (pp->partner_oper_port_state & VTSS_LACP_PORTSTATE_AGGREGATION))
        /* update the state machine Selected variable */
    {
        pp->sm_vars &= ~VTSS_LACP_PORT_SELECTED;
    }
}

/**
 * agg_ports_are_ready - check if all ports in an aggregator are ready
 * @aggregator: the aggregator we're looking at
 *
 */
static vtss_common_bool_t all_aggr_ports_ready(vtss_lacp_aggregator_vars_t *aggregator)
{
    vtss_lacp_port_vars_t *pp;

    /* scan all ports in this aggregator to verfy if they are all ready */
    for (pp = aggregator->lag_ports; pp; pp = pp->next_lag_port)
        if (!(pp->sm_vars & VTSS_LACP_PORT_READY_N)) {
            return VTSS_COMMON_BOOL_FALSE;
        }
    return VTSS_COMMON_BOOL_TRUE;
}

/**
 * set_all_aggr_ports - set value of Ready bit in all ports of an aggregator
 * @aggregator: the aggregator we're looking at
 * @val: Should the ports' ready bit be set on or off
 *
 */
static void set_all_aggr_ports(vtss_lacp_aggregator_vars_t *aggregator, vtss_common_bool_t val)
{
    vtss_lacp_port_vars_t *pp;

    for (pp = aggregator->lag_ports; pp; pp = pp->next_lag_port)
        if (val) {
            pp->sm_vars |= VTSS_LACP_PORT_READY;
        } else {
            pp->sm_vars &= ~VTSS_LACP_PORT_READY;
        }
}

static int lacpdu_send(vtss_lacp_port_vars_t *pp)
{
#define actorinfo   ((vtss_lacp_info_t *)lacpdu->actor_info)
#define partnerinfo ((vtss_lacp_info_t *)lacpdu->partner_info)
    vtss_lacp_lacpdu_t *lacpdu;
    vtss_common_bufref_t bufref;
    vtss_common_octet_t  macaddr[VTSS_COMMON_MACADDR_SIZE];

    VTSS_COMMON_MACADDR_ASSIGN(macaddr, LACP->system_config.system_id.macaddr);

    lacpdu = vtss_os_alloc_xmit(pp->actor_port_number + VTSS_PORT_NO_START - 1, sizeof(vtss_lacp_lacpdu_t), &bufref);
    if (lacpdu == NULL) {
        return VTSS_COMMON_CC_GENERR;
    }

    /* Fill out header fields in the PDU buffer */
    fill_lacpheader((vtss_lacp_frame_header_t *)lacpdu->frame_header, pp, VTSS_LACP_SUBTYPE_LACP);

    /* update current actual Actor parameters */
    lacpdu->tvl_type_actor = VTSS_LACP_TVLTYPE_ACTOR_INFO;
    lacpdu->tvl_length_actor = VTSS_LACP_TVLLEN_ACTOR_INFO;
    VTSS_COMMON_UNALIGNED_PUT_2B(actorinfo->system_priority, HOST2NETS(LACP->system_config.system_prio));
    VTSS_COMMON_MACADDR_ASSIGN(actorinfo->system_macaddr, LACP->system_config.system_id.macaddr);
    VTSS_COMMON_MACADDR_ASSIGN(actorinfo->system_macaddr, macaddr);
    VTSS_COMMON_UNALIGNED_PUT_2B(actorinfo->key, HOST2NETS(pp->actor_oper_port_key));
    VTSS_COMMON_UNALIGNED_PUT_2B(actorinfo->port_priority, HOST2NETS(pp->port_config.port_prio));
    VTSS_COMMON_UNALIGNED_PUT_2B(actorinfo->port, HOST2NETS(vtss_os_translate_port(pp->actor_port_number + VTSS_PORT_NO_START - 1, 1)));
    actorinfo->state = pp->actor_oper_port_state;
    actorinfo->reserved[0] = 0;
    actorinfo->reserved[1] = 0;
    actorinfo->reserved[2] = 0;

    /* update current actual Partner parameters */
    lacpdu->tvl_type_partner = VTSS_LACP_TVLTYPE_PARTNER_INFO;
    lacpdu->tvl_length_partner = VTSS_LACP_TVLLEN_PARTNER_INFO;
    VTSS_COMMON_UNALIGNED_PUT_2B(partnerinfo->system_priority, HOST2NETS(pp->partner_oper_system_priority));
    VTSS_COMMON_MACADDR_ASSIGN(partnerinfo->system_macaddr, pp->partner_oper_system.macaddr);
    VTSS_COMMON_UNALIGNED_PUT_2B(partnerinfo->key, HOST2NETS(pp->partner_oper_key));
    VTSS_COMMON_UNALIGNED_PUT_2B(partnerinfo->port_priority, HOST2NETS(pp->partner_oper_port_priority));
    VTSS_COMMON_UNALIGNED_PUT_2B(partnerinfo->port, HOST2NETS(pp->partner_oper_port_number));
    partnerinfo->state = pp->partner_oper_port_state;
    partnerinfo->reserved[0] = 0;
    partnerinfo->reserved[1] = 0;
    partnerinfo->reserved[2] = 0;

    lacpdu->tvl_type_collector = VTSS_LACP_TVLTYPE_COLLECTOR;
    lacpdu->tvl_length_collector = VTSS_LACP_TVLLEN_COLLECTOR;
    VTSS_COMMON_UNALIGNED_PUT_2B(lacpdu->collector_max_delay, HOST2NETS(VTSS_LACP_COLLECTOR_MAX_DELAY));
    memset(lacpdu->reserved, 0, sizeof(lacpdu->reserved));

    lacpdu->tvl_type_terminator = VTSS_LACP_TVLTYPE_TERMINATOR;
    lacpdu->tvl_length_terminator = VTSS_LACP_TVLLEN_TERMINATOR;
    memset(lacpdu->reserved2, 0, sizeof(lacpdu->reserved2));

    VTSS_LACP_TRACE(VTSS_LACP_TRLVL_NOISE, ("Sent LACPDU on port %d\n",
                                            (int)pp->actor_port_number));
    VTSS_LACP_TRACE(VTSS_LACP_TRLVL_NOISE, ("actor system %s key 0x%x port state = 0x%x %s\n",
                                            vtss_common_str_macaddr((const vtss_common_macaddr_t VTSS_COMMON_PTR_ATTRIB *)actorinfo->system_macaddr),
                                            (unsigned)actorinfo->key, (unsigned)actorinfo->state,
                                            vtss_lacp_str_portstate(actorinfo->state)));
    VTSS_LACP_TRACE(VTSS_LACP_TRLVL_NOISE, ("partner system %s key 0x%x port state = 0x%x %s\n",
                                            vtss_common_str_macaddr((const vtss_common_macaddr_t VTSS_COMMON_PTR_ATTRIB *)partnerinfo->system_macaddr),
                                            (unsigned)partnerinfo->key, (unsigned)partnerinfo->state,
                                            vtss_lacp_str_portstate(partnerinfo->state)));
    pp->stats.lacp_frame_xmits++;
    return vtss_os_xmit(pp->actor_port_number + VTSS_PORT_NO_START - 1, lacpdu, sizeof(vtss_lacp_lacpdu_t), bufref);
#undef partnerinfo
#undef actorinfo
}

static void handle_markerframe(vtss_lacp_port_vars_t *pp,
                               const vtss_lacp_markerpdu_t *markerpdu)
{
    vtss_lacp_markerpdu_t *mpdu;
    vtss_common_bufref_t bufref;
#define frm_head ((vtss_lacp_frame_header_t *)mpdu->frame_header)

    switch (markerpdu->tvl_type_marker_info) {
    case VTSS_LACP_TVLTYPE_MARKER_INFO :
        pp->stats.markreq_frame_recvs++;
        mpdu = vtss_os_alloc_xmit(pp->actor_port_number + VTSS_PORT_NO_START - 1, sizeof(vtss_lacp_markerpdu_t), &bufref);
        if (mpdu == NULL) {
            return;
        }
        if (mpdu != markerpdu) { /* Some environments reuse the same buffer */
            *mpdu = *markerpdu;    /* Just bounce it back */
        }
        VTSS_COMMON_MACADDR_ASSIGN(frm_head->dst_mac, VTSS_LACP_LACPMAC);
        VTSS_COMMON_MACADDR_ASSIGN(frm_head->src_mac, pp->port_macaddr.macaddr);
        mpdu->tvl_type_marker_info = VTSS_LACP_TVLTYPE_MARKER_RESPONS_INFO;
        VTSS_LACP_TRACE(VTSS_LACP_TRLVL_DEBUG, ("Sent MARKERPDU (resp 0x%x) on port %u\n",
                                                (unsigned)markerpdu->requester_transaction_id[0],
                                                (unsigned)pp->actor_port_number));
        pp->stats.markresp_frame_xmits++;
        vtss_os_xmit(pp->actor_port_number + VTSS_PORT_NO_START - 1, mpdu->frame_header, sizeof(vtss_lacp_markerpdu_t), bufref);
        break;
    case VTSS_LACP_TVLTYPE_MARKER_RESPONS_INFO :
        pp->stats.markresp_frame_recvs++;
#ifdef VTSS_LACP_USE_MARKER
        if (UNAL_NET2HOSTS(markerpdu->requester_port) == pp->actor_port_number &&
            VTSS_COMMON_MACADDR_CMP(markerpdu->requester_system_macaddr, LACP->system_config.system_id.macaddr) == 0 &&
            markerpdu->requester_transaction_id[0] == pp->marker_transid &&
            markerpdu->requester_transaction_id[1] == 0 &&
            markerpdu->requester_transaction_id[2] == 0 &&
            markerpdu->requester_transaction_id[3] == 0) {
            got_mark_reply_or_tmo(pp, VTSS_COMMON_BOOL_TRUE);
        } else {
            VTSS_LACP_TRACE(VTSS_LACP_TRLVL_DEBUG, ("Ignored bad marker resp: got (%u,%s,%u)\n",
                                                    (unsigned)UNAL_NET2HOSTS(markerpdu->requester_port),
                                                    vtss_common_str_macaddr((const vtss_common_macaddr_t VTSS_COMMON_PTR_ATTRIB *)markerpdu->requester_system_macaddr),
                                                    markerpdu->requester_transaction_id[0]));
            VTSS_LACP_TRACE(VTSS_LACP_TRLVL_DEBUG, ("But wanted (%u,%s,%u)\n",
                                                    (unsigned)pp->actor_port_number,
                                                    vtss_common_str_macaddr(&LACP->system_config.system_id),
                                                    pp->marker_transid));
        }
#endif /* VTSS_LACP_USE_MARKER */
        break;
    default :
        pp->stats.illegal_frame_recvs++;
        VTSS_LACP_TRACE(VTSS_LACP_TRLVL_DEBUG, ("Ignored bad marker resp from port %u tvl type 0x%x\n",
                                                (unsigned)pp->actor_port_number,
                                                (unsigned)markerpdu->tvl_type_marker_info));
        break;
    }
#undef frm_head
}

static void rx_stev(vtss_lacp_port_vars_t *pp, const vtss_lacp_lacpdu_t *lacpdu)
{
    vtss_lacp_rx_state_t last_state;

    /* keep current State Machine state to compare later if it was changed */
    last_state = pp->sm_rx_state;

    /* check if state machine should change state */
    /* first, check if port was reinitialized */
    if (pp->sm_vars & VTSS_LACP_PORT_BEGIN) {
        pp->sm_rx_state = VTSS_LACP_RXSTATE_INITIALIZE;    /* next state */
    }
    /* check if port is not enabled */
    else if (!(pp->sm_vars & VTSS_LACP_PORT_BEGIN) && !pp->port_up && !(pp->sm_vars & VTSS_LACP_PORT_MOVED)) {
        pp->sm_rx_state = VTSS_LACP_RXSTATE_PORT_DISABLED;    /* next state */
    }
    /* check if new lacpdu arrived */
    else if (lacpdu && ((pp->sm_rx_state == VTSS_LACP_RXSTATE_EXPIRED) || (pp->sm_rx_state == VTSS_LACP_RXSTATE_DEFAULTED) || (pp->sm_rx_state == VTSS_LACP_RXSTATE_CURRENT))) {
        pp->sm_rx_timer_counter = 0; /* zero timer */
        pp->sm_rx_state = VTSS_LACP_RXSTATE_CURRENT;
    } else {
        /* if timer is on, and if it is expired */
        if (pp->sm_rx_timer_counter && !(--pp->sm_rx_timer_counter)) {
            switch (pp->sm_rx_state) {
            case VTSS_LACP_RXSTATE_EXPIRED :
                pp->sm_rx_state = VTSS_LACP_RXSTATE_DEFAULTED; /* next state */
                break;
            case VTSS_LACP_RXSTATE_CURRENT :
                pp->sm_rx_state = VTSS_LACP_RXSTATE_EXPIRED; /* next state */
                break;
            default : /* to silence the compiler */
                break;
            }
        } else {
            /* if no lacpdu arrived and no timer is on */
            switch (pp->sm_rx_state) {
            case VTSS_LACP_RXSTATE_PORT_DISABLED :
                if (pp->sm_vars & VTSS_LACP_PORT_MOVED) {
                    pp->sm_rx_state = VTSS_LACP_RXSTATE_INITIALIZE;    /* next state */
                } else if (pp->port_up && (pp->sm_vars & VTSS_LACP_PORT_LACP_ENABLED)) {
                    pp->sm_rx_state = VTSS_LACP_RXSTATE_EXPIRED;    /* next state */
                } else if (pp->port_up && ((pp->sm_vars & VTSS_LACP_PORT_LACP_ENABLED) == 0)) {
                    pp->sm_rx_state = VTSS_LACP_RXSTATE_LACP_DISABLED;    /* next state */
                }
                break;
            default :    /* to silence the compiler */
                break;
            }
        }
    }

    /* check if the State machine was changed or new lacpdu arrived */
    if (pp->sm_rx_state != last_state || lacpdu) {
        VTSS_LACP_TRACE(VTSS_LACP_TRLVL_NOISE, ("Rx Machine: Port=%u, Last State=%u, Curr State=%u smvars 0x%x %s\n",
                                                (unsigned)pp->actor_port_number, (unsigned)last_state,
                                                (unsigned)pp->sm_rx_state, (unsigned)pp->sm_vars,
                                                vtss_lacp_str_sm(pp->sm_vars)));
        switch (pp->sm_rx_state) {
        case VTSS_LACP_RXSTATE_INITIALIZE :
            if (!pp->port_config.enable_lacp || pp->duplex_mode == VTSS_COMMON_LINKDUPLEX_HALF) {
                pp->sm_vars &= ~VTSS_LACP_PORT_LACP_ENABLED;
            } else {
                start_partner_churn(pp);
                pp->sm_vars |= VTSS_LACP_PORT_LACP_ENABLED;
            }
            pp->sm_vars &= ~VTSS_LACP_PORT_SELECTED;
            record_default(pp);
            pp->actor_oper_port_state &= ~VTSS_LACP_PORTSTATE_EXPIRED;
            pp->sm_vars &= ~VTSS_LACP_PORT_MOVED;
            pp->sm_rx_state = VTSS_LACP_RXSTATE_PORT_DISABLED;  /* next state */

        /*- Fall Through -*/

        case VTSS_LACP_RXSTATE_PORT_DISABLED :
            pp->sm_vars &= ~VTSS_LACP_PORT_MATCHED;
            break;
        case VTSS_LACP_RXSTATE_LACP_DISABLED :
            pp->sm_vars &= ~VTSS_LACP_PORT_SELECTED;
            record_default(pp);
            pp->partner_oper_port_state &= ~VTSS_LACP_PORTSTATE_AGGREGATION;
            pp->sm_vars |= VTSS_LACP_PORT_MATCHED;
            pp->actor_oper_port_state &= ~VTSS_LACP_PORTSTATE_EXPIRED;
            break;
        case VTSS_LACP_RXSTATE_EXPIRED :
            /* Reset of the Synchronization flag. (Standard 43.4.12) */
            /* This reset cause to disable this port in the COLLECTING_DISTRIBUTING state of the */
            /* mux machine in case of EXPIRED even if LINK_DOWN didn't arrive for the port. */
            pp->partner_oper_port_state &= ~VTSS_LACP_PORTSTATE_SYNCHRONIZATION;
            pp->sm_vars &= ~VTSS_LACP_PORT_MATCHED;
            pp->partner_oper_port_state |= VTSS_LACP_PORTSTATE_LACP_TIMEOUT;
            pp->sm_rx_timer_counter = timer2ticks(VTSS_LACP_TID_CURRENT_WHILE_TIMER, VTSS_LACP_SHORT_TIMEOUT);
            pp->actor_oper_port_state |= VTSS_LACP_PORTSTATE_EXPIRED;
            break;
        case VTSS_LACP_RXSTATE_DEFAULTED :
            update_default_selected(pp);
            record_default(pp);
            pp->sm_vars |= VTSS_LACP_PORT_MATCHED;
            pp->actor_oper_port_state &= ~VTSS_LACP_PORTSTATE_EXPIRED;
            break;
        case VTSS_LACP_RXSTATE_CURRENT :
            /* detect loopback situation */
            if (!VTSS_COMMON_MACADDR_CMP(((const vtss_lacp_info_t *)lacpdu->actor_info)->system_macaddr, LACP->system_config.system_id.macaddr)) {
                /* INFO_RECEIVED_LOOPBACK_FRAMES */
                VTSS_LACP_TRACE(VTSS_LACP_TRLVL_WARNING, ("An illegal loopback occurred on port %u\n",
                                                          (unsigned)pp->actor_port_number));
                return;
            }
            rx_update(lacpdu, pp);
            pp->sm_rx_timer_counter = timer2ticks(VTSS_LACP_TID_CURRENT_WHILE_TIMER, (vtss_lacp_time_interval_t)(pp->actor_oper_port_state & VTSS_LACP_PORTSTATE_LACP_TIMEOUT));
            pp->actor_oper_port_state &= ~VTSS_LACP_PORTSTATE_EXPIRED;
            break;
        default :    /* to silence the compiler */
            break;
        }
    }
}

static void periodic_stev(vtss_lacp_port_vars_t *pp)
{
    vtss_lacp_periodic_state_t last_state;

    /* keep current state machine state to compare later if it was changed */
    last_state = pp->sm_periodic_state;

    /* check if port was reinitialized */
    if (((pp->sm_vars & VTSS_LACP_PORT_BEGIN) ||
         !(pp->sm_vars & VTSS_LACP_PORT_LACP_ENABLED) ||
         !pp->port_up) ||
        (!(pp->actor_oper_port_state & VTSS_LACP_PORTSTATE_LACP_ACTIVITY) &&
         !(pp->partner_oper_port_state & VTSS_LACP_PORTSTATE_LACP_ACTIVITY))) {
        pp->sm_periodic_state = VTSS_LACP_PERIODICSTATE_NONE;    /* next state */
    } else if (pp->sm_periodic_timer_counter) { /* check if state machine should change state */
        /* check if periodic state machine expired */
        if (!(--pp->sm_periodic_timer_counter)) { /* if expired then do tx */
            pp->sm_periodic_state = VTSS_LACP_PERIODICSTATE_TX;    /* next state */
        } else {
            /* If not expired, check if there is some new timeout parameter from the partner state */
            switch (pp->sm_periodic_state) {
            case VTSS_LACP_PERIODICSTATE_FAST :
                if (!(pp->partner_oper_port_state & VTSS_LACP_PORTSTATE_LACP_TIMEOUT)) {
                    pp->sm_periodic_state = VTSS_LACP_PERIODICSTATE_SLOW;    /* next state */
                }
                break;
            case VTSS_LACP_PERIODICSTATE_SLOW :
                if ((pp->partner_oper_port_state & VTSS_LACP_PORTSTATE_LACP_TIMEOUT)) {
                    /* stop current timer */
                    pp->sm_periodic_timer_counter = 0;
                    pp->sm_periodic_state = VTSS_LACP_PERIODICSTATE_TX; /* next state */
                }
                break;
            default :    /* to silence the compiler */
                break;
            }
        }
    } else {
        switch (pp->sm_periodic_state) {
        case VTSS_LACP_PERIODICSTATE_NONE :
            pp->sm_periodic_state = VTSS_LACP_PERIODICSTATE_FAST;    /* next state */
            break;
        case VTSS_LACP_PERIODICSTATE_TX :
            pp->sm_periodic_state = (pp->partner_oper_port_state & VTSS_LACP_PORTSTATE_LACP_TIMEOUT) ?
                                    VTSS_LACP_PERIODICSTATE_FAST : VTSS_LACP_PERIODICSTATE_SLOW; /* next state */
            break;
        default :    /* to silence the compiler */
            break;
        }
    }

    /* check if the state machine was changed */
    if (pp->sm_periodic_state != last_state) {
        VTSS_LACP_TRACE(VTSS_LACP_TRLVL_NOISE, ("Periodic Machine: Port=%u, Last State=%u, Curr State=%u\n",
                                                (unsigned)pp->actor_port_number, (unsigned)last_state, (unsigned)pp->sm_periodic_state));
        switch (pp->sm_periodic_state) {
        case VTSS_LACP_PERIODICSTATE_NONE :
            pp->sm_periodic_timer_counter = 0; /* zero timer */
            break;
        case VTSS_LACP_PERIODICSTATE_FAST :
            pp->sm_periodic_timer_counter = timer2ticks(VTSS_LACP_TID_PERIODIC_TIMER, VTSS_LACP_FAST_PERIODIC_TIME);
            if (pp->sm_periodic_timer_counter > 1) {
                pp->sm_periodic_timer_counter--;    /* decrement 1 tick we lost in the PERIODIC_TX cycle */
            }
            break;
        case VTSS_LACP_PERIODICSTATE_SLOW :
            pp->sm_periodic_timer_counter = timer2ticks(VTSS_LACP_TID_PERIODIC_TIMER, VTSS_LACP_SLOW_PERIODIC_TIME);
            if (pp->sm_periodic_timer_counter > 1) {
                pp->sm_periodic_timer_counter--;    /* decrement 1 tick we lost in the PERIODIC_TX cycle */
            }
            break;
        case VTSS_LACP_PERIODICSTATE_TX :
            pp->ntt = VTSS_COMMON_BOOL_TRUE;
            break;
        default :    /* to silence the compiler */
            break;
        }
    }
}

static void clear_aggregation(vtss_lacp_aggregator_vars_t *aggregator)
{
    aggregator->is_individual = 0;
    aggregator->actor_admin_aggregator_key = 0;
    aggregator->actor_oper_aggregator_key = 0;
    VTSS_COMMON_MACADDR_ASSIGN(aggregator->partner_system.macaddr, VTSS_COMMON_ZEROMAC);
    aggregator->partner_system_priority = 0;
    aggregator->partner_oper_aggregator_key = 0;
    /* aggregator->receive_state = 0; -- Never used */
    /* aggregator->transmit_state = 0; -- Never used */
    aggregator->lag_ports = NULL;
    aggregator->num_of_ports = 0;
    VTSS_LACP_TRACE(VTSS_LACP_TRLVL_NOISE, ("LAG %d was cleared\n",
                                            (int)aggregator->aggregator_identifier));
}

static BOOL local_priority (vtss_lacp_prio_t *partner_prio, vtss_common_macaddr_t *partner_system)
{
    u32 i;
    if (*partner_prio > LACP->system_config.system_prio) {
        return 1;
    } else if (*partner_prio == LACP->system_config.system_prio) {
        for (i = 0; i < 6; i++) {
            if (partner_system->macaddr[i] > LACP->system_config.system_id.macaddr[i]) {
                return 1;
            } else if (partner_system->macaddr[i] < LACP->system_config.system_id.macaddr[i]) {
                return 0;
            }
        }
    }
    return 0;
}

static void port_selection_logic(vtss_lacp_port_vars_t *pp)
{
    vtss_lacp_aggregator_vars_t *aggregator, *free_aggregator = NULL, *temp_aggregator;
    vtss_lacp_port_vars_t *last_pp = NULL, *curr_pp, *next_pp;
    int found = 0;
    int tmp_prio = 0xffff, tmp_port, local_control = 0;

    /* if the port is already Selected, do nothing */
    if (pp->sm_vars & VTSS_LACP_PORT_SELECTED) {
        return;
    } else if ((pp->sm_vars & VTSS_LACP_PORT_STANDBY) && !(pp->sm_vars & VTSS_LACP_PORT_BEGIN)) {
        if (!local_priority(&pp->aggregator->partner_system_priority, &pp->partner_oper_system)) {
            /* The link partner has the priority and controls which ports are active */
            tmp_prio = pp->partner_oper_port_priority;
            tmp_port = pp->partner_oper_port_number;
            for (next_pp = pp->aggregator->lag_ports; next_pp; next_pp = next_pp->next_lag_port) {
                if ((next_pp->partner_oper_port_state & VTSS_LACP_PORTSTATE_DISTRIBUTING) || (!(next_pp->sm_vars & VTSS_LACP_PORT_SELECTED))) {
                    continue;
                }
                if (tmp_prio > next_pp->partner_oper_port_priority) {
                    continue;
                } else if (tmp_prio < next_pp->partner_oper_port_priority) {
                    tmp_prio = next_pp->partner_oper_port_priority;
                    tmp_port = next_pp->partner_oper_port_number;
                } else if (tmp_port < next_pp->partner_oper_port_number) {
                    tmp_port = next_pp->partner_oper_port_number;
                }
            }
            if (tmp_port != pp->partner_oper_port_number) {
                for (next_pp = pp->aggregator->lag_ports; next_pp; next_pp = next_pp->next_lag_port) {
                    if (tmp_port == next_pp->partner_oper_port_number) {
                        break;
                    }
                }
                if (next_pp->sm_vars & VTSS_LACP_PORT_SELECTED) {
                    next_pp->sm_vars &= ~VTSS_LACP_PORT_SELECTED;
                    next_pp->sm_vars |= VTSS_LACP_PORT_STANDBY;
                    pp->sm_vars |= VTSS_LACP_PORT_SELECTED;
                    pp->sm_vars &= ~VTSS_LACP_PORT_STANDBY;
                }
            }
        } else {
            /* if the port is on Standby and there is a spot available, then change the state to Selected */
            if (pp->aggregator->num_of_ports < VTSS_LACP_MAX_PORTS_IN_AGGR) {
                pp->sm_vars &= ~VTSS_LACP_PORT_STANDBY;
                pp->sm_vars |= VTSS_LACP_PORT_SELECTED;
                pp->aggregator->num_of_ports++;
            }
        }
        return;
    }

    /* if the port is connected to other aggregator, detach it */
    if (pp->aggregator) {
        /* detach the port from its former aggregator */
        temp_aggregator = pp->aggregator;
        for (curr_pp = temp_aggregator->lag_ports; curr_pp; last_pp = curr_pp, curr_pp = curr_pp->next_lag_port) {
            if (curr_pp == pp) {
                if (pp->sm_vars & VTSS_LACP_PORT_STANDBY) {
                    pp->sm_vars &= ~VTSS_LACP_PORT_STANDBY;
                } else {
                    temp_aggregator->num_of_ports--;
                }

                if (!last_pp) { /* if it is the first port attached to the aggregator */
                    temp_aggregator->lag_ports = pp->next_lag_port;
                } else { /* not the first port attached to the aggregator */
                    last_pp->next_lag_port = pp->next_lag_port;
                }

                /* clear the port's relations to this aggregator */
                pp->aggregator = NULL;
                pp->next_lag_port = NULL;

                VTSS_LACP_TRACE(VTSS_LACP_TRLVL_DEBUG, ("Port %u left LAG %u\n",
                                                        (unsigned)pp->actor_port_number,
                                                        (unsigned)temp_aggregator->aggregator_identifier));
                /* if the aggregator is empty, clear its parameters, and set it ready to be attached */
                if (temp_aggregator->lag_ports == NULL) {
                    clear_aggregation(temp_aggregator);
                }
                break;
            }
        }
        if (!curr_pp) { /* meaning: the port was related to an aggregator but was not on the aggregator port list */
            VTSS_LACP_TRACE(VTSS_LACP_TRLVL_WARNING, ("Port %u was related to aggregator %u but was not on its port list\n",
                                                      (unsigned)pp->actor_port_number,
                                                      (unsigned)pp->aggregator->aggregator_identifier));
        }
    }
    /* search on all aggregators for a suitable aggregator for this port */
    for (aggregator = &LACP->aggregators[0]; aggregator < &LACP->aggregators[VTSS_LACP_MAX_AGGR]; aggregator++) {
        /* keep a free aggregator for later use(if needed) */
        if (!aggregator->lag_ports) {
            if (!free_aggregator) {
                free_aggregator = aggregator;
            }
            continue;
        }
        /* check if current aggregator suits us */
        if ((aggregator->actor_port_speed == pp->port_speed && /* if all parameters match AND */
             aggregator->actor_oper_aggregator_key == pp->actor_oper_port_key &&
             !VTSS_COMMON_MACADDR_CMP(aggregator->partner_system.macaddr, pp->partner_oper_system.macaddr) &&
             aggregator->partner_system_priority == pp->partner_oper_system_priority &&
             aggregator->partner_oper_aggregator_key == pp->partner_oper_key)
            &&
            ((VTSS_COMMON_MACADDR_CMP(pp->partner_oper_system.macaddr, VTSS_COMMON_ZEROMAC) && /* partner answers */
              !aggregator->is_individual))                                       /* but is not individual OR */
           ) {
            /* attach to the founded aggregator */
            pp->aggregator = aggregator;
            pp->next_lag_port = aggregator->lag_ports;
            aggregator->lag_ports = pp;

            /* mark this port as selected or standby */

            if (pp->aggregator->num_of_ports >= VTSS_LACP_MAX_PORTS_IN_AGGR) {
                local_control = local_priority(&pp->aggregator->partner_system_priority, &pp->partner_oper_system);

                tmp_prio = pp->port_config.port_prio;
                tmp_port = pp->actor_port_number;

                /* Check if there is a member that has a lower priority which this port can replace  */
                for (next_pp = aggregator->lag_ports; next_pp && local_control; next_pp = next_pp->next_lag_port) {
                    if ((next_pp->sm_vars & VTSS_LACP_PORT_SELECTED) == 0) {
                        continue;
                    }

                    if (tmp_prio > next_pp->port_config.port_prio) {
                        continue;
                    } else if (tmp_prio < next_pp->port_config.port_prio) {
                        tmp_prio = next_pp->port_config.port_prio;
                        tmp_port = next_pp->actor_port_number;
                    } else if (tmp_port < next_pp->actor_port_number) {
                        tmp_port = next_pp->actor_port_number;
                    }
                }

                if ((tmp_port != pp->actor_port_number) && local_control) {
                    for (next_pp = aggregator->lag_ports; next_pp; next_pp = next_pp->next_lag_port) {
                        if (tmp_port == next_pp->actor_port_number) {
                            break;
                        }
                    }
                    next_pp->sm_vars &= ~VTSS_LACP_PORT_SELECTED;
                    next_pp->sm_vars |= VTSS_LACP_PORT_STANDBY;
                    pp->sm_vars |= VTSS_LACP_PORT_SELECTED;
                    pp->sm_vars &= ~VTSS_LACP_PORT_STANDBY;

                } else {
                    /* Lower priority member not found - put this port on Standby */
                    pp->sm_vars |= VTSS_LACP_PORT_STANDBY;
                }
            } else {
                VTSS_LACP_TRACE(VTSS_LACP_TRLVL_DEBUG, ("Port %u joined LAG %u (existing LAG)\n",
                                                        (unsigned)pp->actor_port_number,
                                                        (unsigned)pp->aggregator->aggregator_identifier));
                pp->sm_vars |= VTSS_LACP_PORT_SELECTED;
                pp->aggregator->num_of_ports++;
            }
            found = 1;
            break;
        }
    }

    /* the port couldn't find an aggregator - attach it to a new aggregator */
    if (!found) {
        if (free_aggregator) {
            /* assign port a new aggregator */
            pp->aggregator = free_aggregator;

            /* update the new aggregator's parameters */
            /* if port was responsed from the end-user */
            if (pp->duplex_mode == VTSS_COMMON_LINKDUPLEX_FULL) {
                pp->aggregator->is_individual = 0;
            } else {
                pp->aggregator->is_individual = 1;
            }

            pp->aggregator->actor_admin_aggregator_key = pp->port_config.port_key;
            pp->aggregator->actor_oper_aggregator_key = pp->actor_oper_port_key;
            pp->aggregator->actor_port_speed = pp->port_speed;;
            pp->aggregator->partner_system = pp->partner_oper_system;
            pp->aggregator->partner_system_priority = pp->partner_oper_system_priority;
            pp->aggregator->partner_oper_aggregator_key = pp->partner_oper_key;
            /* pp->aggregator->receive_state = 1; -- Never used */
            /* pp->aggregator->transmit_state = 1; -- Never used */
            pp->aggregator->lag_ports = pp;
            pp->aggregator->num_of_ports++;

            /* mark this port as selected */
            pp->sm_vars |= VTSS_LACP_PORT_SELECTED;

            VTSS_LACP_TRACE(VTSS_LACP_TRLVL_DEBUG, ("Port %u joined LAG %u (new LAG)\n",
                                                    (unsigned)pp->actor_port_number,
                                                    (unsigned)pp->aggregator->aggregator_identifier));
        } else
            VTSS_LACP_TRACE(VTSS_LACP_TRLVL_WARNING, ("Port %u did not find a suitable aggregator\n",
                                                      (unsigned)pp->actor_port_number));
    }
    /* if all aggregator's ports are READY_N == TRUE, set ready=TRUE in all aggregator's ports */
    /* else set ready=FALSE in all aggregator's ports */
    if (pp->aggregator != NULL) {
        set_all_aggr_ports(pp->aggregator, all_aggr_ports_ready(pp->aggregator));
    }
}

static void mux_stev(vtss_lacp_port_vars_t *pp)
{
    vtss_lacp_mux_state_t last_state;

    /* keep current State Machine state to compare later if it was changed */
    last_state = pp->sm_mux_state;

    if (pp->sm_vars & VTSS_LACP_PORT_BEGIN) {
        pp->sm_mux_state = VTSS_LACP_MUXSTATE_DETACHED;    /* next state */
    } else {
        switch (pp->sm_mux_state) {
        case VTSS_LACP_MUXSTATE_DETACHED :
            if (pp->sm_vars & (VTSS_LACP_PORT_SELECTED | VTSS_LACP_PORT_STANDBY)) {
                pp->sm_mux_state = VTSS_LACP_MUXSTATE_WAITING;    /* next state */
            }
            break;
        case VTSS_LACP_MUXSTATE_WAITING :
            /* if SELECTED == FALSE return to DETACH state */
            if (!(pp->sm_vars & VTSS_LACP_PORT_SELECTED) && !(pp->sm_vars & VTSS_LACP_PORT_STANDBY)) {
                pp->sm_vars &= ~VTSS_LACP_PORT_READY_N;
                /* in order to withhold the Selection Logic to check all ports READY_N value */
                /* every callback cycle to update ready variable, we check READY_N and update READY here */
                set_all_aggr_ports(pp->aggregator, all_aggr_ports_ready(pp->aggregator));
                pp->sm_mux_state = VTSS_LACP_MUXSTATE_DETACHED; /* next state */
                break;
            }
            /* check if the wait_while_timer expired */
            if (pp->sm_mux_timer_counter && !(--pp->sm_mux_timer_counter)) {
                pp->sm_vars |= VTSS_LACP_PORT_READY_N;
            }

            /* in order to withhold the selection logic to check all ports READY_N value */
            /* every callback cycle to update ready variable, we check READY_N and update READY here */
            set_all_aggr_ports(pp->aggregator, all_aggr_ports_ready(pp->aggregator));

            /* if the wait_while_timer expired, and the port is in READY state, move to ATTACHED state */
            if ((pp->sm_vars & VTSS_LACP_PORT_READY) && !pp->sm_mux_timer_counter) {
                pp->sm_mux_state = VTSS_LACP_MUXSTATE_ATTACHED; /* next state */
            }
            break;
        case VTSS_LACP_MUXSTATE_ATTACHED :
            /* check also if agg_select_timer expired(so the edable port will take place only after this timer) */
            if ((pp->sm_vars & VTSS_LACP_PORT_SELECTED) && (pp->partner_oper_port_state & VTSS_LACP_PORTSTATE_SYNCHRONIZATION)) {
                pp->sm_mux_state = VTSS_LACP_MUXSTATE_COLLDIST;    /* next state */
            } else if (!(pp->sm_vars & VTSS_LACP_PORT_SELECTED) || (pp->sm_vars & VTSS_LACP_PORT_STANDBY)) {  /* if UNSELECTED or STANDBY */
                pp->sm_vars &= ~VTSS_LACP_PORT_READY_N;
                /* in order to withhold the selection logic to check all ports READY_N value */
                /* every callback cycle to update ready variable, we check READY_N and update READY here */
                set_all_aggr_ports(pp->aggregator, all_aggr_ports_ready(pp->aggregator));
                pp->sm_mux_state = VTSS_LACP_MUXSTATE_DETACHED; /* next state */
            }
            break;
        case VTSS_LACP_MUXSTATE_COLLDIST :
            if (!(pp->sm_vars & VTSS_LACP_PORT_SELECTED) ||
                (pp->sm_vars & VTSS_LACP_PORT_STANDBY) ||
                !(pp->partner_oper_port_state & VTSS_LACP_PORTSTATE_SYNCHRONIZATION)) {
                pp->sm_mux_state = VTSS_LACP_MUXSTATE_ATTACHED;    /* next state */
            }
            break;
        default :    /* to silence the compiler */
            break;
        }
    }

    /* check if the state machine was changed */
    if (pp->sm_mux_state != last_state) {
        VTSS_LACP_TRACE(VTSS_LACP_TRLVL_DEBUG, ("Mux Machine: Port=%u, Last State=%u, Curr State=%u\n",
                                                (unsigned)pp->actor_port_number, (unsigned)last_state, (unsigned)pp->sm_mux_state));
        switch (pp->sm_mux_state) {
        case VTSS_LACP_MUXSTATE_DETACHED :
            pp->actor_oper_port_state &= ~VTSS_LACP_PORTSTATE_SYNCHRONIZATION;
            disable_collecting_distributing(pp);
            pp->actor_oper_port_state &= ~(VTSS_LACP_PORTSTATE_COLLECTING | VTSS_LACP_PORTSTATE_DISTRIBUTING);
            pp->ntt = VTSS_COMMON_BOOL_TRUE;
            break;
        case VTSS_LACP_MUXSTATE_WAITING :
            pp->sm_mux_timer_counter = timer2ticks(VTSS_LACP_TID_WAIT_WHILE_TIMER, 0);
            break;
        case VTSS_LACP_MUXSTATE_ATTACHED :
            pp->actor_oper_port_state |= VTSS_LACP_PORTSTATE_SYNCHRONIZATION;
            pp->actor_oper_port_state &= ~(VTSS_LACP_PORTSTATE_COLLECTING | VTSS_LACP_PORTSTATE_DISTRIBUTING);
            disable_collecting_distributing(pp);
            pp->ntt = VTSS_COMMON_BOOL_TRUE;
            break;
        case VTSS_LACP_MUXSTATE_COLLDIST :
            pp->actor_oper_port_state |= VTSS_LACP_PORTSTATE_COLLECTING | VTSS_LACP_PORTSTATE_DISTRIBUTING;
            enable_collecting_distributing(pp);
            pp->ntt = VTSS_COMMON_BOOL_TRUE;
            break;
        default :    /* to silence the compiler */
            VTSS_LACP_ASSERT(0);
            break;
        }
    }
}

static void tx_stev(vtss_lacp_port_vars_t *pp)
{
    /* check if there is something to send */
    if (pp->ntt && (pp->sm_vars & VTSS_LACP_PORT_LACP_ENABLED) &&
        ((pp->actor_oper_port_state & VTSS_LACP_PORTSTATE_LACP_ACTIVITY) ||
         (pp->partner_oper_port_state & VTSS_LACP_PORTSTATE_LACP_ACTIVITY))) {

        /* Verify that we do not send more than 3 packets per second */
        if (pp->sm_tx_timer_counter) {
            /* send the lacpdu */
            if (lacpdu_send(pp) == VTSS_COMMON_CC_OK) {
                /* mark ntt as false, so it will not be sent again until demanded */
                pp->ntt = VTSS_COMMON_BOOL_FALSE;
                pp->sm_tx_timer_counter--;
            }
        } else
            VTSS_LACP_TRACE(VTSS_LACP_TRLVL_DEBUG, ("tx_stev: Port=%u dropped xmit for timelimit\n",
                                                    (unsigned)pp->actor_port_number));
    }
}

static void partner_churn_stev(vtss_lacp_port_vars_t *pp)
{
    /* check if pc timer expired, to verify that the partner should have gone into sync */
    if (pp->sm_pc_timer_counter && !(--pp->sm_pc_timer_counter)) {
        if (pp->sm_partner_churn_state == VTSS_LACP_CHURNSTATE_MONITOR) {
            if (VTSS_COMMON_MACADDR_CMP(pp->partner_oper_system.macaddr, VTSS_COMMON_ZEROMAC)) {
                VTSS_LACP_TRACE(VTSS_LACP_TRLVL_WARNING, ("Port %u to partner %s (port %u) is churning\n",
                                                          (unsigned)pp->actor_port_number,
                                                          vtss_common_str_macaddr(&pp->partner_oper_system),
                                                          (unsigned)pp->partner_oper_port_number));
                pp->partner_oper_port_state |= VTSS_LACP_PORTSTATE_SYNCHRONIZATION;
            }
            pp->sm_partner_churn_state = VTSS_LACP_CHURNSTATE_NONE;
        }
    }
}

static volatile vtss_common_port_t next_work_port = VTSS_LACP_MAX_PORTS;

void vtss_lacp_more_work(void)
{
    vtss_lacp_port_vars_t *pp;

    if (next_work_port == VTSS_LACP_MAX_PORTS) { /* No more work to do */
        return;
    }
    pp = &LACP->ports[next_work_port++];
#ifdef VTSS_LACP_USE_MARKER
    got_mark_reply_or_tmo(pp, VTSS_COMMON_BOOL_FALSE);
#endif /* VTSS_LACP_USE_MARKER */
    rx_stev(pp, NULL);
    periodic_stev(pp);
    port_selection_logic(pp);
    mux_stev(pp);
    tx_stev(pp);
    partner_churn_stev(pp);
    pp->sm_vars &= ~VTSS_LACP_PORT_BEGIN;
}

static void finish_work(void)
{
    while (next_work_port < VTSS_LACP_MAX_PORTS) {
        vtss_lacp_more_work();
    }
}

static void queue_more_work(void)
{
    next_work_port = 0;
}

static void run_state_event_machines(void)
{
    finish_work();
    queue_more_work();
    vtss_lacp_more_work();
}

static void restart_port(vtss_lacp_port_vars_t *pp)
{
    /* there is no need to reselect a new aggregator, just signal the */
    /* state machines to reinitialize */
    pp->sm_vars |= VTSS_LACP_PORT_BEGIN;
    stop_partner_churn(pp);
}

static void lacp_sw_init(void)
{
    vtss_common_port_t pix;
    vtss_lacp_agid_t aix;
    vtss_lacp_port_vars_t *pp;

    memset(LACP, 0, sizeof(*LACP));
    LACP->initialized = VTSS_COMMON_BOOL_TRUE;
    LACP->system_config.system_prio = VTSS_LACP_DEFAULT_SYSTEMPRIO;
    for (aix = 0; aix < VTSS_LACP_MAX_AGGR; aix++) {
        LACP->aggregators[aix].aggregator_identifier = aix + 1;
    }
    pp = &LACP->ports[0];
    for (pix = 0; pix < VTSS_LACP_MAX_PORTS; pp++) {
        pp->actor_port_number = ++pix;
        pp->port_config.port_prio = VTSS_LACP_DEFAULT_PORTPRIO;
        pp->port_config.xmit_mode = VTSS_LACP_FSMODE_FAST;
        pp->port_config.active_or_passive = VTSS_LACP_ACTMODE_ACTIVE;
        pp->actor_oper_port_state = VTSS_LACP_PORTSTATE_AGGREGATION | VTSS_LACP_PORTSTATE_LACP_ACTIVITY | VTSS_LACP_PORTSTATE_LACP_TIMEOUT;
        pp->sm_vars = VTSS_LACP_PORT_BEGIN;
        pp->sm_tx_timer_counter = VTSS_LACP_MAX_TX_IN_SECOND;
        /* Disabled by default: pp->port_config.enable_lacp = VTSS_COMMON_BOOL_TRUE; */
    }
}

static void new_port_key(vtss_lacp_port_vars_t *pp)
{
    pp->actor_oper_port_key = vtss_os_make_key(pp->actor_port_number + VTSS_PORT_NO_START - 1, pp->port_config.port_key);
}


static void new_port_state(vtss_lacp_port_vars_t *pp, vtss_common_linkstate_t new_state)
{
    pp->port_up = new_state;
    pp->duplex_mode = vtss_os_get_linkduplex(pp->actor_port_number + VTSS_PORT_NO_START - 1);
    pp->port_speed = vtss_os_get_linkspeed(pp->actor_port_number + VTSS_PORT_NO_START - 1);
    new_port_key(pp);
    restart_port(pp);
}

static void lacp_hw_init(void)
{
    vtss_common_port_t pix;
    vtss_lacp_port_vars_t *pp;

    if (!VTSS_COMMON_MACADDR_CMP(LACP->system_config.system_id.macaddr, VTSS_COMMON_ZEROMAC)) {
        vtss_os_get_systemmac(&LACP->system_config.system_id);
    }

    pp = &LACP->ports[0];
    for (pix = 1; pix <= VTSS_LACP_MAX_PORTS; pix++, pp++) {
        vtss_os_get_portmac(pix + VTSS_PORT_NO_START - 1, &pp->port_macaddr);
        new_port_state(pp, vtss_os_get_linkstate(pix + VTSS_PORT_NO_START - 1));
    }
}

void vtss_lacp_init(void)
{
#ifdef VTSS_COMMENTS
    vtss_printf("LACP Total bytes: %u: %u ports @ %u bytes, %u aggregations @ %u bytes\n",
                (unsigned)sizeof(vtss_lacp_system_vars_t), (unsigned)VTSS_LACP_MAX_PORTS,
                (unsigned)sizeof(vtss_lacp_port_vars_t), (unsigned)VTSS_LACP_MAX_AGGR,
                (unsigned)sizeof(vtss_lacp_aggregator_vars_t));
#endif /* VTSS_COMMENTS */

    LACP_INIT;
    lacp_hw_init();
}

void vtss_lacp_deinit(void)
{
    lacp_sw_init();
}

void vtss_lacp_set_config(const vtss_lacp_system_config_t *system_config)
{
    vtss_lacp_system_config_t new_config;
    vtss_lacp_port_vars_t *pp;

    LACP_INIT;
    new_config = *system_config;
    if (!VTSS_COMMON_MACADDR_CMP(new_config.system_id.macaddr, VTSS_COMMON_ZEROMAC)) {
        new_config.system_id = LACP->system_config.system_id;
    }
    if (memcmp(&new_config, &LACP->system_config, sizeof(LACP->system_config)) == 0) {
        return;
    }
    finish_work();
    LACP->system_config = new_config;
    for (pp = &LACP->ports[0]; pp < &LACP->ports[VTSS_LACP_MAX_PORTS]; pp++) {
        restart_port(pp);
    }
}

void vtss_lacp_set_portconfig(vtss_common_port_t portno,
                              const vtss_lacp_port_config_t *port_config)
{
    vtss_lacp_port_vars_t *pp;

    portno = portno - VTSS_PORT_NO_START + 1; /* First port is always 1 */
    VTSS_LACP_ASSERT(portno > 0 && portno <= VTSS_LACP_MAX_PORTS);
    LACP_INIT;
    pp = &LACP->ports[portno - 1];
    if (memcmp(port_config, &pp->port_config, sizeof(pp->port_config)) == 0) {
        return;
    }
    finish_work();
    pp->port_config = *port_config;
    new_port_key(pp);
    if (port_config->xmit_mode == VTSS_LACP_FSMODE_FAST) {
        pp->actor_oper_port_state |= VTSS_LACP_PORTSTATE_LACP_TIMEOUT;
    } else {
        pp->actor_oper_port_state &= ~VTSS_LACP_PORTSTATE_LACP_TIMEOUT;
    }
    if (port_config->active_or_passive == VTSS_LACP_ACTMODE_PASSIVE) {
        pp->actor_oper_port_state &= ~VTSS_LACP_PORTSTATE_LACP_ACTIVITY;
    } else {
        pp->actor_oper_port_state |= VTSS_LACP_PORTSTATE_LACP_ACTIVITY;
    }
    restart_port(pp);
}

void vtss_lacp_get_config(vtss_lacp_system_config_t *system_config)
{
    LACP_INIT;
    *system_config = LACP->system_config;
}

void vtss_lacp_get_portconfig(vtss_common_port_t portno,
                              vtss_lacp_port_config_t *port_config)
{
    vtss_lacp_port_vars_t *pp;

    portno = portno - VTSS_PORT_NO_START + 1; /* First port is always 1 */
    VTSS_LACP_ASSERT(portno > 0 && portno <= VTSS_LACP_MAX_PORTS);
    LACP_INIT;
    pp = &LACP->ports[portno - 1];
    *port_config = pp->port_config;
}

/* Get LACP status */
vtss_common_bool_t vtss_lacp_get_aggr_status(vtss_lacp_agid_t aggrid, vtss_lacp_aggregatorstatus_t *aggr_status)
{
    const vtss_lacp_port_vars_t *ccp;
    const vtss_lacp_aggregator_vars_t *cap;

    aggrid = aggrid - VTSS_PORT_NO_START + 1; /* First id is always 1 */
    VTSS_LACP_ASSERT(aggrid > 0 && aggrid <= VTSS_LACP_MAX_AGGR);
    LACP_INIT;
    cap = &LACP->aggregators[aggrid - 1];
    VTSS_LACP_ASSERT(aggrid == cap->aggregator_identifier);
    if (aggr_status) {
        memset(aggr_status, 0, sizeof(*aggr_status));
        aggr_status->aggrid = aggrid + VTSS_PORT_NO_START - 1;
        aggr_status->secs_since_last_change = (LACP->ticks_since_start - cap->last_change) / VTSS_LACP_TICKS_PER_SEC;
        aggr_status->partner_oper_system = cap->partner_system;
        aggr_status->partner_oper_system_priority = cap->partner_system_priority;
        aggr_status->partner_oper_key = cap->partner_oper_aggregator_key;
        for (ccp = cap->lag_ports; ccp; ccp = ccp->next_lag_port) {
            aggr_status->port_list[ccp->actor_port_number - 1] = (ccp->actor_oper_port_state &
                                                                  (VTSS_LACP_PORTSTATE_COLLECTING | VTSS_LACP_PORTSTATE_DISTRIBUTING)) ?
                                                                 VTSS_COMMON_BOOL_TRUE : VTSS_COMMON_BOOL_FALSE;
        }
    }
    return (cap->num_of_ports > 1) ? VTSS_COMMON_BOOL_TRUE : VTSS_COMMON_BOOL_FALSE;
}

void vtss_lacp_get_port_status(vtss_common_port_t portno, vtss_lacp_portstatus_t *port_status)
{
    const vtss_lacp_port_vars_t *ccp;

    portno = portno - VTSS_PORT_NO_START + 1; /* First port is always 1 */

    VTSS_LACP_ASSERT(port_status != NULL);
    VTSS_LACP_ASSERT(portno > 0 && portno <= VTSS_LACP_MAX_PORTS);
    LACP_INIT;
    ccp = &LACP->ports[portno - 1];
    port_status->port_number = portno + VTSS_PORT_NO_START - 1;
    port_status->port_enabled = (ccp->sm_vars & VTSS_LACP_PORT_LACP_ENABLED) ? VTSS_COMMON_BOOL_TRUE : VTSS_COMMON_BOOL_FALSE;
#if defined(HP_PROCURVE_HW)
    port_status->port_forwarding = vtss_os_get_fwdstate(portno + VTSS_PORT_NO_START - 1);
#else
    port_status->port_forwarding = (VTSS_COMMON_STPSTATE_FORWARDING == vtss_os_get_stpstate(portno + VTSS_PORT_NO_START - 1));
#endif
    port_status->actor_oper_port_key = ccp->actor_oper_port_key;
    port_status->actor_admin_port_key = ccp->port_config.port_key;
    port_status->actor_port_aggregator_identifier = ccp->aggregator ? (ccp->aggregator->aggregator_identifier + VTSS_PORT_NO_START - 1) : 0;
    port_status->partner_oper_port_priority = ccp->partner_oper_port_priority;
    port_status->partner_oper_port_number = ccp->partner_oper_port_number;
    port_status->port_stats = ccp->stats;
    port_status->port_state = (ccp->sm_vars & VTSS_LACP_PORT_STANDBY) ? LACP_PORT_STANDBY :
                              (ccp->actor_oper_port_state & (VTSS_LACP_PORTSTATE_COLLECTING | VTSS_LACP_PORTSTATE_DISTRIBUTING)) ?
                              LACP_PORT_ACTIVE : LACP_PORT_NOT_ACTIVE;
}

#ifndef VTSS_LACP_NDEBUG
void vtss_lacp_dump(void)
{
    const vtss_lacp_port_vars_t *cpp;
    const vtss_lacp_aggregator_vars_t *cap;

    vtss_printf("LACP dump (%u ticks old):\n", (unsigned)LACP->ticks_since_start);
    for (cap = &LACP->aggregators[0]; cap < &LACP->aggregators[VTSS_LACP_MAX_AGGR]; cap++) {
        if (cap->num_of_ports <= 1) {
            continue;
        }
        vtss_printf("Aggr %u: %u ports (max %u in one group)\n",
                    (unsigned)cap->aggregator_identifier,
                    (unsigned)cap->num_of_ports, VTSS_LACP_MAX_PORTS_IN_AGGR);
        for (cpp = cap->lag_ports; cpp; cpp = cpp->next_lag_port) {
#if defined(HP_PROCURVE_HW)
            vtss_printf("  Port %u: operkey 0x%x fwd %u ",
                        (unsigned)cpp->actor_port_number,
                        (unsigned)cpp->actor_oper_port_key,
                        (unsigned)vtss_os_get_fwdstate(cpp->actor_port_number + VTSS_PORT_NO_START - 1));
#else
            vtss_printf("  Port %u (LACP:%s): operkey:0x%x fwd:%u ",
                        (unsigned)cpp->actor_port_number,
                        ((cpp->actor_oper_port_state & VTSS_LACP_PORTSTATE_COLLECTING) &&
                         (cpp->actor_oper_port_state & VTSS_LACP_PORTSTATE_DISTRIBUTING)) ? "UP" : "Down",
                        (unsigned)cpp->actor_oper_port_key,
                        (unsigned)(VTSS_COMMON_STPSTATE_FORWARDING == vtss_os_get_stpstate(cpp->actor_port_number + VTSS_PORT_NO_START - 1)));
#endif
            vtss_printf("rx_state:%u tx_state:%u mux_state:%u period_state: %u churn_state:%u\n",
                        (unsigned)cpp->sm_rx_state,
                        (unsigned)cpp->sm_tx_state,
                        (unsigned)cpp->sm_mux_state,
                        (unsigned)cpp->sm_periodic_state,
                        (unsigned)cpp->sm_partner_churn_state);
            vtss_printf("    sm_vars 0x%x:%s\n",
                        (unsigned)cpp->sm_vars,
                        (const char *)vtss_lacp_str_sm(cpp->sm_vars));
            vtss_printf("    port state:%s\n",
                        vtss_lacp_str_portstate(cpp->actor_oper_port_state));
        }
    }
}
#endif /* !VTSS_LACP_NDEBUG */

/* Event callback functions provided by the vtss_lacp protocol module */
void vtss_lacp_linkstate_changed(vtss_common_port_t portno, vtss_common_linkstate_t new_state)
{
    vtss_lacp_port_vars_t *pp;

    portno = portno - VTSS_PORT_NO_START + 1; /* First port is always 1 */
    VTSS_LACP_ASSERT(portno > 0 && portno <= VTSS_LACP_MAX_PORTS);
    if (LACP_IS_UNINITED) {
        return;
    }
    pp = &LACP->ports[portno - 1];

#if !defined(HP_PROCURVE_HW)
#if defined(VTSS_LACP_DELAYED_PORTMAC_ADDRESS)
    /* If port is UP - get MAC address now! */
    if (new_state == VTSS_COMMON_LINKSTATE_UP) {
        vtss_os_get_portmac(portno + VTSS_PORT_NO_START - 1, &pp->port_macaddr);
    }
#endif /* defined(VTSS_LACP_DELAYED_PORTMAC_ADDRESS) */
#endif

#if defined(HP_PROCURVE_HW)
    if (pp->port_up != new_state) {
        VTSS_LACP_TRACE(VTSS_LACP_TRLVL_DEBUG, ("Port %u changed state from %u to %s (duplex %s)\n",
                                                (unsigned)portno, (unsigned)pp->port_up,
                                                vtss_common_str_linkstate(new_state),
                                                vtss_common_str_linkduplex(vtss_os_get_linkduplex(portno + VTSS_PORT_NO_START - 1))));
        finish_work();
        new_port_state(pp, new_state);
    }
#else
    VTSS_LACP_TRACE(VTSS_LACP_TRLVL_DEBUG, ("Port %u changed state from %u to %s (duplex %s)\n",
                                            (unsigned)portno, (unsigned)pp->port_up,
                                            vtss_common_str_linkstate(new_state),
                                            vtss_common_str_linkduplex(vtss_os_get_linkduplex(portno + VTSS_PORT_NO_START - 1))));
    finish_work();
    new_port_state(pp, new_state);
#endif

}

void vtss_lacp_tick(void)
{
    vtss_common_port_t pix;

    LACP->ticks_since_start++;
    if (!(LACP->ticks_since_start % VTSS_LACP_TICKS_PER_SEC)) {
        for (pix = 0; pix < VTSS_LACP_MAX_PORTS; pix++) { /* Allow more transmits */
            LACP->ports[pix].sm_tx_timer_counter = VTSS_LACP_MAX_TX_IN_SECOND;
        }
    }
    run_state_event_machines();
}

void vtss_lacp_receive(vtss_common_port_t from_port,
                       const vtss_common_octet_t VTSS_COMMON_BUFMEM_ATTRIB *frame,
                       vtss_common_framelen_t len)
{
    vtss_lacp_port_vars_t *pp;
#define frmhead ((const vtss_lacp_frame_header_t *)frame)
    from_port = from_port - VTSS_PORT_NO_START + 1; /* First port is always 1 */
    VTSS_LACP_ASSERT(from_port > 0 && from_port <= VTSS_LACP_MAX_PORTS);
    VTSS_LACP_ASSERT(frame != NULL);
    pp = &LACP->ports[from_port - 1];
    if (len >= SIZEOF_VTSS_LACP_LACPDU) {
        finish_work();
        switch (frmhead->subtype) {
        case VTSS_LACP_SUBTYPE_LACP :
            VTSS_LACP_TRACE(VTSS_LACP_TRLVL_NOISE, ("Received LACPDU on port %u\n",
                                                    (unsigned)from_port));
            pp->stats.lacp_frame_recvs++;
            rx_stev(pp, (const vtss_lacp_lacpdu_t *)frame);
            tx_stev(pp);
            break;
        case VTSS_LACP_SUBTYPE_MARK :
            VTSS_LACP_TRACE(VTSS_LACP_TRLVL_DEBUG, ("Received MARKERPDU on port %u\n",
                                                    (unsigned)from_port));
            handle_markerframe(pp, (const vtss_lacp_markerpdu_t *)frame);
            break;
        default :
            pp->stats.unknown_frame_recvs++;
            VTSS_LACP_TRACE(VTSS_LACP_TRLVL_DEBUG, ("Ignoring xxPDU frame from port %u with unknown subtype (0x%x)\n",
                                                    (unsigned)from_port, (unsigned)frmhead->subtype));
            break;
        }
    } else {
        pp->stats.illegal_frame_recvs++;
        VTSS_LACP_TRACE(VTSS_LACP_TRLVL_DEBUG, ("Ignoring xxPDU frame from port %u with short length (%u)\n",
                                                (unsigned)from_port, (unsigned)len));
    }
#undef frmhead
}

#if !defined(HP_PROCURVE_HW)
void vtss_lacp_clear_statistics(vtss_common_port_t port)
{
    vtss_lacp_port_vars_t *pp;
    port = port - VTSS_PORT_NO_START + 1;
    pp = &LACP->ports[port - 1];

    pp->stats.lacp_frame_xmits = 0;     /* LACP frames transmitted */
    pp->stats.lacp_frame_recvs = 0;     /* LACP frames received */
    pp->stats.markreq_frame_xmits = 0;  /* MARKER frames transmitted */
    pp->stats.markreq_frame_recvs = 0;  /* MARKER frames received */
    pp->stats.markresp_frame_xmits = 0; /* MARKER respnse frames transmitted */
    pp->stats.markresp_frame_recvs = 0; /* MARKER respnse frames received */
    pp->stats.illegal_frame_recvs = 0;  /* Illegal frames received and discarded in error */
    pp->stats.unknown_frame_recvs = 0;  /* Unknown frames received and discarded in error */

}
#endif
#endif /* VTSS_LACP_NOT_WANTED */
