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

#include "main.h"
#include "cli.h"
#include "cli_grp_help.h"
#include "cli_api.h"
#include "acl_cli.h"
#include "acl_api.h"
#include "mgmt_api.h"
#include "port_api.h"
#include "topo_api.h"
#include "msg_api.h"    //msg_switch_exists()

#ifdef VTSS_SW_OPTION_IP_SOURCE_GUARD
#include "ip_source_guard_api.h"
#endif

#include "cli_trace_def.h"

static char acl_cli_sid_help_str[30];

/* ACE flag type */
enum {
    CLI_ACE_FLAG_NONE = 0, /* Flag not specified */
    CLI_ACE_FLAG_ANY,      /* Wildcard */
    CLI_ACE_FLAG_0,        /* Zero */
    CLI_ACE_FLAG_1         /* One */
};

typedef int cli_ace_flag_t;

#if defined(VTSS_FEATURE_ACL_V2)
/* ACL rate unit type */
enum {
    CLI_RATE_UNIT_PPS = 0,
    CLI_RATE_UNIT_KBPS
};
#endif /* VTSS_FEATURE_ACL_V2 */

typedef struct {
    vtss_usid_t           usid_ace;
    cli_spec_t            usid_ace_spec;
    vtss_ace_id_t         ace_id;
    vtss_ace_id_t         ace_id_next;
#if defined(VTSS_ARCH_SERVAL)
    cli_spec_t            lookup_spec;
    BOOL                  lookup;
#endif /* VTSS_ARCH_SERVAL */
    cli_spec_t            policy_no_spec;
    vtss_acl_policy_no_t  policy_no;
    vtss_acl_policy_no_t  policy_bitmask;
#if defined(VTSS_FEATURE_ACL_V2)
    cli_spec_t            rate_unit;
    cli_spec_t            tagged_spec;
    uchar                 tagged;
    cli_spec_t            mirror_spec;
    BOOL                  mirror;
    BOOL                  sip_smac;
#endif /* VTSS_FEATURE_ACL_V2 */
    cli_spec_t            rate_spec;
    vtss_packet_rate_t    rate;
    cli_ace_flag_t        ace_flags[ACE_FLAG_COUNT];
    cli_spec_t            ace_type_spec;
    vtss_ace_type_t       ace_type;
    cli_spec_t            tag_prio_spec;
    uchar                 tag_prio;
    uchar                 tag_prio_bitmask;
    cli_spec_t            etype_spec;
    vtss_etype_t          etype;
    cli_spec_t            smac_spec;
    uchar                 smac[6];
    cli_spec_t            dmac_spec;
    uchar                 dmac[6];
    cli_spec_t            sip_spec;
    ulong                 sip;
    ulong                 sip_mask;
    cli_spec_t            dip_spec;
    ulong                 dip;
    ulong                 dip_mask;
    cli_spec_t            ip_proto_spec;
    uchar                 ip_proto;
    cli_spec_t            icmp_code_spec;
    uchar                 icmp_code;
    cli_spec_t            icmp_type_spec;
    uchar                 icmp_type;
    cli_spec_t            sport_spec;
    vtss_udp_tcp_t        sport_min;
    vtss_udp_tcp_t        sport_max;
    cli_spec_t            dport_spec;
    vtss_udp_tcp_t        dport_min;
    vtss_udp_tcp_t        dport_max;
    cli_spec_t            permit_spec;
    uchar                 permit;
#if defined(VTSS_FEATURE_ACL_V2)
    cli_spec_t            port_filter_spec;
    BOOL                  uport_filter_list[VTSS_PORT_ARRAY_SIZE + 1];
#endif /* VTSS_FEATURE_ACL_V2 */
    cli_spec_t            policer_spec;
    vtss_acl_policer_no_t policer;
#if defined(VTSS_ARCH_CARACAL) && defined(VTSS_SW_OPTION_EVC)
    cli_spec_t            evc_policer_spec;
    vtss_evc_policer_id_t evc_policer;
#endif /* VTSS_ARCH_CARACAL && VTSS_SW_OPTION_EVC */
    cli_spec_t            port_copy_spec;
    vtss_uport_no_t       uport_copy;
    BOOL                  uport_copy_list[VTSS_PORT_ARRAY_SIZE + 1]; /* Index 0 may be used for type CLI_PARM_TYPE_PORT_LIST_0. This is the reason why we can't make a corresponding iport_list[] */
    BOOL                  policer_list[VTSS_ACL_POLICER_NO_END + 1];
    cli_spec_t            logging_spec;
    BOOL                  logging;
    cli_spec_t            shutdown_spec;
    BOOL                  shutdown;
    cli_spec_t            sipv6_spec;
    vtss_ipv6_t           sipv6;
#if defined(ACL_IPV6_SUPPORTED)
    cli_spec_t            sip_v6_mask_spec;
    ulong                 sip_v6_mask;
#endif /* ACL_IPV6_SUPPORTED */

    /* Keywords */
    BOOL                  policy;
    BOOL                  ip;
    BOOL                  ipv6;
    int                   acl_user;
    BOOL                  acl_user_valid;
} acl_cli_req_t;

/****************************************************************************/
/*  Initialization                                                          */
/****************************************************************************/

void acl_cli_req_init(void)
{
    (void)snprintf(acl_cli_sid_help_str, sizeof(acl_cli_sid_help_str), "Switch ID (%d-%d) or 'any'", VTSS_USID_START, VTSS_USID_END - 1);

    /* register the size required for port req. structure */
    cli_req_size_register(sizeof(acl_cli_req_t));
}

/****************************************************************************/
/*  Command functions                                                       */
/****************************************************************************/

static void cli_ace_mac_set(vtss_ace_u48_t *ace_mac, uchar *mac, cli_spec_t spec, BOOL add)
{
    int i;

    if (add || spec != CLI_SPEC_NONE) {
        for (i = 0; i < 6; i++) {
            ace_mac->value[i] = mac[i];
            ace_mac->mask[i] = (spec == CLI_SPEC_VAL ? 0xff : 0);
        }
    }
}

static void cli_ace_ip_set(vtss_ace_ip_t *ace_ip, ulong ip, ulong mask,
                           cli_spec_t spec, BOOL add)
{
    if (add || spec != CLI_SPEC_NONE) {
        ace_ip->value = ip;
        ace_ip->mask = (spec == CLI_SPEC_VAL ? mask : 0);
    }
}

#if defined(VTSS_FEATURE_ACL_V2)
static void cli_ace_sip_smc_set(vtss_ace_sip_smac_t *ace_sip_smac, ulong ip, cli_spec_t ip_spec,
                                uchar *mac, cli_spec_t mac_spec, BOOL add)
{
    if (add || ip_spec != CLI_SPEC_NONE || mac_spec != CLI_SPEC_NONE) {
        ace_sip_smac->enable = 1;
        ace_sip_smac->sip = ip;
        memcpy(ace_sip_smac->smac.addr, mac, 6);
    }
}
#endif /* VTSS_FEATURE_ACL_V2 */

static void cli_ace_port_set(vtss_ace_udp_tcp_t *ace_port,
                             vtss_udp_tcp_t min, vtss_udp_tcp_t max,
                             cli_spec_t spec, BOOL add)
{
    if (add || spec != CLI_SPEC_NONE) {
        ace_port->in_range = 1;
        ace_port->low = (spec == CLI_SPEC_VAL ? min : 0);
        ace_port->high = (spec == CLI_SPEC_VAL ? max : 0xffff);
    }
}

#ifndef VTSS_FEATURE_ACL_V2
static char *cli_acl_port_no_txt(vtss_port_no_t iport, char *buf)
{
    if (iport == VTSS_PORT_NO_NONE) {
        strcpy(buf, cli_bool_txt(0));
    } else {
        sprintf(buf, "%u", iport2uport(iport));
    }
    return buf;
}
#endif /* VTSS_FEATURE_ACL_V2 */

static char *cli_acl_usid_txt(vtss_isid_t isid, char *buf)
{
    if (isid == VTSS_ISID_GLOBAL) {
#if VTSS_SWITCH_STACKABLE
        strcpy(buf, "Any");
#else
        strcpy(buf, "User");
#endif /* VTSS_SWITCH_STACKABLE */
    } else if (isid == VTSS_ISID_LOCAL) {
        sprintf(buf, "LOCAL");
    } else {
        sprintf(buf, "%-2d", topo_isid2usid(isid));
    }
    return buf;
}

static char *cli_acl_policer_no_txt(vtss_acl_policer_no_t policer_no, char *buf)
{
    if (policer_no == ACL_POLICER_NONE) {
        strcpy(buf, cli_bool_txt(0));
    } else {
        sprintf(buf, "%u", ipolicer2upolicer(policer_no));
    }
    return buf;
}

static char *cli_acl_user_txt(BOOL acronym, acl_user_t user_id, char *buf)
{
    if (acronym) {
        switch (user_id) {
        case ACL_USER_STATIC:
            strcpy(buf, "S   ");
            break;
#if defined(VTSS_ARCH_JAGUAR_1_CE_MAC) || defined(IP_MGMT_USING_MAC_ACL_ENTRIES)
        case ACL_USER_IP_MGMT:
            strcpy(buf, "IPMG");
            break;
#endif
#ifdef VTSS_SW_OPTION_IP_SOURCE_GUARD
        case ACL_USER_IP_SOURCE_GUARD:
            strcpy(buf, "IPSG");
            break;
#endif
#if defined(VTSS_SW_OPTION_IPMC) || defined(VTSS_SW_OPTION_IGMPS) || defined(VTSS_SW_OPTION_MLDSNP)
        case ACL_USER_IPMC:
            strcpy(buf, "IPMC");
            break;
#endif
#ifdef VTSS_SW_OPTION_MEP
        case ACL_USER_MEP:
            strcpy(buf, "MEP ");
            break;
#endif
#ifdef VTSS_SW_OPTION_ARP_INSPECTION
        case ACL_USER_ARP_INSPECTION:
            strcpy(buf, "ARPI");
            break;
#endif
#ifdef VTSS_SW_OPTION_UPNP
        case ACL_USER_UPNP:
            strcpy(buf, "UPnP");
            break;
#endif
#if defined(VTSS_SW_OPTION_PTP) || defined(VTSS_SW_OPTION_PHY_1588_SIM)
        case ACL_USER_PTP:
            strcpy(buf, "PTP ");
            break;
#endif
#ifdef VTSS_SW_OPTION_DHCP_HELPER
        case ACL_USER_DHCP:
            strcpy(buf, "DHCP");
            break;
#endif
#ifdef VTSS_SW_OPTION_LOOP_PROTECT
        case ACL_USER_LOOP_PROTECT:
            strcpy(buf, "LOOP");
            break;
#endif
#ifdef VTSS_SW_OPTION_ETH_LINK_OAM
        case ACL_USER_LINK_OAM:
            strcpy(buf, "LOAM");
            break;
#endif
        default:
            strcpy(buf, "?   ");
        }
    } else {
        sprintf(buf, "%s", acl_user_names[user_id]);
    }

    return buf;
}

static void cli_cmd_acl_overview(acl_user_t user_id, vtss_ace_id_t id, acl_entry_conf_t *conf,
                                 ulong counter, BOOL *first, BOOL local_status)
{
    vtss_ace_u8_t   *proto;
    char            buf[MGMT_PORT_BUF_SIZE * 2];
    acl_action_t    *action = &conf->action;
    int             i;
#if defined(VTSS_FEATURE_ACL_V2)
    int             port_list_cnt, port_filter_list_cnt;
    BOOL            is_filter_enable;
#endif /* VTSS_FEATURE_ACL_V2 */

    if (*first) {
        *first = 0;
        if (local_status) {
            CPRINTF("User\n");
            CPRINTF("----\n");
            for (i = 0; i < ACL_USER_CNT; i++) {
                CPRINTF("%s: %s\n", cli_acl_user_txt(1, i, buf), acl_user_names[i]);
            }
#if defined(VTSS_FEATURE_ACL_V2)
            CPRINTF("\nUser ID   Port    Frame  Action Rate L.  Port R.  Mirror   CPU    Counter Confl.\n");
            CPRINTF("---- --   ------- -----  ------ -------- -------- -------- ------ ------- ------\n");
#else
            CPRINTF("\nUser ID   Port    Frame  Action Rate L.  Port R.  CPU    Counter Confl.\n");
            CPRINTF("---- --   ------- -----  ------ -------- -------- ------ ------- ------\n");
#endif /* VTSS_FEATURE_ACL_V2 */
        } else {
            CPRINTF("ID   ");
#if VTSS_SWITCH_STACKABLE
            CPRINTF("SID      ");
#else
            CPRINTF("Type     ");
#endif
#if defined(VTSS_FEATURE_ACL_V2)
            CPRINTF("Port    Policy   Frame  Action Rate L.  Port R.  Mirror   Counter\n");
            CPRINTF("--   ------- -------- -------- -----  ------ -------- -------- -------- -------\n");
#else
            CPRINTF("Port    Policy   Frame  Action Rate L.  Port R.  Counter\n");
            CPRINTF("--   ------- -------- -------- -----  ------ -------- -------- -------\n");
#endif /* VTSS_FEATURE_ACL_V2 */
        }
    }

    if (local_status) {
        CPRINTF("%-5s", cli_acl_user_txt(1, user_id, buf));
        CPRINTF("%-5d", id);
    } else {
        CPRINTF("%-5d%-9s", id, cli_acl_usid_txt(conf->isid, buf));
    }

    //ingress port
#if defined(VTSS_FEATURE_ACL_V2)
    for (port_list_cnt = 0, port_filter_list_cnt = 0, i = VTSS_PORT_NO_START; i < VTSS_PORT_NO_END; i++) {
        if (conf->port_list[i]) {
            port_list_cnt++;
        }
        if (action->port_list[i]) {
            port_filter_list_cnt++;
        }
    }
    if (port_list_cnt == VTSS_PORT_NO_END) {
        CPRINTF("%-8s", "All");
    } else {
        CPRINTF("%-8s", mgmt_iport_list2txt(conf->port_list, buf));
    }
#else
    if (conf->port_no == VTSS_PORT_NO_ANY) {
        CPRINTF("%-8s", "All");
    } else {
        CPRINTF("%-8u", iport2uport(conf->port_no));
    }
#endif

    //policy, policy_bitmask
    if (!local_status) {
        if (conf->policy.mask == 0x0) {
            CPRINTF("%-9s", "Any");
        } else {
            CPRINTF("%-3d/0x%-3x", conf->policy.value, conf->policy.mask);
        }
    }
    proto = &conf->frame.ipv4.proto;
    CPRINTF("%-7s",
            conf->type != VTSS_ACE_TYPE_IPV4 ? mgmt_acl_type_txt(conf->type, buf, 0) :
            proto->mask == 0xff && proto->value == 1 ? "ICMP" :
            proto->mask == 0xff && proto->value == 17 ? "UDP" :
            proto->mask == 0xff && proto->value == 6 ? "TCP" : "IP");
#if defined(VTSS_FEATURE_ACL_V2)
    if (action->port_action == VTSS_ACL_PORT_ACTION_FILTER && port_filter_list_cnt != 0) {
        is_filter_enable = TRUE;
    } else {
        is_filter_enable = FALSE;
    }
    CPRINTF("%-7s", action->port_action == VTSS_ACL_PORT_ACTION_NONE ? "Permit" : is_filter_enable ? "Filter" : "Deny");
#else
    CPRINTF("%-7s", action->permit ? "Permit" : "Deny");
#endif /* VTSS_FEATURE_ACL_V2 */
    CPRINTF("%-9s", cli_acl_policer_no_txt(action->policer, buf));
#if defined(VTSS_FEATURE_ACL_V2)
    if (action->port_action == VTSS_ACL_PORT_ACTION_REDIR) {
        CPRINTF("%-9s", mgmt_iport_list2txt(action->port_list, buf));
    } else {
        CPRINTF("%-9s", "Disabled");
    }
    CPRINTF("%-9s", cli_bool_txt(action->mirror));
#else
    CPRINTF("%-9s", cli_acl_port_no_txt(action->port_no, buf));
#endif /* VTSS_FEATURE_ACL_V2 */
    if (local_status) {
        CPRINTF("%-3s%-4s", conf->action.force_cpu ? "Yes" : "No", conf->action.cpu_once ? "(O)" : "");
    }
    CPRINTF("%7u", counter);
    if (local_status) {
        CPRINTF(" %-3s", conf->conflict ? "Yes" : "No");
    }
    CPRINTF("\n");
}

static void cli_mgmt_counters_clear(cli_req_t *req)
{
    if (acl_mgmt_counters_clear() != VTSS_OK) {
        CPRINTF("ACL counter clear failed\n");
    }
}

static char *acl_tag_prio_range_txt(char *buf, ulong min, ulong max)
{
    sprintf(buf, "%d-%d", min, max);
    return buf;
}

static void acl_cli_cmd_list(cli_req_t *req, acl_user_t user_id)
{
    acl_entry_conf_t    conf;
    acl_action_t        *action = &conf.action;
    uchar               ip_proto;
    BOOL                first = 1, icmp, udp, tcp;
    vtss_ace_id_t       id;
    ulong               counter;
    char                col[80], buf[MGMT_PORT_BUF_SIZE];
    vtss_isid_t         isid;
    int                 i;
    acl_cli_req_t       *acl_req = req->module_req;
#if defined(ACL_IPV6_SUPPORTED)
    ulong               sip_v6_mask;
#endif /* ACL_IPV6_SUPPORTED */
#if defined(VTSS_FEATURE_ACL_V2)
    int                 port_list_cnt, port_filter_list_cnt;
    BOOL                is_filter_enable;
#endif /* VTSS_FEATURE_ACL_V2 */

    if (cli_cmd_switch_none(req)) {
        return;
    }

    isid = (req->stack.master ?
            (req->usid_sel == VTSS_USID_ALL ?
             VTSS_ISID_GLOBAL : req->stack.isid[req->usid_sel]) :
            VTSS_ISID_LOCAL);
    id = acl_req->ace_id;
    if (id != ACE_ID_NONE) {
        /* Detailed view */
        if (acl_mgmt_ace_get(acl_req->acl_user, isid, id, &conf, &counter, 0) != VTSS_OK) {
            return;
        }

        sprintf(col, "ACE ID        : %d", conf.id);
        CPRINTF("%-35s Rate Limiter : %s\n", col, cli_acl_policer_no_txt(action->policer, buf));
#if defined(VTSS_FEATURE_ACL_V2)
        for (port_list_cnt = 0, port_filter_list_cnt = 0, i = VTSS_PORT_NO_START; i < VTSS_PORT_NO_END; i++) {
            if (conf.port_list[i]) {
                port_list_cnt++;
            }
            if (action->port_list[i]) {
                port_filter_list_cnt++;
            }
        }
        if (port_list_cnt == VTSS_PORT_NO_END) {
            sprintf(col, "Ingress Port  : All");
        } else {
            sprintf(col, "Ingress Port  : %s", mgmt_iport_list2txt(conf.port_list, buf));
        }
#else
        if (conf.port_no == VTSS_PORT_NO_ANY) {
            sprintf(col, "Ingress Port  : All");
        } else {
            sprintf(col, "Ingress Port  : %u", iport2uport(conf.port_no));
        }
#endif /* VTSS_FEATURE_ACL_V2 */

#if defined(VTSS_FEATURE_ACL_V2)
        if (action->port_action == VTSS_ACL_PORT_ACTION_REDIR) {
#if defined(VTSS_ARCH_LUTON28)
            CPRINTF("%-35s Port Copy    : %s\n", col, mgmt_iport_list2txt(action->port_list, buf));
#else
            CPRINTF("%-35s Port Redirect: %s\n", col, mgmt_iport_list2txt(action->port_list, buf));
#endif
        } else {
#if defined(VTSS_ARCH_LUTON28)
            CPRINTF("%-35s Port Copy    : Disabled\n", col);
#else
            CPRINTF("%-35s Port Redirect: Disabled\n", col);
#endif
        }
        col[0] = '\0';
#if defined(VTSS_ARCH_CARACAL) && defined(VTSS_SW_OPTION_EVC)
        if (action->evc_police) {
            sprintf(col, "EVC Policer   : %u", ievcpolicer2uevcpolicer(action->evc_policer_id));
        } else {
            sprintf(col, "EVC Policer   : Disabled");
        }
#endif /* VTSS_ARCH_CARACAL && VTSS_SW_OPTION_EVC */
        CPRINTF("%-35s Mirror       : %s\n", col, cli_bool_txt(action->mirror));
#else
#if defined(VTSS_ARCH_CARACAL) && defined(VTSS_SW_OPTION_EVC)
        if (action->evc_police) {
            CPRINTF("EVC Policer   : %u\n", ievcpolicer2uevcpolicer(action->evc_policer_id));
        } else {
            CPRINTF("EVC Policer   : Disabled\n");
        }
#endif /* VTSS_ARCH_CARACAL && VTSS_SW_OPTION_EVC */
#if defined(VTSS_ARCH_LUTON28)
        CPRINTF("%-35s Port Copy    : %s\n", col, cli_acl_port_no_txt(action->port_no, buf));
#else
        CPRINTF("%-35s Port Redirect: %s\n", col, cli_acl_port_no_txt(action->port_no, buf));
#endif
#endif /* VTSS_FEATURE_ACL_V2 */

        if (conf.policy.mask == 0x0) {
            sprintf(col, "Policy/Bitmask: Any");
        } else {
            sprintf(col, "Policy/Bitmask: %d/0x%X", conf.policy.value, conf.policy.mask);
        }
        CPRINTF("%-35s Logging      : %s\n", col, cli_bool_txt(action->logging));
#if VTSS_SWITCH_STACKABLE
        sprintf(col, "Switch ID     : %s", cli_acl_usid_txt(conf.isid, buf));
#else
        sprintf(col, "Type          : %s", cli_acl_usid_txt(conf.isid, buf));
#endif
        CPRINTF("%-35s Shutdown     : %s\n", col, cli_bool_txt(action->shutdown));
        sprintf(col, "Frame Type    : %s", mgmt_acl_type_txt(conf.type, buf, 0));
        CPRINTF("%-35s Counter      : %u\n", col, counter);
#if defined(VTSS_FEATURE_ACL_V2)
        if (action->port_action == VTSS_ACL_PORT_ACTION_FILTER && port_filter_list_cnt != 0) {
            is_filter_enable = TRUE;
        } else {
            is_filter_enable = FALSE;
        }
        sprintf(col, "Action        : %s", action->port_action == VTSS_ACL_PORT_ACTION_NONE ? "Permit" : is_filter_enable ? "Filter" : "Deny");
        CPRINTF("%-35s %-11s  %s %s\n", col, is_filter_enable ? "Filter Port" : " ", is_filter_enable ? ":" : " ", is_filter_enable ? port_filter_list_cnt == VTSS_PORT_NO_END ? "All" : mgmt_iport_list2txt(action->port_list, buf) : " ");
#else
        CPRINTF("Action        : %s\n", action->permit ? "Permit" : "Deny");
#endif /* VTSS_FEATURE_ACL_V2 */
#if defined(VTSS_ARCH_SERVAL)
        CPRINTF("Lookup        : %s\n", cli_bool_txt(conf.lookup));
#endif /* VTSS_ARCH_SERVAL */
        if (acl_req->acl_user != ACL_USER_STATIC) {
            CPRINTF("%-35s Force CPU     : %s\n", col, action->force_cpu ? "Yes" : "No");
            sprintf(col, "CPU Once    : %s", action->cpu_once ? "Yes" : "No");
            CPRINTF("Conflict    : %s\n", conf.conflict ? "Yes" : "No");
        }
        sprintf(buf, "\n%-35s %s", "MAC Parameters", "VLAN Parameters");
        cli_parm_header(buf);
#if defined(VTSS_FEATURE_ACL_V2)
        if (conf.type == VTSS_ACE_TYPE_ANY) {
            col[0] = '\0';
        } else {
            sprintf(col, "DMAC Type     : %s", mgmt_acl_dmac_txt(&conf, buf, 0));
        }
        CPRINTF("%-35s 802.1Q Tagged: %s\n", col, conf.tagged == VTSS_ACE_BIT_ANY ? "Any" : conf.tagged == VTSS_ACE_BIT_0 ? "Disabled" : "Enabled");
#else
        sprintf(col, "DMAC Type     : %s", mgmt_acl_dmac_txt(&conf, buf, 0));
        CPRINTF("%-35s VLAN ID      : %s\n",
                col, mgmt_acl_ulong_txt(conf.vid.value, conf.vid.mask, buf, 0));
#endif /* VTSS_FEATURE_ACL_V2 */
        col[0] = '\0';
        if (conf.type == VTSS_ACE_TYPE_ETYPE)
            sprintf(col, "DMAC          : %s",
                    mgmt_acl_uchar6_txt(&conf.frame.etype.dmac, buf, 0));
        if (conf.type == VTSS_ACE_TYPE_ARP)
            sprintf(col, "SMAC          : %s",
                    mgmt_acl_uchar6_txt(&conf.frame.arp.smac, buf, 0));
#if defined(VTSS_FEATURE_ACL_V2)
        if (conf.type == VTSS_ACE_TYPE_IPV4 && conf.frame.ipv4.sip_smac.enable)
            sprintf(col, "SMAC          : %s",
                    misc_mac_txt(conf.frame.ipv4.sip_smac.smac.addr, buf));
        CPRINTF("%-35s VLAN ID      : %s\n",
                col, mgmt_acl_ulong_txt(conf.vid.value, conf.vid.mask, buf, 0));
        if (conf.type != VTSS_ACE_TYPE_ETYPE)
            CPRINTF("%-35s Tag Priority : %s\n",
                    "", (conf.usr_prio.mask != 0 && conf.usr_prio.mask != 0x7) ? acl_tag_prio_range_txt(buf, conf.usr_prio.value & conf.usr_prio.mask, conf.usr_prio.value) :
                    mgmt_acl_ulong_txt(conf.usr_prio.value, conf.usr_prio.mask, buf, 0));
#else
        CPRINTF("%-35s Tag Priority : %s\n",
                col, (conf.usr_prio.mask != 0 && conf.usr_prio.mask != 0x7) ? acl_tag_prio_range_txt(buf, conf.usr_prio.value & conf.usr_prio.mask, conf.usr_prio.value) :
                mgmt_acl_ulong_txt(conf.usr_prio.value, conf.usr_prio.mask, buf, 0));
#endif /* VTSS_FEATURE_ACL_V2 */

        if (conf.type == VTSS_ACE_TYPE_ETYPE) {
#if defined(VTSS_FEATURE_ACL_V2)
            sprintf(col, "SMAC          : %s", mgmt_acl_uchar6_txt(&conf.frame.etype.smac, buf, 0));
            CPRINTF("%-35s Tag Priority : %s\n",
                    col, mgmt_acl_ulong_txt(conf.usr_prio.value, conf.usr_prio.mask, buf, 0));
#else
            CPRINTF("SMAC          : %s\n", mgmt_acl_uchar6_txt(&conf.frame.etype.smac, buf, 0));
#endif /* VTSS_FEATURE_ACL_V2 */
            CPRINTF("Ether Type    : %s\n",
                    mgmt_acl_uchar2_txt(&conf.frame.etype.etype, buf, 0));
        }

        if (conf.type == VTSS_ACE_TYPE_ARP) {
            CPRINTF("\n");
            cli_parm_header("ARP/RARP Parameters");
            sprintf(col, "Opcode        : %s", mgmt_acl_opcode_txt(&conf, buf, 0));
            CPRINTF("%-35s Sender MAC   : %s\n", col,
                    mgmt_acl_flag_txt(&conf, ACE_FLAG_ARP_SMAC, 0));
            sprintf(col, "Request       : %s", mgmt_acl_flag_txt(&conf, ACE_FLAG_ARP_REQ, 0));
            CPRINTF("%-35s Target MAC   : %s\n", col,
                    mgmt_acl_flag_txt(&conf, ACE_FLAG_ARP_DMAC, 0));
            sprintf(col, "Sender IP     : %s", mgmt_acl_ipv4_txt(&conf.frame.arp.sip, buf, 0));
            CPRINTF("%-35s Length Check : %s\n", col,
                    mgmt_acl_flag_txt(&conf, ACE_FLAG_ARP_LEN, 0));
            sprintf(col, "Target IP     : %s", mgmt_acl_ipv4_txt(&conf.frame.arp.dip, buf, 0));
            CPRINTF("%-35s Ethernet     : %s\n", col,
                    mgmt_acl_flag_txt(&conf, ACE_FLAG_ARP_ETHER, 0));
            CPRINTF("%-35s IP           : %s\n", "",
                    mgmt_acl_flag_txt(&conf, ACE_FLAG_ARP_IP, 0));
        }

        if (conf.type == VTSS_ACE_TYPE_IPV4) {
            ip_proto = mgmt_acl_ip_proto(&conf);
            icmp = (ip_proto == 1);
            udp = (ip_proto == 17);
            tcp = (ip_proto == 6);
            if (icmp || udp || tcp) {
                CPRINTF("\n");
            }
            sprintf(buf, "%-35s %s",
                    "IP Parameters",
                    icmp ? "ICMP Parameters" :
                    udp ? "UDP Parameters" :
                    tcp ? "TCP Parameters" : "");
            cli_parm_header(buf);
            sprintf(col, "Protocol      : %s",
                    mgmt_acl_uchar_txt(&conf.frame.ipv4.proto, buf, 0));
            CPRINTF("%-35s ", col);
            if (icmp)
                CPRINTF("Type         : %s",
                        mgmt_acl_ulong_txt(conf.frame.ipv4.data.value[0],
                                           conf.frame.ipv4.data.mask[0], buf, 0));
            if (udp || tcp)
                CPRINTF("Source Port  : %s",
                        mgmt_acl_port_txt(&conf.frame.ipv4.sport, buf, 0));
            CPRINTF("\n");
#if defined(VTSS_FEATURE_ACL_V2)
            if (conf.type == VTSS_ACE_TYPE_IPV4 && conf.frame.ipv4.sip_smac.enable) {
                sprintf(col, "Source        : %s", misc_ipv4_txt(conf.frame.ipv4.sip_smac.sip, buf));
            } else {
                sprintf(col, "Source        : %s", mgmt_acl_ipv4_txt(&conf.frame.ipv4.sip, buf, 0));
            }
#else
            sprintf(col, "Source        : %s", mgmt_acl_ipv4_txt(&conf.frame.ipv4.sip, buf, 0));
#endif /* VTSS_FEATURE_ACL_V2 */
            CPRINTF("%-35s ", col);
            if (icmp)
                CPRINTF("Code         : %s",
                        mgmt_acl_ulong_txt(conf.frame.ipv4.data.value[1],
                                           conf.frame.ipv4.data.mask[1], buf, 0));
            if (udp || tcp)
                CPRINTF("Dest. Port   : %s",
                        mgmt_acl_port_txt(&conf.frame.ipv4.dport, buf, 0));
            CPRINTF("\n");
            sprintf(col, "Destination   : %s", mgmt_acl_ipv4_txt(&conf.frame.ipv4.dip, buf, 0));
            CPRINTF("%-35s ", col);
            if (tcp) {
                CPRINTF("SYN          : %s", mgmt_acl_flag_txt(&conf, ACE_FLAG_TCP_SYN, 0));
            }
            CPRINTF("\n");
            sprintf(col, "TTL           : %s", mgmt_acl_flag_txt(&conf, ACE_FLAG_IP_TTL, 0));
            CPRINTF("%-35s ", col);
            if (tcp) {
                CPRINTF("ACK          : %s", mgmt_acl_flag_txt(&conf, ACE_FLAG_TCP_ACK, 0));
            }
            CPRINTF("\n");
            sprintf(col, "Fragment      : %s",
                    mgmt_acl_flag_txt(&conf, ACE_FLAG_IP_FRAGMENT, 0));
            CPRINTF("%-35s ", col);
            if (tcp) {
                CPRINTF("FIN          : %s", mgmt_acl_flag_txt(&conf, ACE_FLAG_TCP_FIN, 0));
            }
            CPRINTF("\n");

            sprintf(col, "Options       : %s", mgmt_acl_flag_txt(&conf, ACE_FLAG_IP_OPTIONS, 0));
            CPRINTF("%-35s ", col);
            if (tcp) {
                CPRINTF("RST          : %s", mgmt_acl_flag_txt(&conf, ACE_FLAG_TCP_RST, 0));
            }
            CPRINTF("\n");

            if (tcp) {
                CPRINTF("%-35s URG          : %s\n",
                        "",  mgmt_acl_flag_txt(&conf, ACE_FLAG_TCP_URG, 0));
                CPRINTF("%-35s PSH          : %s\n",
                        "",  mgmt_acl_flag_txt(&conf, ACE_FLAG_TCP_PSH, 0));
            }
        }

        if (conf.type == VTSS_ACE_TYPE_IPV6) {
            CPRINTF("\n");
            sprintf(buf, "%-35s", "IPv6 Parameters");
            cli_parm_header(buf);
            sprintf(col, "Next Header : %s",
                    mgmt_acl_uchar_txt(&conf.frame.ipv6.proto, buf, 0));
            CPRINTF("%-35s Hop Limit    : %s\n",
                    col, conf.frame.ipv6.ttl == VTSS_ACE_BIT_ANY ? "Any" : conf.frame.ipv6.ttl == VTSS_ACE_BIT_0 ? "0" : "1");
            CPRINTF("Source      : %s\n",
                    mgmt_acl_ipv6_txt(&conf.frame.ipv6.sip, buf, 0));
#if defined(ACL_IPV6_SUPPORTED)
            sip_v6_mask = (conf.frame.ipv6.sip.mask[12] << 24) +
                          (conf.frame.ipv6.sip.mask[13] << 16) +
                          (conf.frame.ipv6.sip.mask[14] << 8) +
                          conf.frame.ipv6.sip.mask[15];
            if (sip_v6_mask) {
                CPRINTF("Source Mask : 0x%X\n", sip_v6_mask);
            }
#endif /* ACL_IPV6_SUPPORTED */
            ip_proto = mgmt_acl_ip_proto(&conf);
            icmp = (ip_proto == 1 || ip_proto == 58);
            udp = (ip_proto == 17);
            tcp = (ip_proto == 6);
            if (icmp || udp || tcp) {
                CPRINTF("\n");
                sprintf(buf, "%s",
                        icmp ? "ICMP Parameters" :
                        udp ? "UDP Parameters" : "TCP Parameters");
                cli_parm_header(buf);
            }
            if (icmp) {
                CPRINTF("Type         : %s\n",
                        mgmt_acl_ulong_txt(conf.frame.ipv6.data.value[0],
                                           conf.frame.ipv6.data.mask[0], buf, 0));
                CPRINTF("Code         : %s\n",
                        mgmt_acl_ulong_txt(conf.frame.ipv6.data.value[1],
                                           conf.frame.ipv6.data.mask[1], buf, 0));
            }
            if (udp || tcp) {
                CPRINTF("Source Port : %s\n",
                        mgmt_acl_port_txt(&conf.frame.ipv6.sport, buf, 0));
                CPRINTF("Dest. Port  : %s\n",
                        mgmt_acl_port_txt(&conf.frame.ipv6.dport, buf, 0));
                if (tcp) {
                    CPRINTF("SYN         : %s\n", mgmt_acl_flag_txt(&conf, ACE_FLAG_TCP_SYN, 0));
                    CPRINTF("ACK         : %s\n", mgmt_acl_flag_txt(&conf, ACE_FLAG_TCP_ACK, 0));
                    CPRINTF("FIN         : %s\n", mgmt_acl_flag_txt(&conf, ACE_FLAG_TCP_FIN, 0));
                    CPRINTF("RST         : %s\n", mgmt_acl_flag_txt(&conf, ACE_FLAG_TCP_RST, 0));
                    CPRINTF("URG         : %s\n", mgmt_acl_flag_txt(&conf, ACE_FLAG_TCP_URG, 0));
                    CPRINTF("PSH         : %s\n", mgmt_acl_flag_txt(&conf, ACE_FLAG_TCP_PSH, 0));
                }
            }
        }

#if defined(VTSS_FEATURE_ACL_V2)
        if (user_id != ACL_USER_STATIC &&
            (conf.type == VTSS_ACE_TYPE_ETYPE || conf.type == VTSS_ACE_TYPE_IPV4 || conf.type == VTSS_ACE_TYPE_IPV6)) {
            vtss_ace_ptp_t *ptp;

            CPRINTF("\n");
            sprintf(buf, "%s", "PTP Parameters");
            cli_parm_header(buf);
            if (conf.type == VTSS_ACE_TYPE_ETYPE) {
                ptp = &conf.frame.etype.ptp;
            } else if (conf.type == VTSS_ACE_TYPE_IPV4) {
                ptp = &conf.frame.ipv4.ptp;
            } else {
                ptp = &conf.frame.ipv6.ptp;
            }
            sprintf(col, "PTP         : %s", cli_bool_txt(ptp->enable));
            CPRINTF("%-35s PTP Header    : %s, %s, %s, %s\n",
                    col,
                    mgmt_acl_ulong_txt(ptp->header.value[0], ptp->header.value[0], buf, 0),
                    mgmt_acl_ulong_txt(ptp->header.value[1], ptp->header.value[1], buf, 0),
                    mgmt_acl_ulong_txt(ptp->header.value[2], ptp->header.value[2], buf, 0),
                    mgmt_acl_ulong_txt(ptp->header.value[3], ptp->header.value[3], buf, 0));
        }
#endif /* VTSS_FEATURE_ACL_V2 */

        return;
    }

    /* Overview */
    i = 0;
    id = ACE_ID_NONE;
    while (acl_mgmt_ace_get(ACL_USER_STATIC, isid, id, &conf, &counter, 1) == VTSS_OK) {
        id = conf.id;
        cli_cmd_acl_overview(ACL_USER_STATIC, id, &conf, counter, &first, 0);
        i++;
    }
    CPRINTF("%sNumber of ACEs: %d\n", i ? "\n" : "", i);
}

#ifdef VTSS_SW_OPTION_IP_SOURCE_GUARD
static int acl_ip_source_guard_enabled(void)
{
    ulong mode;

    return (ip_source_guard_mgmt_conf_get_mode(&mode) == VTSS_OK &&
            mode == IP_SOURCE_GUARD_MGMT_ENABLED ? 1 : 0);
}
#endif

static int32_t cli_acl_action_set(cli_req_t *req, acl_action_t *action,
                                  BOOL add, u32 port_count)
{
#if defined(VTSS_FEATURE_ACL_V2)
    vtss_port_no_t iport;
#endif /* VTSS_FEATURE_ACL_V2 */
    acl_cli_req_t  *acl_req = req->module_req;

    if (add || acl_req->policer_spec != CLI_SPEC_NONE) {
        action->policer = (acl_req->policer == 0 ? ACL_POLICER_NONE : upolicer2ipolicer(acl_req->policer));
    }

#if defined(VTSS_ARCH_CARACAL) && defined(VTSS_SW_OPTION_EVC)
    if (add || acl_req->evc_policer_spec != CLI_SPEC_NONE) {
        action->evc_police = acl_req->evc_policer == 0 ? FALSE : TRUE;
        if (action->evc_police) {
            action->evc_policer_id = uevcpolicer2ievcpolicer(acl_req->evc_policer);
        }
    }
#endif /* VTSS_ARCH_CARACAL && VTSS_SW_OPTION_EVC */

#if defined(VTSS_FEATURE_ACL_V2)
    if (add) {
        /* No filtering by default */
        action->port_action = VTSS_ACL_PORT_ACTION_NONE;
        for (iport = VTSS_PORT_NO_START; iport < VTSS_PORT_NO_END; iport++) {
            action->port_list[iport] = 0;
        }
    }
    if (acl_req->permit_spec != CLI_SPEC_NONE) {
        /* Permit/deny */
        if (action->port_action != VTSS_ACL_PORT_ACTION_REDIR) {
            action->port_action = ((acl_req->permit == 1) ? VTSS_ACL_PORT_ACTION_NONE :
                                   VTSS_ACL_PORT_ACTION_FILTER);
            for (iport = VTSS_PORT_NO_START; iport < VTSS_PORT_NO_END; iport++) {
                if (acl_req->permit == 2) { // filter
                    action->port_list[iport] = acl_req->uport_filter_list[iport2uport(iport)];
                } else {
                    action->port_list[iport] = acl_req->permit;
                }
            }
        }
    }
    if (acl_req->port_copy_spec != CLI_SPEC_NONE) {
        /* Port copy */
        if (acl_req->permit_spec == CLI_SPEC_VAL && acl_req->permit != 0) {
            CPRINTF("The parameter of 'port_redirect' can't be set when action is permitted or filtered\n");
            return VTSS_UNSPECIFIED_ERROR;
        }
        action->port_action = VTSS_ACL_PORT_ACTION_REDIR;
        for (iport = VTSS_PORT_NO_START; iport < VTSS_PORT_NO_END; iport++) {
            action->port_list[iport] = acl_req->uport_copy_list[iport2uport(iport)];
        }
    }
    if (add || acl_req->mirror_spec != CLI_SPEC_NONE) {
        action->mirror = acl_req->mirror;
    }
#else
    if (add || acl_req->permit_spec != CLI_SPEC_NONE) {
        action->permit = (acl_req->permit_spec == CLI_SPEC_VAL ? acl_req->permit : 1);
    }
    if (add || acl_req->port_copy_spec != CLI_SPEC_NONE) {
        vtss_uport_no_t uport = acl_req->uport_copy;
        if (uport <= port_count) {
            action->port_no = (uport ? uport2iport(uport) : VTSS_PORT_NO_NONE);
        }
    }
#if defined(VTSS_ARCH_JAGUAR_1)
    if (acl_req->port_copy_spec != CLI_SPEC_NONE) {
        /* Port copy */
        if (action->permit) {
            CPRINTF("The parameter of 'port_redirect' can't be set when action is permitted\n");
            return VTSS_UNSPECIFIED_ERROR;
        }
    }
#endif
#endif /* VTSS_FEATURE_ACL_V2 */
    if (add || acl_req->logging_spec != CLI_SPEC_NONE) {
        action->logging = acl_req->logging;
    }
    if (add || acl_req->shutdown_spec != CLI_SPEC_NONE) {
        action->shutdown = acl_req->shutdown;
    }
    return VTSS_OK;
}

static void cli_cmd_acl_conf(cli_req_t *req, BOOL action, BOOL policy, BOOL rate, BOOL acl)
{
    vtss_usid_t           usid;
    vtss_isid_t           isid;
    vtss_uport_no_t       uport;
    vtss_port_no_t        iport;
    vtss_acl_policer_no_t policer_no;
    acl_port_conf_t       port_conf;
    acl_action_t          *act;
    acl_policer_conf_t    policer_conf;
    BOOL                  first;
    char                  buf[MGMT_PORT_BUF_SIZE * 2], *p;
    ulong                 counter, port_count;
    acl_cli_req_t         *acl_req = req->module_req;

    if (cli_cmd_conf_slave(req)) {
        return;
    }

    if ((action || policy) && cli_cmd_switch_none(req)) {
        return;
    }

    act = &port_conf.action;
    for (usid = VTSS_USID_START; (action || policy) && usid < VTSS_USID_END; usid++) {
        if ((isid = req->stack.isid[usid]) == VTSS_ISID_END) {
            continue;
        }

        first = 1;
        port_count = port_isid_port_count(isid);
        for (iport = VTSS_PORT_NO_START; iport < port_count; iport++) {
            uport = iport2uport(iport);
            if (req->uport_list[uport] == 0 ||
                port_isid_port_no_is_stack(isid, iport) ||
                acl_mgmt_port_conf_get(isid, iport, &port_conf) != VTSS_OK) {
                continue;
            }

            if (req->set) {
                if (cli_acl_action_set(req, act, 0, port_count)) {
                    return;
                }
                if (policy) {
#ifdef VTSS_SW_OPTION_IP_SOURCE_GUARD
                    if (acl_ip_source_guard_enabled()) {
                        CPRINTF("Can't set default port policy cause of reserved for IP source guard management\n");
                        return;
                    }
#endif
                    port_conf.policy_no = acl_req->policy_no;
                }
                if (acl_mgmt_port_conf_set(isid, iport, &port_conf) != VTSS_OK) {
                    CPRINTF("ACL port configuration failed\n");
                }
            } else {
                if (first) {
                    cli_cmd_usid_print(usid, req, 1);
                    p = &buf[0];
                    p += sprintf(p, "Port  ");
                    if (policy) {
                        p += sprintf(p, "Policy  ");
                    }
                    if (action) {
#if defined(VTSS_FEATURE_ACL_V2)
#if defined(VTSS_ARCH_CARACAL) && defined(VTSS_SW_OPTION_EVC)
                        p += sprintf(p, "Action  Rate L.  EVC P.   Port R.  Mirror    Logging   Shutdown  ");
#else
                        p += sprintf(p, "Action  Rate L.  Port R.  Mirror    Logging   Shutdown  ");
#endif /* VTSS_ARCH_CARACAL && VTSS_SW_OPTION_EVC */
#else
#if defined(VTSS_ARCH_CARACAL) && defined(VTSS_SW_OPTION_EVC)
                        p += sprintf(p, "Action  Rate L.  EVC P.   Port R.  Logging   Shutdown  ");
#else
                        p += sprintf(p, "Action  Rate L.  Port R.  Logging   Shutdown  ");
#endif /* VTSS_ARCH_CARACAL && VTSS_SW_OPTION_EVC */
#endif /* VTSS_FEATURE_ACL_V2 */
                    }
                    if (action) {
                        p += sprintf(p, "Counter");
                    }
                    cli_table_header(buf);
                    first = 0;
                }
                CPRINTF("%-2u    ", uport);
                if (policy) {
                    CPRINTF("%u       ", port_conf.policy_no);
                }

                if (action) {
#if defined(VTSS_FEATURE_ACL_V2)
                    CPRINTF("%-8s%-9s", act->port_action == VTSS_ACL_PORT_ACTION_NONE ? "Permit" : "Deny",
                            cli_acl_policer_no_txt(act->policer, buf));
#else
                    CPRINTF("%-8s%-9s", act->permit ? "Permit" : "Deny",
                            cli_acl_policer_no_txt(act->policer, buf));
#endif /* VTSS_FEATURE_ACL_V2 */

#if defined(VTSS_ARCH_CARACAL) && defined(VTSS_SW_OPTION_EVC)
                    if (act->evc_police) {
                        CPRINTF("%-9d", ievcpolicer2uevcpolicer(act->evc_policer_id));
                    } else {
                        CPRINTF("%-9s", "Disabled");
                    }
#endif /* VTSS_ARCH_CARACAL && VTSS_SW_OPTION_EVC */

#if defined(VTSS_FEATURE_ACL_V2)
                    if (act->port_action == VTSS_ACL_PORT_ACTION_REDIR) {
                        CPRINTF("%-9s", mgmt_iport_list2txt(act->port_list, buf));
                    } else {
                        CPRINTF("%-9s", "Disabled");
                    }
                    CPRINTF("%-10s", cli_bool_txt(act->mirror));
#else
                    CPRINTF("%-9s", cli_acl_port_no_txt(act->port_no, buf));
#endif /* VTSS_FEATURE_ACL_V2 */

                    CPRINTF("%-10s%-10s",
                            cli_bool_txt(act->logging), cli_bool_txt(act->shutdown));
                }
                if (action &&
                    acl_mgmt_port_counter_get(isid, iport, &counter) == VTSS_OK) {
                    CPRINTF("%u", counter);
                }
                CPRINTF("\n");
            }
        }
    }

    if (rate) {
        first = 1;
        for (policer_no = VTSS_ACL_POLICER_NO_START;
             policer_no < ACL_POLICER_NO_END; policer_no++) {
            if (acl_req->policer_list[ipolicer2upolicer(policer_no)] == 0) {
                continue;
            }

            if (req->set) {
                if (acl_mgmt_policer_conf_get(policer_no, &policer_conf) == VTSS_OK) {
#if defined(VTSS_FEATURE_ACL_V2)
                    if (acl_req->rate_unit != CLI_RATE_UNIT_KBPS) {
                        policer_conf.bit_rate_enable = FALSE;
                        policer_conf.packet_rate = acl_req->rate;
                    } else {
                        policer_conf.bit_rate_enable = TRUE;
                        policer_conf.bit_rate = acl_req->rate;
                    }
#else
                    policer_conf.packet_rate = acl_req->rate;
#endif /* VTSS_FEATURE_ACL_V2 */
                    if (acl_mgmt_policer_conf_set(policer_no, &policer_conf) != VTSS_OK) {
                        CPRINTF("ACL policer configuration failed\n");
                    }
                }
            } else if (acl_mgmt_policer_conf_get(policer_no, &policer_conf) == VTSS_OK) {
                if (first) {
                    CPRINTF("\n");
#if defined(VTSS_FEATURE_ACL_V2)
                    cli_table_header("Rate Limiter  Rate");
#else
                    cli_table_header("Rate Limiter  Rate (PPS)");
#endif /* VTSS_FEATURE_ACL_V2 */
                    first = 0;
                }
                CPRINTF("%-2u            ", ipolicer2upolicer(policer_no));
#if defined(VTSS_FEATURE_ACL_V2)
                CPRINTF("%d %s\n",
                        policer_conf.bit_rate_enable ? policer_conf.bit_rate : policer_conf.packet_rate,
                        policer_conf.bit_rate_enable ? "KBPS" : "PPS");
#else
                CPRINTF("%d\n", policer_conf.packet_rate);
#endif /* VTSS_FEATURE_ACL_V2 */
            }
        }
    }

    if (acl) {
        CPRINTF("\n");
        acl_cli_cmd_list(req, ACL_USER_STATIC);
    }
}

static void cli_mgmt_debug_lookup(cli_req_t *req)
{
    vtss_usid_t usid;
    vtss_isid_t isid;

    for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) {
        isid = req->stack.isid[usid];
        if (isid == VTSS_ISID_END) {
            continue;
        }
        req->usid_sel = usid;
        acl_cli_cmd_list(req, usid);
    }
}

static void cli_mgmt_debug_del(cli_req_t *req)
{
    acl_cli_req_t *acl_req = req->module_req;

    if (acl_mgmt_ace_del(acl_req->acl_user, acl_req->ace_id) != VTSS_OK) {
        CPRINTF("ACL Delete failed\n");
    }
}

static void acl_cli_cmd_status(cli_req_t *req)
{
    acl_cli_req_t       *acl_req = req->module_req;
    int                 acl_user = acl_req->acl_user;
    vtss_ace_id_t       ace_id = ACE_ID_NONE;
    acl_entry_conf_t    ace_conf;
    ulong               ace_counter;
    BOOL                first = 1;
    vtss_usid_t         usid;
    vtss_isid_t         isid;
    int                 i = 0;

    for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++, first = 1) {
        if ((isid = req->stack.isid[usid]) == VTSS_ISID_END) {
            continue;
        }
        if (first) {
            cli_cmd_usid_print(usid, req, 1);
        }
        i = 0;
        if (acl_req->acl_user_valid == 0) {
            acl_req->acl_user = -1;
        }
        if (acl_req->acl_user < 0) {
            for (acl_user = ACL_USER_CNT - 1; acl_user >= ACL_USER_STATIC; acl_user--) {
                ace_id = ACE_ID_NONE;
                while (acl_mgmt_ace_get(acl_user, isid, ace_id, &ace_conf, &ace_counter, 1) == VTSS_OK) {
                    ace_id = ace_conf.id;
                    if (acl_req->acl_user == -1 || (acl_req->acl_user == -2 && ace_conf.conflict)) {
                        cli_cmd_acl_overview(acl_user, ace_id, &ace_conf, ace_counter, &first, 1);
                        i++;
                    }
                }
            }
        } else {
            ace_id = ACE_ID_NONE;
            while (acl_mgmt_ace_get(acl_user, isid, ace_id, &ace_conf, &ace_counter, 1) == VTSS_OK) {
                ace_id = ace_conf.id;
                cli_cmd_acl_overview(acl_user, ace_id, &ace_conf, ace_counter, &first, 1);
                i++;
            }
        }
        CPRINTF("%sNumber of ACEs: %d\n", i ? "\n" : "", i);
    }
}

static void cli_cmd_acl_config(cli_req_t *req)
{
    if (!req->set) {
        cli_header("ACL Configuration", 1);
    }
    cli_cmd_acl_conf(req, 1, 1, 1, 1);
}

static void cli_cmd_acl_action(cli_req_t *req)
{
    cli_cmd_acl_conf(req, 1, 0, 0, 0);
}

static void cli_cmd_acl_policy(cli_req_t *req)
{
    cli_cmd_acl_conf(req, 0, 1, 0, 0);
}

static void cli_cmd_acl_rate(cli_req_t *req)
{
    cli_cmd_acl_conf(req, 0, 0, 1, 0);
}

static void acl_cli_cmd_lookup(cli_req_t *req)
{
    acl_cli_cmd_list(req, ACL_USER_STATIC);
}

static void acl_cli_cmd_add(cli_req_t *req)
{
    acl_entry_conf_t conf;
    vtss_ace_id_t    id;
    BOOL             add; /* Add: 1, Modify: 0 */
    BOOL             frame_type_change = 0; /* change: 1, no change: 0 */
    int              i;
    acl_flag_t       flag;
    acl_cli_req_t    *acl_req = req->module_req;
    vtss_rc          rc = VTSS_OK;
#if defined(VTSS_FEATURE_ACL_V2)
    vtss_port_no_t        iport;
#endif /* VTSS_FEATURE_ACL_V2 */

    if (acl_mgmt_ace_init(VTSS_ACE_TYPE_ANY, &conf) != VTSS_OK) {
        return;
    }
    id = acl_req->ace_id;
    add = (id == ACE_ID_NONE ||
           acl_mgmt_ace_get(ACL_USER_STATIC, VTSS_ISID_GLOBAL, id, &conf, NULL, 0) != VTSS_OK);

    /* ACE ID */
    conf.id = id;

#if defined(VTSS_ARCH_SERVAL)
    if (add || acl_req->lookup_spec != CLI_SPEC_NONE) {
        conf.lookup = acl_req->lookup;
    }
#endif /* VTSS_ARCH_SERVAL */

#if defined(VTSS_FEATURE_ACL_V2)
    if (req->uport) {
        for (iport = VTSS_PORT_NO_START; iport < VTSS_PORT_NO_END; iport++) {
            conf.port_list[iport] = req->uport_list[iport2uport(iport)];
        }
    } else if (add) {
        for (iport = VTSS_PORT_NO_START; iport < VTSS_PORT_NO_END; iport++) {
            conf.port_list[iport] = TRUE;
        }
    }
#else
    if (req->uport) {
        conf.port_no = uport2iport(req->uport);
    } else if (add) {
        conf.port_no = VTSS_PORT_NO_ANY;
    }
#endif /* VTSS_FEATURE_ACL_V2 */

    if (add || acl_req->policy_no_spec != CLI_SPEC_NONE) {
        conf.policy.value = acl_req->policy_no;
        conf.policy.mask = acl_req->policy_bitmask;
    }

    /* If this operation is modified entry and the frame type had changed,
       we need clear related fileds first */
    if (!add && acl_req->ace_type_spec != CLI_SPEC_NONE) {
        if (conf.type != (acl_req->ace_type_spec == CLI_SPEC_ANY ? VTSS_ACE_TYPE_ANY : acl_req->ace_type)) {
            frame_type_change = 1;
            memset(&conf.frame, 0, sizeof(conf.frame));
#if defined(VTSS_FEATURE_ACL_V2)
            /* (Chip limitation) The DMAC type parameter is not supported for VTSS_ACE_TYPE_ANY. */
            if (acl_req->ace_type_spec == CLI_SPEC_ANY) {
                for (i = ACE_FLAG_DMAC_BC; i < ACE_FLAG_ARP_ARP; i++) {
                    VTSS_BF_SET(conf.flags.mask, i, 0);
                    VTSS_BF_SET(conf.flags.value, i, 0);
                }
            }
#endif /* VTSS_FEATURE_ACL_V2 */
            for (i = ACE_FLAG_ARP_ARP; i < ACE_FLAG_COUNT; i++) {
                VTSS_BF_SET(conf.flags.mask, i, 0);
                VTSS_BF_SET(conf.flags.value, i, 0);
            }
        }
    }

    /* Frame type */
    if (add || acl_req->ace_type_spec != CLI_SPEC_NONE) {
        conf.type = (acl_req->ace_type_spec == CLI_SPEC_ANY ? VTSS_ACE_TYPE_ANY : acl_req->ace_type);
    }

    /* Action */
    if (cli_acl_action_set(req, &conf.action, add, VTSS_PORTS)) {
        return;
    }

    /* Switch ID */
    if (add || acl_req->usid_ace_spec != CLI_SPEC_NONE)
        conf.isid = (
#if VTSS_SWITCH_STACKABLE
                        acl_req->usid_ace_spec == CLI_SPEC_VAL ? topo_usid2isid(acl_req->usid_ace) :
#endif /* VTSS_SWITCH_STACKABLE */
                        VTSS_ISID_GLOBAL);

    /* Flags */
    if (conf.type != VTSS_ACE_TYPE_IPV6) {
        for (i = ACE_FLAG_DMAC_BC; i < ACE_FLAG_COUNT; i++) {
            flag = acl_req->ace_flags[i];
            if (add || flag != CLI_ACE_FLAG_NONE) {
                VTSS_BF_SET(conf.flags.mask, i, flag == CLI_ACE_FLAG_0 || flag == CLI_ACE_FLAG_1);
                VTSS_BF_SET(conf.flags.value, i, flag == CLI_ACE_FLAG_1);
            }
        }
    }

    /* VLAN ID */
    if (add || req->vid_spec != CLI_SPEC_NONE) {
        conf.vid.value = req->vid;
        conf.vid.mask = (req->vid_spec == CLI_SPEC_VAL ? 0xfff : 0);
    }

    /* User priority */
    if (add || acl_req->tag_prio_spec != CLI_SPEC_NONE) {
        conf.usr_prio.value = acl_req->tag_prio;
        conf.usr_prio.mask = (acl_req->tag_prio_spec == CLI_SPEC_VAL ?  acl_req->tag_prio_bitmask : 0);
    }

#if defined(VTSS_FEATURE_ACL_V2)
    /* User priority */
    if (add || acl_req->tagged_spec != CLI_SPEC_NONE) {
        conf.tagged = acl_req->tagged;
    }
    if (conf.tagged == VTSS_ACE_BIT_0 && (conf.vid.mask || conf.usr_prio.mask)) {
        CPRINTF("The parameter of 'vid' and 'tag_prio' can't be set when 802.1Q Tagged is disabled\n");
        return;
    }
#endif /* VTSS_FEATURE_ACL_V2 */

    /* Frame specific data */
    switch (conf.type) {
    case VTSS_ACE_TYPE_ETYPE:
        cli_ace_mac_set(&conf.frame.etype.dmac, acl_req->dmac, acl_req->dmac_spec, add || frame_type_change);
        cli_ace_mac_set(&conf.frame.etype.smac, acl_req->smac, acl_req->smac_spec, add || frame_type_change);
        for (i = 0; i < 2; i++) {
            if (add || frame_type_change || acl_req->etype_spec != CLI_SPEC_NONE) {
                conf.frame.etype.etype.value[i] = ((acl_req->etype >> (i == 0 ? 8 : 0)) & 0xff);
                conf.frame.etype.etype.mask[i] = (acl_req->etype_spec == CLI_SPEC_VAL ? 0xff : 0);
            }
            conf.frame.etype.data.value[i] = 0;
            conf.frame.etype.data.mask[i] = 0;
        }
        break;
    case VTSS_ACE_TYPE_ARP:
        cli_ace_mac_set(&conf.frame.arp.smac, acl_req->smac, acl_req->smac_spec, add || frame_type_change);
        cli_ace_ip_set(&conf.frame.arp.sip, acl_req->sip, acl_req->sip_mask, acl_req->sip_spec, add || frame_type_change);
        cli_ace_ip_set(&conf.frame.arp.dip, acl_req->dip, acl_req->dip_mask, acl_req->dip_spec, add || frame_type_change);
        break;
    case VTSS_ACE_TYPE_IPV4:
#if defined(VTSS_FEATURE_ACL_V2)
        if (acl_req->sip_smac) {
            cli_ace_sip_smc_set(&conf.frame.ipv4.sip_smac, acl_req->sip, acl_req->sip_spec, acl_req->smac, acl_req->smac_spec, add || frame_type_change);
            break;
        }
#endif /* VTSS_FEATURE_ACL_V2 */
        conf.frame.ipv4.ds.value = 0;
        conf.frame.ipv4.ds.mask = 0;
        if (add || frame_type_change || acl_req->ip_proto_spec != CLI_SPEC_NONE) {
            conf.frame.ipv4.proto.value = acl_req->ip_proto;
            conf.frame.ipv4.proto.mask = (acl_req->ip_proto_spec == CLI_SPEC_VAL ? 0xff : 0);
        }
        cli_ace_ip_set(&conf.frame.ipv4.sip, acl_req->sip, acl_req->sip_mask, acl_req->sip_spec, add || frame_type_change);
        cli_ace_ip_set(&conf.frame.ipv4.dip, acl_req->dip, acl_req->dip_mask, acl_req->dip_spec, add || frame_type_change);
        if (add || frame_type_change || acl_req->icmp_type_spec != CLI_SPEC_NONE) {
            conf.frame.ipv4.data.value[0] = acl_req->icmp_type;
            conf.frame.ipv4.data.mask[0] = (acl_req->icmp_type_spec == CLI_SPEC_VAL ? 0xff : 0);
        }
        if (add || frame_type_change || acl_req->icmp_code_spec != CLI_SPEC_NONE) {
            conf.frame.ipv4.data.value[1] = acl_req->icmp_code;
            conf.frame.ipv4.data.mask[1] = (acl_req->icmp_code_spec == CLI_SPEC_VAL ? 0xff : 0);
        }
        for (i = 2; i < 6; i++) {
            conf.frame.ipv4.data.value[i] = 0;
            conf.frame.ipv4.data.mask[i] = 0;
        }
        cli_ace_port_set(&conf.frame.ipv4.sport, acl_req->sport_min, acl_req->sport_max,
                         acl_req->sport_spec, add || frame_type_change);
        cli_ace_port_set(&conf.frame.ipv4.dport, acl_req->dport_min, acl_req->dport_max,
                         acl_req->dport_spec, add || frame_type_change);
        break;
    case VTSS_ACE_TYPE_IPV6:
        if (add || frame_type_change || acl_req->ip_proto_spec != CLI_SPEC_NONE) {
            conf.frame.ipv6.proto.value = acl_req->ip_proto;
            conf.frame.ipv6.proto.mask = (acl_req->ip_proto_spec == CLI_SPEC_VAL ? 0xff : 0);
        }

        if (add || frame_type_change || acl_req->sipv6_spec != CLI_SPEC_NONE) {
            for (i = 0; i < 16; i++) {
                conf.frame.ipv6.sip.value[i] = acl_req->sipv6.addr[i];
#if defined(VTSS_ARCH_LUTON28)
                conf.frame.ipv6.sip.mask[i] = (acl_req->sipv6_spec == CLI_SPEC_VAL ? 0xff : 0);
#else
                if (i >= 12) {
                    conf.frame.ipv6.sip.mask[i] = (acl_req->sipv6_spec == CLI_SPEC_VAL ? 0xff : 0);
                }
#endif /* VTSS_ARCH_LUTON28 */
            }

#if defined(ACL_IPV6_SUPPORTED)
            if (add || frame_type_change || acl_req->sip_v6_mask_spec != CLI_SPEC_NONE) {
                if (acl_req->sip_v6_mask_spec == CLI_SPEC_VAL) {
                    conf.frame.ipv6.sip.mask[12] = (acl_req->sip_v6_mask & 0xff000000) >> 24;
                    conf.frame.ipv6.sip.mask[13] = (acl_req->sip_v6_mask & 0x00ff0000) >> 16;
                    conf.frame.ipv6.sip.mask[14] = (acl_req->sip_v6_mask & 0x0000ff00) >>  8;
                    conf.frame.ipv6.sip.mask[15] = (acl_req->sip_v6_mask & 0x000000ff) >>  0;
                }
            }
#endif /* ACL_IPV6_SUPPORTED */
        }

#if defined(VTSS_FEATURE_ACL_V2)
        conf.frame.ipv6.ds.value = 0;
        conf.frame.ipv6.ds.mask = 0;
        if (add || frame_type_change || acl_req->icmp_type_spec != CLI_SPEC_NONE) {
            conf.frame.ipv6.data.value[0] = acl_req->icmp_type;
            conf.frame.ipv6.data.mask[0] = (acl_req->icmp_type_spec == CLI_SPEC_VAL ? 0xff : 0);
        }
        if (add || frame_type_change || acl_req->icmp_code_spec != CLI_SPEC_NONE) {
            conf.frame.ipv6.data.value[1] = acl_req->icmp_code;
            conf.frame.ipv6.data.mask[1] = (acl_req->icmp_code_spec == CLI_SPEC_VAL ? 0xff : 0);
        }
        for (i = 2; i < 6; i++) {
            conf.frame.ipv6.data.value[i] = 0;
            conf.frame.ipv6.data.mask[i] = 0;
        }
        flag = acl_req->ace_flags[ACE_FLAG_IP_TTL];
        if (add || frame_type_change || flag != CLI_ACE_FLAG_NONE) {
            conf.frame.ipv6.ttl = (flag == CLI_ACE_FLAG_0 ? VTSS_ACE_BIT_0 : flag == CLI_ACE_FLAG_1 ? VTSS_ACE_BIT_1 : VTSS_ACE_BIT_ANY);
        }
        flag = acl_req->ace_flags[ACE_FLAG_TCP_FIN];
        if (add || frame_type_change || flag != CLI_ACE_FLAG_NONE) {
            conf.frame.ipv6.tcp_fin = (flag == CLI_ACE_FLAG_0 ? VTSS_ACE_BIT_0 : flag == CLI_ACE_FLAG_1 ? VTSS_ACE_BIT_1 : VTSS_ACE_BIT_ANY);
        }
        flag = acl_req->ace_flags[ACE_FLAG_TCP_SYN];
        if (add || frame_type_change || flag != CLI_ACE_FLAG_NONE) {
            conf.frame.ipv6.tcp_syn = (flag == CLI_ACE_FLAG_0 ? VTSS_ACE_BIT_0 : flag == CLI_ACE_FLAG_1 ? VTSS_ACE_BIT_1 : VTSS_ACE_BIT_ANY);
        }
        flag = acl_req->ace_flags[ACE_FLAG_TCP_RST];
        if (add || frame_type_change || flag != CLI_ACE_FLAG_NONE) {
            conf.frame.ipv6.tcp_rst = (flag == CLI_ACE_FLAG_0 ? VTSS_ACE_BIT_0 : flag == CLI_ACE_FLAG_1 ? VTSS_ACE_BIT_1 : VTSS_ACE_BIT_ANY);
        }
        flag = acl_req->ace_flags[ACE_FLAG_TCP_PSH];
        if (add || frame_type_change || flag != CLI_ACE_FLAG_NONE) {
            conf.frame.ipv6.tcp_psh = (flag == CLI_ACE_FLAG_0 ? VTSS_ACE_BIT_0 : flag == CLI_ACE_FLAG_1 ? VTSS_ACE_BIT_1 : VTSS_ACE_BIT_ANY);
        }
        flag = acl_req->ace_flags[ACE_FLAG_TCP_ACK];
        if (add || frame_type_change || flag != CLI_ACE_FLAG_NONE) {
            conf.frame.ipv6.tcp_ack = (flag == CLI_ACE_FLAG_0 ? VTSS_ACE_BIT_0 : flag == CLI_ACE_FLAG_1 ? VTSS_ACE_BIT_1 : VTSS_ACE_BIT_ANY);
        }
        flag = acl_req->ace_flags[ACE_FLAG_TCP_URG];
        if (add || frame_type_change || flag != CLI_ACE_FLAG_NONE) {
            conf.frame.ipv6.tcp_urg = (flag == CLI_ACE_FLAG_0 ? VTSS_ACE_BIT_0 : flag == CLI_ACE_FLAG_1 ? VTSS_ACE_BIT_1 : VTSS_ACE_BIT_ANY);
        }
        cli_ace_port_set(&conf.frame.ipv6.sport, acl_req->sport_min, acl_req->sport_max,
                         acl_req->sport_spec, add || frame_type_change);
        cli_ace_port_set(&conf.frame.ipv6.dport, acl_req->dport_min, acl_req->dport_max,
                         acl_req->dport_spec, add || frame_type_change);
#endif /* VTSS_FEATURE_ACL_V2 */
        break;
    case VTSS_ACE_TYPE_ANY:
#if defined(VTSS_FEATURE_ACL_V2)
        /* The DMAC type parameter is not supported for VTSS_ACE_TYPE_ANY.
           This is a Luton26 chip limitation. */
        if (acl_req->ace_flags[ACE_FLAG_DMAC_BC] || acl_req->ace_flags[ACE_FLAG_DMAC_MC]) {
            CPRINTF("The <dmac_type> parameter is not supported for 'Frame Type Any'\n");
            return;
        }
#endif /* VTSS_FEATURE_ACL_V2 */
        break;
    default:
        break;
    }

    if ((rc = acl_mgmt_ace_add(ACL_USER_STATIC, acl_req->ace_id_next, &conf)) == VTSS_OK) {
        CPRINTF("ACE ID %d %s ", conf.id, add ? "added" : "modified");
        if (acl_req->ace_id_next) {
            CPRINTF("before ACE ID %d\n", acl_req->ace_id_next);
        } else {
            CPRINTF("last\n");
        }
    } else if (rc == ACL_ERROR_ACE_TABLE_FULL) {
        CPRINTF("ACL table full\n");
    } else {
        CPRINTF("ACL Add failed\n");
    }
}

static void acl_cli_cmd_debug_ipv6(cli_req_t *req)
{
    acl_cli_cmd_add(req);
}

static void acl_cli_cmd_del(cli_req_t *req)
{
    acl_cli_req_t *acl_req = req->module_req;

    if (acl_mgmt_ace_del(ACL_USER_STATIC, acl_req->ace_id) != VTSS_OK) {
        CPRINTF("ACL Delete failed\n");
    }
}

static void acl_cli_cmd_port_state(cli_req_t *req)
{
    vtss_usid_t     usid;
    vtss_isid_t     isid;
    vtss_uport_no_t uport;
    vtss_port_no_t  iport;
    port_vol_conf_t conf;
    u32             port_count;
    BOOL            first;
    port_user_t     user = PORT_USER_ACL;

    /* Check that we are master and the selected switch is active */
    if (cli_cmd_slave(req) || cli_cmd_switch_none(req)) {
        return;
    }

    for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) {
        if ((isid = req->stack.isid[usid]) == VTSS_ISID_END) {
            continue;
        }

        first = 1;
        port_count = port_isid_port_count(isid);
        memset(&conf, 0, sizeof(conf));
        for (iport = VTSS_PORT_NO_START; iport < port_count; iport++) {
            uport = iport2uport(iport);
            if (req->uport_list[uport] == 0 ||
                port_isid_port_no_is_stack(isid, iport) ||
                (msg_switch_exists(isid) && port_vol_conf_get(user, isid, iport, &conf) != VTSS_OK)) {
                continue;
            }

            if (req->set) {
                conf.disable = req->disable;
                if (port_vol_conf_set(user, isid, iport, &conf) != VTSS_OK) {
                    CPRINTF("failed\n");
                }
            } else {
                if (first) {
                    cli_cmd_usid_print(usid, req, 1);
                    cli_table_header("Port  State  ");
                    first = 0;
                }
                CPRINTF("%-4u  %s\n", uport, cli_bool_txt(!conf.disable));
            }
        }
    }
}

/****************************************************************************/
/*  Parameter functions                                                     */
/****************************************************************************/

#if defined(VTSS_ARCH_SERVAL)
static int32_t cli_parm_parse_lookup(char *cmd, char *cmd2,
                                     char *stx, char *cmd_org, cli_req_t *req)
{
    int           error;
    acl_cli_req_t *acl_req = req->module_req;

    if (!(error = cli_parse_word(cmd, "enable"))) {
        acl_req->lookup = 1;
    } else if (!(error = cli_parse_word(cmd, "disable"))) {
        acl_req->lookup = 0;
    }
    if (!error) {
        acl_req->lookup_spec = CLI_SPEC_VAL;
    }

    return error;
}
#endif /* VTSS_ARCH_SERVAL */

static int cli_parse_dmac_flag(char *stx, cli_ace_flag_t bc, cli_ace_flag_t mc,
                               char *cmd, cli_req_t *req)
{
    int           error = 1;
    acl_cli_req_t *acl_req = req->module_req;

    if (strstr(stx, cmd) == stx) {
        acl_req->ace_flags[ACE_FLAG_DMAC_BC] = bc;
        acl_req->ace_flags[ACE_FLAG_DMAC_MC] = mc;
        error = 0;
    }
    return error;
}

/* Parse ACL ARP opcode flag */
static int cli_parse_arp_flag(char *stx, cli_ace_flag_t arp, cli_ace_flag_t unknown,
                              char *cmd, cli_req_t *req)
{
    int           error = 1;
    acl_cli_req_t *acl_req = req->module_req;

    if (strstr(stx, cmd) == stx) {
        acl_req->ace_flags[ACE_FLAG_ARP_ARP] = arp;
        acl_req->ace_flags[ACE_FLAG_ARP_UNKNOWN] = unknown;
        error = 0;
    }
    return error;
}

static int32_t cli_acl_parse_keyword(char *cmd, char *cmd2, char *stx, char *cmd_org,
                                     cli_req_t *req)
{
    acl_cli_req_t *acl_req = req->module_req;
    char          *found = cli_parse_find(cmd, stx);

    if (found != NULL) {
        if (!strncmp(found, "permit", 6 )) {
            acl_req->permit_spec = CLI_SPEC_VAL;
            acl_req->permit = 1;
        } else if (!strncmp(found, "deny", 4 )) {
            acl_req->permit_spec = CLI_SPEC_VAL;
#if defined(VTSS_FEATURE_ACL_V2)
        } else if ( !strncmp(found, "filter", 6 )) {
            acl_req->permit_spec = CLI_SPEC_VAL;
            acl_req->permit = 2; // filter
#endif /* VTSS_FEATURE_ACL_V2 */
        } else if ( !strncmp(found, "policy", 6 )) {
            acl_req->policy_no = 0;
            acl_req->policy_bitmask = 0x0;
        } else if (!strncmp(found, "etype", 5)) {
            acl_req->ace_type_spec = CLI_SPEC_VAL;
            acl_req->ace_type = VTSS_ACE_TYPE_ETYPE;
        } else if (!strncmp(found, "arp", 3)) {
            acl_req->ace_type_spec = CLI_SPEC_VAL;
            acl_req->ace_type = VTSS_ACE_TYPE_ARP;
        } else if (!strncmp(found, "ipv6_std", 8)) {
            acl_req->ace_type_spec = CLI_SPEC_VAL;
            acl_req->ace_type = VTSS_ACE_TYPE_IPV6;
        } else if (!strncmp(found, "ipv6_icmp", 9)) {
            acl_req->ace_type_spec = CLI_SPEC_VAL;
            acl_req->ace_type = VTSS_ACE_TYPE_IPV6;
            acl_req->ip_proto_spec = CLI_SPEC_VAL;
            acl_req->ip_proto = 58; /* ICMPv6 protocol */
        } else if (!strncmp(found, "ipv6_udp", 8)) {
            acl_req->ace_type_spec = CLI_SPEC_VAL;
            acl_req->ace_type = VTSS_ACE_TYPE_IPV6;
            acl_req->ip_proto_spec = CLI_SPEC_VAL;
            acl_req->ip_proto = 17; /* UDP protocol */
        } else if (!strncmp(found, "ipv6_tcp", 8)) {
            acl_req->ace_type_spec = CLI_SPEC_VAL;
            acl_req->ace_type = VTSS_ACE_TYPE_IPV6;
            acl_req->ip_proto_spec = CLI_SPEC_VAL;
            acl_req->ip_proto = 6; /* TCP protocol */
        } else if (!strncmp(found, "ip", 2)) {
            acl_req->ace_type_spec = CLI_SPEC_VAL;
            acl_req->ace_type = VTSS_ACE_TYPE_IPV4;
            acl_req->ip = 1;
        } else if (!strncmp(found, "tcp", 3)) {
            acl_req->ace_type_spec = CLI_SPEC_VAL;
            acl_req->ace_type = VTSS_ACE_TYPE_IPV4;
            acl_req->ip_proto_spec = CLI_SPEC_VAL;
            acl_req->ip_proto = 6; /* TCP protocol */
        } else if (!strncmp(found, "udp", 3)) {
            acl_req->ace_type_spec = CLI_SPEC_VAL;
            acl_req->ace_type = VTSS_ACE_TYPE_IPV4;
            acl_req->ip_proto_spec = CLI_SPEC_VAL;
            acl_req->ip_proto = 17; /* UDP protocol */
        } else if (!strncmp(found, "icmp", 4)) {
            acl_req->ace_type_spec = CLI_SPEC_VAL;
            acl_req->ace_type = VTSS_ACE_TYPE_IPV4;
            acl_req->ip_proto_spec = CLI_SPEC_VAL;
            acl_req->ip_proto = 1; /* ICMP protocol */
        } else if (!strncmp(found, "switch", 6)) {
#if defined(VTSS_FEATURE_ACL_V2)
        } else if (!strncmp(found, "sip_smac", 8)) {
            acl_req->ace_type_spec = CLI_SPEC_VAL;
            acl_req->ace_type = VTSS_ACE_TYPE_IPV4;
            acl_req->sip_smac = 1;
#endif /* VTSS_FEATURE_ACL_V2 */
        } else if (!strncmp(found, "port", 4)) {
            req->uport = 1;
        }
    }
    return (found == NULL ? 1 : 0);
}

static int32_t cli_parm_parse_rate_limiter(char *cmd, char *cmd2,
                                           char *stx, char *cmd_org, cli_req_t *req)
{
    ulong         value = 0;
    int           error;
    acl_cli_req_t *acl_req = req->module_req;

    error = cli_parse_ulong(cmd, &value, 1, ACL_POLICER_NO_END);
    if (error) {
        error = cli_parse_disable(cmd);
    } else {
        acl_req->policer = value;
    }
    if (!error) {
        acl_req->policer_spec = CLI_SPEC_VAL;
    }

    return error;
}

#if defined(VTSS_FEATURE_ACL_V2)
static int32_t cli_parm_parse_filter_port_list(char *cmd, char *cmd2,
                                               char *stx, char *cmd_org, cli_req_t *req)
{
    int           error;
    acl_cli_req_t *acl_req = req->module_req;

    error = (cli_parse_all(cmd) && cli_parm_parse_list(cmd, acl_req->uport_filter_list, 1, VTSS_PORT_NO_END, 1));
    if (!error) {
        if (acl_req->permit_spec == CLI_SPEC_VAL && acl_req->permit != 2) {
            CPRINTF("The parameter of 'filter_port_list' can't be set when action is permitted or denied\n");
            return VTSS_UNSPECIFIED_ERROR;
        } else {
            acl_req->port_filter_spec = CLI_SPEC_VAL;
        }
    }

    return error;
}
#endif /* VTSS_FEATURE_ACL_V2 */

#if defined(VTSS_ARCH_CARACAL) && defined(VTSS_SW_OPTION_EVC)
static int32_t cli_parm_parse_evc_policer(char *cmd, char *cmd2,
                                          char *stx, char *cmd_org, cli_req_t *req)
{
    ulong         value = 0;
    int           error;
    acl_cli_req_t *acl_req = req->module_req;

    error = cli_parse_ulong(cmd, &value, ACL_EVC_POLICER_NO_START, ACL_EVC_POLICER_NO_END);
    if (error) {
        error = cli_parse_disable(cmd);
    } else {
        acl_req->evc_policer = value;
    }
    if (!error) {
        acl_req->evc_policer_spec = CLI_SPEC_VAL;
    }

    return error;
}
#endif /* VTSS_ARCH_CARACAL && VTSS_SW_OPTION_EVC */

static int32_t cli_parm_parse_port_copy(char *cmd, char *cmd2,
                                        char *stx, char *cmd_org, cli_req_t *req)
{
#ifndef VTSS_FEATURE_ACL_V2
    ulong         value = 0;
#endif
    int           error;
    acl_cli_req_t *acl_req = req->module_req;

#if defined(VTSS_FEATURE_ACL_V2)
    error = (cli_parm_parse_list(cmd, acl_req->uport_copy_list, 1, VTSS_PORT_NO_END, 1));
    if (error) {
        error = cli_parse_disable(cmd);
    } else {
        acl_req->port_copy_spec = CLI_SPEC_VAL;
    }
#else
    error = cli_parse_ulong(cmd, &value, 1, VTSS_PORT_NO_END);
    if (error) {
        error = cli_parse_disable(cmd);
    } else {
        acl_req->uport_copy = value;
    }
    if (!error) {
        acl_req->port_copy_spec = CLI_SPEC_VAL;
    }
#endif /* VTSS_FEATURE_ACL_V2 */

    return error;
}

static int32_t cli_parm_parse_front_port(char *cmd, char *cmd2,
                                         char *stx, char *cmd_org, cli_req_t *req)
{
    ulong value = 0;
    int   error;

    error = cli_parse_ulong(cmd, &value, 1, VTSS_PORT_NO_END);
    if (error) {
        req->uport = 0;
        error = cli_parse_all(cmd);
    } else {
        req->uport = value;
    }

    return error;
}

#if defined(VTSS_FEATURE_ACL_V2)
static int32_t cli_parm_parse_mirror(char *cmd, char *cmd2,
                                     char *stx, char *cmd_org, cli_req_t *req)
{
    int           error;
    acl_cli_req_t *acl_req = req->module_req;

    if (!(error = cli_parse_word(cmd, "enable"))) {
        acl_req->mirror = 1;
    } else if (!(error = cli_parse_word(cmd, "disable"))) {
        acl_req->mirror = 0;
    }
    if (!error) {
        acl_req->mirror_spec = CLI_SPEC_VAL;
    }

    return error;
}
#endif /* VTSS_FEATURE_ACL_V2 */

static int32_t cli_parm_parse_logging(char *cmd, char *cmd2,
                                      char *stx, char *cmd_org, cli_req_t *req)
{
    int           error;
    acl_cli_req_t *acl_req = req->module_req;

    if (!(error = cli_parse_word(cmd, "log"))) {
        acl_req->logging = 1;
    } else if (!(error = cli_parse_word(cmd, "log_disable"))) {
        acl_req->logging = 0;
    }
    if (!error) {
        acl_req->logging_spec = CLI_SPEC_VAL;
    }

    return error;
}

static int32_t cli_parm_parse_shutdown(char *cmd, char *cmd2,
                                       char *stx, char *cmd_org, cli_req_t *req)
{
    int           error;
    acl_cli_req_t *acl_req = req->module_req;

    if (!(error = cli_parse_word(cmd, "shut"))) {
        acl_req->shutdown = 1;
    } else if (!(error = cli_parse_word(cmd, "shut_disable"))) {
        acl_req->shutdown = 0;
    }
    if (!error) {
        acl_req->shutdown_spec = CLI_SPEC_VAL;
    }

    return error;
}

static int32_t cli_parm_parse_policy(char *cmd, char *cmd2,
                                     char *stx, char *cmd_org, cli_req_t *req)
{
    int           error;
    acl_cli_req_t *acl_req = req->module_req;

    error = cli_parse_ulong(cmd, &acl_req->policy_no, VTSS_ACL_POLICY_NO_START, VTSS_ACL_POLICY_NO_END - 1);
    if (!error) {
        acl_req->policy_no_spec = CLI_SPEC_VAL;
    }

    return error;
}

static int32_t cli_parm_parse_policy_bitmask(char *cmd, char *cmd2,
                                             char *stx, char *cmd_org, cli_req_t *req)
{
    acl_cli_req_t *acl_req = req->module_req;

    return (cli_parse_ulong(cmd, &acl_req->policy_bitmask, 0x0, ACL_POLICIES_BITMASK));
}

static int32_t cli_parm_parse_ratelim_list(char *cmd, char *cmd2,
                                           char *stx, char *cmd_org, cli_req_t *req)
{
    acl_cli_req_t *acl_req = req->module_req;

    return (cli_parm_parse_list(cmd, acl_req->policer_list, 1, ACL_POLICER_NO_END, 1));
}

#if defined(VTSS_FEATURE_ACL_V2)
static int32_t cli_parm_parse_rate_unit(char *cmd, char *cmd2,
                                        char *stx, char *cmd_org, cli_req_t *req)
{
    int           error = 0;
    acl_cli_req_t *acl_req = req->module_req;

    if (!strncmp(cmd, "pps", 3)) {
        acl_req->rate_unit = CLI_RATE_UNIT_PPS;
    } else if (!strncmp(cmd, "kbps", 4)) {
        acl_req->rate_unit = CLI_RATE_UNIT_KBPS;
    } else {
        error = 1;
    }
    return error;
}
#endif /* VTSS_FEATURE_ACL_V2 */

static int32_t cli_parm_parse_rate(char *cmd, char *cmd2,
                                   char *stx, char *cmd_org, cli_req_t *req)
{
    int           error;
    acl_cli_req_t *acl_req = req->module_req;

#if defined(ACL_PACKET_RATE_MAX) && defined(ACL_BIT_RATE_MAX)
    error = (mgmt_txt2rate_v2(cmd, &acl_req->rate, acl_req->rate_unit, ACL_PACKET_RATE_MAX, ACL_BIT_RATE_MAX, ACL_BIT_RATE_GRANULARITY) != VTSS_OK);
#elif defined(ACL_PACKET_RATE_IN_RANGE)
    error = mgmt_txt2ulong(cmd, &acl_req->rate, 0, ACL_PACKET_RATE_MAX);
#else
    ulong i = 0, max_rate = ACL_PACKET_RATE_MAX;
    while (max_rate) {
        max_rate = max_rate >> 1;
        i++;
    }
    error = (mgmt_txt2rate(cmd, &acl_req->rate, i - 1) != VTSS_OK);
#endif /* VTSS_FEATURE_ACL_V2 */
    if (!error) {
        acl_req->rate_spec = CLI_SPEC_VAL;
    }

    return error;
}

static int32_t cli_parm_parse_ace_id(char *cmd, char *cmd2,
                                     char *stx, char *cmd_org, cli_req_t *req)
{
    acl_cli_req_t *acl_req = req->module_req;

    return (cli_parse_ulong(cmd, &acl_req->ace_id, 1, ACE_ID_END));
}

static int32_t cli_parm_parse_aceid_next(char *cmd, char *cmd2,
                                         char *stx, char *cmd_org, cli_req_t *req)
{
    acl_cli_req_t *acl_req = req->module_req;

    return (cli_parse_ulong(cmd, &acl_req->ace_id_next, 1, ACE_ID_END));
}

static int32_t cli_parm_parse_dmac_type(char *cmd, char *cmd2,
                                        char *stx, char *cmd_org, cli_req_t *req)
{
    return (cli_parse_dmac_flag("any", CLI_ACE_FLAG_ANY, CLI_ACE_FLAG_ANY, cmd, req) &&
            cli_parse_dmac_flag("unicast", CLI_ACE_FLAG_0, CLI_ACE_FLAG_0, cmd, req) &&
            cli_parse_dmac_flag("multicast", CLI_ACE_FLAG_0, CLI_ACE_FLAG_1, cmd, req) &&
            cli_parse_dmac_flag("broadcast", CLI_ACE_FLAG_1, CLI_ACE_FLAG_1, cmd, req));
}

static int32_t cli_parm_parse_arp_opcode(char *cmd, char *cmd2,
                                         char *stx, char *cmd_org, cli_req_t *req)
{
    return (cli_parse_arp_flag("any", CLI_ACE_FLAG_ANY, CLI_ACE_FLAG_ANY, cmd, req) &&
            cli_parse_arp_flag("arp", CLI_ACE_FLAG_1, CLI_ACE_FLAG_0, cmd, req) &&
            cli_parse_arp_flag("rarp", CLI_ACE_FLAG_0, CLI_ACE_FLAG_0, cmd, req) &&
            cli_parse_arp_flag("other", CLI_ACE_FLAG_ANY, CLI_ACE_FLAG_1, cmd, req));
}

static int cli_parse_acl_flag(char *stx, acl_flag_t flag, char *cmd, char *cmd2, cli_req_t *req)
{
    int            error = 1;
    cli_ace_flag_t ace_flag;
    acl_cli_req_t  *acl_req = req->module_req;

    if (strstr(stx, cmd) == stx) {
        error = 0;
        req->parm_parsed = 2;
        ace_flag = CLI_ACE_FLAG_1;
        if (cmd2 == NULL) {
            req->parm_parsed = 1;
        } else if (strcmp(cmd2, "0") == 0) {
            ace_flag = CLI_ACE_FLAG_0;
        } else if (strcmp(cmd2, "1") == 0) {
            ace_flag = CLI_ACE_FLAG_1;
        } else if (cli_parse_wc(cmd2, NULL) == 0) {
            ace_flag = CLI_ACE_FLAG_ANY;
        } else {
            req->parm_parsed = 1;
        }
        acl_req->ace_flags[flag] = ace_flag;
    }
    return error;
}


static int32_t cli_parm_parse_arp_flags(char *cmd, char *cmd2,
                                        char *stx, char *cmd_org, cli_req_t *req)
{
    return (cli_parse_acl_flag("request", ACE_FLAG_ARP_REQ, cmd, cmd2, req) &&
            cli_parse_acl_flag("unknown", ACE_FLAG_ARP_UNKNOWN, cmd, cmd2, req) &&
            cli_parse_acl_flag("smac", ACE_FLAG_ARP_SMAC, cmd, cmd2, req) &&
            cli_parse_acl_flag("tmac", ACE_FLAG_ARP_DMAC, cmd, cmd2, req) &&
            cli_parse_acl_flag("len", ACE_FLAG_ARP_LEN, cmd, cmd2, req) &&
            cli_parse_acl_flag("ip", ACE_FLAG_ARP_IP, cmd, cmd2, req) &&
            cli_parse_acl_flag("ether", ACE_FLAG_ARP_ETHER, cmd, cmd2, req));
}

static int32_t cli_parm_parse_etype_any(char *cmd, char *cmd2,
                                        char *stx, char *cmd_org, cli_req_t *req)
{
    int           error;
    ulong         value;
    acl_cli_req_t *acl_req = req->module_req;

    error = cli_parse_ulong_wc(cmd, &value, 0x0600, 0xffff, &acl_req->etype_spec);
    if (! error && acl_req->etype_spec == CLI_SPEC_VAL) {
        if (value == 0x800 || value == 0x806 || value == 0x86DD) {
            CPRINTF("Ether Type 0x0800(IPv4), 0x0806(ARP) and 0x86DD(IPv6) are reserved\n");
            return VTSS_UNSPECIFIED_ERROR;
        }
        acl_req->etype = (vtss_etype_t) value;
    }
    return error;
}

static int32_t cli_parm_parse_smac_any(char *cmd, char *cmd2,
                                       char *stx, char *cmd_org, cli_req_t *req)
{
    acl_cli_req_t *acl_req = req->module_req;

    return (cli_parse_mac(cmd, acl_req->smac, &acl_req->smac_spec, 1));
}

static int32_t cli_parm_parse_dmac_any(char *cmd, char *cmd2,
                                       char *stx, char *cmd_org, cli_req_t *req)
{
    acl_cli_req_t *acl_req = req->module_req;

    return (cli_parse_mac(cmd, acl_req->dmac, &acl_req->dmac_spec, 1));
}

static int32_t cli_parm_parse_sip(char *cmd, char *cmd2,
                                  char *stx, char *cmd_org, cli_req_t *req)
{
    acl_cli_req_t *acl_req = req->module_req;

    return (cli_parse_ipv4(cmd, &acl_req->sip, &acl_req->sip_mask, &acl_req->sip_spec, 0));
}

static int32_t cli_parm_parse_sipv6(char *cmd, char *cmd2,
                                    char *stx, char *cmd_org, cli_req_t *req)
{
    acl_cli_req_t *acl_req = req->module_req;
    int error;

    error = cli_parse_wc(cmd, &acl_req->sipv6_spec);
    if (error) {
        error = cli_parse_ipv6(cmd, &acl_req->sipv6, &acl_req->sipv6_spec);
    }
    return error;
}

#if defined(ACL_IPV6_SUPPORTED)
static int32_t cli_parm_parse_sip_v6_mask(char *cmd, char *cmd2,
                                          char *stx, char *cmd_org, cli_req_t *req)
{
    int           error;
    ulong         value = 0;
    acl_cli_req_t *acl_req = req->module_req;

    error = cli_parse_ulong_wc(cmd, &value, 0, 0xffffffff, &acl_req->sip_v6_mask_spec);
    acl_req->sip_v6_mask = value;

    return error;
}
#endif /* ACL_IPV6_SUPPORTED */

static int32_t cli_parm_parse_dip(char *cmd, char *cmd2,
                                  char *stx, char *cmd_org, cli_req_t *req)
{
    acl_cli_req_t *acl_req = req->module_req;

    return (cli_parse_ipv4(cmd, &acl_req->dip, &acl_req->dip_mask, &acl_req->dip_spec, 0));
}

static int32_t cli_parm_parse_protocol(char *cmd, char *cmd2,
                                       char *stx, char *cmd_org, cli_req_t *req)
{
    int           error;
    ulong         value = 0;
    acl_cli_req_t *acl_req = req->module_req;

    error = cli_parse_ulong_wc(cmd, &value, 0, 0xff, &acl_req->ip_proto_spec);
    acl_req->ip_proto = value;
    return error;
}

static int32_t cli_parm_parse_ipv6_flags(char *cmd, char *cmd2,
                                         char *stx, char *cmd_org, cli_req_t *req)
{
    return (cli_parse_acl_flag("hop_limit", ACE_FLAG_IP_TTL, cmd, cmd2, req));
}

static int32_t cli_parm_parse_ip_flags(char *cmd, char *cmd2,
                                       char *stx, char *cmd_org, cli_req_t *req)
{
    return (cli_parse_acl_flag("ttl", ACE_FLAG_IP_TTL, cmd, cmd2, req) &&
            cli_parse_acl_flag("options", ACE_FLAG_IP_OPTIONS, cmd, cmd2, req) &&
            cli_parse_acl_flag("fragment", ACE_FLAG_IP_FRAGMENT, cmd, cmd2, req));
}

static int32_t cli_parm_parse_icmp_type(char *cmd, char *cmd2,
                                        char *stx, char *cmd_org, cli_req_t *req)
{
    int           error;
    ulong         value = 0;
    acl_cli_req_t *acl_req = req->module_req;

    error = cli_parse_ulong_wc(cmd, &value, 0, 0xff, &acl_req->icmp_type_spec);
    acl_req->icmp_type = value;

    return error;
}

static int32_t cli_parm_parse_icmp_code(char *cmd, char *cmd2,
                                        char *stx, char *cmd_org, cli_req_t *req)
{
    int           error;
    ulong         value = 0;
    acl_cli_req_t *acl_req = req->module_req;

    error = cli_parse_ulong_wc(cmd, &value, 0, 0xff, &acl_req->icmp_code_spec);
    acl_req->icmp_code = value;
    return error;
}

static int32_t cli_parm_parse_sport(char *cmd, char *cmd2,
                                    char *stx, char *cmd_org, cli_req_t *req)
{
    int            error;
    ulong          min = 1, max = VTSS_PORTS;
    acl_cli_req_t *acl_req = req->module_req;

    error = cli_parse_range(cmd, &min, &max, 0, 0xffff);
    if (error) {
        error = cli_parse_wc(cmd, &acl_req->sport_spec);
    } else {
        acl_req->sport_spec = CLI_SPEC_VAL;
        acl_req->sport_min = min;
        acl_req->sport_max = max;
    }
    return error;
}

static int32_t cli_parm_parse_dport(char *cmd, char *cmd2,
                                    char *stx, char *cmd_org, cli_req_t *req)
{
    int           error;
    ulong         min = 1, max = VTSS_PORTS;
    acl_cli_req_t *acl_req = req->module_req;

    error = cli_parse_range(cmd, &min, &max, 0, 0xffff);
    if (error) {
        error = cli_parse_wc(cmd, &acl_req->dport_spec);
    } else {
        acl_req->dport_spec = CLI_SPEC_VAL;
        acl_req->dport_min = min;
        acl_req->dport_max = max;
    }
    return error;
}

static int32_t cli_parm_parse_tcp_flags(char *cmd, char *cmd2,
                                        char *stx, char *cmd_org, cli_req_t *req)
{
    return (cli_parse_acl_flag("fin", ACE_FLAG_TCP_FIN, cmd, cmd2, req) &&
            cli_parse_acl_flag("syn", ACE_FLAG_TCP_SYN, cmd, cmd2, req) &&
            cli_parse_acl_flag("rst", ACE_FLAG_TCP_RST, cmd, cmd2, req) &&
            cli_parse_acl_flag("psh", ACE_FLAG_TCP_PSH, cmd, cmd2, req) &&
            cli_parse_acl_flag("ack", ACE_FLAG_TCP_ACK, cmd, cmd2, req) &&
            cli_parse_acl_flag("urg", ACE_FLAG_TCP_URG, cmd, cmd2, req));
}

static int32_t cli_parm_parse_usid_any (char *cmd, char *cmd2,
                                        char *stx, char *cmd_org, cli_req_t *req)
{
    int           error;
    ulong         value = 0;
    acl_cli_req_t *acl_req = req->module_req;

    error = cli_parse_ulong(cmd, &value, VTSS_USID_START, VTSS_USID_END - 1);
    if (!error) {
        acl_req->usid_ace_spec = CLI_SPEC_VAL;
        acl_req->usid_ace = value;
    } else {
        error = cli_parse_wc(cmd, &acl_req->usid_ace_spec);
    }
    return error;
}

#if defined(VTSS_FEATURE_ACL_V2)
static int32_t cli_parm_parse_tagged(char *cmd, char *cmd2,
                                     char *stx, char *cmd_org, cli_req_t *req)
{
    int           error;
    acl_cli_req_t *acl_req = req->module_req;

    if (!(error = cli_parse_word(cmd, "any"))) {
        acl_req->tagged = VTSS_ACE_BIT_ANY;
    } else if (!(error = cli_parse_word(cmd, "disable"))) {
        acl_req->tagged = VTSS_ACE_BIT_0;
    } else if (!(error = cli_parse_word(cmd, "enable"))) {
        acl_req->tagged = VTSS_ACE_BIT_1;
    }
    if (!error) {
        acl_req->tagged_spec = CLI_SPEC_VAL;
    }

    return error;
}
#endif /* VTSS_FEATURE_ACL_V2 */

static int32_t cli_parm_parse_vid_any (char *cmd, char *cmd2,
                                       char *stx, char *cmd_org, cli_req_t *req)
{
    int error = cli_parm_parse_vid(cmd, cmd2, stx, cmd_org, req);

    if (error) {
        return (cli_parse_wc(cmd, &req->vid_spec));
    }
    return error;
}

static int32_t cli_parm_parse_tag_prio_any (char *cmd, char *cmd2,
                                            char *stx, char *cmd_org, cli_req_t *req)
{
    int           error;
    ulong         value = 0;
    ulong         low, high, mask, min = 0, max = 7;
    acl_cli_req_t *acl_req = req->module_req;


    if ((error = cli_parse_range(cmd, &low, &high, min, max)) == 0) {
        error = VTSS_RC_ERROR;
        for (mask = 0; mask <= max; mask = (mask * 2 + 1)) {
            if ((low & ~mask) == (high & ~mask) &&  /* Upper bits match */
                (low & mask) == 0 &&                /* Lower bits of 'low' are zero */
                (high | mask) == high) {            /* Lower bits of 'high are one */
                error = 0;
                acl_req->tag_prio_spec = CLI_SPEC_VAL;
                acl_req->tag_prio = high;
                acl_req->tag_prio_bitmask = ~mask;
                acl_req->tag_prio_bitmask &= max;
                break;
            }
        }
    } else {
        error = cli_parse_ulong_wc(cmd, &value, 0, 7, &acl_req->tag_prio_spec);
        acl_req->tag_prio = value;
        acl_req->tag_prio_bitmask = 0x7;
    }

    return error;
}

static int32_t cli_parm_parse_tag_prio (char *cmd, char *cmd2,
                                        char *stx, char *cmd_org, cli_req_t *req)
{
    int           error;
    ulong         value = 0;
    acl_cli_req_t *acl_req = req->module_req;

    error = cli_parse_ulong(cmd, &value, 0, 7);
    acl_req->tag_prio = value;
    return error;
}

static int32_t cli_parm_parse_smac (char *cmd, char *cmd2,
                                    char *stx, char *cmd_org, cli_req_t *req)
{
    acl_cli_req_t *acl_req = req->module_req;

    return (cli_parse_mac(cmd, acl_req->smac, &acl_req->smac_spec, 0));
}

static int32_t cli_parm_parse_dmac (char *cmd, char *cmd2,
                                    char *stx, char *cmd_org, cli_req_t *req)
{
    acl_cli_req_t *acl_req = req->module_req;

    return (cli_parse_mac(cmd, acl_req->dmac, &acl_req->dmac_spec, 0));
}

static int32_t cli_parm_parse_acl_user(char *cmd, char *cmd2, char *stx,
                                       char *cmd_org, cli_req_t *req)
{
    char *found = cli_parse_find(cmd, stx);
    acl_cli_req_t *acl_req = req->module_req;

    if (found != NULL) {
        if (!strncmp(found, "combined", 8)) {
            acl_req->acl_user = -1;
            acl_req->acl_user_valid = 1;
        } else if (!strncmp(found, "static", 6)) {
            acl_req->acl_user = ACL_USER_STATIC;
            acl_req->acl_user_valid = 1;
        }
#ifdef VTSS_SW_OPTION_LOOP_PROTECT
        else if (!strncmp(found, "loop_protect", 12)) {
            acl_req->acl_user = ACL_USER_LOOP_PROTECT;
            acl_req->acl_user_valid = 1;
        }
#endif
#ifdef VTSS_SW_OPTION_ETH_LINK_OAM
        else if (!strncmp(found, "link_oam", 8)) {
            acl_req->acl_user = ACL_USER_LINK_OAM;
            acl_req->acl_user_valid = 1;
        }
#endif
#ifdef VTSS_SW_OPTION_DHCP_HELPER
        else if (!strncmp(found, "dhcp", 4)) {
            acl_req->acl_user = ACL_USER_DHCP;
            acl_req->acl_user_valid = 1;
        }
#endif
#if defined(VTSS_SW_OPTION_PTP) || defined(VTSS_SW_OPTION_PHY_1588_SIM)
        else if (!strncmp(found, "ptp", 3)) {
            acl_req->acl_user = ACL_USER_PTP;
            acl_req->acl_user_valid = 1;
        }
#endif
#ifdef VTSS_SW_OPTION_UPNP
        else if (!strncmp(found, "upnp", 4)) {
            acl_req->acl_user = ACL_USER_UPNP;
            acl_req->acl_user_valid = 1;
        }
#endif
#ifdef VTSS_SW_OPTION_ARP_INSPECTION
        else if (!strncmp(found, "arp_inspection", 14)) {
            acl_req->acl_user = ACL_USER_ARP_INSPECTION;
            acl_req->acl_user_valid = 1;
        }
#endif
#ifdef VTSS_SW_OPTION_MEP
        else if (!strncmp(found, "mep", 3)) {
            acl_req->acl_user = ACL_USER_MEP;
            acl_req->acl_user_valid = 1;
        }
#endif
#ifdef VTSS_SW_OPTION_IPMC
        else if (!strncmp(found, "ipmc", 4)) {
            acl_req->acl_user = ACL_USER_IPMC;
            acl_req->acl_user_valid = 1;
        }
#endif
#if defined(VTSS_ARCH_JAGUAR_1_CE_MAC) || defined(IP_MGMT_USING_MAC_ACL_ENTRIES)
        else if (!strncmp(found, "ip_mgmt", 7)) {
            acl_req->acl_user = ACL_USER_IP_MGMT;
            acl_req->acl_user_valid = 1;
        }
#endif
#ifdef VTSS_SW_OPTION_IP_SOURCE_GUARD
        else if (!strncmp(found, "ip_source_guard", 15)) {
            acl_req->acl_user = ACL_USER_IP_SOURCE_GUARD;
            acl_req->acl_user_valid = 1;
        }
#endif
#if defined(VTSS_ARCH_JAGUAR_1_CE_MAC) || defined(IP_MGMT_USING_MAC_ACL_ENTRIES)
        else if (!strncmp(found, "ip_mgmt", 7)) {
            acl_req->acl_user = ACL_USER_IP_MGMT;
            acl_req->acl_user_valid = 1;
        }
#endif
        else if (!strncmp(found, "conflict", 8)) {
            acl_req->acl_user = -2;
            acl_req->acl_user_valid = 1;
        }
    }

    return (found == NULL ? 1 : 0);
}

static void acl_cli_req_default_set ( cli_req_t *req )
{
    acl_cli_req_t *acl_req = req->module_req;
    (void) cli_parm_parse_list(NULL, acl_req->policer_list, ACL_POLICER_NO_START, ACL_POLICER_NO_END, 1);
#if defined(VTSS_FEATURE_ACL_V2)
    (void) cli_parm_parse_list(NULL, acl_req->uport_filter_list, 1, VTSS_PORT_NO_END, 1);
#endif /* VTSS_FEATURE_ACL_V2 */
}

/****************************************************************************/
/*  Parameter table                                                         */
/****************************************************************************/

static cli_parm_t acl_parm_table[] = {
#if defined(VTSS_ARCH_SERVAL)
    {
        "<lookup>",
        "Second lookup: enable|disable",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_lookup,
        NULL,
    },
#endif /* VTSS_ARCH_SERVAL */
#if defined(VTSS_FEATURE_ACL_V2)
    {
        "permit",
        "ACE Action Type keyword",
        CLI_PARM_FLAG_NONE,
        cli_acl_parse_keyword,
        NULL,
    },
    {
        "deny",
        "ACE Action Type keyword",
        CLI_PARM_FLAG_NONE,
        cli_acl_parse_keyword,
        NULL,
    },
    {
        "filter",
        "ACE Action Type keyword",
        CLI_PARM_FLAG_NONE,
        cli_acl_parse_keyword,
        NULL,
    },
    {
        "<filter_port_list>",
        "Filter Port list or 'all'",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_filter_port_list,
        NULL,
    },
#endif /* VTSS_FEATURE_ACL_V2 */
    {
        "permit|deny",
        "permit          : Permit forwarding (default)\n"
        "deny            : Deny forwarding",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_acl_parse_keyword,
        NULL,
    },
    {
        "<rate_limiter>",
        "Rate limiter number (1-15) or 'disable'",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_rate_limiter,
        NULL,
    },
#if defined(VTSS_ARCH_CARACAL) && defined(VTSS_SW_OPTION_EVC)
    {
        "<evc_policer>",
        "EVC policer number ("vtss_xstr(ACL_EVC_POLICER_NO_START)"-"vtss_xstr(ACL_EVC_POLICER_NO_END)") or 'disable'",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_evc_policer,
        NULL,
    },
#endif /* VTSS_ARCH_CARACAL && VTSS_SW_OPTION_EVC */
    {
#if defined(VTSS_ARCH_LUTON28)
        "<port_copy>",
#else
        "<port_redirect>",
#endif
#if defined(VTSS_FEATURE_ACL_V2)
        "Port list for copy of frames or 'disable'. Notice: If the value is a specific port number, it can't be set when action is permitted or filtered",
#elif defined(VTSS_ARCH_JAGUAR_1)
        "Port number for redirect of frames or 'disable'. Notice: If the value is a specific port number, it can't be set when action is permitted",
#else
        "Port number for redirect of frames or 'disable'",
#endif /* VTSS_FEATURE_ACL_V2 */
        CLI_PARM_FLAG_SET,
        cli_parm_parse_port_copy,
        NULL,
    },
    {
        "<logging>",
        "System logging of frames: log|log_disable",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_logging,
        NULL,
    },
    {
        "<shutdown>",
        "Shut down ingress port: shut|shut_disable",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_shutdown,
        NULL,
    },
    {
        "<policy>",
        "Policy number ("vtss_xstr(VTSS_ACL_POLICY_NO_START)"-"vtss_xstr(ACL_POLICIES)")",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_policy,
        NULL,
    },
    {
        "<policy_bitmask>",
        "Policy number bitmask (0x0-"vtss_xstr(ACL_POLICIES_BITMASK)")",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_policy_bitmask,
        NULL,
    },
    {
        "<rate_limiter_list>",
#if defined(VTSS_ARCH_LUTON28)
        "Rate limiter list (1-15), default: All rate limiters",
#else
        "Rate limiter list (1-"vtss_xstr(VTSS_ACL_POLICERS)"), default: All rate limiters",
#endif /* VTSS_ARCH_LUTON28 */
        CLI_PARM_FLAG_NONE,
        cli_parm_parse_ratelim_list,
        NULL,
    },

#if defined(VTSS_FEATURE_ACL_V2)
    {
        "<rate_unit>",
        "IP flags: pps|kbps, default: pps",
        CLI_PARM_FLAG_DUAL,
        cli_parm_parse_rate_unit,
        NULL,
    },
    {
        "<mirror>",
        "Mirror of frames: enable|disable",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_mirror,
        NULL,
    },
#endif /* VTSS_FEATURE_ACL_V2 */
    {
        "<rate>",
#if defined(ACL_PACKET_RATE_MAX) && defined(ACL_BIT_RATE_MAX)
        "Rate in pps (0-"vtss_xstr(ACL_PACKET_RATE_MAX)") or kbps (0, "vtss_xstr(ACL_BIT_RATE_GRANULARITY)", 2*"vtss_xstr(ACL_BIT_RATE_GRANULARITY)", 3*"vtss_xstr(ACL_BIT_RATE_GRANULARITY)", ..., "vtss_xstr(ACL_BIT_RATE_MAX)")",
#elif defined(ACL_PACKET_RATE_IN_RANGE)
        "Rate in pps (0-"vtss_xstr(ACL_PACKET_RATE_MAX)")",
#else
        "Rate in pps (1, 2, 4, ..., 512, 1k, 2k, 4k, ..., "vtss_xstr(ACL_PACKET_RATE_MAX)"k)",
#endif /* VTSS_FEATURE_ACL_V2 */
        CLI_PARM_FLAG_SET,
        cli_parm_parse_rate,
        NULL,
    },
    {
        "<ace_id>",
        "ACE ID (1-"vtss_xstr(ACE_MAX)"), default: Next available ID",
        CLI_PARM_FLAG_NONE,
        cli_parm_parse_ace_id,
        acl_cli_cmd_debug_ipv6,
    },
    {
        "<sid>",
        acl_cli_sid_help_str,
        CLI_PARM_FLAG_SET,
        cli_parm_parse_usid_any,
        acl_cli_cmd_debug_ipv6,
    },
    {
        "<vid>",
        "VLAN ID (1-4095) or 'any'",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_vid_any,
        acl_cli_cmd_debug_ipv6,
    },
    {
        "port",
        "Port ACE keyword",
        CLI_PARM_FLAG_NONE,
        cli_acl_parse_keyword,
        acl_cli_cmd_debug_ipv6,
    },
    {
        "<tag_prio>",
        "VLAN tag priority (0-7), range(0-1/2-3/4-5/6-7/0-3/4-7) or 'any'",
        CLI_PARM_FLAG_NONE,
        cli_parm_parse_tag_prio_any,
        acl_cli_cmd_debug_ipv6,
    },
    {
        "<ipv6_flags>",
        "IP flags: hop_limit [0|1|any]",
        CLI_PARM_FLAG_DUAL,
        cli_parm_parse_ipv6_flags,
        NULL,
    },
#if VTSS_SWITCH_STACKABLE
    {
        "<ace_id>",
        "ACE ID (1-"vtss_xstr(VTSS_ISID_CNT)"*"vtss_xstr(ACE_MAX)"), default: Next available ID",
        CLI_PARM_FLAG_NONE,
        cli_parm_parse_ace_id,
        acl_cli_cmd_add,
    },
    {
        "<ace_id>",
        "ACE ID (1-"vtss_xstr(VTSS_ISID_CNT)"*"vtss_xstr(ACE_MAX)")",
        CLI_PARM_FLAG_NONE,
        cli_parm_parse_ace_id,
        NULL,
    },
    {
        "<ace_id_next>",
        "Next ACE ID (1-"vtss_xstr(VTSS_ISID_CNT)"*"vtss_xstr(ACE_MAX)"), default: Add ACE last",
        CLI_PARM_FLAG_NONE,
        cli_parm_parse_aceid_next,
        NULL,
    },
#else
    {
        "<ace_id>",
        "ACE ID (1-"vtss_xstr(ACE_MAX)"), default: Next available ID",
        CLI_PARM_FLAG_NONE,
        cli_parm_parse_ace_id,
        acl_cli_cmd_add,
    },
    {
        "<ace_id>",
        "ACE ID (1-"vtss_xstr(ACE_MAX)")",
        CLI_PARM_FLAG_NONE,
        cli_parm_parse_ace_id,
        NULL,
    },
    {
        "<ace_id_next>",
        "Next ACE ID (1-"vtss_xstr(ACE_MAX)"), default: Add ACE last",
        CLI_PARM_FLAG_NONE,
        cli_parm_parse_aceid_next,
        NULL,
    },
#endif /* VTSS_SWITCH_STACKABLE */
    {
        "switch",
        "Switch ACE keyword",
        CLI_PARM_FLAG_NONE,
        cli_acl_parse_keyword,
        NULL,
    },
    {
        "<port>",
        "Port number or 'all'",
        CLI_PARM_FLAG_NONE,
        cli_parm_parse_front_port,
        NULL,
    },
    {
        "policy",
        "Policy ACE keyword",
        CLI_PARM_FLAG_NONE,
        cli_acl_parse_keyword,
        NULL,
    },
    {
        "<dmac_type>",
        "DMAC type: any|unicast|multicast|broadcast",
        CLI_PARM_FLAG_NONE,
        cli_parm_parse_dmac_type,
        NULL,
    },
    {
        "<etype>",
        "Ethernet Type: 0x600 - 0xFFFF or 'any' but excluding\n"
        "                  0x800(IPv4) 0x806(ARP) and 0x86DD(IPv6)",
        CLI_PARM_FLAG_NONE,
        cli_parm_parse_etype_any,
        acl_cli_cmd_add,
    },
    {
        "etype",
        "Ethernet Type keyword",
        CLI_PARM_FLAG_NONE,
        cli_acl_parse_keyword,
        NULL,
    },
    {
        "<smac>",
        "Source MAC address ('xx-xx-xx-xx-xx-xx' or 'xx.xx.xx.xx.xx.xx' or 'xxxxxxxxxxxx', x is a hexadecimal digit) or 'any'",
        CLI_PARM_FLAG_NONE,
        cli_parm_parse_smac_any,
        acl_cli_cmd_add,
    },
    {
        "<dmac>",
        "Destination MAC address ('xx-xx-xx-xx-xx-xx' or 'xx.xx.xx.xx.xx.xx' or 'xxxxxxxxxxxx', x is a hexadecimal digit) or 'any'",
        CLI_PARM_FLAG_NONE,
        cli_parm_parse_dmac_any,
        acl_cli_cmd_add,
    },
    {
        "arp",
        "ARP keyword",
        CLI_PARM_FLAG_NONE,
        cli_acl_parse_keyword,
        NULL,
    },
    {
        "<sip>",
        "Source IP address (a.b.c.d/n) or 'any'",
        CLI_PARM_FLAG_NONE,
        cli_parm_parse_sip,
        acl_cli_cmd_add,
    },
    {
        "<dip>",
        "Destination IP address (a.b.c.d/n) or 'any'",
        CLI_PARM_FLAG_NONE,
        cli_parm_parse_dip,
        acl_cli_cmd_add,
    },
    {
        "<arp_opcode>",
        "ARP operation code: any|arp|rarp|other",
        CLI_PARM_FLAG_NONE,
        cli_parm_parse_arp_opcode,
        NULL,
    },
    {
        "<arp_flags>",
        "ARP flags: request|smac|tmac|len|ip|ether [0|1|any]",
        CLI_PARM_FLAG_DUAL,
        cli_parm_parse_arp_flags,
        NULL,
    },
    {
        "<protocol>",
        "IP protocol number (0-255) or 'any'",
        CLI_PARM_FLAG_NONE,
        cli_parm_parse_protocol,
        NULL,
    },
    {
        "<ip_flags>",
        "IP flags: ttl|options|fragment [0|1|any]",
        CLI_PARM_FLAG_DUAL,
        cli_parm_parse_ip_flags,
        NULL,
    },
    {
        "<icmp_type>",
        "ICMP type number (0-255) or 'any'",
        CLI_PARM_FLAG_NONE,
        cli_parm_parse_icmp_type,
        NULL,
    },
    {
        "<icmp_code>",
        "ICMP code number (0-255) or 'any'",
        CLI_PARM_FLAG_NONE,
        cli_parm_parse_icmp_code,
        NULL,
    },
    {
        "<sport>",
        "Source UDP/TCP port range (0-65535) or 'any'",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_sport,
        NULL,
    },
    {
        "<dport>",
        "Destination UDP/TCP port range (0-65535) or 'any'",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_dport,
        NULL,
    },
    {
        "<tcp_flags>",
        "TCP flags: fin|syn|rst|psh|ack|urg [0|1|any]",
        CLI_PARM_FLAG_DUAL,
        cli_parm_parse_tcp_flags,
        NULL,
    },
    {
        "<sid>",
        acl_cli_sid_help_str,
        CLI_PARM_FLAG_SET,
        cli_parm_parse_usid_any,
        acl_cli_cmd_add,
    },
    {
        "<vid>",
        "VLAN ID (1-4095) or 'any'",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_vid_any,
        acl_cli_cmd_add,
    },
    {
        "port",
        "Port ACE keyword",
        CLI_PARM_FLAG_NONE,
        cli_acl_parse_keyword,
        acl_cli_cmd_add,
    },
#if defined(VTSS_FEATURE_ACL_V2)
    {
        "<tagged>",
        "Tagged of frames: any|enable|disable",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_tagged,
        NULL,
    },
    {
        "sip_smac",
        "IP keyword",
        CLI_PARM_FLAG_NONE,
        cli_acl_parse_keyword,
        NULL,
    },
    {
        "<sip>",
        "Source IP address (a.b.c.d/n) or 'any'",
        CLI_PARM_FLAG_NONE,
        cli_parm_parse_sip,
        NULL,
    },
#endif /* VTSS_FEATURE_ACL_V2 */
    {
        "<tag_prio>",
        "VLAN tag priority (0-7) or 'any'",
        CLI_PARM_FLAG_NONE,
        cli_parm_parse_tag_prio_any,
        acl_cli_cmd_add,
    },
    {
        "<tag_prio>",
        "VLAN tag priority (0-7)",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_tag_prio,
        NULL,
    },
    {
        "<smac>",
        "Source MAC address ('xx-xx-xx-xx-xx-xx' or 'xx.xx.xx.xx.xx.xx' or 'xxxxxxxxxxxx', x is a hexadecimal digit), default: Board MAC address",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_smac,
        NULL,
    },
    {
        "<dmac>",
        "Destination MAC address ('xx-xx-xx-xx-xx-xx' or 'xx.xx.xx.xx.xx.xx' or 'xxxxxxxxxxxx', x is a hexadecimal digit), default: Broadcast MAC address",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_dmac,
        NULL,
    },
    {
        "<next_header>",
        "IPv6 next header (0-255) or 'any'",
        CLI_PARM_FLAG_NONE,
        cli_parm_parse_protocol,
        NULL,
    },
    {
        "<sip_v6>",
        "IPv6 source address or 'any'",
        CLI_PARM_FLAG_NONE,
        cli_parm_parse_sipv6,
        NULL,
    },
#if defined(ACL_IPV6_SUPPORTED)
    {
        "<sip_v6_mask>",
        "IPv6 source address Mask (0x0-0xFFFFFFFF) or 'any'",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_sip_v6_mask,
        NULL,
    },
#endif /* ACL_IPV6_SUPPORTED */
    {
        "ipv6_std",
        "IPv6 keyword",
        CLI_PARM_FLAG_NONE,
        cli_acl_parse_keyword,
        NULL,
    },
    {
        "ipv6_icmp",
        "IPv6 keyword",
        CLI_PARM_FLAG_NONE,
        cli_acl_parse_keyword,
        NULL,
    },
    {
        "ipv6_udp",
        "IPv6 keyword",
        CLI_PARM_FLAG_NONE,
        cli_acl_parse_keyword,
        NULL,
    },
    {
        "ipv6_tcp",
        "IPv6 keyword",
        CLI_PARM_FLAG_NONE,
        cli_acl_parse_keyword,
        NULL,
    },
    {
        "ip",
        "IP keyword",
        CLI_PARM_FLAG_NONE,
        cli_acl_parse_keyword,
        NULL,
    },
    {
        "icmp",
        "ICMP keyword",
        CLI_PARM_FLAG_NONE,
        cli_acl_parse_keyword,
        NULL,
    },
    {
        "udp",
        "UDP keyword",
        CLI_PARM_FLAG_NONE,
        cli_acl_parse_keyword,
        NULL,
    },
    {
        "tcp",
        "TCP keyword",
        CLI_PARM_FLAG_NONE,
        cli_acl_parse_keyword,
        NULL,
    },
    {
        "static"
#ifdef VTSS_SW_OPTION_ETH_LINK_OAM
        "|link_oam"
#endif
#ifdef VTSS_SW_OPTION_LOOP_PROTECT
        "|loop_protect"
#endif
#ifdef VTSS_SW_OPTION_DHCP_HELPER
        "|dhcp"
#endif
#if defined(VTSS_SW_OPTION_PTP) || defined(VTSS_SW_OPTION_PHY_1588_SIM)
        "|ptp"
#endif
#ifdef VTSS_SW_OPTION_UPNP
        "|upnp"
#endif
#ifdef VTSS_SW_OPTION_ARP_INSPECTION
        "|arp_inspection"
#endif
#ifdef VTSS_SW_OPTION_MEP
        "|mep"
#endif
#ifdef VTSS_SW_OPTION_IPMC
        "|ipmc"
#endif
#ifdef VTSS_SW_OPTION_IP_SOURCE_GUARD
        "|ip_source_guard"
#endif
#if defined(VTSS_ARCH_JAGUAR_1_CE_MAC) || defined(IP_MGMT_USING_MAC_ACL_ENTRIES)
        "|ip_mgmt"
#endif
        ,
        "static          : Show detailed statically configured configuration\n"
#ifdef VTSS_SW_OPTION_ETH_LINK_OAM
        "link_oam        : Show detailed Link OAM configuration\n"
#endif
#ifdef VTSS_SW_OPTION_LOOP_PROTECT
        "loop_protect    : Show the deatil configuration by Loop Protect\n"
#endif
#ifdef VTSS_SW_OPTION_DHCP_HELPER
        "dhcp            : Show detailed DHCP configuration\n"
#endif
#if defined(VTSS_SW_OPTION_PTP) || defined(VTSS_SW_OPTION_PHY_1588_SIM)
        "ptp             : Show detailed PTP configuration\n"
#endif
#ifdef VTSS_SW_OPTION_UPNP
        "upnp            : Show detailed UPnP configuration\n"
#endif
#ifdef VTSS_SW_OPTION_ARP_INSPECTION
        "arp_inspection  : Show detailed ARP Inspection configuration\n"
#endif
#ifdef VTSS_SW_OPTION_MEP
        "mep             : Show detailed MEP configuration\n"
#endif
#ifdef VTSS_SW_OPTION_IPMC
        "ipmc            : Show detailed IPMC configuration\n"
#endif
#ifdef VTSS_SW_OPTION_IP_SOURCE_GUARD
        "ip_source_guard : Show deatiled IP Source Guard configuration\n"
#endif
#if defined(VTSS_ARCH_JAGUAR_1_CE_MAC) || defined(IP_MGMT_USING_MAC_ACL_ENTRIES)
        "ip_mgmt         : Show detailed IP Management configuration\n"
#endif
        ,
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_acl_user,
        NULL
    },
    {
        "combined|static"
#ifdef VTSS_SW_OPTION_ETH_LINK_OAM
        "|link_oam"
#endif
#ifdef VTSS_SW_OPTION_LOOP_PROTECT
        "|loop_protect"
#endif
#ifdef VTSS_SW_OPTION_DHCP_HELPER
        "|dhcp"
#endif
#if defined(VTSS_SW_OPTION_PTP) || defined(VTSS_SW_OPTION_PHY_1588_SIM)
        "|ptp"
#endif
#ifdef VTSS_SW_OPTION_UPNP
        "|upnp"
#endif
#ifdef VTSS_SW_OPTION_ARP_INSPECTION
        "|arp_inspection"
#endif
#ifdef VTSS_SW_OPTION_MEP
        "|mep"
#endif
#ifdef VTSS_SW_OPTION_IPMC
        "|ipmc"
#endif
#ifdef VTSS_SW_OPTION_IP_SOURCE_GUARD
        "|ip_source_guard"
#endif
#if defined(VTSS_ARCH_JAGUAR_1_CE_MAC) || defined(IP_MGMT_USING_MAC_ACL_ENTRIES)
        "|ip_mgmt"
#endif
        "|conflicts",
        "combined        : Show combined status\n"
        "static          : Show static user configured status\n"
#ifdef VTSS_SW_OPTION_ETH_LINK_OAM
        "link_oam        : Show Link OAM status\n"
#endif
#ifdef VTSS_SW_OPTION_LOOP_PROTECT
        "loop_protect    : Shows the status by Loop Protect\n"
#endif
#ifdef VTSS_SW_OPTION_DHCP_HELPER
        "dhcp            : Show DHCP status\n"
#endif
#if defined(VTSS_SW_OPTION_PTP) || defined(VTSS_SW_OPTION_PHY_1588_SIM)
        "ptp             : Show PTP status\n"
#endif
#ifdef VTSS_SW_OPTION_UPNP
        "upnp            : Show UPnP status\n"
#endif
#ifdef VTSS_SW_OPTION_ARP_INSPECTION
        "arp_inspection  : Show ARP Inspection status\n"
#endif
#ifdef VTSS_SW_OPTION_MEP
        "mep             : Show MEP status\n"
#endif
#ifdef VTSS_SW_OPTION_IPMC
        "ipmc            : Show IPMC status\n"
#endif
#ifdef VTSS_SW_OPTION_IP_SOURCE_GUARD
        "ip_source_guard : Show IP Source Guard status\n"
#endif
#if defined(VTSS_ARCH_JAGUAR_1_CE_MAC) || defined(IP_MGMT_USING_MAC_ACL_ENTRIES)
        "ip_mgmt         : Show IP Management status\n"
#endif
        "conflicts       : Show conflict status\n"
        "(default        : Show combined status)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_acl_user,
        NULL
    },
    {
        "enable|disable",
        "ACL port state",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        NULL,
    },
    {
        NULL,
        NULL,
        0,
        0,
        NULL
    },
};

/****************************************************************************/
/*  Command table                                                           */
/****************************************************************************/

enum {
    PRIO_ACL_CONF,
    PRIO_ACL_ACTION,
    PRIO_ACL_POLICY,
    PRIO_ACL_RATE,
    PRIO_ACL_ADD,
    PRIO_ACL_DEL,
    PRIO_ACL_LOOKUP,
    PRIO_ACL_CLEAR,
    PRIO_ACL_STATUS,
    PRIO_ACL_PORT_STATE,
    PRIO_ACL_DEBUG_LOOKUP = CLI_CMD_SORT_KEY_DEFAULT,
    PRIO_ACL_DEBUG_DEL    = CLI_CMD_SORT_KEY_DEFAULT,
};

/* Command table entries */
cli_cmd_tab_entry (
    VTSS_CLI_GRP_SEC_NETWORK_PATH "ACL Configuration [<port_list>]",
    NULL,
    "Show ACL Configuration",
    PRIO_ACL_CONF,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_SECURITY,
    cli_cmd_acl_config,
    acl_cli_req_default_set,
    acl_parm_table,
    CLI_CMD_FLAG_SYS_CONF
);

CLI_CMD_TAB_ENTRY_DECL(cli_cmd_acl_action) = {
    VTSS_CLI_GRP_SEC_NETWORK_PATH "ACL Action [<port_list>]",
    VTSS_CLI_GRP_SEC_NETWORK_PATH "ACL Action [<port_list>] [permit|deny] [<rate_limiter>]\n"
#if defined(VTSS_FEATURE_ACL_V2)
#if defined(VTSS_ARCH_CARACAL) && defined(VTSS_SW_OPTION_EVC)
    "        [<evc_policer>] [<port_redirect>] [<mirror>] [<logging>] [<shutdown>]",
#else
#if defined(VTSS_ARCH_LUTON28)
    "        [<port_copy>] [<mirror>] [<logging>] [<shutdown>]",
#else
    "        [<port_redirect>] [<mirror>] [<logging>] [<shutdown>]",
#endif
#endif /* VTSS_ARCH_CARACAL && VTSS_SW_OPTION_EVC */
#else
#if defined(VTSS_ARCH_CARACAL) && defined(VTSS_SW_OPTION_EVC)
    "        [<evc_policer>] [<port_redirect>] [<logging>] [<shutdown>]",
#else
#if defined(VTSS_ARCH_LUTON28)
    "        [<port_copy>] [<logging>] [<shutdown>]",
#else
    "        [<port_redirect>] [<logging>] [<shutdown>]",
#endif
#endif /* VTSS_ARCH_CARACAL && VTSS_SW_OPTION_EVC */
#endif /* VTSS_FEATURE_ACL_V2 */
    "Set or show the ACL port default action",
    PRIO_ACL_ACTION,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SECURITY,
    cli_cmd_acl_action,
    acl_cli_req_default_set,
    acl_parm_table,
    CLI_CMD_FLAG_NONE
};

cli_cmd_tab_entry (
    VTSS_CLI_GRP_SEC_NETWORK_PATH "ACL Policy [<port_list>]",
    VTSS_CLI_GRP_SEC_NETWORK_PATH "ACL Policy [<port_list>] [<policy>]",
    "Set or show the ACL port policy",
    PRIO_ACL_POLICY,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SECURITY,
    cli_cmd_acl_policy,
    acl_cli_req_default_set,
    acl_parm_table,
    CLI_CMD_FLAG_NONE
);

#if defined(ACL_PACKET_RATE_MAX) && defined(ACL_BIT_RATE_MAX)
cli_cmd_tab_entry (
    VTSS_CLI_GRP_SEC_NETWORK_PATH "ACL Rate [<rate_limiter_list>]",
    VTSS_CLI_GRP_SEC_NETWORK_PATH "ACL Rate [<rate_limiter_list>] [<rate_unit>] [<rate>]",
    "Set or show the ACL rate limiter",
    PRIO_ACL_RATE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SECURITY,
    cli_cmd_acl_rate,
    acl_cli_req_default_set,
    acl_parm_table,
    CLI_CMD_FLAG_NONE
);
#else
cli_cmd_tab_entry (
    VTSS_CLI_GRP_SEC_NETWORK_PATH "ACL Rate [<rate_limiter_list>]",
    VTSS_CLI_GRP_SEC_NETWORK_PATH "ACL Rate [<rate_limiter_list>] [<rate>]",
    "Set or show the ACL rate limiter",
    PRIO_ACL_RATE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SECURITY,
    cli_cmd_acl_rate,
    acl_cli_req_default_set,
    acl_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif /* ACL_PACKET_RATE_MAX && ACL_BIT_RATE_MAX */

cli_cmd_tab_entry (
    NULL,
    VTSS_CLI_GRP_SEC_NETWORK_PATH "ACL Delete <ace_id>",
    "Delete ACE",
    PRIO_ACL_DEL,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SECURITY,
    acl_cli_cmd_del,
    acl_cli_req_default_set,
    acl_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    VTSS_CLI_GRP_SEC_NETWORK_PATH "ACL Lookup [<ace_id>]",
    NULL,
    "Show ACE, default: All ACEs",
    PRIO_ACL_LOOKUP,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_SECURITY,
    acl_cli_cmd_lookup,
    acl_cli_req_default_set,
    acl_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    NULL,
    VTSS_CLI_GRP_SEC_NETWORK_PATH "ACL Clear",
    "Clear all ACL counters",
    PRIO_ACL_CLEAR,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SECURITY,
    cli_mgmt_counters_clear,
    acl_cli_req_default_set,
    acl_parm_table,
    CLI_CMD_FLAG_NONE
);

CLI_CMD_TAB_ENTRY_DECL(acl_cli_cmd_debug_ipv6) = {
    NULL,
    "Debug ACL Add [<ace_id>] [<ace_id_next>]\n"
#if defined(VTSS_ARCH_SERVAL)
    "        [<lookup>]\n"
#endif /* VTSS_ARCH_SERVAL */
#if defined(VTSS_FEATURE_ACL_V2)
    "        [(port <port_list>)] "
#else
    "        [(port <port>)] "
#endif /* VTSS_ARCH_SERVAL */
    "[(policy <policy> <policy_bitmask>)]\n"
#if VTSS_SWITCH_STACKABLE
    "        [<sid>]\n"
#endif /* VTSS_SWITCH_STACKABLE */
#if defined(VTSS_FEATURE_ACL_V2)
    "        [<tagged>] [<vid>] [<tag_prio>] [<dmac_type>]\n"
    "        [(ipv6_std [<next_header>] [<sip_v6>] [<ipv6_flags>]) |\n"
    "         (ipv6_icmp [<sip_v6>] [<icmp_type>] [<icmp_code>] [<ipv6_flags>]) |\n"
    "         (ipv6_udp [<sip_v6>] [<sport>] [<dport>] [<ipv6_flags>]) |\n"
    "         (ipv6_tcp [<sip_v6>] [<sport>] [<dport>] [<ipv6_flags>] [<tcp_flags>]) |\n"
    "         (sip_smac <sip> <smac>)]\n"
    "        [(permit) | (deny) | (filter <filter_port_list>)] [<rate_limiter>] "
#else
    "        [<vid>] [<tag_prio>] [<dmac_type>]\n"
    "        [(ipv6_std [<next_header>] [<sip_v6>] [<sip_v6_mask>]) |\n"
    "         (ipv6_icmp [<sip_v6>] [<icmp_type>] [<icmp_code>] [<ipv6_flags>]) |\n"
    "         (ipv6_udp [<sip_v6>] [<sport>] [<dport>] [<ipv6_flags>]) |\n"
    "         (ipv6_tcp [<sip_v6>] [<sport>] [<dport>] [<ipv6_flags>] [<tcp_flags>])]\n"
    "        [permit|deny] [<rate_limiter>] "
#endif /* VTSS_ARCH_SERVAL */
#if defined(VTSS_ARCH_CARACAL) && defined(VTSS_SW_OPTION_EVC)
    "[<evc_policer>] "
#endif /* VTSS_ARCH_CARACAL && VTSS_SW_OPTION_EVC */
#if defined(VTSS_ARCH_LUTON28)
    "[<port_copy>] "
#else
    "[<port_redirect>] "
#endif /* VTSS_ARCH_LUTON28 */
#if defined(VTSS_FEATURE_ACL_V2)
    "[<mirror>] "
#endif /* VTSS_ARCH_LUTON28 */
    "[<logging>] [<shutdown>]",
    "Add or modify Access Control Entry (ACE).\n\n"
    "If the ACE ID parameter <ace_id> is specified and an entry with this ACE ID\n"
    "already exists, the ACE will be modified. Otherwise, a new ACE will be added.\n"
    "If the ACE ID is not specified, the next available ACE ID will be used.\n\n"
    "If the next ACE ID parameter <ace_id_next> is specified, the ACE will be placed\n"
    "before this ACE in the list. If the next ACE ID is not specified, the ACE\n"
    "will be placed last in the list.\n\n"
    "If the Switch keyword is used, the rule applies to all ports.\n"
    "If the Port keyword is used, the rule applies to the specified port only.\n"
    "If the Policy keyword is used, the rule applies to all ports configured\n"
    "with the specified policy. The default is that the rule applies to all ports"
#if VTSS_SWITCH_STACKABLE
    ".\n\nNotice: the ACE won't apply to any stacking or none existing port"
#endif /* VTSS_SWITCH_STACKABLE */
    ,
    CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SECURITY,
    acl_cli_cmd_debug_ipv6,
    acl_cli_req_default_set,
    acl_parm_table,
    CLI_CMD_FLAG_NONE
};

CLI_CMD_TAB_ENTRY_DECL(acl_cli_cmd_add) = {
    NULL,
    VTSS_CLI_GRP_SEC_NETWORK_PATH "ACL Add [<ace_id>] [<ace_id_next>]\n"
#if defined(VTSS_ARCH_SERVAL)
    "        [<lookup>]\n"
#endif /* VTSS_ARCH_SERVAL */
#if defined(VTSS_FEATURE_ACL_V2)
    "        [(port <port_list>)] "
#else
    "        [(port <port>)] "
#endif /* VTSS_FEATURE_ACL_V2 */
    "[(policy <policy> <policy_bitmask>)]\n"
#if VTSS_SWITCH_STACKABLE
    "        [<sid>]\n"
#endif /* VTSS_SWITCH_STACKABLE */
#if defined(VTSS_FEATURE_ACL_V2)
    "        [<tagged>] [<vid>] [<tag_prio>] [<dmac_type>]\n"
#else
    "        [<vid>] [<tag_prio>] [<dmac_type>]\n"
#endif /* VTSS_FEATURE_ACL_V2 */
    "        [(etype [<etype>] [<smac>] [<dmac>]) |\n"
    "         (arp  [<sip>] [<dip>] [<smac>] [<arp_opcode>] [<arp_flags>]) |\n"
    "         (ip   [<sip>] [<dip>] [<protocol>] [<ip_flags>]) |\n"
    "         (icmp [<sip>] [<dip>] [<icmp_type>] [<icmp_code>] [<ip_flags>]) |\n"
    "         (udp  [<sip>] [<dip>] [<sport>] [<dport>] [<ip_flags>]) |\n"
#if defined(ACL_IPV6_SUPPORTED)
    "         (tcp  [<sip>] [<dip>] [<sport>] [<dport>] [<ip_flags>] [<tcp_flags>]) |\n"
    "         (ipv6_std [<next_header>] [<sip_v6>] [<ipv6_flags>]) |\n"
    "         (ipv6_icmp [<sip_v6>] [<icmp_type>] [<icmp_code>] [<ipv6_flags>]) |\n"
    "         (ipv6_udp [<sip_v6>] [<sport>] [<dport>] [<ipv6_flags>]) |\n"
    "         (ipv6_tcp [<sip_v6>] [<sport>] [<dport>] [<ipv6_flags>] [<tcp_flags>])]\n"
#else
    "         (tcp  [<sip>] [<dip>] [<sport>] [<dport>] [<ip_flags>] [<tcp_flags>])]\n"
#endif /* ACL_IPV6_SUPPORTED */
#if defined(VTSS_FEATURE_ACL_V2)
    "        [(permit) | (deny) | (filter <filter_port_list>)] [<rate_limiter>] "
#else
    "        [permit|deny] [<rate_limiter>] "
#endif /* VTSS_FEATURE_ACL_V2 */
#if defined(VTSS_ARCH_CARACAL) && defined(VTSS_SW_OPTION_EVC)
    "[<evc_policer>] "
#endif /* VTSS_ARCH_CARACAL && VTSS_SW_OPTION_EVC */
#if defined(VTSS_ARCH_LUTON28)
    "[<port_copy>] "
#else
    "[<port_redirect>] "
#endif /* VTSS_ARCH_LUTON28 */
#if defined(VTSS_FEATURE_ACL_V2)
    "[<mirror>] "
#endif /* VTSS_FEATURE_ACL_V2 */
    "[<logging>] [<shutdown>]",
    "Add or modify Access Control Entry (ACE).\n\n"
    "If the ACE ID parameter <ace_id> is specified and an entry with this ACE ID\n"
    "already exists, the ACE will be modified. Otherwise, a new ACE will be added.\n"
    "If the ACE ID is not specified, the next available ACE ID will be used.\n\n"
    "If the next ACE ID parameter <ace_id_next> is specified, the ACE will be placed\n"
    "before this ACE in the list. If the next ACE ID is not specified, the ACE\n"
    "will be placed last in the list.\n\n"
    "If the Switch keyword is used, the rule applies to all ports.\n"
    "If the Port keyword is used, the rule applies to the specified port only.\n"
    "If the Policy keyword is used, the rule applies to all ports configured\n"
    "with the specified policy. The default is that the rule applies to all ports"
#if VTSS_SWITCH_STACKABLE
    ".\n\nNotice: the ACE won't apply to any stacking or none existing port"
#endif /* VTSS_SWITCH_STACKABLE */
    ,
    PRIO_ACL_ADD,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SECURITY,
    acl_cli_cmd_add,
    acl_cli_req_default_set,
    acl_parm_table,
    CLI_CMD_FLAG_NONE
};

CLI_CMD_TAB_ENTRY_DECL(cli_mgmt_debug_lookup) = {
    "Debug ACL Lookup "
    "[static"
#ifdef VTSS_SW_OPTION_ETH_LINK_OAM
    "|link_oam"
#endif
#ifdef VTSS_SW_OPTION_LOOP_PROTECT
    "|loop_protect"
#endif
#ifdef VTSS_SW_OPTION_DHCP_HELPER
    "|dhcp"
#endif
#if defined(VTSS_SW_OPTION_PTP) || defined(VTSS_SW_OPTION_PHY_1588_SIM)
    "|ptp"
#endif
#ifdef VTSS_SW_OPTION_UPNP
    "|upnp"
#endif
#ifdef VTSS_SW_OPTION_ARP_INSPECTION
    "|arp_inspection"
#endif
#ifdef VTSS_SW_OPTION_MEP
    "|mep"
#endif
#ifdef VTSS_SW_OPTION_IPMC
    "|ipmc"
#endif
#ifdef VTSS_SW_OPTION_IP_SOURCE_GUARD
    "|ip_source_guard"
#endif
#if defined(VTSS_ARCH_JAGUAR_1_CE_MAC) || defined(IP_MGMT_USING_MAC_ACL_ENTRIES)
    "|ip_mgmt"
#endif
    "] <ace_id>",
    NULL,
    "Show detail configuration",
    PRIO_ACL_DEBUG_LOOKUP,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    cli_mgmt_debug_lookup,
    acl_cli_req_default_set,
    acl_parm_table,
    CLI_CMD_FLAG_NONE
};

CLI_CMD_TAB_ENTRY_DECL(cli_mgmt_debug_del) = {
    NULL,
    "Debug ACL Delete "
    "[static"
#ifdef VTSS_SW_OPTION_ETH_LINK_OAM
    "|link_oam"
#endif
#ifdef VTSS_SW_OPTION_LOOP_PROTECT
    "|loop_protect"
#endif
#ifdef VTSS_SW_OPTION_DHCP_HELPER
    "|dhcp"
#endif
#if defined(VTSS_SW_OPTION_PTP) || defined(VTSS_SW_OPTION_PHY_1588_SIM)
    "|ptp"
#endif
#ifdef VTSS_SW_OPTION_UPNP
    "|upnp"
#endif
#ifdef VTSS_SW_OPTION_ARP_INSPECTION
    "|arp_inspection"
#endif
#ifdef VTSS_SW_OPTION_MEP
    "|mep"
#endif
#ifdef VTSS_SW_OPTION_IPMC
    "|ipmc"
#endif
#ifdef VTSS_SW_OPTION_IP_SOURCE_GUARD
    "|ip_source_guard"
#endif
#if defined(VTSS_ARCH_JAGUAR_1_CE_MAC) || defined(IP_MGMT_USING_MAC_ACL_ENTRIES)
    "|ip_mgmt"
#endif
    "] <ace_id>",
    "Delete ACE configuration",
    PRIO_ACL_DEBUG_DEL,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    cli_mgmt_debug_del,
    acl_cli_req_default_set,
    acl_parm_table,
    CLI_CMD_FLAG_NONE
};

CLI_CMD_TAB_ENTRY_DECL(acl_cli_cmd_status) = {
    VTSS_CLI_GRP_SEC_NETWORK_PATH "ACL Status "
    "[combined|static"
#ifdef VTSS_SW_OPTION_ETH_LINK_OAM
    "|link_oam"
#endif
#ifdef VTSS_SW_OPTION_LOOP_PROTECT
    "|loop_protect"
#endif
#ifdef VTSS_SW_OPTION_DHCP_HELPER
    "|dhcp"
#endif
#if defined(VTSS_SW_OPTION_PTP) || defined(VTSS_SW_OPTION_PHY_1588_SIM)
    "|ptp"
#endif
#ifdef VTSS_SW_OPTION_UPNP
    "|upnp"
#endif
#ifdef VTSS_SW_OPTION_ARP_INSPECTION
    "|arp_inspection"
#endif
#ifdef VTSS_SW_OPTION_MEP
    "|mep"
#endif
#ifdef VTSS_SW_OPTION_IPMC
    "|ipmc"
#endif
#ifdef VTSS_SW_OPTION_IP_SOURCE_GUARD
    "|ip_source_guard"
#endif
#if defined(VTSS_ARCH_JAGUAR_1_CE_MAC) || defined(IP_MGMT_USING_MAC_ACL_ENTRIES)
    "|ip_mgmt"
#endif
    "|conflicts]",
    NULL,
    "Show ACL status",
    PRIO_ACL_STATUS,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_SECURITY,
    acl_cli_cmd_status,
    acl_cli_req_default_set,
    acl_parm_table,
    CLI_CMD_FLAG_NONE
};

cli_cmd_tab_entry (
    NULL,
    VTSS_CLI_GRP_SEC_NETWORK_PATH "ACL Port State [<port_list>] [enable|disable]",
    "Set or show the ACL port state",
    PRIO_ACL_PORT_STATE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SECURITY,
    acl_cli_cmd_port_state,
    acl_cli_req_default_set,
    acl_parm_table,
    CLI_CMD_FLAG_NONE
);
