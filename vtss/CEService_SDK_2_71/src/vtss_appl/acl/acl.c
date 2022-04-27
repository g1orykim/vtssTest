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

 $Id$
 $Revision$

*/

#include "main.h"
#include "conf_api.h"
#ifdef VTSS_SW_OPTION_PACKET
#include "packet_api.h"
#endif
#include "msg_api.h"    //msg_switch_exists(), msg_switch_configurable()
#include "port_api.h"
#include "critd_api.h"
#define _ACL_USER_NAME_C_
#include "acl_api.h"
#include "acl.h"
#include "misc_api.h"

#ifdef VTSS_SW_OPTION_SYSLOG
#include "syslog_api.h"
#endif

#ifdef VTSS_SW_OPTION_VCLI
#include "acl_cli.h"
#endif
#ifdef VTSS_SW_OPTION_ICFG
#include "acl_icfg.h"
#endif

#if CYG_BYTEORDER == CYG_MSBFIRST
#define BYTE_ORDER BIG_ENDIAN
#else
#define BYTE_ORDER LITTLE_ENDIAN
#endif
#define LITTLE_ENDIAN   1234
#define BIG_ENDIAN  4321
#include <netinet/in.h>
#include <netinet/ip6.h>


#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_ACL

/* Define this for testing CPU packet reception performance */
#undef ACL_PACKET_TEST

/****************************************************************************/
/*  Global variables                                                        */
/****************************************************************************/

/* Global structure */
static acl_global_t acl;

#if (VTSS_TRACE_ENABLED)
static vtss_trace_reg_t trace_reg = {
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "acl",
    .descr     = "Access Control Lists"
};

static vtss_trace_grp_t trace_grps[TRACE_GRP_CNT] = {
    [VTSS_TRACE_GRP_DEFAULT] = {
        .name      = "default",
        .descr     = "Default",
        .lvl       = VTSS_TRACE_LVL_ERROR,
        .timestamp = 1,
    },
    [TRACE_GRP_CRIT] = {
        .name      = "crit",
        .descr     = "Critical regions ",
        .lvl       = VTSS_TRACE_LVL_ERROR,
        .timestamp = 1,
    }
};
#define ACL_CRIT_ENTER() critd_enter(&acl.crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define ACL_CRIT_EXIT()  critd_exit( &acl.crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#else
#define ACL_CRIT_ENTER() critd_enter(&acl.crit)
#define ACL_CRIT_EXIT()  critd_exit( &acl.crit)
#endif /* VTSS_TRACE_ENABLED */

static void acl_packet_register(void);
static void acl_mgmt_conflict_solve(void);
static vtss_rc acl_stack_ace_get(vtss_isid_t isid_get,
                                 vtss_ace_id_t id, vtss_ace_counter_t *counter);

/****************************************************************************/
/*  Various local functions                                                 */
/****************************************************************************/

/* Determine if ACL port configuration has changed */
int acl_mgmt_port_conf_changed(acl_port_conf_t *old, acl_port_conf_t *new)
{
    return (memcmp(new, old, sizeof(*new)));
}

/* Set ACL port defaults */
void acl_mgmt_port_conf_get_default(acl_port_conf_t *conf)
{
    memset(conf, 0, sizeof(*conf));
    conf->policy_no = VTSS_ACL_POLICY_NO_START;
    conf->action.policer = ACL_POLICER_NONE;

#if defined(VTSS_FEATURE_ACL_V2)
    conf->action.port_action = VTSS_ACL_PORT_ACTION_NONE;
    memset(conf->action.port_list, 0x0, sizeof(conf->action.port_list));
    conf->action.mirror = FALSE;
    conf->action.ptp_action = VTSS_ACL_PTP_ACTION_NONE;
#else
    conf->action.permit = TRUE;
    conf->action.port_no = VTSS_PORT_NO_NONE;
#endif /* VTSS_FEATURE_ACL_V2 */
#if defined(VTSS_ARCH_CARACAL) && defined(VTSS_SW_OPTION_EVC)
    conf->action.evc_police = FALSE;
    conf->action.evc_policer_id = 0;
#endif /* VTSS_ARCH_CARACAL && VTSS_SW_OPTION_EVC */
    conf->action.logging = FALSE;
    conf->action.shutdown = FALSE;
    conf->action.force_cpu = FALSE;
    conf->action.cpu_once = FALSE;
    conf->action.cpu_queue = PACKET_XTR_QU_ACL_COPY; /* Default ACL queue */
#if defined(VTSS_ARCH_JAGUAR_1)
    conf->action.irq_trigger = FALSE;
#endif
}

/* Set ACL action */
static void acl_action_set(vtss_ace_id_t id, vtss_acl_action_t *action, acl_action_t *new)
{
    vtss_port_no_t port_no;
#if PACKET_XTR_QU_ACL_REDIR != PACKET_XTR_QU_ACL_COPY
    BOOL           cpu_copy = FALSE;
#endif /* PACKET_XTR_QU_ACL_REDIR != PACKET_XTR_QU_ACL_COPY */

    memset(action, 0, sizeof(*action));
#if defined(VTSS_FEATURE_ACL_V1)
    action->forward = (new->permit ? 1 : 0);
    action->learn = action->forward;
#endif /* VTSS_FEATURE_ACL_V1 */

    /* If the ACE covers multiple ports, which must be shut down, we must send it all to CPU */
    action->cpu = (new->force_cpu || new->logging || new->shutdown ? 1 : 0);
    action->cpu_once = new->cpu_once;
    action->cpu_queue = new->cpu_queue;

    action->police = (new->policer == ACL_POLICER_NONE ? 0 : 1);
    action->policer_no = new->policer;
#if defined(VTSS_ARCH_LUTON28)
    if (action->cpu) {
        /* Always police frames to CPU */
        action->police = 1;
        action->policer_no = ACL_POLICER_NO_RESV;
    }
#endif /* VTSS_ARCH_LUTON28 */

#if defined(VTSS_FEATURE_ACL_V2)
    action->port_action = new->port_action;
    action->learn = 1;
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        action->port_list[port_no] = (PORT_NO_IS_STACK(port_no) ? 0 : new->port_list[port_no]);
#if PACKET_XTR_QU_ACL_REDIR != PACKET_XTR_QU_ACL_COPY
        if (action->port_list[port_no]) {
            cpu_copy = TRUE;
        }
#endif /* PACKET_XTR_QU_ACL_REDIR != PACKET_XTR_QU_ACL_COPY */
    }
    action->mirror = new->mirror;
    action->ptp_action = new->ptp_action;
#else
    port_no = new->port_no;
    if (port_no != VTSS_PORT_NO_NONE && PORT_NO_IS_STACK(port_no)) {
        port_no = VTSS_PORT_NO_NONE;
    }
#if PACKET_XTR_QU_ACL_REDIR != PACKET_XTR_QU_ACL_COPY
    if (port_no != VTSS_PORT_NO_NONE) {
        cpu_copy = TRUE;
    }
#endif /* PACKET_XTR_QU_ACL_REDIR != PACKET_XTR_QU_ACL_COPY */
    action->port_no = port_no;
    action->port_forward = (port_no == VTSS_PORT_NO_NONE ? 0 : 1);
#endif /* VTSS_FEATURE_ACL_V2 */

#if PACKET_XTR_QU_ACL_REDIR != PACKET_XTR_QU_ACL_COPY
    if (action->cpu || action->cpu_once) {
        vtss_packet_rx_queue_t new_cpu_queue = cpu_copy ? PACKET_XTR_QU_ACL_COPY : PACKET_XTR_QU_ACL_REDIR;

        if (new_cpu_queue != action->cpu_queue) {
            T_W("Changing CPU queue from %u to %u", action->cpu_queue, new_cpu_queue);
            action->cpu_queue = new_cpu_queue;
        }
    }
#endif /* PACKET_XTR_QU_ACL_REDIR != PACKET_XTR_QU_ACL_COPY */

#if defined(VTSS_ARCH_CARACAL) && defined(VTSS_SW_OPTION_EVC)
    action->evc_police = new->evc_police;
    action->evc_policer_id = new->evc_policer_id;
#endif /* VTSS_ARCH_CARACAL && VTSS_SW_OPTION_EVC */
#if defined(VTSS_ARCH_JAGUAR_1)
    action->irq_trigger = new->irq_trigger;
#endif
#if defined(VTSS_ARCH_SERVAL)
    action->lm_cnt_disable = new->lm_cnt_disable;
#endif
}

/* Setup ACL port configuration via switch API */
static vtss_rc acl_port_conf_set(vtss_port_no_t port_no, acl_port_conf_t *conf)
{
    vtss_acl_port_conf_t port_conf;

    /* Disable ACL on stack ports */
    port_conf.policy_no = (PORT_NO_IS_STACK(port_no) ? ACL_POLICER_NONE : conf->policy_no);
    acl_action_set(ACE_ID_NONE, &port_conf.action, &conf->action);
    return vtss_acl_port_conf_set(NULL, port_no, &port_conf);
}

/* Determine if ACL policer configuration has changed */
int acl_mgmt_policer_conf_changed(acl_policer_conf_t *old, acl_policer_conf_t *new)
{
#if defined(VTSS_FEATURE_ACL_V2) /* 100kbps, pps, */
    if (new->bit_rate_enable == old->bit_rate_enable) {
        if (new->bit_rate_enable) {
            return (new->bit_rate != old->bit_rate);
        } else {
            return (new->packet_rate != old->packet_rate);
        }
    }
    return 1;
#else
    return (memcmp(new, old, sizeof(acl_policer_conf_t)));
#endif /* VTSS_FEATURE_ACL_V2 */
}

/* Get ACL policer defaults */
void acl_mgmt_policer_conf_get_default(acl_policer_conf_t *conf)
{
#if defined(VTSS_FEATURE_ACL_V2)
    conf->bit_rate_enable = FALSE;
    conf->bit_rate = 0;
#endif /* VTSS_FEATURE_ACL_V2 */
    conf->packet_rate = 1;
}

/* Setup policer configuration via switch API */
static vtss_rc acl_policer_conf_set(vtss_acl_policer_no_t policer_no,
                                    acl_policer_conf_t    *conf)
{
    vtss_acl_policer_conf_t policer_conf;

#if defined(VTSS_FEATURE_ACL_V2)
    policer_conf.bit_rate_enable = conf->bit_rate_enable;
    policer_conf.bit_rate = conf->bit_rate;
#endif /* VTSS_FEATURE_ACL_V2 */

#if defined(VTSS_ARCH_LUTON28)
    policer_conf.rate = (policer_no == ACL_POLICER_NO_RESV ? PACKET_XTR_POLICER_RATE : conf->packet_rate);
#else
    policer_conf.rate = conf->packet_rate;
#endif /* VTSS_ARCH_LUTON28 */

    return vtss_acl_policer_conf_set(NULL, policer_no, &policer_conf);
}

/* Convert flag to vtss_ace_bit_t type */
static vtss_ace_bit_t acl_bit_get(acl_flag_t flag, acl_entry_conf_t *conf)
{
    return (VTSS_BF_GET(conf->flags.mask, flag) ?
            (VTSS_BF_GET(conf->flags.value, flag) ?
             VTSS_ACE_BIT_1 : VTSS_ACE_BIT_0) : VTSS_ACE_BIT_ANY);
}

/* Add ACE via switch API */
static vtss_rc acl_ace_add(vtss_ace_id_t id, acl_entry_conf_t *conf)
{
    vtss_rc    rc;
    vtss_ace_t ace;

    T_D("enter, id: %d, type: %d", id, conf->type);
    if (vtss_ace_init(NULL, conf->type, &ace) != VTSS_OK) {
        T_W("Calling vtss_ace_init() failed\n");
    }
    ace.id = conf->id;
#if defined(VTSS_ARCH_SERVAL)
    ace.lookup = conf->lookup;
    ace.isdx_enable = conf->isdx_enable;
    ace.isdx_disable = conf->isdx_disable;
#endif /* VTSS_ARCH_SERVAL */
    ace.policy = conf->policy;
    acl_action_set(conf->id, &ace.action, &conf->action);
    ace.dmac_mc = acl_bit_get(ACE_FLAG_DMAC_MC, conf);
    ace.dmac_bc = acl_bit_get(ACE_FLAG_DMAC_BC, conf);
    ace.vlan.vid = conf->vid;
    ace.vlan.usr_prio = conf->usr_prio;
    ace.vlan.cfi = acl_bit_get(ACE_FLAG_VLAN_CFI, conf);
#if defined(VTSS_FEATURE_ACL_V2)
    ace.vlan.tagged = conf->tagged;
    memcpy(ace.port_list, conf->port_list, sizeof(ace.port_list));
#else
    ace.port_no = conf->port_no;
#endif /* VTSS_FEATURE_ACL_V2 */

    switch (conf->type) {
    case VTSS_ACE_TYPE_ETYPE:
        ace.frame.etype.dmac = conf->frame.etype.dmac;
        ace.frame.etype.smac = conf->frame.etype.smac;
        ace.frame.etype.etype = conf->frame.etype.etype;
        ace.frame.etype.data = conf->frame.etype.data;
#if defined(VTSS_FEATURE_ACL_V2)
        ace.frame.etype.ptp = conf->frame.etype.ptp;
#endif /* VTSS_FEATURE_ACL_V2 */
        break;
    case VTSS_ACE_TYPE_LLC:
        ace.frame.llc.dmac = conf->frame.llc.dmac;
        ace.frame.llc.smac = conf->frame.llc.smac;
        ace.frame.llc.llc = conf->frame.llc.llc;
        break;
    case VTSS_ACE_TYPE_SNAP:
        ace.frame.snap.dmac = conf->frame.snap.dmac;
        ace.frame.snap.smac = conf->frame.snap.smac;
        ace.frame.snap.snap = conf->frame.snap.snap;
        break;
    case VTSS_ACE_TYPE_ARP:
        ace.frame.arp.smac = conf->frame.arp.smac;
        ace.frame.arp.arp = acl_bit_get(ACE_FLAG_ARP_ARP, conf);
        ace.frame.arp.req = acl_bit_get(ACE_FLAG_ARP_REQ, conf);
        ace.frame.arp.unknown = acl_bit_get(ACE_FLAG_ARP_UNKNOWN, conf);
        ace.frame.arp.smac_match = acl_bit_get(ACE_FLAG_ARP_SMAC, conf);
        ace.frame.arp.dmac_match = acl_bit_get(ACE_FLAG_ARP_DMAC, conf);
        ace.frame.arp.length = acl_bit_get(ACE_FLAG_ARP_LEN, conf);
        ace.frame.arp.ip = acl_bit_get(ACE_FLAG_ARP_IP, conf);
        ace.frame.arp.ethernet = acl_bit_get(ACE_FLAG_ARP_ETHER, conf);
        ace.frame.arp.sip = conf->frame.arp.sip;
        ace.frame.arp.dip = conf->frame.arp.dip;
        break;
    case VTSS_ACE_TYPE_IPV4:
        ace.frame.ipv4.ttl = acl_bit_get(ACE_FLAG_IP_TTL, conf);
        ace.frame.ipv4.fragment = acl_bit_get(ACE_FLAG_IP_FRAGMENT, conf);
        ace.frame.ipv4.options = acl_bit_get(ACE_FLAG_IP_OPTIONS, conf);
        ace.frame.ipv4.ds = conf->frame.ipv4.ds;
        ace.frame.ipv4.proto = conf->frame.ipv4.proto;
        ace.frame.ipv4.sip = conf->frame.ipv4.sip;
        ace.frame.ipv4.dip = conf->frame.ipv4.dip;
        if (ace.frame.ipv4.options != VTSS_ACE_BIT_1 &&
            ace.frame.ipv4.fragment != VTSS_ACE_BIT_1) {
            /* IPv4 payload and UDP/TCP header ignored if IP options/fragment */
            ace.frame.ipv4.data = conf->frame.ipv4.data;
            ace.frame.ipv4.sport = conf->frame.ipv4.sport;
            ace.frame.ipv4.dport = conf->frame.ipv4.dport;
            ace.frame.ipv4.tcp_fin = acl_bit_get(ACE_FLAG_TCP_FIN, conf);
            ace.frame.ipv4.tcp_syn = acl_bit_get(ACE_FLAG_TCP_SYN, conf);
            ace.frame.ipv4.tcp_rst = acl_bit_get(ACE_FLAG_TCP_RST, conf);
            ace.frame.ipv4.tcp_psh = acl_bit_get(ACE_FLAG_TCP_PSH, conf);
            ace.frame.ipv4.tcp_ack = acl_bit_get(ACE_FLAG_TCP_ACK, conf);
            ace.frame.ipv4.tcp_urg = acl_bit_get(ACE_FLAG_TCP_URG, conf);
        }
#if defined(VTSS_FEATURE_ACL_V2)
        ace.frame.ipv4.sip_smac = conf->frame.ipv4.sip_smac;
        ace.frame.ipv4.ptp = conf->frame.ipv4.ptp;
#endif /* VTSS_FEATURE_ACL_V2 */
        break;
    case VTSS_ACE_TYPE_IPV6:
        ace.frame.ipv6.proto = conf->frame.ipv6.proto;
        ace.frame.ipv6.sip = conf->frame.ipv6.sip;
#if !defined(VTSS_ARCH_LUTON28)
        ace.frame.ipv6.ttl = conf->frame.ipv6.ttl;
        ace.frame.ipv6.ds = conf->frame.ipv6.ds;
        ace.frame.ipv6.data = conf->frame.ipv6.data;
        ace.frame.ipv6.sport = conf->frame.ipv6.sport;
        ace.frame.ipv6.dport = conf->frame.ipv6.dport;
        ace.frame.ipv6.tcp_fin = conf->frame.ipv6.tcp_fin;
        ace.frame.ipv6.tcp_syn = conf->frame.ipv6.tcp_syn;
        ace.frame.ipv6.tcp_rst = conf->frame.ipv6.tcp_rst;
        ace.frame.ipv6.tcp_psh = conf->frame.ipv6.tcp_psh;
        ace.frame.ipv6.tcp_ack = conf->frame.ipv6.tcp_ack;
        ace.frame.ipv6.tcp_urg = conf->frame.ipv6.tcp_urg;
#endif /* VTSS_ARCH_LUTON28 */
#if defined(VTSS_FEATURE_ACL_V2)
        ace.frame.ipv6.ptp = conf->frame.ipv6.ptp;
#endif /* VTSS_FEATURE_ACL_V2 */
        break;
    case VTSS_ACE_TYPE_ANY:
    default:
        break;
    }

    rc = vtss_ace_add(NULL, id, &ace);
    T_D("exit, id: %d", id);

    return rc;
}

/* Delete ACE via switch API */
static vtss_rc acl_ace_del(vtss_ace_id_t id)
{
    vtss_rc rc;

    T_D("enter, id: %d", id);
    rc = vtss_ace_del(NULL, id);
    T_D("exit, id: %d", id);

    return rc;
}

/* Change ACE's conflict status */
static void acl_ace_conflict_set(vtss_ace_id_t ace_id, BOOL conflict)
{
    acl_ace_t     *ace;
    acl_list_t    *list;

    T_D("enter, ACE id: %d", ace_id);

    ACL_CRIT_ENTER();
    list = &acl.switch_acl;
    for (ace = list->used; ace != NULL; ace = ace->next) {
        if (ace->conf.id == ace_id) { /* Found existing entry */
            ace->conf.conflict = conflict;
            break;
        }
    }
    ACL_CRIT_EXIT();
}

/*lint -e{593} */
/* There is a lint error message: Custodial pointer 'new' possibly not freed or returned.
   We skip the lint error cause we freed it in acl_list_ace_del() */
/* Add ACE to list */
static vtss_rc acl_list_ace_add(BOOL local, vtss_ace_id_t *next_id, acl_entry_conf_t *conf,
                                vtss_isid_t *isid_del)
{
    vtss_rc       rc = VTSS_OK;
    acl_ace_t     *ace, *prev, *ins = NULL, *ins_prev = NULL, *new = NULL, *new_prev = NULL;
    acl_list_t    *list;
    uchar         id_used[VTSS_BF_SIZE(ACE_ID_END)];
    vtss_ace_id_t i;
    vtss_isid_t   isid;
    int           isid_count[VTSS_ISID_END + 1], found_insertion_place = 0;
    BOOL          new_malloc = FALSE;

    T_D("enter, id: %d, conf->id: %d", *next_id, conf->id);

    if (*next_id == conf->id && ACL_ACE_ID_GET(*next_id) != 0) {
        T_W("illegal user_id: %d, next_id: %d", conf->id, *next_id);
        return ACL_ERROR_PARM;
    }

    memset(id_used, 0, sizeof(id_used));
    memset(isid_count, 0, sizeof(isid_count));
    isid_count[conf->isid]++;

    ACL_CRIT_ENTER();
    list = (local ? &acl.switch_acl : &acl.stack_acl);

    /* Lookup the right place in the ACEs list. */
    for (ace = list->used, prev = NULL; ace != NULL; prev = ace, ace = ace->next) {
        if (ace->conf.id == conf->id) {
            /* Found existing entry */
            new = ace;
            new_prev = prev;
        } else {
            isid_count[ace->conf.isid]++;
        }

        if (ace->conf.id == *next_id) {
            /* Found insertion place */
            ins = ace;
            ins_prev = prev;
        } else if (ACL_ACE_ID_GET(*next_id) == ACE_ID_NONE) {
            if (found_insertion_place == 0 && ACL_USER_ID_GET(conf->id) > ACL_USER_ID_GET(ace->conf.id)) {
                /* Found insertion place by ordered priority */
                ins = ace;
                ins_prev = prev;
                found_insertion_place = 1;
            }
        }

        /* Mark ID as used */
        if (ACL_USER_ID_GET(conf->id) == ACL_USER_ID_GET(ace->conf.id)) {
            VTSS_BF_SET(id_used, ACL_ACE_ID_GET(ace->conf.id) - ACE_ID_START, 1);
        }
    }

    if (ACL_ACE_ID_GET(*next_id) == ACE_ID_NONE && found_insertion_place == 0) {
        ins_prev = prev;
    }

    /* Check that the insert ID was found */
    if (ACL_ACE_ID_GET(*next_id) != ACE_ID_NONE && ins == NULL) {
        T_W("user_id: %d, next_id: %d not found", conf->id, *next_id);
        rc = ACL_ERROR_ACE_NOT_FOUND;
    }

    /* Check that the ACL is not full for any switch */
    if (!local && ACL_USER_ID_GET(conf->id) == ACL_USER_STATIC) {
        for (isid = VTSS_ISID_START; rc == VTSS_OK && isid < VTSS_ISID_END; isid++) {
            if ((isid_count[isid] + isid_count[VTSS_ISID_GLOBAL]) > ACE_MAX) {
                T_W("table is full, isid %d", isid);
                rc = ACL_ERROR_ACE_TABLE_FULL;
            }
        }
    }

    /* Check that a new entry can be allocated */
    if (rc == VTSS_OK && new == NULL && list->free == NULL) {
        /* The maximum number of entries supported by the hardware is exceeded.
           We alloc a new memory for saving this entry and turn on "conflict" flag */
        if ((new = VTSS_MALLOC(sizeof(acl_ace_t))) == NULL) {
            T_D("exit");
            ACL_CRIT_EXIT();
            return ACL_ERROR_MEM_ALLOC_FAIL;
        }
        new->conf.new_allocated = TRUE;
        new->conf.conflict = TRUE;
        new->next = NULL;
        new_malloc = TRUE;
    }

    if (rc == VTSS_OK) {
        T_D("using from %s list", new == NULL ? "free" : "used");
        *isid_del = VTSS_ISID_LOCAL;
        if (new == NULL) {
            /* Use first entry in free list */
            new = list->free;
            if (new) {
                new->conf.new_allocated = FALSE;
                new->conf.conflict = FALSE;
                list->free = new->next;
            }
        } else if (new_malloc == FALSE) {
            /* Take existing entry out of used list */
            if (ins_prev == new) {
                ins_prev = new_prev;
            }
            if (new_prev == NULL) {
                list->used = new->next;
            } else {
                new_prev->next = new->next;
            }

            /* If changing to a specific SID, delete ACE on old SIDs */
            if (new->conf.isid != conf->isid && conf->isid != VTSS_ISID_GLOBAL) {
                *isid_del = new->conf.isid;
            }
        }

        /* Insert new entry in used list */
        if (new) {
            if (ins_prev == NULL) {
                T_D("inserting first");
                new->next = list->used;
                list->used = new;
            } else {
                T_D("inserting after ID %d", ins_prev->conf.id);
                new->next = ins_prev->next;
                ins_prev->next = new;
            }

            if (ACL_ACE_ID_GET(conf->id) == ACE_ID_NONE) {
                /* Use next available ID */
                for (i = ACE_ID_START; i <= ACE_ID_END; i++) {
                    if (!VTSS_BF_GET(id_used, i - ACE_ID_START)) {
                        conf->id = ACL_COMBINED_ID_SET(ACL_USER_ID_GET(conf->id), i);
                        break;
                    }
                }
                if (i > ACE_ID_END) {
                    ACL_CRIT_EXIT();
                    T_W("ACE Auto-assigned fail");
                    if (new_malloc == TRUE) {
                        VTSS_FREE(new);
                    }
                    return ACL_ERROR_ACE_AUTO_ASSIGNED_FAIL;
                }
            }
            conf->new_allocated = new->conf.new_allocated;
            conf->conflict = new->conf.conflict;
            new->conf = *conf;

            /* Update the next_id, it must existing on chip layer when we call acl_ace_add() */
            *next_id = ACE_ID_NONE;
            while (new->next) {
                if (!new->next->conf.conflict) {
                    *next_id = new->next->conf.id;
                    break;
                }
                new = new->next;
            }
            T_D("next_id: %d, conf->id: %d", *next_id, conf->id);
        } else {
            // It should never happpen new = NULL
            T_E("Found new = NULL in acl_list_ace_add()\n");
            rc = ACL_ERROR_GEN;
        }
    }
    ACL_CRIT_EXIT();

    if (rc != VTSS_OK && new_malloc == TRUE) {
        VTSS_FREE(new);
    }

    return rc;
}

static vtss_rc acl_list_ace_del(BOOL local, vtss_ace_id_t id, vtss_isid_t *isid_del, BOOL *conflict)
{
    acl_ace_t  *ace, *prev;
    acl_list_t *list;
    BOOL        found = FALSE;

    T_D("enter, id: %d", id);

    ACL_CRIT_ENTER();
    list = (local ? &acl.switch_acl : &acl.stack_acl);
    for (ace = list->used, prev = NULL; ace != NULL; prev = ace, ace = ace->next) {
        if (ace->conf.id == id) {
            *isid_del = ace->conf.isid;
            if (prev == NULL) {
                list->used = ace->next;
            } else {
                prev->next = ace->next;
            }
            *conflict = ace->conf.conflict;
            if (ace->conf.new_allocated) {
                /* This entry is saving in new memory */
                VTSS_FREE(ace);
            } else {
                /* Move entry from used list to free list */
                ace->next = list->free;
                list->free = ace;
            }
            found = TRUE;
            break;
        }
    }
    ACL_CRIT_EXIT();

    if (found) {
        T_D("exit, id: %d not found", id);
    } else {
        T_D("exit, id: %d found", id);
    }
    return (found ? VTSS_OK : ACL_ERROR_ACE_NOT_FOUND);
}

/* Get ACE or next ACE (use ACE_ID_NONE to get first) */
vtss_rc acl_list_ace_get(vtss_isid_t isid,
                         vtss_ace_id_t id, acl_entry_conf_t *conf,
                         vtss_ace_counter_t *counter, BOOL next)
{
    vtss_rc    rc = VTSS_OK;
    acl_ace_t  *ace;
    acl_list_t *list;
    BOOL       use_next = 0;

    T_D("enter, isid: %d, id: %d, next: %d", isid, id, next);

    ACL_CRIT_ENTER();
    list = (isid == VTSS_ISID_LOCAL ? &acl.switch_acl : &acl.stack_acl);
    for (ace = list->used; ace != NULL; ace = ace->next) {
        /* Skip ACEs for other switches */
        if ((isid != VTSS_ISID_LOCAL && isid != VTSS_ISID_GLOBAL && ace->conf.isid != isid) ||
            ACL_USER_ID_GET(ace->conf.id) != ACL_USER_ID_GET(id)) {
            continue;
        }

        if (ACL_ACE_ID_GET(id) == ACE_ID_NONE) {
            /* Found first ACE */
            break;
        }

        if (use_next) {
            /* Found next ACE */
            break;
        }

        if (ace->conf.id == id) {
            /* Found ACE */
            if (next) {
                use_next = 1;
            } else {
                break;
            }
        }
    }
    if (ace != NULL) {
        /* Found it */
        *conf = ace->conf;
    }
    ACL_CRIT_EXIT();

    T_D("exit, id %d %sfound", id, ace == NULL ? "not " : "");

    /* Get counter */
    if (counter) {
        *counter = 0;
    }
    if (ace != NULL && conf->conflict == FALSE && counter != NULL) {
        if (isid == VTSS_ISID_LOCAL || ace->conf.isid == VTSS_ISID_LOCAL) {
            /* Return local counters on slave switch */
            rc = vtss_ace_counter_get(NULL, conf->id, counter);
        } else {
            rc = acl_stack_ace_get(isid, ace->conf.id, counter);
        }
    }

    return (ace == NULL ? ACL_ERROR_ACE_NOT_FOUND : rc);
}

static void acl_counters_clear(void)
{
    acl_ace_t      *ace;
    vtss_port_no_t port_no;
    u32            port_count = port_isid_port_count(VTSS_ISID_LOCAL);
    T_D("enter");

    /* Clear port counters */
    for (port_no = VTSS_PORT_NO_START; port_no < port_count; port_no++) {
        (void) vtss_acl_port_counter_clear(NULL, port_no);
    }

    /* Clear ACE counters */
    ACL_CRIT_ENTER();
    for (ace = acl.switch_acl.used; ace != NULL; ace = ace->next) {
        if (ace->conf.conflict == FALSE) {
            (void) vtss_ace_counter_clear(NULL, ace->conf.id);
        }
    }
    ACL_CRIT_EXIT();

    T_D("exit");
}

/****************************************************************************/
/*  Stack/switch functions                                                  */
/****************************************************************************/

#if (VTSS_TRACE_LVL_MIN <= VTSS_TRACE_LVL_DEBUG)
static char *acl_msg_id_txt(acl_msg_id_t msg_id)
{
    char *txt;

    switch (msg_id) {
    case ACL_MSG_ID_PORT_CONF_SET_REQ:
        txt = "PORT_CONF_SET_REQ";
        break;
    case ACL_MSG_ID_PORT_COUNTERS_GET_REQ:
        txt = "PORT_COUNTERS_GET_REQ";
        break;
    case ACL_MSG_ID_PORT_COUNTERS_GET_REP:
        txt = "PORT_COUNTERS_GET_REP";
        break;
    case ACL_MSG_ID_POLICER_CONF_SET_REQ:
        txt = "POLICER_CONF_SET_REQ";
        break;
    case ACL_MSG_ID_ACE_CONF_SET_REQ:
        txt = "ACE_CONF_SET_REQ";
        break;
    case ACL_MSG_ID_ACE_CONF_ADD_REQ:
        txt = "ACE_CONF_ADD_REQ";
        break;
    case ACL_MSG_ID_ACE_CONF_DEL_REQ:
        txt = "ACE_CONF_DEL_REQ";
        break;
    case ACL_MSG_ID_ACE_COUNTERS_GET_REQ:
        txt = "ACE_COUNTERS_GET_REQ";
        break;
    case ACL_MSG_ID_ACE_COUNTERS_GET_REP:
        txt = "ACE_COUNTERS_GET_REP";
        break;
    case ACL_MSG_ID_PORT_SHUTDOWN_SET_REQ:
        txt = "PORT_SHUTDOWN_SET_REQ";
        break;
    case ACL_MSG_ID_ACE_CONF_GET_REP:
        txt = "ACL_MSG_ID_ACE_CONF_ADD_REP";
        break;
    case ACL_MSG_ID_ACE_CONF_GET_REQ:
        txt = "ACL_MSG_ID_ACE_CONF_DEL_REQ";
        break;
    default:
        txt = "?";
        break;
    }
    return txt;
}
#endif /* VTSS_TRACE_LVL_DEBUG */

/* Allocate request buffer */
static acl_msg_req_t *acl_msg_req_alloc(acl_msg_id_t msg_id, u32 ref_cnt)
{
    acl_msg_req_t *msg;

    if (ref_cnt == 0) {
        return NULL;
    }

    msg = msg_buf_pool_get(acl.request);
    VTSS_ASSERT(msg);
    if (ref_cnt > 1) {
        msg_buf_pool_ref_cnt_set(msg, ref_cnt);
    }
    msg->msg_id = msg_id;
    return msg;
}

/* Allocate reply buffer */
static acl_msg_rep_t *acl_msg_rep_alloc(acl_msg_id_t msg_id)
{
    acl_msg_rep_t *msg = msg_buf_pool_get(acl.reply);
    VTSS_ASSERT(msg);
    msg->msg_id = msg_id;
    return msg;
}

/* Free request/reply buffer */
static void acl_msg_free(void *msg)
{
    (void)msg_buf_pool_put(msg);
}

static void acl_msg_tx_done(void *contxt, void *msg, msg_tx_rc_t rc)
{
#if (VTSS_TRACE_LVL_MIN <= VTSS_TRACE_LVL_DEBUG)
    acl_msg_id_t msg_id = *(acl_msg_id_t *)msg;
#endif /* VTSS_TRACE_LVL_DEBUG */

    T_D("msg_id: %d, %s", msg_id, acl_msg_id_txt(msg_id));
    acl_msg_free(msg);
}

static void acl_msg_tx(void *msg, vtss_isid_t isid, size_t len)
{
    size_t req_len = len + MSG_TX_DATA_HDR_LEN(acl_msg_req_t, req);
    size_t rep_len = len + MSG_TX_DATA_HDR_LEN(acl_msg_rep_t, rep);

#if (VTSS_TRACE_LVL_MIN <= VTSS_TRACE_LVL_DEBUG)
    acl_msg_id_t msg_id = *(acl_msg_id_t *)msg;
#endif /* VTSS_TRACE_LVL_DEBUG */

    T_D("msg_id: %d, %s, len: %zd, isid: %d", msg_id, acl_msg_id_txt(msg_id), len, isid);
    msg_tx_adv(NULL, acl_msg_tx_done, MSG_TX_OPT_DONT_FREE, VTSS_MODULE_ID_ACL, isid, msg, req_len > rep_len ? req_len : rep_len);
}

static BOOL acl_msg_rx(void *contxt, const void *const rx_msg, const size_t len, const vtss_module_id_t modid, const ulong isid)
{
    u32          port_count = port_isid_port_count(VTSS_ISID_LOCAL);
    acl_msg_id_t msg_id = *(acl_msg_id_t *)rx_msg;

    T_D("msg_id: %d, %s, len: %zd, isid: %u", msg_id, acl_msg_id_txt(msg_id), len, isid);

    switch (msg_id) {
    case ACL_MSG_ID_PORT_CONF_SET_REQ: {
        acl_msg_req_t   *msg;
        vtss_port_no_t  port_no;
        acl_port_conf_t *conf;
        int             i;

        msg = (acl_msg_req_t *)rx_msg;
        for (port_no = VTSS_PORT_NO_START; port_no < port_count; port_no++) {
            i = (port_no - VTSS_PORT_NO_START);
            conf = &msg->req.port_conf_set.conf[i];
            ACL_CRIT_ENTER();
            acl.port_conf[VTSS_ISID_LOCAL][i] = *conf;
            ACL_CRIT_EXIT();
            if (acl_port_conf_set(port_no, conf) != VTSS_OK) {
                T_W("Calling acl_port_conf_set() failed\n");
            }
        }
        acl_packet_register();
        break;
    }
    case ACL_MSG_ID_PORT_COUNTERS_GET_REQ: {
        acl_msg_rep_t           *msg;
        vtss_port_no_t          port_no;
        vtss_acl_port_counter_t *counter;

        msg = acl_msg_rep_alloc(ACL_MSG_ID_PORT_COUNTERS_GET_REP);
        ACL_CRIT_ENTER();
        for (port_no = VTSS_PORT_NO_START; port_no < port_count; port_no++) {
            counter = &msg->rep.port_counters_get.counters[port_no - VTSS_PORT_NO_START];
            (void) vtss_acl_port_counter_get(NULL, port_no, counter);
        }
        ACL_CRIT_EXIT();
        acl_msg_tx(msg, isid, sizeof(msg->rep.port_counters_get));
        break;
    }
    case ACL_MSG_ID_PORT_COUNTERS_GET_REP: {
        acl_msg_rep_t  *msg;
        vtss_port_no_t port_no;
        int            i, j;

        msg = (acl_msg_rep_t *)rx_msg;
        if (VTSS_ISID_LEGAL(isid)) {
            ACL_CRIT_ENTER();
            i = (isid - VTSS_ISID_START);
            for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
                j = (port_no - VTSS_PORT_NO_START);
                acl.port_counters[i][j] = msg->rep.port_counters_get.counters[j];
            }
            VTSS_MTIMER_START(&acl.port_counters_timer[i], ACL_COUNTERS_TIMER);
            ACL_CRIT_EXIT();
            cyg_flag_setbits(&acl.port_counters_flags, 1 << isid);
        }
        break;
    }
    case ACL_MSG_ID_POLICER_CONF_SET_REQ: {
        acl_msg_req_t         *msg;
        vtss_acl_policer_no_t policer_no;
        acl_policer_conf_t    *conf;
        int                   i;

        msg = (acl_msg_req_t *)rx_msg;
        ACL_CRIT_ENTER();
        for (policer_no = VTSS_ACL_POLICER_NO_START;
             policer_no < VTSS_ACL_POLICER_NO_END;
             policer_no++) {
            i = (policer_no - VTSS_ACL_POLICER_NO_START);
            conf = &msg->req.policer_conf_set.conf[i];
            if (!msg_switch_is_master()) { /* Avoid overwriting local data on master */
                acl.policer_conf[i] = *conf;
            }
            if (acl_policer_conf_set(policer_no, conf) != VTSS_OK) {
                T_W("Calling acl_policer_conf_set() failed\n");
            }
        }
        ACL_CRIT_EXIT();
        break;
    }
    case ACL_MSG_ID_ACE_CONF_SET_REQ: {
        acl_msg_req_t       *msg;
        ulong               i;
        vtss_ace_id_t       id;
        acl_entry_conf_t    conf, *conf_ptr;
        vtss_isid_t         dummy;
        BOOL                conflict;
        int                 acl_user_idx;

        msg = (acl_msg_req_t *)rx_msg;

        /* Delete current ACL */
        for (acl_user_idx = ACL_USER_CNT - 1; acl_user_idx >= ACL_USER_STATIC; acl_user_idx--) {
            if (acl_user_reg_modes[acl_user_idx] == ACL_USER_REG_MODE_LOCAL) {
                continue;
            }
            id = ACE_ID_NONE;
            while (acl_list_ace_get(VTSS_ISID_LOCAL, ACL_COMBINED_ID_SET(acl_user_idx, id), &conf, NULL, TRUE) == VTSS_OK) {
                if (acl_list_ace_del(1, conf.id, &dummy, &conflict) == VTSS_OK && conflict == FALSE) {
                    /* Remove this entry from ASIC layer */
                    if (acl_ace_del(conf.id) != VTSS_OK) {
                        T_W("Calling acl_ace_del() failed\n");
                    }
                }
            }
        }

        /* Add new ACL */
        id = ACE_ID_NONE;
        for (i = 0; i < msg->req.ace_conf_set.count; i++) {
            conf_ptr = &msg->req.ace_conf_set.table[i];
            if (acl_list_ace_add(1, &id, conf_ptr, &dummy) == VTSS_OK && conf_ptr->new_allocated == FALSE) {
                /* Set this entry to ASIC layer */
                if (acl_ace_add(id, conf_ptr) != VTSS_OK) {
                    conf_ptr->conflict = TRUE;
                    acl_ace_conflict_set(conf_ptr->id, conf_ptr->conflict);
                }
            }
        }
        acl_packet_register();
        break;
    }
    case ACL_MSG_ID_ACE_CONF_ADD_REQ: {
        acl_msg_req_t    *msg;
        vtss_ace_id_t    id;
        acl_entry_conf_t *conf;
        vtss_isid_t      dummy;

        msg = (acl_msg_req_t *)rx_msg;
        id = msg->req.ace_conf_add.id;
        conf = &msg->req.ace_conf_add.conf;
        if (acl_list_ace_add(1, &id, conf, &dummy) == VTSS_OK && conf->new_allocated == FALSE) {
            /* Set this entry to ASIC layer */
            if (acl_ace_add(id, conf) == VTSS_OK) {
                acl_packet_register();
            } else {
                conf->conflict = TRUE;
                acl_ace_conflict_set(conf->id, conf->conflict);
            }
        }
        break;
    }
    case ACL_MSG_ID_ACE_CONF_DEL_REQ: {
        acl_msg_req_t   *msg;
        vtss_ace_id_t   id;
        vtss_isid_t     dummy;
        BOOL            conflict = 1;

        msg = (acl_msg_req_t *)rx_msg;
        id = msg->req.ace_conf_del.id;
        if (acl_list_ace_del(1, id, &dummy, &conflict) == VTSS_OK && conflict == FALSE) {
            /* Remove this entry from ASIC layer */
            if (acl_ace_del(id) == VTSS_OK) {
                acl_packet_register();
            } else {
                T_W("Calling acl_ace_del() failed\n");
            }
            acl_mgmt_conflict_solve();
        }
        break;
    }
    case ACL_MSG_ID_ACE_COUNTERS_GET_REQ: {
        acl_msg_rep_t *msg;
        acl_ace_t     *ace;
        ace_counter_t *ace_counter;
        int           i;

        msg = acl_msg_rep_alloc(ACL_MSG_ID_ACE_COUNTERS_GET_REP);
        ACL_CRIT_ENTER();
        for (ace = acl.switch_acl.used, i = 0; ace != NULL; ace = ace->next) {
            if (ace->conf.conflict) {
                continue;
            }
            ace_counter = &msg->rep.ace_counters_get.counters[i++];
            ace_counter->id = ace->conf.id;
            ace_counter->counter = 0;
            (void) vtss_ace_counter_get(NULL, ace->conf.id, &ace_counter->counter);
            if (i == ACE_MAX) {
                T_W("ACL_MSG_ID_ACE_COUNTERS_GET_REQ: Reach maximum of ACE number");
                break;
            }
        }
        ACL_CRIT_EXIT();
        msg->rep.ace_counters_get.count = i;
        acl_msg_tx(msg, isid, sizeof(msg->rep.ace_counters_get));
        break;
    }
    case ACL_MSG_ID_ACE_COUNTERS_GET_REP: {
        acl_msg_rep_t *msg;
        ace_counter_t *ace_counter;
        ulong         i, j;

        msg = (acl_msg_rep_t *)rx_msg;
        if (VTSS_ISID_LEGAL(isid)) {
            i = (isid - VTSS_ISID_START);
            ACL_CRIT_ENTER();
            for (j = 0; j < ACE_MAX; j++) {
                ace_counter = &acl.ace_counters[i][j];
                ace_counter->id = VTSS_ACE_ID_LAST;
                if (j < msg->rep.ace_counters_get.count) {
                    *ace_counter = msg->rep.ace_counters_get.counters[j];
                }
            }
            VTSS_MTIMER_START(&acl.ace_counters_timer[i], ACL_COUNTERS_TIMER);
            cyg_flag_setbits(&acl.ace_counters_flags, 1 << isid);
            ACL_CRIT_EXIT();
        }
        break;
    }
    case ACL_MSG_ID_COUNTERS_CLEAR_REQ: {
        acl_counters_clear();
        break;
    }
    case ACL_MSG_ID_PORT_SHUTDOWN_SET_REQ: {
        acl_msg_req_t   *msg;
        vtss_port_no_t  port_no;
        port_vol_conf_t conf;
        port_user_t     user = PORT_USER_ACL;

        msg = (acl_msg_req_t *)rx_msg;
        port_no = msg->req.port_shutdown.port_no;
        T_D("shutdown isid: %d, port_no: %u", isid, port_no);
        if (port_no < port_isid_port_count(isid) &&
            port_vol_conf_get(user, isid, port_no, &conf) == VTSS_OK) {
            conf.disable = 1;
            if (port_vol_conf_set(user, isid, port_no, &conf) != VTSS_OK) {
                T_W("port_vol_conf_set() failed");
            }
        }
        break;
    }
    case ACL_MSG_ID_ACE_CONF_GET_REQ: {
        acl_msg_req_t *msg;
        acl_msg_rep_t *rep_msg;
        acl_entry_conf_t conf;
        vtss_ace_counter_t counter = 0;
        vtss_rc rc;

        msg = (acl_msg_req_t *)rx_msg;
        if (msg->req.ace_conf_get.counter) {
            rc = acl_list_ace_get(VTSS_ISID_LOCAL, msg->req.ace_conf_get.id, &conf, &counter, msg->req.ace_conf_get.next);
        } else {
            rc = acl_list_ace_get(VTSS_ISID_LOCAL, msg->req.ace_conf_get.id, &conf, NULL, msg->req.ace_conf_get.next);
        }
        rep_msg = acl_msg_rep_alloc(ACL_MSG_ID_ACE_CONF_GET_REP);
        ACL_CRIT_ENTER();
        if (rc == VTSS_OK) {
            rep_msg->rep.ace_conf_get.conf = conf;
            if (msg->req.ace_conf_get.counter) {
                rep_msg->rep.ace_conf_get.counter = counter;
            } else {
                rep_msg->rep.ace_conf_get.counter = 0;
            }
        } else {
            rep_msg->rep.ace_conf_get.conf.id = ACE_ID_NONE;
        }
        ACL_CRIT_EXIT();
        acl_msg_tx(rep_msg, 0, sizeof(rep_msg->rep.ace_conf_get));
        break;
    }
    case ACL_MSG_ID_ACE_CONF_GET_REP: {
        acl_msg_rep_t *msg;

        msg = (acl_msg_rep_t *)rx_msg;

        ACL_CRIT_ENTER();
        acl.ace_conf_get_rep_info[isid] = msg->rep.ace_conf_get.conf;
        acl.ace_conf_get_rep_counter[isid] = msg->rep.ace_conf_get.counter;
        ACL_CRIT_EXIT();
        cyg_flag_setbits(&acl.ace_conf_get_flags, 1 << isid);
        break;
    }
    default:
        T_W("unknown message ID: %d", msg_id);
        break;
    }
    return TRUE;
}

static vtss_rc acl_stack_register(void)
{
    msg_rx_filter_t filter;

    memset(&filter, 0, sizeof(filter));
    filter.cb = acl_msg_rx;
    filter.modid = VTSS_MODULE_ID_ACL;
    return msg_rx_filter_register(&filter);
}

/* Set port configuration */
static vtss_rc acl_stack_port_conf_set(vtss_isid_t isid)
{
    acl_msg_req_t  *msg;
    vtss_port_no_t port_no;
    int            i;

    T_D("enter, isid: %d", isid);
    msg = acl_msg_req_alloc(ACL_MSG_ID_PORT_CONF_SET_REQ, 1);
    ACL_CRIT_ENTER();
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        i = (port_no - VTSS_PORT_NO_START);
        msg->req.port_conf_set.conf[i] = acl.port_conf[isid][i];
    }
    ACL_CRIT_EXIT();
    acl_msg_tx(msg, isid, sizeof(msg->req.port_conf_set));
    T_D("exit, isid: %d", isid);

    return VTSS_OK;
}

/* Wait for reply to request */
static BOOL acl_stack_req_timeout(vtss_isid_t isid, acl_msg_id_t msg_id,
                                  vtss_mtimer_t *timer, cyg_flag_t *flags)
{
    acl_msg_req_t    *msg;
    BOOL             timeout;
    cyg_flag_value_t flag;
    cyg_tick_count_t time_tick;

    T_D("enter, isid: %d", isid);

    ACL_CRIT_ENTER();
    timeout = VTSS_MTIMER_TIMEOUT(timer);
    ACL_CRIT_EXIT();

    if (timeout) {
        T_D("info old, sending GET_REQ(isid=%d)", isid);
        msg = acl_msg_req_alloc(msg_id, 1);
        flag = (1 << isid);
        cyg_flag_maskbits(flags, ~flag);
        acl_msg_tx(msg, isid, 0);
        time_tick = cyg_current_time() + VTSS_OS_MSEC2TICK(ACL_REQ_TIMEOUT * 1000);
        return (cyg_flag_timed_wait(flags, flag, CYG_FLAG_WAITMODE_OR, time_tick) & flag ? 0 : 1);
    }
    return 0;
}

/*lint -sem(acl_stack_port_counter_get, thread_protected)*/
/* Its safe to access global var 'port_counters_flags' */
/* Get ACL port counter */
static vtss_rc acl_stack_port_counter_get(vtss_isid_t isid,
                                          vtss_port_no_t port_no,
                                          vtss_acl_port_counter_t *counter)
{
    T_D("enter, isid: %d, port_no: %u", isid, port_no);

    if (acl_stack_req_timeout(isid, ACL_MSG_ID_PORT_COUNTERS_GET_REQ,
                              &acl.port_counters_timer[isid - VTSS_ISID_START],
                              &acl.port_counters_flags)) {
        T_W("timeout, PORT_COUNTERS_GET_REQ");
        return ACL_ERROR_REQ_TIMEOUT;
    }

    ACL_CRIT_ENTER();
    *counter = acl.port_counters[isid - VTSS_ISID_START][port_no - VTSS_PORT_NO_START];
    ACL_CRIT_EXIT();

    T_D("exit, isid: %d, port_no: %u", isid, port_no);
    return VTSS_OK;
}

/* Set ACL policer configuration */
static vtss_rc acl_stack_policer_conf_set(vtss_isid_t isid_add)
{
    acl_msg_req_t         *msg;
    vtss_acl_policer_no_t policer_no;
    int                   i;
    switch_iter_t         sit;

    T_D("enter, isid: %d", isid_add);

    (void)switch_iter_init(&sit, isid_add, SWITCH_ITER_SORT_ORDER_ISID);

    /* Allocate a message with a ref-count corresponding to the number of times switch_iter_getnext() will return TRUE. */
    if ((msg = acl_msg_req_alloc(ACL_MSG_ID_POLICER_CONF_SET_REQ, sit.remaining)) != NULL) {
        ACL_CRIT_ENTER();
        for (policer_no = VTSS_ACL_POLICER_NO_START; policer_no < VTSS_ACL_POLICER_NO_END; policer_no++) {
            i = policer_no - VTSS_ACL_POLICER_NO_START;
            msg->req.policer_conf_set.conf[i] = acl.policer_conf[i];
        }
        ACL_CRIT_EXIT();

        while (switch_iter_getnext(&sit)) {
            acl_msg_tx(msg, sit.isid, sizeof(msg->req.policer_conf_set));
        }
    }

    T_D("exit, isid: %d", isid_add);
    return VTSS_OK;
}

/*lint -sem(acl_stack_ace_get, thread_protected)*/
/* Its safe to access global var 'ace_counters_flags' */
/* Get ACE counter */
static vtss_rc acl_stack_ace_get(vtss_isid_t isid_get,
                                 vtss_ace_id_t id, vtss_ace_counter_t *counter)
{
    int           i;
    vtss_isid_t   isid;
    ace_counter_t *ace_counter;

    T_N("enter, isid: %d, id: %d", isid_get, id);

    *counter = 0;
    for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
        if ((isid_get != VTSS_ISID_GLOBAL && isid_get != isid) ||
            !msg_switch_exists(isid)) {
            continue;
        }

        if (acl_stack_req_timeout(isid, ACL_MSG_ID_ACE_COUNTERS_GET_REQ,
                                  &acl.ace_counters_timer[isid - VTSS_ISID_START],
                                  &acl.ace_counters_flags)) {
            T_W("timeout, ACE_COUNTERS_GET_REQ");
            return ACL_ERROR_REQ_TIMEOUT;
        }

        ACL_CRIT_ENTER();
        for (i = 0; i < ACE_MAX; i++) {
            ace_counter = &acl.ace_counters[isid - VTSS_ISID_START][i];
            if (ace_counter->id == id) {
                *counter += ace_counter->counter;
                break;
            }
        }
        ACL_CRIT_EXIT();
    }

    T_N("exit, isid: %d, id: %d", isid_get, id);
    return VTSS_OK;
}

/* Set ACE configuration to switch */
static vtss_rc acl_stack_ace_conf_set(vtss_isid_t isid_add)
{
    acl_msg_req_t *msg;
    vtss_isid_t   isid;
    int           i;
    acl_ace_t     *ace;

    T_D("enter, isid: %d", isid_add);

    for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
        if ((isid_add != VTSS_ISID_GLOBAL && isid_add != isid) ||
            !msg_switch_exists(isid)) {
            continue;
        }

        msg = acl_msg_req_alloc(ACL_MSG_ID_ACE_CONF_SET_REQ, 1);
        ACL_CRIT_ENTER();
        for (ace = acl.stack_acl.used, i = 0; ace != NULL; ace = ace->next) {
            if (ace->conf.isid == VTSS_ISID_GLOBAL || ace->conf.isid == isid) {
                if (i >= ACE_MAX + ACL_MAX_EXTEND_ACE_CNT) {
                    T_W("Calling acl_stack_ace_conf_set(): Reach maximum ACEs count");
                    break;
                }
                msg->req.ace_conf_set.table[i++] = ace->conf;
            }
        }
        ACL_CRIT_EXIT();
        msg->req.ace_conf_set.count = i;
        acl_msg_tx(msg, isid, sizeof(msg->req.ace_conf_set) - (ACE_MAX + ACL_MAX_EXTEND_ACE_CNT - i) * sizeof(acl_entry_conf_t));
    }

    T_D("exit, isid: %d", isid_add);
    return VTSS_OK;
}

/* Add ACE to switch */
static vtss_rc acl_stack_ace_conf_add(vtss_isid_t isid_add, vtss_ace_id_t id)
{
    acl_msg_req_t *msg;
    vtss_isid_t   isid;
    acl_ace_t     *ace;
    BOOL          found;

    T_D("enter, isid: %d, id: %d", isid_add, id);

    for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
        if ((isid_add != VTSS_ISID_GLOBAL && isid_add != isid) ||
            !msg_switch_exists(isid)) {
            continue;
        }

        msg = acl_msg_req_alloc(ACL_MSG_ID_ACE_CONF_ADD_REQ, 1);
        msg->req.ace_conf_add.id = ACE_ID_NONE;
        ACL_CRIT_ENTER();
        for (found = 0, ace = acl.stack_acl.used; ace != NULL; ace = ace->next) {
            if (ace->conf.isid == VTSS_ISID_GLOBAL || ace->conf.isid == isid) {
                if (found) {
                    msg->req.ace_conf_add.id = ace->conf.id;
                    break;
                } else if (ace->conf.id == id) {
                    msg->req.ace_conf_add.conf = ace->conf;
                    found = 1;
                }
            }
        }
        ACL_CRIT_EXIT();
        if (found) {
            acl_msg_tx(msg, isid, sizeof(msg->req.ace_conf_add));
        } else {
            T_W("ACE ID %d not found, isid: %d", id, isid);
            acl_msg_free(msg);
        }
    }

    T_D("exit, isid: %d, id: %d", isid_add, id);
    return VTSS_OK;
}

/* Delete ACE from switch */
static vtss_rc acl_stack_ace_conf_del(vtss_isid_t isid_del, vtss_ace_id_t id)
{
    acl_msg_req_t *msg;
    switch_iter_t sit;

    T_D("enter, isid: %d, id: %d", isid_del, id);

    (void)switch_iter_init(&sit, isid_del, SWITCH_ITER_SORT_ORDER_ISID);

    /* Allocate a message with a ref-count corresponding to the number of times switch_iter_getnext() will return TRUE. */
    if ((msg = acl_msg_req_alloc(ACL_MSG_ID_ACE_CONF_DEL_REQ, sit.remaining)) != NULL) {
        msg->req.ace_conf_del.id = id;

        while (switch_iter_getnext(&sit)) {
            acl_msg_tx(msg, sit.isid, sizeof(msg->req.ace_conf_del));
        }
    }

    T_D("exit, isid: %d, id: %d", isid_del, id);
    return VTSS_OK;
}

/* Clear all ACL counters in stack */
static vtss_rc acl_stack_clear(void)
{
    acl_msg_req_t *msg;
    switch_iter_t sit;

    (void)switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID);

    /* Allocate a message with a ref-count corresponding to the number of times switch_iter_getnext() will return TRUE. */
    if ((msg = acl_msg_req_alloc(ACL_MSG_ID_COUNTERS_CLEAR_REQ, sit.remaining)) != NULL) {
        while (switch_iter_getnext(&sit)) {
            acl_msg_tx(msg, sit.isid, 0);
        }
    }

    return VTSS_OK;
}

#ifdef VTSS_SW_OPTION_PACKET
/* Shutdown port */
static void acl_stack_port_shutdown(vtss_port_no_t port_no)
{
    acl_msg_req_t *msg;

    msg = acl_msg_req_alloc(ACL_MSG_ID_PORT_SHUTDOWN_SET_REQ, 1);
    msg->req.port_shutdown.port_no = port_no;
    acl_msg_tx(msg, 0, sizeof(msg->req.port_shutdown));
}

/****************************************************************************/
/*  Packet logging functions                                                */
/****************************************************************************/
static ushort acl_getb16(uchar *p)
{
    return (p[0] << 8 | p[1]);
}

static ulong acl_getb32(uchar *p)
{
    return ((p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3]);
}

static BOOL acl_flag_mismatch(acl_entry_conf_t *conf, uchar *flags, acl_flag_t flag)
{
    return (VTSS_BF_GET(conf->flags.mask, flag) &&
            VTSS_BF_GET(conf->flags.value, flag) != VTSS_BF_GET(flags, flag));
}

static BOOL acl_packet_rx(void *contxt, const uchar *const frame, const vtss_packet_rx_info_t *const rx_info)
{
    ulong                len;
    uint                 i, n;
    vtss_port_no_t       port_no;
    vtss_ace_type_t      type;
    uchar                *dmac, *smac, *p;
    BOOL                 bc, match;
    ushort               etype, sport = 0, dport = 0;
    uchar                vh, ds = 0, proto = 0, tcp_flags;
    ulong                sip = 0, dip = 0;
    acl_ace_t            *ace;
    acl_entry_conf_t     *conf;
    uchar                flags[ACE_FLAG_SIZE];
    acl_port_conf_t      *port_conf;
    acl_action_t         *action;
    vtss_mtimer_t        *timer;
    BOOL                 is_permit, found_matched = FALSE;
    vtss_vid_t           vid;
    struct ip6_hdr       *ip6_hdr = NULL;
    vtss_ipv6_t          sip_v6, dip_v6;

    memset(&sip_v6, 0, sizeof(sip_v6));
    memset(&dip_v6, 0, sizeof(dip_v6));

    len = rx_info->length;
    port_no = rx_info->port_no;
    vid = rx_info->tag.vid;
    T_N("Frame of length %d received on port %u, vid: %u", len, port_no, vid);
    T_N_HEX(frame, len);

    /* Extract MAC header fields */
    p = (uchar *)frame;
    dmac = (p + 0);
    smac = (p + 6);
    etype = acl_getb16(p + 12);
    p += 14; /* Skip MAC header */

    /* Determine DMAC flags */
    memset(flags, 0, sizeof(flags));
    for (bc = 1, i = 0; i < 6; i++)
        if (dmac[i] != 0xff) {
            bc = 0;
        }
    VTSS_BF_SET(flags, ACE_FLAG_DMAC_BC, bc);
    VTSS_BF_SET(flags, ACE_FLAG_DMAC_MC, dmac[0] & 0x01);

    /* Extract frame type specific fields */
    switch (etype) {
    case 0x0800: /* IPv4 */
        vh = p[0];
        if ((vh & 0xf0) != 0x40) { /* Not IPv4 */
            type = VTSS_ACE_TYPE_ETYPE;
            break;
        }
        type = VTSS_ACE_TYPE_IPV4;
        ds = p[1];
        VTSS_BF_SET(flags, ACE_FLAG_IP_FRAGMENT, acl_getb16(p + 6) & 0x3fff);
        VTSS_BF_SET(flags, ACE_FLAG_IP_TTL, p[8]);
        VTSS_BF_SET(flags, ACE_FLAG_IP_OPTIONS, vh != 0x45);
        proto = p[9];
        sip = acl_getb32(p + 12);
        dip = acl_getb32(p + 16);
        p += ((vh & 0x0f) * 4); /* Skip IP header */
        sport = acl_getb16(p);
        dport = acl_getb16(p + 2);
        if (proto == 6) {
            tcp_flags = p[13];
            VTSS_BF_SET(flags, ACE_FLAG_TCP_FIN, tcp_flags & 0x01);
            VTSS_BF_SET(flags, ACE_FLAG_TCP_SYN, tcp_flags & 0x02);
            VTSS_BF_SET(flags, ACE_FLAG_TCP_RST, tcp_flags & 0x04);
            VTSS_BF_SET(flags, ACE_FLAG_TCP_PSH, tcp_flags & 0x08);
            VTSS_BF_SET(flags, ACE_FLAG_TCP_ACK, tcp_flags & 0x10);
            VTSS_BF_SET(flags, ACE_FLAG_TCP_URG, tcp_flags & 0x20);
        }
        break;
    case 0x0806: /* ARP/RARP */
    case 0x8035:
        type = VTSS_ACE_TYPE_ARP;

        /* Opcode flags */
        switch (acl_getb16(p + 6)) {
        case 1:  /* ARP request */
            VTSS_BF_SET(flags, ACE_FLAG_ARP_ARP, 1);
            VTSS_BF_SET(flags, ACE_FLAG_ARP_REQ, 1);
            break;
        case 2:  /* ARP reply */
            VTSS_BF_SET(flags, ACE_FLAG_ARP_ARP, 1);
            break;
        case 3:  /* RARP request */
            VTSS_BF_SET(flags, ACE_FLAG_ARP_REQ, 1);
            break;
        case 4:  /* RARP reply */
            break;
        default: /* Unknown */
            VTSS_BF_SET(flags, ACE_FLAG_ARP_UNKNOWN, 1);
            break;
        }

        /* SMAC flag */
        for (i = 0, match = 1; i < 6; i++)
            if (smac[i] != p[8 + i]) {
                match = 0;
            }
        VTSS_BF_SET(flags, ACE_FLAG_ARP_SMAC, match);

        /* DMAC flag */
        for (i = 0, match = 1; i < 6; i++)
            if (dmac[i] != p[18 + i]) {
                match = 0;
            }
        VTSS_BF_SET(flags, ACE_FLAG_ARP_DMAC, match);

        /* Length, IP and Ethernet flags */
        VTSS_BF_SET(flags, ACE_FLAG_ARP_LEN, p[4] == 6 && p[5] == 4);
        VTSS_BF_SET(flags, ACE_FLAG_ARP_IP, acl_getb16(p + 2) == 0x0800);
        VTSS_BF_SET(flags, ACE_FLAG_ARP_ETHER, acl_getb16(p + 0) == 0x0001);

        /* SIP/DIP */
        sip = acl_getb32(p + 14);
        dip = acl_getb32(p + 24);
        break;
    case 0x86DD: /* IPv6 */
        vh = p[0];
        if ((vh & 0xf0) != 0x60) { /* Not IPv6 */
            type = VTSS_ACE_TYPE_ETYPE;
            break;
        }
        type = VTSS_ACE_TYPE_IPV6;
        ip6_hdr = (struct ip6_hdr *)p;
        memcpy(&sip_v6, ip6_hdr->ip6_src.__u6_addr.__u6_addr8, sizeof(sip_v6));
        memcpy(&dip_v6, ip6_hdr->ip6_dst.__u6_addr.__u6_addr8, sizeof(dip_v6));

        ds = p[1];
        VTSS_BF_SET(flags, ACE_FLAG_IP_TTL, ip6_hdr->ip6_ctlun.ip6_un1.ip6_un1_hlim);
        proto = ip6_hdr->ip6_ctlun.ip6_un1.ip6_un1_nxt;
        p += sizeof(struct ip6_hdr); /* Skip IPv6 header */
        sport = acl_getb16(p);
        dport = acl_getb16(p + 2);
        if (proto == 6) {
            tcp_flags = p[13];
            VTSS_BF_SET(flags, ACE_FLAG_TCP_FIN, tcp_flags & 0x01);
            VTSS_BF_SET(flags, ACE_FLAG_TCP_SYN, tcp_flags & 0x02);
            VTSS_BF_SET(flags, ACE_FLAG_TCP_RST, tcp_flags & 0x04);
            VTSS_BF_SET(flags, ACE_FLAG_TCP_PSH, tcp_flags & 0x08);
            VTSS_BF_SET(flags, ACE_FLAG_TCP_ACK, tcp_flags & 0x10);
            VTSS_BF_SET(flags, ACE_FLAG_TCP_URG, tcp_flags & 0x20);
        }
        break;
    default:
        if (etype >= 0x0600) {
            type = VTSS_ACE_TYPE_ETYPE;
        } else if (p[0] == 0xaa && p[1] == 0xaa && p[2] == 0x03) {
            type = VTSS_ACE_TYPE_SNAP;
        } else {
            type = VTSS_ACE_TYPE_LLC;
        }
        break;
    }

    ACL_CRIT_ENTER();

    /* Look for matching ACE */
    port_conf = &acl.port_conf[VTSS_ISID_LOCAL][port_no - VTSS_PORT_NO_START];
    action = &port_conf->action;
    for (ace = acl.switch_acl.used; ace != NULL && ace->conf.conflict == FALSE; ace = ace->next) {
        conf = &ace->conf;

#if defined(VTSS_FEATURE_ACL_V2)
        /* Rule type */
        if (conf->port_list[port_no] != TRUE ||
            ((conf->policy.value & conf->policy.mask) != (port_conf->policy_no & conf->policy.mask))) {
            continue;
        }
#else
        if (conf->port_no != VTSS_PORT_NO_ANY &&
            (conf->port_no != port_no ||
             ((conf->policy.value & conf->policy.mask) != (port_conf->policy_no & conf->policy.mask)))) {
            continue;
        }
#endif /* VTSS_FEATURE_ACL_V2 */

        /* DMAC flags */
        if (acl_flag_mismatch(conf, flags, ACE_FLAG_DMAC_BC) ||
            acl_flag_mismatch(conf, flags, ACE_FLAG_DMAC_MC)) {
            continue;
        }

        /* VLAN ID */
        if ((vid & conf->vid.mask) != (conf->vid.value & conf->vid.mask)) {
            continue;
        }

        /* Frame type */
        if (conf->type == VTSS_ACE_TYPE_ANY) {
            type = VTSS_ACE_TYPE_ANY;
        } else if (conf->type != type) {
            continue;
        }

        switch (type) {
        case VTSS_ACE_TYPE_ANY:
            break;
        case VTSS_ACE_TYPE_ETYPE:
            /* SMAC/DMAC */
            for (i = 0, match = 1; i < 6; i++)
                if ((dmac[i] & conf->frame.etype.dmac.mask[i]) !=
                    (conf->frame.etype.dmac.value[i] & conf->frame.etype.dmac.mask[i]) ||
                    (smac[i] & conf->frame.etype.smac.mask[i]) !=
                    (conf->frame.etype.smac.value[i] & conf->frame.etype.smac.mask[i])) {
                    match = 0;
                }
            if (!match) {
                continue;
            }

            /* Ethernet Type */
            if (((etype >> 8) & conf->frame.etype.etype.mask[0]) !=
                (conf->frame.etype.etype.value[0] & conf->frame.etype.etype.mask[0]) ||
                (etype & conf->frame.etype.etype.mask[1]) !=
                (conf->frame.etype.etype.value[1] & conf->frame.etype.etype.mask[1])) {
                continue;
            }

            /* Ethernet data */
            for (i = 0, match = 1; i < 2; i++)
                if ((p[i] & conf->frame.etype.data.mask[i]) !=
                    (conf->frame.etype.data.value[i] & conf->frame.etype.data.mask[i])) {
                    match = 0;
                }
            if (!match) {
                continue;
            }
            break;
        case VTSS_ACE_TYPE_ARP:
            /* ARP flags */
            if (acl_flag_mismatch(conf, flags, ACE_FLAG_ARP_ARP) ||
                acl_flag_mismatch(conf, flags, ACE_FLAG_ARP_REQ) ||
                acl_flag_mismatch(conf, flags, ACE_FLAG_ARP_UNKNOWN) ||
                acl_flag_mismatch(conf, flags, ACE_FLAG_ARP_SMAC) ||
                acl_flag_mismatch(conf, flags, ACE_FLAG_ARP_DMAC) ||
                acl_flag_mismatch(conf, flags, ACE_FLAG_ARP_LEN) ||
                acl_flag_mismatch(conf, flags, ACE_FLAG_ARP_IP) ||
                acl_flag_mismatch(conf, flags, ACE_FLAG_ARP_ETHER)) {
                continue;
            }

            /* SMAC */
            for (i = 0, match = 1; i < 6; i++)
                if ((smac[i] & conf->frame.arp.smac.mask[i]) !=
                    (conf->frame.arp.smac.value[i] & conf->frame.arp.smac.mask[i])) {
                    match = 0;
                }
            if (!match) {
                continue;
            }

            /* SIP/DIP */
            if ((sip & conf->frame.arp.sip.mask) !=
                (conf->frame.arp.sip.value & conf->frame.arp.sip.mask) ||
                (dip & conf->frame.arp.dip.mask) !=
                (conf->frame.arp.dip.value & conf->frame.arp.dip.mask)) {
                continue;
            }

            break;
        case VTSS_ACE_TYPE_IPV4:
            /* IP flags */
            if (acl_flag_mismatch(conf, flags, ACE_FLAG_IP_TTL) ||
                acl_flag_mismatch(conf, flags, ACE_FLAG_IP_FRAGMENT) ||
                acl_flag_mismatch(conf, flags, ACE_FLAG_IP_OPTIONS)) {
                continue;
            }

            /* DS, proto, SIP and DIP */
            if ((ds & conf->frame.ipv4.ds.mask) !=
                (conf->frame.ipv4.ds.value & conf->frame.ipv4.ds.mask) ||
                (proto & conf->frame.ipv4.proto.mask) !=
                (conf->frame.ipv4.proto.value & conf->frame.ipv4.proto.mask) ||
                (sip & conf->frame.ipv4.sip.mask) !=
                (conf->frame.ipv4.sip.value & conf->frame.ipv4.sip.mask) ||
                (dip & conf->frame.ipv4.dip.mask) !=
                (conf->frame.ipv4.dip.value & conf->frame.ipv4.dip.mask)) {
                continue;
            }

            if (proto == 6 || proto == 17) {
                /* UDP/TCP port numbers */
                if (sport < conf->frame.ipv4.sport.low ||
                    sport > conf->frame.ipv4.sport.high ||
                    dport < conf->frame.ipv4.dport.low ||
                    dport > conf->frame.ipv4.dport.high) {
                    continue;
                }

                /* TCP flags */
                if (proto == 6 &&
                    (acl_flag_mismatch(conf, flags, ACE_FLAG_TCP_FIN) ||
                     acl_flag_mismatch(conf, flags, ACE_FLAG_TCP_SYN) ||
                     acl_flag_mismatch(conf, flags, ACE_FLAG_TCP_RST) ||
                     acl_flag_mismatch(conf, flags, ACE_FLAG_TCP_PSH) ||
                     acl_flag_mismatch(conf, flags, ACE_FLAG_TCP_ACK) ||
                     acl_flag_mismatch(conf, flags, ACE_FLAG_TCP_URG))) {
                    continue;
                }
            } else {
                /* IP data */
                for (i = 0, match = 1; i < 6; i++)
                    if ((p[i] & conf->frame.ipv4.data.mask[i]) !=
                        (conf->frame.ipv4.data.value[i] & conf->frame.ipv4.data.mask[i])) {
                        match = 0;
                    }
                if (!match) {
                    continue;
                }
            }
            break;

        case VTSS_ACE_TYPE_IPV6: {
            /* DS, proto */
            if ((ds & conf->frame.ipv6.ds.mask) !=
                (conf->frame.ipv6.ds.value & conf->frame.ipv6.ds.mask) ||
                (proto & conf->frame.ipv6.proto.mask) !=
                (conf->frame.ipv6.proto.value & conf->frame.ipv6.proto.mask)) {
                continue;
            }

            /* SIPv6 */
            u32 v6_network = ((sip_v6.addr[12] & conf->frame.ipv6.sip.mask[12]) << 24) |
                             ((sip_v6.addr[13] & conf->frame.ipv6.sip.mask[13]) << 16) |
                             ((sip_v6.addr[14] & conf->frame.ipv6.sip.mask[14]) << 8) |
                             (sip_v6.addr[15] & conf->frame.ipv6.sip.mask[15]);
            if (v6_network !=
                (((conf->frame.ipv6.sip.value[12] & conf->frame.ipv6.sip.mask[12]) << 24) |
                 ((conf->frame.ipv6.sip.value[13] & conf->frame.ipv6.sip.mask[13]) << 16) |
                 ((conf->frame.ipv6.sip.value[14] & conf->frame.ipv6.sip.mask[14]) << 8) |
                 (conf->frame.ipv6.sip.value[15] & conf->frame.ipv6.sip.mask[15]))) {
                continue;
            }

            if (proto == 6 || proto == 17) {
                /* UDP/TCP port numbers */
                if (sport < conf->frame.ipv6.sport.low ||
                    sport > conf->frame.ipv6.sport.high ||
                    dport < conf->frame.ipv6.dport.low ||
                    dport > conf->frame.ipv6.dport.high) {
                    continue;
                }

                /* TCP flags */
                if (proto == 6 &&
                    (acl_flag_mismatch(conf, flags, ACE_FLAG_TCP_FIN) ||
                     acl_flag_mismatch(conf, flags, ACE_FLAG_TCP_SYN) ||
                     acl_flag_mismatch(conf, flags, ACE_FLAG_TCP_RST) ||
                     acl_flag_mismatch(conf, flags, ACE_FLAG_TCP_PSH) ||
                     acl_flag_mismatch(conf, flags, ACE_FLAG_TCP_ACK) ||
                     acl_flag_mismatch(conf, flags, ACE_FLAG_TCP_URG))) {
                    continue;
                }
            } else {
                /* IP data */
                for (i = 0, match = 1; i < 6; i++)
                    if ((p[i] & conf->frame.ipv6.data.mask[i]) !=
                        (conf->frame.ipv6.data.value[i] & conf->frame.ipv6.data.mask[i])) {
                        match = 0;
                    }
                if (!match) {
                    continue;
                }
            }
            break;
        }
        case VTSS_ACE_TYPE_LLC:
        case VTSS_ACE_TYPE_SNAP:
        default:
            /* These types are currently not used */
            continue;
        }

        /* ACE match ! */
        T_N("ACE ID %d match", conf->id);
        found_matched = TRUE;
        action = &conf->action;
        break;
    }

    /* Use port or ACE action */
    if (ace == NULL) {
        T_N("port %u match", port_no);
        action = &port_conf->action;
    }

    /* Logging */
    if (action->logging) {
        char *msg, buf0[40], buf1[40];

        msg = acl.log_buf;
        msg += sprintf(msg,
                       "Frame of %d bytes received on port %u\n"
                       "MAC:\n"
                       "  Destination: %s\n"
                       "  Source     : %s\n"
                       "  Type/Length: 0x%04x\n"
                       "  VLAN ID    : %d",
                       len, iport2uport(port_no),
                       misc_mac_txt(dmac, buf0), misc_mac_txt(smac, buf1), etype, vid);

        switch (type) {
        case VTSS_ACE_TYPE_IPV4:
        case VTSS_ACE_TYPE_IPV6:
            if (type == VTSS_ACE_TYPE_IPV6) {
                msg += sprintf(msg,
                               "\nIPv6:\n"
                               "  Protocol   : %d\n"
                               "  Source     : %s\n"
                               "  Destination: %s",
                               proto, misc_ipv6_txt(&sip_v6, buf0), misc_ipv6_txt(&dip_v6, buf1));
            } else {
                msg += sprintf(msg,
                               "\nIPv4:\n"
                               "  Protocol   : %d\n"
                               "  Source     : %s\n"
                               "  Destination: %s",
                               proto, misc_ipv4_txt(sip, buf0), misc_ipv4_txt(dip, buf1));
            }
            if (VTSS_BF_GET(flags, ACE_FLAG_IP_FRAGMENT) == 0) {
                switch (proto) {
                case 1:
                    msg += sprintf(msg,
                                   "\nICMP:\n"
                                   "  Type: %d\n"
                                   "  Code: %d",
                                   p[0], p[1]);
                    break;
                case 6:
                    msg += sprintf(msg,
                                   "\nTCP:\n"
                                   "  Source     : %d\n"
                                   "  Destination: %d\n"
                                   "  Flags      : %s%s%s%s%s%s",
                                   sport, dport,
                                   VTSS_BF_GET(flags, ACE_FLAG_TCP_FIN) ? "FIN " : "",
                                   VTSS_BF_GET(flags, ACE_FLAG_TCP_SYN) ? "SYN " : "",
                                   VTSS_BF_GET(flags, ACE_FLAG_TCP_RST) ? "RST " : "",
                                   VTSS_BF_GET(flags, ACE_FLAG_TCP_PSH) ? "PSH " : "",
                                   VTSS_BF_GET(flags, ACE_FLAG_TCP_ACK) ? "ACK " : "",
                                   VTSS_BF_GET(flags, ACE_FLAG_TCP_URG) ? "URG " : "");
                    break;
                case 17:
                    msg += sprintf(msg,
                                   "\nUDP\n"
                                   "  Source     : %d\n"
                                   "  Destination: %d ",
                                   sport, dport);
                    break;
                case 58:
                    msg += sprintf(msg,
                                   "\nICMPv6:\n"
                                   "  Type: %d\n"
                                   "  Code: %d",
                                   p[0], p[1]);
                    break;
                default:
                    break;
                }
            }
            break;
        case VTSS_ACE_TYPE_ARP:
            if (VTSS_BF_GET(flags, ACE_FLAG_ARP_UNKNOWN) == 0) {
                msg += sprintf(msg,
                               "\n%sARP\n"
                               "  Opcode: Re%s\n"
                               "  Sender: %s\n"
                               "  Target: %s",
                               VTSS_BF_GET(flags, ACE_FLAG_ARP_ARP) ? "" : "R",
                               VTSS_BF_GET(flags, ACE_FLAG_ARP_REQ) ? "quest" : "ply",
                               misc_ipv4_txt(sip, buf0), misc_ipv4_txt(dip, buf1));
            }
            break;
        default:
            break;
        }

        msg += sprintf(msg, "\n\nFrame Dump:\n");
        for (i = 0; i < len && i < 1600; i++) {
            if ((n = (i % 16)) == 0) {
                msg += sprintf(msg, "%04X: ", i);
            }
            msg += sprintf(msg, "%02X%c", frame[i], n == 15 ? '\n' : n == 7 ? '-' : ' ');
        }
        msg[-1] = '\0';

#ifdef VTSS_SW_OPTION_SYSLOG
        S_I(acl.log_buf);
#endif
        T_D_HEX(frame, len);
    }

    /* Port shut down */
    timer = &acl.port_shutdown_timer[port_no - VTSS_PORT_NO_START];
    if (action->shutdown && VTSS_MTIMER_TIMEOUT(timer)) {
#ifdef VTSS_SW_OPTION_SYSLOG
        char        syslog_txt[128], *syslog_txt_p = &syslog_txt[0];
        syslog_txt_p += sprintf(syslog_txt_p, "Port %u", iport2uport(port_no));
        syslog_txt_p += sprintf(syslog_txt_p, " shut down");
        S_I(syslog_txt);
#endif /* VTSS_SW_OPTION_SYSLOG */

        VTSS_MTIMER_START(timer, 1000);
        acl_stack_port_shutdown(port_no);
    }

#if defined(VTSS_FEATURE_ACL_V2)
    is_permit = action->port_action == VTSS_ACL_PORT_ACTION_NONE ? 0 : 1;
#else
    is_permit = action->permit ? 0 : 1;
#endif /* VTSS_FEATURE_ACL_V2 */

    ACL_CRIT_EXIT();

    // Pass to other subscribers to receive the packet if the matched ACE is created by other ACL user
    if (found_matched && ace && ACL_USER_ID_GET(ace->conf.id) != ACL_USER_STATIC) {
        return FALSE;
    }

    return is_permit;
}
#endif /* VTSS_SW_OPTION_PACKET */

static void acl_packet_register(void)
{
#ifdef VTSS_SW_OPTION_PACKET
    packet_rx_filter_t filter;
    vtss_port_no_t     port_no;
    BOOL               reg, port_stack;
    acl_action_t       *action;
    acl_ace_t          *ace;
    u32                port_count = port_isid_port_count(VTSS_ISID_LOCAL);

    /* Remove previous registration */
    ACL_CRIT_ENTER();
    if (acl.filter_id != NULL) {
        if (packet_rx_filter_unregister(acl.filter_id) == VTSS_OK) {
            acl.filter_id = NULL;
        }
    }

    /* Determine if registration is neccessary */
    reg = 0;
    for (port_no = VTSS_PORT_NO_START; port_no < port_count; port_no++) {
        action = &acl.port_conf[VTSS_ISID_LOCAL][port_no - VTSS_PORT_NO_START].action;
        if (action->logging || action->shutdown) {
            reg = 1;
        }
    }
    for (ace = acl.switch_acl.used; ace != NULL && ace->conf.conflict == FALSE; ace = ace->next) {
        action = &ace->conf.action;
        if (action->logging || action->shutdown) {
            reg = 1;
        }
    }

    if (reg) {
        memset(&filter, 0, sizeof(filter));
        filter.prio  = PACKET_RX_FILTER_PRIO_HIGH;
        filter.modid = VTSS_MODULE_ID_ACL;
        filter.match = (PACKET_RX_FILTER_MATCH_ANY | PACKET_RX_FILTER_MATCH_SRC_PORT);
        for (port_no = VTSS_PORT_NO_START; port_no < port_count; port_no++) {
            port_stack = PORT_NO_IS_STACK(port_no);
            VTSS_PORT_BF_SET(filter.src_port_mask, port_no, !port_stack);
        }
        filter.cb = acl_packet_rx;
        if (packet_rx_filter_register(&filter, &acl.filter_id) != VTSS_OK) {
            T_W("ACL module register packet RX filter fail./n");
        }
    }

    ACL_CRIT_EXIT();
#endif
}

/****************************************************************************/
/*  Management functions                                                    */
/****************************************************************************/

/* ACL error text */
char *acl_error_txt(vtss_rc rc)
{
    char *txt;

    switch (rc) {
    case ACL_ERROR_GEN:
        txt = "ACL generic error";
        break;
    case ACL_ERROR_ISID_NON_EXISTING:
        txt = "Switch ID is non-existing";
        break;
    case ACL_ERROR_PARM:
        txt = "ACL parameter error";
        break;
    case ACL_ERROR_STACK_STATE:
        txt = "ACL stack state error";
        break;
    case ACL_ERROR_ACE_NOT_FOUND:
        txt = "ACE not found";
        break;
    case ACL_ERROR_ACE_TABLE_FULL:
        txt = "ACE table full";
        break;
    case ACL_ERROR_USER_NOT_FOUND:
        txt = "ACL USER not found";
        break;
    case ACL_ERROR_MEM_ALLOC_FAIL:
        txt = "Alloc memory fail";
        break;
    case ACL_ERROR_ACE_AUTO_ASSIGNED_FAIL:
        txt = "ACE auto-assinged fail";
        break;
    case ACL_ERROR_UNKNOWN_ACE_TYPE:
        txt = "Unknown ACE type";
        break;
    default:
        txt = "ACL unknown error";
        break;
    }
    return txt;
}

/* Determine if port and ISID are valid */
static BOOL acl_mgmt_port_sid_invalid(vtss_isid_t isid, vtss_port_no_t port_no, BOOL set)
{
    if (isid != VTSS_ISID_LOCAL && !msg_switch_is_master()) {
        T_W("not master");
        return 1;
    }

    if (port_no >= port_isid_port_count(isid)) {
        T_W("illegal port_no: %u", port_no);
        return 1;
    }

    /* Check ISID */
    if (isid >= VTSS_ISID_END) {
        T_W("illegal isid: %d", isid);
        return 1;
    }

    if (set && isid == VTSS_ISID_LOCAL) {
        T_W("SET not allowed, isid: %d", isid);
        return 1;
    }

    return 0;
}

/* Get ACL port configuration */
vtss_rc acl_mgmt_port_conf_get(vtss_isid_t isid,
                               vtss_port_no_t port_no, acl_port_conf_t *conf)
{
    T_D("enter, isid: %d, port_no: %u", isid, port_no);

    if (acl_mgmt_port_sid_invalid(isid, port_no, 0)) {
        return ACL_ERROR_PARM;
    }

    ACL_CRIT_ENTER();
    *conf = acl.port_conf[isid][port_no - VTSS_PORT_NO_START];
    ACL_CRIT_EXIT();
    T_D("exit");

    return VTSS_OK;
}

/* Set ACL port configuration */
vtss_rc acl_mgmt_port_conf_set(vtss_isid_t isid,
                               vtss_port_no_t port_no, acl_port_conf_t *conf)
{
    vtss_rc         rc = VTSS_OK;
    int             i, changed = 0;
    acl_port_conf_t *port_conf;

#if defined(VTSS_FEATURE_ACL_V2)
#if defined(VTSS_ARCH_CARACAL) && defined(VTSS_SW_OPTION_EVC)
    T_D("enter, isid: %d, port_no: %u, policy: %u, policer: %u, evc_police: %s, evc_policer: %u",
        isid,
        port_no,
        conf->policy_no,
        conf->action.policer,
        conf->action.evc_police ? "Enabled" : "Disabled",
        conf->action.evc_policer_id);
#else
    T_D("enter, isid: %d, port_no: %u, policy: %u, policer: %u",
        isid,
        port_no,
        conf->policy_no,
        conf->action.policer);
#endif /* VTSS_ARCH_CARACAL && VTSS_SW_OPTION_EVC */
#else
#if defined(VTSS_ARCH_CARACAL) && defined(VTSS_SW_OPTION_EVC)
    T_D("enter, isid: %d, port_no: %lu, policy: %lu, port: %lu, %s, policer: %lu, evc_police: %s, evc_policer: %u",
        isid,
        port_no,
        conf->policy_no,
        conf->action.port_no,
        conf->action.permit ? "Permit" : "Deny",
        conf->action.policer,
        conf->action.evc_police ? "Enabled" : "Disabled",
        conf->action.evc_policer_id);
#else
    T_D("enter, isid: %d, port_no: %u, policy: %u, port: %u, %s, policer: %u",
        isid,
        port_no,
        conf->policy_no,
        conf->action.port_no,
        conf->action.permit ? "Permit" : "Deny",
        conf->action.policer);
#endif /* VTSS_ARCH_CARACAL && VTSS_SW_OPTION_EVC */
#endif /* VTSS_FEATURE_ACL_V2 */

    if (acl_mgmt_port_sid_invalid(isid, port_no, 1)) {
        return ACL_ERROR_PARM;
    }

    i = (port_no - VTSS_PORT_NO_START);
    ACL_CRIT_ENTER();
    if (msg_switch_is_master()) {
        if (msg_switch_configurable(isid)) {
            port_conf = &acl.port_conf[isid][i];
            changed = acl_mgmt_port_conf_changed(port_conf, conf);
            *port_conf = *conf;
        } else {
            T_W("isid %d not active", isid);
            rc = ACL_ERROR_STACK_STATE;
        }
    } else {
        T_W("not master");
        rc = ACL_ERROR_STACK_STATE;
    }
    ACL_CRIT_EXIT();

    if (changed) {
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
        /* Save changed configuration */
        conf_blk_id_t   blk_id = CONF_BLK_ACL_PORT_TABLE;
        acl_port_blk_t  *port_blk;
        if ((port_blk = conf_sec_open(CONF_SEC_GLOBAL, blk_id, NULL)) == NULL) {
            T_W("failed to open ACL port table");
        } else {
            port_blk->conf[(isid - VTSS_ISID_START)*VTSS_PORTS + i] = *conf;
            conf_sec_close(CONF_SEC_GLOBAL, blk_id);
        }
#endif
        /* Activate changed configuration */
        rc = acl_stack_port_conf_set(isid);
    }
    T_D("exit, isid: %d, port_no: %u", isid, port_no);
    return rc;
}

/* Get ACL port counter */
vtss_rc acl_mgmt_port_counter_get(vtss_isid_t isid,
                                  vtss_port_no_t port_no, vtss_acl_port_counter_t *counter)
{
    vtss_rc rc = VTSS_OK;

    T_D("enter, isid: %d, port_no: %u", isid, port_no);

    if (acl_mgmt_port_sid_invalid(isid, port_no, 0)) {
        return ACL_ERROR_PARM;
    }

    if (isid == VTSS_ISID_LOCAL) {
        /* Return local counters on slave switch */
        rc = vtss_acl_port_counter_get(NULL, port_no, counter);
    } else if (msg_switch_exists(isid)) {
        rc = acl_stack_port_counter_get(isid, port_no, counter);
    } else {
        *counter = 0;
    }

    return rc;
}

/* Get ACL policer configuration */
vtss_rc acl_mgmt_policer_conf_get(vtss_acl_policer_no_t policer_no,
                                  acl_policer_conf_t *conf)
{
    T_D("enter, policer_no: %u", policer_no);

    if (!msg_switch_is_master()) {
        T_W("not master");
        return ACL_ERROR_STACK_STATE;
    }

    ACL_CRIT_ENTER();
    *conf = acl.policer_conf[policer_no - VTSS_ACL_POLICER_NO_START];
    ACL_CRIT_EXIT();
    T_D("exit");

    return VTSS_OK;
}

/* Set ACL policer configuration */
vtss_rc acl_mgmt_policer_conf_set(vtss_acl_policer_no_t policer_no,
                                  acl_policer_conf_t *conf)
{
    vtss_rc           rc = VTSS_OK;
    int               i = 0, changed = 0;

#if defined(VTSS_FEATURE_ACL_V2)
    T_D("enter, policer_no: %u, rate %d %s", policer_no, conf->bit_rate_enable ? conf->bit_rate : conf->packet_rate, conf->bit_rate_enable ? "kbps" : "pps");
#else
    T_D("enter, policer_no: %u, rate %d PPS", policer_no, conf->packet_rate);
#endif /* VTSS_FEATURE_ACL_V2 */

    ACL_CRIT_ENTER();
    if (msg_switch_is_master()) {
        i = (policer_no - VTSS_ACL_POLICER_NO_START);
        changed = acl_mgmt_policer_conf_changed(&acl.policer_conf[i], conf);
        acl.policer_conf[i] = *conf;
    } else {
        T_W("not master");
        rc = ACL_ERROR_STACK_STATE;
    }
    ACL_CRIT_EXIT();

    if (changed) {
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
        /* Save changed configuration */
        conf_blk_id_t     blk_id = CONF_BLK_ACL_POLICER_TABLE;
        acl_policer_blk_t *policer_blk;
        if ((policer_blk = conf_sec_open(CONF_SEC_GLOBAL, blk_id, NULL)) == NULL) {
            T_W("failed to open ACL policer table");
        } else {
            policer_blk->conf[i] = *conf;
            conf_sec_close(CONF_SEC_GLOBAL, blk_id);
        }
#endif
        /* Activate changed configuration */
        rc = acl_stack_policer_conf_set(VTSS_ISID_GLOBAL);
    }
    T_D("exit, policer_no: %u", policer_no);
    return rc;
}

static BOOL acl_mgmt_isid_invalid(vtss_isid_t isid)
{
    if (isid > VTSS_ISID_END) {
        T_W("illegal isid: %d", isid);
        return 1;
    }

    if (isid != VTSS_ISID_LOCAL && !msg_switch_is_master()) {
        T_W("not master");
        return 1;
    }

    if (VTSS_ISID_LEGAL(isid) && !msg_switch_configurable(isid)) {
        T_W("isid %d not active", isid);
        return 1;
    }

    return 0;
}

/*lint -sem(acl_mgmt_ace_get, thread_protected)*/
/* Its safe to access global var 'ace_conf_get_flags' */
/* Get ACE or next ACE (use ACE_ID_NONE to get first) */
vtss_rc acl_mgmt_ace_get(acl_user_t user_id, vtss_isid_t isid,
                         vtss_ace_id_t id, acl_entry_conf_t *conf,
                         vtss_ace_counter_t *counter, BOOL next)
{
    vtss_rc rc = VTSS_OK;

    T_D("enter, user_id: %d, isid: %d, id: %d, next: %d", user_id, isid, id, next);

    /* Check stack role */
    if (acl_mgmt_isid_invalid(isid)) {
        T_D("exit");
        return ACL_ERROR_STACK_STATE;
    }

    /* Check user ID */
    if (user_id >= ACL_USER_CNT) {
        T_W("user_id: %d not exist", user_id);
        T_D("exit");
        return ACL_ERROR_USER_NOT_FOUND;
    }

    /* Check ACE ID */
    if (id > ACE_ID_END) {
        T_W("id: %d out of range", id);
        T_D("exit");
        return ACL_ERROR_PARM;
    }

    if (msg_switch_is_local(isid)) {
        isid = VTSS_ISID_LOCAL;
    }

    if (isid == VTSS_ISID_LOCAL || isid == VTSS_ISID_GLOBAL) {
        rc = acl_list_ace_get(isid, ACL_COMBINED_ID_SET(user_id, id), conf, counter, next);
    } else {
        acl_msg_req_t       *msg;
        cyg_flag_value_t    flag;
        cyg_tick_count_t    time_tick;

        msg = acl_msg_req_alloc(ACL_MSG_ID_ACE_CONF_GET_REQ, 1);
        msg->req.ace_conf_get.id = ACL_COMBINED_ID_SET(user_id, id);
        if (counter) {
            msg->req.ace_conf_get.counter = TRUE;
        } else {
            msg->req.ace_conf_get.counter = FALSE;
        }
        msg->req.ace_conf_get.next = next;
        flag = (1 << isid);
        cyg_flag_maskbits(&acl.ace_conf_get_flags, ~flag);
        acl_msg_tx(msg, isid, sizeof(msg->req.ace_conf_get));
        time_tick = cyg_current_time() + VTSS_OS_MSEC2TICK(ACL_REQ_TIMEOUT * 1000);
        if (cyg_flag_timed_wait(&acl.ace_conf_get_flags, flag, CYG_FLAG_WAITMODE_OR, time_tick) & flag ? 0 : 1) {
            T_W("timeout, ACL_MSG_ID_VOLATILE_ACE_CONF_ADD_REQ");
            return ACL_ERROR_REQ_TIMEOUT;
        }

        ACL_CRIT_ENTER();
        if (acl.ace_conf_get_rep_info[isid].id == ACE_ID_NONE) {
            ACL_CRIT_EXIT();
            return ACL_ERROR_ACE_NOT_FOUND;
        } else {
            *conf = acl.ace_conf_get_rep_info[isid];
            conf->id = ACL_ACE_ID_GET(conf->id);
            if (counter) {
                *counter = acl.ace_conf_get_rep_counter[isid];
            }
        }
        ACL_CRIT_EXIT();
    }

    /* Restore independent ACE ID */
    conf->id = ACL_ACE_ID_GET(conf->id);

    return rc;
}

/* Store ACL configuration */
static vtss_rc acl_conf_commit(void)
{
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    conf_blk_id_t blk_id;
    acl_ace_blk_t *ace_blk;
    acl_ace_t     *ace;
    ulong         i;

    T_D("enter");
    blk_id = CONF_BLK_ACL_ACE_TABLE;
    if ((ace_blk = conf_sec_open(CONF_SEC_GLOBAL, blk_id, NULL)) == NULL) {
        T_W("failed to open ACL ACE table");
        T_D("exit");
        return ACL_ERROR_GEN;
    }

    ACL_CRIT_ENTER();
    for (ace = acl.stack_acl.used, i = 0; ace != NULL; ace = ace->next) {
        if (ACL_USER_ID_GET(ace->conf.id) == ACL_USER_STATIC) {
            ace_blk->table[i] = ace->conf;
            i++;
        }
    }
    ace_blk->count = i;
    ACL_CRIT_EXIT();

    conf_sec_close(CONF_SEC_GLOBAL, blk_id);
    T_D("exit");
#endif
    return VTSS_OK;
}

/* Add ACE entry before given ACE or last (ACE_ID_NONE) */
vtss_rc acl_mgmt_ace_add(acl_user_t user_id, vtss_ace_id_t next_id, acl_entry_conf_t *conf)
{
    vtss_rc     rc = VTSS_OK;
    vtss_isid_t isid_del;

    T_D("enter, user_id: %d, next_id: %d, conf->id: %d", user_id, next_id, conf->id);

    /* Check stack role */
    if (acl_mgmt_isid_invalid(conf->isid)) {
        T_D("exit");
        return ACL_ERROR_STACK_STATE;
    }

    /* Check user ID */
    if (user_id >= ACL_USER_CNT) {
        T_W("user_id: %d not exist", user_id);
        T_D("exit");
        return ACL_ERROR_USER_NOT_FOUND;
    }

    /* Check illegal parameter */
    if (conf->id > ACE_ID_END ||
        (acl_user_reg_modes[user_id] == ACL_USER_REG_MODE_LOCAL && conf->isid != VTSS_ISID_LOCAL)) {
        T_D("exit");
        return ACL_ERROR_PARM;
    }

#if defined(VTSS_FEATURE_ACL_V2)
    if (conf->action.port_action > VTSS_ACL_PORT_ACTION_REDIR) {
        T_D("exit");
        return ACL_ERROR_PARM;
    }
#endif /* VTSS_FEATURE_ACL_V2 */
    /* Force setting */
    if (user_id == ACL_USER_STATIC) {
        conf->action.force_cpu = 0;
        conf->action.cpu_once = 0;
#if defined(VTSS_ARCH_JAGUAR_1)
        conf->action.irq_trigger = 0;
#endif
    }

    /* Convert to combined ID */
    conf->id = ACL_COMBINED_ID_SET(user_id, conf->id);
    next_id = ACL_COMBINED_ID_SET(user_id, next_id);

    if (conf->isid == VTSS_ISID_LOCAL) {
        if ((rc = acl_list_ace_add(1, &next_id, conf, &isid_del)) == VTSS_OK && conf->new_allocated == FALSE) {
            /* Set this entry to ASIC layer */
            if (acl_ace_add(next_id, conf) == VTSS_OK) {
                acl_packet_register();
            } else {
                conf->conflict = TRUE;
                acl_ace_conflict_set(conf->id, conf->conflict);
            }
        }

        /* Restore independent ACE ID */
        conf->id = ACL_ACE_ID_GET(conf->id);

        T_D("exit");
        return rc;
    } else {
        rc = acl_list_ace_add(0, &next_id, conf, &isid_del);
    }

    if (rc == VTSS_OK && user_id == ACL_USER_STATIC) {
        rc = acl_conf_commit();
    }

    if (rc == VTSS_OK && isid_del != VTSS_ISID_LOCAL) {
        rc = acl_stack_ace_conf_del(isid_del, conf->id);
    }

    if (rc == VTSS_OK) {
        rc = acl_stack_ace_conf_add(conf->isid, conf->id);
    }

    /* Restore ACE ID */
    conf->id = ACL_ACE_ID_GET(conf->id);

    return rc;
}

/* Solve conflict volatile ACE */
static void acl_mgmt_conflict_solve(void)
{
    acl_list_t          *list;
    acl_ace_t           *ace, *prev, *new;
    BOOL                found = FALSE;
    vtss_ace_id_t       next_id;

    T_D("enter");

    /* Lookup the first conflict entry, try to set it to ASIC layer again.
       If the setting is fail, try the next conflict entry. */
    ACL_CRIT_ENTER();
    list = &acl.switch_acl;
    for (ace = list->used, prev = NULL, new = NULL; ace != NULL; prev = ace, ace = ace->next) {
        if (ace->conf.conflict == TRUE) {
            /* Find the active next_id */
            next_id = ACE_ID_NONE;
            while (ace->next && ace->next->conf.conflict == FALSE) {
                next_id = ace->next->conf.id;
                break;
            }

            /* Try to set this entry to ASIC layer again */
            if (acl_ace_add(next_id, &ace->conf) == VTSS_OK) {
                ace->conf.conflict = FALSE;
                if (ace->conf.new_allocated == TRUE && list->free) {
                    /* Free the new alloc memory and change to use the list */
                    new = list->free;
                    list->free = new->next;
                    new->next = ace->next;
                    new->conf = ace->conf;
                    new->conf.new_allocated = FALSE;
                    new->conf.conflict = FALSE;
                    if (prev) {
                        prev->next = new;
                    } else {
                        list->used = new;
                    }
                    VTSS_FREE(ace);
                }
                found = TRUE;
                break;
            }

            /* The setting is fail, try the next conflict entry */
        }
    }

    ACL_CRIT_EXIT();

    if (found) {
        acl_packet_register();
    }

    T_D("exit");
}

/* Delete ACE */
vtss_rc acl_mgmt_ace_del(acl_user_t user_id, vtss_ace_id_t id)
{
    vtss_rc     rc;
    vtss_isid_t isid_del;
    BOOL        conflict = 1;

    T_D("enter, user_id: %d, id: %d", user_id, id);

    /* Check user ID */
    if (user_id >= ACL_USER_CNT) {
        T_W("user_id: %d not exist", user_id);
        T_D("exit");
        return ACL_ERROR_USER_NOT_FOUND;
    }

    /* Check ACE ID */
    if (id > ACE_ID_END) {
        T_W("id: %d out of range", id);
        T_D("exit");
        return ACL_ERROR_PARM;
    }

    /* Convert to combined ID */
    id = ACL_COMBINED_ID_SET(user_id, id);

    /* Delete this entry if it is existing in global list */
    if (acl_user_reg_modes[user_id] == ACL_USER_REG_MODE_GLOBAL) {
        rc = acl_list_ace_del(0, id, &isid_del, &conflict);

        if (rc == VTSS_OK && user_id == ACL_USER_STATIC) {
            rc = acl_conf_commit();
        }

        if (rc == VTSS_OK) {
            rc = acl_stack_ace_conf_del(isid_del, id);
        }
    } else {
        /* Not found in global list, lookup local list */
        if ((rc = acl_list_ace_del(1, id, &isid_del, &conflict)) == VTSS_OK && conflict == FALSE) {
            /* Remove this entry from ASIC layer */
            if (acl_ace_del(id) == VTSS_OK) {
                acl_packet_register();
            } else {
                T_W("Calling acl_ace_del() failed\n");
            }
            acl_mgmt_conflict_solve();
        }
    }

    T_D("exit");
    return rc;
}

vtss_rc acl_mgmt_counters_clear(void)
{
    T_D("enter");

    if (!msg_switch_is_master()) {
        T_W("not master");
        return ACL_ERROR_STACK_STATE;
    }

    return acl_stack_clear();
}

/****************************************************************************/
/*  Configuration silent upgrade                                            */
/****************************************************************************/

/** \brief ACL entry configuration */
typedef struct {
    vtss_ace_id_t           id;                                 /**< ACE ID */
#if defined(VTSS_ARCH_SERVAL)
    u8                      lookup;                             /**< Lookup, any non-zero value means second lookup */
    BOOL                    isdx_enable;                        /**< Use VID field for ISDX value */
#endif /* VTSS_ARCH_SERVAL */
#if defined(VTSS_FEATURE_ACL_V2)
    BOOL                    port_list[VTSS_PORT_ARRAY_SIZE];    /**< Port list: VTSS_ACL_RULE_PORT */
#else
    vtss_port_no_t          port_no;                            /**< Port number: VTSS_ACL_RULE_PORT */
#endif /* VTSS_FEATURE_ACL_V2 */
    vtss_ace_u8_t           policy;                             /**< Policy number */
    vtss_ace_type_t         type;                               /**< ACE frame type */
    acl_action_t            action;                             /**< ACE action */
    vtss_isid_t             isid;                               /**< Switch ID, VTSS_ISID_GLOBAL means any */
    BOOL                    conflict;                           /**< Volatile ACE conflict flag */
    BOOL                    new_allocated;                      /**< This ACE entry is new allocated. (Only for internal used) */

    struct {
        uchar value[ACE_FLAG_SIZE];     /* ACE flag value */
        uchar mask[ACE_FLAG_SIZE];      /**< ACE flag mask */
    } flags;

    vtss_ace_vid_t  vid;                /**< VLAN ID (12 bit) */
    vtss_ace_u8_t   usr_prio;           /**< User priority (3 bit) */
#if defined(VTSS_FEATURE_ACL_V2)
    vtss_ace_bit_t  tagged;             /**< Tagged/untagged frame */
#endif /* VTSS_FEATURE_ACL_V2 */

    /* Frame type specific data */
    union {
        /* VTSS_ACE_TYPE_ANY: No specific fields */

        /**< VTSS_ACE_TYPE_ETYPE */
        struct {
            vtss_ace_u48_t  dmac;   /**< DMAC */
            vtss_ace_u48_t  smac;   /**< SMAC */
            vtss_ace_u16_t  etype;  /**< Ethernet Type value */
            vtss_ace_u16_t  data;   /**< MAC data */
#if defined(VTSS_FEATURE_ACL_V2)
            vtss_ace_ptp_t  ptp;    /**< PTP header filtering (overrides smac byte 0-1 and data fields) */
#endif /* VTSS_FEATURE_ACL_V2 */
        } etype;

        /**< VTSS_ACE_TYPE_LLC */
        struct {
            vtss_ace_u48_t dmac; /**< DMAC */
            vtss_ace_u48_t smac; /**< SMAC */
            vtss_ace_u32_t llc;  /**< LLC */
        } llc;

        /**< VTSS_ACE_TYPE_SNAP */
        struct {
            vtss_ace_u48_t dmac; /**< DMAC */
            vtss_ace_u48_t smac; /**< SMAC */
            vtss_ace_u40_t snap; /**< SNAP */
        } snap;

        /** VTSS_ACE_TYPE_ARP */
        struct {
            vtss_ace_u48_t smac; /**< SMAC */
            vtss_ace_ip_t  sip;  /**< Sender IP address */
            vtss_ace_ip_t  dip;  /**< Target IP address */
        } arp;

        /**< VTSS_ACE_TYPE_IPV4 */
        struct {
            vtss_ace_u8_t       ds;         /* DS field */
            vtss_ace_u8_t       proto;      /**< Protocol */
            vtss_ace_ip_t       sip;        /**< Source IP address */
            vtss_ace_ip_t       dip;        /**< Destination IP address */
            vtss_ace_u48_t      data;       /**< Not UDP/TCP: IP data */
            vtss_ace_udp_tcp_t  sport;      /**< UDP/TCP: Source port */
            vtss_ace_udp_tcp_t  dport;      /**< UDP/TCP: Destination port */
#if defined(VTSS_FEATURE_ACL_V2)
            vtss_ace_sip_smac_t sip_smac;   /**< SIP/SMAC matching (overrides sip field) */
            vtss_ace_ptp_t      ptp;        /**< PTP header filtering (overrides sip field) */
#endif /* VTSS_FEATURE_ACL_V2 */
        } ipv4;

        /**< VTSS_ACE_TYPE_IPV6 */
        struct {
            vtss_ace_u8_t   proto; /**< IPv6 protocol */
            vtss_ace_u128_t sip;   /**< IPv6 source address */
#if defined(VTSS_FEATURE_ACL_V2)
            vtss_ace_bit_t     ttl;       /**< TTL zero */
            vtss_ace_u8_t      ds;        /**< DS field */
            vtss_ace_u48_t     data;      /**< Not UDP/TCP: IP data */
            vtss_ace_udp_tcp_t sport;     /**< UDP/TCP: Source port */
            vtss_ace_udp_tcp_t dport;     /**< UDP/TCP: Destination port */
            vtss_ace_bit_t     tcp_fin;   /**< TCP FIN */
            vtss_ace_bit_t     tcp_syn;   /**< TCP SYN */
            vtss_ace_bit_t     tcp_rst;   /**< TCP RST */
            vtss_ace_bit_t     tcp_psh;   /**< TCP PSH */
            vtss_ace_bit_t     tcp_ack;   /**< TCP ACK */
            vtss_ace_bit_t     tcp_urg;   /**< TCP URG */
            vtss_ace_ptp_t     ptp;       /**< PTP header filtering (overrides sip byte 0-3) */
#endif /* VTSS_FEATURE_ACL_V2 */
        } ipv6;
    } frame;
} acl_entry_conf_v1_t;

/* ACE configuration block */
typedef struct {
    ulong               version;
    ulong               count;             /* Number of entries */
    ulong               size;              /* Size of each entry */
    acl_entry_conf_v1_t table[ACE_ID_END]; /* Entries */
} acl_ace_blk_v1_t;

/* Silent upgrade from old configuration to new one.
 * Returns a (malloc'ed) pointer to the upgraded new configuration
 * or NULL if conversion failed.
 */
static acl_ace_blk_t *acl_conf_silent_upgrade(const void *blk, u32 old_ver)
{
    acl_ace_blk_t *new_blk = NULL;

    if (old_ver == 1 && (new_blk = VTSS_MALLOC(sizeof(*new_blk)))) {
        acl_ace_blk_v1_t    *old_blk = (acl_ace_blk_v1_t *) blk;
        u32                 idx;

        /* upgrade configuration from v1 to v2 */
        memset(new_blk, 0, sizeof(*new_blk));
        new_blk->version = 2;
        new_blk->count = old_blk->count;
        new_blk->size = sizeof(acl_entry_conf_t);
        for (idx = 0; idx < new_blk->count; ++idx) {
            memcpy(&new_blk->table[idx], &old_blk->table[idx], sizeof(acl_entry_conf_v1_t));
#if defined(VTSS_FEATURE_ACL_V1)
            if (old_blk->table[idx].type == VTSS_ACE_TYPE_IPV6) {
                acl_entry_conf_t ipv6_ace;
                (void) acl_mgmt_ace_init(VTSS_ACE_TYPE_IPV6, &ipv6_ace);
                new_blk->table[idx].frame.ipv6 = ipv6_ace.frame.ipv6;
                new_blk->table[idx].frame.ipv6.proto = old_blk->table[idx].frame.ipv6.proto;
                memcpy(&new_blk->table[idx].frame.ipv6.sip, &old_blk->table[idx].frame.ipv6.sip, sizeof(vtss_ace_u128_t));
            }
#endif /* VTSS_FEATURE_ACL_V1 */
        }
    }

    return new_blk;
}


/****************************************************************************/
/*  Initialization functions                                                */
/****************************************************************************/

/* Read/create ACL switch configuration */
static vtss_rc acl_conf_read_switch(vtss_isid_t isid_add)
{
    vtss_rc         rc = VTSS_OK;
    conf_blk_id_t   blk_id;
    vtss_port_no_t  port_no;
    acl_port_conf_t *port_conf, new_port_conf;
    acl_port_blk_t  *port_blk = NULL;
    int             i, j, changed;
    BOOL            do_create;
    ulong           size;
    vtss_isid_t     isid;

    T_D("enter, isid_add: %d", isid_add);

    blk_id = CONF_BLK_ACL_PORT_TABLE;

    if (misc_conf_read_use()) {
        /* read ACL port configuration */
        if ((port_blk = conf_sec_open(CONF_SEC_GLOBAL, blk_id, &size)) == NULL ||
            size != sizeof(*port_blk)) {
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
            T_W("conf_sec_open failed or size mismatch, creating defaults");
            port_blk = conf_sec_create(CONF_SEC_GLOBAL, blk_id, sizeof(*port_blk));
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
            do_create = 1;
        } else if (port_blk->version != ACL_PORT_BLK_VERSION) {
            T_W("version mismatch, creating defaults");
            do_create = 1;
        } else {
            do_create = (isid_add != VTSS_ISID_GLOBAL);
        }
    } else {
        do_create = 1;
    }

    for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
        if (isid_add != VTSS_ISID_GLOBAL && isid_add != isid) {
            continue;
        }

        changed = 0;
        ACL_CRIT_ENTER();
        for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
            i = (isid - VTSS_ISID_START);
            j = (port_no - VTSS_PORT_NO_START);
            if (do_create) {
                /* Use default values */
                acl_mgmt_port_conf_get_default(&new_port_conf);
                if (port_blk != NULL) {
                    port_blk->conf[i * VTSS_PORTS + j] = new_port_conf;
                }
            } else {
                /* Use new configuration */
                if (port_blk != NULL) {
                    new_port_conf = port_blk->conf[i * VTSS_PORTS + j];
                }
            }
            port_conf = &acl.port_conf[isid][j];
            if (acl_mgmt_port_conf_changed(port_conf, &new_port_conf)) {
                changed = 1;
            }
            *port_conf = new_port_conf;
        }
        ACL_CRIT_EXIT();
        if (changed && isid_add != VTSS_ISID_GLOBAL && msg_switch_exists(isid)) {
            rc = acl_stack_port_conf_set(isid);
        }
    }

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (port_blk == NULL) {
        T_W("failed to open ACL port table");
    } else {
        port_blk->version = ACL_PORT_BLK_VERSION;
        conf_sec_close(CONF_SEC_GLOBAL, blk_id);
    }
#endif

    T_D("exit");
    return rc;
}

/* Read/create ACL stack configuration */
static vtss_rc acl_conf_read_stack(BOOL create)
{
    vtss_rc               rc = VTSS_OK;
    conf_blk_id_t         blk_id;
    vtss_acl_policer_no_t policer_no;
    acl_policer_conf_t    new_conf;
    acl_policer_blk_t     *policer_blk = NULL;
    acl_ace_blk_t         *ace_blk = NULL, *silent_upgrade_new_ace_blk = NULL;
    acl_list_t            *list;
    acl_ace_t             *ace;
    int                   i, changed;
    BOOL                  do_create;
    ulong                 size, blk_version;

    T_D("enter, create: %d", create);

    blk_id = CONF_BLK_ACL_POLICER_TABLE;

    if (misc_conf_read_use()) {
        /* Read/create policer configuration */
        if ((policer_blk = conf_sec_open(CONF_SEC_GLOBAL, blk_id, &size)) == NULL ||
            size != sizeof(*policer_blk)) {
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
            T_W("conf_sec_open failed or size mismatch, creating defaults");
            policer_blk = conf_sec_create(CONF_SEC_GLOBAL, blk_id, sizeof(*policer_blk));
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
            do_create = 1;
        } else if (policer_blk->version != ACL_POLICER_BLK_VERSION) {
            T_W("version mismatch, creating defaults");
            do_create = 1;
        } else {
            do_create = create;
        }
    } else {
        do_create   = TRUE;
    }

    changed = 0;
    ACL_CRIT_ENTER();
    for (policer_no = VTSS_ACL_POLICER_NO_START;
         policer_no < VTSS_ACL_POLICER_NO_END;
         policer_no++) {
        i = (policer_no - VTSS_ACL_POLICER_NO_START);

        acl_mgmt_policer_conf_get_default(&new_conf);
        if (do_create) {
            /* Use default values */
            if (policer_blk != NULL) {
                policer_blk->conf[i] = new_conf;
            }
        } else {
            /* Use new configuration */
            if (policer_blk != NULL) {
                new_conf = policer_blk->conf[i];
            }
        }
        if (acl_mgmt_policer_conf_changed(&acl.policer_conf[i], &new_conf)) {
            changed = 1;
        }
        acl.policer_conf[i] = new_conf;
    }
    ACL_CRIT_EXIT();

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (policer_blk == NULL) {
        T_W("failed to open ACL policer table");
    } else {
        policer_blk->version = ACL_POLICER_BLK_VERSION;
        conf_sec_close(CONF_SEC_GLOBAL, blk_id);
    }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */

    if (changed && create) {
        if (acl_stack_policer_conf_set(VTSS_ISID_GLOBAL) != VTSS_OK) {
            T_W("Calling acl_stack_policer_conf_set() failed\n");
        }
    }

    blk_id = CONF_BLK_ACL_ACE_TABLE;

    if (misc_conf_read_use()) {
        /* Read/create ACE configuration */
        blk_version = ACL_ACE_BLK_VERSION;
        if ((ace_blk = conf_sec_open(CONF_SEC_GLOBAL, blk_id, &size)) == NULL) {
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
            T_W("conf_sec_open failed or size mismatch, creating defaults");
            ace_blk = conf_sec_create(CONF_SEC_GLOBAL, blk_id, sizeof(*ace_blk));
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
            do_create = 1;
        } else if (ace_blk->version == 1 && blk_version == 2) {
            T_I("version upgrade, run silent upgrade");
            silent_upgrade_new_ace_blk = acl_conf_silent_upgrade(ace_blk, ace_blk->version);
            if (silent_upgrade_new_ace_blk) {
                T_I("upgrade ok");
                ace_blk = silent_upgrade_new_ace_blk;
                do_create = 0;
            } else {
                T_W("upgrade failed, creating defaults");
                do_create = 1;
            }
        } else if (ace_blk->version != ACL_ACE_BLK_VERSION) {
            T_W("version mismatch, creating defaults");
            do_create = 1;
        } else if (size != sizeof(*ace_blk)) {
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
            T_W("size mismatch, creating defaults");
            silent_upgrade_new_ace_blk = acl_conf_silent_upgrade(ace_blk, ace_blk->version);
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
            do_create = 1;
        } else {
            do_create = create;
        }
    } else {
        do_create = TRUE;
    }

    if (do_create && ace_blk != NULL) {
        ace_blk->count = 0;
        ace_blk->size = sizeof(acl_entry_conf_t);
    }

    ACL_CRIT_ENTER();
    list = &acl.stack_acl;
    changed = (list->used != NULL);

    /* Free old ACEs */
    while (list->used != NULL) {
        ace = list->used;
        list->used = ace->next;
        if (ace->conf.new_allocated == TRUE) {
            /* This entry is saving in new memory */
            VTSS_FREE(ace);
        } else {
            /* Move to free list */
            ace->next = list->free;
            list->free = ace;
        }
    };

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (ace_blk == NULL) {
        T_W("failed to open ACL ACE table");
    }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */

    if (ace_blk) {
        /* Add new ACEs */
        for (i = ace_blk->count; i != 0; i--) {
            /* Move entry from free list to used list */
            if ((ace = list->free) == NULL) {
                break;
            }
            list->free = ace->next;
            ace->next = list->used;
            list->used = ace;
            ace->conf = ace_blk->table[i - 1];
        }
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
        ace_blk->version = ACL_ACE_BLK_VERSION;
        conf_sec_close(CONF_SEC_GLOBAL, blk_id);
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
    }
    ACL_CRIT_EXIT();

    if (silent_upgrade_new_ace_blk) {
        VTSS_FREE(silent_upgrade_new_ace_blk);
    }

    if (changed && create) {
        rc = acl_stack_ace_conf_set(VTSS_ISID_GLOBAL);
    }

    T_D("exit");
    return rc;
}

#if defined(ACL_PACKET_TEST)
static BOOL acl_packet_etype_rx(void *contxt, const uchar *const frame, const vtss_packet_rx_info_t *const rx_info)
{
    return 1; /* Consume frame */
}

static void acl_packet_etype_register(void)
{
    packet_rx_filter_t filter;
    void               *filter_id;

    memset(&filter, 0, sizeof(filter));
    filter.prio  = (PACKET_RX_FILTER_PRIO_SUPER - 1);
    filter.modid = VTSS_MODULE_ID_ACL;
    filter.match = PACKET_RX_FILTER_MATCH_ETYPE;
    filter.etype = 0xaaaa;
    filter.cb = acl_packet_etype_rx;
    if (packet_rx_filter_register(&filter, &filter_id) != VTSS_OK) {
        T_W("ACL module register etype packet RX filter fail./n");
    }
}
#endif /* ACL_PACKET_TEST */

/* Initialize ACE to default values (permit on all front ports) */
vtss_rc acl_mgmt_ace_init(vtss_ace_type_t type, acl_entry_conf_t *ace)
{
#if defined(VTSS_FEATURE_ACL_V2)
    vtss_port_no_t iport;
#endif /* VTSS_FEATURE_ACL_V2 */

    T_D("Enter, type: %d", type);

    memset(ace, 0, sizeof(*ace));
    ace->id = ACE_ID_NONE;
    ace->type = type;
    ace->isid = VTSS_ISID_GLOBAL;
    ace->action.policer = ACL_POLICER_NONE;
    ace->action.cpu_queue = PACKET_XTR_QU_ACL_COPY; /* Default ACL queue */

#if defined(VTSS_FEATURE_ACL_V2)
    ace->action.port_action = VTSS_ACL_PORT_ACTION_NONE;
    for (iport = VTSS_PORT_NO_START; iport < VTSS_PORT_NO_END; iport++) {
        ace->port_list[iport] = TRUE;
        ace->action.port_list[iport] = TRUE;
    }
#else
    ace->port_no = VTSS_PORT_NO_ANY;
    ace->action.permit = TRUE;
    ace->action.port_no = VTSS_PORT_NO_NONE;
#endif /* VTSS_FEATURE_ACL_V2 */

    switch (type) {
    case VTSS_ACE_TYPE_ANY:
    case VTSS_ACE_TYPE_ETYPE:
    case VTSS_ACE_TYPE_LLC:
    case VTSS_ACE_TYPE_SNAP:
    case VTSS_ACE_TYPE_ARP:
        break;
    case VTSS_ACE_TYPE_IPV4:
        ace->frame.ipv4.sport.in_range = ace->frame.ipv4.dport.in_range = 1;
        ace->frame.ipv4.sport.high = ace->frame.ipv4.dport.high = 65535;
        break;
    case VTSS_ACE_TYPE_IPV6:
        ace->frame.ipv6.sport.in_range = ace->frame.ipv6.dport.in_range = 1;
        ace->frame.ipv6.sport.high = ace->frame.ipv6.dport.high = 65535;
        break;
    default:
        T_E("unknown type: %d", type);
        T_D("Exit");
        return ACL_ERROR_UNKNOWN_ACE_TYPE;
    }

    T_D("Exit");
    return VTSS_OK;
}

/* Module start */
static void acl_start(BOOL init)
{
    int                   i;
    acl_ace_t             *ace;
    acl_list_t            *list;
    vtss_isid_t           isid;
    vtss_port_no_t        port_no;
    acl_port_conf_t       *port_conf;
    vtss_acl_policer_no_t policer_no;

    T_D("enter, init: %d", init);

    if (init) {
        /* Initialize port configuration */
        for (isid = VTSS_ISID_LOCAL; isid < VTSS_ISID_END; isid++) {
            for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
                port_conf = &acl.port_conf[isid][port_no - VTSS_PORT_NO_START];
                acl_mgmt_port_conf_get_default(port_conf);
            }
        }

        /* Initialize policer configuration */
        for (policer_no = VTSS_ACL_POLICER_NO_START;
             policer_no < VTSS_ACL_POLICER_NO_END;
             policer_no++) {
            acl_mgmt_policer_conf_get_default(&acl.policer_conf[policer_no - VTSS_ACL_POLICER_NO_START]);
        }

        /* Initialize ACL for switch: All free */
        list = &acl.switch_acl;
        list->used = NULL;
        list->free = NULL;
        for (i = 0; i < ACE_MAX; i++) {
            ace = &acl.switch_ace_table[i];
            ace->next = list->free;
            list->free = ace;
        }

        /* Initialize ACL for stack: All free */
        list = &acl.stack_acl;
        list->used = NULL;
        list->free = NULL;
        for (i = 0; i < ACE_ID_END; i++) {
            ace = &acl.stack_ace_table[i];
            ace->next = list->free;
            list->free = ace;
        }

        /* Initialize message buffer pools */
        /* The SWITCH_ADD event sends three messages in a row. Make sure we have
         * enough for them to succeed without having to wait for msg_tx_done() */
        acl.request = msg_buf_pool_create(VTSS_MODULE_ID_ACL, "Request", 3, sizeof(acl_msg_req_t));
        acl.reply   = msg_buf_pool_create(VTSS_MODULE_ID_ACL, "Reply",   1, sizeof(acl_msg_rep_t));

        /* Initialize counter timers */
        for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
            i = (isid - VTSS_ISID_START);
            VTSS_MTIMER_START(&acl.ace_counters_timer[i], 1);
            VTSS_MTIMER_START(&acl.port_counters_timer[i], 1);
        }

        cyg_flag_init(&acl.ace_counters_flags);
        cyg_flag_init(&acl.port_counters_flags);
        cyg_flag_init(&acl.ace_conf_get_flags);

        acl.filter_id = NULL;
        for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
            VTSS_MTIMER_START(&acl.port_shutdown_timer[port_no - VTSS_PORT_NO_START], 0);
        }

        /* Create semaphore for critical regions */
        critd_init(&acl.crit, "acl.crit", VTSS_MODULE_ID_ACL, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
        ACL_CRIT_EXIT();
    } else {
        /* Register for stack messages */
        if (acl_stack_register() != VTSS_OK) {
            T_W("Calling acl_stack_register() failed\n");
        }

#if defined(ACL_PACKET_TEST)
        acl_packet_etype_register();
#endif /* ACL_PACKET_TEST */
    }
    T_D("exit");
}

/* Initialize module */
vtss_rc acl_init(vtss_init_data_t *data)
{
    vtss_isid_t isid = data->isid;

    if (data->cmd == INIT_CMD_INIT) {
        /* Initialize and register trace ressources */
        VTSS_TRACE_REG_INIT(&trace_reg, trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&trace_reg);
    }

    T_D("enter, cmd: %d, isid: %u, flags: 0x%x", data->cmd, data->isid, data->flags);

    switch (data->cmd) {
    case INIT_CMD_INIT:
        T_D("INIT");
        acl_start(1);
#ifdef VTSS_SW_OPTION_VCLI
        acl_cli_req_init();
#endif
#ifdef VTSS_SW_OPTION_ICFG
        if (acl_icfg_init() != VTSS_OK) {
            T_W("Calling acl_icfg_init() failed\n");
        }
#endif
        break;
    case INIT_CMD_START:
        T_D("START");
        acl_start(0);
        break;
    case INIT_CMD_CONF_DEF:
        T_D("CONF_DEF, isid: %d", isid);
        if (isid == VTSS_ISID_LOCAL) {
            /* Reset local configuration */
        } else if (isid == VTSS_ISID_GLOBAL) {
            /* Reset stack configuration */
            if (acl_conf_read_stack(1) != VTSS_OK) {
                T_W("Calling acl_conf_read_stack() failed\n");
            }
        } else if (VTSS_ISID_LEGAL(isid)) {
            /* Reset switch configuration */
            if (acl_conf_read_switch(isid) != VTSS_OK) {
                T_W("Calling acl_conf_read_switch() failed\n");
            }
        }
        break;
    case INIT_CMD_MASTER_UP:
        T_D("MASTER_UP");
        /* Read stack and switch configuration */
        if (acl_conf_read_stack(0) != VTSS_OK) {
            T_W("Calling acl_conf_read_stack() failed\n");
        }
        if (acl_conf_read_switch(VTSS_ISID_GLOBAL) != VTSS_OK) {
            T_W("Calling acl_conf_read_switch() failed\n");
        }
        break;
    case INIT_CMD_MASTER_DOWN:
        T_D("MASTER_DOWN");
        break;
    case INIT_CMD_SWITCH_ADD:
        T_D("SWITCH_ADD, isid: %d", isid);
        /* Apply all configuration to switch */
        if (acl_stack_port_conf_set(isid) != VTSS_OK) {
            T_W("Calling acl_stack_port_conf_set() failed\n");
        }
        if (acl_stack_policer_conf_set(isid) != VTSS_OK) {
            T_W("Calling acl_stack_policer_conf_set() failed\n");
        }
        if (acl_stack_ace_conf_set(isid) != VTSS_OK) {
            T_W("Calling acl_stack_policer_conf_set() failed\n");
        }
        break;
    case INIT_CMD_SWITCH_DEL:
        T_D("SWITCH_DEL, isid: %d", isid);
        break;
    default:
        break;
    }

    T_D("exit");
    return VTSS_OK;
}

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
