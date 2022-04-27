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

#ifndef _VTSS_SNMP_API_H_
#define _VTSS_SNMP_API_H_

#include "ip2_api.h"
#include "sysutil_api.h"    // For VTSS_SYS_HOSTNAME_LEN
#include <ucd-snmp/config.h>
#include <ucd-snmp/asn1.h>

#define SNMP_HAS_UCD_SNMP
//#define SNMP_HAS_NET_SNMP

/* Thread variables, if using trap API snmp_send_trap(),
   we should increase the stack size */
#define SNMP_TRAP_STACK_SIZE    16384 /* 24K for SNMPv3 trap */

#define VTSS_TRAP_CONF_MAX      4
#define VTSS_TRAP_CONF_ID_MIN   0
#define VTSS_TRAP_CONF_ID_MAX   (VTSS_TRAP_CONF_MAX - 1)


/* SNMP version control,
   defination is the same as in
   eCos\package\net\snmp\lib\current\include\snmp.h */
#define SNMP_SUPPORT_V1         0
#define SNMP_SUPPORT_V2C        1
/* define the variable if we want SNMP manager supported SNMPv3
#define SNMP_SUPPORT_V3         3 */
#define SNMP_SUPPORT_V3         3

#ifdef SNMP_SUPPORT_V3
#define RFC3414_SUPPORTED_USMSTATS       1
#define RFC3414_SUPPORTED_USMUSER        1
#else
#define RFC3414_SUPPORTED_USMSTATS       0
#define RFC3414_SUPPORTED_USMUSER        0
#endif /* SNMP_SUPPORT_V3 */

#ifdef SNMP_SUPPORT_V3
#define RFC3415_SUPPORTED_VACMCONTEXTTABLE              1
#define RFC3415_SUPPORTED_VACMSECURITYTOGROUPTABLE      1
#define RFC3415_SUPPORTED_VACMACCESSTABLE               1
#define RFC3415_SUPPORTED_VACMMIBVIEWS                  1
#else
#define RFC3415_SUPPORTED_VACMCONTEXTTABLE              0
#define RFC3415_SUPPORTED_VACMSECURITYTOGROUPTABLE      0
#define RFC3415_SUPPORTED_VACMACCESSTABLE               0
#define RFC3415_SUPPORTED_VACMMIBVIEWS                  0
#endif /* SNMP_SUPPORT_V3 */

#define VTSS_TRAP_MSG_TRAP2     0
#define VTSS_TRAP_MSG_INFORM    1
/* SNMP managent switch enable/disable */
#define SNMP_MGMT_ENABLED       1
#define SNMP_MGMT_DISABLED      0

/* SNMP max OID length,
   defination is the same as in
   eCos\package\net\snmp\lib\current\include\ans1.h */
#define SNMP_MGMT_MAX_OID_LEN               128

#define SNMP_MGMT_MAX_SUBTREE_LEN           16

/* SNMP max name length,
   defination is the same as in
   eCos\package\net\snmp\lib\current\include\ans1.h */
#define SNMP_MGMT_MAX_NAME_LEN              128

/* SNMP max community length,
   defination is the same as in
   eCos\package\net\snmp\lib\current\include\snmp_impl.h */
#define SNMP_MGMT_MAX_COMMUNITY_LEN         256
#define SNMP_MGMT_INPUT_COMMUNITY_LEN       255
#define TRAP_MAX_NAME_LEN                   32
#define TRAP_MIN_NAME_LEN                   1

/* SNMP max trap inform retry times */
#define SNMP_MGMT_MAX_TRAP_INFORM_TIMEOUT   2147 //0x7FFFFFFF usec

/* SNMP max trap inform retry times */
#define SNMP_MGMT_MAX_TRAP_INFORM_RETRIES   255

/* row storage values,
   defination is the same as in
   eCos\package\net\snmp\lib\current\include\snmp.h */
#define SNMP_MGMT_STORAGE_OTHER         1
#define SNMP_MGMT_STORAGE_VOLATILE      2
#define SNMP_MGMT_STORAGE_NONVOLATILE   3
#define SNMP_MGMT_STORAGE_PERMANENT     4
#define SNMP_MGMT_STORAGE_READONLY      5

/* row status values,
   defination is the same as in
   eCos\package\net\snmp\lib\current\include\snmp.h */
#define SNMP_MGMT_ROW_NONEXISTENT       0
#define SNMP_MGMT_ROW_ACTIVE            1
#define SNMP_MGMT_ROW_NOTINSERVICE      2
#define SNMP_MGMT_ROW_NOTREADY          3
#define SNMP_MGMT_ROW_CREATEANDGO       4
#define SNMP_MGMT_ROW_CREATEANDWAIT     5
#define SNMP_MGMT_ROW_DESTROY           6

#ifdef SNMP_SUPPORT_V3
/* SNMPv3 max engine ID length */
#define SNMPV3_MIN_ENGINE_ID_LEN                5
#define SNMPV3_MAX_ENGINE_ID_LEN                32

/* SNMPv3 max name length */
#define SNMPV3_MIN_NAME_LEN                     1
#define SNMPV3_MAX_NAME_LEN                     32

/* SNMPv3 max password length */
#define SNMPV3_MIN_PASSWORD_LEN                 8
#define SNMPV3_MAX_MD5_PASSWORD_LEN             32
#define SNMPV3_MAX_SHA_PASSWORD_LEN             40
#define SNMPV3_MAX_DES_PASSWORD_LEN             32
#endif /* SNMP_SUPPORT_V3 */

#define SNMPV3_MIN_EVENT_DESC_LEN           1
#define SNMPV3_MAX_EVENT_DESC_NAME_LEN      127

#define SNMP_DEFAULT_MODE                           SNMP_MGMT_ENABLED
#define SNMP_DEFAULT_VER                            SNMP_SUPPORT_V2C
#define SNMP_DEFAULT_RO_COMMUNITY                   "public"
#define SNMP_DEFAULT_RW_COMMUNITY                   "private"

#ifdef SNMP_SUPPORT_V3
/* SNMPv3 default configuration */
#define SNMPV3_DEFAULT_ENGINE_ID                {0x80,0x00,0x07,0xe5,0x01,0x7f,0x00,0x00,0x01}
#define SNMPV3_DEFAULT_COMMUNITY_SIP            0x0
#define SNMPV3_DEFAULT_COMMUNITY_SIP_MASK       0x0
#define SNMPV3_DEFAULT_USER                     "default_user"
#define SNMPV3_DEFAULT_RO_GROUP                 "default_ro_group"
#define SNMPV3_DEFAULT_RW_GROUP                 "default_rw_group"
#define SNMPV3_DEFAULT_VIEW                     "default_view"
#endif /* SNMP_SUPPORT_V3 */

#define TRAP_CONF_DEFAULT_ENABLE            FALSE
#define TRAP_CONF_DEFAULT_DPORT             162
#define TRAP_CONF_DEFAULT_DIP               ""
#define TRAP_CONF_DEFAULT_VER               SNMP_SUPPORT_V2C
#define TRAP_CONF_DEFAULT_COMM              "Public"
#define TRAP_CONF_DEFAULT_INFORM_MODE       VTSS_TRAP_MSG_TRAP2
#define TRAP_CONF_DEFAULT_INFORM_RETRIES    5
#define TRAP_CONF_DEFAULT_INFORM_TIMEOUT    3
#define TRAP_CONF_DEFAULT_PROBE_ENG         TRUE
#define TRAP_CONF_DEFAULT_ENG_ID            ""
#define TRAP_CONF_DEFAULT_ENG_ID_LEN        0
#define TRAP_CONF_DEFAULT_SEC_NAME          ""

#define TRAP_EVENT_DEFAULT_WARM_START       FALSE
#define TRAP_EVENT_DEFAULT_COLD_START       FALSE
#define TRAP_EVENT_DEFAULT_LINK_UP          FALSE
#define TRAP_EVENT_DEFAULT_LINK_DOWN        FALSE
#define TRAP_EVENT_DEFAULT_LLDP             FALSE
#define TRAP_EVENT_DEFAULT_AUTH_FAIL        FALSE
#define TRAP_EVENT_DEFAULT_STP              FALSE
#define TRAP_EVENT_DEFAULT_RMON             FALSE


/* SNMP error codes (vtss_rc) */
typedef enum {
    SNMP_ERROR_GEN = MODULE_ERROR_START(VTSS_MODULE_ID_SNMP),  /* Generic error code */
    SNMP_ERROR_PARM,                        /* Illegal parameter */
    SNMP_ERROR_STACK_STATE,                 /* Illegal MASTER/SLAVE state */
    SNMP_ERROR_ENGINE_FAIL,                 /* SNMP engine occur fail */
    SNMP_ERROR_ROW_STATUS_INCONSISTENT,     /* Illegal operation inconsistent with row status */
    SNMP_ERROR_SMON_STAT_TABLE_FULL,
#ifdef SNMP_SUPPORT_V3
    SNMPV3_ERROR_COMMUNITIES_TABLE_FULL,    /* SNMPv3 users table full */
    SNMPV3_ERROR_USERS_TABLE_FULL,          /* SNMPv3 users table full */
    SNMPV3_ERROR_GROUPS_TABLE_FULL,         /* SNMPv3 groups table full */
    SNMPV3_ERROR_VIEWS_TABLE_FULL,          /* SNMPv3 views table full */
    SNMPV3_ERROR_ACCESSES_TABLE_FULL,       /* SNMPv3 accesses table full */
#endif /* SNMP_SUPPORT_V3 */
} snmp_error_t;


/* SNMP configuration */
typedef struct {
    ulong       mode;
    ulong       version;
    ushort      port;
    char        read_community[SNMP_MGMT_MAX_COMMUNITY_LEN];
    char        write_community[SNMP_MGMT_MAX_COMMUNITY_LEN];
    ulong       trap_mode;
    ulong       trap_version;
    ushort      trap_port;
    char        trap_community[SNMP_MGMT_MAX_COMMUNITY_LEN];
    char        trap_dip_string[VTSS_SYS_HOSTNAME_LEN];
#ifdef VTSS_SW_OPTION_IPV6
    vtss_ipv6_t trap_dipv6;
#endif /* VTSS_SW_OPTION_IPV6 */
    ulong       trap_authen_fail;
    ulong       trap_linkup_linkdown;
    ulong       trap_inform_mode;
    ulong       trap_inform_timeout;
    ulong       trap_inform_retries;
#ifdef SNMP_SUPPORT_V3
    ulong       trap_probe_security_engineid;
    uchar       trap_security_engineid[SNMPV3_MAX_ENGINE_ID_LEN];
    ulong       trap_security_engineid_len;
    char        trap_security_name[SNMPV3_MAX_NAME_LEN + 1];
    uchar       engineid[SNMPV3_MAX_ENGINE_ID_LEN];
    ulong       engineid_len;
#endif /* SNMP_SUPPORT_V3 */
} snmp_conf_t;

/* SNMP port configuration */
typedef struct {
    BOOL  linkupdown_trap_enable;    /* linkUp/linkDown trap enable */
} snmp_port_conf_t;


/* SNMP statistics row entry */
typedef struct {
    BOOL  valid;
    ulong ctrl_index;
    ulong if_index;
} snmp_rmon_stat_entry_t;
/* SNMP statistics row entry */
typedef struct {
    BOOL  valid;
    ulong source_index;
    ulong dest_index;
    ulong ctrl_index;
    ulong copydirection;
} snmp_port_copy_entry_t;

/* SNMP vars trap entry,
   Note: the struct only suit to SNMP/eCos module */
typedef struct {
    ulong oid[SNMP_MGMT_MAX_OID_LEN];
    ulong oid_len;
    struct variable_list *vars;
} snmp_vars_trap_entry_t;

typedef struct {
    BOOL ipv6_flag;
    union {
        char ipv4_str[VTSS_SYS_HOSTNAME_LEN + 1];
#ifdef VTSS_SW_OPTION_IPV6
        vtss_ipv6_t ipv6;
#endif /* VTSS_SW_OPTION_IPV6 */
    } addr;
} vtss_trap_addr_t;

typedef struct {
    i32        conf_id;
    BOOL       enable;
    i8         trap_src_host_name [VTSS_SYS_HOSTNAME_LEN + 1];

    vtss_trap_addr_t dip;
    u16        trap_port;
    u32        trap_version;
    i8         trap_community [SNMP_MGMT_MAX_COMMUNITY_LEN + 1];
    u32        trap_inform_mode;
    u32        trap_inform_timeout;
    u32        trap_inform_retries;
#ifdef SNMP_SUPPORT_V3
    BOOL       trap_probe_engineid;
    u8         trap_engineid[SNMPV3_MAX_ENGINE_ID_LEN];
    u32        trap_engineid_len;
    i8         trap_security_name[SNMPV3_MAX_NAME_LEN + 1];
#endif /* SNMP_SUPPORT_V3 */
} vtss_trap_conf_t;

/* SNMP event row entry */
typedef struct {
    struct system_s {
        BOOL       warm_start;
        BOOL       cold_start;
    } system;
    struct interface_s {
        BOOL       trap_linkup[VTSS_ISID_END] [VTSS_PORTS];
        BOOL       trap_linkdown [VTSS_ISID_END] [VTSS_PORTS];
        BOOL       trap_lldp[VTSS_ISID_END][VTSS_PORTS];
    } interface;
    struct aaa_s {
        BOOL       trap_authen_fail;
    } aaa;
    struct switch_s {
        BOOL       stp;
        BOOL       rmon;
    } sw;
} vtss_trap_event_t;

/* SNMP event trap entry */
typedef struct {
    /* Index */
    i8                  trap_conf_name [TRAP_MAX_NAME_LEN + 1];
    vtss_trap_conf_t    trap_conf;
    vtss_trap_event_t   trap_event;
    BOOL                valid;
} vtss_trap_entry_t;

typedef struct {
    BOOL trap_mode;
    vtss_trap_entry_t trap_entry[VTSS_TRAP_CONF_MAX];
} vtss_trap_sys_conf_t;

/* SNMP error text */
char *snmp_error_txt(snmp_error_t rc);

/* Get SNMP configuration */
vtss_rc snmp_mgmt_snmp_conf_get(snmp_conf_t *conf);

/* Set SNMP configuration */
vtss_rc snmp_mgmt_snmp_conf_set(snmp_conf_t *conf);

/* Get SNMP defaults */
void snmp_default_get(snmp_conf_t *conf);

/* Determine if SNMP configuration has changed */
int snmp_conf_changed(snmp_conf_t *old, snmp_conf_t *_new);

/* Get SNMP port configuration */
vtss_rc snmp_mgmt_snmp_port_conf_get(vtss_isid_t isid,
                                     vtss_port_no_t port_no,
                                     snmp_port_conf_t *conf);

/* Set SNMP port configuration */
vtss_rc snmp_mgmt_snmp_port_conf_set(vtss_isid_t isid,
                                     vtss_port_no_t port_no,
                                     snmp_port_conf_t *conf);

vtss_rc snmp_mgmt_smon_stat_entry_get(snmp_rmon_stat_entry_t *entry, BOOL next);
vtss_rc snmp_mgmt_port_copy_entry_get(snmp_port_copy_entry_t *entry, BOOL next);
vtss_rc snmp_mgmt_smon_stat_entry_set(snmp_rmon_stat_entry_t *entry);
vtss_rc snmp_mgmt_port_copy_entry_set(snmp_port_copy_entry_t *entry);

struct variable_list *
snmp_bind_var(struct variable_list *prev,
              void *value, int type, size_t sz_val, oid *oidVar, size_t sz_oid);


/* Send SNMP vars trap,
   Note: this function will free vars memory automatically */
void snmp_send_vars_trap(int specific, snmp_vars_trap_entry_t *entry);

/* check row status */
vtss_rc snmp_row_status_check(ulong action, ulong old_state, ulong *new_state);

/* Initialize module */
vtss_rc snmp_init(vtss_init_data_t *data);

#ifdef SNMP_SUPPORT_V3
/* SNMPv3 max password length */
#define SNMPV3_MGMT_CONTEX_MATCH_EXACT          1
#define SNMPV3_MGMT_CONTEX_MATCH_PREFIX         2

/* SNMPv3 none group view */
#define SNMPV3_NONAME                           "None"

/* SNMPv3 security model,
   defination is the same as in
   eCos\package\net\snmp\lib\current\include\snmp.h */
#define SNMP_MGMT_SEC_MODEL_ANY                 0
#define SNMP_MGMT_SEC_MODEL_SNMPV1              1
#define SNMP_MGMT_SEC_MODEL_SNMPV2C             2
#define SNMP_MGMT_SEC_MODEL_USM                 3

/* SNMPv3 security level,
   defination is the same as in
   eCos\package\net\snmp\lib\current\include\snmp.h */
#define SNMP_MGMT_SEC_LEVEL_NOAUTH              1
#define SNMP_MGMT_SEC_LEVEL_AUTHNOPRIV          2
#define SNMP_MGMT_SEC_LEVEL_AUTHPRIV            3

/* SNMPv3 views type,
   defination is the same as in
   eCos\package\net\snmp\lib\current\include\snmp.h */
#define SNMPV3_MGMT_VIEW_INCLUDED               1
#define SNMPV3_MGMT_VIEW_EXCLUDED               2

/* SNMPv3 max table size */
#define SNMPV3_MAX_COMMUNITIES                  16
#define SNMPV3_MAX_USERS                        32
#define SNMPV3_MAX_GROUPS                       (SNMPV3_MAX_COMMUNITIES * 2 + SNMPV3_MAX_USERS)
#define SNMPV3_MAX_VIEWS                        32
#define SNMPV3_MAX_ACCESSES                     32

/* SNMPv3 configuration access keys */
#define SNMPV3_CONF_ACESS_GETFIRST              ""

/* SNMPv3 authencation protocol */
#define SNMP_MGMT_AUTH_PROTO_NONE   0x0
#define SNMP_MGMT_AUTH_PROTO_MD5    0x1
#define SNMP_MGMT_AUTH_PROTO_SHA    0x2

/* SNMPv3 privacy protocol */
#define SNMP_MGMT_PRIV_PROTO_NONE   0x0
#define SNMP_MGMT_PRIV_PROTO_DES    0x1
#define SNMP_MGMT_PRIV_PROTO_AES    0x2

/* SNMPv3 communities entry */
typedef struct {
    ulong       idx;
    BOOL        valid;
    char        community[SNMPV3_MAX_NAME_LEN + 1];   /* key */
    vtss_ipv4_t sip;
    vtss_ipv4_t sip_mask;
    ulong       storage_type;
    ulong       status;
} snmpv3_communities_conf_t;

/* SNMPv3 users entry */
typedef struct {
    ulong   idx;
    BOOL    valid;
    uchar   engineid[SNMPV3_MAX_ENGINE_ID_LEN];   /* key */
    ulong   engineid_len;                         /* key */
    char    user_name[SNMPV3_MAX_NAME_LEN + 1];   /* key */
    ulong   security_level;
    ulong   auth_protocol;
    char    auth_password[SNMPV3_MAX_SHA_PASSWORD_LEN + 1];
    ulong   priv_protocol;
    char    priv_password[SNMPV3_MAX_DES_PASSWORD_LEN + 1];
    ulong   storage_type;
    ulong   status;
} snmpv3_users_conf_t;

/* SNMPv3 groups entry */
typedef struct {
    ulong   idx;
    BOOL    valid;
    ulong   security_model;                            /* key */
    char    security_name[SNMPV3_MAX_NAME_LEN + 1];    /* key */
    char    group_name[SNMPV3_MAX_NAME_LEN + 1];
    ulong   storage_type;
    ulong   status;
} snmpv3_groups_conf_t;

/* SNMPv3 views entry */
typedef struct {
    ulong   idx;
    BOOL    valid;
    char    view_name[SNMPV3_MAX_NAME_LEN + 1];   /* key */
    ulong   subtree[SNMP_MGMT_MAX_OID_LEN + 1];   /* key */
    ulong   subtree_len;                          /* key */
    uchar   subtree_mask[SNMP_MGMT_MAX_SUBTREE_LEN];
    ulong   subtree_mask_len;
    ulong   view_type;
    ulong   storage_type;
    ulong   status;
} snmpv3_views_conf_t;

/* SNMPv3 accesses entry */
typedef struct {
    ulong   idx;
    BOOL    valid;
    char    group_name[SNMPV3_MAX_NAME_LEN + 1];       /* key */
    char    context_prefix[SNMPV3_MAX_NAME_LEN + 1];   /* key */
    ulong   security_model;                            /* key */
    ulong   security_level;                            /* key */
    ulong   context_match;
    char    read_view_name[SNMPV3_MAX_NAME_LEN + 1];
    char    write_view_name[SNMPV3_MAX_NAME_LEN + 1];
    char    notify_view_name[SNMPV3_MAX_NAME_LEN + 1];
    ulong   storage_type;
    ulong   status;
} snmpv3_accesses_conf_t;

/* check is valod engine ID */
BOOL snmpv3_is_valid_engineid(uchar *engineid, ulong engineid_len);

/* check is SNMP admin string format */
BOOL snmpv3_is_admin_string(const char *str);

int snmpv3_communities_conf_changed(snmpv3_communities_conf_t *old, snmpv3_communities_conf_t *new_conf);

/* check SNMPv3 communities configuration */
vtss_rc snmpv3_mgmt_communities_conf_check(snmpv3_communities_conf_t *conf);

/* Get SNMPv3 communities configuration,
fill community = SNMPV3_CONF_ACESS_GETFIRST will get first entry */
vtss_rc snmpv3_mgmt_communities_conf_get(snmpv3_communities_conf_t *conf, BOOL next);

/* Set SNMPv3 communities configuration,
fill valid = 0 or status = SNMP_MGMT_ROW_DESTROY will destroy the entry */
vtss_rc snmpv3_mgmt_communities_conf_set(snmpv3_communities_conf_t *conf);

/* Delete SNMPv3 communities entry */
vtss_rc snmpv3_mgmt_communities_conf_del(ulong idx);

void snmpv3_default_communities_get(ulong *conf_num, snmpv3_communities_conf_t *conf);

/* check SNMPv3 users configuration */
vtss_rc snmpv3_mgmt_users_conf_check(snmpv3_users_conf_t *conf);

int snmpv3_users_conf_changed(snmpv3_users_conf_t *old, snmpv3_users_conf_t *new_conf);

/* Get SNMPv3 users configuration,
fill user_name = SNMPV3_CONF_ACESS_GETFIRST will get first entry */
vtss_rc snmpv3_mgmt_users_conf_get(snmpv3_users_conf_t *conf, BOOL next);

void snmpv3_default_users_get(ulong *conf_num, snmpv3_users_conf_t *conf);

/**
  * \brief Get next user table by Key.
  *
  */
vtss_rc snmpv3_mgmt_users_conf_get_next(snmpv3_users_conf_t *conf);
/* Set SNMPv3 users configuration,
fill valid = 0 or status = SNMP_MGMT_ROW_DESTROY will destroy the entry */
vtss_rc snmpv3_mgmt_users_conf_set(snmpv3_users_conf_t *conf);

/* Save the SNMPv3 users into flash */
vtss_rc snmpv3_mgmt_users_conf_save(void);

/* Delete SNMPv3 users entry */
vtss_rc snmpv3_mgmt_users_conf_del(ulong idx);

/* check SNMPv3 groups configuration */
vtss_rc snmpv3_mgmt_groups_conf_check(snmpv3_groups_conf_t *conf);

int snmpv3_groups_conf_changed(snmpv3_groups_conf_t *old, snmpv3_groups_conf_t *new_conf);

/* Get SNMPv3 groups configuration,
fill security_name = SNMPV3_CONF_ACESS_GETFIRST will get first entry */
vtss_rc snmpv3_mgmt_groups_conf_get(snmpv3_groups_conf_t *conf, BOOL next);

/* Get SNMPv3 groups defaults */
void snmpv3_default_groups_get(ulong *conf_num, snmpv3_groups_conf_t *conf);

/**
  * \brief Get next group table by Key.
  *
  */
vtss_rc snmpv3_mgmt_groups_conf_get_next(snmpv3_groups_conf_t *conf);

/* Set SNMPv3 groups configuration,
fill valid = 0 or status = SNMP_MGMT_ROW_DESTROY will destroy the entry */
vtss_rc snmpv3_mgmt_groups_conf_set(snmpv3_groups_conf_t *conf);

/* Delete SNMPv3 groups entry */
vtss_rc snmpv3_mgmt_groups_conf_del(ulong idx);

int snmpv3_views_conf_changed(snmpv3_views_conf_t *old, snmpv3_views_conf_t *new_conf);
/* check SNMPv3 views configuration */
vtss_rc snmpv3_mgmt_views_conf_check(snmpv3_views_conf_t *conf);

/* Get SNMPv3 views configuration,
fill view_name = SNMPV3_CONF_ACESS_GETFIRST will get first entry */
vtss_rc snmpv3_mgmt_views_conf_get(snmpv3_views_conf_t *conf, BOOL next);

vtss_rc snmpv3_mgmt_views_conf_get_next(snmpv3_views_conf_t *conf);

/* Set SNMPv3 views configuration,
fill valid = 0 or status = SNMP_MGMT_ROW_DESTROY will destroy the entry */
vtss_rc snmpv3_mgmt_views_conf_set(snmpv3_views_conf_t *conf);

/* Delete SNMPv3 views entry */
vtss_rc snmpv3_mgmt_views_conf_del(ulong idx);

void snmpv3_default_views_get(ulong *conf_num, snmpv3_views_conf_t *conf);

int snmpv3_accesses_conf_changed(snmpv3_accesses_conf_t *old, snmpv3_accesses_conf_t *new_conf);
/* check SNMPv3 accesses configuration */
vtss_rc snmpv3_mgmt_accesses_conf_check(snmpv3_accesses_conf_t *conf);

/* Get SNMPv3 accesses configuration,
fill group_name = SNMPV3_CONF_ACESS_GETFIRST will get first entry */
vtss_rc snmpv3_mgmt_accesses_conf_get(snmpv3_accesses_conf_t *conf, BOOL next);

vtss_rc snmpv3_mgmt_accesses_conf_get_next(snmpv3_accesses_conf_t *conf);

/* Set SNMPv3 accesses configuration,
fill valid = 0 or status = SNMP_MGMT_ROW_DESTROY will destroy the entry */
vtss_rc snmpv3_mgmt_accesses_conf_set(snmpv3_accesses_conf_t *conf);

/* Delete SNMPv3 accesses entry */
vtss_rc snmpv3_mgmt_accesses_conf_del(ulong idx);

void snmpv3_default_accesses_get(ulong *conf_num, snmpv3_accesses_conf_t *conf);

/* Post configure if using NTP */
void snmpv3_mgmt_ntp_post_conf(void);
#endif /* SNMP_SUPPORT_V3 */

/**
  * \brief Get trap mode configuration
  *
  * \param global_enable   [OUT]: the global configuration of trap configuration
  * \return
  *    Always VTSS_RC_OK\n
  */

vtss_rc trap_mgmt_mode_get(BOOL *enable);

/**
  * \brief Set trap mode configuration
  *
  * \param global_enable   [IN]: the global configuration of trap configuration
  * \return
  *    Always VTSS_RC_OK\n
  */

vtss_rc trap_mgmt_mode_set(BOOL enable);

/**
  * \brief Get trap configuration entry
  *
  * \param trap_entry   [IN] trap_conf_name: Name of the trap configuration
  * \return
  *    VTSS_RC_OK indicates it is successful, otherwise is failed.
  */

vtss_rc trap_mgmt_conf_get (vtss_trap_entry_t  *trap_entry);
/**
  * \brief Get next trap configuration entry
  *
  * \param trap_entry   [INOUT] trap_conf_name: Name of the trap configuration
  * \return
  *    VTSS_RC_OK indicates it is successful, otherwise is failed.
  */

vtss_rc trap_mgmt_conf_get_next (vtss_trap_entry_t  *trap_entry);

/**
  * \brief Set trap configuration entry
  *
  * \param trap_entry   [IN] : The trap configuration
  * \return
  *    VTSS_RC_OK indicates it is successful, otherwise is failed.
  */

vtss_rc trap_mgmt_conf_set (vtss_trap_entry_t  *trap_entry);

void trap_mgmt_sys_default_get(vtss_trap_sys_conf_t *trap_conf);

/**
  * \brief Get trap default configuration entry
  *
  * \param trap_entry   [OUT] : The trap configuration
  */

void trap_mgmt_conf_default_get(vtss_trap_entry_t  *trap_entry);

/**
  * \brief Send SNMP vars trap
  *
  * \param specific      [IN]: 0 indicates that specific trap, 1 ~ 5 indicates generic trap, -1 indicates v2 version.
  * \param entry         [IN]: the event OID and variable binding
  *
 */

void snmp_send_vars_trap(int specific, snmp_vars_trap_entry_t *entry);


#endif /* _VTSS_SNMP_API_H_ */


// ***************************************************************************
//
//  End of file.
//
// ***************************************************************************

