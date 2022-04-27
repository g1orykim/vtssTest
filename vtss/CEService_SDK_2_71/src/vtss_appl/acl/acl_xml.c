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

#include "conf_xml_api.h"
#include "conf_xml_trace_def.h"
#include "misc_api.h"
#include "mgmt_api.h"
#include "port_api.h"
#include "vlan_api.h"
#if VTSS_SWITCH_STACKABLE
#include "topo_api.h"
#endif
#include "acl_api.h"

#define CX_ULONG_ANY    0XFFFFFFFF

/* Macros for ctringification */
#define cx_xstr(s) cx_str(s)
#define cx_str(s) #s

/* Tag IDs */
enum {
    /* Module tags */
    CX_TAG_ACL,

    /* Group tags */
    CX_TAG_PORT_TABLE,
    CX_TAG_ACL_TABLE,
    CX_TAG_RATE_LIMITER_TABLE,

    /* Parameter tags */
    CX_TAG_ENTRY,

    /* Last entry */
    CX_TAG_NONE
};

/* Tag table */
static cx_tag_entry_t acl_cx_tag_table[CX_TAG_NONE + 1] = {
    [CX_TAG_ACL] = {
        .name  = "acl",
        .descr = "Access Control List",
        .type = CX_TAG_TYPE_MODULE
    },
    [CX_TAG_PORT_TABLE] = {
        .name  = "port_table",
        .descr = "",
        .type = CX_TAG_TYPE_GROUP
    },
    [CX_TAG_ACL_TABLE] = {
        .name  = "acl_table",
        .descr = "",
        .type = CX_TAG_TYPE_GROUP
    },
    [CX_TAG_RATE_LIMITER_TABLE] = {
        .name  = "rate_limiter_table",
        .descr = "",
        .type = CX_TAG_TYPE_GROUP
    },
    [CX_TAG_ENTRY] = {
        .name  = "entry",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },

    /* Last entry */
    [CX_TAG_NONE] = {
        .name  = "",
        .descr = "",
        .type = CX_TAG_TYPE_NONE
    }
};

#define CX_PROTO_ICMP   1
#define CX_PROTO_TCP    6
#define CX_PROTO_UDP    17
#define CX_PROTO_ICMPV6 58

/* ACL DMAC type */
typedef enum {
    CX_DMAC_ANY,
    CX_DMAC_UC,
    CX_DMAC_MC,
    CX_DMAC_BC
} cx_dmac_type_t;

/* Keyword for ACL dmac type */
static const cx_kw_t cx_kw_dmac[] = {
    { "any",       CX_DMAC_ANY },
    { "unicast",   CX_DMAC_UC },
    { "multicast", CX_DMAC_MC },
    { "broadcast", CX_DMAC_BC },
    { NULL,        0 }
};

/* Keyword for ACL frame types */
static const cx_kw_t cx_kw_frame_type[] = {
    { "any",   VTSS_ACE_TYPE_ANY },
    { "etype", VTSS_ACE_TYPE_ETYPE },
    { "arp",   VTSS_ACE_TYPE_ARP },
    { "ipv4",  VTSS_ACE_TYPE_IPV4 },
    { "ipv6",  VTSS_ACE_TYPE_IPV6 },
    { NULL,    0 }
};

/* ACL opcode type */
typedef enum {
    CX_OPCODE_ANY,
    CX_OPCODE_ARP,
    CX_OPCODE_RARP,
    CX_OPCODE_OTHER
} cx_opcode_t;

/* Keyword for ACL opcode */
static const cx_kw_t cx_kw_opcode[] = {
    { "any",   CX_OPCODE_ANY },
    { "arp",   CX_OPCODE_ARP },
    { "rarp",  CX_OPCODE_RARP },
    { "other", CX_OPCODE_OTHER },
    { NULL,    0 }
};

/* Keyword for ACL action */
static const cx_kw_t cx_kw_action[] = {
#if defined(VTSS_FEATURE_ACL_V2)
    { "filter", 2 },
#endif /* VTSS_FEATURE_ACL_V2 */
    { "permit", 1 },
    { "deny",   0 },
    { NULL,     0 }
};

static const cx_kw_t cx_kw_port_action[] = {
    { "permit", 1 },
    { "deny",   0 },
    { NULL,     0 }
};

#if defined(VTSS_FEATURE_ACL_V2)
/* Keyword for tagged */
static const cx_kw_t cx_kw_tagged[] = {
    { "any",        VTSS_ACE_BIT_ANY},
    { "disabled",   VTSS_ACE_BIT_0},
    { "enabled",    VTSS_ACE_BIT_1},
    { NULL,       0 }
};
#endif /* VTSS_FEATURE_ACL_V2 */

/* Add ulong/disabled attribute */
static vtss_rc cx_add_attr_ulong_dis(cx_get_state_t *s, char *name, ulong val, ulong match)
{
    return cx_add_attr_ulong_word(s, name, val, "disabled", match);
}

/* Add ulong/disabled syntax */
static vtss_rc cx_add_stx_ulong_dis(cx_get_state_t *s, char *name, ulong min, ulong max)
{
    return cx_add_stx_ulong_word(s, name, "disabled", min, max);
}

/* Add ulong/any syntax */
static vtss_rc cx_add_stx_ulong_any(cx_get_state_t *s, char *name, ulong min, ulong max)
{
    return cx_add_stx_ulong_word(s, name, "any", min, max);
}

#if defined(VTSS_FEATURE_ACL_V1)
/* Add ulong/all syntax */
static vtss_rc cx_add_stx_ulong_all(cx_get_state_t *s, char *name, ulong min, ulong max)
{
    return cx_add_stx_ulong_word(s, name, "all", min, max);
}
#endif /* VTSS_FEATURE_ACL_V1 */

#if defined(ACL_PACKET_RATE_MAX) && defined(ACL_BIT_RATE_MAX)
static char *cx_policer_rate_txt = "rate_pps is 0-"cx_xstr(ACL_PACKET_RATE_MAX)" or rate_kbps is 0, 100, 200, 300, ..., 1000000";
#elif defined(ACL_PACKET_RATE_IN_RANGE)
static char *cx_policer_rate_txt = "rate_pps is 0-"cx_xstr(ACL_PACKET_RATE_MAX);
#else
static char *cx_policer_rate_txt = "1/2/4/.../512/1k/2k/4k/.../"cx_xstr(ACL_PACKET_RATE_MAX)"k";
#endif /* ACL_PACKET_RATE_MAX && ACL_BIT_RATE_MAX */

/* Add packet rate syntax */
static vtss_rc cx_add_stx_policer_rate(cx_get_state_t *s, char *name)
{
    return cx_add_attr_txt(s, name, cx_policer_rate_txt);
}

/* Add 'any' attribute */
static vtss_rc cx_add_attr_any(cx_get_state_t *s, char *name, char *alias)
{
    return cx_add_attr_txt(s, name, alias);
}

/* Add ulong or 'any' attribute */
static vtss_rc cx_add_attr_ulong_any(cx_get_state_t *s, char *name, ulong val, BOOL any)
{
    return (any ? cx_add_attr_any(s, name, "any") : cx_add_attr_ulong(s, name, val));
}

#if defined(VTSS_FEATURE_ACL_V1)
/* Add ulong or 'all' attribute */
static vtss_rc cx_add_attr_ulong_all(cx_get_state_t *s, char *name, ulong val, BOOL any)
{
    return (any ? cx_add_attr_any(s, name, "all") : cx_add_attr_ulong(s, name, val));
}
#endif /* VTSS_FEATURE_ACL_V1 */

/* Add flag attribute */
static vtss_rc cx_add_attr_flag(cx_get_state_t *s, char *name,
                                acl_entry_conf_t *ace, acl_flag_t flag)
{
    char buf[32];

    sprintf(buf, "flag_%s", name);
    return cx_add_attr_txt(s, buf, mgmt_acl_flag_txt(ace, flag, 1));
}

/* Add ACl action */
static vtss_rc cx_add_action(cx_get_state_t *s, acl_action_t *action, BOOL is_port_action)
{
#if defined(VTSS_FEATURE_ACL_V2)
    int     i, port_filter_list_cnt;
    BOOL    is_filter_enable;
    char    buf[MGMT_PORT_BUF_SIZE], buf_1[80], buf_2[80];
#endif /* VTSS_FEATURE_ACL_V2 */

    if (action == NULL) {
        /* Syntax */
        CX_RC(cx_add_stx_kw(s, "action", is_port_action ? cx_kw_port_action : cx_kw_action));
#if defined(VTSS_FEATURE_ACL_V2)
        if (!is_port_action) {
            sprintf(buf_1, "all or %s", cx_list_txt(buf_2, 1, s->port_count));
            CX_RC(cx_add_attr_txt(s, "filter_port", buf_1));
        }
#endif /* VTSS_FEATURE_ACL_V2 */
        CX_RC(cx_add_stx_ulong_dis(s, "rate_limiter", 1, ACL_POLICER_NO_END));
#if defined(VTSS_ARCH_CARACAL) && defined(VTSS_SW_OPTION_EVC)
        CX_RC(cx_add_stx_ulong_dis(s, "evc_policer", ACL_EVC_POLICER_NO_START, ACL_EVC_POLICER_NO_END));
#endif /* VTSS_ARCH_CARACAL && VTSS_SW_OPTION_EVC */
#if defined(VTSS_FEATURE_ACL_V2)
        sprintf(buf_1, "disabled or %s", cx_list_txt(buf_2, 1, s->port_count));
        CX_RC(cx_add_attr_txt(s, "port_redirect", buf_1));
        CX_RC(cx_add_stx_bool(s, "mirror"));
#elif defined(VTSS_ARCH_JAGUAR_1)
        CX_RC(cx_add_stx_ulong_dis(s, "port_redirect", 1, s->port_count));
#else
        CX_RC(cx_add_stx_ulong_dis(s, "port_copy", 1, s->port_count));
#endif /* VTSS_FEATURE_ACL_V2 */
        CX_RC(cx_add_stx_bool(s, "logging"));
        return cx_add_stx_bool(s, "shutdown");
    }

#if defined(VTSS_FEATURE_ACL_V2)
    for (port_filter_list_cnt = 0, i = VTSS_PORT_NO_START; i < VTSS_PORT_NO_END; i++) {
        if (action->port_list[i]) {
            port_filter_list_cnt++;
        }
    }
    if (action->port_action == VTSS_ACL_PORT_ACTION_FILTER && port_filter_list_cnt != 0) {
        is_filter_enable = TRUE;
    } else {
        is_filter_enable = FALSE;
    }

    CX_RC(cx_add_attr_kw(s, "action", is_port_action ? cx_kw_port_action : cx_kw_action, action->port_action == VTSS_ACL_PORT_ACTION_NONE ? 1 : is_filter_enable ? 2 : 0));
    if (is_filter_enable) {
        if (port_filter_list_cnt == VTSS_PORT_NO_END) {
            CX_RC(cx_add_attr_txt(s, "filter_port", "all"));
        } else {
            CX_RC(cx_add_attr_txt(s, "filter_port", mgmt_iport_list2txt(action->port_list, buf)));
        }
    }
#else
    CX_RC(cx_add_attr_kw(s, "action", is_port_action ? cx_kw_port_action : cx_kw_action, action->permit));
#endif /* VTSS_FEATURE_ACL_V2 */
    CX_RC(cx_add_attr_ulong_dis(s, "rate_limiter", action->policer == ACL_POLICER_NONE ? ACL_POLICER_NONE : ipolicer2upolicer(action->policer), ACL_POLICER_NONE));
#if defined(VTSS_ARCH_CARACAL) && defined(VTSS_SW_OPTION_EVC)
    CX_RC(cx_add_attr_ulong_dis(s, "evc_policer", action->evc_police ? ievcpolicer2uevcpolicer(action->evc_policer_id) : 0, 0));
#endif /* VTSS_ARCH_CARACAL && VTSS_SW_OPTION_EVC */
#if defined(VTSS_FEATURE_ACL_V2)
    if (action->port_action == VTSS_ACL_PORT_ACTION_REDIR) {
        CX_RC(cx_add_attr_txt(s, "port_redirect", mgmt_iport_list2txt(action->port_list, buf)));
    } else {
        CX_RC(cx_add_attr_txt(s, "port_redirect", "disabled"));
    }
    CX_RC(cx_add_attr_bool(s, "mirror", action->mirror));
#elif defined(VTSS_ARCH_JAGUAR_1)
    CX_RC(cx_add_attr_ulong_dis(s, "port_redirect", action->port_no == VTSS_PORT_NO_NONE ? action->port_no : iport2uport(action->port_no), VTSS_PORT_NO_NONE));
#else
    CX_RC(cx_add_attr_ulong_dis(s, "port_copy", action->port_no == VTSS_PORT_NO_NONE ? action->port_no : iport2uport(action->port_no), VTSS_PORT_NO_NONE));
#endif /* VTSS_FEATURE_ACL_V2 */
    CX_RC(cx_add_attr_bool(s, "logging", action->logging));
    CX_RC(cx_add_attr_bool(s, "shutdown", action->shutdown));
    return cx_size_check(s);
}

static BOOL cx_rl_match(const cx_table_context_t *context, ulong policer_a, ulong policer_b)
{
    acl_policer_conf_t conf_a, conf_b;

    return (acl_mgmt_policer_conf_get(policer_a, &conf_a) == VTSS_OK &&
            acl_mgmt_policer_conf_get(policer_b, &conf_b) == VTSS_OK &&
            memcmp(&conf_a, &conf_b, sizeof(conf_a)) == 0);
}

static vtss_rc cx_rl_print(cx_get_state_t *s, const cx_table_context_t *context, ulong policer_no, char *txt)
{
    acl_policer_conf_t conf;
    char               buf[80];

    if (txt == NULL) {
        /* Syntax */
        CX_RC(cx_add_stx_start(s));
        CX_RC(cx_add_attr_txt(s, "rate_limiter", cx_list_txt(buf, 1, ACL_POLICER_NO_END)));
        CX_RC(cx_add_stx_policer_rate(s, "rate"));
        return cx_add_stx_end(s);
    }

    CX_RC(acl_mgmt_policer_conf_get(policer_no, &conf));
    CX_RC(cx_add_attr_start(s, CX_TAG_ENTRY));
    CX_RC(cx_add_attr_txt(s, "rate_limiter", txt));
#if defined(ACL_PACKET_RATE_MAX) && defined(ACL_BIT_RATE_MAX)
    if (conf.bit_rate_enable) {
        CX_RC(cx_add_attr_ulong(s, "rate_kbps", conf.bit_rate));
    } else {
        CX_RC(cx_add_attr_ulong(s, "rate_pps", conf.packet_rate));
    }
#elif defined(ACL_PACKET_RATE_IN_RANGE)
    CX_RC(cx_add_attr_ulong(s, "rate_pps", conf.packet_rate));
#else
    if (conf.packet_rate < 10000) {
        CX_RC(cx_add_attr_ulong(s, "rate", conf.packet_rate));
    } else {
        sprintf(buf, "%uk", conf.packet_rate / 1000);
        CX_RC(cx_add_attr_txt(s, "rate", buf));
    }
#endif /* VTSS_FEATURE_ACL_V2 */
    return cx_add_attr_end(s, CX_TAG_ENTRY);
}

static BOOL cx_acl_match(const cx_table_context_t *context, ulong port_a, ulong port_b)
{
    acl_port_conf_t conf_a, conf_b;

    return (acl_mgmt_port_conf_get(context->isid, port_a, &conf_a) == VTSS_OK &&
            acl_mgmt_port_conf_get(context->isid, port_b, &conf_b) == VTSS_OK &&
            memcmp(&conf_a, &conf_b, sizeof(conf_a)) == 0);
}

static vtss_rc cx_acl_print(cx_get_state_t *s, const cx_table_context_t *context, ulong port_no, char *ports)
{
    acl_port_conf_t conf;
    port_vol_conf_t port_state_conf;
    port_user_t     user = PORT_USER_ACL;

    if (ports == NULL) {
        /* Syntax */
        CX_RC(cx_add_stx_start(s));
        CX_RC(cx_add_stx_port(s));
        CX_RC(cx_add_stx_ulong(s, "policy", VTSS_ACL_POLICY_NO_START, VTSS_ACL_POLICY_NO_END - 1));
        CX_RC(cx_add_action(s, NULL, TRUE));
        CX_RC(cx_add_stx_bool(s, "state"));
        return cx_add_stx_end(s);
    }

    CX_RC(acl_mgmt_port_conf_get(context->isid, port_no, &conf));
    CX_RC(cx_add_port_start(s, CX_TAG_ENTRY, ports));
    CX_RC(cx_add_attr_ulong(s, "policy", conf.policy_no));
    CX_RC(cx_add_action(s, &conf.action, TRUE));
    CX_RC(port_vol_conf_get(user, context->isid, port_no, &port_state_conf));
    CX_RC(cx_add_attr_bool(s, "state", !port_state_conf.disable));
    return cx_add_port_end(s, CX_TAG_ENTRY);
}

static BOOL cx_skip_none(ulong n)
{
    return 0;
}

/* Parameter error message for frame type */
static vtss_rc cx_parm_invalid_frame(cx_set_state_t *s, const char *name)
{
    if (s->rc == VTSS_OK) {
        sprintf(s->msg, "Parameter '%s' is not allowed for this frame type", name);
        s->rc = CONF_XML_ERROR_FILE_PARM;
    }
    return s->rc;
}

#if defined(VTSS_FEATURE_ACL_V2)
/* Parse portlist or word parameter, e.g. 'disabled' or 'all' */
static vtss_rc cx_parse_portlist_word(cx_set_state_t *s, char *name, char *word,
                                      BOOL word_val, BOOL *list, BOOL *word_match)
{
    vtss_rc rc1 = VTSS_OK;
    char    buf[80];
    int     iport;
    BOOL    uport_list[VTSS_PORT_ARRAY_SIZE + 1]; /* Index 0 may be used for type CLI_PARM_TYPE_PORT_LIST_0. This is the reason why we can't make a corresponding iport_list[] */

    *word_match = FALSE;
    memset(uport_list, 0, sizeof(uport_list));
    if (cx_parse_word(s, name, word) == VTSS_OK) {
        *word_match = TRUE;
        for (iport = VTSS_PORT_NO_START; iport < VTSS_PORT_NO_END; iport++) {
            list[iport] = word_val;
        }
    } else if ((rc1 = cx_parse_txt(s, name, buf, sizeof(buf))) == VTSS_OK) {
        if (mgmt_txt2list(buf, uport_list, 1, s->port_count, 0) != VTSS_OK) {
            CX_RC(cx_parm_invalid(s));
        } else {
            for (iport = VTSS_PORT_NO_START; iport < VTSS_PORT_NO_END; iport++) {
                list[iport] = uport_list[iport2uport(iport)];
            }
        }
    }
    return (rc1 == VTSS_OK ? s->rc : rc1);
}

#else

/* Parse ulong or 'all' parameter */
static vtss_rc cx_parse_ulong_all(cx_set_state_t *s, char *name, ulong *val,
                                  ulong min, ulong max)
{
    *val = CX_ULONG_ANY;
    return (cx_parse_all(s, name) == VTSS_OK ? VTSS_OK :
            cx_parse_ulong(s, name, val, min, max));
}
#endif /* VTSS_FEATURE_ACL_V2 */

/* Parse ulong or 'any' parameter */
static vtss_rc cx_parse_ulong_any(cx_set_state_t *s, char *name, ulong *val,
                                  ulong min, ulong max)
{
    *val = CX_ULONG_ANY;
    return (cx_parse_any(s, name) == VTSS_OK ? VTSS_OK :
            cx_parse_ulong(s, name, val, min, max));
}

/* Parse uchar or 'any' parameter */
static vtss_rc cx_parse_uchar_any(cx_set_state_t *s, char *name, vtss_ace_u8_t *data,
                                  ulong min, ulong max)
{
    ulong val;

    CX_RC(cx_parse_ulong_any(s, name, &val, min, max));
    data->value = val;
    data->mask = (val == CX_ULONG_ANY ? 0 : 0xff);
    return VTSS_OK;
}

/* Parse uchar or 'any' parameter */
static vtss_rc cx_parse_tag_prio(cx_set_state_t *s, char *name, vtss_ace_u8_t *data)
{
    char  buf[32];
    ulong low, high, mask, min = 0, max = 7;

    if (cx_parse_any(s, name) == VTSS_OK) {
        data->value = data->mask = 0;
        return VTSS_OK;
    } else {
        CX_RC(cx_parse_txt(s, name, buf, sizeof(buf)));
        if (mgmt_txt2range(buf, &low, &high, min, max) == VTSS_OK) {
            s->rc = CONF_XML_ERROR_FILE_PARM;
            for (mask = 0; mask <= max; mask = (mask * 2 + 1)) {
                if ((low & ~mask) == (high & ~mask) &&  /* Upper bits match */
                    (low & mask) == 0 &&                /* Lower bits of 'low' are zero */
                    (high | mask) == high) {            /* Lower bits of 'high are one */
                    s->rc = VTSS_OK;
                    data->value = high;
                    data->mask = ~mask;
                    data->mask &= max;
                    break;
                }
            }

            if (s->rc) {
                return s->rc;
            }
        } else {
            CX_RC(cx_parse_ulong_any(s, name, &high, 0, 7));
            data->value = high;
            data->mask = 0x7;
        }
    }

    return VTSS_OK;
}

/* Parse IPv4 address or 'any' */
static vtss_rc cx_parse_ipv4_any(cx_set_state_t *s, char *name, vtss_ace_ip_t *ip)
{
    if (cx_parse_any(s, name) == VTSS_OK) {
        ip->value = 0;
        ip->mask = 0;
        return VTSS_OK;
    }
    return cx_parse_ipv4(s, name, &ip->value, &ip->mask, 0);
}

/* Parse IPv6 address or 'any' */
static vtss_rc cx_parse_ipv6_any(cx_set_state_t *s, char *name, vtss_ace_u128_t *ipv6)
{
    vtss_rc rc;
    int i;

    if (cx_parse_any(s, name) == VTSS_OK) {
        memset(ipv6, 0, sizeof(*ipv6));
        return VTSS_OK;
    }

    if ((rc = cx_parse_ipv6(s, name, (vtss_ipv6_t *)ipv6->value)) == VTSS_OK) {
        for (i = 0; i < 16; i++) {
            ipv6->mask[i] = 0xff;
        }
    }
    return rc;
}

/* Set ACL flag */
static void cx_flag_set(acl_entry_conf_t *ace, acl_flag_t flag, vtss_ace_bit_t val)
{
#if defined(VTSS_FEATURE_ACL_V2)
    if (ace->type == VTSS_ACE_TYPE_IPV6) {
        switch (flag) {
        case ACE_FLAG_IP_TTL:
            ace->frame.ipv6.ttl = val;
            break;
        case ACE_FLAG_TCP_FIN:
            ace->frame.ipv6.tcp_fin = val;
            break;
        case ACE_FLAG_TCP_SYN:
            ace->frame.ipv6.tcp_syn = val;
            break;
        case ACE_FLAG_TCP_RST:
            ace->frame.ipv6.tcp_rst = val;
            break;
        case ACE_FLAG_TCP_PSH:
            ace->frame.ipv6.tcp_psh = val;
            break;
        case ACE_FLAG_TCP_ACK:
            ace->frame.ipv6.tcp_ack = val;
            break;
        case ACE_FLAG_TCP_URG:
            ace->frame.ipv6.tcp_urg = val;
            break;
        default:
            break;
        }
        return;
    }
#endif /* VTSS_FEATURE_ACL_V2 */

    VTSS_BF_SET(ace->flags.mask, flag, val != VTSS_ACE_BIT_ANY);
    VTSS_BF_SET(ace->flags.value, flag, val == VTSS_ACE_BIT_1);
}

/* Parse ACL policy */
static vtss_rc cx_parse_policy(cx_set_state_t *s, vtss_acl_policy_no_t *policy_no)
{
    ulong val;

    CX_RC(cx_parse_ulong(s, "policy", &val, VTSS_ACL_POLICY_NO_START, VTSS_ACL_POLICY_NO_END - 1));
    *policy_no = val;
    return VTSS_OK;
}

/* Parse ACL policy bitmask */
static vtss_rc cx_parse_policy_bitmask(cx_set_state_t *s, vtss_acl_policy_no_t *policy_bitmask)
{
    ulong val;

    CX_RC(cx_parse_ulong(s, "policy_bitmask", &val, 0x0, ACL_POLICIES_BITMASK));
    *policy_bitmask = val;
    return VTSS_OK;
}

/* Parse ACL flag */
static vtss_rc cx_parse_flag(cx_set_state_t *s, char *name,
                             acl_entry_conf_t *ace, acl_flag_t flag)
{
    char           buf[16], name_buf[32];
    vtss_ace_bit_t val;

    sprintf(name_buf, "flag_%s", name);
    CX_RC(cx_parse_txt(s, name_buf, buf, sizeof(buf)));
    if (cx_word_match("any", buf)) {
        val = VTSS_ACE_BIT_ANY;
    } else if (cx_word_match("1", buf)) {
        val = VTSS_ACE_BIT_1;
    } else if (cx_word_match("0", buf)) {
        val = VTSS_ACE_BIT_0;
    } else {
        return cx_parm_invalid(s);
    }
    cx_flag_set(ace, flag, val);
    return VTSS_OK;
}

/* Parse UDP/TCP port range */

/* Parse ACL action */
/*lint -esym(459, cx_parse_permit_value)*/
/* Get ACL port counter */
static vtss_rc cx_parse_action(cx_set_state_t *s, acl_action_t *action,
                               BOOL *forward, BOOL *policer, BOOL *evc_policer, BOOL *port_copy,
                               BOOL *mirror, BOOL *logging, BOOL *shutdown)
{
    ulong val;
#if defined(VTSS_FEATURE_ACL_V2)
    vtss_port_no_t                  iport, port_filter_list_cnt;;
    BOOL                            is_disabled, is_all, temp_port_list[VTSS_PORT_ARRAY_SIZE];
#elif defined(VTSS_ARCH_JAGUAR_1)
    static BOOL                     cx_parse_permit_value = TRUE;
#endif /* VTSS_FEATURE_ACL_V2 */

    if (cx_parse_kw(s, "action", cx_kw_action, &val, 1) == VTSS_OK) {
#if defined(VTSS_FEATURE_ACL_V2)
        action->port_action = ((val == 1) ? VTSS_ACL_PORT_ACTION_NONE : VTSS_ACL_PORT_ACTION_FILTER);
        if (val < 2) {
            action->port_action = val ? VTSS_ACL_PORT_ACTION_NONE : VTSS_ACL_PORT_ACTION_FILTER;
            for (iport = VTSS_PORT_NO_START; iport < VTSS_PORT_NO_END; iport++) {
                action->port_list[iport] = val;
            }
        }
#elif defined(VTSS_ARCH_JAGUAR_1)
        cx_parse_permit_value = action->permit = val;
#else
        action->permit = val;
#endif /* VTSS_FEATURE_ACL_V2 */
        *forward = 1;
    } else if (cx_parse_ulong_dis(s, "rate_limiter", &val, 1,
                                  ACL_POLICER_NO_END, ACL_POLICER_NONE) == VTSS_OK) {
        action->policer = (val == ACL_POLICER_NONE ? val : upolicer2ipolicer(val));
        *policer = 1;
#if defined(VTSS_ARCH_CARACAL) && defined(VTSS_SW_OPTION_EVC)
    } else if (cx_parse_ulong_dis(s, "evc_policer", &val, ACL_EVC_POLICER_NO_START,
                                  ACL_EVC_POLICER_NO_END, 0) == VTSS_OK) {
        action->evc_police = (val == 0 ? FALSE : TRUE);
        if (action->evc_police) {
            action->evc_policer_id = uevcpolicer2ievcpolicer(val);
        }
        *evc_policer = 1;
#endif /* VTSS_ARCH_CARACAL && VTSS_SW_OPTION_EVC */
#if defined(VTSS_FEATURE_ACL_V2)
    } else if (cx_parse_portlist_word(s, "filter_port", "all", TRUE, temp_port_list, &is_all) == VTSS_OK) {
        if (action->port_action == VTSS_ACL_PORT_ACTION_NONE) {
            CX_RC(cx_parm_error(s, "The parameter of 'filter_port' can't be set when action is permitted or denied"));
        }
        for (port_filter_list_cnt = 0, iport = VTSS_PORT_NO_START; iport < VTSS_PORT_NO_END; iport++) {
            if (action->port_list[iport]) {
                port_filter_list_cnt++;
            }
        }
        if (port_filter_list_cnt == 0) {
            CX_RC(cx_parm_error(s, "The parameter of 'filter_port' can't be set when action is permitted or denied"));
        } else {
            memcpy(action->port_list, temp_port_list, sizeof(temp_port_list));
            action->port_action = VTSS_ACL_PORT_ACTION_FILTER;
        }
    } else if (cx_parse_portlist_word(s, "port_redirect", "disabled", FALSE, temp_port_list, &is_disabled) == VTSS_OK) {
        if (!is_disabled) {
            if (action->port_action == VTSS_ACL_PORT_ACTION_NONE) {
                CX_RC(cx_parm_error(s, "The parameter of 'port_redirect' can't be set when action is permitted or filtered"));
            } else {
                for (port_filter_list_cnt = 0, iport = VTSS_PORT_NO_START; iport < VTSS_PORT_NO_END; iport++) {
                    if (action->port_list[iport]) {
                        port_filter_list_cnt++;
                    }
                }
                if (port_filter_list_cnt) {
                    CX_RC(cx_parm_error(s, "The parameter of 'port_redirect' can't be set when action is permitted or filtered"));
                } else {
                    action->port_action = VTSS_ACL_PORT_ACTION_REDIR;
                    memcpy(action->port_list, temp_port_list, sizeof(temp_port_list));
                }
            }
        }
        *port_copy = 1;
    } else if (cx_parse_bool(s, "mirror", &action->mirror, 1) == VTSS_OK) {
        *mirror = 1;
#elif defined(VTSS_ARCH_JAGUAR_1)
    } else if (cx_parse_ulong_dis(s, "port_redirect", &val, 1, s->port_count,
                                  VTSS_PORT_NO_NONE) == VTSS_OK) {
        action->port_no = (val == VTSS_PORT_NO_NONE ? val : uport2iport(val));
        *port_copy = 1;
        if (cx_parse_permit_value && val != VTSS_PORT_NO_NONE) {
            CX_RC(cx_parm_error(s, "The parameter of 'port_redirect' can't be set when action is permitted"));
        }
#else
    } else if (cx_parse_ulong_dis(s, "port_copy", &val, 1, s->port_count,
                                  VTSS_PORT_NO_NONE) == VTSS_OK) {
        action->port_no = (val == VTSS_PORT_NO_NONE ? val : uport2iport(val));
        *port_copy = 1;
#endif /* VTSS_FEATURE_ACL_V2 */
    } else if (cx_parse_bool(s, "logging", &action->logging, 1) == VTSS_OK) {
        *logging = 1;
    } else if (cx_parse_bool(s, "shutdown", &action->shutdown, 1) == VTSS_OK) {
        *shutdown = 1;
    } else {
        CX_RC(cx_parm_unknown(s));
    }

    return s->rc;
}

/* rate_unit - 0 : PPS, 1 : 1KBPS */
static vtss_rc cx_parse_policer_rate(cx_set_state_t *s, char *name, ulong *rate, ulong rate_unit)
{
    char buf[32];

    CX_RC(cx_parse_txt(s, name, buf, sizeof(buf)));
#if defined(ACL_PACKET_RATE_MAX) && defined(ACL_BIT_RATE_MAX)
    if (mgmt_txt2rate_v2(buf, rate, rate_unit, ACL_PACKET_RATE_MAX, ACL_BIT_RATE_MAX, ACL_BIT_RATE_GRANULARITY) != VTSS_OK) {
        CX_RC(cx_parm_invalid(s));
    }
#elif defined(ACL_PACKET_RATE_IN_RANGE)
    if (mgmt_txt2ulong(buf, rate, 0, ACL_PACKET_RATE_MAX) != VTSS_OK) {
        CX_RC(cx_parm_invalid(s));
    }
#else
    ulong i = 0, max_rate = ACL_PACKET_RATE_MAX;
    while (max_rate) {
        max_rate = max_rate >> 1;
        i++;
    }
    if (mgmt_txt2rate(buf, rate, i - 1) != VTSS_OK) {
        CX_RC(cx_parm_invalid(s));
    }
#endif /* VTSS_FEATURE_ACL_V2 */
    return s->rc;
}

static vtss_rc acl_cx_parse_func(cx_set_state_t *s)
{
    switch (s->cmd) {
    case CX_PARSE_CMD_PARM: {
        acl_entry_conf_t        ace;
        vtss_ace_u48_t          *dmac, smac;
        vtss_ace_u16_t          *etype;
        vtss_ace_ip_t           sip = {}, dip = {};
        vtss_ace_u128_t         sipv6 = {};
        vtss_ace_u8_t           *proto, *protov6;
        vtss_ace_u8_t           type, code;
        vtss_ace_udp_tcp_t      *sport, *dport;
#if defined(VTSS_FEATURE_ACL_V2)
        vtss_ace_u8_t           typev6, codev6;
        vtss_ace_udp_tcp_t      *sportv6, *dportv6;
        BOOL                    typev6_valid = 0, codev6_valid = 0;
        BOOL                    sportv6_valid = 0, dportv6_valid = 0;
#endif /* VTSS_FEATURE_ACL_V2 */
        uchar                   ip_proto;
        BOOL                    uc = 0, mc = 0, bc = 0;
        BOOL                    dmac_valid = 0, smac_valid = 0, etype_valid = 0;
        BOOL                    sip_valid = 0, dip_valid = 0, proto_valid = 0;
        BOOL                    type_valid = 0, code_valid = 0;
        BOOL                    sport_valid = 0, dport_valid = 0, u;
        BOOL                    protov6_valid = 0, sipv6_valid = 0, sipv6_mask_valid = 0;
        char                    buf[256];
        vtss_port_no_t          port_idx;
        BOOL                    port_list[VTSS_PORT_ARRAY_SIZE + 1];
        int                     i;
        ulong                   val, sipv6_mask = 0xFFFFFFFF;
        BOOL                    global;
        vtss_acl_policy_no_t    policy_no;

        global = (s->isid == VTSS_ISID_GLOBAL);

        switch (s->id) {
        case CX_TAG_ACL_TABLE:
            if (global) {
                /* Flush ACL table */
                while (s->apply &&
                       acl_mgmt_ace_get(ACL_USER_STATIC, s->isid, ACE_ID_NONE, &ace, NULL, 1) == VTSS_OK) {
                    CX_RC(acl_mgmt_ace_del(ACL_USER_STATIC, ace.id));
                }
            } else {
                s->ignored = 1;
            }
            break;
        case CX_TAG_RATE_LIMITER_TABLE:
            if (!global) {
                s->ignored = 1;
            }
            break;
        case CX_TAG_PORT_TABLE:
            if (global) {
                s->ignored = 1;
            }
            break;
        case CX_TAG_ENTRY:
            if (global && s->group == CX_TAG_ACL_TABLE &&
                cx_parse_ulong(s, "ace_id", &val, ACE_ID_START, ACE_ID_END) == VTSS_OK) {

                /* Initialize ACE before parsing parameters */
                CX_RC(acl_mgmt_ace_init(VTSS_ACE_TYPE_ANY, &ace));
                dmac = &ace.frame.etype.dmac;
                etype = &ace.frame.etype.etype;
                memset(&smac, 0, sizeof(smac));
                proto = &ace.frame.ipv4.proto;
                sport = &ace.frame.ipv4.sport;
                dport = &ace.frame.ipv4.dport;
                protov6 = &ace.frame.ipv6.proto;
                ace.id = val;
#if defined(VTSS_ARCH_SERVAL)
                ace.lookup = FALSE;
#endif /* VTSS_ARCH_SERVAL */
#if defined(VTSS_FEATURE_ACL_V2)
                ace.action.port_action = VTSS_ACL_PORT_ACTION_NONE;
                sportv6 = &ace.frame.ipv6.sport;
                dportv6 = &ace.frame.ipv6.dport;
#else
                ace.action.port_no = VTSS_PORT_NO_NONE;
                ace.action.permit = 1;
#endif /* VTSS_FEATURE_ACL_V2 */
                ace.isid = VTSS_ISID_GLOBAL;

                s->p = s->next;
                for (; s->rc == VTSS_OK && cx_parse_attr(s) == VTSS_OK; s->p = s->next) {
#if defined(VTSS_FEATURE_ACL_V2)
                    if (cx_parse_portlist_word(s, "port", "all", TRUE, ace.port_list, &u) == VTSS_OK) {
#else
                    if (cx_parse_ulong_all(s, "port", &val, 1, VTSS_PORTS) == VTSS_OK) {
                        if (val == CX_ULONG_ANY) {
                            ace.port_no = VTSS_PORT_NO_ANY;
                        } else {
                            ace.port_no = uport2iport(val);
                        }
#endif
#if defined(VTSS_ARCH_SERVAL)
                    } else if (cx_parse_bool(s, "lookup", &ace.lookup, 1) == VTSS_OK) {
#endif /* VTSS_ARCH_SERVAL */
                    } else if (cx_parse_policy(s, &policy_no) == VTSS_OK) {
                        ace.policy.value = policy_no;
                    } else if (cx_parse_policy_bitmask(s, &policy_no) == VTSS_OK) {
                        ace.policy.mask = policy_no < ACL_POLICIES_BITMASK ? policy_no : ACL_POLICIES_BITMASK;
                    } else if (cx_parse_kw(s, "frame", cx_kw_frame_type, &val, 1) == VTSS_OK) {
                        ace.type = val;
#if VTSS_SWITCH_STACKABLE
                    } else if (cx_parse_ulong_any(s, "sid", &val, VTSS_USID_START,
                                                  VTSS_USID_END - 1) == VTSS_OK) {
                        ace.isid = (val == CX_ULONG_ANY ? VTSS_ISID_GLOBAL :
                                    topo_usid2isid(val));
#endif /* VTSS_SWITCH_STACKABLE */
                    } else if (cx_parse_kw(s, "dmac", cx_kw_dmac, &val, 0) == VTSS_OK) {
                        dmac_valid = 0;
                        uc = (val == CX_DMAC_UC);
                        mc = (val == CX_DMAC_MC);
                        bc = (val == CX_DMAC_BC);
#if defined(VTSS_FEATURE_ACL_V2)
                        if (ace.type == VTSS_ACE_TYPE_ANY && val != 0) {
                            CX_RC(cx_parm_error(s, "DMAC type not supported for this frame type"));
                        }
#endif /* VTSS_FEATURE_ACL_V2 */
                    } else if (cx_parse_mac(s, "dmac", &dmac->value[0]) == VTSS_OK) {
                        dmac_valid = 1;
                        uc = 0;
                        mc = 0;
                        bc = 0;
                    } else if (cx_parse_any(s, "smac") == VTSS_OK) {
                        smac_valid = 0;
                    } else if (cx_parse_mac(s, "smac", &smac.value[0]) == VTSS_OK) {
                        smac_valid = 1;
                        for (i = 0; i < 6; i++) {
                            smac.mask[i] = 0xff;
                        }
                    } else if (cx_parse_ulong_any(s, "vid", &val, VLAN_ID_MIN, VLAN_ID_MAX) == VTSS_OK) {
                        ace.vid.value = val;
                        ace.vid.mask = (val == CX_ULONG_ANY ? 0 : 0xffff);
                    } else if (cx_parse_tag_prio(s, "tag_prio", &ace.usr_prio) ==
                               VTSS_OK) {
#if defined(VTSS_FEATURE_ACL_V2)
                    } else if (cx_parse_kw(s, "tagged", cx_kw_tagged, &val, 1) == VTSS_OK) {
                        ace.tagged = val;
                        if (ace.tagged == VTSS_ACE_BIT_0 && (ace.vid.mask || ace.usr_prio.mask)) {
                            CX_RC(cx_parm_error(s, "The parameter of 'vid' and 'tag_prio' can't be set when 802.1Q Tagged is disabled"));
                        }
#endif /* VTSS_FEATURE_ACL_V2 */
                    } else if (cx_parse_ulong_any(s, "etype", &val, 0x0600,
                                                  0xffff) == VTSS_OK) {
                        etype_valid = 1;
                        etype->value[0] = ((val >> 8) & 0xff);
                        etype->value[1] = (val & 0xff);
                        etype->mask[0] = (val == CX_ULONG_ANY ? 0 : 0xff);
                        etype->mask[1] = (val == CX_ULONG_ANY ? 0 : 0xff);
                    } else if (cx_parse_ipv4_any(s, "sip", &sip) == VTSS_OK) {
                        sip_valid = 1;
                    } else if (cx_parse_ipv4_any(s, "dip", &dip) == VTSS_OK) {
                        dip_valid = 1;
                    } else if (cx_parse_kw(s, "opcode", cx_kw_opcode, &val, 1) == VTSS_OK) {
                        cx_flag_set(&ace, ACE_FLAG_ARP_ARP,
                                    val == CX_OPCODE_RARP ? VTSS_ACE_BIT_0 :
                                    val == CX_OPCODE_ARP ? VTSS_ACE_BIT_1 : VTSS_ACE_BIT_ANY);
                        cx_flag_set(&ace, ACE_FLAG_ARP_UNKNOWN,
                                    val == CX_OPCODE_OTHER ? VTSS_ACE_BIT_1 :
                                    val == CX_OPCODE_ANY ? VTSS_ACE_BIT_ANY : VTSS_ACE_BIT_0);
                    } else if
                    (cx_parse_flag(s, "request", &ace, ACE_FLAG_ARP_REQ) == VTSS_OK ||
                      cx_parse_flag(s, "smac", &ace, ACE_FLAG_ARP_SMAC) == VTSS_OK ||
                      cx_parse_flag(s, "dmac", &ace, ACE_FLAG_ARP_DMAC) == VTSS_OK ||
                      cx_parse_flag(s, "length", &ace, ACE_FLAG_ARP_LEN) == VTSS_OK ||
                      cx_parse_flag(s, "ether", &ace, ACE_FLAG_ARP_ETHER) == VTSS_OK ||
                      cx_parse_flag(s, "ip", &ace, ACE_FLAG_ARP_IP) == VTSS_OK) {
                    } else if (cx_parse_uchar_any(s, "protocol", proto, 0, 255) == VTSS_OK) {
                        proto_valid = 1;
                    } else if (cx_parse_uchar_any(s, "icmp_type", &type, 0, 255) == VTSS_OK) {
                        type_valid = 1;
                        ace.frame.ipv4.data.value[0] = type.value;
                        ace.frame.ipv4.data.mask[0] = type.mask;
                    } else if (cx_parse_uchar_any(s, "icmp_code", &code, 0, 255) == VTSS_OK) {
                        code_valid = 1;
                        ace.frame.ipv4.data.value[1] = code.value;
                        ace.frame.ipv4.data.mask[1] = code.mask;
                    } else if (cx_parse_udp_tcp(s, "sport", &sport->low, &sport->high) ==
                               VTSS_OK) {
                        sport_valid = 1;
                    } else if (cx_parse_udp_tcp(s, "dport", &dport->low, &dport->high) ==
                               VTSS_OK) {
                        dport_valid = 1;
                    } else if
                    (cx_parse_flag(s, "ttl", &ace, ACE_FLAG_IP_TTL) == VTSS_OK ||
                      cx_parse_flag(s, "fragment", &ace, ACE_FLAG_IP_FRAGMENT) == VTSS_OK ||
                      cx_parse_flag(s, "options", &ace, ACE_FLAG_IP_OPTIONS) == VTSS_OK ||
                      cx_parse_flag(s, "syn", &ace, ACE_FLAG_TCP_SYN) == VTSS_OK ||
                      cx_parse_flag(s, "ack", &ace, ACE_FLAG_TCP_ACK) == VTSS_OK ||
                      cx_parse_flag(s, "fin", &ace, ACE_FLAG_TCP_FIN) == VTSS_OK ||
                      cx_parse_flag(s, "rst", &ace, ACE_FLAG_TCP_RST) == VTSS_OK ||
                      cx_parse_flag(s, "urg", &ace, ACE_FLAG_TCP_URG) == VTSS_OK ||
                      cx_parse_flag(s, "psh", &ace, ACE_FLAG_TCP_PSH) == VTSS_OK) {
#if defined(VTSS_FEATURE_ACL_V2)
                    } else if (cx_parse_uchar_any(s, "icmpv6_type", &typev6, 0, 255) == VTSS_OK) {
                        typev6_valid = 1;
                        ace.frame.ipv6.data.value[0] = typev6.value;
                        ace.frame.ipv6.data.mask[0] = typev6.mask;
                    } else if (cx_parse_uchar_any(s, "icmpv6_code", &codev6, 0, 255) == VTSS_OK) {
                        codev6_valid = 1;
                        ace.frame.ipv6.data.value[1] = codev6.value;
                        ace.frame.ipv6.data.mask[1] = codev6.mask;
                    } else if (cx_parse_udp_tcp(s, "sportv6", &sportv6->low, &sportv6->high) ==
                               VTSS_OK) {
                        sportv6_valid = 1;
                    } else if (cx_parse_udp_tcp(s, "dportv6", &dportv6->low, &dportv6->high) ==
                               VTSS_OK) {
                        dportv6_valid = 1;
                    } else if
                    (cx_parse_flag(s, "hop_limit", &ace, ACE_FLAG_IP_TTL) == VTSS_OK ||
                      cx_parse_flag(s, "tcpv6_syn", &ace, ACE_FLAG_TCP_SYN) == VTSS_OK ||
                      cx_parse_flag(s, "tcpv6_ack", &ace, ACE_FLAG_TCP_ACK) == VTSS_OK ||
                      cx_parse_flag(s, "tcpv6_fin", &ace, ACE_FLAG_TCP_FIN) == VTSS_OK ||
                      cx_parse_flag(s, "tcpv6_rst", &ace, ACE_FLAG_TCP_RST) == VTSS_OK ||
                      cx_parse_flag(s, "tcpv6_urg", &ace, ACE_FLAG_TCP_URG) == VTSS_OK ||
                      cx_parse_flag(s, "tcpv6_psh", &ace, ACE_FLAG_TCP_PSH) == VTSS_OK) {
#endif /* VTSS_FEATURE_ACL_V2 */
                    } else if (cx_parse_uchar_any(s, "next_header", protov6, 0, 255) == VTSS_OK) {
                        protov6_valid = 1;
                    } else if (cx_parse_ipv6_any(s, "sipv6", &sipv6) == VTSS_OK) {
                        sipv6_valid = 1;
                    } else if (cx_parse_ulong(s, "sipv6_mask", &sipv6_mask, 0x0, 0xFFFFFFFF) == VTSS_OK) {
                        sipv6_mask_valid = 1;
                    } else {
                        CX_RC(cx_parse_action(s, &ace.action, &u, &u, &u, &u, &u, &u, &u));
                    }
                }
                if (smac_valid) {
                    if (ace.type == VTSS_ACE_TYPE_ETYPE) {
                        ace.frame.etype.smac = smac;
                    } else if (ace.type == VTSS_ACE_TYPE_ARP) {
                        ace.frame.arp.smac = smac;
                    } else {
                        CX_RC(cx_parm_invalid_frame(s, "smac"));
                    }
                }
                if (dmac_valid) {
                    for (i = 0; i < 6; i++) {
                        dmac->mask[i] = 0xff;
                    }
                    if (ace.type != VTSS_ACE_TYPE_ETYPE) {
                        CX_RC(cx_parm_error(s, "Specific DMAC not allowed for this frame type"));
                    }
                }
                cx_flag_set(&ace, ACE_FLAG_DMAC_MC, uc ? VTSS_ACE_BIT_0 :
                            (mc || bc) ? VTSS_ACE_BIT_1 : VTSS_ACE_BIT_ANY);
                cx_flag_set(&ace, ACE_FLAG_DMAC_BC, bc ? VTSS_ACE_BIT_1 :
                            (uc || mc) ? VTSS_ACE_BIT_0 : VTSS_ACE_BIT_ANY);
                if (etype_valid && ace.type != VTSS_ACE_TYPE_ETYPE) {
                    CX_RC(cx_parm_invalid_frame(s, "etype"));
                }
                if (sip_valid) {
                    if (ace.type == VTSS_ACE_TYPE_ARP) {
                        ace.frame.arp.sip = sip;
                    } else if (ace.type == VTSS_ACE_TYPE_IPV4) {
                        ace.frame.ipv4.sip = sip;
                    } else {
                        CX_RC(cx_parm_invalid_frame(s, "sip"));
                    }
                }
                if (dip_valid) {
                    if (ace.type == VTSS_ACE_TYPE_ARP) {
                        ace.frame.arp.dip = dip;
                    } else if (ace.type == VTSS_ACE_TYPE_IPV4) {
                        ace.frame.ipv4.dip = dip;
                    } else {
                        CX_RC(cx_parm_invalid_frame(s, "dip"));
                    }
                }
#if defined(ACL_IPV6_SUPPORTED)
                if (protov6_valid && ace.type != VTSS_ACE_TYPE_IPV6) {
                    CX_RC(cx_parm_invalid_frame(s, "proto"));
                }
#else
                if (protov6_valid && ace.type == VTSS_ACE_TYPE_IPV6) {
                    CX_RC(cx_parm_invalid_frame(s, "proto"));
                }
#endif /* ACL_IPV6_SUPPORTED */
                if (sipv6_valid) {
                    if (ace.type == VTSS_ACE_TYPE_IPV6) {
                        ace.frame.ipv6.sip = sipv6;
                    } else {
                        CX_RC(cx_parm_invalid_frame(s, "sipv6"));
                    }
                }
                if (sipv6_mask_valid) {
                    if (ace.type == VTSS_ACE_TYPE_IPV6) {
                        ace.frame.ipv6.sip.mask[12] = (sipv6_mask & 0xff000000) >> 24;
                        ace.frame.ipv6.sip.mask[13] = (sipv6_mask & 0x00ff0000) >> 16;
                        ace.frame.ipv6.sip.mask[14] = (sipv6_mask & 0x0000ff00) >>  8;
                        ace.frame.ipv6.sip.mask[15] = (sipv6_mask & 0x000000ff) >>  0;
                    } else {
                        CX_RC(cx_parm_invalid_frame(s, "sipv6_mask"));
                    }
                }
                if (proto_valid && ace.type != VTSS_ACE_TYPE_IPV4) {
                    CX_RC(cx_parm_invalid_frame(s, "proto"));
                }
                ip_proto = mgmt_acl_ip_proto(&ace);
                if (type_valid && ip_proto != CX_PROTO_ICMP) {
                    CX_RC(cx_parm_invalid_frame(s, "icmp_type"));
                }
                if (code_valid && ip_proto != CX_PROTO_ICMP) {
                    CX_RC(cx_parm_invalid_frame(s, "icmp_code"));
                }
#if defined(VTSS_FEATURE_ACL_V2)
                if (typev6_valid && ip_proto != CX_PROTO_ICMPV6) {
                    CX_RC(cx_parm_invalid_frame(s, "icmpv6_type"));
                }
                if (codev6_valid && ip_proto != CX_PROTO_ICMPV6) {
                    CX_RC(cx_parm_invalid_frame(s, "icmpv6_code"));
                }
#endif /* VTSS_FEATURE_ACL_V2 */
                if (ip_proto == CX_PROTO_TCP || ip_proto == CX_PROTO_UDP) {
                    if (ace.type == VTSS_ACE_TYPE_IPV4) {
                        sport->in_range = 1;
                        if (!sport_valid) {
                            sport->high = 0xffff;
                        }
                        dport->in_range = 1;
                        if (!dport_valid) {
                            dport->high = 0xffff;
                        }
                    }
#if defined(VTSS_FEATURE_ACL_V2)
                    else if (ace.type == VTSS_ACE_TYPE_IPV6) {
                        sportv6->in_range = 1;
                        if (!sportv6_valid) {
                            sportv6->high = 0xffff;
                        }
                        dportv6->in_range = 1;
                        if (!dportv6_valid) {
                            dportv6->high = 0xffff;
                        }
                    }
#endif /* VTSS_FEATURE_ACL_V2 */
                } else {
                    if (sport_valid) {
                        CX_RC(cx_parm_invalid_frame(s, "sport"));
                    } else if (dport_valid) {
                        CX_RC(cx_parm_invalid_frame(s, "dport"));
                    }
#if defined(VTSS_FEATURE_ACL_V2)
                    else if (sportv6_valid) {
                        CX_RC(cx_parm_invalid_frame(s, "sportv6"));
                    } else if (dportv6_valid) {
                        CX_RC(cx_parm_invalid_frame(s, "dportv6"));
                    }
#endif /* VTSS_FEATURE_ACL_V2 */
                }
                if (s->apply) {
                    CX_RC(acl_mgmt_ace_add(ACL_USER_STATIC, ACE_ID_NONE, &ace));
                }
            }
            else if (global && s->group == CX_TAG_RATE_LIMITER_TABLE &&
                     cx_parse_txt(s, "rate_limiter", buf, sizeof(buf)) == VTSS_OK) {
                BOOL                  policer_list[ACL_POLICER_NO_END + 1];
                acl_policer_conf_t    conf;
                vtss_acl_policer_no_t policer_no;
                ulong                 rate = 1;
                BOOL                  success = 0;
#if defined(VTSS_FEATURE_ACL_V2)
                ulong                 rate_unit = 0;
#endif /* VTSS_FEATURE_ACL_V2 */

                s->p = s->next;
                if (mgmt_txt2list(buf, policer_list, 1, ACL_POLICER_NO_END, 0) != VTSS_OK) {
                    CX_RC(cx_parm_invalid(s));
                }
#if defined(VTSS_FEATURE_ACL_V2)
                else if (cx_parse_policer_rate(s, "rate_pps", &rate, 0) == VTSS_OK) {
                    rate_unit = 0;
                    success = TRUE;
                } else if (cx_parse_policer_rate(s, "rate_kbps", &rate, 1) == VTSS_OK) {
                    rate_unit = 1;
                    success = TRUE;
                }
#else
                else if (cx_parse_policer_rate(s, "rate", &rate, 0) == VTSS_OK) {
                    success = TRUE;
                }
#endif /* VTSS_FEATURE_ACL_V2 */
                if (success == TRUE && s->apply) {
                    for (policer_no = VTSS_ACL_POLICER_NO_START;
                         policer_no < ACL_POLICER_NO_END; policer_no++) {
                        if (policer_list[ipolicer2upolicer(policer_no)]) {
                            if (acl_mgmt_policer_conf_get(policer_no, &conf) == VTSS_OK) {
#if defined(VTSS_FEATURE_ACL_V2)
                                if (rate_unit == 0) {
                                    conf.bit_rate_enable = 0;
                                    conf.packet_rate = rate;
                                } else {
                                    conf.bit_rate_enable = 1;
                                    conf.bit_rate = rate;
                                }
#else
                                conf.packet_rate = rate;
#endif /* VTSS_FEATURE_ACL_V2 */
                                CX_RC(acl_mgmt_policer_conf_set(policer_no, &conf));
                            }
                        }
                    }
                }
            } else if (!global && s->group == CX_TAG_PORT_TABLE &&
                       cx_parse_ports(s, port_list, 1) == VTSS_OK) {
                acl_port_conf_t conf, new;
                port_vol_conf_t port_state_conf, port_state_newconf;
                port_user_t     user = PORT_USER_ACL;
                BOOL            policy = 0, forward = 0, policer = 0, evc_policer = 0;
                BOOL            port_copy = 0, mirror = 0, logging = 0, shutdown = 0, state = 0;

                memset(&new, 0, sizeof(new));
                memset(&port_state_newconf, 0, sizeof(port_state_newconf));
                s->p = s->next;
                for (; s->rc == VTSS_OK && cx_parse_attr(s) == VTSS_OK; s->p = s->next)
                    if (cx_parse_policy(s, &policy_no) == VTSS_OK) {
                        policy = 1;
                        new.policy_no = policy_no;
                    } else if (cx_parse_bool(s, "state", &port_state_conf.disable, 1) == VTSS_OK) {
                        state = 1;
                        port_state_newconf.disable = !port_state_conf.disable;
                    } else
                        CX_RC(cx_parse_action(s, &new.action, &forward, &policer,
                                              &evc_policer, &port_copy, &mirror, &logging, &shutdown));
                for (port_idx = VTSS_PORT_NO_START; port_idx < VTSS_PORT_NO_END; port_idx++)
                    if (s->apply && port_list[iport2uport(port_idx)] &&
                        acl_mgmt_port_conf_get(s->isid, port_idx, &conf) == VTSS_OK &&
                        port_vol_conf_get(user, s->isid, port_idx, &port_state_conf) == VTSS_OK) {
                        if (policy) {
                            conf.policy_no = new.policy_no;
                        }
                        if (forward) {
#if defined(VTSS_FEATURE_ACL_V2)
                            conf.action.port_action = new.action.port_action;
#else
                            conf.action.permit = new.action.permit;
#endif /* VTSS_FEATURE_ACL_V2 */

                        }
                        if (policer) {
                            conf.action.policer = new.action.policer;
                        }
#if defined(VTSS_ARCH_CARACAL) && defined(VTSS_SW_OPTION_EVC)
                        if (evc_policer) {
                            conf.action.evc_police = new.action.evc_police;
                            conf.action.evc_policer_id = new.action.evc_policer_id;
                        }
#endif /* VTSS_ARCH_CARACAL && VTSS_SW_OPTION_EVC */
#if defined(VTSS_FEATURE_ACL_V2)
                        if (port_copy) {
                            vtss_port_no_t iport;
                            conf.action.port_action = new.action.port_action;
                            for (iport = VTSS_PORT_NO_START; iport < VTSS_PORT_NO_END; iport++) {
                                conf.action.port_list[iport] = new.action.port_list[iport];
                            }
                        }
                        if (mirror) {
                            conf.action.mirror = new.action.mirror;
                        }
#else
                        if (port_copy) {
                            conf.action.port_no = new.action.port_no;
                        }
#endif /* VTSS_FEATURE_ACL_V2 */
                        if (logging) {
                            conf.action.logging = new.action.logging;
                        }
                        if (shutdown) {
                            conf.action.shutdown = new.action.shutdown;
                        }
                        if (state) {
                            port_state_conf.disable = port_state_newconf.disable;
                        }
                        CX_RC(acl_mgmt_port_conf_set(s->isid, port_idx, &conf));
                        CX_RC(port_vol_conf_set(user, s->isid, port_idx, &port_state_conf));
                    }
            } else {
                s->ignored = 1;
            }
            break;
        default:
            s->ignored = 1;
            break;
        }
        break;
    } /* CX_PARSE_CMD_PARM */
    case CX_PARSE_CMD_GLOBAL:
        break;
    case CX_PARSE_CMD_SWITCH:
        break;
    default:
        break;
    }

    return s->rc;
}

static vtss_rc cx_add_attr_tag_prio(cx_get_state_t *s, char *name, vtss_ace_u8_t usr_prio)
{
    char buf[8];

    if (usr_prio.mask == 0x7 || usr_prio.mask == 0) {
        return cx_add_attr_ulong_any(s, name, usr_prio.value, usr_prio.mask == 0);
    } else {
        sprintf(buf, "%d-%d", usr_prio.value & usr_prio.mask, usr_prio.value);
        return cx_add_attr_txt(s, name, buf);
    }
}

static vtss_rc acl_cx_gen_func(cx_get_state_t *s)
{
    char buf[128];

    switch (s->cmd) {
    case CX_GEN_CMD_GLOBAL:
        /* Global - ACL */
        T_D("global - acl");
        CX_RC(cx_add_tag_line(s, CX_TAG_ACL, 0));
        {
            acl_entry_conf_t    ace;
            BOOL                etype, ip, arp;
#if defined(ACL_IPV6_SUPPORTED)
            BOOL                ipv6;
#endif /* ACL_IPV6_SUPPORTED */
            uchar               proto;
            char                *p;
            int                 i;
            const cx_kw_t       *keyword;
#if defined(VTSS_FEATURE_ACL_V2)
            int                 port_list_cnt;
#endif /* VTSS_FEATURE_ACL_V2 */

            CX_RC(cx_add_tag_line(s, CX_TAG_ACL_TABLE, 0));

            /* Entry syntax */
            CX_RC(cx_add_comment(s, "The 'ace_id' is the only mandatory attribute"));
            CX_RC(cx_add_comment(s, "The 'frame' attribute determines the syntax as shown below"));
            for (ace.type = VTSS_ACE_TYPE_ANY; ace.type <= VTSS_ACE_TYPE_IPV6; ace.type++) {
                if (ace.type == VTSS_ACE_TYPE_LLC || ace.type == VTSS_ACE_TYPE_SNAP) {
                    continue;
                }

                etype = (ace.type == VTSS_ACE_TYPE_ETYPE);
                arp = (ace.type == VTSS_ACE_TYPE_ARP);
                ip = (ace.type == VTSS_ACE_TYPE_IPV4);
#if defined(ACL_IPV6_SUPPORTED)
                ipv6 = (ace.type == VTSS_ACE_TYPE_IPV6);
#endif /* ACL_IPV6_SUPPORTED */

                CX_RC(cx_add_stx_start(s));
                CX_RC(cx_add_stx_ulong(s, "ace_id", ACE_ID_START, ACE_ID_END));
#if defined(VTSS_ARCH_SERVAL)
                CX_RC(cx_add_stx_bool(s, "lookup"));
#endif /* VTSS_ARCH_SERVAL */
#if defined(VTSS_FEATURE_ACL_V2)
                {
                    char buf_1[80], buf_2[80];
                    sprintf(buf_1, "all or %s", cx_list_txt(buf_2, 1, VTSS_PORT_NO_END));
                    CX_RC(cx_add_attr_txt(s, "port", buf_1));
                }
#else
                CX_RC(cx_add_stx_ulong_all(s, "port", 1, VTSS_PORT_NO_END));
#endif /* VTSS_FEATURE_ACL_V2 */
                CX_RC(cx_add_stx_ulong(s, "policy", VTSS_ACL_POLICY_NO_START, VTSS_ACL_POLICY_NO_END - 1));
                CX_RC(cx_add_stx_ulong(s, "policy_bitmask", 0x0, ACL_POLICIES_BITMASK));
                CX_RC(cx_add_attr_kw(s, "frame", cx_kw_frame_type, ace.type));
#if VTSS_SWITCH_STACKABLE
                CX_RC(cx_add_stx_ulong_any(s, "sid", VTSS_USID_START, VTSS_USID_END - 1));
#endif /* VTSS_SWITCH_STACKABLE */
                for (i = 0, p = &buf[0], keyword = cx_kw_dmac;
                     keyword->name != NULL; keyword++, i++) {
                    p += sprintf(p, "%s%s", i == 0 ? "" : "/", keyword->name);
                }
                if (etype) {
                    sprintf(p, "/xx-xx-xx-xx-xx-xx/xx.xx.xx.xx.xx.xx/xxxxxxxxxxxx");
                }
#if defined(VTSS_FEATURE_ACL_V2)
                if (ace.type != VTSS_ACE_TYPE_ANY) {
                    CX_RC(cx_add_attr_txt(s, "dmac", buf));
                }
#endif /* VTSS_FEATURE_ACL_V2 */
                if (etype || arp) {
                    CX_RC(cx_add_attr_txt(s, "smac", "any/xx-xx-xx-xx-xx-xx/xx.xx.xx.xx.xx.xx/xxxxxxxxxxxx"));
                }
#if defined(VTSS_FEATURE_ACL_V2)
                CX_RC(cx_add_stx_kw(s, "tagged", cx_kw_tagged));
#endif /* VTSS_FEATURE_ACL_V2 */
                CX_RC(cx_add_stx_ulong_any(s, "vid", VLAN_ID_MIN, VLAN_ID_MAX));
                CX_RC(cx_add_attr_txt(s, "tag_prio", "0-7 or any/0-1/2-3/4-5/6-7/0-3/4-7"));
                if (etype) {
                    CX_RC(cx_add_attr_txt(s, "etype", "any or 0x0600-0xffff"));
                }
                if (arp || ip) {
                    CX_RC(cx_add_stx_ipv4(s, "sip"));
                    CX_RC(cx_add_stx_ipv4(s, "dip"));
                }
                if (arp) {
                    CX_RC(cx_add_stx_kw(s, "opcode", cx_kw_opcode));
                    CX_RC(cx_add_stx_flag(s, "request"));
                    CX_RC(cx_add_stx_flag(s, "smac"));
                    CX_RC(cx_add_stx_flag(s, "dmac"));
                    CX_RC(cx_add_stx_flag(s, "length"));
                    CX_RC(cx_add_stx_flag(s, "ether"));
                    CX_RC(cx_add_stx_flag(s, "ip"));
                }
                if (ip) {
                    CX_RC(cx_add_stx_ulong_any(s, "protocol", 0, 255));
                    CX_RC(cx_add_stx_ulong_any(s, "icmp_type", 0, 255));
                    CX_RC(cx_add_stx_ulong_any(s, "icmp_code", 0, 255));
                    sprintf(buf, "any or 0-65535 (range allowed. e.g. 10-20)");
                    CX_RC(cx_add_attr_txt(s, "sport", buf));
                    CX_RC(cx_add_attr_txt(s, "dport", buf));
                    CX_RC(cx_add_stx_flag(s, "ttl"));
                    CX_RC(cx_add_stx_flag(s, "fragment"));
                    CX_RC(cx_add_stx_flag(s, "options"));
                    CX_RC(cx_add_stx_flag(s, "syn"));
                    CX_RC(cx_add_stx_flag(s, "ack"));
                    CX_RC(cx_add_stx_flag(s, "fin"));
                    CX_RC(cx_add_stx_flag(s, "rst"));
                    CX_RC(cx_add_stx_flag(s, "urg"));
                    CX_RC(cx_add_stx_flag(s, "psh"));
                }
#if defined(ACL_IPV6_SUPPORTED)
                if (ipv6) {
                    CX_RC(cx_add_stx_ulong_any(s, "next_header", 0, 255));
                    CX_RC(cx_add_stx_ipv6(s, "sipv6"));
                    CX_RC(cx_add_stx_ulong(s, "sipv6_mask", 0x0, 0xFFFFFFFF));
#if defined(VTSS_FEATURE_ACL_V2)
                    CX_RC(cx_add_stx_flag(s, "hop_limit"));
                    CX_RC(cx_add_stx_ulong_any(s, "icmpv6_type", 0, 255));
                    CX_RC(cx_add_stx_ulong_any(s, "icmpv6_code", 0, 255));
                    sprintf(buf, "any or 0-65535 (range allowed. e.g. 10-20)");
                    CX_RC(cx_add_attr_txt(s, "sportv6", buf));
                    CX_RC(cx_add_attr_txt(s, "dportv6", buf));
                    CX_RC(cx_add_stx_flag(s, "tcpv6_syn"));
                    CX_RC(cx_add_stx_flag(s, "tcpv6_ack"));
                    CX_RC(cx_add_stx_flag(s, "tcpv6_fin"));
                    CX_RC(cx_add_stx_flag(s, "tcpv6_rst"));
                    CX_RC(cx_add_stx_flag(s, "tcpv6_urg"));
                    CX_RC(cx_add_stx_flag(s, "tcpv6_psh"));
#endif /* VTSS_FEATURE_ACL_V2 */
                }
#endif /* ACL_IPV6_SUPPORTED */
                CX_RC(cx_add_action(s, NULL, FALSE));
                CX_RC(cx_add_stx_end(s));
            }

#if VTSS_SWITCH_STACKABLE
            CX_RC(cx_add_comment(s, "Notice: the ACE won't apply to any stacking or none existing port"));
#endif /* VTSS_SWITCH_STACKABLE */

            ace.id = ACE_ID_NONE;
            while (acl_mgmt_ace_get(ACL_USER_STATIC, VTSS_ISID_GLOBAL, ace.id, &ace, NULL, 1) == VTSS_OK) {
                CX_RC(cx_add_attr_start(s, CX_TAG_ENTRY));
                CX_RC(cx_add_attr_ulong(s, "ace_id", ace.id));
#if defined(VTSS_ARCH_SERVAL)
                CX_RC(cx_add_attr_bool(s, "lookup", ace.lookup));
#endif /* VTSS_ARCH_SERVAL */
#if defined(VTSS_FEATURE_ACL_V2)
                for (port_list_cnt = 0, i = VTSS_PORT_NO_START; i < VTSS_PORT_NO_END; i++) {
                    if (ace.port_list[i]) {
                        port_list_cnt++;
                    }
                }
                if (port_list_cnt == VTSS_PORT_NO_END) {
                    CX_RC(cx_add_attr_txt(s, "port", "all"));
                } else {
                    CX_RC(cx_add_attr_txt(s, "port", mgmt_iport_list2txt(ace.port_list, buf)));
                }
#else
                CX_RC(cx_add_attr_ulong_all(s, "port", iport2uport(ace.port_no),
                                            ace.port_no == VTSS_PORT_NO_ANY));
#endif /* VTSS_FEATURE_ACL_V2 */
                CX_RC(cx_add_attr_ulong(s, "policy", ace.policy.value));
                CX_RC(cx_add_attr_hex(s, "policy_bitmask", ace.policy.mask));
                etype = (ace.type == VTSS_ACE_TYPE_ETYPE);
                CX_RC(cx_add_attr_kw(s, "frame", cx_kw_frame_type, ace.type));
#if VTSS_SWITCH_STACKABLE
                vtss_usid_t usid = (ace.isid == VTSS_ISID_GLOBAL ? 0 : topo_isid2usid(ace.isid));
                CX_RC(cx_add_attr_ulong_any(s, "sid", usid, usid == 0));
#endif /* VTSS_SWITCH_STACKABLE */
                (void) mgmt_acl_uchar6_txt(&ace.frame.etype.dmac, buf, 1);
                if (!etype || strcmp(buf, "any") == 0) {
                    (void) mgmt_acl_dmac_txt(&ace, buf, 1);
                }
                CX_RC(cx_add_attr_txt(s, "dmac", buf));
                if (etype || ace.type == VTSS_ACE_TYPE_ARP) {
                    CX_RC(cx_add_attr_txt(s, "smac",
                                          mgmt_acl_uchar6_txt(etype ? &ace.frame.etype.smac :
                                                              &ace.frame.arp.smac, buf, 1)));
                }
#if defined(VTSS_FEATURE_ACL_V2)
                CX_RC(cx_add_attr_kw(s, "tagged", cx_kw_tagged, ace.tagged));
#endif /* VTSS_FEATURE_ACL_V2 */
                CX_RC(cx_add_attr_ulong_any(s, "vid", ace.vid.value, ace.vid.mask == 0));
                CX_RC(cx_add_attr_tag_prio(s, "tag_prio", ace.usr_prio));
                if (etype) {
                    (void) mgmt_acl_uchar2_txt(&ace.frame.etype.etype, buf, 1);
                    CX_RC(cx_add_attr_txt(s, "etype", buf));
                }
                if (ace.type == VTSS_ACE_TYPE_ARP) {
                    CX_RC(cx_add_attr_txt(s, "sip", mgmt_acl_ipv4_txt(&ace.frame.arp.sip, buf, 1)));
                    CX_RC(cx_add_attr_txt(s, "dip", mgmt_acl_ipv4_txt(&ace.frame.arp.dip, buf, 1)));
                    CX_RC(cx_add_attr_txt(s, "opcode", mgmt_acl_opcode_txt(&ace, buf, 1)));
                    CX_RC(cx_add_attr_flag(s, "request", &ace, ACE_FLAG_ARP_REQ));
                    CX_RC(cx_add_attr_flag(s, "smac", &ace, ACE_FLAG_ARP_SMAC));
                    CX_RC(cx_add_attr_flag(s, "dmac", &ace, ACE_FLAG_ARP_DMAC));
                    CX_RC(cx_add_attr_flag(s, "length", &ace, ACE_FLAG_ARP_LEN));
                    CX_RC(cx_add_attr_flag(s, "ether", &ace, ACE_FLAG_ARP_ETHER));
                    CX_RC(cx_add_attr_flag(s, "ip", &ace, ACE_FLAG_ARP_IP));
                } else if (ace.type == VTSS_ACE_TYPE_IPV4) {
                    CX_RC(cx_add_attr_txt(s, "sip", mgmt_acl_ipv4_txt(&ace.frame.ipv4.sip, buf, 1)));
                    CX_RC(cx_add_attr_txt(s, "dip", mgmt_acl_ipv4_txt(&ace.frame.ipv4.dip, buf, 1)));
                    CX_RC(cx_add_attr_txt(s, "protocol",
                                          mgmt_acl_uchar_txt(&ace.frame.ipv4.proto, buf, 1)));
                    proto = mgmt_acl_ip_proto(&ace);
                    if (proto == CX_PROTO_ICMP) {
                        /* ICMP */
                        CX_RC(cx_add_attr_ulong_any(s, "icmp_type", ace.frame.ipv4.data.value[0],
                                                    ace.frame.ipv4.data.mask[0] == 0));
                        CX_RC(cx_add_attr_ulong_any(s, "icmp_code", ace.frame.ipv4.data.value[1],
                                                    ace.frame.ipv4.data.mask[1] == 0));
                    } else if (proto == CX_PROTO_TCP || proto == CX_PROTO_UDP) {
                        /* UDP/TCP */
                        CX_RC(cx_add_attr_txt(s, "sport",
                                              mgmt_acl_port_txt(&ace.frame.ipv4.sport, buf, 1)));
                        CX_RC(cx_add_attr_txt(s, "dport",
                                              mgmt_acl_port_txt(&ace.frame.ipv4.dport, buf, 1)));
                    }
                    CX_RC(cx_add_attr_flag(s, "ttl", &ace, ACE_FLAG_IP_TTL));
                    CX_RC(cx_add_attr_flag(s, "fragment", &ace, ACE_FLAG_IP_FRAGMENT));
                    CX_RC(cx_add_attr_flag(s, "options", &ace, ACE_FLAG_IP_OPTIONS));
                    if (proto == CX_PROTO_TCP) {
                        CX_RC(cx_add_attr_flag(s, "syn", &ace, ACE_FLAG_TCP_SYN));
                        CX_RC(cx_add_attr_flag(s, "ack", &ace, ACE_FLAG_TCP_ACK));
                        CX_RC(cx_add_attr_flag(s, "fin", &ace, ACE_FLAG_TCP_FIN));
                        CX_RC(cx_add_attr_flag(s, "rst", &ace, ACE_FLAG_TCP_RST));
                        CX_RC(cx_add_attr_flag(s, "urg", &ace, ACE_FLAG_TCP_URG));
                        CX_RC(cx_add_attr_flag(s, "psh", &ace, ACE_FLAG_TCP_PSH));
                    }
#if defined(ACL_IPV6_SUPPORTED)
                } else if (ace.type == VTSS_ACE_TYPE_IPV6) {
                    ulong sip_v6_mask;
                    CX_RC(cx_add_attr_ulong_any(s, "next_header", ace.frame.ipv6.proto.value,
                                                ace.frame.ipv6.proto.mask == 0));
                    CX_RC(cx_add_attr_txt(s, "sipv6", mgmt_acl_ipv6_txt(&ace.frame.ipv6.sip, buf, 1)));
                    sip_v6_mask = (ace.frame.ipv6.sip.mask[12] << 24) +
                                  (ace.frame.ipv6.sip.mask[13] << 16) +
                                  (ace.frame.ipv6.sip.mask[14] << 8) +
                                  ace.frame.ipv6.sip.mask[15];
                    CX_RC(cx_add_attr_hex(s, "sipv6_mask", sip_v6_mask));
#if defined(VTSS_FEATURE_ACL_V2)
                    CX_RC(cx_add_attr_flag(s, "hop_limit", &ace, ACE_FLAG_IP_TTL));
                    /* IPv6 UDP/TCP */
                    proto = mgmt_acl_ip_proto(&ace);
                    if (proto == CX_PROTO_ICMPV6) {
                        /* ICMPv6 */
                        CX_RC(cx_add_attr_ulong_any(s, "icmpv6_type", ace.frame.ipv6.data.value[0],
                                                    ace.frame.ipv6.data.mask[0] == 0));
                        CX_RC(cx_add_attr_ulong_any(s, "icmpv6_code", ace.frame.ipv6.data.value[1],
                                                    ace.frame.ipv6.data.mask[1] == 0));
                    } else if (proto == CX_PROTO_TCP || proto == CX_PROTO_UDP) {
                        /* UDP/TCP */
                        CX_RC(cx_add_attr_txt(s, "sportv6",
                                              mgmt_acl_port_txt(&ace.frame.ipv6.sport, buf, 1)));
                        CX_RC(cx_add_attr_txt(s, "dportv6",
                                              mgmt_acl_port_txt(&ace.frame.ipv6.dport, buf, 1)));
                    }
                    if (proto == CX_PROTO_TCP) {
                        CX_RC(cx_add_attr_flag(s, "tcpv6_syn", &ace, ACE_FLAG_TCP_SYN));
                        CX_RC(cx_add_attr_flag(s, "tcpv6_ack", &ace, ACE_FLAG_TCP_ACK));
                        CX_RC(cx_add_attr_flag(s, "tcpv6_fin", &ace, ACE_FLAG_TCP_FIN));
                        CX_RC(cx_add_attr_flag(s, "tcpv6_rst", &ace, ACE_FLAG_TCP_RST));
                        CX_RC(cx_add_attr_flag(s, "tcpv6_urg", &ace, ACE_FLAG_TCP_URG));
                        CX_RC(cx_add_attr_flag(s, "tcpv6_psh", &ace, ACE_FLAG_TCP_PSH));
                    }
#endif /* VTSS_FEATURE_ACL_V2 */
#endif /* ACL_IPV6_SUPPORTED */
                }
                CX_RC(cx_add_action(s, &ace.action, FALSE));
                CX_RC(cx_add_attr_end(s, CX_TAG_ENTRY));
            }
            CX_RC(cx_add_tag_line(s, CX_TAG_ACL_TABLE, 1));

            CX_RC(cx_add_tag_line(s, CX_TAG_RATE_LIMITER_TABLE, 0));
            CX_RC(cx_add_table(s, NULL, VTSS_ACL_POLICER_NO_START, ACL_POLICER_NO_END,
                               cx_skip_none, cx_rl_match, cx_rl_print, TRUE));
            CX_RC(cx_add_tag_line(s, CX_TAG_RATE_LIMITER_TABLE, 1));
        }
        CX_RC(cx_add_tag_line(s, CX_TAG_ACL, 1));
        break;
    case CX_GEN_CMD_SWITCH:
        /* Switch - ACL */
        T_D("switch - acl");
        CX_RC(cx_add_tag_line(s, CX_TAG_ACL, 0));
        CX_RC(cx_add_port_table(s, s->isid, CX_TAG_PORT_TABLE, cx_acl_match, cx_acl_print));
        CX_RC(cx_add_tag_line(s, CX_TAG_ACL, 1));
        break;
    default:
        T_E("Unknown command");
        return VTSS_RC_ERROR;
    } /* End of Switch */

    return VTSS_OK;
}

/* Register the info in to the cx_module_table */
CX_MODULE_TAB_ENTRY(
    VTSS_MODULE_ID_ACL,
    acl_cx_tag_table,
    0,
    0,
    NULL,                  /* init function       */
    acl_cx_gen_func,       /* Generation fucntion */
    acl_cx_parse_func      /* parse fucntion      */
);

