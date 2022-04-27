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
#include "conf_api.h"
#include "msg_api.h"
#include "critd_api.h"
#include "vtss_ecos_mutex_api.h"
#include "misc_api.h"
#include "vtss_snmp_api.h"
#include "vtss_snmp.h"
#include "vtss_snmp_mibs_init.h"
#include "l2proto_api.h"
#include "control_api.h" /* For control_system_restart_to_str() */
#ifdef VTSS_SW_OPTION_SYSLOG
#include "syslog_api.h"
#endif

//#include "vtss_free_list_api.h"
#include "vtss_avl_tree_api.h"

#include <ucd-snmp/config.h>
#include <network.h>
#include <arpa/inet.h>
#include <ucd-snmp/asn1.h>
#include <ucd-snmp/snmp_api.h>
#include <ucd-snmp/snmp_vars.h>
#include <ucd-snmp/snmpd.h>
#include <ucd-snmp/snmp.h>
#include <ucd-snmp/callback.h>
#include <ucd-snmp/agent_callbacks.h>
#include <ucd-snmp/agent_trap.h>
#include <ucd-snmp/tools.h>
#include "snmp_custom_api.h"

#include "mibContextTable.h"
#include "sysORTable.h"
#include "snmp_mib_redefine.h"
#include "rfc1213_mib2.h"
#ifdef VTSS_SW_OPTION_RMON
#include "rfc2819_rmon.h"
#include "rmon_api.h"
#endif
#ifdef VTSS_SW_OPTION_SMON
#include "rfc2613_smon.h"
#endif
#include "rfc4188_bridge.h"
#include "rfc3635_etherlike.h"

#ifdef SNMP_SUPPORT_V3
#include <ucd-snmp/snmpv3.h>
#include <ucd-snmp/snmpusm.h>
#include <ucd-snmp/vacm.h>
#include <ucd-snmp/vacm_vars.h>
#include <ucd-snmp/mib.h>

#include "rfc3411_framework.h"
#include "rfc3412_mpd.h"
#ifdef VTSS_SW_OPTION_SMB_SNMP
#include "rfc3414_usm.h"
#include "rfc3415_vacm.h"
#endif /* VTSS_SW_OPTION_SMB_SNMP */
#endif /* SNMP_SUPPORT_V3 */

#ifdef VTSS_SW_OPTION_LLDP
#include "dot1ab_lldp.h"
#endif

#ifdef VTSS_SW_OPTION_SMB_SNMP
#include "rfc2674_q_bridge.h"
#include "rfc4363_p_bridge.h"
#include "ieee8021BridgeMib.h"
#include "ieee8021QBridgeMib.h"
#ifdef VTSS_SW_OPTION_DOT1X_ACCT
#include "rfc4670_radiusclient.h"
#endif /* VTSS_SW_OPTION_RADIUS */
#ifdef VTSS_SW_OPTION_LACP
#include "ieee8023_lag_mib.h"
#endif /* VTSS_SW_OPTION_LACP */
#ifdef VTSS_SW_OPTION_MSTP
#include "ieee8021MstpMib.h"
#endif
#ifdef VTSS_SW_OPTION_ETH_LINK_OAM
#include "dot3OamMIB.h"
#endif
#ifdef VTSS_SW_OPTION_IGMPS
#include "rfc2933_igmp.h"
#endif /* VTSS_SW_OPTION_IGMPS */
#ifdef VTSS_SW_OPTION_IPMC
#include "mgmdMIBObjects.h"
#endif /* VTSS_SW_OPTION_IPMC */
#include "rfc2863_ifmib.h"
#ifdef VTSS_SW_OPTION_DOT1X
#include "ieee8021x_mib.h"
#ifdef VTSS_SW_OPTION_RADIUS
#include "rfc4668_radiusclient.h"
#endif /* VTSS_SW_OPTION_RADIUS */
#endif /* VTSS_SW_OPTION_DOT1X */
#if defined(VTSS_SW_OPTION_POE) || defined(VTSS_SW_OPTION_LLDP_MED)
#include "lldpXMedMIB.h"
#endif // VTSS_SW_OPTION_POE
#if defined(VTSS_SW_OPTION_POE)
#include "powerEthernetMIB.h"
#endif
#include "rfc4133_entity.h"
#include "rfc3636_mau.h"
#endif //VTSS_SW_OPTION_SMB_SNMP

#ifdef VTSS_SW_OPTION_SFLOW
#include"sflow_snmp.h"
#endif

#ifdef VTSS_SW_OPTION_VCLI
#include "snmp_cli.h"
#endif
#ifdef VTSS_SW_OPTION_ICFG
#include "snmp_icfg.h"
#include "trap_icfg.h"
#endif

#if defined(VTSS_SW_OPTION_IP2)
#include "rfc4292_ip_forward.h"
#include "rfc4293_ip.h"
#endif /* VTSS_SW_OPTION_IP2 */

#include "ifIndex_api.h"
/* SNMP default configuration */
#define SNMP_DEFAULT_TRAP_MODE                      SNMP_MGMT_DISABLED
#define SNMP_DEFAULT_TRAP_VER                       SNMP_SUPPORT_V1
#define SNMP_DEFAULT_TRAP_COMMUNITY                 "public"
#define SNMP_DEFAULT_TRAP_DIP                       0x0
#define SNMP_DEFAULT_TRAP_AUTHEN_FAIL               SNMP_MGMT_ENABLED
#define SNMP_DEFAULT_TRAP_LINKUP_LINKDOWN           SNMP_MGMT_ENABLED
#define SNMP_DEFAULT_TRAP_INFORM_TIMEOUT            1 //secs
#define SNMP_DEFAULT_TRAP_INFORM_RETRIES            5
#define SNMP_DEFAULT_TRAP_SECURITY_ENGINE_ID_PROBE  SNMP_MGMT_ENABLED

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_SNMP

/****************************************************************************/
/*  Global variables                                                        */
/****************************************************************************/

/* Global structure */
static snmp_global_t snmp_global;
static vtss_trap_sys_conf_t trap_global;

#if (VTSS_TRACE_ENABLED)
static vtss_trace_reg_t trace_reg = {
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "snmp",
    .descr     = "SNMP"
};

static vtss_trace_grp_t trace_grps[TRACE_GRP_CNT] = {
    [VTSS_TRACE_GRP_DEFAULT] = {
        .name      = "default",
        .descr     = "Default",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [TRACE_GRP_CRIT] = {
        .name      = "crit",
        .descr     = "Critical regions ",
        .lvl       = VTSS_TRACE_LVL_ERROR,
        .timestamp = 1,
    }
};
#define SNMP_CRIT_ENTER() critd_enter(&snmp_global.crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define SNMP_CRIT_EXIT()  critd_exit( &snmp_global.crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#else
#define SNMP_CRIT_ENTER() critd_enter(&snmp_global.crit)
#define SNMP_CRIT_EXIT()  critd_exit( &snmp_global.crit)
#endif /* VTSS_TRACE_ENABLED */

/* SNMP access error codes */
#define SNMP_ACCESS_NO_ERR                      0x0
#define SNMP_ACCESS_TRAP_AUTH_FAIL              0x1
#define SNMP_ACCESS_BAD_VER                     0x2
#define SNMP_ACCESS_BAD_COMMUNITY               0x4
#define SNMP_ACCESS_BAD_COMMUNITY_USE           0x8
#define SNMP_ACCESS_BAD_ERROR_STACK_ROLE        0x10
#define SNMP_ACCESS_BAD_COMMAND                 0x20

/* Thread variables */
static cyg_handle_t snmp_trap_thread_handle;
static cyg_thread   snmp_trap_thread_block;
static char         snmp_trap_thread_stack[SNMP_TRAP_STACK_SIZE * 2]; /* 20K for SNMPv3 trap */
oid                 snmp_private_mib_oid[] = {1, 3, 6, 1, 4, 1, SNMP_PRIVATE_MIB_ENTERPRISE, SNMP_PRIVATE_MIB_PRODUCT_ID /* Placeholder for product ID */};

/* values of the generic-trap field in trap PDUs */
typedef enum {
    SNMP_TRAP_TYPE_COLDSTART,
    SNMP_TRAP_TYPE_WARMSTART,
    SNMP_TRAP_TYPE_LINKDOWN,
    SNMP_TRAP_TYPE_LINKUP,
    SNMP_TRAP_TYPE_AUTHFAIL,
    SNMP_TRAP_TYPE_EVENT,
    SNMP_TRAP_TYPE_VARS,
    SNMP_TRAP_TYPE_PROBE,
    SNMP_TRAP_TYPE_SPECIFIC_OID,
    SNMP_TRAP_TYPE_SPECIFIC_OID_WITH_IF_IDX,
    SNMP_TRAP_TYPE_END
} snmp_trap_type_t;

/* Trap interface information */
typedef struct {
    long    if_idx;     /* ifIndex */
    long    if_admin;   /* ifAdminStatus */
    long    if_oper;    /* ifOperStatus */
} snmp_trap_if_info_t;

typedef struct {
    snmp_trap_type_t                trap_type;
    int                             trap_specific;
    snmp_trap_if_info_t             trap_if_info;
#ifdef VTSS_SW_OPTION_RMON
    rmon_event_trap_entry_t         event_trap_entry;
#endif
    snmp_vars_trap_entry_t          trap_entry;
    ucd_trap_conf_t                 trap_conf;
} snmp_trap_buff_t;

#define SNMP_TRAP_BUFF_MAX_CNT      (VTSS_ISID_CNT*VTSS_PORTS + 2) /* +1 means cold start or warm start, +1 means null buffer */
#define SNMP_TRAP_BUFF_CNT(r,w)     ((r) > (w) ? ((SNMP_TRAP_BUFF_MAX_CNT) - ((r)-(w))) : ((w)-(r)))

static snmp_trap_buff_t snmp_trap_buff[SNMP_TRAP_BUFF_MAX_CNT];
static int trap_buff_read_idx = 0, trap_buff_write_idx = 0;
static int snmp_master_down_flag = 0;

static int snmp_check_callback(int majorID, int minorID, void *serverarg, void *clientarg);
#ifdef SNMP_SUPPORT_V3
static int snmpv3_trap_callback(int majorID, int minorID, void *serverarg, void *clientarg);
#endif /* SNMP_SUPPORT_V3 */

typedef enum {
    EVENT_TYPE_WARM_START,
    EVENT_TYPE_COLD_START,
    EVENT_TYPE_LINK_UP,
    EVENT_TYPE_LINK_DOWN,
    EVENT_TYPE_LLDP,
    EVENT_TYPE_AUTH_FAIL,
    EVENT_TYPE_STP,
    EVENT_TYPE_RMON,
    EVENT_TYPE_ENTITY,
    EVENT_TYPE_END
} event_type_e;

typedef struct {
    snmp_vars_trap_entry_t trap_entry;
    event_type_e event_type;
} event_tbl_t;

#define SNMP_TRAP_OID   {1, 3, 6, 1, 6, 3, 1, 1, 4, 1, 0}
#define COLD_START_OID  {1, 3, 6, 1, 6, 3, 1, 1, 5, 1}
#define WARM_START_OID  {1, 3, 6, 1, 6, 3, 1, 1, 5, 2}
#define LINKDOWN_OID    {1, 3, 6, 1, 6, 3, 1, 1, 5, 3}
#define LINKUP_OID      {1, 3, 6, 1, 6, 3, 1, 1, 5, 4}
#define AUTH_FAIL_OID   {1, 3, 6, 1, 6, 3, 1, 1, 5, 5}
#define IFINDEX_OID {1, 3, 6, 1, 2, 1, 2, 2, 1, 1, 0}
#define IFADMIN_OID {1, 3, 6, 1, 2, 1, 2, 2, 1, 7, 0}
#define IFOPER_OID  {1, 3, 6, 1, 2, 1, 2, 2, 1, 8, 0}

static event_tbl_t event_tbl[] = {
    {{{1, 0, 8802, 1, 1, 2, 0,  0, 1  }, 9,  NULL}, EVENT_TYPE_LLDP},
    {{{1, 0, 8802, 1, 1, 2, 1,  5, 4795, 0, 1}, 11,  NULL}, EVENT_TYPE_LLDP},
    {{{1, 3, 6,    1, 2, 1, 17, 0, 1  }, 9,  NULL}, EVENT_TYPE_STP},
    {{{1, 3, 6,    1, 2, 1, 17, 0, 2  }, 9,  NULL}, EVENT_TYPE_STP},
    {{{1, 3, 6,    1, 2, 1, 16, 0, 1  }, 9,  NULL}, EVENT_TYPE_RMON},
    {{{1, 3, 6,    1, 2, 1, 16, 0, 2  }, 9,  NULL}, EVENT_TYPE_RMON},
    {{{1, 3, 6,    1, 2, 1, 47, 2, 0, 1}, 10,  NULL}, EVENT_TYPE_ENTITY},
    {{{1, 3, 6,    1, 6, 3, 1,  1, 5, 1}, 10, NULL}, EVENT_TYPE_COLD_START},
    {{{1, 3, 6,    1, 6, 3, 1,  1, 5, 2}, 10, NULL}, EVENT_TYPE_WARM_START},
    {{{1, 3, 6,    1, 6, 3, 1,  1, 5, 3}, 10, NULL}, EVENT_TYPE_LINK_DOWN},
    {{{1, 3, 6,    1, 6, 3, 1,  1, 5, 4}, 10, NULL}, EVENT_TYPE_LINK_UP},
    {{{1, 3, 6,    1, 6, 3, 1,  1, 5, 5}, 10, NULL}, EVENT_TYPE_AUTH_FAIL},
    {{{0}, 0, NULL}, EVENT_TYPE_END}
};

static vtss_trap_entry_t *get_next_trap_entry_point(vtss_trap_entry_t *trap_entry);

static event_type_e trap_event_type_get( snmp_vars_trap_entry_t *trap_entry)
{
    event_tbl_t  *tbl = &event_tbl[0];
    for (tbl = &event_tbl[0]; tbl->event_type <= EVENT_TYPE_END; tbl++) {
        if (tbl->event_type == EVENT_TYPE_END || (trap_entry->oid_len == tbl->trap_entry.oid_len &&
                                                  !snmp_oid_compare((const oid *)trap_entry->oid, trap_entry->oid_len,
                                                                    (const oid *)tbl->trap_entry.oid, tbl->trap_entry.oid_len))) {
            return tbl->event_type;
        }
    }
    return tbl->event_type;
}

static char *get_localAddr(char *peername, u16 trap_port)
{
    struct in_addr      address;
    struct sockaddr_in  server_addr;
    struct hostent      *host;
    int                 sock;
    socklen_t           my_addr_len;
    struct sockaddr_in  my_addr;
    static char         my_addr_buf[VTSS_SYS_HOSTNAME_LEN + 1] = "\0";

    /* Fill server address information */
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(trap_port);

    /* Look up the DNS host name or IPv4 address */
    if ((server_addr.sin_addr.s_addr = inet_addr(peername)) == (unsigned long) - 1) {
        if ((host = gethostbyname(peername)) != NULL &&
            (host->h_length == sizeof(struct in_addr))) {
            address = *((struct in_addr **)host->h_addr_list)[0];
            server_addr.sin_addr.s_addr = address.s_addr;
        } else {
            T_D("gethostbyname() fail: %s\n", peername);
            return NULL;
        }
    }
    if (server_addr.sin_addr.s_addr == (unsigned long) - 1 || server_addr.sin_addr.s_addr == 0) {
        return NULL;
    }

    /* Create socket */
    if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        T_D("Create syslog socket failed: %s", strerror(errno));
        return NULL;
    }

    /* Connect socket */
    if (connect(sock, (struct sockaddr *)&server_addr,
                sizeof(struct sockaddr)) != 0) {
        close(sock);
        T_D("Connect syslog socket failed: %s", strerror(errno));
        return NULL;
    }

    /* Get my address */
    my_addr_len = sizeof(my_addr);
    if (getsockname(sock, (struct sockaddr *)&my_addr,
                    &my_addr_len) != 0) {
        close(sock);
        T_D("Get syslog my address sockname failed: %s", strerror(errno));
        return NULL;
    }

    /* Convert to my address string */
    my_addr_buf[0] = '\0';
    (void) inet_ntop(my_addr.sin_family,
                     (const char *)&my_addr.sin_addr,
                     my_addr_buf, VTSS_SYS_HOSTNAME_LEN + 1);

    close(sock);
    return my_addr_buf;
}


static void ucd_trap_conf_set(vtss_trap_entry_t *trap_entry, ucd_trap_conf_t *trap_conf )
{
#ifdef VTSS_SW_OPTION_IPV6
    char        ipv6_str_buff[40];
#endif /* VTSS_SW_OPTION_IPV6 */
    vtss_trap_conf_t *conf = &trap_entry->trap_conf;
    i8              *localAddr = NULL;
    trap_conf->conf_id = conf->conf_id;
    if ( FALSE == conf->dip.ipv6_flag) {
        strcpy(trap_conf->trap_dip_str, conf->dip.addr.ipv4_str);
        trap_conf->ipv6_flag = 0;
#ifdef VTSS_SW_OPTION_IPV6
    } else {
        (void )misc_ipv6_txt(&conf->dip.addr.ipv6, ipv6_str_buff);
        strcpy(trap_conf->trap_dip_str, ipv6_str_buff);
        ipv6_str_buff[0] = 0;
        trap_conf->ipv6_flag = 1;
#endif /* VTSS_SW_OPTION_IPV6 */
    }

    if (conf->dip.ipv6_flag == TRUE) {
        localAddr = NULL;
    } else {
        localAddr = get_localAddr(conf->dip.addr.ipv4_str, conf->trap_port);
    }

    if (localAddr != NULL) {
        strcpy(trap_conf->trap_sip_str, localAddr);
    } else {
        trap_conf->trap_sip_str[0] = 0;
    }
    trap_conf->trap_port = conf->trap_port;
    trap_conf->trap_version = conf->trap_version;
    strcpy(trap_conf->trap_community, conf->trap_community);
    trap_conf->trap_inform_mode = (conf->trap_inform_mode == VTSS_TRAP_MSG_TRAP2) ? SNMP_MSG_TRAP2 : SNMP_MSG_INFORM;
    trap_conf->trap_inform_timeout = conf->trap_inform_timeout;
    trap_conf->trap_inform_retries = conf->trap_inform_retries;
#ifdef CYGPKG_SNMPAGENT_V3_SUPPORT
    trap_conf->trap_probe_engineid = conf->trap_probe_engineid;
    trap_conf->trap_engineid_len = conf->trap_engineid_len;
    memcpy(trap_conf->trap_engineid, conf->trap_engineid, conf->trap_engineid_len);
    strcpy((char *)trap_conf->trap_security_name, conf->trap_security_name);
#endif /* CYGPKG_SNMPAGENT_V3_SUPPORT */

}

extern int snmp_set_var_value(struct variable_list *, u_char *, size_t);
extern int snmp_set_var_objid (struct variable_list *vp,
                               const oid *objid, size_t name_length);

static vtss_rc trap_event_send_snmpTraps(snmp_trap_buff_t *trap_buff)
{
    oid ifIndex_oid[] = IFINDEX_OID;
    oid ifAdmin_oid[] = IFADMIN_OID;
    oid ifOper_oid[] = IFOPER_OID;
    int ifTable_len = sizeof(ifIndex_oid) / sizeof(oid);
    struct variable_list trap_var, ifIndex_var, ifAdmin_var, ifOper_var;
    iftable_info_t info;
    vtss_trap_entry_t trap_entry, *tmp;
    vtss_trap_conf_t *conf = &trap_entry.trap_conf;
    vtss_trap_event_t *event = &trap_entry.trap_event;
    ucd_trap_conf_t trap_conf;
    snmp_trap_type_t trap_type = trap_buff->trap_type;

    if (FALSE == trap_global.trap_mode) {
        return VTSS_RC_OK;
    }

    memset(&trap_entry, 0x0, sizeof(trap_entry));
    memset(&trap_var, 0, sizeof(trap_var));
    memset(&ifIndex_var, 0, sizeof(struct variable_list));
    memset(&ifAdmin_var, 0, sizeof(struct variable_list));
    memset(&ifOper_var, 0, sizeof(struct variable_list));

    info.ifIndex = trap_buff->trap_if_info.if_idx;

    ifIndex_var.next_variable  = &ifAdmin_var;
    ifAdmin_var.next_variable  = &ifOper_var;

    ifOper_var.next_variable  = NULL;
    ifIndex_oid[ifTable_len - 1] = info.ifIndex;
    ifAdmin_oid[ifTable_len - 1] = info.ifIndex;
    ifOper_oid[ifTable_len - 1]  = info.ifIndex;

    ( void )snmp_set_var_objid( &ifIndex_var, ifIndex_oid, OID_LENGTH(ifIndex_oid));
    ( void )snmp_set_var_objid( &ifAdmin_var, ifAdmin_oid, OID_LENGTH(ifAdmin_oid));
    ( void )snmp_set_var_objid( &ifOper_var, ifOper_oid, OID_LENGTH(ifOper_oid));

    (void)snmp_set_var_value( &ifIndex_var, (u_char *)&trap_buff->trap_if_info.if_idx,   sizeof(long));
    (void)snmp_set_var_value( &ifAdmin_var, (u_char *)&trap_buff->trap_if_info.if_admin, sizeof(long));
    (void)snmp_set_var_value( &ifOper_var,  (u_char *)&trap_buff->trap_if_info.if_oper,  sizeof(long));

    ifIndex_var.type = ASN_INTEGER;
    ifAdmin_var.type = ASN_INTEGER;
    ifOper_var.type  = ASN_INTEGER;

    if (trap_type == SNMP_TRAP_TYPE_SPECIFIC_OID || trap_type == SNMP_TRAP_TYPE_SPECIFIC_OID_WITH_IF_IDX) {
        oid trap_oid[] = SNMP_TRAP_OID;
        (void)snmp_set_var_objid(&trap_var, trap_oid, OID_LENGTH(trap_oid));
        (void)snmp_set_var_value(&trap_var, (u_char *)trap_buff->trap_entry.oid, trap_buff->trap_entry.oid_len * sizeof(oid));
        trap_var.type = ASN_OBJECT_ID;
        trap_conf.vars = &trap_var;
        trap_conf.generic = SNMP_TRAP_ENTERPRISESPECIFIC;
        if (trap_type == SNMP_TRAP_TYPE_SPECIFIC_OID_WITH_IF_IDX) {
            trap_var.next_variable = &ifIndex_var;
            ifIndex_var.next_variable = NULL;
        }
    } else  {
        trap_conf.vars = &ifIndex_var;
    }

    if ( (trap_type == SNMP_TRAP_TYPE_LINKUP || trap_type == SNMP_TRAP_TYPE_LINKDOWN ) &&
         (FALSE == ifIndex_get(&info) || info.type != IFTABLE_IFINDEX_TYPE_PORT)) {
        return VTSS_RC_ERROR;
    }

    trap_conf.oid[0] = 0;
    trap_conf.oid_len = 0;

    SNMP_CRIT_ENTER();
    while (NULL != (tmp = get_next_trap_entry_point(&trap_entry))) {
        trap_entry = *tmp;
        if ( FALSE == conf->enable) {
            continue;
        }
        ucd_trap_conf_set(&trap_entry, &trap_conf );

        switch (trap_type) {
        case SNMP_TRAP_TYPE_WARMSTART:
            T_D("SNMP_TRAP_TYPE_WARMSTART");
            if ( FALSE == event->system.warm_start) {
                continue;
            }

            trap_conf.vars = NULL;
            trap_conf.generic = SNMP_TRAP_WARMSTART;
            break;
        case SNMP_TRAP_TYPE_COLDSTART:
            T_D("SNMP_TRAP_TYPE_COLDSTART");
            if ( FALSE == event->system.cold_start) {
                continue;
            }
            trap_conf.vars = NULL;
            trap_conf.generic = SNMP_TRAP_COLDSTART;
            break;
        case SNMP_TRAP_TYPE_LINKUP:
            T_D("SNMP_TRAP_TYPE_LINKUP");
            if ( FALSE == event->interface.trap_linkup[info.isid][info.if_id]) {
                continue;
            }
            trap_conf.generic = SNMP_TRAP_LINKUP;
            break;
        case SNMP_TRAP_TYPE_LINKDOWN:
            T_D("SNMP_TRAP_TYPE_LINKDOWN");
            if ( FALSE == event->interface.trap_linkdown[info.isid][info.if_id]) {
                continue;
            }
            trap_conf.generic = SNMP_TRAP_LINKDOWN;
            break;
        case SNMP_TRAP_TYPE_AUTHFAIL:
            T_D("SNMP_TRAP_TYPE_COLDSTART");
            if ( FALSE == event->aaa.trap_authen_fail) {
                continue;
            }
            trap_conf.vars = NULL;
            trap_conf.generic = SNMP_TRAP_AUTHFAIL;
            break;

        case SNMP_TRAP_TYPE_SPECIFIC_OID:
        case SNMP_TRAP_TYPE_SPECIFIC_OID_WITH_IF_IDX:
            break;

        default:
            T_E("Unkown Event");
            continue;
        }

        T_D("Send generic trap Message from %s to %s", trap_conf.trap_sip_str, trap_conf.trap_dip_str);

        SNMP_CRIT_EXIT();
        if ( ucd_send_trap_message (&trap_conf) ) {
            T_D("send trap message fail");
        }
        SNMP_CRIT_ENTER();

    }
    SNMP_CRIT_EXIT();
    return VTSS_RC_OK;

}

static vtss_rc trap_event_send_vars(snmp_vars_trap_entry_t *trap_entry_vars)
{
    vtss_trap_entry_t trap_entry, *tmp;
    vtss_trap_event_t *event = &trap_entry.trap_event;
    ucd_trap_conf_t trap_conf;
    event_type_e event_type = trap_event_type_get(trap_entry_vars);
    /*lint -esym(429, event_var) */
    struct variable_list *event_var;
    oid trap_oid[] = SNMP_TRAP_OID;

    if (FALSE == trap_global.trap_mode) {
        return VTSS_RC_OK;
    }

    // snmp_free_varbind() is used to free this pointer, once used.
    // Therefore we use an SNMP allocation function, rather than VTSS_MALLOC().
    // SNMP_MALLOC_STRUCT() also zeroes out the allocated structure.
    event_var = SNMP_MALLOC_STRUCT(variable_list);

    if (!event_var) {
        T_E("out of memory");
        return VTSS_RC_ERROR;
    }

    memset(&trap_entry, 0, sizeof(trap_entry));
    //event_var->next_variable  = NULL;
    event_var->next_variable  = trap_entry_vars->vars;
    trap_entry_vars->vars = event_var;

    ( void )snmp_set_var_objid( event_var, trap_oid, OID_LENGTH(trap_oid));
    ( void )snmp_set_var_value( event_var, (u_char *)trap_entry_vars->oid, trap_entry_vars->oid_len * sizeof(oid));
    event_var->type           = ASN_OBJECT_ID;

    SNMP_CRIT_ENTER();
    while (NULL != (tmp = get_next_trap_entry_point(&trap_entry))) {
        trap_entry = *tmp;
        ucd_trap_conf_set(&trap_entry, &trap_conf );

        trap_conf.generic = SNMP_TRAP_ENTERPRISESPECIFIC;
        trap_conf.oid[0] = 0;
        trap_conf.oid_len = 0;
        trap_conf.vars = trap_entry_vars->vars;

        switch (event_type) {
        case EVENT_TYPE_WARM_START:
            T_D("EVENT_TYPE_WARM_START");
            if ( FALSE == event->system.warm_start) {
                continue;
            }
            break;
        case EVENT_TYPE_COLD_START:
            T_D("EVENT_TYPE_COLD_START");
            if ( FALSE == event->system.cold_start) {
                continue;
            }
            break;
        case EVENT_TYPE_LINK_UP:
            T_D("EVENT_TYPE_LINK_UP");
            break;
        case EVENT_TYPE_LINK_DOWN:
            T_D("EVENT_TYPE_LINK_DOWN");
            break;
        case EVENT_TYPE_LLDP: {
            struct variable_list *ifIndex_var;
            iftable_info_t info;

            /* old style, the ifIndex OID isn't included in variable binding */
            if ( NULL == trap_entry_vars->vars || NULL == (ifIndex_var = trap_entry_vars->vars->next_variable)) {
                break;
            }

            info.ifIndex = ifIndex_var->name[ifIndex_var->name_length - 1];
            if ( FALSE == ifIndex_get(&info) || info.type != IFTABLE_IFINDEX_TYPE_PORT) {
                T_E("ifIndex(%lu) is not available", info.ifIndex);
            }
            T_D("EVENT_TYPE_LLDP");
            T_D("%d/%d LLDP is %s", info.isid, info.if_id, event->interface.trap_lldp[info.isid][info.if_id] ? "enabled" : "disabled");
            if ( FALSE == event->interface.trap_lldp[info.isid][info.if_id]) {
                continue;
            }
        }
        break;
        case EVENT_TYPE_AUTH_FAIL:
            T_D("EVENT_TYPE_AUTH_FAIL");
            if ( FALSE == event->aaa.trap_authen_fail) {
                continue;
            }
            break;
        case EVENT_TYPE_STP:
            T_D("EVENT_TYPE_STP");
            if ( FALSE == event->sw.stp) {
                continue;
            }
            break;
        case EVENT_TYPE_RMON:
            T_D("EVENT_TYPE_RMON");
            if ( FALSE == event->sw.rmon) {
                continue;
            }
            break;
        case EVENT_TYPE_ENTITY:
            T_D("EVENT_TYPE_ENTITY");
            break;
        default:
            T_E("Unkown Event");
            continue;
        }

        T_D("Send specific trap Message from %s to %s", trap_conf.trap_sip_str, trap_conf.trap_dip_str);
        SNMP_CRIT_EXIT();
        (void) ucd_send_trap_message (&trap_conf);
        SNMP_CRIT_ENTER();

    }
    SNMP_CRIT_EXIT();
    return VTSS_RC_OK;

}

/****************************************************************************/
/*  Various local functions                                                 */
/****************************************************************************/

#ifdef SNMP_SUPPORT_V3
/* Determine if IP address valid */
static BOOL snmp_is_ip_valid(BOOL is_mask, ulong ip_addr)
{
    ulong temp_ip_addr = htonl(ip_addr);
    uchar temp_ip[4];
    int idx, zero_found = 0;

    temp_ip[0] = temp_ip_addr & 0xFF;
    temp_ip[1] = (temp_ip_addr >> 8) & 0xFF;
    temp_ip[2] = (temp_ip_addr >> 16) & 0xFF;
    temp_ip[3] = (temp_ip_addr >> 24) & 0xFF;

    if (is_mask) {
        for (idx = 31; idx >= 0; idx--) {
            if ((ip_addr & (1 << idx)) == 0) {
                zero_found = 1;
            } else if (zero_found) {
                return FALSE;
            }
        }
    } else {
        if (temp_ip[0] == 127 || temp_ip[0] > 223) {
            return FALSE;
        }
    }

    return TRUE;
}
#endif /* SNMP_SUPPORT_V3 */

/* Determine if SNMP configuration has changed */
int snmp_conf_changed(snmp_conf_t *old, snmp_conf_t *new)
{
    return (new->mode != old->mode
            || new->version != old->version
            || new->port != old->port
            || strcmp(new->read_community, old->read_community)
            || strcmp(new->write_community, old->write_community)
            || new->trap_mode != old->trap_mode
            || new->trap_version != old->trap_version
            || new->trap_port != old->trap_port
            || strcmp(new->trap_community, old->trap_community)
            || strcmp((char *)new->trap_dip_string, (char *)old->trap_dip_string)
#ifdef VTSS_SW_OPTION_IPV6
            || (memcmp(&new->trap_dipv6, &old->trap_dipv6, sizeof(old->trap_dipv6)) != 0)
#endif /* VTSS_SW_OPTION_IPV6 */
            || new->trap_authen_fail != old->trap_authen_fail
            || new->trap_linkup_linkdown != old->trap_linkup_linkdown
            || new->trap_inform_mode != old->trap_inform_mode
            || new->trap_inform_timeout != old->trap_inform_timeout
            || new->trap_inform_retries != old->trap_inform_retries
#ifdef SNMP_SUPPORT_V3
            || new->trap_probe_security_engineid != old->trap_probe_security_engineid
            || new->trap_security_engineid_len != old->trap_security_engineid_len
            || memcmp(new->trap_security_engineid, old->trap_security_engineid, new->trap_security_engineid_len > old->trap_security_engineid_len ? new->trap_security_engineid_len : old->trap_security_engineid_len)
            || strcmp(new->trap_security_name, old->trap_security_name)
            || new->engineid_len != old->engineid_len
            || memcmp(new->engineid, old->engineid, new->engineid_len > old->engineid_len ? new->engineid_len : old->engineid_len)
#endif /* SNMP_SUPPORT_V3 */
           );
}

/* Determine if SNMP port configuration has changed */
static int snmp_port_conf_changed(snmp_port_conf_t *old, snmp_port_conf_t *new)
{
    return (new->linkupdown_trap_enable != old->linkupdown_trap_enable);
}

static int snmp_smon_stat_entry_changed(snmp_rmon_stat_entry_t *old, snmp_rmon_stat_entry_t *new)
{
    return (new->valid != old->valid ||
            new->ctrl_index != old->ctrl_index ||
            new->if_index != old->if_index);
}
static int snmp_port_copy_entry_changed(snmp_port_copy_entry_t *old, snmp_port_copy_entry_t *new)
{
    return (new->valid != old->valid ||
            new->source_index != old->source_index ||
            new->dest_index != old->dest_index ||
            new->ctrl_index != old->ctrl_index ||
            new->copydirection != old->copydirection);
}
#ifdef SNMP_SUPPORT_V3
/* Determine if SNMPv3 communities configuration has changed */
int snmpv3_communities_conf_changed(snmpv3_communities_conf_t *old, snmpv3_communities_conf_t *new)
{
    return (new->valid != old->valid ||
            strcmp(new->community, old->community) ||
            new->sip != old->sip ||
            new->sip_mask != old->sip_mask);
}

/* Determine if SNMPv3 users configuration has changed */
int snmpv3_users_conf_changed(snmpv3_users_conf_t *old, snmpv3_users_conf_t *new)
{
    return (new->valid != old->valid ||
            new->engineid_len != old->engineid_len ||
            memcmp(new->engineid, old->engineid, new->engineid_len > old->engineid_len ? new->engineid_len : old->engineid_len) ||
            strcmp(new->user_name, old->user_name) ||
            new->security_level != old->security_level ||
            new->auth_protocol != old->auth_protocol ||
            strcmp(new->auth_password, old->auth_password) ||
            new->priv_protocol != old->priv_protocol ||
            strcmp(new->priv_password, old->priv_password) ||
            new->storage_type != old->storage_type ||
            new->status != old->status);
}

/* Determine if SNMPv3 groups configuration has changed */
int snmpv3_groups_conf_changed(snmpv3_groups_conf_t *old, snmpv3_groups_conf_t *new)
{
    return (new->valid != old->valid ||
            strcmp(new->group_name, old->group_name) ||
            new->security_model != old->security_model ||
            new->storage_type != old->storage_type ||
            new->status != old->status);
}

/* Determine if SNMPv3 views configuration has changed */
int snmpv3_views_conf_changed(snmpv3_views_conf_t *old, snmpv3_views_conf_t *new)
{
    return (new->valid != old->valid ||
            strcmp(new->view_name, old->view_name) ||
            memcmp(new->subtree, old->subtree, new->subtree_len > old->subtree_len ? sizeof(ulong) * new->subtree_len : sizeof(ulong) * old->subtree_len) ||
            new->subtree_len != old->subtree_len ||
            memcmp(new->subtree_mask, old->subtree_mask, new->subtree_mask_len > old->subtree_mask_len ? new->subtree_mask_len : old->subtree_mask_len) ||
            new->subtree_mask_len != old->subtree_mask_len ||
            new->view_type != old->view_type ||
            new->storage_type != old->storage_type ||
            new->status != old->status);
}

/* Determine if SNMPv3 accesses configuration has changed */
int snmpv3_accesses_conf_changed(snmpv3_accesses_conf_t *old, snmpv3_accesses_conf_t *new)
{
    return (new->valid != old->valid ||
            strcmp(new->group_name, old->group_name) ||
            strcmp(new->context_prefix, old->context_prefix) ||
            new->security_model != old->security_model ||
            new->security_level != old->security_level ||
            new->context_match != old->context_match ||
            strcmp(new->read_view_name, old->read_view_name) ||
            strcmp(new->write_view_name, old->write_view_name) ||
            strcmp(new->notify_view_name, old->notify_view_name) ||
            new->storage_type != old->storage_type ||
            new->status != old->status);
}
#endif /* SNMP_SUPPORT_V3 */

/* Get SNMP defaults */
void snmp_default_get(snmp_conf_t *conf)
{
    conf->mode = SNMP_DEFAULT_MODE;
    conf->version = SNMP_SUPPORT_V2C;
    conf->port = 161;
    strcpy(conf->read_community, SNMP_DEFAULT_RO_COMMUNITY);
    strcpy(conf->write_community, SNMP_DEFAULT_RW_COMMUNITY);
    conf->trap_mode = SNMP_DEFAULT_TRAP_MODE;
    conf->trap_version = SNMP_SUPPORT_V1;
    conf->trap_port = 162;
    strcpy(conf->trap_community, SNMP_DEFAULT_TRAP_COMMUNITY);
    strcpy(conf->trap_dip_string, "");
#ifdef VTSS_SW_OPTION_IPV6
    memset(&conf->trap_dipv6, 0, sizeof(conf->trap_dipv6));
#endif /* VTSS_SW_OPTION_IPV6 */
    conf->trap_authen_fail = SNMP_DEFAULT_TRAP_AUTHEN_FAIL;
    conf->trap_linkup_linkdown = SNMP_DEFAULT_TRAP_LINKUP_LINKDOWN;
    conf->trap_inform_mode = SNMP_DEFAULT_MODE;
    conf->trap_inform_timeout = SNMP_DEFAULT_TRAP_INFORM_TIMEOUT;
    conf->trap_inform_retries = SNMP_DEFAULT_TRAP_INFORM_RETRIES;
#ifdef SNMP_SUPPORT_V3
    {
        uchar default_engineid[] = SNMPV3_DEFAULT_ENGINE_ID;

        conf->trap_probe_security_engineid = SNMP_DEFAULT_TRAP_SECURITY_ENGINE_ID_PROBE;
        conf->trap_security_engineid_len = 0;
        memset(conf->trap_security_engineid, 0x0, sizeof(conf->trap_security_engineid));
        strcpy(conf->trap_security_name, SNMPV3_NONAME);
        conf->engineid_len = sizeof(default_engineid);
        memcpy(conf->engineid, default_engineid, conf->engineid_len);
    }
#endif /* SNMP_SUPPORT_V3 */
}

/* Set SNMP port defaults */
static void snmp_port_default_set(snmp_port_conf_t *conf)
{
    conf->linkupdown_trap_enable = 1;
}

#ifdef SNMP_SUPPORT_V3
/* Set SNMPv3 communities defaults */
void snmpv3_default_communities_get(ulong *conf_num, snmpv3_communities_conf_t *conf)
{
    if (conf_num) {
        *conf_num = 2;
    }

    if (conf) {
        conf->idx = 1;
        conf->valid = 1;
        strcpy(conf->community, SNMP_DEFAULT_RO_COMMUNITY);
        conf->sip = SNMPV3_DEFAULT_COMMUNITY_SIP;
        conf->sip_mask = SNMPV3_DEFAULT_COMMUNITY_SIP_MASK;
        conf->storage_type = SNMP_MGMT_STORAGE_PERMANENT;
        conf->status = SNMP_MGMT_ROW_ACTIVE;

        conf++;
        conf->idx = 2;
        conf->valid = 1;
        strcpy(conf->community, SNMP_DEFAULT_RW_COMMUNITY);
        conf->sip = SNMPV3_DEFAULT_COMMUNITY_SIP;
        conf->sip_mask = SNMPV3_DEFAULT_COMMUNITY_SIP_MASK;
        conf->storage_type = SNMP_MGMT_STORAGE_PERMANENT;
        conf->status = SNMP_MGMT_ROW_ACTIVE;
    }
}

/* Get SNMPv3 users defaults */
void snmpv3_default_users_get(ulong *conf_num, snmpv3_users_conf_t *conf)
{
    uchar default_engineid[] = SNMPV3_DEFAULT_ENGINE_ID;

    if ( conf_num ) {
        *conf_num = 1;
    }

    if ( conf ) {
        conf->idx = 1;
        conf->valid = 1;
        conf->engineid_len = sizeof(default_engineid);
        memcpy(conf->engineid, default_engineid, conf->engineid_len);
        strcpy(conf->user_name, SNMPV3_DEFAULT_USER);
        conf->security_level = SNMP_MGMT_SEC_LEVEL_NOAUTH;
        conf->auth_protocol = SNMP_MGMT_AUTH_PROTO_NONE;
        strcpy(conf->auth_password, "");
        conf->priv_protocol = SNMP_MGMT_PRIV_PROTO_NONE;
        strcpy(conf->priv_password, "");
        conf->storage_type = SNMP_MGMT_STORAGE_PERMANENT;
        conf->status = SNMP_MGMT_ROW_ACTIVE;
    }
}

/* Get SNMPv3 groups defaults */
void snmpv3_default_groups_get(ulong *conf_num, snmpv3_groups_conf_t *conf)
{
    if ( conf_num ) {
        *conf_num = 5;
    }

    if (conf) {
        conf->idx = 1;
        conf->valid = 1;
        conf->security_model = SNMP_MGMT_SEC_MODEL_SNMPV1;
        strcpy(conf->security_name, SNMP_DEFAULT_RO_COMMUNITY);
        strcpy(conf->group_name, SNMPV3_DEFAULT_RO_GROUP);
        conf->storage_type = SNMP_MGMT_STORAGE_PERMANENT;
        conf->status = SNMP_MGMT_ROW_ACTIVE;

        conf++;
        conf->idx = 2;
        conf->valid = 1;
        conf->security_model = SNMP_MGMT_SEC_MODEL_SNMPV1;
        strcpy(conf->security_name, SNMP_DEFAULT_RW_COMMUNITY);
        strcpy(conf->group_name, SNMPV3_DEFAULT_RW_GROUP);
        conf->storage_type = SNMP_MGMT_STORAGE_PERMANENT;
        conf->status = SNMP_MGMT_ROW_ACTIVE;

        conf++;
        conf->idx = 3;
        conf->valid = 1;
        conf->security_model = SNMP_MGMT_SEC_MODEL_SNMPV2C;
        strcpy(conf->security_name, SNMP_DEFAULT_RO_COMMUNITY);
        strcpy(conf->group_name, SNMPV3_DEFAULT_RO_GROUP);
        conf->storage_type = SNMP_MGMT_STORAGE_PERMANENT;
        conf->status = SNMP_MGMT_ROW_ACTIVE;

        conf++;
        conf->idx = 4;
        conf->valid = 1;
        conf->security_model = SNMP_MGMT_SEC_MODEL_SNMPV2C;
        strcpy(conf->security_name, SNMP_DEFAULT_RW_COMMUNITY);
        strcpy(conf->group_name, SNMPV3_DEFAULT_RW_GROUP);
        conf->storage_type = SNMP_MGMT_STORAGE_PERMANENT;
        conf->status = SNMP_MGMT_ROW_ACTIVE;

        conf++;
        conf->idx = 5;
        conf->valid = 1;
        conf->security_model = SNMP_MGMT_SEC_MODEL_USM;
        strcpy(conf->security_name, SNMPV3_DEFAULT_USER);
        strcpy(conf->group_name, SNMPV3_DEFAULT_RW_GROUP);
        conf->storage_type = SNMP_MGMT_STORAGE_PERMANENT;
        conf->status = SNMP_MGMT_ROW_ACTIVE;
    }
}

/* Set SNMPv3 views defaults */
void snmpv3_default_views_get(ulong *conf_num, snmpv3_views_conf_t *conf)
{
    ulong default_subtree[] = {1};
    uchar default_subtree_mask[] = {0x80};

    if ( conf_num ) {
        *conf_num = 1;
    }

    if ( conf ) {
        conf->idx = 1;
        conf->valid = 1;
        strcpy(conf->view_name, SNMPV3_DEFAULT_VIEW);
        conf->subtree_len = 1;
        memcpy(conf->subtree, default_subtree, sizeof(ulong) * conf->subtree_len);
        conf->subtree_mask_len = 1;
        memcpy(conf->subtree_mask, default_subtree_mask, conf->subtree_mask_len);
        conf->view_type = SNMPV3_MGMT_VIEW_INCLUDED;
        conf->storage_type = SNMP_MGMT_STORAGE_PERMANENT;
        conf->status = SNMP_MGMT_ROW_ACTIVE;
    }
}

/* Set SNMPv3 accesses defaults */
void snmpv3_default_accesses_get(ulong *conf_num, snmpv3_accesses_conf_t *conf)
{
    if ( conf_num ) {
        *conf_num = 2;
    }

    if ( conf ) {
        conf->idx = 1;
        conf->valid = 1;
        strcpy(conf->group_name, SNMPV3_DEFAULT_RO_GROUP);
        strcpy(conf->context_prefix, "");
        conf->security_model = SNMP_MGMT_SEC_MODEL_ANY;
        conf->security_level = SNMP_MGMT_SEC_LEVEL_NOAUTH;
        conf->context_match = SNMPV3_MGMT_CONTEX_MATCH_EXACT;
        strcpy(conf->read_view_name, SNMPV3_DEFAULT_VIEW);
        strcpy(conf->write_view_name, SNMPV3_NONAME);
        strcpy(conf->notify_view_name, SNMPV3_NONAME);
        conf->storage_type = SNMP_MGMT_STORAGE_PERMANENT;
        conf->status = SNMP_MGMT_ROW_ACTIVE;

        conf++;
        conf->idx = 2;
        conf->valid = 1;
        strcpy(conf->group_name, SNMPV3_DEFAULT_RW_GROUP);
        strcpy(conf->context_prefix, "");
        conf->security_model = SNMP_MGMT_SEC_MODEL_ANY;
        conf->security_level = SNMP_MGMT_SEC_LEVEL_NOAUTH;
        conf->context_match = SNMPV3_MGMT_CONTEX_MATCH_EXACT;
        strcpy(conf->read_view_name, SNMPV3_DEFAULT_VIEW);
        strcpy(conf->write_view_name, SNMPV3_DEFAULT_VIEW);
        strcpy(conf->notify_view_name, SNMPV3_NONAME);
        conf->storage_type = SNMP_MGMT_STORAGE_PERMANENT;
        conf->status = SNMP_MGMT_ROW_ACTIVE;
    }
}
#endif /* SNMP_SUPPORT_V3 */

/* Setup SNMP configuration to engine */
static void snmp_engine_conf_set(snmp_conf_t *new_conf)
{
#ifdef VTSS_SW_OPTION_IPV6
    vtss_ipv6_t zero_ipv6addr;
    char        ipv6_str_buff[40];

    memset(&zero_ipv6addr, 0, sizeof(zero_ipv6addr));
#endif /* VTSS_SW_OPTION_IPV6 */

    /* Set to the snmp engine only when stack role is master */
    if (!msg_switch_is_master()) {
        return;
    }

    if ((!new_conf->mode) ||
        (!new_conf->trap_mode) ||
#ifdef VTSS_SW_OPTION_IPV6
        (!(strcmp(new_conf->trap_dip_string, "") || (memcmp(&new_conf->trap_dipv6, &zero_ipv6addr, sizeof(zero_ipv6addr)) != 0)))) {
#else
        (!strcmp(new_conf->trap_dip_string, ""))) {
#endif /* VTSS_SW_OPTION_IPV6 */
//        SNMP_CRIT_ENTER();
//        trap_buff_read_idx = trap_buff_write_idx = 0;
//        SNMP_CRIT_EXIT();
    }

#ifdef SNMP_SUPPORT_V3
    if (SnmpdSetEngineId((char *) new_conf->engineid, new_conf->engineid_len) < 0) {
        T_W("Calling SnmpdSetEngineId() failed.\n");
    }
#endif /* SNMP_SUPPORT_V3 */

    SnmpdSetVersion(new_conf->version);
    SnmpdSetMode(new_conf->mode); /* Notice: should be set SNMP mode first */
#ifdef SNMP_SUPPORT_V3
    SnmpdSetTrapSecurityEngineIdProbe(new_conf->trap_probe_security_engineid);
#endif /* SNMP_SUPPORT_V3 */
    SnmpdSetTrapInformRetries(new_conf->trap_inform_retries);
    SnmpdSetTrapInformTimeout(new_conf->trap_inform_timeout * 1000000);
    SnmpdSetTrapInformMode(new_conf->trap_inform_mode);
    SnmpdEnableAuthenTraps(new_conf->trap_authen_fail);
    if (strcmp(new_conf->trap_dip_string, "")) {
        SnmpdSetTrapDestinationIp((uchar *) new_conf->trap_dip_string);
    } else {
        SnmpdSetTrapDestinationIp((uchar *) "");
    }
#ifdef VTSS_SW_OPTION_IPV6
    memset(&zero_ipv6addr, 0, sizeof(zero_ipv6addr));
    if (memcmp(&new_conf->trap_dipv6, &zero_ipv6addr, sizeof(zero_ipv6addr))) {
        SnmpdSetTrapDestinationIpv6((uchar *)misc_ipv6_txt(&new_conf->trap_dipv6, ipv6_str_buff));
    } else {
        SnmpdSetTrapDestinationIpv6((uchar *) "");
    }
#endif /* VTSS_SW_OPTION_IPV6 */
    SnmpdSetTrapCommunity((uchar *)new_conf->trap_community);
    SnmpdSetTrapPort(new_conf->trap_port);
    if (SnmpdSetTrapVersion(new_conf->trap_version)) {
        T_W("Calling SnmpdSetTrapVersion() failed.\n");
    }
    SnmpdSetPort(new_conf->port);
    SnmpdSetTrapMode(new_conf->trap_mode);
}

static void snmp_smon_conf_engine_set(void)
{
    /* Set to the snmp engine only when stack role is master */
    if (!msg_switch_is_master()) {
        return;
    }

#ifdef VTSS_SW_OPTION_SMON
#if defined(VTSS_FEATURE_VLAN_COUNTERS)
    smon_create_stat_default_entry();
#endif /* VTSS_FEATURE_VLAN_COUNTERS */
#endif
}
#ifdef SNMP_SUPPORT_V3
static vtss_rc snmpv3_engine_communities_set(snmpv3_communities_conf_t *conf, BOOL is_add)
{
    char buf[256], buf1[32] = "default", ip_buf[16], ip_mask_buf[16];

    if (conf->status != SNMP_MGMT_ROW_ACTIVE) {
        return ((vtss_rc) SNMP_ERROR_ROW_STATUS_INCONSISTENT);
    }

    if (is_add) {
        if (conf->sip & conf->sip_mask) {
            sprintf(buf1, "%s/%s", misc_ipv4_txt(conf->sip & conf->sip_mask, ip_buf), misc_ipv4_txt(conf->sip_mask, ip_mask_buf));
        }

        sprintf(buf, "%s %s %s", conf->community, buf1, conf->community);
        /* Format: name source community */
        vacm_parse_security(NULL, buf);
    } else {
        SnmpdDelCommunityEntry((uchar *) conf->community);
    }

    return VTSS_OK;
}

static vtss_rc snmpv3_engine_users_set(snmpv3_users_conf_t *conf, BOOL is_add)
{
    char buf[512];
    struct usmUser *puserList = NULL;

    if (conf->status != SNMP_MGMT_ROW_ACTIVE) {
        return ((vtss_rc) SNMP_ERROR_ROW_STATUS_INCONSISTENT);
    }

    if (is_add) {
        sprintf(buf, "-e 0x%s %s %s %s %s %s",
                misc_engineid2str(conf->engineid, conf->engineid_len),
                conf->user_name,
                conf->auth_protocol == SNMP_MGMT_AUTH_PROTO_NONE ? "" : conf->auth_protocol == SNMP_MGMT_AUTH_PROTO_MD5 ? "MD5" : "SHA",
                conf->auth_protocol == SNMP_MGMT_AUTH_PROTO_NONE ? "" : (char *)conf->auth_password,
                conf->priv_protocol == SNMP_MGMT_PRIV_PROTO_NONE ? "" : conf->priv_protocol == SNMP_MGMT_PRIV_PROTO_DES ? "DES" : "AES",
                conf->priv_protocol == SNMP_MGMT_PRIV_PROTO_NONE ? "" : (char *)conf->priv_password);
        /* Format: -e [engineid] username (MD5|SHA) passphrase [DES|AES] [passphrase] */
        usm_parse_create_usmUser(NULL, buf);
    } else {
        if (conf->status != SNMP_MGMT_ROW_ACTIVE) {
            return ((vtss_rc) SNMP_ERROR_ROW_STATUS_INCONSISTENT);
        }

        puserList = usm_get_userList();
        while (puserList != NULL) {
            if (!strcmp(conf->user_name, puserList->name)) {
                if (usm_remove_user(puserList) == NULL) {
                    T_D("Calling usm_remove_user() failed.\n");
                }
                if (usm_free_user(puserList) == NULL) {
                    T_D("Calling usm_free_user() failed.\n");
                }
                break;
            }
            puserList = puserList->next;
        }
    }

    return VTSS_OK;
}

static vtss_rc snmpv3_engine_users_change_password(BOOL is_auth, uchar *engineid, ulong engineid_len, uchar *user_name, uchar *new_password)
{
    if (is_auth) {
        if (SnmpdChangeAuthPd(engineid, engineid_len, (char *)user_name, (char *)new_password)) {
            return ((vtss_rc) SNMP_ERROR_ENGINE_FAIL);
        }
    } else {
        if (SnmpdChangePrivPd(engineid, engineid_len, (char *)user_name, (char *)new_password)) {
            return ((vtss_rc) SNMP_ERROR_ENGINE_FAIL);
        }
    }

    return VTSS_OK;
}

static vtss_rc snmpv3_engine_groups_set(snmpv3_groups_conf_t *conf, BOOL is_add)
{
    char buf[256];

    if (conf->status != SNMP_MGMT_ROW_ACTIVE) {
        return ((vtss_rc) SNMP_ERROR_ROW_STATUS_INCONSISTENT);
    }

    if (is_add) {
        sprintf(buf, "%s %s %s",
                conf->group_name,
                conf->security_model == SNMP_MGMT_SEC_MODEL_SNMPV1 ? "v1" : conf->security_model == SNMP_MGMT_SEC_MODEL_SNMPV2C ? "v2c" : "usm",
                conf->security_name);
        /* Format: name v1|v2c|usm security */
        vacm_parse_group(NULL, buf);
    } else {
        vacm_destroyGroupEntry(conf->security_model, conf->security_name);
    }

    return VTSS_OK;
}

static vtss_rc snmpv3_engine_views_set(snmpv3_views_conf_t *conf, BOOL is_add)
{
    char  buf[256];

    if (conf->status != SNMP_MGMT_ROW_ACTIVE) {
        return ((vtss_rc) SNMP_ERROR_ROW_STATUS_INCONSISTENT);
    }

    if (is_add) {
        sprintf(buf, "%s %s %s %s",
                conf->view_name,
                conf->view_type == SNMPV3_MGMT_VIEW_INCLUDED ? "included" : "excluded",
                misc_oid2str(conf->subtree, conf->subtree_len, NULL, 0),
                misc_oidmask2str(conf->subtree_mask, conf->subtree_len));
        /* Format: name type subtree [mask] */
        vacm_parse_view(NULL, buf);
    } else {
        vacm_destroyViewEntry(conf->view_name, (oid *)conf->subtree, conf->subtree_len);
    }

    return VTSS_OK;
}

static vtss_rc snmpv3_engine_accesses_set(snmpv3_accesses_conf_t *conf, BOOL is_add)
{
    char buf[256];

    if (conf->status != SNMP_MGMT_ROW_ACTIVE) {
        return ((vtss_rc) SNMP_ERROR_ROW_STATUS_INCONSISTENT);
    }

    if (is_add) {
        sprintf(buf, "%s %s %s %s %s %s %s %s",
                conf->group_name,
                strcmp(conf->context_prefix, "") ? (char *)conf->context_prefix : "\"\"",
                conf->security_model == SNMP_MGMT_SEC_MODEL_ANY ? "any" : conf->security_model == SNMP_MGMT_SEC_MODEL_SNMPV1 ? "v1" : conf->security_model == SNMP_MGMT_SEC_MODEL_SNMPV2C ? "v2c" : "usm",
                conf->security_level == SNMP_MGMT_SEC_LEVEL_NOAUTH ? "noauthnopriv" : conf->security_level == SNMP_MGMT_SEC_LEVEL_AUTHNOPRIV ? "authnopriv" : "authpriv",
                conf->context_match == SNMPV3_MGMT_CONTEX_MATCH_EXACT ? "exact" : "prefix",
                conf->read_view_name,
                conf->write_view_name,
                conf->notify_view_name);
        /* Format: name context model level prefx read write notify */
        vacm_parse_access(NULL, buf);
    } else {
        vacm_destroyAccessEntry(conf->group_name, conf->context_prefix, conf->security_model, conf->security_level);
    }

    return VTSS_OK;
}

static void snmpv3_engine_create_communities_default_entry(void)
{
    snmpv3_communities_conf_t conf;

    /* clear SNMP engine communities data */
    vacm_free_security();

    /* setup SNMP engine communities form mamaged configuration */
    strcpy(conf.community, SNMPV3_CONF_ACESS_GETFIRST);
    while (snmpv3_mgmt_communities_conf_get(&conf, TRUE) == VTSS_OK) {
        if (snmpv3_engine_communities_set(&conf, TRUE) != VTSS_OK) {
            T_W("SNMP engine occur fail - snmpv3_engine_communities_set(TRUE)");
        }
    }
}

static void snmpv3_engine_create_users_default_entry(void)
{
    snmpv3_users_conf_t conf;

    /* clear SNMP engine users data */
    SnmpdDestroyAllUserEntries();

    /* Note: we set engine ID first time here,
             because new users need refer to the engine ID.
             otherwize the first time will be happened in snmp_engine_conf_set(),
             that maybe lost created users */
    SNMP_CRIT_ENTER();
    if (SnmpdSetEngineId((char *) snmp_global.snmp_conf.engineid, snmp_global.snmp_conf.engineid_len) < 0) {
        T_W("Calling SnmpdSetEngineId() failed.\n");
    }
    SNMP_CRIT_EXIT();

    /* setup SNMP engine users form mamaged configuration */
    strcpy(conf.user_name, SNMPV3_CONF_ACESS_GETFIRST);
    while (snmpv3_mgmt_users_conf_get(&conf, TRUE) == VTSS_OK) {
        if (snmpv3_engine_users_set(&conf, TRUE) != VTSS_OK) {
            T_W("SNMP engine occur fail - snmpv3_engine_users_set(TRUE)");
        }
    }
}

static void snmpv3_engine_create_groups_default_entry(void)
{
    snmpv3_groups_conf_t conf;

    /* clear SNMP engine groups data */
    vacm_destroyAllGroupEntries();

    /* setup SNMP engine groups form mamaged configuration */
    strcpy(conf.security_name, SNMPV3_CONF_ACESS_GETFIRST);
    while (snmpv3_mgmt_groups_conf_get(&conf, TRUE) == VTSS_OK) {
        if (snmpv3_engine_groups_set(&conf, TRUE) != VTSS_OK) {
            T_W("SNMP engine occur fail - snmpv3_engine_groups_set(TRUE)");
        }
    }
}

static void snmpv3_engine_create_views_default_entry(void)
{
    snmpv3_views_conf_t conf;

    /* clear SNMP engine views data */
    vacm_destroyAllViewEntries();

    /* setup SNMP engine views form mamaged configuration */
    strcpy(conf.view_name, SNMPV3_CONF_ACESS_GETFIRST);
    while (snmpv3_mgmt_views_conf_get(&conf, TRUE) == VTSS_OK) {
        if (snmpv3_engine_views_set(&conf, TRUE) != VTSS_OK) {
            T_W("SNMP engine occur fail - snmpv3_engine_views_set(TRUE)");
        }
    }
}

static void snmpv3_engine_create_accesses_default_entry(void)
{
    snmpv3_accesses_conf_t conf;

    /* clear SNMP engine accesses data */
    vacm_destroyAllAccessEntries();

    /* setup SNMP engine accesses form mamaged configuration */
    strcpy(conf.group_name, SNMPV3_CONF_ACESS_GETFIRST);
    while (snmpv3_mgmt_accesses_conf_get(&conf, TRUE) == VTSS_OK) {
        if (snmpv3_engine_accesses_set(&conf, TRUE) != VTSS_OK) {
            T_W("SNMP engine occur fail - snmpv3_engine_accesses_set(TRUE)");
        }
    }
}

/* Set SNMPv3 configuration to engine */
static void snmpv3_engine_conf_set(void)
{
    /* Set to the snmp engine only when stack role is master */
    if (!msg_switch_is_master()) {
        return;
    }

    snmpv3_engine_create_communities_default_entry();
    snmpv3_engine_create_users_default_entry();
    snmpv3_engine_create_groups_default_entry();
    snmpv3_engine_create_views_default_entry();
    snmpv3_engine_create_accesses_default_entry();
}
#endif /* SNMP_SUPPORT_V3 */


/****************************************************************************/
/*  Stack/switch functions                                                  */
/****************************************************************************/

#if (VTSS_TRACE_LVL_MIN <= VTSS_TRACE_LVL_DEBUG)
static char *snmp_msg_id_txt(snmp_msg_id_t msg_id)
{
    char *txt;

    switch (msg_id) {
    case SNMP_MSG_ID_SNMP_CONF_SET_REQ:
        txt = "SNMP_MSG_ID_SNMP_CONF_SET_REQ";
        break;
    default:
        txt = "?";
        break;
    }
    return txt;
}
#endif /* VTSS_TRACE_LVL_DEBUG */

/* Allocate request buffer */
static snmp_msg_req_t *snmp_msg_req_alloc(snmp_msg_buf_t *buf, snmp_msg_id_t msg_id)
{
    snmp_msg_req_t *msg = &snmp_global.request.msg;

    buf->sem = &snmp_global.request.sem;
    buf->msg = msg;
    (void) VTSS_OS_SEM_WAIT(buf->sem);
    msg->msg_id = msg_id;
    return msg;
}

/* Free request/reply buffer */
static void snmp_msg_free(vtss_os_sem_t *sem)
{
    VTSS_OS_SEM_POST(sem);
}

static void snmp_msg_tx_done(void *contxt, void *msg, msg_tx_rc_t rc)
{
#if (VTSS_TRACE_LVL_MIN <= VTSS_TRACE_LVL_DEBUG)
    snmp_msg_id_t msg_id = *(snmp_msg_id_t *)msg;
#endif /* VTSS_TRACE_LVL_DEBUG */

    T_D("msg_id: %d, %s", msg_id, snmp_msg_id_txt(msg_id));
    snmp_msg_free(contxt);
}

static void snmp_msg_tx(snmp_msg_buf_t *buf, vtss_isid_t isid, size_t len)
{
#if (VTSS_TRACE_LVL_MIN <= VTSS_TRACE_LVL_DEBUG)
    snmp_msg_id_t msg_id = *(snmp_msg_id_t *)buf->msg;
#endif /* VTSS_TRACE_LVL_DEBUG */

    T_D("msg_id: %d, %s, len: %zu, isid: %d", msg_id, snmp_msg_id_txt(msg_id), len, isid);
    msg_tx_adv(buf->sem, snmp_msg_tx_done, MSG_TX_OPT_DONT_FREE,
               VTSS_MODULE_ID_SNMP, isid, buf->msg, len + MSG_TX_DATA_HDR_LEN(snmp_msg_req_t, req));
}

static BOOL snmp_msg_rx(void *contxt, const void *const rx_msg, const size_t len, const vtss_module_id_t modid, const ulong isid)
{
    snmp_msg_id_t msg_id = *(snmp_msg_id_t *)rx_msg;

    T_D("msg_id: %d, %s, len: %zu, isid: %u", msg_id, snmp_msg_id_txt(msg_id), len, isid);

    switch (msg_id) {
    case SNMP_MSG_ID_SNMP_CONF_SET_REQ: {
        snmp_msg_req_t  *msg;

        msg = (snmp_msg_req_t *)rx_msg;
        snmp_engine_conf_set(&msg->req.conf_set.conf);
        break;
    }
    default:
        T_W("unknown message ID: %d", msg_id);
        break;
    }
    return TRUE;
}

static vtss_rc snmp_stack_register(void)
{
    msg_rx_filter_t filter;

    memset(&filter, 0, sizeof(filter));
    filter.cb = snmp_msg_rx;
    filter.modid = VTSS_MODULE_ID_SNMP;
    return msg_rx_filter_register(&filter);
}

/* Set stack SNMP configuration */
static void snmp_stack_snmp_conf_set(vtss_isid_t isid_add)
{
    snmp_msg_req_t  *msg;
    snmp_msg_buf_t  buf;
    vtss_isid_t     isid;

    T_D("enter, isid_add: %d", isid_add);
    for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
        if ((isid_add != VTSS_ISID_GLOBAL && isid_add != isid) ||
            !msg_switch_exists(isid)) {
            continue;
        }
        SNMP_CRIT_ENTER();
        msg = snmp_msg_req_alloc(&buf, SNMP_MSG_ID_SNMP_CONF_SET_REQ);
        msg->req.conf_set.conf = snmp_global.snmp_conf;
        SNMP_CRIT_EXIT();
        snmp_msg_tx(&buf, isid, sizeof(msg->req.conf_set.conf));
    }

    T_D("exit, isid_add: %d", isid_add);
}


/****************************************************************************/
/*  Management functions                                                    */
/****************************************************************************/

/* SNMP error text */
char *snmp_error_txt(snmp_error_t rc)
{
    char *txt;

    switch (rc) {
    case SNMP_ERROR_GEN:
        txt = "SNMP generic error";
        break;
    case SNMP_ERROR_PARM:
        txt = "SNMP parameter error";
        break;
    case SNMP_ERROR_STACK_STATE:
        txt = "SNMP stack state error";
        break;
    case SNMP_ERROR_SMON_STAT_TABLE_FULL:
        txt = "SNMP statistics table full";
        break;
#ifdef SNMP_SUPPORT_V3
    case SNMPV3_ERROR_USERS_TABLE_FULL:
        txt = "SNMP user table full";
        break;
    case SNMPV3_ERROR_GROUPS_TABLE_FULL:
        txt = "SNMP group table full";
        break;
#endif /* SNMP_SUPPORT_V3 */
#if 0
    case SNMP_ERROR_RMON_HISTORY_TABLE_FULL:
        txt = "SNMP history control table full";
        break;
    case SNMP_ERROR_RMON_ALARM_TABLE_FULL:
        txt = "SNMP alarm table full";
        break;
    case SNMP_ERROR_RMON_EVENT_TABLE_FULL:
        txt = "SNMP event table full";
        break;
    case SNMP_ERROR_RMON_STAT_ENTRY_NOT_FOUND:
        txt = "SNMP statistics entry not found";
        break;
    case SNMP_ERROR_RMON_HISTORY_ENTRY_NOT_FOUND:
        txt = "SNMP history entry not found";
        break;
    case SNMP_ERROR_RMON_ALARM_ENTRY_NOT_FOUND:
        txt = "SNMP alarm entry not found";
        break;
    case SNMP_ERROR_RMON_EVENT_ENTRY_NOT_FOUND:
        txt = "SNMP event entry not found";
        break;
#endif
    default:
        txt = "SNMP unknown error";
        break;
    }
    return txt;
}

/* Get SNMP configuration */
vtss_rc snmp_mgmt_snmp_conf_get(snmp_conf_t *conf)
{
#ifdef SNMP_SUPPORT_V3
#ifdef VTSS_SW_OPTION_IPV6
    char ipv6_buff[40];
#endif /* VTSS_SW_OPTION_IPV6 */
#endif /* SNMP_SUPPORT_V3 */

    T_D("enter");
    SNMP_CRIT_ENTER();
#ifdef SNMP_SUPPORT_V3
#ifdef VTSS_SW_OPTION_IPV6
    if (snmp_global.snmp_conf.mode &&
        snmp_global.snmp_conf.trap_mode &&
        snmp_global.snmp_conf.trap_version == SNMP_SUPPORT_V3 &&
        ((strcmp(snmp_global.snmp_conf.trap_dip_string, "")) || strcmp((char *) snmp_global.snmp_conf.trap_dipv6.addr, "")) &&
        snmp_global.snmp_conf.trap_probe_security_engineid) {
        if (strcmp(snmp_global.snmp_conf.trap_dip_string, "")) {
            if (SnmpdGetRemoteEngineId(snmp_global.snmp_conf.trap_dip_string, snmp_global.snmp_conf.trap_security_engineid, (unsigned long *)&snmp_global.snmp_conf.trap_security_engineid_len)) {
                T_D("Calling SnmpdGetRemoteEngineId() failed.\n");
            }
        } else if (strcmp((char *) snmp_global.snmp_conf.trap_dipv6.addr, "")) {
            if (SnmpdGetRemoteEngineId(misc_ipv6_txt(&snmp_global.snmp_conf.trap_dipv6, ipv6_buff), snmp_global.snmp_conf.trap_security_engineid, (unsigned long *)&snmp_global.snmp_conf.trap_security_engineid_len)) {
                T_D("Calling SnmpdGetRemoteEngineId() failed.\n");
            }
        }
        if (!snmp_global.snmp_conf.trap_security_engineid_len) {
            /* wait probe trap security engine ID completed */
            SNMP_CRIT_EXIT();
            VTSS_OS_MSLEEP(500);
            SNMP_CRIT_ENTER();
            if (strcmp(snmp_global.snmp_conf.trap_dip_string, "")) {
                if (SnmpdGetRemoteEngineId(snmp_global.snmp_conf.trap_dip_string, snmp_global.snmp_conf.trap_security_engineid, (unsigned long *)&snmp_global.snmp_conf.trap_security_engineid_len)) {
                    T_D("Calling SnmpdGetRemoteEngineId() failed.\n");
                }
            } else if (strcmp((char *) snmp_global.snmp_conf.trap_dipv6.addr, "")) {
                if (SnmpdGetRemoteEngineId(misc_ipv6_txt(&snmp_global.snmp_conf.trap_dipv6, ipv6_buff), snmp_global.snmp_conf.trap_security_engineid, (unsigned long *)&snmp_global.snmp_conf.trap_security_engineid_len)) {
                    T_D("Calling SnmpdGetRemoteEngineId() failed.\n");
                }
            }
        }
    }
#else
    if (snmp_global.snmp_conf.mode &&
        snmp_global.snmp_conf.trap_mode &&
        snmp_global.snmp_conf.trap_version == SNMP_SUPPORT_V3 &&
        strcmp(snmp_global.snmp_conf.trap_dip_string, "") &&
        snmp_global.snmp_conf.trap_probe_security_engineid) {
        if (SnmpdGetRemoteEngineId(snmp_global.snmp_conf.trap_dip_string, snmp_global.snmp_conf.trap_security_engineid, (unsigned long *)&snmp_global.snmp_conf.trap_security_engineid_len)) {
            T_D("Calling SnmpdGetRemoteEngineId() failed.\n");
        }
        if (!snmp_global.snmp_conf.trap_security_engineid_len) {
            /* wait probe trap security engine ID completed */
            SNMP_CRIT_EXIT();
            VTSS_OS_MSLEEP(500);
            SNMP_CRIT_ENTER();
            if (SnmpdGetRemoteEngineId(snmp_global.snmp_conf.trap_dip_string, snmp_global.snmp_conf.trap_security_engineid, (unsigned long *)&snmp_global.snmp_conf.trap_security_engineid_len)) {
                T_D("Calling SnmpdGetRemoteEngineId() failed.\n");
            }
        }
    }
#endif /* VTSS_SW_OPTION_IPV6 */
#endif /* SNMP_SUPPORT_V3 */

    *conf = snmp_global.snmp_conf;

#ifdef SNMP_SUPPORT_V3
#ifdef VTSS_SW_OPTION_IPV6
    if (conf->trap_probe_security_engineid &&
        (!conf->mode || !conf->trap_mode || ((!strcmp(conf->trap_dip_string, "")) && (!strcmp((char *) snmp_global.snmp_conf.trap_dipv6.addr, ""))))) {
        memset(conf->trap_security_engineid, 0x0, sizeof(conf->trap_security_engineid));
        conf->trap_security_engineid_len = 0;
    }
#else
    if (conf->trap_probe_security_engineid &&
        (!conf->mode || !conf->trap_mode || (!strcmp(conf->trap_dip_string, "")))) {
        memset(conf->trap_security_engineid, 0x0, sizeof(conf->trap_security_engineid));
        conf->trap_security_engineid_len = 0;
    }
#endif /* VTSS_SW_OPTION_IPV6 */
#endif /* SNMP_SUPPORT_V3 */

    SNMP_CRIT_EXIT();
    T_D("exit");

    return VTSS_OK;
}

/* Set SNMP configuration */
vtss_rc snmp_mgmt_snmp_conf_set(snmp_conf_t *conf)
{
    vtss_rc         rc      = VTSS_OK;
    int             changed = 0;

#ifdef VTSS_SW_OPTION_IPV6
    char            ipv6_str_buff[40];

    T_D("enter, mode: %d, version: %d, read_community: %s, write_community: %s, trap_mode: %d, trap_version: %d, trap_community: %s, trap_dip: %s, trap_dipv6: %s, trap_authen_fail: %d, trap_linkup_linkdown: %d, trap_inform_mode: %d, trap_inform_timeout: %d, trap_inform_retries: %d",
        conf->mode,
        conf->version,
        conf->read_community,
        conf->write_community,
        conf->trap_mode,
        conf->trap_version,
        conf->trap_community,
        conf->trap_dip_string,
        misc_ipv6_txt(&conf->trap_dipv6 , ipv6_str_buff),
        conf->trap_authen_fail,
        conf->trap_linkup_linkdown,
        conf->trap_inform_mode,
        conf->trap_inform_timeout,
        conf->trap_inform_retries);
#else
    T_D("enter, mode: %d, version: %d, read_community: %s, write_community: %s, trap_mode: %d, trap_version: %d, trap_community: %s, trap_dip: %s, trap_authen_fail: %d, trap_linkup_linkdown: %d, trap_inform_mode: %d, trap_inform_timeout: %d, trap_inform_retries: %d",
        conf->mode,
        conf->version,
        conf->read_community,
        conf->write_community,
        conf->trap_mode,
        conf->trap_version,
        conf->trap_community,
        conf->trap_dip_string,
        conf->trap_authen_fail,
        conf->trap_linkup_linkdown,
        conf->trap_inform_mode,
        conf->trap_inform_timeout,
        conf->trap_inform_retries);
#endif /* VTSS_SW_OPTION_IPV6 */

    /* check illegal parameter */
    if (conf->mode != SNMP_MGMT_ENABLED && conf->mode != SNMP_MGMT_DISABLED) {
        return ((vtss_rc) SNMP_ERROR_PARM);
    }
    if (conf->version != SNMP_SUPPORT_V1
        && conf->version != SNMP_SUPPORT_V2C
#ifdef SNMP_SUPPORT_V3
        && conf->version != SNMP_SUPPORT_V3
#endif /* SNMP_SUPPORT_V3 */
       ) {
        return ((vtss_rc) SNMP_ERROR_PARM);
    }
    if (strlen(conf->read_community) > SNMP_MGMT_MAX_COMMUNITY_LEN) {
        return ((vtss_rc) SNMP_ERROR_PARM);
    }
    if (strlen(conf->write_community) > SNMP_MGMT_MAX_COMMUNITY_LEN) {
        return ((vtss_rc) SNMP_ERROR_PARM);
    }
    if (conf->trap_mode != SNMP_MGMT_ENABLED && conf->trap_mode != SNMP_MGMT_DISABLED) {
        return ((vtss_rc) SNMP_ERROR_PARM);
    }
    if (conf->trap_version != SNMP_SUPPORT_V1
        && conf->trap_version != SNMP_SUPPORT_V2C
#ifdef SNMP_SUPPORT_V3
        && conf->trap_version != SNMP_SUPPORT_V3
#endif /* SNMP_SUPPORT_V3 */
       ) {
        return ((vtss_rc) SNMP_ERROR_PARM);
    }
    if (strlen(conf->trap_community) > SNMP_MGMT_MAX_COMMUNITY_LEN) {
        return ((vtss_rc) SNMP_ERROR_PARM);
    }
    if (!inet_addr(conf->trap_dip_string)) {
        strcpy(conf->trap_dip_string, "");
    }
    if (conf->trap_authen_fail != SNMP_MGMT_ENABLED && conf->trap_authen_fail != SNMP_MGMT_DISABLED) {
        return ((vtss_rc) SNMP_ERROR_PARM);
    }
    if (conf->trap_linkup_linkdown != SNMP_MGMT_ENABLED && conf->trap_linkup_linkdown != SNMP_MGMT_DISABLED) {
        return ((vtss_rc) SNMP_ERROR_PARM);
    }
    if (conf->trap_inform_mode != SNMP_MGMT_ENABLED && conf->trap_inform_mode != SNMP_MGMT_DISABLED) {
        return ((vtss_rc) SNMP_ERROR_PARM);
    }
    if (conf->trap_inform_timeout > SNMP_MGMT_MAX_TRAP_INFORM_TIMEOUT) {
        return ((vtss_rc) SNMP_ERROR_PARM);
    }
    if (conf->trap_inform_retries > SNMP_MGMT_MAX_TRAP_INFORM_RETRIES) {
        return ((vtss_rc) SNMP_ERROR_PARM);
    }

#ifdef SNMP_SUPPORT_V3
    {
        snmp_conf_t snmp_conf;
        snmpv3_users_conf_t pre_user_conf, user_conf;

        if (!snmpv3_is_valid_engineid(conf->engineid, conf->engineid_len)) {
            return ((vtss_rc) SNMP_ERROR_PARM);
        }

        if (msg_switch_is_master()) {
            /* Change Engine ID will clear all local users */
            if ((rc = snmp_mgmt_snmp_conf_get(&snmp_conf)) != VTSS_OK) {
                return rc;
            }
            if (snmp_conf.engineid_len != conf->engineid_len ||
                memcmp(snmp_conf.engineid, conf->engineid, snmp_conf.engineid_len)) {
                /* Compare if the engineID is changed.
                 * Come here, when (1) The length of engineID are different or
                 * (2) The length are equal but the contents are different
                 */
                strcpy(user_conf.user_name, SNMPV3_CONF_ACESS_GETFIRST);
                strcpy(pre_user_conf.user_name, user_conf.user_name );
                while (snmpv3_mgmt_users_conf_get(&user_conf, TRUE) == VTSS_OK) {
                    if (snmp_conf.engineid_len == user_conf.engineid_len &&
                        (!memcmp(snmp_conf.engineid, user_conf.engineid, snmp_conf.engineid_len))) {
                        /* Compare if the changed engineID is used by user entry.
                         * Come here, when (1) The length of engineID are eqaul and
                         * (2) The contents are the same
                         */

                        user_conf.valid = 0;
                        rc = snmpv3_mgmt_users_conf_set(&user_conf);
                        strcpy(user_conf.user_name, pre_user_conf.user_name );
                    } else {
                        strcpy(pre_user_conf.user_name, user_conf.user_name );
                    }
                }
            }
        }
    }
#endif /*SNMP_SUPPORT_V3 */

    SNMP_CRIT_ENTER();
    if (msg_switch_is_master()) {
        changed = snmp_conf_changed(&snmp_global.snmp_conf, conf);
        snmp_global.snmp_conf = *conf;
    } else {
        T_W("not master");
        rc = (vtss_rc) SNMP_ERROR_STACK_STATE;
    }
    SNMP_CRIT_EXIT();

    if (changed) {
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
        /* Save changed configuration */
        conf_blk_id_t   blk_id  = CONF_BLK_SNMP_CONF;
        snmp_conf_blk_t *snmp_conf_blk_p;
        if ((snmp_conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, NULL)) == NULL) {
            T_W("failed to open SNMP table");
        } else {
            snmp_conf_blk_p->snmp_conf = *conf;
            conf_sec_close(CONF_SEC_GLOBAL, blk_id);
        }

#endif
        /* Activate changed configuration */
        snmp_stack_snmp_conf_set(VTSS_ISID_GLOBAL);
    }

    T_D("exit");

    return rc;
}

/* Determine if port and ISID are valid */
static BOOL snmp_mgmt_port_sid_invalid(vtss_isid_t isid, vtss_port_no_t port_no, BOOL is_set)
{
    if (port_no >= VTSS_PORT_NO_END) {
        T_W("illegal port_no: %u", port_no);
        return TRUE;
    }

    /* Check ISID */
    if (isid >= VTSS_ISID_END) {
        T_W("illegal isid: %d", isid);
        return TRUE;
    }

    if (is_set && isid == VTSS_ISID_LOCAL) {
        T_W("SET not allowed, isid: %d", isid);
        return TRUE;
    }

    return FALSE;
}

/* Get SNMP port configuration */
vtss_rc snmp_mgmt_snmp_port_conf_get(vtss_isid_t isid,
                                     vtss_port_no_t port_no,
                                     snmp_port_conf_t *conf)
{
    T_D("enter, isid: %d, port_no: %u", isid, port_no);

    /* check illegal parameter */
    if (snmp_mgmt_port_sid_invalid(isid, port_no, FALSE)) {
        return ((vtss_rc) SNMP_ERROR_PARM);
    }

    SNMP_CRIT_ENTER();
    *conf = snmp_global.snmp_port_conf[isid][port_no - VTSS_PORT_NO_START];
    SNMP_CRIT_EXIT();
    T_D("exit");

    return VTSS_OK;
}

/* Set SNMP port configuration */
vtss_rc snmp_mgmt_snmp_port_conf_set(vtss_isid_t isid,
                                     vtss_port_no_t port_no,
                                     snmp_port_conf_t *conf)
{
    vtss_rc              rc = VTSS_OK;
    int                  i, changed = 0;
    snmp_port_conf_t     *port_conf;

    T_D("enter, isid: %d, port_no: %u, linkupdown_trap_enable: %d",
        isid,
        port_no,
        conf->linkupdown_trap_enable);

    /* check illegal parameter */
    if (snmp_mgmt_port_sid_invalid(isid, port_no, TRUE)) {
        return ((vtss_rc) SNMP_ERROR_PARM);
    }
    if (conf->linkupdown_trap_enable != SNMP_MGMT_ENABLED && conf->linkupdown_trap_enable != SNMP_MGMT_DISABLED) {
        return ((vtss_rc) SNMP_ERROR_PARM);
    }

    i = (port_no - VTSS_PORT_NO_START);
    SNMP_CRIT_ENTER();
    if (msg_switch_is_master()) {
        if (msg_switch_configurable(isid)) {
            port_conf = &snmp_global.snmp_port_conf[isid][i];
            changed = snmp_port_conf_changed(port_conf, conf);
            *port_conf = *conf;
        } else {
            T_W("isid %d not active", isid);
            rc = (vtss_rc) SNMP_ERROR_STACK_STATE;
        }
    } else {
        T_W("not master");
        rc = (vtss_rc) SNMP_ERROR_STACK_STATE;
    }
    SNMP_CRIT_EXIT();

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (changed) {
        /* Save changed configuration */
        conf_blk_id_t        blk_id;
        snmp_port_conf_blk_t *port_blk;
        blk_id = CONF_BLK_SNMP_PORT_CONF;
        if ((port_blk = conf_sec_open(CONF_SEC_GLOBAL, blk_id, NULL)) == NULL) {
            T_W("failed to open SNMP port table");
        } else {
            port_blk->snmp_port_conf[(isid - VTSS_ISID_START)*VTSS_PORTS + i] = *conf;
            conf_sec_close(CONF_SEC_GLOBAL, blk_id);
        }
    }
#else
    (void) changed;  // Quiet lint
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
    T_D("exit, isid: %d, port_no: %u", isid, port_no);
    return rc;
}

/* Get SNMP SMON statistics row entry */
vtss_rc snmp_mgmt_smon_stat_entry_get(snmp_rmon_stat_entry_t *entry, BOOL next)
{
    ulong i, num, found = 0;

    T_D("enter");
    SNMP_CRIT_ENTER();
    for (i = 0, num = 0;
         i < SNMP_SMON_STAT_MAX_ROW_SIZE && num < snmp_global.snmp_smon_stat_entry_num;
         i++) {
        if (!snmp_global.snmp_smon_stat_entry[i].valid) {
            continue;
        }
        num++;
        if (entry->ctrl_index == 0 && next) {
            *entry = snmp_global.snmp_smon_stat_entry[i];
            found = 1;
            break;
        }
        if (snmp_global.snmp_smon_stat_entry[i].ctrl_index == entry->ctrl_index) {
            if (next) {
                if (num == snmp_global.snmp_smon_stat_entry_num) {
                    break;
                }
                i++;
                while (i < SNMP_SMON_STAT_MAX_ROW_SIZE ) {
                    if (snmp_global.snmp_smon_stat_entry[i].valid) {
                        *entry = snmp_global.snmp_smon_stat_entry[i];
                        found = 1;
                        break;
                    }
                    i++;
                }
                break;
            } else {
                *entry = snmp_global.snmp_smon_stat_entry[i];
                found = 1;
            }
            break;
        }
    }
    SNMP_CRIT_EXIT();
    T_D("exit");

    if (found) {
        return VTSS_OK;
    } else {
        return VTSS_RC_ERROR;
    }
}
vtss_rc snmp_mgmt_port_copy_entry_get(snmp_port_copy_entry_t *entry, BOOL next)
{
    ulong i, num, found = 0;


    SNMP_CRIT_ENTER();
    for (i = 0, num = 0;
         i < SNMP_SMON_STAT_MAX_ROW_SIZE && num < snmp_global.snmp_port_copy_entry_num;
         i++) {
        if (!snmp_global.snmp_port_copy_entry[i].valid) {
            continue;
        }
        num++;
        if (entry->ctrl_index == 0 && next) {
            *entry = snmp_global.snmp_port_copy_entry[i];
            found = 1;
            break;
        }
        if (snmp_global.snmp_port_copy_entry[i].ctrl_index == entry->ctrl_index) {
            if (next) {
                if (num == snmp_global.snmp_port_copy_entry_num) {
                    break;
                }
                i++;
                while (i < SNMP_SMON_STAT_MAX_ROW_SIZE ) {
                    if (snmp_global.snmp_port_copy_entry[i].valid) {
                        *entry = snmp_global.snmp_port_copy_entry[i];
                        found = 1;
                        break;
                    }
                    i++;
                }
                break;
            } else {
                *entry = snmp_global.snmp_port_copy_entry[i];
                found = 1;
            }
            break;
        }
    }
    SNMP_CRIT_EXIT();
    T_D("exit");

    if (found) {
        return VTSS_OK;
    } else {
        return VTSS_RC_ERROR;
    }
}
vtss_rc snmp_mgmt_smon_stat_entry_set(snmp_rmon_stat_entry_t *entry)
{
    vtss_rc              rc = VTSS_OK;
    int                  changed = 0, found_flag = 0;
    ulong                i, num;

    T_D("enter");
    SNMP_CRIT_ENTER();
    if (msg_switch_is_master()) {
        for (i = 0, num = 0;
             i < SNMP_SMON_STAT_MAX_ROW_SIZE && num < snmp_global.snmp_smon_stat_entry_num;
             i++) {
            if (!snmp_global.snmp_smon_stat_entry[i].valid) {
                continue;
            }
            num++;
            if (snmp_global.snmp_smon_stat_entry[i].ctrl_index == entry->ctrl_index) {
                found_flag = 1;
                break;
            }
        }

        if (i < SNMP_SMON_STAT_MAX_ROW_SIZE && found_flag) {
            changed = snmp_smon_stat_entry_changed(&snmp_global.snmp_smon_stat_entry[i], entry);
            if (!entry->valid) {
                snmp_global.snmp_smon_stat_entry_num--;
            }
            snmp_global.snmp_smon_stat_entry[i] = *entry;
        } else if (entry->valid) {
            /* add new entry */
            for (i = 0; i < SNMP_SMON_STAT_MAX_ROW_SIZE; i++) {
                if (snmp_global.snmp_smon_stat_entry[i].valid) {
                    continue;
                }
                snmp_global.snmp_smon_stat_entry_num++;
                snmp_global.snmp_smon_stat_entry[i] = *entry;
                break;
            }
            if (i < SNMP_SMON_STAT_MAX_ROW_SIZE) {
                changed = 1;
            } else {
                rc = (vtss_rc) SNMP_ERROR_SMON_STAT_TABLE_FULL;
            }
        }
    } else {
        T_W("not master");
        rc = (vtss_rc) SNMP_ERROR_STACK_STATE;
    }
    SNMP_CRIT_EXIT();

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (changed) {
        /* Save changed configuration */
        conf_blk_id_t        blk_id = CONF_BLK_SNMP_SMON_STAT_TABLE;
        smon_stat_conf_blk_t *smon_stat_conf_blk_p;
        if ((smon_stat_conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, NULL)) == NULL) {
            T_W("failed to open SNMP RMON statistics table");
        } else {
            SNMP_CRIT_ENTER();
            smon_stat_conf_blk_p->snmp_smon_stat_entry_num = snmp_global.snmp_smon_stat_entry_num;
            memcpy(smon_stat_conf_blk_p->snmp_smon_stat_entry, snmp_global.snmp_smon_stat_entry, sizeof(snmp_global.snmp_smon_stat_entry));
            SNMP_CRIT_EXIT();
            conf_sec_close(CONF_SEC_GLOBAL, blk_id);
        }
    }
#else
    (void) changed;  // Quiet lint
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */

    T_D("exit");

    return rc;
}
vtss_rc snmp_mgmt_port_copy_entry_set(snmp_port_copy_entry_t *entry)
{
    vtss_rc              rc = VTSS_OK;
    int                  changed = 0, found_flag = 0;
    ulong                i = 0, num;

    SNMP_CRIT_ENTER();
    if (msg_switch_is_master()) {
        for (i = 0, num = 0;
             i < SNMP_SMON_STAT_MAX_ROW_SIZE && num < snmp_global.snmp_port_copy_entry_num;
             i++) {
            if (!snmp_global.snmp_port_copy_entry[i].valid) {
                continue;
            }
            num++;
            if (snmp_global.snmp_port_copy_entry[i].ctrl_index == entry->ctrl_index) {
                found_flag = 1;
                break;
            }
        }

        if (i < SNMP_SMON_STAT_MAX_ROW_SIZE && found_flag) {
            changed = snmp_port_copy_entry_changed(&snmp_global.snmp_port_copy_entry[i], entry);
            if (!entry->valid) {
                snmp_global.snmp_port_copy_entry_num--;
            }
            snmp_global.snmp_port_copy_entry[i] = *entry;
        } else if (entry->valid) {
            /* add new entry */
            for (i = 0; i < SNMP_SMON_STAT_MAX_ROW_SIZE; i++) {
                if (snmp_global.snmp_port_copy_entry[i].valid) {
                    continue;
                }
                snmp_global.snmp_port_copy_entry_num++;
                snmp_global.snmp_port_copy_entry[i] = *entry;
                break;
            }
            if (i < SNMP_SMON_STAT_MAX_ROW_SIZE) {
                changed = 1;
            } else {
                rc = (vtss_rc) SNMP_ERROR_SMON_STAT_TABLE_FULL;
            }
        }
    } else {
        T_W("not master");
        rc = (vtss_rc) SNMP_ERROR_STACK_STATE;
    }
    SNMP_CRIT_EXIT();

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (changed) {
        /* Save changed configuration */
        conf_blk_id_t        blk_id = CONF_BLK_SNMP_PORT_COPY_TABLE;
        port_copy_conf_blk_t *port_copy_conf_blk_p;
        if ((port_copy_conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, NULL)) == NULL) {
            T_W("failed to open SNMP RMON statistics table");
        } else {
            SNMP_CRIT_ENTER();
            port_copy_conf_blk_p->snmp_port_copy_entry_num = snmp_global.snmp_port_copy_entry_num;
            memcpy(port_copy_conf_blk_p->snmp_port_copy_entry, snmp_global.snmp_port_copy_entry, sizeof(snmp_global.snmp_port_copy_entry));
            SNMP_CRIT_EXIT();
            conf_sec_close(CONF_SEC_GLOBAL, blk_id);
        }
    }
#else
    (void) changed;  // Quiet lint
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */

    T_D("exit");

    return rc;
}

#ifdef SNMP_SUPPORT_V3
/* check is SNMP admin string format */
BOOL snmpv3_is_admin_string(const char *str)
{
    uint idx;

    /* check illegal parameter,
       admin string length is restricted to 1 - 32,
       admin string is restricted to the UTF-8 octet string (33 - 126) */
    if (strlen(str) < SNMPV3_MIN_NAME_LEN || strlen(str) > SNMPV3_MAX_NAME_LEN) {
        return FALSE;
    }
    for (idx = 0; idx < strlen(str); idx++) {
        if (str[idx] < 33 || str[idx] > 126) {
            return FALSE;
        }
    }

    return TRUE;
}

/* check is valid engine ID */
BOOL snmpv3_is_valid_engineid(uchar *engineid, ulong engineid_len)
{
    uint idx, val_0_cnt = 0, val_ff_cnt = 0;

    /* The format of 'Engine ID' may not be all zeros or all 'ff'H
       and is restricted to 5 - 32 octet string */

    if (engineid_len < SNMPV3_MIN_ENGINE_ID_LEN || engineid_len > SNMPV3_MAX_ENGINE_ID_LEN) {
        return FALSE;
    }

    for (idx = 0; idx < engineid_len; idx++) {
        if (engineid[idx] == 0x0) {
            val_0_cnt++;
        } else if (engineid[idx] == 0xff) {
            val_ff_cnt++;
        }
    }
    if (val_0_cnt == engineid_len || val_ff_cnt == engineid_len) {
        return FALSE;
    }

    return TRUE;
}

/* check SNMPv3 communities configuration */
vtss_rc snmpv3_mgmt_communities_conf_check(snmpv3_communities_conf_t *conf)
{
    T_D("enter");

    /* check illegal parameter */
    if (!snmpv3_is_admin_string(conf->community)) {
        return ((vtss_rc) SNMP_ERROR_PARM);
    }
    if (!snmp_is_ip_valid(FALSE, conf->sip)) {
        return ((vtss_rc) SNMP_ERROR_PARM);
    }
    if (conf->sip_mask == 0xFFFFFFFF) {
        return ((vtss_rc) SNMP_ERROR_PARM);
    } else if (!snmp_is_ip_valid(TRUE, conf->sip_mask)) {
        return ((vtss_rc) SNMP_ERROR_PARM);
    }
    if (conf->storage_type < SNMP_MGMT_STORAGE_OTHER ||
        conf->storage_type > SNMP_MGMT_STORAGE_READONLY) {
        return ((vtss_rc) SNMP_ERROR_PARM);
    }
    if (conf->status > SNMP_MGMT_ROW_DESTROY) {
        return ((vtss_rc) SNMP_ERROR_PARM);
    }

    T_D("exit");

    return VTSS_OK;
}

/* Get SNMPv3 communities configuration,
fill community = SNMPV3_CONF_ACESS_GETFIRST will get first entry */
vtss_rc snmpv3_mgmt_communities_conf_get(snmpv3_communities_conf_t *conf, BOOL next)
{
    uint i, num, found = 0;

    T_D("enter");
    SNMP_CRIT_ENTER();
    for (i = 0, num = 0;
         i < SNMPV3_MAX_COMMUNITIES && num < snmp_global.communities_conf_num;
         i++) {
        if (!snmp_global.communities_conf[i].valid) {
            continue;
        }
        num++;
        if (!strcmp(conf->community, SNMPV3_CONF_ACESS_GETFIRST) && next) {
            *conf = snmp_global.communities_conf[i];
            found = 1;
            break;
        }
        if (!strcmp(conf->community, snmp_global.communities_conf[i].community)) {
            if (next) {
                if (num == snmp_global.communities_conf_num) {
                    break;
                }
                i++;
                while (i < SNMPV3_MAX_COMMUNITIES) {
                    if (snmp_global.communities_conf[i].valid) {
                        *conf = snmp_global.communities_conf[i];
                        found = 1;
                        break;
                    }
                    i++;
                }
                break;
            } else {
                *conf = snmp_global.communities_conf[i];
                found = 1;
            }
            break;
        }
    }
    SNMP_CRIT_EXIT();
    T_D("exit");

    if (found) {
        return VTSS_OK;
    } else {
        return VTSS_RC_ERROR;
    }
}

/* Set SNMPv3 communities configuration,
fill valid = 0 or status = SNMP_MGMT_ROW_DESTROY will destroy the entry */
vtss_rc snmpv3_mgmt_communities_conf_set(snmpv3_communities_conf_t *conf)
{
    vtss_rc                 rc = VTSS_OK;
    int                     changed = 0, found_flag = 0;
    uint                    i, num;

    T_D("enter");

    if (conf->valid && conf->status == SNMP_MGMT_ROW_ACTIVE) {
        if ((rc = snmpv3_mgmt_communities_conf_check(conf)) != VTSS_OK) {
            T_D("exit");
            return ((vtss_rc) SNMP_ERROR_PARM);
        }
    }

    SNMP_CRIT_ENTER();
    if (msg_switch_is_master()) {
        for (i = 0, num = 0;
             i < SNMPV3_MAX_COMMUNITIES && num < snmp_global.communities_conf_num;
             i++) {
            if (!snmp_global.communities_conf[i].valid) {
                continue;
            }
            num++;
            if (!strcmp(conf->community, snmp_global.communities_conf[i].community)) {
                found_flag = 1;
                break;
            }
        }

        if (found_flag && i < SNMPV3_MAX_COMMUNITIES) {
            if (!conf->valid || conf->status == SNMP_MGMT_ROW_DESTROY) {
                if (snmpv3_engine_communities_set(conf, FALSE) == VTSS_OK) {
                    snmp_global.communities_conf_num--;
                    conf->valid = 0;
                }
            }
            conf->idx = snmp_global.communities_conf[i].idx;
            changed = snmpv3_communities_conf_changed(&snmp_global.communities_conf[i], conf);
            if (conf->valid && changed) {
                if (snmpv3_engine_communities_set(conf, FALSE) != VTSS_OK) {
                    T_D("Calling snmpv3_engine_communities_set() failed.\n");
                }
                if (snmpv3_engine_communities_set(conf, TRUE) != VTSS_OK) {
                    T_D("Calling snmpv3_engine_communities_set() failed.\n");
                }
            }
            snmp_global.communities_conf[i] = *conf;
        } else if (conf->valid) {
            /* add new entry */
            for (i = 0; i < SNMPV3_MAX_COMMUNITIES; i++) {
                if (snmp_global.communities_conf[i].valid) {
                    continue;
                }
                conf->idx = i + 1;
                snmp_global.communities_conf_num++;
                snmp_global.communities_conf[i] = *conf;
                if (snmpv3_engine_communities_set(conf, TRUE) != VTSS_OK) {
                    T_D("Calling snmpv3_engine_communities_set() failed.\n");
                }
                break;
            }
            if (i < SNMPV3_MAX_COMMUNITIES) {
                changed = 1;
            } else {
                rc = (vtss_rc) SNMPV3_ERROR_COMMUNITIES_TABLE_FULL;
            }
        }
    } else {
        T_W("not master");
        rc = (vtss_rc) SNMP_ERROR_STACK_STATE;
    }
    SNMP_CRIT_EXIT();

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (changed) {
        /* Save changed configuration */
        conf_blk_id_t           blk_id = CONF_BLK_SNMPV3_COMMUNITIES_CONF;
        snmpv3_communities_conf_blk_t *communities_conf_blk_p;
        if ((communities_conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, NULL)) == NULL) {
            T_W("failed to open SNMP community table");
        } else {
            SNMP_CRIT_ENTER();
            communities_conf_blk_p->communities_conf_num = snmp_global.communities_conf_num;
            memcpy(communities_conf_blk_p->communities_conf, snmp_global.communities_conf, sizeof(snmp_global.communities_conf));
            SNMP_CRIT_EXIT();
            conf_sec_close(CONF_SEC_GLOBAL, blk_id);
        }
    }
#else
    (void) changed;  // Quiet lint
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */

    T_D("exit");

    return rc;
}

/* Delete SNMPv3 communities entry */
vtss_rc snmpv3_mgmt_communities_conf_del(ulong idx)
{
    vtss_rc                 rc = VTSS_OK;
    int                     changed = 0, found_flag = 0;
    uint                    i, num;

    T_D("enter");

    SNMP_CRIT_ENTER();
    if (msg_switch_is_master()) {
        for (i = 0, num = 0;
             i < SNMPV3_MAX_COMMUNITIES && num < snmp_global.communities_conf_num;
             i++) {
            if (!snmp_global.communities_conf[i].valid) {
                continue;
            }
            num++;
            if (idx == snmp_global.communities_conf[i].idx) {
                found_flag = 1;
                break;
            }
        }

        if (found_flag && i < SNMPV3_MAX_COMMUNITIES) {
            snmp_global.communities_conf[i].valid = 0;
            if (snmpv3_engine_communities_set(&snmp_global.communities_conf[i], FALSE) == VTSS_OK) {
                snmp_global.communities_conf_num--;
                changed = 1;
            }
        }
    } else {
        T_W("not master");
        rc = (vtss_rc) SNMP_ERROR_STACK_STATE;
    }
    SNMP_CRIT_EXIT();

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (changed) {
        /* Save changed configuration */
        conf_blk_id_t           blk_id = CONF_BLK_SNMPV3_COMMUNITIES_CONF;
        snmpv3_communities_conf_blk_t *communities_conf_blk_p;
        if ((communities_conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, NULL)) == NULL) {
            T_W("failed to open SNMP RMON event table");
        } else {
            SNMP_CRIT_ENTER();
            communities_conf_blk_p->communities_conf_num = snmp_global.communities_conf_num;
            memcpy(communities_conf_blk_p->communities_conf, snmp_global.communities_conf, sizeof(snmp_global.communities_conf));
            SNMP_CRIT_EXIT();
            conf_sec_close(CONF_SEC_GLOBAL, blk_id);
        }
    }
#else
    (void) changed;  // Quiet lint
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */

    T_D("exit");

    return rc;
}

/* check SNMPv3 users configuration */
vtss_rc snmpv3_mgmt_users_conf_check(snmpv3_users_conf_t *conf)
{
    T_D("enter");

    /* check illegal parameter */
    if (!snmpv3_is_valid_engineid(conf->engineid, conf->engineid_len)) {
        return ((vtss_rc) SNMP_ERROR_PARM);
    }
    if (!snmpv3_is_admin_string(conf->user_name)) {
        return ((vtss_rc) SNMP_ERROR_PARM);
    }
    if (!strcmp(conf->user_name, SNMPV3_NONAME)) {
        return ((vtss_rc) SNMP_ERROR_PARM);
    }
    if (conf->security_level < SNMP_MGMT_SEC_LEVEL_NOAUTH ||
        conf->security_level > SNMP_MGMT_SEC_LEVEL_AUTHPRIV) {
        return ((vtss_rc) SNMP_ERROR_PARM);
    }
    if (conf->auth_protocol > SNMP_MGMT_AUTH_PROTO_SHA) {
        return ((vtss_rc) SNMP_ERROR_PARM);
    }
    if (conf->priv_protocol > SNMP_MGMT_PRIV_PROTO_AES) {
        return ((vtss_rc) SNMP_ERROR_PARM);
    }
    if (conf->security_level != SNMP_MGMT_SEC_LEVEL_NOAUTH) {
        if (conf->auth_protocol == SNMP_MGMT_AUTH_PROTO_MD5) {
            if (strlen(conf->auth_password) < SNMPV3_MIN_PASSWORD_LEN ||
                strlen(conf->auth_password) > SNMPV3_MAX_MD5_PASSWORD_LEN) {
                return ((vtss_rc) SNMP_ERROR_PARM);
            }
        } else { //SHA
            if (strlen(conf->auth_password) < SNMPV3_MIN_PASSWORD_LEN ||
                strlen(conf->auth_password) > SNMPV3_MAX_SHA_PASSWORD_LEN) {
                return ((vtss_rc) SNMP_ERROR_PARM);
            }
        }
        if (conf->security_level == SNMP_MGMT_SEC_LEVEL_AUTHPRIV) {
            if (strlen(conf->priv_password) < SNMPV3_MIN_PASSWORD_LEN ||
                strlen(conf->priv_password) > SNMPV3_MAX_DES_PASSWORD_LEN) {
                return ((vtss_rc) SNMP_ERROR_PARM);
            }
        }
    }
    if (conf->storage_type < SNMP_MGMT_STORAGE_OTHER ||
        conf->storage_type > SNMP_MGMT_STORAGE_READONLY) {
        return ((vtss_rc) SNMP_ERROR_PARM);
    }
    if (conf->status > SNMP_MGMT_ROW_DESTROY) {
        return ((vtss_rc) SNMP_ERROR_PARM);
    }

    T_D("exit");

    return VTSS_OK;
}

/* Get SNMPv3 users configuration,
fill user_name = SNMPV3_CONF_ACESS_GETFIRST will get first entry */
vtss_rc snmpv3_mgmt_users_conf_get(snmpv3_users_conf_t *conf, BOOL next)
{
    uint i, num, found = 0;

    T_D("enter");
    SNMP_CRIT_ENTER();
    for (i = 0, num = 0;
         i < SNMPV3_MAX_USERS && num < snmp_global.users_conf_num;
         i++) {
        if (!snmp_global.users_conf[i].valid) {
            continue;
        }
        num++;
        if (!strcmp(conf->user_name, SNMPV3_CONF_ACESS_GETFIRST) && next) {
            *conf = snmp_global.users_conf[i];
            found = 1;
            break;
        }
        if (conf->engineid_len == snmp_global.users_conf[i].engineid_len &&
            (!memcmp(conf->engineid, snmp_global.users_conf[i].engineid, conf->engineid_len > snmp_global.users_conf[i].engineid_len ? conf->engineid_len : snmp_global.users_conf[i].engineid_len)) &&
            (!strcmp(conf->user_name, snmp_global.users_conf[i].user_name))) {
            if (next) {
                if (num == snmp_global.users_conf_num) {
                    break;
                }
                i++;
                while (i < SNMPV3_MAX_USERS) {
                    if (snmp_global.users_conf[i].valid) {
                        *conf = snmp_global.users_conf[i];
                        found = 1;
                        break;
                    }
                    i++;
                }
                break;
            } else {
                *conf = snmp_global.users_conf[i];
                found = 1;
            }
            break;
        }
    }
    SNMP_CRIT_EXIT();
    T_D("exit");

    if (found) {
        return VTSS_OK;
    } else {
        return VTSS_RC_ERROR;
    }
}

static int snmpv3_mgmt_users_cmp (snmpv3_users_conf_t *data, snmpv3_users_conf_t *key)
{
    int cmp;

    cmp = strlen(data->user_name) - strlen(key->user_name);

    if (cmp != 0) {
        return cmp;
    }

    cmp = strcmp(data->user_name, key->user_name);
    if (cmp != 0) {
        return cmp;
    }

    cmp = data->engineid_len - key->engineid_len;

    if (cmp != 0) {
        return cmp;
    }

    return memcmp(data->engineid, key->engineid, key->engineid_len);
}

/**
  * \brief Get next user table by Key.
  *
  */

vtss_rc snmpv3_mgmt_users_conf_get_next(snmpv3_users_conf_t *conf)
{
    snmpv3_users_conf_t tmp, buf;
    BOOL found = FALSE;
    memset(tmp.user_name, 0x7f, SNMPV3_MAX_NAME_LEN);
    tmp.user_name[SNMPV3_MAX_NAME_LEN] = 0;
    tmp.engineid_len = SNMPV3_MAX_ENGINE_ID_LEN;
    memset(tmp.engineid, 0x7f, SNMPV3_MAX_ENGINE_ID_LEN);

    strcpy(buf.user_name, SNMPV3_CONF_ACESS_GETFIRST);

    while ( VTSS_RC_OK == snmpv3_mgmt_users_conf_get (&buf, TRUE)) {
        if (snmpv3_mgmt_users_cmp ( &buf, conf) <= 0) {
            continue;
        }

        if ( snmpv3_mgmt_users_cmp ( &tmp, &buf ) > 0 ) {
            memcpy(&tmp, &buf, sizeof(buf));
            found = TRUE;
        }
    }

    if ( TRUE != found ) {
        return VTSS_RC_ERROR;
    }

    memcpy(conf, &tmp, sizeof(tmp));
    return VTSS_RC_OK;
}





/* Set SNMPv3 users configuration,
fill valid = 0 or status = SNMP_MGMT_ROW_DESTROY will destroy the entry */
vtss_rc snmpv3_mgmt_users_conf_set(snmpv3_users_conf_t *conf)
{
    vtss_rc                 rc = VTSS_OK;
    int                     changed = 0, found_flag = 0;
    uint                    i, num;

    T_D("enter");

    if (conf->valid && conf->status == SNMP_MGMT_ROW_ACTIVE) {
        if ((rc = snmpv3_mgmt_users_conf_check(conf)) != VTSS_OK) {
            T_D("exit");
            return ((vtss_rc) SNMP_ERROR_PARM);
        }
    }

    SNMP_CRIT_ENTER();
    if (msg_switch_is_master()) {
        for (i = 0, num = 0;
             i < SNMPV3_MAX_USERS && num < snmp_global.users_conf_num;
             i++) {
            if (!snmp_global.users_conf[i].valid) {
                continue;
            }
            num++;
            if (conf->engineid_len == snmp_global.users_conf[i].engineid_len &&
                (!memcmp(conf->engineid, snmp_global.users_conf[i].engineid, conf->engineid_len > snmp_global.users_conf[i].engineid_len ? conf->engineid_len : snmp_global.users_conf[i].engineid_len)) &&
                (!strcmp(conf->user_name, snmp_global.users_conf[i].user_name))) {
                found_flag = 1;
                break;
            }
        }

        if (found_flag && i < SNMPV3_MAX_USERS) {
            if (!conf->valid || conf->status == SNMP_MGMT_ROW_DESTROY) {
                if (snmpv3_engine_users_set(conf, FALSE) == VTSS_OK) {
                    snmp_global.users_conf_num--;
                }
            } else {
                /* check illegal operation inconsistent with row status */
                if ((conf->status == SNMP_MGMT_ROW_ACTIVE) &&
                    (snmp_global.users_conf[i].security_level != conf->security_level ||
                     snmp_global.users_conf[i].auth_protocol != conf->auth_protocol ||
                     snmp_global.users_conf[i].priv_protocol != conf->priv_protocol)) {
                    SNMP_CRIT_EXIT();
                    T_D("exit");
                    return ((vtss_rc) SNMP_ERROR_ROW_STATUS_INCONSISTENT);
                }

                /* check change */
                if (conf->security_level != SNMP_MGMT_SEC_LEVEL_NOAUTH) {
                    if (strcmp(snmp_global.users_conf[i].auth_password, conf->auth_password)) {
                        if (snmpv3_engine_users_change_password(TRUE,
                                                                snmp_global.users_conf[i].engineid,
                                                                snmp_global.users_conf[i].engineid_len,
                                                                (uchar *)snmp_global.users_conf[i].user_name,
                                                                (uchar *)conf->auth_password) != VTSS_OK) {
                            T_W("SNMP engine occur fail - snmpv3_engine_users_change_password(TRUE)");
                            SNMP_CRIT_EXIT();
                            T_D("exit");
                            return ((vtss_rc) SNMP_ERROR_ENGINE_FAIL);
                        }
                    }
                    if (conf->security_level == SNMP_MGMT_SEC_LEVEL_AUTHPRIV) {
                        if (strcmp(snmp_global.users_conf[i].priv_password, conf->priv_password)) {
                            if (snmpv3_engine_users_change_password(FALSE,
                                                                    snmp_global.users_conf[i].engineid,
                                                                    snmp_global.users_conf[i].engineid_len,
                                                                    (uchar *)snmp_global.users_conf[i].user_name,
                                                                    (uchar *)conf->priv_password) != VTSS_OK) {
                                T_W("SNMP engine occur fail - snmpv3_engine_users_change_password(FALSE)");
                                SNMP_CRIT_EXIT();
                                T_D("exit");
                                return ((vtss_rc) SNMP_ERROR_ENGINE_FAIL);
                            }
                        }
                    }
                }
            }
            conf->idx = snmp_global.users_conf[i].idx;
            changed = snmpv3_users_conf_changed(&snmp_global.users_conf[i], conf);
            snmp_global.users_conf[i] = *conf;
        } else if (conf->valid) {
            /* add new entry */
            for (i = 0; i < SNMPV3_MAX_USERS; i++) {
                if (snmp_global.users_conf[i].valid) {
                    continue;
                }
                conf->idx = i + 1;
                if (conf->security_level != SNMP_MGMT_SEC_LEVEL_AUTHPRIV) {
                    conf->priv_protocol = SNMP_MGMT_PRIV_PROTO_NONE;
                    strcpy(conf->priv_password, "");
                    if (conf->security_level == SNMP_MGMT_SEC_LEVEL_NOAUTH) {
                        conf->auth_protocol = SNMP_MGMT_AUTH_PROTO_NONE;
                        strcpy(conf->auth_password, "");
                    }
                }
                snmp_global.users_conf_num++;
                snmp_global.users_conf[i] = *conf;
                if (snmpv3_engine_users_set(conf, TRUE) != VTSS_OK) {
                    T_D("Calling snmpv3_engine_users_set() failed.\n");
                }
                break;
            }
            if (i < SNMPV3_MAX_USERS) {
                changed = 1;
            } else {
                rc = (vtss_rc) SNMPV3_ERROR_USERS_TABLE_FULL;
            }
        }
    } else {
        T_W("not master");
        rc = (vtss_rc) SNMP_ERROR_STACK_STATE;
    }
    SNMP_CRIT_EXIT();

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (changed) {
        /* Save changed configuration */
        conf_blk_id_t           blk_id = CONF_BLK_SNMPV3_USERS_CONF;
        snmpv3_users_conf_blk_t *users_conf_blk_p;
        if ((users_conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, NULL)) == NULL) {
            T_W("failed to open SNMP user table");
        } else {
            SNMP_CRIT_ENTER();
            users_conf_blk_p->users_conf_num = snmp_global.users_conf_num;
            memcpy(users_conf_blk_p->users_conf, snmp_global.users_conf, sizeof(snmp_global.users_conf));
            SNMP_CRIT_EXIT();
            conf_sec_close(CONF_SEC_GLOBAL, blk_id);
        }
    }
#else
    (void) changed;  // Quiet lint
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
    T_D("exit");

    return rc;
}

vtss_rc snmpv3_mgmt_users_conf_save(void)
{
    conf_blk_id_t           blk_id = CONF_BLK_SNMPV3_USERS_CONF;
    snmpv3_users_conf_blk_t *users_conf_blk_p;
    if ((users_conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, NULL)) == NULL) {
        T_W("failed to open SNMP user table");
        return VTSS_RC_ERROR;
    } else {
        T_D("number of user entries is %lu", snmp_global.users_conf_num);
        SNMP_CRIT_ENTER();
        users_conf_blk_p->users_conf_num = snmp_global.users_conf_num;
        memcpy(users_conf_blk_p->users_conf, snmp_global.users_conf, sizeof(snmp_global.users_conf));
        SNMP_CRIT_EXIT();
        conf_sec_close(CONF_SEC_GLOBAL, blk_id);
    }
    return VTSS_RC_OK;
}

/* Delete SNMPv3 users entry */
vtss_rc snmpv3_mgmt_users_conf_del(ulong idx)
{
    vtss_rc                 rc = VTSS_OK;
    int                     changed = 0, found_flag = 0;
    uint                    i, num;

    T_D("enter");

    SNMP_CRIT_ENTER();
    if (msg_switch_is_master()) {
        for (i = 0, num = 0;
             i < SNMPV3_MAX_USERS && num < snmp_global.users_conf_num;
             i++) {
            if (!snmp_global.users_conf[i].valid) {
                continue;
            }
            num++;
            if (idx == snmp_global.users_conf[i].idx) {
                found_flag = 1;
                break;
            }
        }

        if (found_flag && i < SNMPV3_MAX_USERS) {
            snmp_global.users_conf[i].valid = 0;
            if (snmpv3_engine_users_set(&snmp_global.users_conf[i], FALSE) == VTSS_OK) {
                snmp_global.users_conf_num--;
                changed = 1;
            }
        }
    } else {
        T_W("not master");
        rc = (vtss_rc) SNMP_ERROR_STACK_STATE;
    }
    SNMP_CRIT_EXIT();

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (changed) {
        /* Save changed configuration */
        conf_blk_id_t           blk_id = CONF_BLK_SNMPV3_USERS_CONF;
        snmpv3_users_conf_blk_t *users_conf_blk_p;
        if ((users_conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, NULL)) == NULL) {
            T_W("failed to open SNMP RMON event table");
        } else {
            SNMP_CRIT_ENTER();
            users_conf_blk_p->users_conf_num = snmp_global.users_conf_num;
            memcpy(users_conf_blk_p->users_conf, snmp_global.users_conf, sizeof(snmp_global.users_conf));
            SNMP_CRIT_EXIT();
            conf_sec_close(CONF_SEC_GLOBAL, blk_id);
        }
    }
#else
    (void) changed;  // Quiet lint
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */

    T_D("exit");

    return rc;
}

/* check SNMPv3 groups configuration */
vtss_rc snmpv3_mgmt_groups_conf_check(snmpv3_groups_conf_t *conf)
{
    T_D("enter");

    /* check illegal parameter */
    if (conf->security_model < SNMP_MGMT_SEC_MODEL_SNMPV1 ||
        conf->security_model > SNMP_MGMT_SEC_MODEL_USM) {
        return ((vtss_rc) SNMP_ERROR_PARM);
    }
    if (!snmpv3_is_admin_string(conf->security_name)) {
        return ((vtss_rc) SNMP_ERROR_PARM);
    }
    if (!snmpv3_is_admin_string(conf->group_name)) {
        return ((vtss_rc) SNMP_ERROR_PARM);
    }
    if (conf->storage_type < SNMP_MGMT_STORAGE_OTHER ||
        conf->storage_type > SNMP_MGMT_STORAGE_READONLY) {
        return ((vtss_rc) SNMP_ERROR_PARM);
    }
    if (conf->status > SNMP_MGMT_ROW_DESTROY) {
        return ((vtss_rc) SNMP_ERROR_PARM);
    }

    T_D("exit");

    return VTSS_OK;
}

/* Get SNMPv3 groups configuration,
fill security_name = SNMPV3_CONF_ACESS_GETFIRST will get first entry */
vtss_rc snmpv3_mgmt_groups_conf_get(snmpv3_groups_conf_t *conf, BOOL next)
{
    uint i, num, found = 0;

    T_D("enter");
    SNMP_CRIT_ENTER();
    for (i = 0, num = 0;
         i < SNMPV3_MAX_GROUPS && num < snmp_global.groups_conf_num;
         i++) {
        if (!snmp_global.groups_conf[i].valid) {
            continue;
        }
        num++;
        if (!strcmp(conf->security_name, SNMPV3_CONF_ACESS_GETFIRST) && next) {
            *conf = snmp_global.groups_conf[i];
            found = 1;
            break;
        }
        if (conf->security_model == snmp_global.groups_conf[i].security_model &&
            (!strcmp(conf->security_name, snmp_global.groups_conf[i].security_name))) {
            if (next) {
                if (num == snmp_global.groups_conf_num) {
                    break;
                }
                i++;
                while (i < SNMPV3_MAX_GROUPS) {
                    if (snmp_global.groups_conf[i].valid) {
                        *conf = snmp_global.groups_conf[i];
                        found = 1;
                        break;
                    }
                    i++;
                }
                break;
            } else {
                *conf = snmp_global.groups_conf[i];
                found = 1;
            }
            break;
        }
    }
    SNMP_CRIT_EXIT();
    T_D("exit");

    if (found) {
        return VTSS_OK;
    } else {
        return VTSS_RC_ERROR;
    }
}

static int snmpv3_mgmt_groups_cmp (snmpv3_groups_conf_t *data, snmpv3_groups_conf_t *key)
{
    int cmp;

    if ((cmp = data->security_model - key->security_model) != 0) {
        return cmp;
    }

    if ((cmp = strlen(data->security_name) - strlen(key->security_name)) != 0) {
        return cmp;
    }

    return strcmp(data->security_name, key->security_name);
}

/**
  * \brief Get next group table by Key.
  *
  */

vtss_rc snmpv3_mgmt_groups_conf_get_next(snmpv3_groups_conf_t *conf)
{
    snmpv3_groups_conf_t tmp, buf;
    BOOL found = FALSE;
    tmp.security_model = 0x7fffffff;
    memset(tmp.security_name, 0x7f, SNMPV3_MAX_NAME_LEN);
    tmp.security_name[SNMPV3_MAX_NAME_LEN] = 0;

    strcpy(buf.security_name, SNMPV3_CONF_ACESS_GETFIRST);

    while ( VTSS_RC_OK == snmpv3_mgmt_groups_conf_get (&buf, TRUE)) {
        if (snmpv3_mgmt_groups_cmp ( &buf, conf) <= 0) {
            continue;
        }

        if ( snmpv3_mgmt_groups_cmp ( &tmp, &buf ) > 0 ) {
            memcpy(&tmp, &buf, sizeof(buf));
            found = TRUE;
        }
    }

    if ( TRUE != found ) {
        return VTSS_RC_ERROR;
    }

    memcpy(conf, &tmp, sizeof(tmp));
    return VTSS_RC_OK;
}

/* Set SNMPv3 groups configuration,
fill valid = 0 or status = SNMP_MGMT_ROW_DESTROY will destroy the entry */
vtss_rc snmpv3_mgmt_groups_conf_set(snmpv3_groups_conf_t *conf)
{
    vtss_rc                 rc = VTSS_OK;
    int                     changed = 0, found_flag = 0;
    uint                    i, num;

    T_D("enter");

    if (conf->valid && conf->status == SNMP_MGMT_ROW_ACTIVE) {
        if ((rc = snmpv3_mgmt_groups_conf_check(conf)) != VTSS_OK) {
            T_D("exit");
            return ((vtss_rc) SNMP_ERROR_PARM);
        }
    }

    SNMP_CRIT_ENTER();
    if (msg_switch_is_master()) {
        for (i = 0, num = 0;
             i < SNMPV3_MAX_GROUPS && num < snmp_global.groups_conf_num;
             i++) {
            if (!snmp_global.groups_conf[i].valid) {
                continue;
            }
            num++;
            if (conf->security_model == snmp_global.groups_conf[i].security_model &&
                (!strcmp(conf->security_name, snmp_global.groups_conf[i].security_name))) {
                found_flag = 1;
                break;
            }
        }

        if (found_flag && i < SNMPV3_MAX_GROUPS) {
            if (!conf->valid || conf->status == SNMP_MGMT_ROW_DESTROY) {
                if (snmpv3_engine_groups_set(conf, FALSE) == VTSS_OK) {
                    snmp_global.groups_conf_num--;
                    conf->valid = 0;
                }
            }
            conf->idx = snmp_global.groups_conf[i].idx;
            changed = snmpv3_groups_conf_changed(&snmp_global.groups_conf[i], conf);
            if (conf->valid && changed) {
                if (snmpv3_engine_groups_set(conf, FALSE) != VTSS_OK) {
                    T_D("Calling snmpv3_engine_groups_set() failed.\n");
                }
                if (snmpv3_engine_groups_set(conf, TRUE) != VTSS_OK) {
                    T_D("Calling snmpv3_engine_groups_set() failed.\n");
                }
            }
            snmp_global.groups_conf[i] = *conf;
        } else if (conf->valid) {
            /* add new entry */
            for (i = 0; i < SNMPV3_MAX_GROUPS; i++) {
                if (snmp_global.groups_conf[i].valid) {
                    continue;
                }
                conf->idx = i + 1;
                snmp_global.groups_conf_num++;
                snmp_global.groups_conf[i] = *conf;
                if (snmpv3_engine_groups_set(conf, TRUE) != VTSS_OK) {
                    T_D("Calling snmpv3_engine_groups_set() failed.\n");
                }
                break;
            }
            if (i < SNMPV3_MAX_GROUPS) {
                changed = 1;
            } else {
                rc = (vtss_rc) SNMPV3_ERROR_GROUPS_TABLE_FULL;
            }
        }
    } else {
        T_W("not master");
        rc = (vtss_rc) SNMP_ERROR_STACK_STATE;
    }
    SNMP_CRIT_EXIT();

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (changed) {
        /* Save changed configuration */
        conf_blk_id_t           blk_id = CONF_BLK_SNMPV3_GROUPS_CONF;
        snmpv3_groups_conf_blk_t *groups_conf_blk_p;
        if ((groups_conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, NULL)) == NULL) {
            T_W("failed to open SNMP RMON event table");
        } else {
            SNMP_CRIT_ENTER();
            groups_conf_blk_p->groups_conf_num = snmp_global.groups_conf_num;
            memcpy(groups_conf_blk_p->groups_conf, snmp_global.groups_conf, sizeof(snmp_global.groups_conf));
            SNMP_CRIT_EXIT();
            conf_sec_close(CONF_SEC_GLOBAL, blk_id);
        }
    }
#else
    (void) changed;  // Quiet lint
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */

    T_D("exit");

    return rc;
}

/* Delete SNMPv3 groups entry */
vtss_rc snmpv3_mgmt_groups_conf_del(ulong idx)
{
    vtss_rc                 rc = VTSS_OK;
    int                     changed = 0, found_flag = 0;
    uint                    i, num;

    T_D("enter");

    SNMP_CRIT_ENTER();
    if (msg_switch_is_master()) {
        for (i = 0, num = 0;
             i < SNMPV3_MAX_GROUPS && num < snmp_global.groups_conf_num;
             i++) {
            if (!snmp_global.groups_conf[i].valid) {
                continue;
            }
            num++;
            if (idx == snmp_global.groups_conf[i].idx) {
                found_flag = 1;
                break;
            }
        }

        if (found_flag && i < SNMPV3_MAX_GROUPS) {
            snmp_global.groups_conf[i].valid = 0;
            if (snmpv3_engine_groups_set(&snmp_global.groups_conf[i], FALSE) == VTSS_OK) {
                snmp_global.groups_conf_num--;
                changed = 1;
            }
        }
    } else {
        T_W("not master");
        rc = (vtss_rc) SNMP_ERROR_STACK_STATE;
    }
    SNMP_CRIT_EXIT();

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (changed) {
        /* Save changed configuration */
        conf_blk_id_t           blk_id = CONF_BLK_SNMPV3_GROUPS_CONF;
        snmpv3_groups_conf_blk_t *groups_conf_blk_p;
        if ((groups_conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, NULL)) == NULL) {
            T_W("failed to open SNMP RMON event table");
        } else {
            SNMP_CRIT_ENTER();
            groups_conf_blk_p->groups_conf_num = snmp_global.groups_conf_num;
            memcpy(groups_conf_blk_p->groups_conf, snmp_global.groups_conf, sizeof(snmp_global.groups_conf));
            SNMP_CRIT_EXIT();
            conf_sec_close(CONF_SEC_GLOBAL, blk_id);
        }
    }
#else
    (void) changed;  // Quiet lint
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */

    T_D("exit");

    return rc;
}

/* check SNMPv3 views configuration */
vtss_rc snmpv3_mgmt_views_conf_check(snmpv3_views_conf_t *conf)
{
    T_D("enter");

    /* check illegal parameter */
    if (!snmpv3_is_admin_string(conf->view_name)) {
        return ((vtss_rc) SNMP_ERROR_PARM);
    }
    if (!strcmp(conf->view_name, SNMPV3_NONAME)) {
        return ((vtss_rc) SNMP_ERROR_PARM);
    }
    if (!conf->subtree_len) {
        return ((vtss_rc) SNMP_ERROR_PARM);
    }
    if (conf->view_type != SNMPV3_MGMT_VIEW_INCLUDED &&
        conf->view_type != SNMPV3_MGMT_VIEW_EXCLUDED) {
        return ((vtss_rc) SNMP_ERROR_PARM);
    }
    if (conf->storage_type < SNMP_MGMT_STORAGE_OTHER ||
        conf->storage_type > SNMP_MGMT_STORAGE_READONLY) {
        return ((vtss_rc) SNMP_ERROR_PARM);
    }
    if (conf->status > SNMP_MGMT_ROW_DESTROY) {
        return ((vtss_rc) SNMP_ERROR_PARM);
    }

    T_D("exit");

    return VTSS_OK;
}

/* Get SNMPv3 views configuration,
fill view_name = SNMPV3_CONF_ACESS_GETFIRST will get first entry */
vtss_rc snmpv3_mgmt_views_conf_get(snmpv3_views_conf_t *conf, BOOL next)
{
    uint i, num, found = 0;

    T_D("enter");
    SNMP_CRIT_ENTER();
    for (i = 0, num = 0;
         i < SNMPV3_MAX_VIEWS && num < snmp_global.views_conf_num;
         i++) {
        if (!snmp_global.views_conf[i].valid) {
            continue;
        }
        num++;
        if (!strcmp(conf->view_name, SNMPV3_CONF_ACESS_GETFIRST) && next) {
            *conf = snmp_global.views_conf[i];
            found = 1;
            break;
        }
        if ((!strcmp(conf->view_name, snmp_global.views_conf[i].view_name)) &&
            (!memcmp(conf->subtree, snmp_global.views_conf[i].subtree, conf->subtree_len > snmp_global.views_conf[i].subtree_len ? sizeof(ulong) * conf->subtree_len : sizeof(ulong) * snmp_global.views_conf[i].subtree_len)) &&
            conf->subtree_len == snmp_global.views_conf[i].subtree_len) {
            if (next) {
                if (num == snmp_global.views_conf_num) {
                    break;
                }
                i++;
                while (i < SNMPV3_MAX_VIEWS) {
                    if (snmp_global.views_conf[i].valid) {
                        *conf = snmp_global.views_conf[i];
                        found = 1;
                        break;
                    }
                    i++;
                }
                break;
            } else {
                *conf = snmp_global.views_conf[i];
                found = 1;
            }
            break;
        }
    }
    SNMP_CRIT_EXIT();
    T_D("exit");

    if (found) {
        return VTSS_OK;
    } else {
        return VTSS_RC_ERROR;
    }
}

static int snmpv3_mgmt_views_cmp (snmpv3_views_conf_t *data, snmpv3_views_conf_t *key)
{
    int cmp;

    if ((cmp = strlen(data->view_name) - strlen(key->view_name)) != 0) {
        return cmp;
    }

    if ((cmp = strcmp(data->view_name, key->view_name) != 0 )) {
        return cmp;
    }

    if ((cmp = data->subtree_len - key->subtree_len) != 0) {
        return cmp;
    }

    return snmp_oid_compare((oid *)data->subtree, data->subtree_len, (oid *)key->subtree, key->subtree_len);
}

vtss_rc snmpv3_mgmt_views_conf_get_next(snmpv3_views_conf_t *conf)
{
    snmpv3_views_conf_t tmp, buf;
    BOOL found = FALSE;
    memset(tmp.view_name, 0x7f, SNMPV3_MAX_NAME_LEN);
    tmp.view_name[SNMPV3_MAX_NAME_LEN] = 0;
    tmp.subtree_len = SNMP_MGMT_MAX_OID_LEN;
    memset(tmp.subtree, 0xff, sizeof(ulong)*SNMP_MGMT_MAX_OID_LEN);

    strcpy(buf.view_name, SNMPV3_CONF_ACESS_GETFIRST);

    while ( VTSS_RC_OK == snmpv3_mgmt_views_conf_get (&buf, TRUE)) {
        if (snmpv3_mgmt_views_cmp ( &buf, conf) <= 0) {
            continue;
        }

        if ( snmpv3_mgmt_views_cmp ( &tmp, &buf ) > 0 ) {
            memcpy(&tmp, &buf, sizeof(buf));
            found = TRUE;
        }
    }

    if ( TRUE != found ) {
        return VTSS_RC_ERROR;
    }

    memcpy(conf, &tmp, sizeof(tmp));
    return VTSS_RC_OK;
}

/* Set SNMPv3 views configuration,
fill valid = 0 or status = SNMP_MGMT_ROW_DESTROY will destroy the entry */
vtss_rc snmpv3_mgmt_views_conf_set(snmpv3_views_conf_t *conf)
{
    vtss_rc                 rc = VTSS_OK;
    int                     changed = 0, found_flag = 0;
    uint                    i, num;

    T_D("enter, valid: %d, view_name: %s, subtree: %s, view_type: %d, storage_type: %d, status: %d",
        conf->valid,
        conf->view_name,
        misc_oid2str(conf->subtree, conf->subtree_len, conf->subtree_mask, conf->subtree_mask_len),
        conf->view_type,
        conf->storage_type,
        conf->status);

    if (conf->valid && conf->status == SNMP_MGMT_ROW_ACTIVE) {
        if ((rc = snmpv3_mgmt_views_conf_check(conf)) != VTSS_OK) {
            T_D("exit");
            return ((vtss_rc) SNMP_ERROR_PARM);
        }
    }

    SNMP_CRIT_ENTER();
    if (msg_switch_is_master()) {
        for (i = 0, num = 0;
             i < SNMPV3_MAX_VIEWS && num < snmp_global.views_conf_num;
             i++) {
            if (!snmp_global.views_conf[i].valid) {
                continue;
            }
            num++;
            if ((!strcmp(conf->view_name, snmp_global.views_conf[i].view_name)) &&
                (!memcmp(conf->subtree, snmp_global.views_conf[i].subtree, conf->subtree_len > snmp_global.views_conf[i].subtree_len ? sizeof(ulong) * conf->subtree_len : sizeof(ulong) * snmp_global.views_conf[i].subtree_len)) &&
                conf->subtree_len == snmp_global.views_conf[i].subtree_len) {
                found_flag = 1;
                break;
            }
        }

        if (found_flag && i < SNMPV3_MAX_VIEWS) {
            if (!conf->valid || conf->status == SNMP_MGMT_ROW_DESTROY) {
                if (snmpv3_engine_views_set(conf, FALSE) == VTSS_OK) {
                    snmp_global.views_conf_num--;
                    conf->valid = 0;
                }
            }
            conf->idx = snmp_global.views_conf[i].idx;
            changed = snmpv3_views_conf_changed(&snmp_global.views_conf[i], conf);
            if (conf->valid && changed) {
                if (snmpv3_engine_views_set(conf, FALSE) != VTSS_OK) {
                    T_D("Calling snmpv3_engine_views_set() failed.\n");
                }
                if (snmpv3_engine_views_set(conf, TRUE) != VTSS_OK) {
                    T_D("Calling snmpv3_engine_views_set() failed.\n");
                }
            }
            snmp_global.views_conf[i] = *conf;
        } else if (conf->valid) {
            /* add new entry */
            for (i = 0; i < SNMPV3_MAX_VIEWS; i++) {
                if (snmp_global.views_conf[i].valid) {
                    continue;
                }
                conf->idx = i + 1;
                snmp_global.views_conf_num++;
                snmp_global.views_conf[i] = *conf;
                if (snmpv3_engine_views_set(conf, TRUE) != VTSS_OK) {
                    T_D("Calling snmpv3_engine_views_set() failed.\n");
                }
                break;
            }
            if (i < SNMPV3_MAX_VIEWS) {
                changed = 1;
            } else {
                rc = (vtss_rc) SNMPV3_ERROR_VIEWS_TABLE_FULL;
            }
        }
    } else {
        T_W("not master");
        rc = (vtss_rc) SNMP_ERROR_STACK_STATE;
    }
    SNMP_CRIT_EXIT();

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (changed) {
        /* Save changed configuration */
        conf_blk_id_t           blk_id = CONF_BLK_SNMPV3_VIEWS_CONF;
        snmpv3_views_conf_blk_t *views_conf_blk_p;
        if ((views_conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, NULL)) == NULL) {
            T_W("failed to open SNMP views table");
        } else {
            SNMP_CRIT_ENTER();
            views_conf_blk_p->views_conf_num = snmp_global.views_conf_num;
            memcpy(views_conf_blk_p->views_conf, snmp_global.views_conf, sizeof(snmp_global.views_conf));
            SNMP_CRIT_EXIT();
            conf_sec_close(CONF_SEC_GLOBAL, blk_id);
        }
    }
#else
    (void) changed;  // Quiet lint
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */

    T_D("exit");

    return rc;
}

/* Delete SNMPv3 views entry */
vtss_rc snmpv3_mgmt_views_conf_del(ulong idx)
{
    vtss_rc                 rc = VTSS_OK;
    int                     changed = 0, found_flag = 0;
    uint                    i, num;

    T_D("enter");

    SNMP_CRIT_ENTER();
    if (msg_switch_is_master()) {
        for (i = 0, num = 0;
             i < SNMPV3_MAX_VIEWS && num < snmp_global.views_conf_num;
             i++) {
            if (!snmp_global.views_conf[i].valid) {
                continue;
            }
            num++;
            if (idx == snmp_global.views_conf[i].idx) {
                found_flag = 1;
                break;
            }
        }

        if (found_flag && i < SNMPV3_MAX_VIEWS) {
            snmp_global.views_conf[i].valid = 0;
            if (snmpv3_engine_views_set(&snmp_global.views_conf[i], FALSE) == VTSS_OK) {
                snmp_global.views_conf_num--;
                changed = 1;
            }
        }
    } else {
        T_W("not master");
        rc = (vtss_rc) SNMP_ERROR_STACK_STATE;
    }
    SNMP_CRIT_EXIT();

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (changed) {
        /* Save changed configuration */
        conf_blk_id_t           blk_id = CONF_BLK_SNMPV3_VIEWS_CONF;
        snmpv3_views_conf_blk_t *views_conf_blk_p;
        if ((views_conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, NULL)) == NULL) {
            T_W("failed to open SNMP RMON event table");
        } else {
            SNMP_CRIT_ENTER();
            views_conf_blk_p->views_conf_num = snmp_global.views_conf_num;
            memcpy(views_conf_blk_p->views_conf, snmp_global.views_conf, sizeof(snmp_global.views_conf));
            SNMP_CRIT_EXIT();
            conf_sec_close(CONF_SEC_GLOBAL, blk_id);
        }
    }
#else
    (void) changed;  // Quiet lint
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */

    T_D("exit");

    return rc;
}

/* check SNMPv3 accesses configuration */
vtss_rc snmpv3_mgmt_accesses_conf_check(snmpv3_accesses_conf_t *conf)
{
    T_D("enter");

    /* check illegal parameter */
    if (!snmpv3_is_admin_string(conf->group_name)) {
        return ((vtss_rc) SNMP_ERROR_PARM);
    }
    if (conf->security_model > SNMP_MGMT_SEC_MODEL_USM) {
        return ((vtss_rc) SNMP_ERROR_PARM);
    }
    if (conf->security_level < SNMP_MGMT_SEC_LEVEL_NOAUTH ||
        conf->security_level > SNMP_MGMT_SEC_LEVEL_AUTHPRIV) {
        return ((vtss_rc) SNMP_ERROR_PARM);
    }
    if (conf->context_match != SNMPV3_MGMT_CONTEX_MATCH_EXACT &&
        conf->context_match != SNMPV3_MGMT_CONTEX_MATCH_PREFIX) {
        return ((vtss_rc) SNMP_ERROR_PARM);
    }
    if (!snmpv3_is_admin_string(conf->read_view_name)) {
        return ((vtss_rc) SNMP_ERROR_PARM);
    }
    if (!snmpv3_is_admin_string(conf->write_view_name)) {
        return ((vtss_rc) SNMP_ERROR_PARM);
    }
    if (!snmpv3_is_admin_string(conf->notify_view_name)) {
        return ((vtss_rc) SNMP_ERROR_PARM);
    }
    if (conf->storage_type < SNMP_MGMT_STORAGE_OTHER ||
        conf->storage_type > SNMP_MGMT_STORAGE_READONLY) {
        return ((vtss_rc) SNMP_ERROR_PARM);
    }
    if (conf->status > SNMP_MGMT_ROW_DESTROY) {
        return ((vtss_rc) SNMP_ERROR_PARM);
    }

    T_D("exit");

    return VTSS_OK;
}

/* Get SNMPv3 accesses configuration,
fill group_name = SNMPV3_CONF_ACESS_GETFIRST will get first entry */
vtss_rc snmpv3_mgmt_accesses_conf_get(snmpv3_accesses_conf_t *conf, BOOL next)
{
    uint i, num, found = 0;

    T_D("enter");
    SNMP_CRIT_ENTER();
    for (i = 0, num = 0;
         i < SNMPV3_MAX_ACCESSES && num < snmp_global.accesses_conf_num;
         i++) {
        if (!snmp_global.accesses_conf[i].valid) {
            continue;
        }
        num++;
        if (!strcmp(conf->group_name, SNMPV3_CONF_ACESS_GETFIRST) && next) {
            *conf = snmp_global.accesses_conf[i];
            found = 1;
            break;
        }
        if ((!strcmp(conf->group_name, snmp_global.accesses_conf[i].group_name)) &&
            (!strcmp(conf->context_prefix, snmp_global.accesses_conf[i].context_prefix)) &&
            conf->security_model == snmp_global.accesses_conf[i].security_model &&
            conf->security_level == snmp_global.accesses_conf[i].security_level) {
            if (next) {
                if (num == snmp_global.accesses_conf_num) {
                    break;
                }
                i++;
                while (i < SNMPV3_MAX_ACCESSES) {
                    if (snmp_global.accesses_conf[i].valid) {
                        *conf = snmp_global.accesses_conf[i];
                        found = 1;
                        break;
                    }
                    i++;
                }
                break;
            } else {
                *conf = snmp_global.accesses_conf[i];
                found = 1;
            }
            break;
        }
    }
    SNMP_CRIT_EXIT();
    T_D("exit");

    if (found) {
        return VTSS_OK;
    } else {
        return VTSS_RC_ERROR;
    }
}

static int snmpv3_mgmt_accesses_cmp (snmpv3_accesses_conf_t *data, snmpv3_accesses_conf_t *key)
{
    int cmp;

    if ((cmp = strlen(data->group_name) - strlen(key->group_name)) != 0) {
        return cmp;
    }

    if ( (cmp = strcmp(data->group_name, key->group_name)) != 0 ) {
        return cmp;
    }

    if ((cmp = data->security_model - key->security_model) != 0) {
        return cmp;
    }


    return (data->security_level - key->security_level);
}


vtss_rc snmpv3_mgmt_accesses_conf_get_next(snmpv3_accesses_conf_t *conf)
{
    snmpv3_accesses_conf_t tmp, buf;
    BOOL found = FALSE;
    memset(tmp.group_name, 0x7f, SNMPV3_MAX_NAME_LEN);
    tmp.group_name[SNMPV3_MAX_NAME_LEN] = 0;
    tmp.security_model = 0x7fffffff;
    tmp.security_level = 0x7fffffff;

    strcpy(buf.group_name, SNMPV3_CONF_ACESS_GETFIRST);

    while ( VTSS_RC_OK == snmpv3_mgmt_accesses_conf_get (&buf, TRUE)) {
        if (snmpv3_mgmt_accesses_cmp ( &buf, conf) <= 0) {
            continue;
        }

        if ( snmpv3_mgmt_accesses_cmp ( &tmp, &buf ) > 0 ) {
            memcpy(&tmp, &buf, sizeof(buf));
            found = TRUE;
        }
    }

    if ( TRUE != found ) {
        return VTSS_RC_ERROR;
    }

    memcpy(conf, &tmp, sizeof(tmp));
    return VTSS_RC_OK;
}

/* Set SNMPv3 accesses configuration,
fill valid = 0 or status = SNMP_MGMT_ROW_DESTROY will destroy the entry */
vtss_rc snmpv3_mgmt_accesses_conf_set(snmpv3_accesses_conf_t *conf)
{
    vtss_rc                    rc = VTSS_OK;
    int                        changed = 0, found_flag = 0;
    uint                       i, num;

    T_D("enter");

    if (conf->valid && conf->status == SNMP_MGMT_ROW_ACTIVE) {
        if ((rc = snmpv3_mgmt_accesses_conf_check(conf)) != VTSS_OK) {
            T_D("exit");
            return ((vtss_rc) SNMP_ERROR_PARM);
        }
    }

    SNMP_CRIT_ENTER();
    if (msg_switch_is_master()) {
        for (i = 0, num = 0;
             i < SNMPV3_MAX_ACCESSES && num < snmp_global.accesses_conf_num;
             i++) {
            if (!snmp_global.accesses_conf[i].valid) {
                continue;
            }
            num++;
            if ((!strcmp(conf->group_name, snmp_global.accesses_conf[i].group_name)) &&
                (!strcmp(conf->context_prefix, snmp_global.accesses_conf[i].context_prefix)) &&
                conf->security_model == snmp_global.accesses_conf[i].security_model &&
                conf->security_level == snmp_global.accesses_conf[i].security_level) {
                found_flag = 1;
                break;
            }
        }

        if (found_flag && i < SNMPV3_MAX_ACCESSES) {
            if (!conf->valid || conf->status == SNMP_MGMT_ROW_DESTROY) {
                if (snmpv3_engine_accesses_set(conf, FALSE) == VTSS_OK) {
                    snmp_global.accesses_conf_num--;
                    conf->valid = 0;
                }
            }
            conf->idx = snmp_global.accesses_conf[i].idx;
            changed = snmpv3_accesses_conf_changed(&snmp_global.accesses_conf[i], conf);
            if (conf->valid && changed) {
                if (snmpv3_engine_accesses_set(conf, FALSE) != VTSS_OK) {
                    T_D("Calling snmpv3_engine_accesses_set() failed.\n");
                }
                if (snmpv3_engine_accesses_set(conf, TRUE) != VTSS_OK) {
                    T_D("Calling snmpv3_engine_accesses_set() failed.\n");
                }
            }
            snmp_global.accesses_conf[i] = *conf;
        } else if (conf->valid) {
            /* add new entry */
            for (i = 0; i < SNMPV3_MAX_ACCESSES; i++) {
                if (snmp_global.accesses_conf[i].valid) {
                    continue;
                }
                conf->idx = i + 1;
                snmp_global.accesses_conf_num++;
                snmp_global.accesses_conf[i] = *conf;
                if (snmpv3_engine_accesses_set(conf, TRUE) != VTSS_OK) {
                    T_D("Calling snmpv3_engine_accesses_set() failed.\n");
                }
                break;
            }
            if (i < SNMPV3_MAX_ACCESSES) {
                changed = 1;
            } else {
                rc = (vtss_rc) SNMPV3_ERROR_ACCESSES_TABLE_FULL;
            }
        }
    } else {
        T_W("not master");
        rc = (vtss_rc) SNMP_ERROR_STACK_STATE;
    }
    SNMP_CRIT_EXIT();

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (changed) {
        /* Save changed configuration */
        conf_blk_id_t              blk_id = CONF_BLK_SNMPV3_ACCESSES_CONF;
        snmpv3_accesses_conf_blk_t *accesses_conf_blk_p;
        if ((accesses_conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, NULL)) == NULL) {
            T_W("failed to open SNMP access table");
        } else {
            SNMP_CRIT_ENTER();
            accesses_conf_blk_p->accesses_conf_num = snmp_global.accesses_conf_num;
            memcpy(accesses_conf_blk_p->accesses_conf, snmp_global.accesses_conf, sizeof(snmp_global.accesses_conf));
            SNMP_CRIT_EXIT();
            conf_sec_close(CONF_SEC_GLOBAL, blk_id);
        }
    }
#else
    (void) changed;  // Quiet lint
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */

    T_D("exit");

    return rc;
}
/* Delete SNMPv3 accesses entry */
vtss_rc snmpv3_mgmt_accesses_conf_del(ulong idx)
{
    vtss_rc                 rc = VTSS_OK;
    int                     changed = 0, found_flag = 0;
    uint                    i, num;

    T_D("enter");

    SNMP_CRIT_ENTER();
    if (msg_switch_is_master()) {
        for (i = 0, num = 0;
             i < SNMPV3_MAX_ACCESSES && num < snmp_global.accesses_conf_num;
             i++) {
            if (!snmp_global.accesses_conf[i].valid) {
                continue;
            }
            num++;
            if (idx == snmp_global.accesses_conf[i].idx) {
                found_flag = 1;
                break;
            }
        }

        if (found_flag && i < SNMPV3_MAX_ACCESSES) {
            snmp_global.accesses_conf[i].valid = 0;
            if (snmpv3_engine_accesses_set(&snmp_global.accesses_conf[i], FALSE) == VTSS_OK) {
                snmp_global.accesses_conf_num--;
                changed = 1;
            }
        }
    } else {
        T_W("not master");
        rc = (vtss_rc) SNMP_ERROR_STACK_STATE;
    }
    SNMP_CRIT_EXIT();

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (changed) {
        /* Save changed configuration */
        conf_blk_id_t           blk_id = CONF_BLK_SNMPV3_ACCESSES_CONF;
        snmpv3_accesses_conf_blk_t *accesses_conf_blk_p;
        if ((accesses_conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, NULL)) == NULL) {
            T_W("failed to open SNMP access table");
        } else {
            SNMP_CRIT_ENTER();
            accesses_conf_blk_p->accesses_conf_num = snmp_global.accesses_conf_num;
            memcpy(accesses_conf_blk_p->accesses_conf, snmp_global.accesses_conf, sizeof(snmp_global.accesses_conf));
            SNMP_CRIT_EXIT();
            conf_sec_close(CONF_SEC_GLOBAL, blk_id);
        }
    }
#else
    (void) changed;  // Quiet lint
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */

    T_D("exit");

    return rc;
}
#endif /* SNMP_SUPPORT_V3 */


/****************************************************************************
 * Callbacks
 ****************************************************************************/

/* Send trap packet */
static void snmp_mgmt_send_trap(snmp_trap_type_t trap_type, snmp_trap_if_info_t trap_if_info)
{
    if (msg_switch_is_master() && (SNMP_TRAP_BUFF_CNT(trap_buff_read_idx, trap_buff_write_idx) < (SNMP_TRAP_BUFF_MAX_CNT - 1))) {
        SNMP_CRIT_ENTER();
        snmp_trap_buff[trap_buff_write_idx].trap_type = trap_type;
        snmp_trap_buff[trap_buff_write_idx].trap_if_info = trap_if_info;
        trap_buff_write_idx = (trap_buff_write_idx + 1) % SNMP_TRAP_BUFF_MAX_CNT;
        SNMP_CRIT_EXIT();
    }
}

static void snmp_mgmt_send_probe(ucd_trap_conf_t  *trap_conf)
{
    if (msg_switch_is_master() && (SNMP_TRAP_BUFF_CNT(trap_buff_read_idx, trap_buff_write_idx) < (SNMP_TRAP_BUFF_MAX_CNT - 1))) {
        snmp_trap_buff[trap_buff_write_idx].trap_type = SNMP_TRAP_TYPE_PROBE;
        snmp_trap_buff[trap_buff_write_idx].trap_conf = *trap_conf;
        trap_buff_write_idx = (trap_buff_write_idx + 1) % SNMP_TRAP_BUFF_MAX_CNT;
    }
}

/* RFC 1903             Textual Conventions for SNMPv2         January 1996


            To summarize the effect of having a conceptual row with a
            status column having a SYNTAX clause value of RowStatus,
            consider the following state diagram:


                                         STATE
              +--------------+-----------+-------------+-------------
              |      A       |     B     |      C      |      D
              |              |status col.|status column|
              |status column |    is     |      is     |status column
    ACTION    |does not exist|  notReady | notInService|  is active
--------------+--------------+-----------+-------------+-------------
set status    |noError    ->D|inconsist- |inconsistent-|inconsistent-
column to     |       or     |   entValue|        Value|        Value
createAndGo   |inconsistent- |           |             |
              |         Value|           |             |
--------------+--------------+-----------+-------------+-------------
set status    |noError  see 1|inconsist- |inconsistent-|inconsistent-
column to     |       or     |   entValue|        Value|        Value
createAndWait |wrongValue    |           |             |
--------------+--------------+-----------+-------------+-------------
set status    |inconsistent- |inconsist- |noError      |noError
column to     |         Value|   entValue|             |
active        |              |           |             |
              |              |     or    |             |
              |              |           |             |
              |              |see 2   ->D|          ->D|          ->D
--------------+--------------+-----------+-------------+-------------
set status    |inconsistent- |inconsist- |noError      |noError   ->C
column to     |         Value|   entValue|             |
notInService  |              |           |             |
              |              |     or    |             |      or
              |              |           |             |
              |              |see 3   ->C|          ->C|wrongValue
--------------+--------------+-----------+-------------+-------------
set status    |noError       |noError    |noError      |noError
column to     |              |           |             |
destroy       |           ->A|        ->A|          ->A|          ->A
--------------+--------------+-----------+-------------+-------------
set any other |see 4         |noError    |noError      |see 5
column to some|              |           |             |
value         |              |      see 1|          ->C|          ->D
--------------+--------------+-----------+-------------+-------------

            (1) goto B or C, depending on information available to the
            agent.

            (2) if other variable bindings included in the same PDU,
            provide values for all columns which are missing but
            required, then return noError and goto D.

            (3) if other variable bindings included in the same PDU,
            provide values for all columns which are missing but
            required, then return noError and goto C.

            (4) at the discretion of the agent, the return value may be
            either:

                 inconsistentName:  because the agent does not choose to
                 create such an instance when the corresponding
                 RowStatus instance does not exist, or

                 inconsistentValue:  if the supplied value is
                 inconsistent with the state of some other MIB object's
                 value, or

                 noError: because the agent chooses to create the
                 instance.

            If noError is returned, then the instance of the status
            column must also be created, and the new state is B or C,
            depending on the information available to the agent.  If
            inconsistentName or inconsistentValue is returned, the row
            remains in state A.

            (5) depending on the MIB definition for the column/table,
            either noError or inconsistentValue may be returned.

            NOTE: Other processing of the set request may result in a
            response other than noError being returned, e.g.,
            wrongValue, noCreation, etc. */

static ulong row_status[6][4] = {
    /* createAndGo */   {SNMP_MGMT_ROW_ACTIVE,                          SNMP_MGMT_ROW_NOTREADY,         (ulong) SNMP_ERROR_ROW_STATUS_INCONSISTENT,     (ulong) SNMP_ERROR_ROW_STATUS_INCONSISTENT},
    /* createAndWait */ {SNMP_MGMT_ROW_NOTREADY,                        SNMP_MGMT_ROW_NOTREADY,         (ulong) SNMP_ERROR_ROW_STATUS_INCONSISTENT,     (ulong) SNMP_ERROR_ROW_STATUS_INCONSISTENT},
    /* active */        {(ulong) SNMP_ERROR_ROW_STATUS_INCONSISTENT,    SNMP_MGMT_ROW_ACTIVE,           SNMP_MGMT_ROW_ACTIVE,                           SNMP_MGMT_ROW_ACTIVE},
    /* notInService */  {(ulong) SNMP_ERROR_ROW_STATUS_INCONSISTENT,    SNMP_MGMT_ROW_NOTINSERVICE,     SNMP_MGMT_ROW_NOTINSERVICE,                     SNMP_MGMT_ROW_NOTINSERVICE},
    /* destroy */       {SNMP_MGMT_ROW_NONEXISTENT,                     SNMP_MGMT_ROW_NONEXISTENT,      SNMP_MGMT_ROW_NONEXISTENT,                      SNMP_MGMT_ROW_NONEXISTENT},
    /* other column */  {SNMP_MGMT_ROW_NOTREADY,                        SNMP_MGMT_ROW_NOTREADY,         SNMP_MGMT_ROW_NOTINSERVICE,                     SNMP_MGMT_ROW_ACTIVE}
};

/* check row status */
vtss_rc snmp_row_status_check(ulong action, ulong old_state, ulong *new_state)
{
    ulong result, row, column;

    if (action > SNMP_MGMT_ROW_DESTROY || old_state > SNMP_MGMT_ROW_DESTROY) {
        return ((vtss_rc) SNMP_ERROR_ROW_STATUS_INCONSISTENT);
    }

    switch (action) {
    case SNMP_MGMT_ROW_CREATEANDGO:
        column = 0;
        break;
    case SNMP_MGMT_ROW_CREATEANDWAIT:
        column = 1;
        break;
    case SNMP_MGMT_ROW_ACTIVE:
        column = 2;
        break;
    case SNMP_MGMT_ROW_NOTINSERVICE:
        column = 3;
        break;
    case SNMP_MGMT_ROW_DESTROY:
        column = 4;
        break;
    default:
        column = 5;
        break;
    }

    switch (old_state) {
    case SNMP_MGMT_ROW_NONEXISTENT:
        row = 0;
        break;
    case SNMP_MGMT_ROW_NOTREADY:
        row = 1;
        break;
    case SNMP_MGMT_ROW_NOTINSERVICE:
        row = 2;
        break;
    case SNMP_MGMT_ROW_ACTIVE:
        row = 3;
        break;
    default:
        return ((vtss_rc) SNMP_ERROR_ROW_STATUS_INCONSISTENT);
    }

    result = row_status[column][row];

    if (result != (ulong) SNMP_ERROR_ROW_STATUS_INCONSISTENT) {
        *new_state = result;
        return VTSS_OK;
    } else {
        return ((vtss_rc) SNMP_ERROR_ROW_STATUS_INCONSISTENT);
    }
}

static vtss_trap_entry_t  *alloc_trap_entry(void)
{
    int i = 0;
    BOOL found = FALSE;
    vtss_trap_entry_t *tmp;

    for (i = 0, tmp = &trap_global.trap_entry[i]; i < VTSS_TRAP_CONF_MAX; i++, tmp++) {
        if ( FALSE == tmp->valid) {
            found = TRUE;
            break;
        }
    }

    if (FALSE == found ) {
        return NULL;
    }

    tmp->valid = TRUE;
    tmp->trap_conf.conf_id = i;
    return tmp;
}

static void free_trap_entry(vtss_trap_entry_t *entry)
{
    memset(entry, 0, sizeof(vtss_trap_entry_t));
}
/* if elm1 lager than elm2, return 1, else if elm1 smaller than elm2, return -1, otherwise return 0 */
static i32 trap_conf_entry_compare_func(void *elm1, void *elm2)
{
    vtss_trap_entry_t *in_list = elm1;
    vtss_trap_entry_t *new_entry = elm2;
    int cmp = strcmp(in_list->trap_conf_name, new_entry->trap_conf_name);
    int str_len = strlen(in_list->trap_conf_name) - strlen(new_entry->trap_conf_name);

    if ( cmp > 0 || (cmp == 0 && str_len > 0) ) {
        return 1;
    } else if ( cmp < 0 || (cmp == 0 && str_len < 0) ) {
        return -1;
    } else {
        return 0;
    }

}

VTSS_AVL_TREE(trap_entry_avl, "TRAP_conf_entry", VTSS_MODULE_ID_SNMP, trap_conf_entry_compare_func, VTSS_TRAP_CONF_MAX)

static vtss_trap_entry_t *get_trap_entry_point(vtss_trap_entry_t *trap_entry)
{
    vtss_trap_entry_t *tmp = trap_entry;

    if (vtss_avl_tree_get(&trap_entry_avl, (void **) &tmp, VTSS_AVL_TREE_GET) != TRUE) { // entry not existing
        return NULL;
    }
    return tmp;
}

static vtss_trap_entry_t *get_next_trap_entry_point(vtss_trap_entry_t *trap_entry)
{
    vtss_trap_entry_t *tmp = trap_entry;
    if (vtss_avl_tree_get(&trap_entry_avl, (void **) &tmp, VTSS_AVL_TREE_GET_NEXT) != TRUE) { // entry not existing
        return NULL;
    }
    return tmp;
}

vtss_rc trap_init(void)
{

    SNMP_CRIT_ENTER();
    memset( &trap_global, 0, sizeof(trap_global));
    SNMP_CRIT_EXIT();

    if (FALSE == vtss_avl_tree_init(&trap_entry_avl)) {
        T_E(" Init trap AVL fail");
    }
    return VTSS_RC_OK;
}

void trap_mgmt_sys_default_get(vtss_trap_sys_conf_t *trap_conf)
{
    trap_conf->trap_mode = FALSE;
}

/**
  * \brief Get trap mode configuration
  *
  * \param global_enable   [OUT]: the global configuration of trap configuration
  * \return
  *    Always VTSS_RC_OK\n
  */

vtss_rc trap_mgmt_mode_get(BOOL *enable)
{
    vtss_trap_sys_conf_t *trap_conf;
    SNMP_CRIT_ENTER();
    trap_conf = &trap_global;
    *enable = trap_conf->trap_mode;
    SNMP_CRIT_EXIT();
    return VTSS_RC_OK;
}

/**
  * \brief Set trap mode configuration
  *
  * \param global_enable   [IN]: the global configuration of trap configuration
  * \return
  *    Always VTSS_RC_OK\n
  */

vtss_rc trap_mgmt_mode_set(BOOL enable)
{
    vtss_trap_sys_conf_t *trap_conf;
    i32 conf_id = 0;

    SNMP_CRIT_ENTER();
    trap_conf = &trap_global;


    if (!msg_switch_is_master()) {
        T_W("not master");
        T_D("exit");
        SNMP_CRIT_EXIT();
        return VTSS_RC_ERROR;
    }

    if ( trap_conf->trap_mode == enable ) {
        SNMP_CRIT_EXIT();
        return VTSS_RC_OK;
    }

    if ( FALSE == enable) {
        for ( conf_id = VTSS_TRAP_CONF_ID_MAX; conf_id < VTSS_TRAP_CONF_MAX; conf_id++ ) {
            ucd_destroy_session (conf_id);
        }
    }

    trap_conf->trap_mode = enable;

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    {
        /* Save changed configuration */
        conf_blk_id_t               blk_id  = CONF_BLK_TRAP_CONF;
        trap_conf_blk_t             *trap_conf_blk_p;
        if ((trap_conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, NULL)) == NULL) {
            T_W("failed to open CONF_BLK_TRAP_CONF table");
        } else {
            trap_conf_blk_p->trap_sys_conf.trap_mode = enable;
            conf_sec_close(CONF_SEC_GLOBAL, blk_id);
        }
    }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */

    SNMP_CRIT_EXIT();
    return VTSS_RC_OK;
}




/**
  * \brief Get trap configuration entry
  *
  * \param trap_entry   [IN] trap_conf_name: Name of the trap configuration
  * \return
  *    VTSS_RC_OK indicates it is successful, otherwise is failed.
  */

vtss_rc trap_mgmt_conf_get (vtss_trap_entry_t  *trap_entry)
{
    vtss_trap_entry_t *tmp;
    SNMP_CRIT_ENTER();
    tmp = get_trap_entry_point(trap_entry);

    if ( !tmp ) {
        SNMP_CRIT_EXIT();
        return VTSS_RC_ERROR;
    }
    memcpy(trap_entry, tmp, sizeof(vtss_trap_entry_t));
    SNMP_CRIT_EXIT();
    return VTSS_RC_OK;
}

/**
  * \brief Get next trap configuration entry
  *
  * \param trap_entry   [INOUT] trap_conf_name: Name of the trap configuration
  * \return
  *    VTSS_RC_OK indicates it is successful, otherwise is failed.
  */

vtss_rc trap_mgmt_conf_get_next (vtss_trap_entry_t  *trap_entry)
{
    SNMP_CRIT_ENTER();
    vtss_trap_entry_t *tmp = get_next_trap_entry_point(trap_entry);
    if ( !tmp ) {
        SNMP_CRIT_EXIT();
        return VTSS_RC_ERROR;
    }
    memcpy(trap_entry, tmp, sizeof(vtss_trap_entry_t));
    SNMP_CRIT_EXIT();
    return VTSS_RC_OK;
}

/**
  * \brief Set trap configuration entry
  *
  * \param trap_entry   [IN] : The trap configuration
  * \return
  *    VTSS_RC_OK indicates it is successful, otherwise is failed.
  */

vtss_rc trap_mgmt_conf_set (vtss_trap_entry_t *trap_entry)
{
    int                         conf_id;
    BOOL                        delete = FALSE, update = FALSE;
    ucd_trap_conf_t             trap_conf;

    SNMP_CRIT_ENTER();
    vtss_trap_entry_t *tmp = get_trap_entry_point(trap_entry);

    memset(&trap_conf, 0, sizeof(trap_conf));
    if ( (tmp && !memcmp(tmp, trap_entry, sizeof(vtss_trap_entry_t))) ||
         (NULL == tmp && FALSE == trap_entry->valid)) {
        SNMP_CRIT_EXIT();
        return VTSS_RC_OK;
    } else if ( tmp ) {
        ucd_destroy_session (tmp->trap_conf.conf_id);
        if (TRUE == trap_entry->valid) {
            update = TRUE;
        }
    } else if ( NULL == (tmp = alloc_trap_entry())) {
        T_D("reach the max conf");
        SNMP_CRIT_EXIT();
        return VTSS_RC_ERROR;
    }

    if (FALSE == trap_entry->valid) {
        delete = TRUE;
    }

    conf_id = tmp->trap_conf.conf_id;

    if ( TRUE == delete ) {
        (void) vtss_avl_tree_delete (&trap_entry_avl, (void **)&tmp);
        free_trap_entry(tmp);
    } else {
        memcpy(tmp, trap_entry, sizeof(vtss_trap_entry_t));
        tmp->trap_conf.conf_id = conf_id;
        if ( FALSE == update ) {
            (void) vtss_avl_tree_add (&trap_entry_avl, tmp);
        } else {
//            T_E("update (ID:%s)", trap_entry->trap_conf_name);
        }
    }

    if ( tmp->trap_conf.trap_version == SNMP_SUPPORT_V3 && tmp->trap_conf.trap_probe_engineid) {
        ucd_trap_conf_set(tmp, &trap_conf );
        snmp_mgmt_send_probe(&trap_conf);
    }
    SNMP_CRIT_EXIT();

    VTSS_OS_MSLEEP(100);

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    {
        /* Save changed configuration */
        conf_blk_id_t               blk_id  = CONF_BLK_TRAP_CONF;
        trap_conf_blk_t             *trap_conf_blk_p;
        if ((trap_conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, NULL)) == NULL) {
            T_W("failed to open CONF_BLK_TRAP_CONF table");
        } else {
            if ( TRUE == delete ) {
                memset(&trap_conf_blk_p->trap_sys_conf.trap_entry[conf_id], 0, sizeof(vtss_trap_entry_t));
            } else {
                memcpy(&trap_conf_blk_p->trap_sys_conf.trap_entry[conf_id], tmp, sizeof(vtss_trap_entry_t));
            }
            conf_sec_close(CONF_SEC_GLOBAL, blk_id);
        }
    }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */

    return VTSS_RC_OK;
}

/**
  * \brief Get trap default configuration entry
  *
  * \param trap_entry   [OUT] : The trap configuration
  */

void trap_mgmt_conf_default_get(vtss_trap_entry_t  *trap_entry)
{

    SNMP_CRIT_ENTER();
    vtss_trap_conf_t          *conf = &trap_entry->trap_conf;
    vtss_trap_event_t         *event = &trap_entry->trap_event;

    conf->enable = TRAP_CONF_DEFAULT_ENABLE;
    strcpy(conf->trap_src_host_name, "");
    strcpy(conf->dip.addr.ipv4_str, TRAP_CONF_DEFAULT_DIP);
    conf->dip.ipv6_flag = FALSE;
    conf->trap_port = TRAP_CONF_DEFAULT_DPORT;
    conf->trap_version = TRAP_CONF_DEFAULT_VER;
    strcpy(conf->trap_community, TRAP_CONF_DEFAULT_COMM);
    conf->trap_inform_mode = TRAP_CONF_DEFAULT_INFORM_MODE;
    conf->trap_inform_timeout = TRAP_CONF_DEFAULT_INFORM_TIMEOUT ;
    conf->trap_inform_retries = TRAP_CONF_DEFAULT_INFORM_RETRIES;
#ifdef SNMP_SUPPORT_V3
    conf->trap_probe_engineid = TRAP_CONF_DEFAULT_PROBE_ENG;
    conf->trap_engineid_len = 0;
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
    SNMP_CRIT_EXIT();
}

struct variable_list *
snmp_bind_var(struct variable_list *prev,
              void *value, int type, size_t sz_val, oid *oidVar, size_t sz_oid)
{
    struct variable_list *var;

    var = (struct variable_list *)VTSS_MALLOC(sizeof(struct variable_list));
    if (!var) {
        T_E("FATAL: cannot VTSS_MALLOC in snmp_bind_var");
        exit(-1);               /* Sorry :( */
    }
    memset(var, 0, sizeof(struct variable_list));
    var->next_variable = prev;
    (void)snmp_set_var_objid(var, oidVar, sz_oid);
    (void)snmp_set_var_value(var, (u_char *) value, sz_val);
    var->type = type;

    return var;
}

/**
  * \brief Send SNMP vars trap
  *
  * \param specific      [IN]: 0 indicates that specific trap, 1 ~ 5 indicates generic trap, -1 indicates v2 version.
  * \param entry         [IN]: the event OID and variable binding
  *
 */

void snmp_send_vars_trap(int specific, snmp_vars_trap_entry_t *entry)
{

    SNMP_CRIT_ENTER();
    if (msg_switch_is_master() && (SNMP_TRAP_BUFF_CNT(trap_buff_read_idx, trap_buff_write_idx) < (SNMP_TRAP_BUFF_MAX_CNT - 1))) {
        snmp_trap_buff[trap_buff_write_idx].trap_type = SNMP_TRAP_TYPE_VARS;
        snmp_trap_buff[trap_buff_write_idx].trap_specific = specific;
        snmp_trap_buff[trap_buff_write_idx].trap_entry = *entry;
        trap_buff_write_idx = (trap_buff_write_idx + 1) % SNMP_TRAP_BUFF_MAX_CNT;
    }

    SNMP_CRIT_EXIT();
}

/**
 * Get OID length (number of items) of private MIB OID
 */
u32 snmp_private_mib_oid_len_get(void)
{
    return OID_LENGTH(snmp_private_mib_oid);
}

/**
 * \brief Send private SNMP trap
 */
void snmp_private_mib_trap_send(vtss_isid_t isid, vtss_port_no_t iport, oid trap_number)
{
    snmp_vars_trap_entry_t *trap_entry;
    snmp_trap_buff_t       *trap_buf;

    SNMP_CRIT_ENTER();

    if (msg_switch_is_master() && (SNMP_TRAP_BUFF_CNT(trap_buff_read_idx, trap_buff_write_idx) < (SNMP_TRAP_BUFF_MAX_CNT - 1))) {
        trap_buf   = &snmp_trap_buff[trap_buff_write_idx];
        trap_entry = &trap_buf->trap_entry;

        memset(trap_entry, 0, sizeof(*trap_entry));

        // First the OID identifying the private MIB
        memcpy(trap_entry->oid, snmp_private_mib_oid, sizeof(snmp_private_mib_oid));
        trap_entry->oid_len = snmp_private_mib_oid_len_get();

        // Then the trap branch
        trap_entry->oid[trap_entry->oid_len++] = SNMP_PRIVATE_MIB_SWITCH_NOTIFICATIONS;
        trap_entry->oid[trap_entry->oid_len++] = SNMP_PRIVATE_MIB_SWITCH_TRAPS;

        // Finally this particular trap
        trap_entry->oid[trap_entry->oid_len++] = trap_number;

        trap_buf->trap_type = SNMP_TRAP_TYPE_SPECIFIC_OID;

        if (iport != VTSS_PORT_NO_NONE) {
            iftable_info_t  table_info;

            table_info.type  = IFTABLE_IFINDEX_TYPE_PORT;
            table_info.isid  = isid;
            table_info.if_id = iport;

            if (!ifIndex_get_by_interface(&table_info)) {
                T_E("Unable to convert <isid, iport> = <%u, %u> to ifIdx. Sending trap w/o port info", isid, iport);
            } else {
                trap_buf->trap_if_info.if_idx = table_info.ifIndex;
                trap_buf->trap_type = SNMP_TRAP_TYPE_SPECIFIC_OID_WITH_IF_IDX;
            }
        }

        // Housekeeping
        trap_buf->trap_specific = SNMP_TRAP_ENTERPRISESPECIFIC;
        trap_buff_write_idx = (trap_buff_write_idx + 1) % SNMP_TRAP_BUFF_MAX_CNT;
    }

    SNMP_CRIT_EXIT();
}

/*
 * Port state change indication
 */
#ifdef RFC3636_SUPPORTED_MAU
#if RFC3636_SUPPORTED_MAU
extern u_long          linkdown_counter[VTSS_ISID_CNT][VTSS_PORT_COUNT];
#endif      /*RFC3636_SUPPORTED_MAU*/
#endif // end ifdef
static void snmp_port_state_change_callback(vtss_isid_t isid, vtss_port_no_t port_no, port_info_t *info)
{
    snmp_port_conf_t    snmp_port_conf;
    port_conf_t         port_conf;
    port_status_t       port_status;
    snmp_trap_if_info_t trap_if_info;
    iftable_info_t table_info;

    if (msg_switch_is_master() && !info->stack) {
        T_D("port_no: [%d,%u] link %s", isid, port_no, info->link ? "up" : "down");
        if (!msg_switch_exists(isid)) { /* IP interface maybe change, don't send trap */
            return;
        }
        if (snmp_mgmt_snmp_port_conf_get(isid, port_no, &snmp_port_conf) != VTSS_OK) {
            return;
        }
        if (port_mgmt_conf_get(isid, port_no, &port_conf) != VTSS_OK) {
            return;
        }
        if (port_mgmt_status_get(isid, port_no, &port_status) != VTSS_OK) {
            return;
        }
        table_info.if_id = port_no;
        table_info.type = IFTABLE_IFINDEX_TYPE_PORT;
        table_info.isid = isid;
        (void) ifIndex_get_by_interface(&table_info);
        trap_if_info.if_idx = table_info.ifIndex;
        trap_if_info.if_admin = port_conf.enable ? 1 : 2;
        trap_if_info.if_oper = port_status.status.link ? 1 : 2;
        snmp_mgmt_send_trap(info->link ? SNMP_TRAP_TYPE_LINKUP : SNMP_TRAP_TYPE_LINKDOWN, trap_if_info);
#ifdef RFC3636_SUPPORTED_MAU
#if RFC3636_SUPPORTED_MAU
        if (! info->link) {
            SNMP_CRIT_ENTER();
            linkdown_counter[isid - VTSS_ISID_START][port_no]++;
            SNMP_CRIT_EXIT();
        }
#endif      /*RFC3636_SUPPORTED_MAU*/
#endif // end ifdef
    }
}


/****************************************************************************
 * Module thread
 ****************************************************************************/

extern void send_enterprise_trap_vars(int, int, oid *, int, struct variable_list *);

static void snmp_trap_thread(cyg_addrword_t data)
{
#ifdef RFC2819_SUPPORTED_EVENT
    char sys_trap_community[256];
    int  compare_result = 0;
#endif // RFC2819_SUPPORTED_EVENT

    /* The stackable device booting will take 3~5 seconds. */
    VTSS_OS_MSLEEP(5000);

    /* If STP mode is enabled, the port state became forwarding state
       will take about 30 seconds. (2 forwarding delay time)
       In order to prevent trap packet lossing during system booting state.
       We make a simple way to do it.
       Always waiting system booting passed 30 seconds then the SNMP module
       starting sendout trap packets. */
    VTSS_OS_MSLEEP(30000);

#ifdef SNMP_SUPPORT_V3
    /* Update the SNMPv3 engine time again, the reason is same as above */
    snmpv3_mgmt_ntp_post_conf();
#endif /* SNMP_SUPPORT_V3 */

    while (1) {
        if (msg_switch_is_master()) {
            while (msg_switch_is_master()) {
                if (SnmpdGetAgentState()) {
                    while (trap_buff_read_idx != trap_buff_write_idx) {
                        SNMP_CRIT_ENTER();
                        switch (snmp_trap_buff[trap_buff_read_idx].trap_type) {
                        case SNMP_TRAP_TYPE_COLDSTART:
                        case SNMP_TRAP_TYPE_WARMSTART:
                        case SNMP_TRAP_TYPE_LINKDOWN:
                        case SNMP_TRAP_TYPE_LINKUP:
                        case SNMP_TRAP_TYPE_AUTHFAIL:
                        case SNMP_TRAP_TYPE_SPECIFIC_OID:
                        case SNMP_TRAP_TYPE_SPECIFIC_OID_WITH_IF_IDX: {

                            snmp_trap_buff_t   trap_buf = snmp_trap_buff[trap_buff_read_idx];
                            SNMP_CRIT_EXIT();
                            (void)trap_event_send_snmpTraps(&trap_buf);
                            SNMP_CRIT_ENTER();
                            break;
                        }
                        case SNMP_TRAP_TYPE_EVENT: {
#ifdef RFC2819_SUPPORTED_EVENT
                            strcpy(sys_trap_community, snmp_global.snmp_conf.trap_community);
                            /* if event trap community is different between system trap communitry,
                               change it template and wait a moment */
                            if ((compare_result = strcmp(sys_trap_community, snmp_trap_buff[trap_buff_read_idx].event_trap_entry.trap_community))) {
                                SnmpdSetTrapCommunity((uchar *)snmp_trap_buff[trap_buff_read_idx].event_trap_entry.trap_community);
                                VTSS_OS_MSLEEP(100);
                            }


                            rmon_event_trap_entry_t         event_trap_entry = snmp_trap_buff[trap_buff_read_idx].event_trap_entry;
                            SNMP_CRIT_EXIT();
                            event_send_trap(event_trap_entry.is_rising,
                                            event_trap_entry.alarm_index,
                                            event_trap_entry.value,
                                            event_trap_entry.the_threshold,
                                            (oid *)event_trap_entry.alarmed_var,
                                            event_trap_entry.alarmed_var_length,
                                            event_trap_entry.sample_type);
                            SNMP_CRIT_ENTER();

                            if (compare_result) {
                                SnmpdSetTrapCommunity((uchar *) sys_trap_community);
                                VTSS_OS_MSLEEP(100);
                            }
#endif /* RFC2819_SUPPORTED_EVENT */
                            break;
                        }
                        case SNMP_TRAP_TYPE_VARS: {
                            snmp_vars_trap_entry_t trap_entry = snmp_trap_buff[trap_buff_read_idx].trap_entry;
                            SNMP_CRIT_EXIT();
                            (void)trap_event_send_vars(&trap_entry);
                            snmp_free_varbind(trap_entry.vars);
                            SNMP_CRIT_ENTER();
                            break;
                        }
                        case SNMP_TRAP_TYPE_PROBE: {
                            ucd_trap_conf_t trap_conf = snmp_trap_buff[trap_buff_read_idx].trap_conf;
                            vtss_trap_entry_t *tmp = &trap_global.trap_entry[trap_conf.conf_id];
                            SNMP_CRIT_EXIT();
                            if (ucd_send_probe_message(&trap_conf) != 0 ) {
                                SNMP_CRIT_ENTER();
                                T_D("probe fail");
                                if ( tmp->valid ) {
                                    tmp->trap_conf.trap_engineid_len = 0;
                                }
                            } else {
                                SNMP_CRIT_ENTER();
                                T_D("probe sucessfully");
                                if ( tmp->valid ) {
                                    tmp->trap_conf.trap_engineid_len = trap_conf.trap_engineid_len;
                                    memcpy(tmp->trap_conf.trap_engineid, trap_conf.trap_engineid,
                                           tmp->trap_conf.trap_engineid_len);
                                }
                            }
                            break;
                        }
                        default:
                            T_E("Unkown Event");
                            break;
                        }
                        trap_buff_read_idx = (trap_buff_read_idx + 1) % SNMP_TRAP_BUFF_MAX_CNT;
                        SNMP_CRIT_EXIT();
                        VTSS_OS_MSLEEP(100);
                    }
                } // if (SnmpdGetAgentState())
                VTSS_OS_MSLEEP(1000);
            } // while(msg_switch_is_master())
        } // if(msg_switch_is_master())

        // No reason for using CPU ressources when we're a slave
        T_D("Suspending SNMP trap thread");
        cyg_thread_suspend(snmp_trap_thread_handle);
        T_D("Resumed SNMP trap thread");
    } // while(1)
}


/****************************************************************************/
/*  Initialization functions                                                */
/****************************************************************************/

/* Read/create SNMP switch configuration */
static void snmp_conf_read_switch(vtss_isid_t isid_add)
{
    conf_blk_id_t        blk_id;
    vtss_port_no_t       port_no;
    snmp_port_conf_t     *port_conf, new_port_conf;
    snmp_port_conf_blk_t *port_blk;
    int                  i, j;
    BOOL                 do_create;
    ulong                size;
    vtss_isid_t          isid;

    T_D("enter, isid_add: %d", isid_add);

    /* read SNMP port configuration */
    blk_id = CONF_BLK_SNMP_PORT_CONF;

    if (misc_conf_read_use()) {
        if ((port_blk = conf_sec_open(CONF_SEC_GLOBAL, blk_id, &size)) == NULL ||
            size != sizeof(*port_blk)) {
            T_W("conf_sec_open failed or size mismatch, creating defaults");
            port_blk = conf_sec_create(CONF_SEC_GLOBAL, blk_id, sizeof(*port_blk));
            do_create = 1;
        } else if (port_blk->version != SNMP_PORT_CONF_BLK_VERSION) {
            T_W("version mismatch, creating defaults");
            do_create = 1;
        } else {
            do_create = (isid_add != VTSS_ISID_GLOBAL);
        }
    } else {
        port_blk  = NULL;
        do_create = 1;
    }

    snmp_port_default_set(&new_port_conf);
    for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
        if (isid_add != VTSS_ISID_GLOBAL && isid_add != isid) {
            continue;
        }
        SNMP_CRIT_ENTER();
        for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
            i = (isid - VTSS_ISID_START);
            j = (port_no - VTSS_PORT_NO_START);
            if (do_create) {
                /* Use default values */
                if (port_blk != NULL) {
                    port_blk->snmp_port_conf[i * VTSS_PORTS + j] = new_port_conf;
                }
            } else {
                /* Use new configuration */
                if (port_blk) {  // Quiet lint
                    new_port_conf = port_blk->snmp_port_conf[i * VTSS_PORTS + j];
                }
            }
            port_conf = &snmp_global.snmp_port_conf[isid][j];
            *port_conf = new_port_conf;
        }
        SNMP_CRIT_EXIT();
    }

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (port_blk == NULL) {
        T_W("failed to open SNMP port table");
    } else {
        port_blk->version = SNMP_PORT_CONF_BLK_VERSION;
        conf_sec_close(CONF_SEC_GLOBAL, blk_id);
    }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */

    T_D("exit");
}

/* Silent upgrade from old configuration to new one.
 * Returns a (malloc'ed) pointer to the upgraded new configuration
 * or NULL if conversion failed.
 */
static snmp_conf_blk_t *snmp_conf_flash_silent_upgrade(const void *blk, u32 old_ver)
{
    snmp_conf_blk_t *new_blk = NULL;
    if ( old_ver == 1) {
        conf_blk_id_t           blk_id;
        vtss_ipv6_t             ipv6_zero;
        trap_conf_blk_t         *trap_blk = NULL;
        snmp_conf_blk_t         *snmp_blk = (snmp_conf_blk_t *)blk;
        snmp_conf_t             *snmp_conf = &snmp_blk->snmp_conf;
        vtss_trap_sys_conf_t    *trap_sys_conf;
        vtss_trap_entry_t       *trap_entry;
        vtss_trap_conf_t        *trap_conf;
        vtss_trap_event_t       *trap_event;

        blk_id = CONF_BLK_TRAP_CONF;
        memset(&ipv6_zero, 0, sizeof(ipv6_zero));
        if ((trap_blk = conf_sec_create(CONF_SEC_GLOBAL, blk_id, sizeof(trap_conf_blk_t))) == NULL ) {
            return NULL;
        }
        trap_sys_conf = &trap_blk->trap_sys_conf;
        trap_entry = &trap_sys_conf->trap_entry[0];
        trap_conf = &trap_entry->trap_conf;
        trap_event = &trap_entry->trap_event;

        trap_sys_conf->trap_mode = snmp_conf->trap_mode;
        sprintf(trap_entry->trap_conf_name, "legacy");
        trap_conf->conf_id = 0;
        trap_conf->enable = snmp_conf->trap_mode;

#ifdef VTSS_SW_OPTION_IPV6
        if (memcmp(&ipv6_zero, &snmp_conf->trap_dipv6, sizeof(ipv6_zero))) {
            trap_conf->dip.ipv6_flag = TRUE;
            trap_conf->dip.addr.ipv6 = snmp_conf->trap_dipv6;
        } else {
            trap_conf->dip.ipv6_flag = FALSE;
            memcpy(trap_conf->dip.addr.ipv4_str, snmp_conf->trap_dip_string, VTSS_SYS_HOSTNAME_LEN);
            trap_conf->dip.addr.ipv4_str[VTSS_SYS_HOSTNAME_LEN] = 0;
        }
#else
        trap_conf->dip.ipv6_flag = FALSE;
        memcpy(trap_conf->dip.addr.ipv4_str, snmp_conf->trap_dip_string, VTSS_SYS_HOSTNAME_LEN);
        trap_conf->dip.addr.ipv4_str[VTSS_SYS_HOSTNAME_LEN] = 0;
#endif /* VTSS_SW_OPTION_IPV6 */

        trap_conf->trap_port = snmp_conf->trap_port;
        trap_conf->trap_version = snmp_conf->trap_version;
        trap_conf->trap_inform_mode = snmp_conf->trap_inform_mode;
        trap_conf->trap_inform_timeout = snmp_conf->trap_inform_timeout;
        trap_conf->trap_inform_retries = snmp_conf->trap_inform_retries;
        strcpy(trap_conf->trap_community, snmp_conf->trap_community);
#ifdef SNMP_SUPPORT_V3
        trap_conf->trap_probe_engineid = snmp_conf->trap_probe_security_engineid;
        memcpy(trap_conf->trap_engineid, snmp_conf->trap_security_engineid, SNMPV3_MAX_ENGINE_ID_LEN);
        trap_conf->trap_engineid_len = snmp_conf->trap_security_engineid_len;
        strcpy(trap_conf->trap_security_name, snmp_conf->trap_security_name);
#endif

        memset(trap_event, 0, sizeof(vtss_trap_event_t));
        if (snmp_conf->trap_authen_fail ) {
            trap_event->aaa.trap_authen_fail = TRUE;
        }
        if (snmp_conf->trap_linkup_linkdown ) {
            memset(trap_event->interface.trap_linkup, TRUE, sizeof(trap_event->interface.trap_linkup));
            memset(trap_event->interface.trap_linkdown, TRUE, sizeof(trap_event->interface.trap_linkdown));
        }

        if (snmp_conf->trap_mode) {
            memset(trap_event->interface.trap_lldp, TRUE, sizeof(trap_event->interface.trap_lldp));
            memset(&trap_event->system, TRUE, sizeof(trap_event->system));
            memset(&trap_event->sw, TRUE, sizeof(trap_event->sw));
        }

        trap_entry->valid = TRUE;

        trap_blk->version = TRAP_CONF_BLK_VERSION;
        conf_sec_close(CONF_SEC_GLOBAL, blk_id);

    }
    return new_blk;
}

static void del_all_avl_trap_entry(void)
{
    vtss_trap_entry_t *tmp;

    if (vtss_avl_tree_get(&trap_entry_avl, (void **) &tmp, VTSS_AVL_TREE_GET_FIRST) != TRUE) { // entry not existing
        return;
    }

    do {
        if ( FALSE == vtss_avl_tree_delete (&trap_entry_avl, (void **)&tmp) ) {
            T_E("vtss_avl_tree_delete fail(conf_name = %s)", tmp->trap_conf_name);
            break;
        }
    } while (vtss_avl_tree_get(&trap_entry_avl, (void **) &tmp, VTSS_AVL_TREE_GET_NEXT) == TRUE);

}

/* Read/create SNMP stack configuration */
static void snmp_conf_read_stack(BOOL create)
{
    int                           changed;
    BOOL                          do_create;
    ulong                         size;
    snmp_conf_t                   *old_snmp_conf_p, new_snmp_conf;
    snmp_conf_blk_t               *conf_blk_p;
    smon_stat_conf_blk_t          *smon_stat_conf_blk_p;
    port_copy_conf_blk_t          *port_copy_conf_blk_p;
    trap_conf_blk_t               *trap_conf_blk_p;
    conf_blk_id_t                 blk_id;
    ulong                         blk_version;
    ulong                         old_snmp_blk_version = 0;
#ifdef SNMP_SUPPORT_V3
    ulong                         idx;
    snmpv3_communities_conf_blk_t *communities_conf_blk_p;
    snmpv3_users_conf_blk_t       *users_conf_blk_p;
    snmpv3_groups_conf_blk_t      *groups_conf_blk_p;
    snmpv3_views_conf_blk_t       *views_conf_blk_p;
    snmpv3_accesses_conf_blk_t    *accesses_conf_blk_p;
#endif /* SNMP_SUPPORT_V3 */

    T_D("enter, create: %d", create);

    /* Read/create SNMP configuration */
    blk_id = CONF_BLK_SNMP_CONF;
    blk_version = SNMP_CONF_BLK_VERSION;

    if (misc_conf_read_use()) {
        if ((conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, &size)) == NULL ) {
            T_W("conf_sec_open failed, creating defaults");
            conf_blk_p = conf_sec_create(CONF_SEC_GLOBAL, blk_id, sizeof(*conf_blk_p));
            do_create = TRUE;
        } else if (conf_blk_p->version < blk_version) {
            old_snmp_blk_version = conf_blk_p->version;
            T_I("version upgrade, run silent upgrade");
            (void) snmp_conf_flash_silent_upgrade(conf_blk_p, old_snmp_blk_version);
            do_create = FALSE;
        } else if (size != sizeof(snmp_conf_blk_t)) {
            T_W("size mismatch, creating defaults");
            conf_blk_p = conf_sec_create(CONF_SEC_GLOBAL, blk_id, sizeof(*conf_blk_p));
            do_create = TRUE;
        } else {
            do_create = create;
        }
    } else {
        conf_blk_p = NULL;
        do_create  = 1;
    }

    changed = 0;
    SNMP_CRIT_ENTER();
    if (do_create) {
        /* Use default values */
        snmp_default_get(&new_snmp_conf);
        if (conf_blk_p != NULL) {
            conf_blk_p->snmp_conf = new_snmp_conf;
        }
    } else {
        /* Use new configuration */
        if (conf_blk_p != NULL) {  // Quiet lint
            new_snmp_conf = conf_blk_p->snmp_conf;
        }
    }
    old_snmp_conf_p = &snmp_global.snmp_conf;
    if (snmp_conf_changed(old_snmp_conf_p, &new_snmp_conf)) {
        changed = 1;
    }
    snmp_global.snmp_conf = new_snmp_conf;
    SNMP_CRIT_EXIT();

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (conf_blk_p == NULL) {
        T_W("failed to open SNMP table");
    } else {
        conf_blk_p->version = blk_version;
        conf_sec_close(CONF_SEC_GLOBAL, blk_id);
    }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */

    if (changed && create) {
        snmp_stack_snmp_conf_set(VTSS_ISID_GLOBAL);
    }

    blk_id = CONF_BLK_SNMP_SMON_STAT_TABLE;
    blk_version = SNMP_RMON_STAT_CONF_BLK_VERSION;

    if (misc_conf_read_use()) {
        if ((smon_stat_conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, &size)) == NULL ||
            size != sizeof(*smon_stat_conf_blk_p)) {
            T_W("conf_sec_open failed or size mismatch, creating defaults");
            smon_stat_conf_blk_p = conf_sec_create(CONF_SEC_GLOBAL, blk_id, sizeof(*smon_stat_conf_blk_p));
            do_create = 1;
        } else if (smon_stat_conf_blk_p->version != blk_version) {
            T_W("version mismatch, creating defaults");
            do_create = 1;
        } else {
            do_create = create;
        }
    } else {
        smon_stat_conf_blk_p = NULL;
        do_create            = 1;
    }

    SNMP_CRIT_ENTER();
    if (do_create) {
        /* Use default values */
        snmp_global.snmp_smon_stat_entry_num = 0;
        memset(snmp_global.snmp_smon_stat_entry, 0x0, sizeof(snmp_global.snmp_smon_stat_entry));
        if (smon_stat_conf_blk_p) {
            smon_stat_conf_blk_p->snmp_smon_stat_entry_num = snmp_global.snmp_smon_stat_entry_num;
            memcpy(smon_stat_conf_blk_p->snmp_smon_stat_entry, snmp_global.snmp_smon_stat_entry, sizeof(smon_stat_conf_blk_p->snmp_smon_stat_entry));
        }
    } else {
        if (smon_stat_conf_blk_p) {
            snmp_global.snmp_smon_stat_entry_num = smon_stat_conf_blk_p->snmp_smon_stat_entry_num;
            memcpy(snmp_global.snmp_smon_stat_entry, smon_stat_conf_blk_p->snmp_smon_stat_entry, sizeof(smon_stat_conf_blk_p->snmp_smon_stat_entry));
        }
    }
    SNMP_CRIT_EXIT();

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (smon_stat_conf_blk_p == NULL) {
        T_W("failed to open SNMP RMON statistics table");
    } else {
        smon_stat_conf_blk_p->version = blk_version;
        conf_sec_close(CONF_SEC_GLOBAL, blk_id);
    }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */

    blk_id = CONF_BLK_SNMP_PORT_COPY_TABLE;
    blk_version = SNMP_RMON_STAT_CONF_BLK_VERSION;
    if (misc_conf_read_use()) {
        if ((port_copy_conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, &size)) == NULL ||
            size != sizeof(*port_copy_conf_blk_p)) {
            T_W("conf_sec_open failed or size mismatch, creating defaults");
            port_copy_conf_blk_p = conf_sec_create(CONF_SEC_GLOBAL, blk_id, sizeof(*port_copy_conf_blk_p));
            do_create = 1;
        } else if (port_copy_conf_blk_p->version != blk_version) {
            T_W("version mismatch, creating defaults");
            do_create = 1;
        } else {
            do_create = create;
        }
    } else {
        port_copy_conf_blk_p = NULL;
        do_create            = 1;
    }

    SNMP_CRIT_ENTER();
    if (do_create) {
        /* Use default values */
        snmp_global.snmp_port_copy_entry_num = 0;
        memset(snmp_global.snmp_port_copy_entry, 0, sizeof(snmp_global.snmp_port_copy_entry));

        if (port_copy_conf_blk_p != NULL) {
            port_copy_conf_blk_p->snmp_port_copy_entry_num = snmp_global.snmp_port_copy_entry_num;
            memcpy(port_copy_conf_blk_p->snmp_port_copy_entry, snmp_global.snmp_port_copy_entry, sizeof(port_copy_conf_blk_p->snmp_port_copy_entry));
        }
    } else {
        if (port_copy_conf_blk_p) {
            snmp_global.snmp_port_copy_entry_num = port_copy_conf_blk_p->snmp_port_copy_entry_num;
            memcpy(snmp_global.snmp_port_copy_entry, port_copy_conf_blk_p->snmp_port_copy_entry, sizeof(port_copy_conf_blk_p->snmp_port_copy_entry));
        }
    }

    SNMP_CRIT_EXIT();

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (port_copy_conf_blk_p == NULL) {
        T_W("failed to open SNMP RMON statistics table");
    } else {
        port_copy_conf_blk_p->version = blk_version;
        conf_sec_close(CONF_SEC_GLOBAL, blk_id);
    }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */

#ifdef SNMP_SUPPORT_V3
    /* Read/create SNMPv3 communities configuration */
    blk_id = CONF_BLK_SNMPV3_COMMUNITIES_CONF;
    blk_version = SNMPV3_COMMUNITIES_CONF_BLK_VERSION;
    if (misc_conf_read_use()) {
        if ((communities_conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, &size)) == NULL ||
            size != sizeof(*communities_conf_blk_p)) {
            T_W("conf_sec_open failed or size mismatch, creating defaults");
            communities_conf_blk_p = conf_sec_create(CONF_SEC_GLOBAL, blk_id, sizeof(*communities_conf_blk_p));
            do_create = 1;
        } else if (communities_conf_blk_p->version != blk_version) {
            T_W("version mismatch, creating defaults");
            do_create = 1;
        } else {
            do_create = create;
        }
    } else {
        communities_conf_blk_p = NULL;
        do_create              = 1;
    }

    SNMP_CRIT_ENTER();
    if (do_create) {
        /* Use default values */
        memset(snmp_global.communities_conf, 0x0, sizeof(snmp_global.communities_conf));
        snmpv3_default_communities_get((ulong *)&snmp_global.communities_conf_num, &snmp_global.communities_conf[0]);
        if (communities_conf_blk_p != NULL) {
            memset(communities_conf_blk_p->communities_conf, 0x0, sizeof(communities_conf_blk_p->communities_conf));
            snmpv3_default_communities_get((ulong *)&communities_conf_blk_p->communities_conf_num, &communities_conf_blk_p->communities_conf[0]);
        }
    } else {
        if (communities_conf_blk_p) {  // Quiet lint
            snmp_global.communities_conf_num = communities_conf_blk_p->communities_conf_num;
            memcpy(snmp_global.communities_conf, communities_conf_blk_p->communities_conf, sizeof(communities_conf_blk_p->communities_conf));
        }
    }

    for (idx = 0; idx < SNMPV3_MAX_COMMUNITIES; idx++) {
        if (snmp_global.communities_conf[idx].valid) {
            if (snmp_global.communities_conf[idx].storage_type != SNMP_MGMT_STORAGE_PERMANENT ||
                snmp_global.communities_conf[idx].status != SNMP_MGMT_ROW_ACTIVE) {
                snmp_global.communities_conf[idx].valid = 0;
                snmp_global.communities_conf_num--;
            }
        }
    }
    SNMP_CRIT_EXIT();
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (communities_conf_blk_p == NULL) {
        T_W("failed to open SNMPv3 communities table");
    } else {
        communities_conf_blk_p->version = blk_version;
        conf_sec_close(CONF_SEC_GLOBAL, blk_id);
    }
#endif

    /* Read/create SNMPv3 users configuration */
    blk_id = CONF_BLK_SNMPV3_USERS_CONF;
    blk_version = SNMPV3_USERS_CONF_BLK_VERSION;
    if ((users_conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, &size)) == NULL) {
        T_W("conf_sec_open failed, creating defaults");
        users_conf_blk_p = conf_sec_create(CONF_SEC_GLOBAL, blk_id, sizeof(*users_conf_blk_p));
        do_create = TRUE;
    } else if (size != sizeof(snmpv3_users_conf_blk_t)) {
        T_W("size mismatch, creating defaults");
        users_conf_blk_p = conf_sec_create(CONF_SEC_GLOBAL, blk_id, sizeof(*users_conf_blk_p));
        do_create = TRUE;
    } else {
        do_create = create;
    }

    SNMP_CRIT_ENTER();
    if (do_create) {
        /* Use default values */
        memset(snmp_global.users_conf, 0x0, sizeof(snmp_global.users_conf));
        snmpv3_default_users_get((ulong *)&snmp_global.users_conf_num, &snmp_global.users_conf[0]);

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
        if (users_conf_blk_p != NULL) {
            memset(users_conf_blk_p->users_conf, 0x0, sizeof(users_conf_blk_p->users_conf));
            snmpv3_default_users_get((ulong *)&users_conf_blk_p->users_conf_num, &users_conf_blk_p->users_conf[0]);
        }
#endif
    } else {
        if (users_conf_blk_p) {
            snmp_global.users_conf_num = users_conf_blk_p->users_conf_num;
            memcpy(snmp_global.users_conf, users_conf_blk_p->users_conf, sizeof(users_conf_blk_p->users_conf));
        }
    }

    for (idx = 0; idx < SNMPV3_MAX_USERS; idx++) {
        if (snmp_global.users_conf[idx].valid) {
            if (snmp_global.users_conf[idx].storage_type != SNMP_MGMT_STORAGE_PERMANENT ||
                snmp_global.users_conf[idx].status != SNMP_MGMT_ROW_ACTIVE) {
                snmp_global.users_conf[idx].valid = 0;
                snmp_global.users_conf_num--;
            }
        }
    }
    SNMP_CRIT_EXIT();
    if (users_conf_blk_p == NULL) {
        T_W("failed to open SNMPv3 users table");
    } else {
        users_conf_blk_p->version = blk_version;
        conf_sec_close(CONF_SEC_GLOBAL, blk_id);
    }

    /* Read/create SNMPv3 groups configuration */
    blk_id = CONF_BLK_SNMPV3_GROUPS_CONF;
    blk_version = SNMPV3_GROUPS_CONF_BLK_VERSION;
    if (misc_conf_read_use()) {
        if ((groups_conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, &size)) == NULL ||
            size != sizeof(*groups_conf_blk_p)) {
            T_W("conf_sec_open failed or size mismatch, creating defaults");
            groups_conf_blk_p = conf_sec_create(CONF_SEC_GLOBAL, blk_id, sizeof(*groups_conf_blk_p));
            do_create = 1;
        } else if (groups_conf_blk_p->version != blk_version) {
            T_W("version mismatch, creating defaults");
            do_create = 1;
        } else {
            do_create = create;
        }
    } else {
        groups_conf_blk_p = NULL;
        do_create         = 1;
    }

    SNMP_CRIT_ENTER();
    if (do_create) {
        /* Use default values */
        memset(snmp_global.groups_conf, 0x0, sizeof(snmp_global.groups_conf));
        snmpv3_default_groups_get((ulong *)&snmp_global.groups_conf_num, &snmp_global.groups_conf[0]);
        if (groups_conf_blk_p != NULL) {
            memset(groups_conf_blk_p->groups_conf, 0x0, sizeof(groups_conf_blk_p->groups_conf));
            snmpv3_default_groups_get((ulong *)&groups_conf_blk_p->groups_conf_num, &groups_conf_blk_p->groups_conf[0]);
        }
    } else {
        if (groups_conf_blk_p) {
            snmp_global.groups_conf_num = groups_conf_blk_p->groups_conf_num;
            memcpy(snmp_global.groups_conf, groups_conf_blk_p->groups_conf, sizeof(groups_conf_blk_p->groups_conf));
        }
    }

    for (idx = 0; idx < SNMPV3_MAX_GROUPS; idx++) {
        if (snmp_global.groups_conf[idx].valid) {
            if (snmp_global.groups_conf[idx].storage_type != SNMP_MGMT_STORAGE_PERMANENT ||
                snmp_global.groups_conf[idx].status != SNMP_MGMT_ROW_ACTIVE) {
                snmp_global.groups_conf[idx].valid = 0;
                snmp_global.groups_conf_num--;
            }
        }
    }

    SNMP_CRIT_EXIT();
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (groups_conf_blk_p == NULL) {
        T_W("failed to open SNMPv3 groups table");
    } else {
        groups_conf_blk_p->version = blk_version;
        conf_sec_close(CONF_SEC_GLOBAL, blk_id);
    }
#endif

    /* Read/create SNMPv3 views configuration */
    blk_id = CONF_BLK_SNMPV3_VIEWS_CONF;
    blk_version = SNMPV3_VIEWS_CONF_BLK_VERSION;
    if (misc_conf_read_use()) {
        if ((views_conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, &size)) == NULL ||
            size != sizeof(*views_conf_blk_p)) {
            T_W("conf_sec_open failed or size mismatch, creating defaults");
            views_conf_blk_p = conf_sec_create(CONF_SEC_GLOBAL, blk_id, sizeof(*views_conf_blk_p));
            do_create = 1;
        } else if (views_conf_blk_p->version != blk_version) {
            T_W("version mismatch, creating defaults");
            do_create = 1;
        } else {
            do_create = create;
        }
    } else {
        views_conf_blk_p = NULL;
        do_create        = 1;
    }

    SNMP_CRIT_ENTER();
    if (do_create) {
        /* Use default values */
        memset(snmp_global.views_conf, 0x0, sizeof(snmp_global.views_conf));
        snmpv3_default_views_get((ulong *)&snmp_global.views_conf_num, &snmp_global.views_conf[0]);
        if (views_conf_blk_p != NULL) {
            memset(views_conf_blk_p->views_conf, 0x0, sizeof(views_conf_blk_p->views_conf));
            snmpv3_default_views_get((ulong *)&views_conf_blk_p->views_conf_num, &views_conf_blk_p->views_conf[0]);
        }
    } else {
        if (views_conf_blk_p) {
            snmp_global.views_conf_num = views_conf_blk_p->views_conf_num;
            memcpy(snmp_global.views_conf, views_conf_blk_p->views_conf, sizeof(views_conf_blk_p->views_conf));
        }
    }

    for (idx = 0; idx < SNMPV3_MAX_VIEWS; idx++) {
        if (snmp_global.views_conf[idx].valid) {
            if (snmp_global.views_conf[idx].storage_type != SNMP_MGMT_STORAGE_PERMANENT ||
                snmp_global.views_conf[idx].status != SNMP_MGMT_ROW_ACTIVE) {
                snmp_global.views_conf[idx].valid = 0;
                snmp_global.views_conf_num--;
            }
        }
    }

    SNMP_CRIT_EXIT();
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (views_conf_blk_p == NULL) {
        T_W("failed to open SNMPv3 views table");
    } else {
        views_conf_blk_p->version = blk_version;
        conf_sec_close(CONF_SEC_GLOBAL, blk_id);
    }
#endif

    /* Read/create SNMPv3 accesses configuration */
    blk_id = CONF_BLK_SNMPV3_ACCESSES_CONF;
    blk_version = SNMPV3_ACCESSES_CONF_BLK_VERSION;
    if (misc_conf_read_use()) {
        if ((accesses_conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, &size)) == NULL ||
            size != sizeof(*accesses_conf_blk_p)) {
            T_W("conf_sec_open failed or size mismatch, creating defaults");
            accesses_conf_blk_p = conf_sec_create(CONF_SEC_GLOBAL, blk_id, sizeof(*accesses_conf_blk_p));
            do_create = 1;
        } else if (accesses_conf_blk_p->version != blk_version) {
            T_W("version mismatch, creating defaults");
            do_create = 1;
        } else {
            do_create = create;
        }
    } else {
        accesses_conf_blk_p = NULL;
        do_create           = 1;
    }

    SNMP_CRIT_ENTER();
    if (do_create) {
        /* Use default values */
        memset(snmp_global.accesses_conf, 0x0, sizeof(snmp_global.accesses_conf));
        snmpv3_default_accesses_get((ulong *)&snmp_global.accesses_conf_num, &snmp_global.accesses_conf[0]);
        if (accesses_conf_blk_p != NULL) {
            memset(accesses_conf_blk_p->accesses_conf, 0x0, sizeof(accesses_conf_blk_p->accesses_conf));
            snmpv3_default_accesses_get((ulong *)&accesses_conf_blk_p->accesses_conf_num, &accesses_conf_blk_p->accesses_conf[0]);
        }
    } else {
        if (accesses_conf_blk_p) {
            snmp_global.accesses_conf_num = accesses_conf_blk_p->accesses_conf_num;
            memcpy(snmp_global.accesses_conf, accesses_conf_blk_p->accesses_conf, sizeof(accesses_conf_blk_p->accesses_conf));
        }
    }

    for (idx = 0; idx < SNMPV3_MAX_ACCESSES; idx++) {
        if (snmp_global.accesses_conf[idx].valid) {
            if (snmp_global.accesses_conf[idx].storage_type != SNMP_MGMT_STORAGE_PERMANENT ||
                snmp_global.accesses_conf[idx].status != SNMP_MGMT_ROW_ACTIVE) {
                snmp_global.accesses_conf[idx].valid = 0;
                snmp_global.accesses_conf_num--;
            }
        }
    }

    SNMP_CRIT_EXIT();
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (accesses_conf_blk_p == NULL) {
        T_W("failed to open SNMPv3 accesses table");
    } else {
        accesses_conf_blk_p->version = blk_version;
        conf_sec_close(CONF_SEC_GLOBAL, blk_id);
    }
#endif
#endif /* SNMP_SUPPORT_V3 */

    blk_id = CONF_BLK_TRAP_CONF;
    blk_version = TRAP_CONF_BLK_VERSION;
    if (misc_conf_read_use()) {
        if ((trap_conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, &size)) == NULL) {
            T_W("conf_sec_open failed or new configuration, creating defaults");
            trap_conf_blk_p = conf_sec_create(CONF_SEC_GLOBAL, blk_id, sizeof(*trap_conf_blk_p));
            do_create = TRUE;
        } else if (trap_conf_blk_p->version < blk_version) {
            T_I("version upgrade, run silent upgrade");
            T_W("upgrade failed, creating defaults");
            do_create = TRUE;
        } else if (size != sizeof(*trap_conf_blk_p)) {
            T_W("size mismatch, creating defaults");
            trap_conf_blk_p = conf_sec_create(CONF_SEC_GLOBAL, blk_id, sizeof(*trap_conf_blk_p));
            do_create = TRUE;
        } else {
            do_create = create;
        }
    } else {
        trap_conf_blk_p = NULL;
        do_create       = TRUE;
    }

    SNMP_CRIT_ENTER();
    if (do_create) {
        /* Use default values */
        del_all_avl_trap_entry();
        memset(&trap_global, 0, sizeof(trap_global));
        trap_global.trap_mode = FALSE;
        if (trap_conf_blk_p != NULL) {
            memset(&trap_conf_blk_p->trap_sys_conf, 0, sizeof(trap_conf_blk_p->trap_sys_conf));
            trap_conf_blk_p->trap_sys_conf.trap_mode = FALSE;
        }

    } else {
        if (trap_conf_blk_p) {
            memcpy(&trap_global, &trap_conf_blk_p->trap_sys_conf, sizeof(trap_conf_blk_p->trap_sys_conf));
        }
    }

    int i;
    vtss_trap_entry_t *tmp;
    for (i = 0, tmp = &trap_global.trap_entry[i]; i < VTSS_TRAP_CONF_MAX; i++, tmp++) {
        if ( TRUE == tmp->valid) {
            (void) vtss_avl_tree_add(&trap_entry_avl, tmp);
        }
    }
    SNMP_CRIT_EXIT();
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (trap_conf_blk_p == NULL) {
        T_W("failed to open SNMP RMON statistics table");
    } else {
        trap_conf_blk_p->version = blk_version;
        conf_sec_close(CONF_SEC_GLOBAL, blk_id);
    }
#endif

    T_D("exit");
}

/* Module start */
static void snmp_start(BOOL init)
{
    vtss_isid_t      isid;
    vtss_port_no_t   port_no;
    snmp_conf_t      *conf_p;
    snmp_port_conf_t *port_conf;
    vtss_rc          rc;

    T_D("enter, init: %d", init);

    if (init) {
        /* Initialize SNMP configuration */
        conf_p = &snmp_global.snmp_conf;
        snmp_default_get(conf_p);

        /* Initialize SNMP port configuration */
        for (isid = VTSS_ISID_LOCAL; isid < VTSS_ISID_END; isid++) {
            for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
                port_conf = &snmp_global.snmp_port_conf[isid][port_no - VTSS_PORT_NO_START];
                snmp_port_default_set(port_conf);
            }
        }

        /* Initialize message buffers */
        VTSS_OS_SEM_CREATE(&snmp_global.request.sem, 1);

        /* Create semaphore for critical regions */
        critd_init(&snmp_global.crit, "snmp_global.crit", VTSS_MODULE_ID_SNMP, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
        SNMP_CRIT_EXIT();

        /* Initialize mibContextTable semaphore for critical regions */
        mibContextTable_init();

        /* Create SNMP thread */
        cyg_thread_create(THREAD_DEFAULT_PRIO,
                          snmp_trap_thread,
                          0,
                          "SNMP Trap",
                          snmp_trap_thread_stack,
                          sizeof(snmp_trap_thread_stack),
                          &snmp_trap_thread_handle,
                          &snmp_trap_thread_block);

        /* place any other initialization junk you need here,
           Initialize SNMP/eCos */
        if (snmp_register_callback(SNMP_CALLBACK_APPLICATION,
                                   SNMPD_CALLBACK_ACM_CHECK,
                                   snmp_check_callback, NULL)) {
            T_W("Calling snmp_register_callback() failed.\n");
        }
        if (snmp_register_callback(SNMP_CALLBACK_APPLICATION,
                                   SNMPD_CALLBACK_ACM_CHECK_INITIAL,
                                   snmp_check_callback, NULL)) {
            T_W("Calling snmp_register_callback() failed.\n");
        }
#ifdef SNMP_SUPPORT_V3
        if (snmp_register_callback(SNMP_CALLBACK_APPLICATION,
                                   SNMPD_CALLBACK_SEND_TRAP2,
                                   snmpv3_trap_callback, NULL)) {
            T_W("Calling snmp_register_callback() failed.\n");
        }
#endif /* SNMP_SUPPORT_V3 */
        SnmpdMibModulesRegister(vtss_snmp_mibs_init);
        cyg_net_snmp_init();
    } else {
        /* Register for stack messages */
        if ((rc = snmp_stack_register()) != VTSS_OK) {
            T_W("snmp_stack_register(): failed rc = %d", rc);
        }

        /* Register port change callback */
        if ((rc = port_global_change_register(VTSS_MODULE_ID_SNMP, snmp_port_state_change_callback)) != VTSS_OK) {
            T_W("port_global_change_register(): failed rc = %d", rc);
        }

#if SNMP_PRIVATE_MIB_ENTERPRISE == 6603 /* VTSS */
        // Save a unique product ID in the last entry of snmp_private_mib_oid[] array.
        // The product ID is a concatenation of the software type (Web/SMB/CE, Stackable/Standalone) and the
        // chip ID that the API is instantiated with.
        snmp_private_mib_oid[OID_LENGTH(snmp_private_mib_oid) - 1] = (misc_softwaretype() << 24) | misc_chiptype();
#endif
    }
    T_D("exit");
}

/* Initialize module */
vtss_rc snmp_init(vtss_init_data_t *data)
{
    vtss_rc rc = VTSS_OK;
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
        snmp_master_down_flag = 0;
        snmp_start(1);
        ( void ) trap_init();
#ifdef VTSS_SW_OPTION_VCLI
        snmp_cli_init();
#endif
#ifdef VTSS_SW_OPTION_ICFG
        if ((rc = snmp_icfg_init()) != VTSS_OK) {
            T_D("Calling snmp_icfg_init() failed rc = %s", error_txt(rc));
        }
        if ((rc = trap_icfg_init()) != VTSS_OK) {
            T_D("Calling snmp_icfg_init() failed rc = %s", error_txt(rc));
        }
#endif
        break;
    case INIT_CMD_START:
        T_D("START");
        snmp_start(0);
        break;
    case INIT_CMD_CONF_DEF:
        T_D("CONF_DEF, isid: %d", isid);
        if (isid == VTSS_ISID_LOCAL) {
            /* Reset local configuration */
        } else if (isid == VTSS_ISID_GLOBAL) {
            /* Reset stack configuration */
            snmp_conf_read_stack(1);
            snmp_smon_conf_engine_set();
            SNMP_CRIT_ENTER();
            trap_mgmt_sys_default_get(&trap_global);
            SNMP_CRIT_EXIT();
#ifdef SNMP_SUPPORT_V3
            snmpv3_engine_conf_set();
#endif /* SNMP_SUPPORT_V3 */
        } else if (VTSS_ISID_LEGAL(isid)) {
            /* Reset switch configuration */
        }
        break;
    case INIT_CMD_MASTER_UP: {
        T_D("MASTER_UP");

        /* Read stack and switch configuration */
        snmp_conf_read_stack(0);
        snmp_conf_read_switch(VTSS_ISID_GLOBAL);

        if (!snmp_master_down_flag) {
            vtss_restart_status_t status;
            vtss_restart_t        restart;
            snmp_trap_if_info_t   trap_if_info;

            memset(&trap_if_info, 0x0, sizeof(trap_if_info));
            restart = (vtss_restart_status_get(NULL, &status) == VTSS_RC_OK ?
                       status.restart : VTSS_RESTART_COLD);
#ifdef VTSS_SW_OPTION_SYSLOG
            S_I("Switch just made a %s boot.", control_system_restart_to_str(restart));
#endif
            snmp_mgmt_send_trap(restart == VTSS_RESTART_COLD ? SNMP_TRAP_TYPE_COLDSTART : SNMP_TRAP_TYPE_WARMSTART, trap_if_info);
        }

        /* Starting SNMP trap thread (became master) */
        cyg_thread_resume(snmp_trap_thread_handle);


#ifdef SNMP_SUPPORT_V3
        snmpv3_engine_conf_set();
#endif /* SNMP_SUPPORT_V3 */

        snmp_master_down_flag = 0;
        break;
    }
    case INIT_CMD_MASTER_DOWN:
        T_D("MASTER_DOWN");
        snmp_master_down_flag = 1;
        trap_buff_read_idx = trap_buff_write_idx = 0;
        break;
    case INIT_CMD_SWITCH_ADD:
        T_D("SWITCH_ADD, isid: %d", isid);
        /* Apply all configuration to switch */
        snmp_stack_snmp_conf_set(isid);
#ifdef RFC4133_SUPPORTED_ENTITY
#if RFC4133_SUPPORTED_ENTITY
        rc = entConfigChange(isid);
#endif  /*RFC4133_SUPPORTED_ENTITY*/
#endif // RFC4133_SUPPORTED_ENTITY ifdef
        break;
    case INIT_CMD_SWITCH_DEL:
        T_D("SWITCH_DEL, isid: %d", isid);
#ifdef RFC4133_SUPPORTED_ENTITY
#if RFC4133_SUPPORTED_ENTITY
        rc = entConfigChange(isid);
#endif  /*RFC4133_SUPPORTED_ENTITY*/
#endif // RFC4133_SUPPORTED_ENTITY ifdef
        break;
    default:
        break;
    }

    T_D("exit");

    return rc;
}

#ifdef VTSS_SW_OPTION_SYSLOG
/* SNMP command text */
char *snmp_command_txt(int command)
{
    char *txt;

    switch (command) {
    case SNMP_MSG_GET:
        txt = "GET";
        break;
    case SNMP_MSG_GETNEXT:
        txt = "GETNEXT";
        break;
    case SNMP_MSG_GETBULK:
        txt = "GETBULK";
        break;
    case SNMP_MSG_SET:
        txt = "SET";
        break;
    default:
        txt = "SNMP unknown command type";
        break;
    }
    return txt;
}
#endif /* VTSS_SW_OPTION_SYSLOG */

static int snmp_check_callback(int majorID, int minorID, void *serverarg, void *clientarg)
{
    struct view_parameters *view_parms = (struct view_parameters *) serverarg;
    char community[256];
    ulong config_version;
#ifdef VTSS_SW_OPTION_SYSLOG
    char buf[40];
#ifdef VTSS_SW_OPTION_IPV6
    vtss_ipv6_t sip_v6;
#endif /* VTSS_SW_OPTION_IPV6 */
#endif /* VTSS_SW_OPTION_SYSLOG */

    /* Process SNMP packet only when stack role is master */
    if (!msg_switch_is_master()) {
        view_parms->errorcode = SNMP_ACCESS_BAD_ERROR_STACK_ROLE;
        return view_parms->errorcode;
    } else {
        view_parms->errorcode = 0;
    }

    /* check version */
    if (view_parms->pdu->version != SNMP_VERSION_1 && view_parms->pdu->version != SNMP_VERSION_2c && view_parms->pdu->version != SNMP_VERSION_3) {
        view_parms->errorcode = SNMP_ACCESS_BAD_VER;
        return SNMP_ACCESS_BAD_VER; /* EstaX-34 project only supported V1/V2C/V3 now */
    }

    SNMP_CRIT_ENTER();
    config_version = snmp_global.snmp_conf.version;
    SNMP_CRIT_EXIT();

    switch (config_version) {
    case SNMP_SUPPORT_V1:
        if (view_parms->pdu->version != SNMP_VERSION_1) {
            view_parms->errorcode = SNMP_ACCESS_BAD_VER;
            return SNMP_ACCESS_BAD_VER;
        }
        break;
    case SNMP_SUPPORT_V2C:
        if (view_parms->pdu->version == SNMP_VERSION_3) {
            view_parms->errorcode = SNMP_ACCESS_BAD_VER;
            return SNMP_ACCESS_BAD_VER;
        }
        break;
#ifdef SNMP_SUPPORT_V3
    case SNMP_SUPPORT_V3:
        /* check SNMPv3 community in
           managed\eCos\packages\net\snmp\agent\current\src\mibgroup\mibII\vacm_vars.c\vacm_in_view() */
        return SNMP_ACCESS_NO_ERR;
#endif /* SNMP_SUPPORT_V3 */
    }

    /* check community */
    memcpy(community, view_parms->pdu->community, view_parms->pdu->community_len);
    community[view_parms->pdu->community_len] = '\0';

    switch (view_parms->pdu->command) {
    case SNMP_MSG_GET:
    case SNMP_MSG_GETNEXT:
    case SNMP_MSG_GETBULK:
        SNMP_CRIT_ENTER();
        if (!strcmp(snmp_global.snmp_conf.read_community, community) || !strcmp(snmp_global.snmp_conf.write_community, community)) {
            SNMP_CRIT_EXIT();
            return SNMP_ACCESS_NO_ERR;
        }
        view_parms->errorcode = SNMP_ACCESS_BAD_COMMUNITY;
        if (!strcmp(snmp_global.snmp_conf.write_community, community)) {
            view_parms->errorcode |= SNMP_ACCESS_BAD_COMMUNITY_USE;
        }
        SNMP_CRIT_EXIT();
        break;
    case SNMP_MSG_SET:
        SNMP_CRIT_ENTER();
        if (!strcmp(snmp_global.snmp_conf.write_community, community)) {
            SNMP_CRIT_EXIT();
            return SNMP_ACCESS_NO_ERR;
        }
        view_parms->errorcode = SNMP_ACCESS_BAD_COMMUNITY;
        if (!strcmp(snmp_global.snmp_conf.read_community, community)) {
            view_parms->errorcode |= SNMP_ACCESS_BAD_COMMUNITY_USE;
        }
        SNMP_CRIT_EXIT();
        break;

    default:
        return view_parms->errorcode |= SNMP_ACCESS_BAD_COMMAND;
    }

    SNMP_CRIT_ENTER();
    if (snmp_global.snmp_conf.trap_authen_fail) {
        view_parms->errorcode |= SNMP_ACCESS_TRAP_AUTH_FAIL;
    }
    SNMP_CRIT_EXIT();

#ifdef VTSS_SW_OPTION_SYSLOG
    if (view_parms->errorcode & SNMP_ACCESS_BAD_COMMUNITY) {
#ifdef VTSS_SW_OPTION_IPV6
        if (view_parms->pdu->address.sa_family == AF_INET6) {
            memcpy(sip_v6.addr, view_parms->pdu->address.sa_pad2, 16);
            S_I("SNMP %s %s authorization fail from IP address %s.",
                view_parms->pdu->version == SNMP_VERSION_1 ? "v1" : "v2c",
                snmp_command_txt(view_parms->pdu->command),
                misc_ipv6_txt(&sip_v6, buf));
        } else {
            S_I("SNMP %s %s authorization fail from IP address %s.",
                view_parms->pdu->version == SNMP_VERSION_1 ? "v1" : "v2c",
                snmp_command_txt(view_parms->pdu->command),
                misc_ipv4_txt(ntohl(view_parms->pdu->address.sa_align), buf));
        }
#else
        S_I("SNMP %s %s authorization fail from IP address %s.",
            view_parms->pdu->version == SNMP_VERSION_1 ? "v1" : "v2c",
            snmp_command_txt(view_parms->pdu->command),
            misc_ipv4_txt(ntohl(view_parms->pdu->address.sa_align), buf));
#endif /* VTSS_SW_OPTION_IPV6 */
    }
#endif /* VTSS_SW_OPTION_SYSLOG */

    if (view_parms->errorcode & SNMP_ACCESS_TRAP_AUTH_FAIL) {
        snmp_vars_trap_entry_t  trap_entry;
        oid auth_fail_oid[] = AUTH_FAIL_OID;
        trap_entry.oid_len = OID_LENGTH(auth_fail_oid);
        trap_entry.vars = NULL;
        memcpy(trap_entry.oid, auth_fail_oid,
               sizeof(oid) * trap_entry.oid_len);
        snmp_send_vars_trap(SNMP_TRAP_AUTHFAIL, &trap_entry);
    }

    return view_parms->errorcode; /* Agent Engine will drop the request */
}

#ifdef SNMP_SUPPORT_V3
static int snmpv3_trap_callback(int majorID, int minorID, void *serverarg, void *clientarg)
{
    struct snmp_pdu *pdu = (struct snmp_pdu *) serverarg;
    snmpv3_users_conf_t user_conf;

    /* Process SNMP packet only when stack role is master */
    if (!msg_switch_is_master()) {
        return 1;
    }

    /* check version */
    if (pdu->version != SNMP_VERSION_3) {
        return 1;
    }

    memset(&user_conf, 0, sizeof(user_conf));
    if (pdu->securityEngineIDLen) {
        memcpy(user_conf.engineid, pdu->securityEngineID, pdu->securityEngineIDLen);
        user_conf.engineid_len = pdu->securityEngineIDLen;
    } else {
        SNMP_CRIT_ENTER();
        memcpy(user_conf.engineid, snmp_global.snmp_conf.trap_security_engineid, snmp_global.snmp_conf.trap_security_engineid_len);
        user_conf.engineid_len = snmp_global.snmp_conf.trap_security_engineid_len;
        SNMP_CRIT_EXIT();
    }

    SNMP_CRIT_ENTER();
    strcpy(user_conf.user_name, snmp_global.snmp_conf.trap_security_name);
    SNMP_CRIT_EXIT();
    if (snmpv3_mgmt_users_conf_get(&user_conf, FALSE) != VTSS_OK) {
        return 1;
    }

    /* process continue, even search users database fail (default name: SNMPV3_NONAME) */
    SNMP_CRIT_ENTER();
    if (!pdu->securityEngineIDLen && snmp_global.snmp_conf.trap_security_engineid_len) {
        pdu->securityEngineID = VTSS_MALLOC(snmp_global.snmp_conf.trap_security_engineid_len);
        if (!pdu->securityEngineID) {
            SNMP_CRIT_EXIT();
            return 1;
        }
        memcpy(pdu->securityEngineID, snmp_global.snmp_conf.trap_security_engineid, snmp_global.snmp_conf.trap_security_engineid_len);
        pdu->securityEngineIDLen = snmp_global.snmp_conf.trap_security_engineid_len;
    }
    SNMP_CRIT_EXIT();

    pdu->securityName = VTSS_MALLOC(strlen(user_conf.user_name) + 1);
    if (!pdu->securityName) {
        return 1;
    }

    strcpy(pdu->securityName, user_conf.user_name);
    pdu->securityNameLen = strlen(user_conf.user_name);

    pdu->securityLevel = user_conf.security_level;

    /* the other PDU parameters (context name, security model ...) will do
       in \managed\eCos\packages\net\snmp\lib\current\include\snmp_api.c\snmpv3_build() */

    return 0;
}

/* Post configure if using NTP */
void snmpv3_mgmt_ntp_post_conf(void)
{
    SnmpdNtpPostConfig();
}
#endif /* SNMP_SUPPORT_V3 */

/* we remove SNMP/eCos inital function to EstaX-34 management module,
 so always need compiler this function for eCos/SNMP module using */
/*lint -esym(459, vtss_snmp_mibs_init) ... We're thread-safe */
void vtss_snmp_mibs_init(void)
{
    // The sysORTable must be initialized first because the rest of MIB inital
    // function will register to sysORTable.
    sysORTable_init();

    // Initialize SNMP MIB redefine table
    snmp_mib_redefine_init();

#if RFC1213_SUPPORTED_SYSTEM
    init_mib2_system();
#endif /* RFC1213_SUPPORTED_SYSTEM */

#if RFC1213_SUPPORTED_INTERFACES
    init_mib2_interfaces();
#endif /* RFC1213_SUPPORTED_INTERFACES */

#if RFC1213_SUPPORTED_IP
    init_mib2_ip();
#endif /* RFC1213_SUPPORTED_IP */

#ifdef RFC2674_SUPPORTED_Q_BRIDGE
    init_rfc2674_q_bridge();
#endif  /* RFC2674_Q_BRIDGE  */

#ifdef RFC4363_SUPPORTED_P_BRIDGE
    init_rfc4363_p_bridge();
#endif  /* RFC2674_Q_BRIDGE  */

#ifdef IEEE8021_Q_BRIDGE
    ieee8021BridgeMib_init(); /* ieee8021QBridgeMib needs to reference ieee8021BridgeBasePortTable */
    ieee8021QBridgeMib_init();
#endif

#if RFC1213_SUPPORTED_ICMP
    init_mib2_icmp();
#endif /* RFC1213_SUPPORTED_ICMP */

#if RFC1213_SUPPORTED_TCP
    init_mib2_tcp();
#endif /* RFC1213_SUPPORTED_TCP */

#if RFC1213_SUPPORTED_UDP
    init_mib2_udp();
#endif /* RFC1213_SUPPORTED_UDP */

#if RFC1213_SUPPORTED_SNMP
    init_mib2_snmp();
#endif /* RFC1213_SUPPORTED_SNMP */

#ifdef RFC2819_SUPPORTED_STATISTICS
    init_rmon_statisticsMIB();
#endif /* RFC2819_SUPPORTED_STATISTICS */

#ifdef RFC2819_SUPPORTED_HISTORY
    init_rmon_historyMIB();
#endif /* RFC2819_SUPPORTED_HISTORY */

#ifdef RFC2819_SUPPORTED_EVENT
    init_rmon_eventMIB();
#endif /* RFC2819_SUPPORTED_EVENT */

#ifdef RFC2819_SUPPORTED_AlARM
    init_rmon_alarmMIB();
#endif /* RFC2819_SUPPORTED_AlARM */

#ifdef VTSS_SW_OPTION_SMON
    init_switchRMON();
#endif /* VTSS_SW_OPTION_SMON */

#if RFC4188_SUPPORTED_DOT1D_BRIDGE
    init_dot1dBridge();
#endif /* RFC4188_SUPPORTED_DOT1D_BRIDGE */

#ifdef VTSS_SW_OPTION_LLDP
#ifdef DOT1AB_LLDP
    init_lldpObjects();
#endif /* DOT1AB_LLDP */
#endif /* VTSS_SW_OPTION_LLDP */

#if RFC3635_SUPPORTED_TRANSMISSION
    init_transmission();
#endif /* RFC36353_SUPPORTED_TRANSMISSION */

#ifdef VTSS_SW_OPTION_DOT1X
#ifdef IEEE8021X_SUPPORTED_MIB
#if IEEE8021X_SUPPORTED_MIB
    init_ieee8021paeMIB();
#endif /* IEE8021X_SUPPORTED_MIB */
#endif /* IEEE8021X_SUPPORTED_MIB */
#endif /* VTSS_SW_OPTION_DOT1X */

#ifdef RFC4133_SUPPORTED_ENTITY
#if RFC4133_SUPPORTED_ENTITY
    init_entityMIB();
#endif /* RFC4133_SUPPORTED_ENTITY */
#endif /* RFC4133_SUPPORTED_ENTITY */

#ifdef RFC3636_SUPPORTED_MAU
#if RFC3636_SUPPORTED_MAU
    init_snmpDot3MauMgt();
#endif /* RFC3636_SUPPORTED_MAU */
#endif /* RFC3636_SUPPORTED_MAU */

#ifdef VTSS_SW_OPTION_SFLOW
    sflow_snmp_init();
#endif /* VTSS_SW_OPTION_SFLOW */

#if defined(VTSS_SW_OPTION_POE)
#ifdef VTSS_POE_SUPPORT_RFC3621_ENABLE
    init_powerEthernetMIB();
#endif /* VTSS_SW_OPTION_POE */
#endif /* VTSS_POE_SUPPORT_RFC3621_ENABLE */

#if defined(VTSS_SW_OPTION_IP2)
    init_ipForward();
    init_ip();
#endif /* VTSS_SW_OPTION_IP2 */

#ifdef SNMP_SUPPORT_V3
    init_vacm_vars();
    init_snmpMPDStats();
#if RFC3411_SUPPORTED_FRAMEWORK_SNMPENGINE
    init_snmpEngine();
#endif /* RFC3411_SUPPORTED_FRAMEWORK_SNMPENGINE */
#endif /* SNMP_SUPPORT_V3 */


//SMB_MIBs
#ifdef VTSS_SW_OPTION_SMB_SNMP

#ifdef RFC2863_SUPPORTED_IFMIB
#if RFC2863_SUPPORTED_IFMIB
    init_ifMIB();
#endif /* RFC2863_SUPPORTED_IFMIB */
#endif /* RFC2863_SUPPORTED_IFMIB */

#ifdef VTSS_SW_OPTION_LLDP
#ifdef DOT1AB_LLDP
#if defined(VTSS_SW_OPTION_POE) || defined(VTSS_SW_OPTION_LLDP_MED)
    init_lldpXMedMIB();
#endif /* TSS_SW_OPTION_POE */
#endif /* DOT1AB_LLDP */
#endif /* VTSS_SW_OPTION_LLDP */

#ifdef VTSS_SW_OPTION_IGMPS
#ifdef RFC2933_SUPPORTED_IGMP
    init_igmpInterfaceTable();
#endif /* RFC2933_SUPPORTED_IGMP */
#endif /* VTSS_SW_OPTION_IGMPS */

#ifdef VTSS_SW_OPTION_IPMC
#ifdef RFC5519_SUPPORTED_MGMD
    init_mgmdMIBObjects();
#endif /* RFC5519_SUPPORTED_MGMD */
#endif /* VTSS_SW_OPTION_IPMC */

#ifdef RFC4668_SUPPORTED_RADIUS
#if RFC4668_SUPPORTED_RADIUS
    init_radiusAuthClientMIBObjects();
#endif /* RFC4668_SUPPORTED_RADIUS */
#endif /* RFC4668_SUPPORTED_RADIUS */

#ifdef VTSS_SW_OPTION_LACP
    init_lagMIBObjects();
#endif /* VTSS_SW_OPTION_LACP */

#ifdef RFC4670_SUPPORTED_RADIUS
#if RFC4670_SUPPORTED_RADIUS
    init_radiusAccClientMIBObjects();
#endif /* RFC4670_SUPPORTED_RADIUS */
#endif /* RFC4670_SUPPORTED_RADIUS */

#ifdef VTSS_SW_OPTION_MSTP
    ieee8021MstpMib_init();
#endif /* VTSS_SW_OPTION_MSTP */

#ifdef VTSS_SW_OPTION_ETH_LINK_OAM
#if RFC4878_SUPPORTED_ETH_LINK_OAM
    init_dot3OamMIB();
#endif /* VTSS_SW_OPTION_ETH_LINK_OAM */
#endif  /* RFC4878_SUPPORTED_ETH_LINK_OAM */

#ifdef SNMP_SUPPORT_V3
#if RFC3414_SUPPORTED_USMSTATS
    init_usmStats();
#endif /* RFC3414_SUPPORTED_USMSTATS */
#if RFC3414_SUPPORTED_USMUSER
    init_usmUser();
#endif /* RFC3414_SUPPORTED_USMUSER */
#if RFC3415_SUPPORTED_VACMCONTEXTTABLE
    init_vacmContextTable();
#endif /* RFC3415_SUPPORTED_VACMCONTEXTTABLE */
#if RFC3415_SUPPORTED_VACMSECURITYTOGROUPTABLE
    init_vacmSecurityToGroupTable();
#endif /* RFC3415_SUPPORTED_VACMSECURITYTOGROUPTABLE */
#if RFC3415_SUPPORTED_VACMACCESSTABLE
    init_vacmAccessTable();
#endif /* RFC3415_SUPPORTED_VACMACCESSTABLE */
#if RFC3415_SUPPORTED_VACMMIBVIEWS
    init_vacmMIBViews();
#endif /* RFC3415_SUPPORTED_VACMMIBVIEWS */
#endif /* SNMP_SUPPORT_V3 */

#endif /* VTSS_SW_OPTION_SMB_SNMP */
}

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/

