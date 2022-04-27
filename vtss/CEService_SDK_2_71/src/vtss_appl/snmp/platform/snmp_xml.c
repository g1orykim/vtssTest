/*

 Vitesse Switch API software.

 Copyright (c) 2002-2012 Vitesse Semiconductor Corporation "Vitesse". All
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
#include "vtss_snmp_api.h"

#include "conf_xml_api.h"
#include "conf_xml_trace_def.h"
#include "misc_api.h"
#include "mgmt_api.h"
#include "port_api.h"

#include <network.h>         /* For INET6_ADDRSTRLEN     */
//#include "rmon_api.h"
//#include "rfc1213_mib2.h"

#undef EXTEND_RMON_TO_WEB_CLI
/* Tag IDs */
enum {
    /* Module tags */
    CX_TAG_SNMP,

    /* Group tags */
    CX_TAG_TRAP,
    CX_TAG_TRAP_TABLE,
    CX_TAG_COMMUNITY_TABLE,
    CX_TAG_USER_TABLE,
    CX_TAG_GROUP_TABLE,
    CX_TAG_VIEW_TABLE,
    CX_TAG_ACCESS_TABLE,

    /* Parameter tags */
    CX_TAG_TRAP_CONF,
    CX_TAG_GLOBAL_TRAP_EVENT,
    CX_TAG_INTF_LINKUP_TRAP_EVENT,
    CX_TAG_INTF_LINKDOWN_TRAP_EVENT,
    CX_TAG_INTF_LLDP_TRAP_EVENT,
    CX_TAG_TRAP_ENTRY,
    CX_TAG_VERSION,
    CX_TAG_ENTRY,
    CX_TAG_MODE,
    CX_TAG_READ_COMMUNITY,
    CX_TAG_WRITE_COMMUNITY,
    CX_TAG_COMMUNITY,
    CX_TAG_DESTINATION,
#ifdef VTSS_SW_OPTION_IPV6
    CX_TAG_DESTINATION_V6,
#endif /* VTSS_SW_OPTION_IPV6 */
    CX_TAG_AUTH_FAIL,
    CX_TAG_LINKUP_LINKDOWN,
    CX_TAG_INFORM_MODE,
    CX_TAG_INFORM_TIMEOUT,
    CX_TAG_INFORM_RETRY,
#ifdef SNMP_SUPPORT_V3
    CX_TAG_SECURITY_MODE,
    CX_TAG_SECURITY_ENGINE_ID,
    CX_TAG_SECURITY_NAME,
    CX_TAG_ENGINE_ID,
#endif
#ifdef EXTEND_RMON_TO_WEB_CLI
    CX_TAG_RMON_STATS_TABLE,
    CX_TAG_RMON_HISTORY_TABLE,
    CX_TAG_RMON_ALARM_TABLE,
    CX_TAG_RMON_EVENT_TABLE,
#endif
    /* Last entry */
    CX_TAG_NONE
};

/* Tag table */
static cx_tag_entry_t snmp_cx_tag_table[CX_TAG_NONE + 1] = {
    [CX_TAG_SNMP] = {
        .name  = "snmp",
        .descr = "Simple Network Management Protocol",
        .type = CX_TAG_TYPE_MODULE
    },
    [CX_TAG_TRAP] = {
        .name  = "trap",
        .descr = "",
        .type = CX_TAG_TYPE_GROUP
    },
    [CX_TAG_COMMUNITY_TABLE] = {
        .name  = "community_table",
        .descr = "",
        .type = CX_TAG_TYPE_GROUP
    },
    [CX_TAG_USER_TABLE] = {
        .name  = "user_table",
        .descr = "",
        .type = CX_TAG_TYPE_GROUP
    },
    [CX_TAG_GROUP_TABLE] = {
        .name  = "group_table",
        .descr = "",
        .type = CX_TAG_TYPE_GROUP
    },
    [CX_TAG_VIEW_TABLE] = {
        .name  = "view_table",
        .descr = "",
        .type = CX_TAG_TYPE_GROUP
    },
    [CX_TAG_ACCESS_TABLE] = {
        .name  = "access_table",
        .descr = "",
        .type = CX_TAG_TYPE_GROUP
    },
    [CX_TAG_TRAP_TABLE] = {
        .name  = "trap_table",
        .descr = "",
        .type = CX_TAG_TYPE_GROUP
    },
    [CX_TAG_TRAP_CONF] = {
        .name  = "trap_conf",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_GLOBAL_TRAP_EVENT] = {
        .name  = "trap_global_event",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_INTF_LINKUP_TRAP_EVENT] = {
        .name  = "trap_intf_linkup_event",
        .descr = "",
        .type = CX_TAG_TYPE_GROUP
    },
    [CX_TAG_INTF_LINKDOWN_TRAP_EVENT] = {
        .name  = "trap_intf_linkdown_event",
        .descr = "",
        .type = CX_TAG_TYPE_GROUP
    },
    [CX_TAG_INTF_LLDP_TRAP_EVENT] = {
        .name  = "trap_intf_lldp_event",
        .descr = "",
        .type = CX_TAG_TYPE_GROUP
    },
    [CX_TAG_TRAP_ENTRY] = {
        .name  = "trap_entry",
        .descr = "",
//        .type = CX_TAG_TYPE_SECTION
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_VERSION] = {
        .name  = "version",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_ENTRY] = {
        .name  = "entry",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_MODE] = {
        .name  = "mode",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_READ_COMMUNITY] = {
        .name  = "read_community",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_WRITE_COMMUNITY] = {
        .name  = "write_community",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_COMMUNITY] = {
        .name  = "community",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_DESTINATION] = {
        .name  = "destination",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
#ifdef VTSS_SW_OPTION_IPV6
    [CX_TAG_DESTINATION_V6] = {
        .name  = "destination_ipv6",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
#endif /* VTSS_SW_OPTION_IPV6 */
    [CX_TAG_AUTH_FAIL] = {
        .name  = "auth_fail",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_LINKUP_LINKDOWN] = {
        .name  = "linkup_linkdown",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_INFORM_MODE] = {
        .name  = "inform_mode",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_INFORM_TIMEOUT] = {
        .name  = "inform_timeout",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_INFORM_RETRY] = {
        .name  = "inform_retry",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
#ifdef SNMP_SUPPORT_V3
    [CX_TAG_SECURITY_MODE] = {
        .name  = "security_mode",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_SECURITY_ENGINE_ID] = {
        .name  = "security_engine_id",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_SECURITY_NAME] = {
        .name  = "security_name",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_ENGINE_ID] = {
        .name  = "engine_id",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
#endif

#ifdef EXTEND_RMON_TO_WEB_CLI
    [CX_TAG_RMON_STATS_TABLE] = {
        .name  = "rmon_stats_table",
        .descr = "",
        .type = CX_TAG_TYPE_GROUP
    },

    [CX_TAG_RMON_HISTORY_TABLE] = {
        .name  = "rmon_history_table",
        .descr = "",
        .type = CX_TAG_TYPE_GROUP
    },

    [CX_TAG_RMON_ALARM_TABLE] = {
        .name  = "rmon_alarm_table",
        .descr = "",
        .type = CX_TAG_TYPE_GROUP
    },

    [CX_TAG_RMON_EVENT_TABLE] = {
        .name  = "rmon_event_table",
        .descr = "",
        .type = CX_TAG_TYPE_GROUP
    },

#endif
    /* Last entry */
    [CX_TAG_NONE] = {
        .name  = "",
        .descr = "",
        .type = CX_TAG_TYPE_NONE
    }
};

/* SNMP specific set state structure */
typedef struct {
#ifdef SNMP_SUPPORT_V3
    /* SNMPv3 tables */
    struct {
        int                       count;
        snmpv3_communities_conf_t entry[SNMPV3_MAX_COMMUNITIES];
    } communities;

    struct {
        int                 count;
        snmpv3_users_conf_t entry[SNMPV3_MAX_USERS];
    } users;

    struct {
        int                  count;
        snmpv3_groups_conf_t entry[SNMPV3_MAX_GROUPS];
        cx_line_t            line[SNMPV3_MAX_GROUPS];
    } groups;

    struct {
        int                 count;
        snmpv3_views_conf_t entry[SNMPV3_MAX_VIEWS];
    } views;

    struct {
        int                    count;
        snmpv3_accesses_conf_t entry[SNMPV3_MAX_ACCESSES];
        cx_line_t              line[SNMPV3_MAX_ACCESSES];
    } accesses;
#endif /* SNMP_SUPPORT_V3 */
    struct {
        int                    count;
        vtss_trap_entry_t entry[VTSS_TRAP_CONF_MAX];
        cx_line_t              line[VTSS_TRAP_CONF_MAX];
    } trap;
} snmp_cx_set_state_t;

#ifdef VTSS_SW_OPTION_SNMP
/* Keyword for SNMP version */
static const cx_kw_t cx_kw_snmp_version[] = {
    { "1",  SNMP_SUPPORT_V1 },
    { "2c", SNMP_SUPPORT_V2C },
#ifdef SNMP_SUPPORT_V3
    { "3",  SNMP_SUPPORT_V3 },
#endif /* SNMP_SUPPORT_V3 */
    { NULL, 0 }
};


static const cx_kw_t cx_kw_snmp_inform[] = {
    { "trap",  SNMP_SUPPORT_V1 },
    { "informs", SNMP_SUPPORT_V2C },
    { NULL, 0 }
};

/* Keyword for SNMP authentication */
static const cx_kw_t cx_kw_snmp_auth[] = {
    { "disabled", SNMP_MGMT_AUTH_PROTO_NONE },
    { "md5",      SNMP_MGMT_AUTH_PROTO_MD5 },
    { "sha",      SNMP_MGMT_AUTH_PROTO_SHA },
    { NULL,       0 }
};

/* Keyword for SNMP privacy */
static const cx_kw_t cx_kw_snmp_priv[] = {
    { "disabled", SNMP_MGMT_PRIV_PROTO_NONE },
    { "des",      SNMP_MGMT_PRIV_PROTO_DES },
    { "aes",      SNMP_MGMT_PRIV_PROTO_AES },
    { NULL,       0 }
};

/* Keyword for SNMP view */
static const cx_kw_t cx_kw_snmp_view[] = {
    { "included", SNMPV3_MGMT_VIEW_INCLUDED },
    { "excluded", SNMPV3_MGMT_VIEW_EXCLUDED },
    { NULL,       0 }
};

/* Keyword for SNMP security model */
static const cx_kw_t cx_kw_snmp_model[] = {
    { "any", SNMP_MGMT_SEC_MODEL_ANY },
    { "v1",  SNMP_MGMT_SEC_MODEL_SNMPV1 },
    { "v2c", SNMP_MGMT_SEC_MODEL_SNMPV2C },
    { "usm", SNMP_MGMT_SEC_MODEL_USM },
    { NULL,       0 }
};

/* Keyword for SNMP security level */
static const cx_kw_t cx_kw_snmp_level[] = {
    { "disabled", SNMP_MGMT_SEC_LEVEL_NOAUTH },
    { "enabled",  SNMP_MGMT_SEC_LEVEL_AUTHPRIV },
    { "auth",     SNMP_MGMT_SEC_LEVEL_AUTHNOPRIV },
    { NULL,       0 }
};
#endif /* VTSS_SW_OPTION_SNMP */

/*lint -esym(459,vtss_trap_entry)*/
static vtss_trap_entry_t vtss_trap_entry;
/*lint -esym(459,parse_trap_entry)*/
static vtss_trap_entry_t parse_trap_entry;

#ifdef SNMP_SUPPORT_V3
static char *cx_engine_id_txt = "10-64 hexadecimal digit characters (0-9/a-f/A-F)";

/* Add engine ID */
static vtss_rc cx_add_val_engine_id(cx_get_state_t *s, cx_tag_id_t id,
                                    uchar *engine_id, ulong len)
{
    return cx_add_val_txt(s, id, misc_engineid2str(engine_id, len), cx_engine_id_txt);
}
#endif /* SNMP_SUPPORT_V3 */

static char *cx_stx_snmp_txt(char *buf, ulong min, ulong max)
{
    sprintf(buf, "%u-%u characters (spaces not allowed)", min, max);
    return buf;
}

#ifdef SNMP_SUPPORT_V3
static char *cx_stx_snmp_name(char *buf)
{
    return cx_stx_snmp_txt(buf, SNMPV3_MIN_NAME_LEN, SNMPV3_MAX_NAME_LEN);
}

/* Parse SNMPv3 admin string */
static vtss_rc cx_parse_snmp_name(cx_set_state_t *s, char *name, char *snmp_name)
{
    CX_RC(cx_parse_txt(s, name, snmp_name, SNMPV3_MAX_NAME_LEN + 1));
    if (!snmpv3_is_admin_string(snmp_name)) {
        CX_RC(cx_parm_invalid(s));
    }
    return s->rc;
}

/* Parse SNMPv3 password string */
static vtss_rc cx_parse_snmp_password(cx_set_state_t *s, char *name, char *password)
{
    int i, len, error;

    CX_RC(cx_parse_txt(s, name, password, SNMPV3_MAX_SHA_PASSWORD_LEN + 1));
    len = strlen(password);
    error = (len != 0 && len < SNMPV3_MIN_PASSWORD_LEN);
    for (i = 0; !error && i < len; i++) {
        if (password[i] == ' ') {
            error = 1;
        }
    }
    if (error) {
        CX_RC(cx_parm_invalid(s));
    }
    return s->rc;
}

#if 0
/* Parse engine ID */
static vtss_rc cx_parse_engine_id_no_check(cx_set_state_t *s, char *name, uchar *id, ulong *len)
{
    int  i, c, sum = 0, error;

    CX_RC(cx_parse_attr_name(s, name));
    *len = s->val_len / 2;
    error = ((s->val_len & 1) != 0 || *len > SNMPV3_MAX_ENGINE_ID_LEN);
    for (i = 0; !error && i < s->val_len; i++) {
        c = tolower(s->val[i]);
        if (isxdigit(c)) {
            c -= (isdigit(c) ? '0' : ('a' - 10));
            if (i & 1) {
                id[i / 2] = (sum + c);
            } else {
                sum = (c * 16);
            }
        } else {
            error = 1;
        }
    }
    if (error || !snmpv3_is_valid_engineid(id, *len)) {
        CX_RC(cx_parm_invalid(s));
    }
    return s->rc;
}
#endif

/* Parse engine ID */
static vtss_rc cx_parse_engine_id(cx_set_state_t *s, char *name, uchar *id, ulong *len)
{
    int  i, c, sum = 0, error;

    CX_RC(cx_parse_attr_name(s, name));
    *len = s->val_len / 2;
    error = ((s->val_len & 1) != 0 || *len > SNMPV3_MAX_ENGINE_ID_LEN);
    for (i = 0; !error && i < s->val_len; i++) {
        c = tolower(s->val[i]);
        if (isxdigit(c)) {
            c -= (isdigit(c) ? '0' : ('a' - 10));
            if (i & 1) {
                id[i / 2] = (sum + c);
            } else {
                sum = (c * 16);
            }
        } else {
            error = 1;
        }
    }
    if (error || !snmpv3_is_valid_engineid(id, *len)) {
        CX_RC(cx_parm_invalid(s));
    }
    return s->rc;
}

/* Parse engine ID name 'val' */
static vtss_rc cx_parse_val_engine_id(cx_set_state_t *s, uchar *id, ulong *len)
{
    return cx_parse_engine_id(s, "val", id, len);
}

/* Parse SNMP security model */
static vtss_rc cx_parse_sec_model(cx_set_state_t *s, BOOL any, ulong *val)
{
    CX_RC(cx_parse_kw(s, "security_model", cx_kw_snmp_model, val, 1));
    if (!any && *val == SNMP_MGMT_SEC_MODEL_ANY) {
        CX_RC(cx_parm_invalid(s));
    }
    return s->rc;
}
#endif /* SNMP_SUPPORT_V3 */

static void give_default_trap_entry(i8 *conf_name)
{
    vtss_trap_conf_t          *conf = &parse_trap_entry.trap_conf;
    vtss_trap_event_t         *event = &parse_trap_entry.trap_event;

    strcpy(vtss_trap_entry.trap_conf_name, conf_name);
    conf->enable = TRAP_CONF_DEFAULT_ENABLE;
    conf->dip.ipv6_flag = FALSE;
    strcpy(conf->dip.addr.ipv4_str, TRAP_CONF_DEFAULT_DIP);
    conf->trap_port = TRAP_CONF_DEFAULT_DPORT;
    conf->trap_version = TRAP_CONF_DEFAULT_VER;
    strcpy(conf->trap_community, TRAP_CONF_DEFAULT_COMM);
    conf->trap_inform_mode = TRAP_CONF_DEFAULT_INFORM_MODE;
    conf->trap_inform_timeout = TRAP_CONF_DEFAULT_INFORM_TIMEOUT ;
    conf->trap_inform_retries = TRAP_CONF_DEFAULT_INFORM_RETRIES;
#ifdef SNMP_SUPPORT_V3
    conf->trap_probe_engineid = TRAP_CONF_DEFAULT_PROBE_ENG;
    memset(conf->trap_engineid, 0, SNMPV3_MAX_ENGINE_ID_LEN);
    strcpy(conf->trap_security_name, TRAP_CONF_DEFAULT_SEC_NAME);
#endif /* SNMP_SUPPORT_V3 */

    event->system.warm_start = FALSE;
    event->system.cold_start = FALSE;
    event->aaa.trap_authen_fail     = FALSE;
    event->sw.stp            = FALSE;
    event->sw.rmon           = FALSE;

    memset( event->interface.trap_linkup  , 0, sizeof(event->interface.trap_linkup));
    memset( event->interface.trap_linkdown, 0, sizeof(event->interface.trap_linkdown));
    memset( event->interface.trap_lldp     , 0, sizeof(event->interface.trap_lldp));

}


static vtss_rc snmp_cx_parse_func(cx_set_state_t *s)
{
#ifdef SNMP_SUPPORT_V3
    snmpv3_communities_conf_t community;
    snmpv3_users_conf_t       user;
    snmpv3_groups_conf_t      group;
    snmpv3_views_conf_t       view;
    snmpv3_accesses_conf_t    access;
#endif /* SNMP_SUPPORT_V3 */
    int                       i;
    vtss_trap_conf_t          *trap_conf = &parse_trap_entry.trap_conf;
    vtss_trap_event_t         *trap_event = &parse_trap_entry.trap_event;
    char                      buf[40] = {'\0'};


    switch (s->cmd) {
    case CX_PARSE_CMD_PARM: {
        ulong       val;
        BOOL        global;
        snmp_conf_t conf;
        BOOL        mode;
        char        *p;
        snmp_cx_set_state_t *snmp_state = s->mod_state;
        BOOL        port_list[VTSS_PORT_ARRAY_SIZE + 1];

        global = (s->isid == VTSS_ISID_GLOBAL);

        memset(&conf, 0, sizeof(conf));
        if (s->apply && snmp_mgmt_snmp_conf_get(&conf) != VTSS_OK) {
            break;
        }

        switch (s->id) {
        case CX_TAG_MODE:
            CX_RC(cx_parse_val_bool(s, &mode, 1));
            if (s->group == CX_TAG_TRAP) {

                ( void ) trap_mgmt_mode_set(mode);
            } else {
                conf.mode = mode;
            }
            break;
        case CX_TAG_VERSION:
            if (cx_parse_val_kw(s, cx_kw_snmp_version, &val, 1) == VTSS_OK) {
                conf.version = val;
            }
            break;
        case CX_TAG_INTF_LINKUP_TRAP_EVENT:
        case CX_TAG_INTF_LINKDOWN_TRAP_EVENT:
        case CX_TAG_INTF_LLDP_TRAP_EVENT:
            break;
        case CX_TAG_READ_COMMUNITY:
        case CX_TAG_WRITE_COMMUNITY:
        case CX_TAG_COMMUNITY:
            p = (s->id ==  CX_TAG_READ_COMMUNITY ? conf.read_community :
                 s->id ==  CX_TAG_WRITE_COMMUNITY ? conf.write_community :
                 conf.trap_community);
            if (cx_parse_val_txt(s, p, SNMP_MGMT_MAX_COMMUNITY_LEN) == VTSS_OK) {
                for ( ; *p != '\0'; p++) {
                    if (*p == ' ') { /* Spaces not allowed */
                        CX_RC(cx_parm_invalid(s));
                    }
                }
            }
            break;
        case CX_TAG_DESTINATION:
            p = conf.trap_dip_string;
            if (cx_parse_val_txt(s, p, INET6_ADDRSTRLEN) == VTSS_OK) {
                for ( ; *p != '\0'; p++) {
                    if (*p == ' ') { /* Spaces not allowed */
                        CX_RC(cx_parm_invalid(s));
                    }
                }
            }
            break;
#ifdef VTSS_SW_OPTION_IPV6
        case CX_TAG_DESTINATION_V6:
            CX_RC(cx_parse_val_ipv6(s, &conf.trap_dipv6));
            break;
#endif /* VTSS_SW_OPTION_IPV6 */
        case CX_TAG_AUTH_FAIL:
            CX_RC(cx_parse_val_bool(s, &mode, 1));
            conf.trap_authen_fail = mode;
            break;
        case CX_TAG_LINKUP_LINKDOWN:
            CX_RC(cx_parse_val_bool(s, &mode, 1));
            conf.trap_linkup_linkdown = mode;
            break;
        case CX_TAG_INFORM_MODE:
            CX_RC(cx_parse_val_bool(s, &mode, 1));
            conf.trap_inform_mode = mode;
            break;
        case CX_TAG_INFORM_TIMEOUT:
            CX_RC(cx_parse_val_ulong(s, &conf.trap_inform_timeout,
                                     0, SNMP_MGMT_MAX_TRAP_INFORM_TIMEOUT));
            break;
        case CX_TAG_INFORM_RETRY:
            CX_RC(cx_parse_val_ulong(s, &conf.trap_inform_retries,
                                     0, SNMP_MGMT_MAX_TRAP_INFORM_RETRIES));
            break;
        case CX_TAG_TRAP:
            break;
        case CX_TAG_TRAP_TABLE:
            /* Flush access table */
            snmp_state->trap.count = 0;
            break;
        case CX_TAG_TRAP_ENTRY:

            CX_RC(cx_parse_txt(s, "conf_name", parse_trap_entry.trap_conf_name, TRAP_MAX_NAME_LEN));
            parse_trap_entry.trap_conf_name[TRAP_MAX_NAME_LEN] = 0;
//            T_E("conf_name = %s", parse_trap_entry.trap_conf_name);
            parse_trap_entry.valid = FALSE;
            break;
        case CX_TAG_TRAP_CONF:
//            T_E("conf_name = %s", parse_trap_entry.trap_conf_name);
            give_default_trap_entry(parse_trap_entry.trap_conf_name);

            for (; s->rc == VTSS_OK && cx_parse_attr(s) == VTSS_OK; s->p = s->next) {
                if ( VTSS_RC_OK == cx_parse_bool(s, "mode", &trap_conf->enable, TRUE)) {
                    continue;
                } else if ( VTSS_RC_OK == cx_parse_txt(s, "dip", buf, 40)) {
#ifdef VTSS_SW_OPTION_IPV6
                    if ( VTSS_OK == mgmt_txt2ipv6(buf, &trap_conf->dip.addr.ipv6)) {
                        trap_conf->dip.ipv6_flag = TRUE;
                    } else {
                        trap_conf->dip.ipv6_flag = FALSE;
                        strncpy(trap_conf->dip.addr.ipv4_str, buf, VTSS_SYS_HOSTNAME_LEN);
                    }
#else
                    trap_conf->dip.ipv6_flag = FALSE;
                    strncpy(trap_conf->dip.addr.ipv4_str, buf, VTSS_SYS_HOSTNAME_LEN);
#endif /* VTSS_SW_OPTION_IPV6 */
                    continue;
                } else if ( VTSS_RC_OK == cx_parse_ulong(s, "dport", (ulong *)&trap_conf->trap_port, 1, 65535)) {
                    continue;
                } else if ( VTSS_RC_OK == cx_parse_kw(s, "version", cx_kw_snmp_version, &trap_conf->trap_version, 1)) {
                    continue;
                } else if ( VTSS_RC_OK == cx_parse_kw(s, "inform", cx_kw_snmp_inform, &trap_conf->trap_inform_mode, 1)) {
                    continue;
                } else if ( VTSS_RC_OK == cx_parse_ulong(s, "timeout", &trap_conf->trap_inform_timeout, 0, SNMP_MGMT_MAX_TRAP_INFORM_TIMEOUT)) {
                    continue;
                } else if ( VTSS_RC_OK == cx_parse_ulong(s, "retries", &trap_conf->trap_inform_retries, 0, SNMP_MGMT_MAX_TRAP_INFORM_RETRIES)) {
                    continue;
                } else if ( VTSS_RC_OK == cx_parse_txt(s, "community", trap_conf->trap_community, SNMP_MGMT_MAX_COMMUNITY_LEN)) {
                    continue;
#ifdef SNMP_SUPPORT_V3
                } else if ( VTSS_RC_OK == cx_parse_bool(s, "probe", &trap_conf->trap_probe_engineid, TRUE)) {
                    continue;
                } else if ( trap_conf->trap_probe_engineid == FALSE &&
                            VTSS_RC_OK == cx_parse_engine_id(s, "engine_id", trap_conf->trap_engineid, &trap_conf->trap_engineid_len) ) {
                    continue;
                } else if ( trap_conf->trap_version == SNMP_SUPPORT_V3 && cx_parse_snmp_name(s, "security_name", trap_conf->trap_security_name)) {
                    continue;
#endif /* SNMP_SUPPORT_V3 */
                }
            }

#if 0
            T_E("trap_conf_name = %s, trap_conf->enable = %d, trap_conf->trap_dst_host_name = %s, trap_conf->trap_port = %d, trap_conf->trap_version = %d,"
                "trap_conf->trap_community = %s, trap_conf->trap_inform_mode = %d, trap_conf->trap_inform_retries = %d,"
                "trap_conf->trap_inform_timeout = %d, trap_conf->trap_probe_engineid = %d, trap_conf->trap_security_name = %s",
                parse_trap_entry.trap_conf_name, trap_conf->enable, trap_conf->trap_dst_host_name, trap_conf->trap_port, trap_conf->trap_version, trap_conf->trap_community,
                trap_conf->trap_inform_mode, trap_conf->trap_inform_retries, trap_conf->trap_inform_timeout, trap_conf->trap_probe_engineid, trap_conf->trap_security_name);
#endif

            parse_trap_entry.valid = TRUE;

            if (trap_mgmt_conf_set(&parse_trap_entry) != VTSS_RC_OK) {
                T_E("set fail");
            }
//            T_E("%s parse OK", parse_trap_entry.trap_conf_name);
            break;
        case CX_TAG_GLOBAL_TRAP_EVENT:
            (void) trap_mgmt_conf_get(&parse_trap_entry);
            trap_event->system.warm_start  = 0;
            trap_event->system.cold_start  = 0;
            trap_event->aaa.trap_authen_fail   = 0;
            trap_event->sw.stp         = 0;
            trap_event->sw.rmon        = 0;

            for (; s->rc == VTSS_OK && cx_parse_attr(s) == VTSS_OK; s->p = s->next) {
                if ( VTSS_RC_OK == cx_parse_bool(s, "warm_start", &trap_event->system.warm_start, TRUE)) {
                    continue;
                } else if ( VTSS_RC_OK == cx_parse_bool(s, "cold_start", &trap_event->system.cold_start, TRUE)) {
                    continue;
                } else if ( VTSS_RC_OK == cx_parse_bool(s, "auth_fail", &trap_event->aaa.trap_authen_fail, TRUE)) {
                    continue;
                } else if ( VTSS_RC_OK == cx_parse_bool(s, "stp", &trap_event->sw.stp, TRUE)) {
                    continue;
                } else if ( VTSS_RC_OK == cx_parse_bool(s, "rmon", &trap_event->sw.rmon, TRUE)) {
                    continue;
                }
            }

            if (trap_mgmt_conf_set(&parse_trap_entry) != VTSS_RC_OK) {
                T_E("set fail");
            }
//            T_E("%s parse OK", parse_trap_entry.trap_conf_name);

            break;
#ifdef SNMP_SUPPORT_V3
        case CX_TAG_SECURITY_MODE:
            CX_RC(cx_parse_val_bool(s, &mode, 1));
            conf.trap_probe_security_engineid = mode;
            break;
        case CX_TAG_SECURITY_NAME:
            CX_RC(cx_parse_snmp_name(s, "val", conf.trap_security_name));
            break;
        case CX_TAG_ENGINE_ID:
            CX_RC(cx_parse_val_engine_id(s, conf.engineid, &conf.engineid_len));
            break;
        case CX_TAG_SECURITY_ENGINE_ID:
            if (snmp_mgmt_snmp_conf_get(&conf) == VTSS_OK) {
                if (conf.trap_probe_security_engineid == SNMP_MGMT_DISABLED) {
                    CX_RC(cx_parse_val_engine_id(s, conf.trap_security_engineid,
                                                 &conf.trap_security_engineid_len));
                }
            }
            break;
        case CX_TAG_COMMUNITY_TABLE:
            /* Flush community table */
            snmp_state->communities.count = 0;
            break;
        case CX_TAG_USER_TABLE:
            /* Flush user table */
            snmp_state->users.count = 0;
            break;
        case CX_TAG_GROUP_TABLE:
            /* Flush group table */
            snmp_state->groups.count = 0;
            break;
        case CX_TAG_VIEW_TABLE:
            /* Flush view table */
            snmp_state->views.count = 0;
            break;
        case CX_TAG_ACCESS_TABLE:
            /* Flush access table */
            snmp_state->accesses.count = 0;
            break;
        case CX_TAG_ENTRY:
            if (s->group == CX_TAG_COMMUNITY_TABLE) {
                snmpv3_communities_conf_t community_conf;

                memset(&community_conf, 0x0, sizeof(community_conf));
                community_conf.storage_type = SNMP_MGMT_STORAGE_PERMANENT;
                community_conf.status = SNMP_MGMT_ROW_ACTIVE;
                for (; s->rc == VTSS_OK && cx_parse_attr(s) == VTSS_OK; s->p = s->next) {
                    if (cx_parse_snmp_name(s, "community", community_conf.community) == VTSS_OK) {
                        community_conf.valid = 1;
                    } else if (cx_parse_ipv4(s, "source_mask", &community_conf.sip_mask, NULL, 1) == VTSS_OK) {
                        if (community_conf.sip_mask == 0xffffffff) {
                            CX_RC(cx_parm_invalid(s));
                        }
                    } else if (cx_parse_ipv4(s, "source_ip", &community_conf.sip, NULL, 0) != VTSS_OK) {
                        CX_RC(cx_parm_unknown(s));
                    }
                }
                if (community_conf.valid == 0) {
                    CX_RC(cx_parm_found_error(s, "community"));
                }
                if (snmp_state->communities.count < SNMPV3_MAX_COMMUNITIES) {
                    snmp_state->communities.entry[snmp_state->communities.count++] = community_conf;
                }
            } else if (s->group == CX_TAG_USER_TABLE) {
                snmpv3_users_conf_t user_conf;
                BOOL                engine = 0, uname = 0, auth = 0, priv = 0;

                memset(&user_conf, 0x0, sizeof(user_conf));
                user_conf.valid = 1;
                user_conf.storage_type = SNMP_MGMT_STORAGE_PERMANENT;
                user_conf.status = SNMP_MGMT_ROW_ACTIVE;
                user_conf.auth_protocol = SNMP_MGMT_AUTH_PROTO_NONE;
                user_conf.priv_protocol = SNMP_MGMT_PRIV_PROTO_NONE;
                for (; s->rc == VTSS_OK && cx_parse_attr(s) == VTSS_OK; s->p = s->next) {
                    if (cx_parse_engine_id(s, "engine_id", user_conf.engineid,
                                           &user_conf.engineid_len) == VTSS_OK) {
                        engine = 1;
                    } else if (cx_parse_snmp_name(s, "user_name", user_conf.user_name) == VTSS_OK) {
                        uname = 1;
                    } else if (cx_parse_kw(s, "auth_proto", cx_kw_snmp_auth, &val,
                                           1) == VTSS_OK) {
                        auth = 1;
                        user_conf.auth_protocol = val;
                    } else if (cx_parse_kw(s, "priv_proto", cx_kw_snmp_priv, &val,
                                           1) == VTSS_OK) {
                        priv = 1;
                        user_conf.priv_protocol = val;
                    } else if (cx_parse_snmp_password(s, "auth_password",
                                                      user_conf.auth_password) != VTSS_OK &&
                               cx_parse_snmp_password(s, "priv_password",
                                                      user_conf.priv_password) != VTSS_OK) {
                        CX_RC(cx_parm_unknown(s));
                    }
                }
                if (s->rc != VTSS_OK) {
                    break;
                }
                if (engine == 0) {
                    CX_RC(cx_parm_found_error(s, "engine_id"));
                }
                if (uname == 0) {
                    CX_RC(cx_parm_found_error(s, "user_name"));
                }

                if (strlen(user_conf.auth_password) == 0) {
                    if (user_conf.auth_protocol != SNMP_MGMT_AUTH_PROTO_NONE) {
                        CX_RC(cx_parm_found_error(s, "auth_password"));
                    }
                } else if (auth == 0) {
                    user_conf.auth_protocol = SNMP_MGMT_AUTH_PROTO_MD5;
                }

                if (strlen(user_conf.priv_password) == 0) {
                    if (user_conf.priv_protocol != SNMP_MGMT_PRIV_PROTO_NONE) {
                        CX_RC(cx_parm_found_error(s, "priv_password"));
                    }
                } else if (priv == 0) {
                    user_conf.priv_protocol = SNMP_MGMT_PRIV_PROTO_DES;
                }

                if (user_conf.auth_protocol == SNMP_MGMT_AUTH_PROTO_NONE) {
                    if (user_conf.priv_protocol == SNMP_MGMT_PRIV_PROTO_NONE) {
                        user_conf.security_level = SNMP_MGMT_SEC_LEVEL_NOAUTH;
                    } else {
                        CX_RC(cx_parm_error(s, "Privacy protocol requires Authentication protocol"));
                    }
                } else {
                    if (user_conf.priv_protocol == SNMP_MGMT_PRIV_PROTO_NONE) {
                        user_conf.security_level = SNMP_MGMT_SEC_LEVEL_AUTHNOPRIV;
                    } else {
                        user_conf.security_level = SNMP_MGMT_SEC_LEVEL_AUTHPRIV;
                    }
                }
                if (snmp_state->users.count < SNMPV3_MAX_USERS) {
                    snmp_state->users.entry[snmp_state->users.count++] = user_conf;
                }
            } else if (s->group == CX_TAG_GROUP_TABLE) {
                snmpv3_groups_conf_t group_conf;

                memset(&group_conf, 0x0, sizeof(group_conf));
                group_conf.storage_type = SNMP_MGMT_STORAGE_PERMANENT;
                group_conf.status = SNMP_MGMT_ROW_ACTIVE;
                for (; s->rc == VTSS_OK && cx_parse_attr(s) == VTSS_OK; s->p = s->next) {
                    if (cx_parse_sec_model(s, 0, &group_conf.security_model) == VTSS_OK) {
                        group_conf.valid = 1;
                    } else if (cx_parse_snmp_name(s, "security_name",
                                                  group_conf.security_name) != VTSS_OK &&
                               cx_parse_snmp_name(s, "group_name", group_conf.group_name) != VTSS_OK) {
                        CX_RC(cx_parm_unknown(s));
                    }
                }
                if (group_conf.valid == 0) {
                    CX_RC(cx_parm_found_error(s, "security_model"));
                }
                if (strlen(group_conf.security_name) == 0) {
                    CX_RC(cx_parm_found_error(s, "security_name"));
                }
                if (strlen(group_conf.group_name) == 0) {
                    CX_RC(cx_parm_found_error(s, "group_name"));
                }
                if (snmp_state->groups.count < SNMPV3_MAX_GROUPS) {
                    snmp_state->groups.line[snmp_state->groups.count] = s->line;
                    snmp_state->groups.entry[snmp_state->groups.count++] = group_conf;
                }
            } else if (s->group == CX_TAG_VIEW_TABLE) {
                snmpv3_views_conf_t view_conf;
                BOOL                type = 0, oid_flag = 0;

                memset(&view_conf, 0x0, sizeof(view_conf));
                view_conf.storage_type = SNMP_MGMT_STORAGE_PERMANENT;
                view_conf.status = SNMP_MGMT_ROW_ACTIVE;

                for (; s->rc == VTSS_OK && cx_parse_attr(s) == VTSS_OK; s->p = s->next) {
                    if (cx_parse_snmp_name(s, "view_name", view_conf.view_name) == VTSS_OK) {
                        view_conf.valid = 1;
                    } else if (cx_parse_kw(s, "view_type", cx_kw_snmp_view, &val,
                                           1) == VTSS_OK) {
                        type = 1;
                        view_conf.view_type = val;
                    } else if (cx_parse_attr_name(s, "oid_subtree") == VTSS_OK) {
                        oid_flag = 1;
                        if (mgmt_txt2oid(s->val, s->val_len, view_conf.subtree,
                                         view_conf.subtree_mask, &view_conf.subtree_len) != VTSS_OK) {
                            CX_RC(cx_parm_invalid(s));
                        }
                        view_conf.subtree_mask_len = view_conf.subtree_len;
                    } else {
                        CX_RC(cx_parm_unknown(s));
                    }
                }
                if (view_conf.valid == 0) {
                    CX_RC(cx_parm_found_error(s, "view_name"));
                }
                if (type == 0) {
                    CX_RC(cx_parm_found_error(s, "view_type"));
                }
                if (oid_flag == 0) {
                    CX_RC(cx_parm_found_error(s, "oid_subtree"));
                }
                if (snmp_state->views.count < SNMPV3_MAX_VIEWS) {
                    snmp_state->views.entry[snmp_state->views.count++] = view_conf;
                }
            } else if (s->group == CX_TAG_ACCESS_TABLE) {
                snmpv3_accesses_conf_t access_conf;
                BOOL                   model = 0, level = 0;

                memset(&access_conf, 0x0, sizeof(access_conf));
                access_conf.context_match = SNMPV3_MGMT_CONTEX_MATCH_EXACT;
                access_conf.storage_type = SNMP_MGMT_STORAGE_PERMANENT;
                access_conf.status = SNMP_MGMT_ROW_ACTIVE;
                for (; s->rc == VTSS_OK && cx_parse_attr(s) == VTSS_OK; s->p = s->next) {
                    if (cx_parse_snmp_name(s, "group_name", access_conf.group_name) == VTSS_OK) {
                        access_conf.valid = 1;
                    } else if (cx_parse_sec_model(s, 1, &access_conf.security_model) == VTSS_OK) {
                        model = 1;
                    } else if (cx_parse_kw(s, "security_level", cx_kw_snmp_level, &val,
                                           1) == VTSS_OK) {
                        level = 1;
                        access_conf.security_level = val;
                    } else if (cx_parse_snmp_name(s, "read_view_name",
                                                  access_conf.read_view_name) != VTSS_OK &&
                               cx_parse_snmp_name(s, "write_view_name",
                                                  access_conf.write_view_name) != VTSS_OK) {
                        CX_RC(cx_parm_unknown(s));
                    }
                }
                if (access_conf.valid == 0) {
                    CX_RC(cx_parm_found_error(s, "group_name"));
                }
                if (model == 0) {
                    CX_RC(cx_parm_found_error(s, "security_model"));
                }
                if (level == 0) {
                    CX_RC(cx_parm_found_error(s, "security_level"));
                }
                if (strlen(access_conf.read_view_name) == 0) {
                    strcpy(access_conf.read_view_name, SNMPV3_NONAME);
                }
                if (strlen(access_conf.write_view_name) == 0) {
                    strcpy(access_conf.write_view_name, SNMPV3_NONAME);
                }
                strcpy(access_conf.notify_view_name, SNMPV3_NONAME);
                if (snmp_state->accesses.count < SNMPV3_MAX_ACCESSES) {
                    snmp_state->accesses.line[snmp_state->accesses.count] = s->line;
                    snmp_state->accesses.entry[snmp_state->accesses.count++] = access_conf;
                }
#endif /* SNMP_SUPPORT_V3 */
            } else if ( !global &&
                        ( s->group == CX_TAG_INTF_LINKUP_TRAP_EVENT || s->group == CX_TAG_INTF_LINKDOWN_TRAP_EVENT || s->group == CX_TAG_INTF_LLDP_TRAP_EVENT) &&
                        cx_parse_ports(s, port_list, 1) == VTSS_OK ) {

                vtss_trap_entry_t trap_entry;
                BOOL                     *intf_event = NULL;
                port_iter_t              pit;

                s->p = s->next;
                for (; s->rc == VTSS_OK && cx_parse_attr(s) == VTSS_OK; s->p = s->next) {
                    if (cx_parse_snmp_name(s, "conf_name", trap_entry.trap_conf_name) == VTSS_OK) {
                        continue;
                    } else if (cx_parse_bool(s, "mode", &mode, 1) == VTSS_OK) {
                        continue;
                    } else {
                        CX_RC(cx_parm_unknown(s));
                    }

                }

                if (trap_mgmt_conf_get(&trap_entry) != VTSS_RC_OK) {
                    CX_RC(cx_parm_found_error(s, "conf_name"));
                }


                switch ( s->group) {
                case CX_TAG_INTF_LINKUP_TRAP_EVENT:
                    intf_event = trap_entry.trap_event.interface.trap_linkup[s->isid];
                    break;
                case CX_TAG_INTF_LINKDOWN_TRAP_EVENT:
                    intf_event = trap_entry.trap_event.interface.trap_linkdown[s->isid];
                    break;
                case CX_TAG_INTF_LLDP_TRAP_EVENT:
                    intf_event = trap_entry.trap_event.interface.trap_lldp[s->isid];
                    break;
                default:
                    break;
                }

                if ( intf_event == NULL) {
                    T_E("error");
                    break;
                }
                (void) port_iter_init(&pit, NULL, s->isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
                while (port_iter_getnext(&pit)) {
                    if (port_list[pit.uport] == TRUE) {
                        intf_event[pit.iport] = mode;
                    }
                }
                (void) trap_mgmt_conf_set(&trap_entry);
            } else {
                s->ignored = 1;
            }
            break;
        default:
            s->ignored = 1;
            break;
        }
        if (s->apply) {
            CX_RC(snmp_mgmt_snmp_conf_set(&conf));
        }
        break;
        } /* CX_PARSE_CMD_PARM */
    case CX_PARSE_CMD_GLOBAL: {
        snmp_cx_set_state_t *snmp_state = s->mod_state;

        if (s->init) {
#ifdef SNMP_SUPPORT_V3
            /* Read community table */
            i = 0;
            strcpy(community.community, SNMPV3_CONF_ACESS_GETFIRST);
            while (snmpv3_mgmt_communities_conf_get(&community, TRUE) == VTSS_OK) {
                if (community.status == SNMP_MGMT_ROW_ACTIVE) {
                    snmp_state->communities.entry[i++] = community;
                }
            }
            snmp_state->communities.count = i;

            /* Read user table */
            i = 0;
            strcpy(user.user_name, SNMPV3_CONF_ACESS_GETFIRST);
            while (snmpv3_mgmt_users_conf_get(&user, TRUE) == VTSS_OK) {
                if (user.status == SNMP_MGMT_ROW_ACTIVE) {
                    snmp_state->users.entry[i++] = user;
                }
            }
            snmp_state->users.count = i;

            /* Read group table */
            i = 0;
            strcpy(group.security_name, SNMPV3_CONF_ACESS_GETFIRST);
            while (snmpv3_mgmt_groups_conf_get(&group, TRUE) == VTSS_OK) {
                if (group.status == SNMP_MGMT_ROW_ACTIVE) {
                    snmp_state->groups.line[i].number = 0;
                    snmp_state->groups.entry[i++] = group;
                }
            }
            snmp_state->groups.count = i;

            /* Read view table */
            i = 0;
            strcpy(view.view_name, SNMPV3_CONF_ACESS_GETFIRST);
            while (snmpv3_mgmt_views_conf_get(&view, TRUE) == VTSS_OK) {
                if (view.status == SNMP_MGMT_ROW_ACTIVE) {
                    snmp_state->views.entry[i++] = view;
                }
            }
            snmp_state->views.count = i;

            /* Read access table */
            i = 0;
            strcpy(access.group_name, SNMPV3_CONF_ACESS_GETFIRST);
            while (snmpv3_mgmt_accesses_conf_get(&access, TRUE) == VTSS_OK) {
                if (access.status == SNMP_MGMT_ROW_ACTIVE) {
                    snmp_state->accesses.line[i].number = 0;
                    snmp_state->accesses.entry[i++] = access;
                }
            }
            snmp_state->accesses.count = i;
#endif /* SNMP_SUPPORT_V3 */
        } else if (s->apply) {
#ifdef SNMP_SUPPORT_V3
            /* Delete community table */
            while (1) {
                strcpy(community.community, SNMPV3_CONF_ACESS_GETFIRST);
                if (snmpv3_mgmt_communities_conf_get(&community, TRUE) != VTSS_OK) {
                    break;
                }
                CX_RC(snmpv3_mgmt_communities_conf_del(community.idx));
            }

            /* Delete user table */
            while (1) {
                strcpy(user.user_name, SNMPV3_CONF_ACESS_GETFIRST);
                if (snmpv3_mgmt_users_conf_get(&user, TRUE) != VTSS_OK) {
                    break;
                }
                CX_RC(snmpv3_mgmt_users_conf_del(user.idx));
            }

            /* Delete group table */
            while (1) {
                strcpy(group.security_name, SNMPV3_CONF_ACESS_GETFIRST);
                if (snmpv3_mgmt_groups_conf_get(&group, TRUE) != VTSS_OK) {
                    break;
                }
                CX_RC(snmpv3_mgmt_groups_conf_del(group.idx));
            }

            /* Delete view table */
            while (1) {
                strcpy(view.view_name, SNMPV3_CONF_ACESS_GETFIRST);
                if (snmpv3_mgmt_views_conf_get(&view, TRUE) != VTSS_OK) {
                    break;
                }
                CX_RC(snmpv3_mgmt_views_conf_del(view.idx));
            }

            /* Delete access table */
            while (1) {
                strcpy(access.group_name, SNMPV3_CONF_ACESS_GETFIRST);
                if (snmpv3_mgmt_accesses_conf_get(&access, TRUE) != VTSS_OK) {
                    break;
                }
                CX_RC(snmpv3_mgmt_accesses_conf_del(access.idx));
            }

            /* Add community table */
            for (i = 0; i < snmp_state->communities.count; i++) {
                CX_RC(snmpv3_mgmt_communities_conf_set(&snmp_state->communities.entry[i]));
            }

            /* Add user table */
            for (i = 0; i < snmp_state->users.count; i++) {
                CX_RC(snmpv3_mgmt_users_conf_set(&snmp_state->users.entry[i]));
            }

            /* Add group table */
            for (i = 0; i < snmp_state->groups.count; i++) {
                CX_RC(snmpv3_mgmt_groups_conf_set(&snmp_state->groups.entry[i]));
            }

            /* Add view table */
            for (i = 0; i < snmp_state->views.count; i++) {
                CX_RC(snmpv3_mgmt_views_conf_set(&snmp_state->views.entry[i]));
            }

            /* Add access table */
            for (i = 0; i < snmp_state->accesses.count; i++) {
                CX_RC(snmpv3_mgmt_accesses_conf_set(&snmp_state->accesses.entry[i]));
            }
#endif /* SNMP_SUPPORT_V3 */
        } else {
#ifdef SNMP_SUPPORT_V3
            {
                snmp_conf_t            conf;
                snmpv3_users_conf_t    *user_p;
                snmpv3_groups_conf_t   *group_p;
                snmpv3_accesses_conf_t *access_p;
                snmpv3_views_conf_t    *view_p;
                BOOL                   usm, found, found_write;
                int                    j;
//                char                   buf[128] = {'\0'};

                CX_RC(snmp_mgmt_snmp_conf_get(&conf));

                /* Check group configuration */
                for (i = 0; i < snmp_state->groups.count; i++)
                {
                    group_p = &snmp_state->groups.entry[i];
                    found = 0;
                    usm = (group_p->security_model == SNMP_MGMT_SEC_MODEL_USM);
                    if (usm) {
                        /* Look for user entry with same engine ID and name */
                        for (j = 0; j < snmp_state->users.count; j++) {
                            user_p = &snmp_state->users.entry[j];
                            if (conf.engineid_len == user_p->engineid_len &&
                            memcmp(conf.engineid, user_p->engineid, conf.engineid_len) == 0 &&
                            strcmp(group_p->security_name, user_p->user_name) == 0) {
                                found = 1;
                            }
                        }
                    } else {
                        /* Look for community entry with same name */
                        for (j = 0; j < snmp_state->communities.count; j++) {
                            if (!strcmp(group_p->security_name, snmp_state->communities.entry[j].community)) {
                                found = 1;
                            }
                        }
                    }
                    if (!found) {
                        sprintf(buf, "The security_name '%s' does not exist in %s table",
                                group_p->security_name, usm ? "user" : "community");
                        CX_RC(cx_parm_error(s, buf));
                        if (snmp_state->groups.line[i].number != 0) {
                            s->line = snmp_state->groups.line[i];
                        }
                        return s->rc;
                    }
                }

                /* Check access configuration */
                for (i = 0; i < snmp_state->accesses.count; i++)
                {
                    access_p = &snmp_state->accesses.entry[i];

                    /* Look for group with the same name */
                    for (j = 0, found = 0; j < snmp_state->groups.count; j++) {
                        if (!strcmp(access_p->group_name, snmp_state->groups.entry[j].group_name)) {
                            found = 1;
                        }
                    }
                    if (!found) {
                        sprintf(buf, "The group_name '%s' does not exist in group table",
                                access_p->group_name);
                        CX_RC(cx_parm_error(s, buf));
                        if (snmp_state->accesses.line[i].number != 0) {
                            s->line = snmp_state->accesses.line[i];
                        }
                        return s->rc;
                    }

                    /* Look for read/write view in view table */
                    found = (strcmp(access_p->read_view_name, SNMPV3_NONAME) == 0);
                    found_write = (strcmp(access_p->write_view_name, SNMPV3_NONAME) == 0);
                    for (j = 0; j < snmp_state->views.count; j++) {
                        view_p = &snmp_state->views.entry[j];
                        if (!strcmp(access_p->read_view_name, view_p->view_name)) {
                            found = 1;
                        }
                        if (!strcmp(access_p->write_view_name, view_p->view_name)) {
                            found_write = 1;
                        }
                    }
                    if (!found) {
                        sprintf(buf, "The read_view_name '%s' does not exist in view table",
                                access_p->read_view_name);
                        CX_RC(cx_parm_error(s, buf));
                        if (snmp_state->accesses.line[i].number != 0) {
                            s->line = snmp_state->accesses.line[i];
                        }
                        return s->rc;
                    }
                    if (!found_write) {
                        sprintf(buf, "The write_view_name '%s' does not exist in view table",
                                access_p->write_view_name);
                        CX_RC(cx_parm_error(s, buf));
                        if (snmp_state->accesses.line[i].number != 0) {
                            s->line = snmp_state->accesses.line[i];
                        }
                        return s->rc;
                    }
                }
            }
#endif /* SNMP_SUPPORT_V3 */
        }
        break;
    }
    case CX_PARSE_CMD_SWITCH:
        break;
    default:
        break;
    }

    return s->rc;
}

vtss_rc cx_add_attr_newline(cx_get_state_t *s, BOOL add)
{
    s->p += sprintf(s->p, ">");
    /* Add end tag or newline */
    s->p += sprintf(s->p, "\n");

    if (add) {
        s->indent += 2;
    } else {
        s->indent -= 2;
    }
    return cx_size_check(s);
}

/* Print tag to buffer */
static int cx_sprint_endTag(char *p, cx_tag_entry_t *tab,
                            cx_tag_id_t id, BOOL end, BOOL nl)
{
    return sprintf(p, "<%s%s>%s%s",
                   end ? "/" : "",
                   tab[id].name,
                   end ? "" : tab[id].descr,
                   nl ? "\n" : "");
}

/* Add attribute end */
static vtss_rc cx_add_attr_noEnd(cx_get_state_t *s, cx_tag_id_t id)
{

    s->p += cx_sprint_endTag(s->p, s->tag, id, 1, 1);
    return cx_size_check(s);
}

static BOOL cx_trap_intf_linkup_event_match(const cx_table_context_t *context, ulong port_a, ulong port_b)
{

    vtss_trap_entry_t *trap_entry = context->custom;
    BOOL *intf_event = trap_entry->trap_event.interface.trap_linkup[context->isid];

    return intf_event[port_a] == intf_event[port_b];
}

static BOOL cx_trap_intf_linkdown_event_match(const cx_table_context_t *context, ulong port_a, ulong port_b)
{

    vtss_trap_entry_t *trap_entry = context->custom;
    BOOL *intf_event = trap_entry->trap_event.interface.trap_linkdown[context->isid];

    return intf_event[port_a] == intf_event[port_b];
}

static BOOL cx_trap_intf_lldp_event_match(const cx_table_context_t *context, ulong port_a, ulong port_b)
{

    vtss_trap_entry_t *trap_entry = context->custom;
    BOOL *intf_event = trap_entry->trap_event.interface.trap_lldp[context->isid];

    return intf_event[port_a] == intf_event[port_b];
}

static vtss_rc cx_trap_intf_linkup_event_print(cx_get_state_t *s, const cx_table_context_t *context, ulong port_no, char *ports)
{

    vtss_trap_entry_t *trap_entry = context->custom;
    BOOL              *intf_event = trap_entry->trap_event.interface.trap_linkup[context->isid];
    char buf[TRAP_MAX_NAME_LEN + 1] = {'\0'};

    if (ports == NULL) {
        /* Syntax */
        CX_RC(cx_add_stx_start(s));
        CX_RC(cx_add_stx_port(s));
        CX_RC(cx_add_attr_txt(s, "conf_name", buf));
        CX_RC(cx_add_stx_bool(s, "mode"));
        return cx_add_stx_end(s);
    }

    CX_RC(cx_add_port_start(s, CX_TAG_ENTRY, ports));
    CX_RC(cx_add_attr_txt(s, "conf_name", trap_entry->trap_conf_name));
    CX_RC(cx_add_attr_bool(s, "mode", intf_event[port_no]));
    return cx_add_port_end(s, CX_TAG_ENTRY);
}

static vtss_rc cx_trap_intf_linkdown_event_print(cx_get_state_t *s, const cx_table_context_t *context, ulong port_no, char *ports)
{

    vtss_trap_entry_t *trap_entry = context->custom;
    BOOL              *intf_event = trap_entry->trap_event.interface.trap_linkdown[context->isid];
    char buf[TRAP_MAX_NAME_LEN + 1] = {'\0'};

    if (ports == NULL) {
        /* Syntax */
        CX_RC(cx_add_stx_start(s));
        CX_RC(cx_add_stx_port(s));
        CX_RC(cx_add_attr_txt(s, "conf_name", buf));
        CX_RC(cx_add_stx_bool(s, "mode"));
        return cx_add_stx_end(s);
    }

    CX_RC(cx_add_port_start(s, CX_TAG_ENTRY, ports));
    CX_RC(cx_add_attr_txt(s, "conf_name", trap_entry->trap_conf_name));
    CX_RC(cx_add_attr_bool(s, "mode", intf_event[port_no]));
    return cx_add_port_end(s, CX_TAG_ENTRY);
}

static vtss_rc cx_trap_intf_lldp_event_print(cx_get_state_t *s, const cx_table_context_t *context, ulong port_no, char *ports)
{

    vtss_trap_entry_t *trap_entry = context->custom;
    BOOL              *intf_event = trap_entry->trap_event.interface.trap_lldp[context->isid];
    char buf[TRAP_MAX_NAME_LEN + 1] = {'\0'};

    if (ports == NULL) {
        /* Syntax */
        CX_RC(cx_add_stx_start(s));
        CX_RC(cx_add_stx_port(s));
        CX_RC(cx_add_attr_txt(s, "conf_name", buf));
        CX_RC(cx_add_stx_bool(s, "mode"));
        return cx_add_stx_end(s);
    }

    CX_RC(cx_add_port_start(s, CX_TAG_ENTRY, ports));
    CX_RC(cx_add_attr_txt(s, "conf_name", trap_entry->trap_conf_name));
    CX_RC(cx_add_attr_bool(s, "mode", intf_event[port_no]));
    return cx_add_port_end(s, CX_TAG_ENTRY);
}

static vtss_rc snmp_cx_gen_func(cx_get_state_t *s)
{
    char buf[128] = {'\0'};
    switch (s->cmd) {
    case CX_GEN_CMD_GLOBAL:
        /* Global - SNMP */
#ifdef VTSS_SW_OPTION_SNMP
        T_D("global - snmp");
        CX_RC(cx_add_tag_line(s, CX_TAG_SNMP, 0));
        {
            snmp_conf_t conf;

            if (snmp_mgmt_snmp_conf_get(&conf) == VTSS_OK) {
                CX_RC(cx_add_val_bool(s, CX_TAG_MODE, conf.mode));
                CX_RC(cx_add_val_kw(s, CX_TAG_VERSION, cx_kw_snmp_version, conf.version));
                (void) cx_stx_snmp_txt(buf, 0, SNMP_MGMT_MAX_COMMUNITY_LEN - 1);
                CX_RC(cx_add_val_txt(s, CX_TAG_READ_COMMUNITY, conf.read_community, buf));
                CX_RC(cx_add_val_txt(s, CX_TAG_WRITE_COMMUNITY, conf.write_community, buf));

#if 0
                CX_RC(cx_add_tag_line(s, CX_TAG_TRAP, 0));
                CX_RC(cx_add_val_bool(s, CX_TAG_MODE, conf.trap_mode));
                CX_RC(cx_add_val_kw(s, CX_TAG_VERSION, cx_kw_snmp_version, conf.trap_version));
                CX_RC(cx_add_val_txt(s, CX_TAG_COMMUNITY, conf.trap_community, buf));
                CX_RC(cx_add_val_txt(s, CX_TAG_DESTINATION, conf.trap_dip_string, buf));
#ifdef VTSS_SW_OPTION_IPV6
                CX_RC(cx_add_val_ipv6(s, CX_TAG_DESTINATION_V6, conf.trap_dipv6));
#endif /* VTSS_SW_OPTION_IPV6 */
                CX_RC(cx_add_val_bool(s, CX_TAG_AUTH_FAIL, conf.trap_authen_fail));
                CX_RC(cx_add_val_bool(s, CX_TAG_LINKUP_LINKDOWN, conf.trap_linkup_linkdown));
                CX_RC(cx_add_val_bool(s, CX_TAG_INFORM_MODE, conf.trap_inform_mode));
                CX_RC(cx_add_val_ulong(s, CX_TAG_INFORM_TIMEOUT, conf.trap_inform_timeout,
                                       0, SNMP_MGMT_MAX_TRAP_INFORM_TIMEOUT));
                CX_RC(cx_add_val_ulong(s, CX_TAG_INFORM_RETRY, conf.trap_inform_retries,
                                       0, SNMP_MGMT_MAX_TRAP_INFORM_RETRIES));
#ifdef SNMP_SUPPORT_V3
                CX_RC(cx_add_val_bool(s, CX_TAG_SECURITY_MODE, conf.trap_probe_security_engineid));
                CX_RC(cx_add_val_engine_id(s, CX_TAG_SECURITY_ENGINE_ID, conf.trap_security_engineid,
                                           conf.trap_security_engineid_len));
                CX_RC(cx_add_val_txt(s, CX_TAG_SECURITY_NAME, conf.trap_security_name,
                                     cx_stx_snmp_name(buf)));
#endif /* SNMP_SUPPORT_V3 */
                CX_RC(cx_add_tag_line(s, CX_TAG_TRAP, 1));
#endif

#ifdef SNMP_SUPPORT_V3
                CX_RC(cx_add_val_engine_id(s, CX_TAG_ENGINE_ID, conf.engineid, conf.engineid_len));

                CX_RC(cx_add_tag_line(s, CX_TAG_COMMUNITY_TABLE, 0));
                {
                    snmpv3_communities_conf_t community_conf;

                    /* Entry syntax */
                    CX_RC(cx_add_stx_start(s));
                    CX_RC(cx_add_attr_txt(s, "community", cx_stx_snmp_name(buf)));
                    strcpy(buf, "a.b.c.d");
                    CX_RC(cx_add_attr_txt(s, "source_ip", buf));
                    CX_RC(cx_add_attr_txt(s, "source_mask", buf));
                    CX_RC(cx_add_stx_end(s));

                    strcpy(community_conf.community, SNMPV3_CONF_ACESS_GETFIRST);
                    while (snmpv3_mgmt_communities_conf_get(&community_conf, TRUE) == VTSS_OK) {
                        if (community_conf.status != SNMP_MGMT_ROW_ACTIVE) {
                            continue;
                        }
                        CX_RC(cx_add_attr_start(s, CX_TAG_ENTRY));
                        CX_RC(cx_add_attr_txt(s, "community", community_conf.community));
                        CX_RC(cx_add_attr_ipv4(s, "source_ip", community_conf.sip));
                        CX_RC(cx_add_attr_ipv4(s, "source_mask", community_conf.sip_mask));
                        CX_RC(cx_add_attr_end(s, CX_TAG_ENTRY));
                    }
                }
                CX_RC(cx_add_tag_line(s, CX_TAG_COMMUNITY_TABLE, 1));

                CX_RC(cx_add_tag_line(s, CX_TAG_USER_TABLE, 0));
                {
                    snmpv3_users_conf_t user_conf;

                    /* Entry syntax */
                    CX_RC(cx_add_stx_start(s));
                    CX_RC(cx_add_attr_txt(s, "engine_id", cx_engine_id_txt));
                    CX_RC(cx_add_attr_txt(s, "user_name", cx_stx_snmp_name(buf)));
                    CX_RC(cx_add_stx_kw(s, "auth_proto", cx_kw_snmp_auth));
                    (void) cx_stx_snmp_txt(buf, SNMPV3_MIN_PASSWORD_LEN, SNMPV3_MAX_SHA_PASSWORD_LEN);
                    CX_RC(cx_add_attr_txt(s, "auth_password", buf));
                    CX_RC(cx_add_stx_kw(s, "priv_proto", cx_kw_snmp_priv));
                    CX_RC(cx_add_attr_txt(s, "priv_password", buf));
                    CX_RC(cx_add_stx_end(s));

                    strcpy(user_conf.user_name, SNMPV3_CONF_ACESS_GETFIRST);
                    while (snmpv3_mgmt_users_conf_get(&user_conf, TRUE) == VTSS_OK) {
                        if (user_conf.status != SNMP_MGMT_ROW_ACTIVE) {
                            continue;
                        }
                        CX_RC(cx_add_attr_start(s, CX_TAG_ENTRY));
                        strcpy(buf, misc_engineid2str(user_conf.engineid, user_conf.engineid_len));
                        CX_RC(cx_add_attr_txt(s, "engine_id", buf));
                        CX_RC(cx_add_attr_txt(s, "user_name", user_conf.user_name));
                        CX_RC(cx_add_attr_kw(s, "auth_proto", cx_kw_snmp_auth, user_conf.auth_protocol));
                        CX_RC(cx_add_attr_txt(s, "auth_password", user_conf.auth_password));
                        CX_RC(cx_add_attr_kw(s, "priv_proto", cx_kw_snmp_priv, user_conf.priv_protocol));
                        CX_RC(cx_add_attr_txt(s, "priv_password", user_conf.priv_password));
                        CX_RC(cx_add_attr_end(s, CX_TAG_ENTRY));
                    }
                }
                CX_RC(cx_add_tag_line(s, CX_TAG_USER_TABLE, 1));

                CX_RC(cx_add_tag_line(s, CX_TAG_GROUP_TABLE, 0));
                {
                    snmpv3_groups_conf_t group_conf;

                    /* Entry syntax */
                    CX_RC(cx_add_stx_start(s));
                    CX_RC(cx_add_attr_txt(s, "security_model", "v1/v2c/usm"));
                    CX_RC(cx_add_attr_txt(s, "security_name", cx_stx_snmp_name(buf)));
                    CX_RC(cx_add_attr_txt(s, "group_name", buf));
                    CX_RC(cx_add_stx_end(s));

                    strcpy(group_conf.security_name, SNMPV3_CONF_ACESS_GETFIRST);
                    while (snmpv3_mgmt_groups_conf_get(&group_conf, TRUE) == VTSS_OK) {
                        if (group_conf.status != SNMP_MGMT_ROW_ACTIVE) {
                            continue;
                        }
                        CX_RC(cx_add_attr_start(s, CX_TAG_ENTRY));
                        CX_RC(cx_add_attr_kw(s, "security_model", cx_kw_snmp_model, group_conf.security_model));
                        CX_RC(cx_add_attr_txt(s, "security_name", group_conf.security_name));
                        CX_RC(cx_add_attr_txt(s, "group_name", group_conf.group_name));
                        CX_RC(cx_add_attr_end(s, CX_TAG_ENTRY));
                    }
                }
                CX_RC(cx_add_tag_line(s, CX_TAG_GROUP_TABLE, 1));

                CX_RC(cx_add_tag_line(s, CX_TAG_VIEW_TABLE, 0));
                {
                    snmpv3_views_conf_t view_conf;

                    /* Entry syntax */
                    CX_RC(cx_add_stx_start(s));
                    CX_RC(cx_add_attr_txt(s, "view_name", cx_stx_snmp_name(buf)));
                    CX_RC(cx_add_stx_kw(s, "view_type", cx_kw_snmp_view));
                    CX_RC(cx_add_attr_txt(s, "oid_subtree",
                                          ".a.b.c.d OID string of 1-128 numbers or '*'"));
                    CX_RC(cx_add_stx_end(s));

                    strcpy(view_conf.view_name, SNMPV3_CONF_ACESS_GETFIRST);
                    while (snmpv3_mgmt_views_conf_get(&view_conf, TRUE) == VTSS_OK) {
                        if (view_conf.status != SNMP_MGMT_ROW_ACTIVE) {
                            continue;
                        }
                        CX_RC(cx_add_attr_start(s, CX_TAG_ENTRY));
                        CX_RC(cx_add_attr_txt(s, "view_name", view_conf.view_name));
                        CX_RC(cx_add_attr_kw(s, "view_type", cx_kw_snmp_view, view_conf.view_type));
                        strncpy(buf, misc_oid2str(view_conf.subtree, view_conf.subtree_len,
                                                  view_conf.subtree_mask, view_conf.subtree_mask_len),
                                sizeof(buf));
                        buf[sizeof(buf) - 1] = '\0';
                        CX_RC(cx_add_attr_txt(s, "oid_subtree", buf));
                        CX_RC(cx_add_attr_end(s, CX_TAG_ENTRY));
                    }
                }
                CX_RC(cx_add_tag_line(s, CX_TAG_VIEW_TABLE, 1));

                CX_RC(cx_add_tag_line(s, CX_TAG_ACCESS_TABLE, 0));
                {
                    snmpv3_accesses_conf_t access_conf;

                    /* Entry syntax */
                    CX_RC(cx_add_stx_start(s));
                    CX_RC(cx_add_attr_txt(s, "group_name", cx_stx_snmp_name(buf)));
                    CX_RC(cx_add_stx_kw(s, "security_model", cx_kw_snmp_model));
                    CX_RC(cx_add_stx_kw(s, "security_level", cx_kw_snmp_level));
                    CX_RC(cx_add_attr_txt(s, "read_view_name", buf));
                    CX_RC(cx_add_attr_txt(s, "write_view_name", buf));
                    CX_RC(cx_add_stx_end(s));

                    strcpy(access_conf.group_name, SNMPV3_CONF_ACESS_GETFIRST);
                    while (snmpv3_mgmt_accesses_conf_get(&access_conf, TRUE) == VTSS_OK) {
                        if (access_conf.status != SNMP_MGMT_ROW_ACTIVE) {
                            continue;
                        }
                        CX_RC(cx_add_attr_start(s, CX_TAG_ENTRY));
                        CX_RC(cx_add_attr_txt(s, "group_name", access_conf.group_name));
                        CX_RC(cx_add_attr_kw(s, "security_model", cx_kw_snmp_model, access_conf.security_model));
                        CX_RC(cx_add_attr_kw(s, "security_level", cx_kw_snmp_level, access_conf.security_level));
                        CX_RC(cx_add_attr_txt(s, "read_view_name", access_conf.read_view_name));
                        CX_RC(cx_add_attr_txt(s, "write_view_name", access_conf.write_view_name));
                        CX_RC(cx_add_attr_end(s, CX_TAG_ENTRY));
                    }
                }
                CX_RC(cx_add_tag_line(s, CX_TAG_ACCESS_TABLE, 1));
#endif /* SNMP_SUPPORT_V3 */
            }
            CX_RC(cx_add_tag_line(s, CX_TAG_TRAP, 0));
            {
                BOOL trap_mode;
                vtss_trap_entry_t trap_entry;
                vtss_trap_conf_t *trap_conf = &trap_entry.trap_conf;
                vtss_trap_event_t *trap_event = &trap_entry.trap_event;

                (void) trap_mgmt_mode_get(&trap_mode);
                CX_RC(cx_add_val_bool(s, CX_TAG_MODE, trap_mode));

                memset (&trap_entry, 0, sizeof(trap_entry));

                CX_RC(cx_add_tag_line(s, CX_TAG_TRAP_TABLE, 0));
                {
                    /* Entry syntax */
                    CX_RC(cx_add_stx_start(s));
                    CX_RC(cx_add_attr_txt(s, "conf_name", buf));
                    CX_RC(cx_add_stx_bool(s, "mode"));
                    CX_RC(cx_add_attr_txt(s, "dip", buf)); /**/
                    CX_RC(cx_add_stx_ulong(s, "dport", 1, 65535));
                    CX_RC(cx_add_stx_kw(s,   "version", cx_kw_snmp_version));
                    CX_RC(cx_add_attr_txt(s, "community", buf));
                    CX_RC(cx_add_stx_kw(s, " inform", cx_kw_snmp_inform));
                    CX_RC(cx_add_stx_ulong(s, "timeout", 0, SNMP_MGMT_MAX_TRAP_INFORM_TIMEOUT));
                    CX_RC(cx_add_stx_ulong(s, "retires", 0, SNMP_MGMT_MAX_TRAP_INFORM_RETRIES));
                    CX_RC(cx_add_stx_bool(s, "probe"));
                    CX_RC(cx_add_attr_txt(s, "engine_id", cx_engine_id_txt));
                    CX_RC(cx_add_attr_txt(s, "security_name", cx_stx_snmp_name(buf)));
                    CX_RC(cx_add_stx_bool(s, "warm_start"));
                    CX_RC(cx_add_stx_bool(s, "cold_start"));

                    CX_RC(cx_add_stx_bool(s, "auth_fail"));
                    CX_RC(cx_add_stx_bool(s, "stp"));
                    CX_RC(cx_add_stx_bool(s, "rmon"));
                    CX_RC(cx_add_stx_ulong(s, "sid", VTSS_USID_START, VTSS_USID_END - 1));
                    CX_RC(cx_add_stx_ulong(s, "link_up", 1, s->port_count));
                    CX_RC(cx_add_stx_ulong(s, "link_down", 1, s->port_count));
                    CX_RC(cx_add_stx_ulong(s, "lldp", 1, s->port_count));
                    CX_RC(cx_add_stx_end(s));

                    while ( VTSS_RC_OK == trap_mgmt_conf_get_next(&trap_entry)) {

                        CX_RC(cx_add_attr_start(s, CX_TAG_TRAP_ENTRY));
                        CX_RC(cx_add_attr_txt(s, "conf_name", trap_entry.trap_conf_name));
                        CX_RC(cx_add_attr_newline(s, 1));
                        CX_RC(cx_add_attr_start(s, CX_TAG_TRAP_CONF));
                        CX_RC(cx_add_attr_bool(s, "mode", trap_conf->enable));

#ifdef VTSS_SW_OPTION_IPV6
                        CX_RC(cx_add_attr_txt(s, "dip",
                                              (!trap_conf->dip.ipv6_flag) ? trap_conf->dip.addr.ipv4_str : misc_ipv6_txt(&trap_conf->dip.addr.ipv6, buf)));
#else
                        CX_RC(cx_add_attr_txt(s, "dip",
                                              trap_conf->dip.addr.ipv4_str));
#endif /* VTSS_SW_OPTION_IPV6 */
                        CX_RC(cx_add_attr_ulong(s, "dport", trap_conf->trap_port));
                        CX_RC(cx_add_attr_kw(s, "version", cx_kw_snmp_version, trap_conf->trap_version));
                        CX_RC(cx_add_attr_txt(s, "community", trap_conf->trap_community));
                        CX_RC(cx_add_attr_kw(s, "inform", cx_kw_snmp_inform, trap_conf->trap_inform_mode));
                        CX_RC(cx_add_attr_ulong(s, "timeout", trap_conf->trap_inform_timeout));
                        CX_RC(cx_add_attr_ulong(s, "retries", trap_conf->trap_inform_retries));
                        CX_RC(cx_add_attr_bool(s, "probe", trap_conf->trap_probe_engineid));

                        strcpy(buf, misc_engineid2str(trap_conf->trap_engineid, trap_conf->trap_engineid_len));
                        CX_RC(cx_add_attr_txt(s, "engine_id", buf));
                        CX_RC(cx_add_attr_txt(s, "security_name", trap_conf->trap_security_name));
                        CX_RC(cx_add_attr_end(s, CX_TAG_TRAP_CONF));

                        CX_RC(cx_add_attr_start(s, CX_TAG_GLOBAL_TRAP_EVENT));
                        CX_RC(cx_add_attr_bool(s, "warm_start", trap_event->system.warm_start));
                        CX_RC(cx_add_attr_bool(s, "cold_start", trap_event->system.cold_start));
                        CX_RC(cx_add_attr_bool(s, "auth_fail", trap_event->aaa.trap_authen_fail));
                        CX_RC(cx_add_attr_bool(s, "stp", trap_event->sw.stp));
                        CX_RC(cx_add_attr_bool(s, "rmon", trap_event->sw.rmon));
                        CX_RC(cx_add_attr_end(s, CX_TAG_GLOBAL_TRAP_EVENT));
                        CX_RC(cx_add_attr_noEnd(s, CX_TAG_TRAP_ENTRY));
                    }
                }
                CX_RC(cx_add_tag_line(s, CX_TAG_TRAP_TABLE, 1));
            }
            CX_RC(cx_add_tag_line(s, CX_TAG_TRAP, 1));



        }
        CX_RC(cx_add_tag_line(s, CX_TAG_SNMP, 1));
#endif /* VTSS_SW_OPTION_SNMP */
        break;
    case CX_GEN_CMD_SWITCH:
        CX_RC(cx_add_tag_line(s, CX_TAG_SNMP, 0));
        CX_RC(cx_add_tag_line(s, CX_TAG_TRAP, 0));
        {
            vtss_trap_entry_t trap_entry;

            memset (&trap_entry, 0, sizeof(trap_entry));
            while ( VTSS_RC_OK == trap_mgmt_conf_get_next(&trap_entry)) {
                CX_RC(cx_add_port_table_ex(s, s->isid, &trap_entry, CX_TAG_INTF_LINKUP_TRAP_EVENT, cx_trap_intf_linkup_event_match, cx_trap_intf_linkup_event_print));
                CX_RC(cx_add_port_table_ex(s, s->isid, &trap_entry, CX_TAG_INTF_LINKDOWN_TRAP_EVENT, cx_trap_intf_linkdown_event_match, cx_trap_intf_linkdown_event_print));
                CX_RC(cx_add_port_table_ex(s, s->isid, &trap_entry, CX_TAG_INTF_LLDP_TRAP_EVENT, cx_trap_intf_lldp_event_match, cx_trap_intf_lldp_event_print));
            }
        }
        CX_RC(cx_add_tag_line(s, CX_TAG_TRAP, 1));
        CX_RC(cx_add_tag_line(s, CX_TAG_SNMP, 1));

        break;
    default:
        T_E("Unknown command");
        return VTSS_RC_ERROR;
    } /* End of Switch */

    return VTSS_OK;
}

/* Register the info in to the cx_module_table */
CX_MODULE_TAB_ENTRY(
    VTSS_MODULE_ID_SNMP,
    snmp_cx_tag_table,
    sizeof(snmp_cx_set_state_t),
    0,
    NULL,                   /* init function       */
    snmp_cx_gen_func,       /* Generation fucntion */
    snmp_cx_parse_func      /* parse fucntion      */
);

