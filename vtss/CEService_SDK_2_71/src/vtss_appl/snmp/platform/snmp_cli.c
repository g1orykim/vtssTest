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

 $Id$
 $Revision$

*/

#include "main.h"
#include "cli.h"
#include "cli_grp_help.h"
#include "snmp_cli.h"
#include "cli_trace_def.h"
#include "vtss_snmp_api.h"

typedef struct {
    u32 version;
#ifdef SNMP_SUPPORT_V3
    u8 engineid[SNMPV3_MAX_ENGINE_ID_LEN];
    u32 engineid_len;
    i8 admin_string[SNMPV3_MAX_NAME_LEN + 1];
    u32 auth_protocol;
    i8 auth_password[SNMPV3_MAX_SHA_PASSWORD_LEN + 1];
    u32 priv_protocol;
    i8 priv_password[SNMPV3_MAX_DES_PASSWORD_LEN + 1];
    u32 security_model;
    u32 security_level;
    i8 security_name[SNMPV3_MAX_NAME_LEN + 1];
    u32 oid[SNMP_MGMT_MAX_OID_LEN + 1];
    u32 oid_len;
    u8 oid_mask[SNMP_MGMT_MAX_SUBTREE_LEN];
    u32 oid_mask_len;
    u32 view_type;
    i8 read_view_name[SNMPV3_MAX_NAME_LEN + 1];
    i8 write_view_name[SNMPV3_MAX_NAME_LEN + 1];
    i8 notify_view_name[SNMPV3_MAX_NAME_LEN + 1];
    i8 username[VTSS_SYS_USERNAME_LEN];
#endif /* SNMP_SUPPORT_V3 */
    u32 entry_idx;

    i8 trap_name[TRAP_MAX_NAME_LEN + 1];
    BOOL enable_set;
    BOOL dip_set;
    BOOL dport_set;
    BOOL version_set;
    BOOL community_set;
    BOOL security_set;
    BOOL inform_set;
    u32  trap_type;
    BOOL retries_set;
    BOOL timeout_set;
    BOOL probe_set;
    BOOL probe;
    BOOL engineid_set;
    BOOL security_name_set;
    BOOL warm_start;
    BOOL cold_start;
    BOOL linkup;
    BOOL linkdown;
    BOOL lldp;
    BOOL auth_fail;
    BOOL rmon;
    BOOL stp;

    /* Keywords */
    BOOL read;
    BOOL write;
    u32  trap_port;
    u32  trap_inform_timeout;
    u32  trap_inform_retries;

} snmp_cli_req_t;

/****************************************************************************/
/*  Initialization                                                          */
/****************************************************************************/

void snmp_cli_init(void)
{
    /* register the size required for snmp req. structure */
    cli_req_size_register(sizeof(snmp_cli_req_t));
}

/****************************************************************************/
/*  Command functions                                                       */
/****************************************************************************/

/* SNMP configuration */
static void SNMP_cli_cmd_conf(cli_req_t *req,
                              BOOL mode, BOOL version, BOOL read_comm, BOOL write_comm)
{
    vtss_rc             rc;
    snmp_conf_t         conf;
    snmp_cli_req_t *snmp_req = req->module_req;

    if (cli_cmd_switch_none(req) ||
        cli_cmd_conf_slave(req) ||
        snmp_mgmt_snmp_conf_get(&conf) != VTSS_OK) {
        return;
    }

    if (req->set) {
        if (mode) {
            conf.mode = req->enable;
        }
        if (version) {
            conf.version = snmp_req->version;
        }
        if (read_comm) {
            strcpy(conf.read_community, req->parm);
        }
        if (write_comm) {
            strcpy(conf.write_community, req->parm);
        }
        if ((rc = snmp_mgmt_snmp_conf_set(&conf)) != VTSS_OK) {
            CPRINTF("%s\n", error_txt(rc));
        }
    } else {
        if (mode) {
            CPRINTF("SNMP Mode                     : %s\n", cli_bool_txt(conf.mode));
        }
        if (version) {
            CPRINTF("SNMP Version                  : %s\n", conf.version == SNMP_SUPPORT_V1 ? "1" : conf.version == SNMP_SUPPORT_V2C ? "2c" : "3");
        }
        if (read_comm) {
            CPRINTF("Read Community                : %s\n", conf.read_community);
        }
        if (write_comm) {
            CPRINTF("Write Community               : %s\n", conf.write_community);
        }
    }
}
//}

static void SNMP_cli_cmd_conf_mode(cli_req_t *req)
{
#ifdef VTSS_SW_OPTION_IPV6
    SNMP_cli_cmd_conf(req, 1, 0, 0, 0);
#else
    SNMP_cli_cmd_conf(req, 1, 0, 0, 0);
#endif /*VTSS_SW_OPTION_IPV6*/
}

static void SNMP_cli_cmd_conf_ver(cli_req_t *req)
{
#ifdef VTSS_SW_OPTION_IPV6
    SNMP_cli_cmd_conf(req, 0, 1, 0, 0);
#else
    SNMP_cli_cmd_conf(req, 0, 1, 0, 0);
#endif /*VTSS_SW_OPTION_IPV6*/
}

static void SNMP_cli_cmd_conf_read_comm(cli_req_t *req)
{
#ifdef VTSS_SW_OPTION_IPV6
    SNMP_cli_cmd_conf(req, 0, 0, 1, 0);
#else
    SNMP_cli_cmd_conf(req, 0, 0, 1, 0);
#endif /*VTSS_SW_OPTION_IPV6*/
}

static void SNMP_cli_cmd_conf_write_comm(cli_req_t *req)
{
#ifdef VTSS_SW_OPTION_IPV6
    SNMP_cli_cmd_conf(req, 0, 0, 0, 1);
#else
    SNMP_cli_cmd_conf(req, 0, 0, 0, 1);
#endif /*VTSS_SW_OPTION_IPV6*/
}

#ifdef SNMP_SUPPORT_V3
/* SNMPv3 engine ID configuration */
static void SNMPV3_cli_cmd_engineid_conf(cli_req_t *req)
{
    vtss_rc         rc;
    snmp_conf_t     conf;
    snmp_cli_req_t  *snmp_req = req->module_req;

    if (cli_cmd_switch_none(req) ||
        cli_cmd_conf_slave(req) ||
        snmp_mgmt_snmp_conf_get(&conf) != VTSS_OK) {
        return;
    }

    if (req->set) {
        memcpy(conf.engineid, snmp_req->engineid, snmp_req->engineid_len);
        conf.engineid_len = snmp_req->engineid_len;
        if ((rc = snmp_mgmt_snmp_conf_set(&conf)) != VTSS_OK) {
            CPRINTF("%s\n", error_txt(rc));
        }
        CPRINTF("Change Engine ID will clear all original local users\n");
    } else {
        CPRINTF("SNMPv3 Engine ID : %s\n", misc_engineid2str(conf.engineid, conf.engineid_len));
    }
}


static void SNMPV3_cli_cmd_community_add(cli_req_t *req)
{
    snmpv3_communities_conf_t conf;
    snmp_cli_req_t *snmp_req = req->module_req;

    if (req->set) {
        memset(&conf, 0x0, sizeof(conf));
        strcpy(conf.community, snmp_req->admin_string);
        conf.valid = 1;
        conf.sip = req->ipv4_addr;
        if (req->ipv4_mask_spec == CLI_SPEC_VAL) {
            conf.sip_mask = req->ipv4_mask;
        } else if (req->ipv4_addr_spec == CLI_SPEC_VAL) {
            conf.sip_mask = 0xFFFFFF00;
        }
        conf.storage_type = SNMP_MGMT_STORAGE_PERMANENT;
        conf.status = SNMP_MGMT_ROW_ACTIVE;
        if (conf.sip_mask == 0xFFFFFFFF) {
            CPRINTF("Parameter <ip_mask> doesn't allow 255.255.255.255''");
            return;
        }
        if (snmpv3_mgmt_communities_conf_set(&conf) != VTSS_OK) {
            CPRINTF("snmpv3_mgmt_communities_conf_set(%s): failed\n",
                    conf.community);
        }
    }
}

static void SNMPV3_cli_cmd_community_del(cli_req_t *req)
{
    vtss_rc                     rc;
    snmp_cli_req_t              *snmp_req = req->module_req;
    snmpv3_communities_conf_t   conf;

    if (req->set) {
        strcpy(conf.community, SNMPV3_CONF_ACESS_GETFIRST);
        while (snmpv3_mgmt_communities_conf_get(&conf, TRUE) == VTSS_OK) {
            if (conf.status != SNMP_MGMT_ROW_ACTIVE) {
                continue;
            }
            if (conf.idx != snmp_req->entry_idx || !conf.valid) {
                continue;
            } else {
                if ((rc = snmpv3_mgmt_communities_conf_del(snmp_req->entry_idx)) != VTSS_OK) {
                    CPRINTF("%s\n", error_txt(rc));
                }
                return;
            }
        }
        CPRINTF("Non-existing entry\n");
    }
}

static void SNMPV3_cli_cmd_community_lookup(cli_req_t *req)
{
    snmpv3_communities_conf_t conf;
    char                      ip_buf[16];
    int                       cnt = 0;
    snmp_cli_req_t *snmp_req = req->module_req;

    strcpy(conf.community, SNMPV3_CONF_ACESS_GETFIRST);
    while (snmpv3_mgmt_communities_conf_get(&conf, TRUE) == VTSS_OK) {
        if (conf.status != SNMP_MGMT_ROW_ACTIVE) {
            continue;
        }
        if (req->set) {
            if (conf.idx != snmp_req->entry_idx || !conf.valid) {
                continue;
            } else {
                CPRINTF("Entry Index : %d\n", conf.idx);
                CPRINTF("Community   : %s\n", conf.community);
                CPRINTF("Source IP   : %s\n", misc_ipv4_txt(conf.sip, ip_buf));
                CPRINTF("Source Mask : %s\n", misc_ipv4_txt(conf.sip_mask, ip_buf));
                return;
            }
        }
        if (++cnt == 1) {
            CPRINTF("Idx Community                        Source IP        Source Mask\n");
            CPRINTF("--- -------------------------------- --------------- ---------------\n");
        }
        CPRINTF("%-3d %-32s %-15s ", conf.idx, conf.community, misc_ipv4_txt(conf.sip, ip_buf));
        CPRINTF("%-15s\n", misc_ipv4_txt(conf.sip_mask, ip_buf));
    }
    if (cnt) {
        CPRINTF("\nNumber of entries: %d\n", cnt);
    } else if (req->set) {
        CPRINTF("Non-existing entry\n");
    }
}

static void SNMPV3_cli_cmd_user_add(cli_req_t *req)
{
    snmpv3_users_conf_t conf;
    snmp_cli_req_t *snmp_req = req->module_req;

    if (req->set) {
        memset(&conf, 0x0, sizeof(conf));
        memcpy(conf.engineid, snmp_req->engineid, snmp_req->engineid_len);
        conf.engineid_len = snmp_req->engineid_len;
        strcpy(conf.user_name, snmp_req->admin_string);
        if (snmpv3_mgmt_users_conf_get(&conf, FALSE) == VTSS_OK) {
            CPRINTF("The entry '%s, %s' is already exist, it doesn't allow modify\n",
                    misc_engineid2str(conf.engineid, conf.engineid_len),
                    conf.user_name);
            return;
        }
        conf.valid = 1;
        if (!strcmp(snmp_req->admin_string, SNMPV3_NONAME)) {
            CPRINTF("The user name of 'None' is reserved\n");
            return;
        }
        if (snmp_req->auth_protocol != SNMP_MGMT_AUTH_PROTO_NONE && (!strcmp(snmp_req->auth_password, ""))) {
            CPRINTF("Invaild parameter <auth_password>\n");
            return;
        }
        if (snmp_req->priv_protocol != SNMP_MGMT_PRIV_PROTO_NONE && (!strcmp(snmp_req->priv_password, ""))) {
            CPRINTF("Invaild parameter <priv_password>\n");
            return;
        }

        conf.security_level = SNMP_MGMT_SEC_LEVEL_NOAUTH;
        if (strcmp(snmp_req->auth_password, "")) {
            if (!snmp_req->auth_protocol) {
                conf.auth_protocol = SNMP_MGMT_AUTH_PROTO_MD5;
            } else {
                conf.auth_protocol = snmp_req->auth_protocol;
            }
            strcpy(conf.auth_password, snmp_req->auth_password);
            conf.security_level = SNMP_MGMT_SEC_LEVEL_AUTHNOPRIV;
        }
        if (strcmp(snmp_req->priv_password, "")) {
            conf.priv_protocol = snmp_req->priv_protocol;
            strcpy(conf.priv_password, snmp_req->priv_password);
            conf.security_level = SNMP_MGMT_SEC_LEVEL_AUTHPRIV;
        }
        conf.storage_type = SNMP_MGMT_STORAGE_PERMANENT;
        conf.status = SNMP_MGMT_ROW_ACTIVE;
        if (snmpv3_mgmt_users_conf_set(&conf) != VTSS_OK) {
            CPRINTF("snmpv3_mgmt_users_conf_set(%s, %s): failed\n",
                    misc_engineid2str(conf.engineid, conf.engineid_len),
                    conf.user_name);
        }
    }
}

static void SNMPV3_cli_cmd_user_del(cli_req_t *req)
{
    vtss_rc             rc;
    snmp_cli_req_t      *snmp_req = req->module_req;
    snmpv3_users_conf_t conf;

    if (req->set) {
        strcpy(conf.user_name, SNMPV3_CONF_ACESS_GETFIRST);
        while (snmpv3_mgmt_users_conf_get(&conf, TRUE) == VTSS_OK) {
            if (conf.status != SNMP_MGMT_ROW_ACTIVE) {
                continue;
            }
            if (conf.idx != snmp_req->entry_idx || !conf.valid) {
                continue;
            } else {
                if ((rc = snmpv3_mgmt_users_conf_del(snmp_req->entry_idx)) != VTSS_OK) {
                    CPRINTF("%s\n", error_txt(rc));
                }
                return;
            }
        }
        CPRINTF("Non-existing entry\n");
    }
}

static void SNMPV3_cli_cmd_user_change_password(cli_req_t *req)
{
    snmpv3_users_conf_t conf;
    snmp_cli_req_t *snmp_req = req->module_req;

    if (req->set) {
        memset(&conf, 0x0, sizeof(conf));
        memcpy(conf.engineid, snmp_req->engineid, snmp_req->engineid_len);
        conf.engineid_len = snmp_req->engineid_len;
        strcpy(conf.user_name, snmp_req->admin_string);
        if (snmpv3_mgmt_users_conf_get(&conf, FALSE) != VTSS_OK) {
            CPRINTF("The entry '%s, %s' is not exist\n",
                    misc_engineid2str(conf.engineid, conf.engineid_len),
                    conf.user_name);
            return;
        }
        if (conf.security_level == SNMP_MGMT_SEC_LEVEL_NOAUTH) {
            CPRINTF("The security level 'NoAuth, NoPriv' doesn't allow modify password\n");
            return;
        } else if (conf.security_level == SNMP_MGMT_SEC_LEVEL_AUTHNOPRIV) {
            if (strcmp(snmp_req->auth_password, "")) {
                strcpy(conf.auth_password, snmp_req->auth_password);
            }
            if (strcmp(snmp_req->priv_password, "")) {
                CPRINTF("The security level 'Auth, NoPriv' doesn't allow modify privacy password\n");
                return;
            }
        } else {
            if (strcmp(snmp_req->auth_password, "")) {
                strcpy(conf.auth_password, snmp_req->auth_password);
            }
            if (strcmp(snmp_req->priv_password, "")) {
                strcpy(conf.priv_password, snmp_req->priv_password);
            }
        }
        if (snmpv3_mgmt_users_conf_set(&conf) != VTSS_OK) {
            CPRINTF("snmpv3_mgmt_users_conf_set(%s, %s): failed\n",
                    misc_engineid2str(conf.engineid, conf.engineid_len),
                    conf.user_name);
        }
    }
}

static void SNMPV3_cli_cmd_user_lookup(cli_req_t *req)
{
    snmp_conf_t         snmp_conf;
    snmpv3_users_conf_t conf;
    int                 cnt = 0;
    snmp_cli_req_t *snmp_req = req->module_req;

    if (snmp_mgmt_snmp_conf_get(&snmp_conf) != VTSS_OK) {
        return;
    }
    strcpy(conf.user_name, SNMPV3_CONF_ACESS_GETFIRST);
    while (snmpv3_mgmt_users_conf_get(&conf, TRUE) == VTSS_OK) {
        if (conf.status != SNMP_MGMT_ROW_ACTIVE) {
            continue;
        }
        if (req->set) {
            if (conf.idx != snmp_req->entry_idx || !conf.valid) {
                continue;
            } else {
                CPRINTF("Entry Index             : %d\n", conf.idx);
                CPRINTF("Engine ID               : %s\n", misc_engineid2str(conf.engineid, conf.engineid_len));
                CPRINTF("User Name               : %s\n", conf.user_name);
                CPRINTF("Security Level          : %s\n", conf.security_level == SNMP_MGMT_SEC_LEVEL_NOAUTH ? "NoAuth, NoPriv" : conf.security_level == SNMP_MGMT_SEC_LEVEL_AUTHNOPRIV ? "Auth, NoPriv" : "Auth, Priv");
                CPRINTF("Authentication Protocol : %s\n", conf.auth_protocol == SNMP_MGMT_AUTH_PROTO_NONE ? "None" : conf.auth_protocol == SNMP_MGMT_AUTH_PROTO_MD5 ? "MD5" : "SHA");
                CPRINTF("Privacy Protocol        : %s\n", conf.priv_protocol == SNMP_MGMT_PRIV_PROTO_NONE ? "None" : "DES");
                return;
            }
        }
        if (++cnt == 1) {
            CPRINTF("Idx Engine ID User Name                        Level          Auth Priv\n");
            CPRINTF("--- --------- -------------------------------- -------------- ---- ----\n");
        }
        CPRINTF("%-3d %-9s %-32s %-14s %-4s %-4s\n",
                conf.idx,
                memcmp(snmp_conf.engineid, conf.engineid, snmp_conf.engineid_len > conf.engineid_len ? snmp_conf.engineid_len : conf.engineid_len) ? "Remote" : "Local",
                conf.user_name,
                conf.security_level == SNMP_MGMT_SEC_LEVEL_NOAUTH ? "NoAuth, NoPriv" : conf.security_level == SNMP_MGMT_SEC_LEVEL_AUTHNOPRIV ? "Auth, NoPriv" : "Auth, Priv",
                conf.auth_protocol == SNMP_MGMT_AUTH_PROTO_NONE ? "None" : conf.auth_protocol == SNMP_MGMT_AUTH_PROTO_MD5 ? "MD5" : "SHA",
                conf.priv_protocol == SNMP_MGMT_PRIV_PROTO_NONE ? "None" : "DES");
    }
    if (cnt) {
        CPRINTF("\nNumber of entries: %d\n", cnt);
    } else if (req->set) {
        CPRINTF("Non-existing entry\n");
    }
}

static void SNMPV3_cli_cmd_group_add(cli_req_t *req)
{
    snmp_conf_t               snmp_conf;
    snmpv3_communities_conf_t community_conf;
    snmpv3_users_conf_t       user_conf;
    snmpv3_groups_conf_t      conf;
    snmp_cli_req_t *snmp_req = req->module_req;

    if (req->set) {
        memset(&snmp_conf, 0x0, sizeof(snmp_conf));
        memset(&community_conf, 0x0, sizeof(community_conf));
        memset(&user_conf, 0x0, sizeof(user_conf));
        memset(&conf, 0x0, sizeof(conf));
        conf.security_model = snmp_req->security_model;
        strcpy(conf.security_name, snmp_req->security_name);

        /* check security name exist */
        if (conf.security_model == SNMP_MGMT_SEC_MODEL_USM) {
            if (snmp_mgmt_snmp_conf_get(&snmp_conf) != VTSS_OK) {
                return;
            }
            memcpy(user_conf.engineid, snmp_conf.engineid, snmp_conf.engineid_len);
            user_conf.engineid_len = snmp_conf.engineid_len;
            strcpy(user_conf.user_name, conf.security_name);
            if (snmpv3_mgmt_users_conf_get(&user_conf, FALSE) != VTSS_OK) {
                CPRINTF("The security name '%s, %s' is not exist\n",
                        misc_engineid2str(user_conf.engineid, user_conf.engineid_len),
                        user_conf.user_name);
                return;
            }
        } else {
            strcpy(community_conf.community, conf.security_name);
            if (snmpv3_mgmt_communities_conf_get(&community_conf, FALSE) != VTSS_OK) {
                CPRINTF("The security name '%s' is not exist\n", conf.security_name);
                return;
            }
        }

        conf.valid = 1;
        strcpy(conf.group_name, snmp_req->admin_string);
        conf.storage_type = SNMP_MGMT_STORAGE_PERMANENT;
        conf.status = SNMP_MGMT_ROW_ACTIVE;
        if (snmpv3_mgmt_groups_conf_set(&conf) != VTSS_OK) {
            CPRINTF("snmpv3_mgmt_groups_conf_set(%d, %s): failed\n",
                    conf.security_model, conf.security_name);
        }
    }
}

static void SNMPV3_cli_cmd_group_del(cli_req_t *req)
{
    vtss_rc             rc;
    snmp_cli_req_t      *snmp_req = req->module_req;
    snmpv3_groups_conf_t conf;

    if (req->set) {
        strcpy(conf.security_name, SNMPV3_CONF_ACESS_GETFIRST);
        while (snmpv3_mgmt_groups_conf_get(&conf, TRUE) == VTSS_OK) {
            if (conf.status != SNMP_MGMT_ROW_ACTIVE) {
                continue;
            }
            if (conf.idx != snmp_req->entry_idx || !conf.valid) {
                continue;
            } else {
                if ((rc = snmpv3_mgmt_groups_conf_del(snmp_req->entry_idx)) != VTSS_OK) {
                    CPRINTF("%s\n", error_txt(rc));
                }
                return;
            }
        }
        CPRINTF("Non-existing entry\n");
    }
}

static void SNMPV3_cli_cmd_group_lookup(cli_req_t *req)
{
    snmpv3_groups_conf_t conf;
    int                  cnt = 0;
    snmp_cli_req_t *snmp_req = req->module_req;

    strcpy(conf.security_name, SNMPV3_CONF_ACESS_GETFIRST);
    while (snmpv3_mgmt_groups_conf_get(&conf, TRUE) == VTSS_OK) {
        if (conf.status != SNMP_MGMT_ROW_ACTIVE) {
            continue;
        }
        if (req->set) {
            if (conf.idx != snmp_req->entry_idx || !conf.valid) {
                continue;
            } else {
                CPRINTF("Entry Index    : %d\n", conf.idx);
                CPRINTF("Security Model : %s\n", conf.security_model == SNMP_MGMT_SEC_MODEL_SNMPV1 ? "v1" : conf.security_model == SNMP_MGMT_SEC_MODEL_SNMPV2C ? "v2c" : "usm");
                CPRINTF("Security Name  : %s\n", conf.security_name);
                CPRINTF("Group Name     : %s\n", conf.group_name);
                return;
            }
        }
        if (++cnt == 1) {
            CPRINTF("Idx Model Security Name                    Group Name                      \n");
            CPRINTF("--- ----- -------------------------------- --------------------------------\n");
        }
        CPRINTF("%-3d %-5s %-32s %-32s\n",
                conf.idx,
                conf.security_model == SNMP_MGMT_SEC_MODEL_SNMPV1 ? "v1" : conf.security_model == SNMP_MGMT_SEC_MODEL_SNMPV2C ? "v2c" : "usm",
                conf.security_name,
                conf.group_name);
    }
    if (cnt) {
        CPRINTF("\nNumber of entries: %d\n", cnt);
    } else if (req->set) {
        CPRINTF("Non-existing entry\n");
    }
}

static void SNMPV3_cli_cmd_view_add(cli_req_t *req)
{
    snmpv3_views_conf_t conf;
    snmp_cli_req_t *snmp_req = req->module_req;

    if (req->set) {
        memset(&conf, 0x0, sizeof(conf));
        strcpy(conf.view_name, snmp_req->admin_string);
        memcpy(conf.subtree, snmp_req->oid, sizeof(u32) * snmp_req->oid_len);
        conf.subtree_len = snmp_req->oid_len;
        memcpy(conf.subtree_mask, snmp_req->oid_mask, snmp_req->oid_mask_len);
        conf.subtree_mask_len = snmp_req->oid_mask_len;
        conf.valid = 1;
        if (snmp_req->view_type) {
            conf.view_type = snmp_req->view_type;
        } else {
            conf.view_type = SNMPV3_MGMT_VIEW_INCLUDED;
        }
        conf.storage_type = SNMP_MGMT_STORAGE_PERMANENT;
        conf.status = SNMP_MGMT_ROW_ACTIVE;
        if (snmpv3_mgmt_views_conf_set(&conf) != VTSS_OK) {
            CPRINTF("snmpv3_mgmt_views_conf_set(%s, %s): failed\n",
                    conf.view_name,
                    misc_oid2str(conf.subtree, conf.subtree_len, conf.subtree_mask, conf.subtree_mask_len));
        }
    }
}

static void SNMPV3_cli_cmd_view_del(cli_req_t *req)
{
    vtss_rc             rc;
    snmp_cli_req_t      *snmp_req = req->module_req;
    snmpv3_views_conf_t conf;

    if (req->set) {
        strcpy(conf.view_name, SNMPV3_CONF_ACESS_GETFIRST);
        while (snmpv3_mgmt_views_conf_get(&conf, TRUE) == VTSS_OK) {
            if (conf.status != SNMP_MGMT_ROW_ACTIVE) {
                continue;
            }
            if (conf.idx != snmp_req->entry_idx || !conf.valid) {
                continue;
            } else {
                if ((rc = snmpv3_mgmt_views_conf_del(snmp_req->entry_idx)) != VTSS_OK) {
                    CPRINTF("%s\n", error_txt(rc));
                }
                return;
            }
        }
        CPRINTF("Non-existing entry\n");
    }
}

static void SNMPV3_cli_cmd_view_lookup(cli_req_t *req)
{
    snmpv3_views_conf_t conf;
    int                 cnt = 0;
    snmp_cli_req_t *snmp_req = req->module_req;

    strcpy(conf.view_name, SNMPV3_CONF_ACESS_GETFIRST);
    while (snmpv3_mgmt_views_conf_get(&conf, TRUE) == VTSS_OK) {
        if (conf.status != SNMP_MGMT_ROW_ACTIVE) {
            continue;
        }
        if (req->set) {
            if (conf.idx != snmp_req->entry_idx || !conf.valid) {
                continue;
            } else {
                CPRINTF("Entry Index    : %d\n", conf.idx);
                CPRINTF("View Name   : %s\n", conf.view_name);
                CPRINTF("View Type   : %s\n", conf.view_type == SNMPV3_MGMT_VIEW_INCLUDED ? "included" : "excluded");
                CPRINTF("OID Subtree : %s\n", misc_oid2str(conf.subtree, conf.subtree_len, conf.subtree_mask, conf.subtree_mask_len));
                return;
            }
        }
        if (++cnt == 1) {
            CPRINTF("Idx View Name                        View Type OID Subtree\n");
            CPRINTF("--- -------------------------------- --------- --------------------------------\n");
        }
        CPRINTF("%-3d %-32s %-9s %-32s\n",
                conf.idx,
                conf.view_name,
                conf.view_type == SNMPV3_MGMT_VIEW_INCLUDED ? "included" : "excluded",
                misc_oid2str(conf.subtree, conf.subtree_len, conf.subtree_mask, conf.subtree_mask_len));
    }
    if (cnt) {
        CPRINTF("\nNumber of entries: %d\n", cnt);
    } else if (req->set) {
        CPRINTF("Non-existing entry\n");
    }
}

static void SNMPV3_cli_cmd_access_add(cli_req_t *req)
{
    BOOL found = 0;
    snmpv3_groups_conf_t group_conf;
    snmpv3_views_conf_t view_conf;
    snmpv3_accesses_conf_t conf;
    snmp_cli_req_t *snmp_req = req->module_req;

    if (req->set) {
        memset(&group_conf, 0x0, sizeof(group_conf));
        memset(&view_conf, 0x0, sizeof(view_conf));
        memset(&conf, 0x0, sizeof(conf));
        strcpy(group_conf.security_name, SNMPV3_CONF_ACESS_GETFIRST);
        while (snmpv3_mgmt_groups_conf_get(&group_conf, TRUE) == VTSS_OK) {
            if (group_conf.status != SNMP_MGMT_ROW_ACTIVE) {
                continue;
            }
            if (!strcmp(group_conf.group_name, snmp_req->admin_string)) {
                found = 1;
                break;
            }
        }
        if (!found) {
            CPRINTF("The group name '%s' is not exist\n", snmp_req->admin_string);
            return;
        }

        strcpy(conf.group_name, snmp_req->admin_string);
        conf.security_model = snmp_req->security_model;
        conf.security_level = snmp_req->security_level;
        conf.context_match = SNMPV3_MGMT_CONTEX_MATCH_EXACT;
        conf.storage_type = SNMP_MGMT_STORAGE_PERMANENT;
        conf.status = SNMP_MGMT_ROW_ACTIVE;
        conf.valid = 1;
        if (strcmp(snmp_req->read_view_name, "") &&
            strcmp(snmp_req->read_view_name, SNMPV3_NONAME)) {
            found = 0;
            strcpy(view_conf.view_name, SNMPV3_CONF_ACESS_GETFIRST);
            while (snmpv3_mgmt_views_conf_get(&view_conf, TRUE) == VTSS_OK) {
                if (view_conf.status != SNMP_MGMT_ROW_ACTIVE) {
                    continue;
                }
                if (!strcmp(view_conf.view_name, snmp_req->read_view_name)) {
                    found = 1;
                    break;
                }
            }
            if (!found) {
                CPRINTF("The view name '%s' is not exist\n", snmp_req->read_view_name);
                return;
            }
            strcpy(conf.read_view_name, snmp_req->read_view_name);
        } else {
            strcpy(conf.read_view_name, SNMPV3_NONAME);
        }
        if (strcmp(snmp_req->write_view_name, "") &&
            strcmp(snmp_req->write_view_name, SNMPV3_NONAME)) {
            found = 0;
            strcpy(view_conf.view_name, SNMPV3_CONF_ACESS_GETFIRST);
            while (snmpv3_mgmt_views_conf_get(&view_conf, TRUE) == VTSS_OK) {
                if (view_conf.status != SNMP_MGMT_ROW_ACTIVE) {
                    continue;
                }
                if (!strcmp(view_conf.view_name, snmp_req->write_view_name)) {
                    found = 1;
                    break;
                }
            }
            if (!found) {
                CPRINTF("The view name '%s' is not exist\n", snmp_req->write_view_name);
                return;
            }
            strcpy(conf.write_view_name, snmp_req->write_view_name);
        } else {
            strcpy(conf.write_view_name, SNMPV3_NONAME);
        }
        if (strcmp(snmp_req->notify_view_name, "") &&
            !strcmp(snmp_req->notify_view_name, SNMPV3_NONAME)) {
            found = 0;
            strcpy(view_conf.view_name, SNMPV3_CONF_ACESS_GETFIRST);
            while (snmpv3_mgmt_views_conf_get(&view_conf, TRUE) == VTSS_OK) {
                if (view_conf.status != SNMP_MGMT_ROW_ACTIVE) {
                    continue;
                }
                if (!strcmp(view_conf.view_name, snmp_req->notify_view_name)) {
                    found = 1;
                    break;
                }
            }
            if (!found) {
                CPRINTF("The view name '%s' is not exist\n", snmp_req->notify_view_name);
                return;
            }
            strcpy(conf.notify_view_name, snmp_req->notify_view_name);
        } else {
            strcpy(conf.notify_view_name, SNMPV3_NONAME);
        }
        conf.storage_type = SNMP_MGMT_STORAGE_PERMANENT;
        conf.status = SNMP_MGMT_ROW_ACTIVE;
        if (snmpv3_mgmt_accesses_conf_set(&conf) != VTSS_OK) {
            CPRINTF("snmpv3_mgmt_accesses_conf_set(%s, %d, %d): failed\n",
                    conf.group_name, conf.security_model, conf.security_level);
        }
    }
}

static void SNMPV3_cli_cmd_access_del(cli_req_t *req)
{
    vtss_rc                 rc;
    snmp_cli_req_t          *snmp_req = req->module_req;
    snmpv3_accesses_conf_t  conf;

    if (req->set) {
        strcpy(conf.group_name, SNMPV3_CONF_ACESS_GETFIRST);
        while (snmpv3_mgmt_accesses_conf_get(&conf, TRUE) == VTSS_OK) {
            if (conf.status != SNMP_MGMT_ROW_ACTIVE) {
                continue;
            }
            if (conf.idx != snmp_req->entry_idx || !conf.valid) {
                continue;
            } else {
                if ((rc = snmpv3_mgmt_accesses_conf_del(snmp_req->entry_idx)) != VTSS_OK) {
                    CPRINTF("%s\n", error_txt(rc));
                }
                return;
            }
        }
        CPRINTF("Non-existing entry\n");
    }
}

static void SNMPV3_cli_cmd_access_lookup(cli_req_t *req)
{
    snmpv3_accesses_conf_t conf;
    int                    cnt = 0;
    snmp_cli_req_t *snmp_req = req->module_req;

    strcpy(conf.group_name, SNMPV3_CONF_ACESS_GETFIRST);
    while (snmpv3_mgmt_accesses_conf_get(&conf, TRUE) == VTSS_OK) {
        if (conf.status != SNMP_MGMT_ROW_ACTIVE) {
            continue;
        }
        if (req->set) {
            if (conf.idx != snmp_req->entry_idx || !conf.valid) {
                continue;
            } else {
                CPRINTF("Entry Index      : %d\n", conf.idx);
                CPRINTF("Group Name       : %s\n", conf.group_name);
                CPRINTF("Security Model   : %s\n", conf.security_model == SNMP_MGMT_SEC_MODEL_ANY ? "any" : conf.security_model == SNMP_MGMT_SEC_MODEL_SNMPV1 ? "v1" : conf.security_model == SNMP_MGMT_SEC_MODEL_SNMPV2C ? "v2c" : "usm");
                CPRINTF("Security Level   : %s\n", conf.security_level == SNMP_MGMT_SEC_LEVEL_NOAUTH ? "NoAuth, NoPriv" : conf.security_level == SNMP_MGMT_SEC_LEVEL_AUTHNOPRIV ? "Auth, NoPriv" : "Auth, Priv");
                CPRINTF("Read View Name   : %s\n", conf.read_view_name);
                CPRINTF("Write View Name  : %s\n", conf.write_view_name);
                return;
            }
        }
        if (++cnt == 1) {
            CPRINTF("Idx Group Name                       Model Level          ReadView         WriteView\n");
            CPRINTF("--- -------------------------------- ----- ----------------------------------------------------\n");
        }
        CPRINTF("%-3d %-32s %-5s %-14s %-16s %-16s\n",
                conf.idx,
                conf.group_name,
                conf.security_model == SNMP_MGMT_SEC_MODEL_ANY ? "any" : conf.security_model == SNMP_MGMT_SEC_MODEL_SNMPV1 ? "v1" : conf.security_model == SNMP_MGMT_SEC_MODEL_SNMPV2C ? "v2c" : "usm",
                conf.security_level == SNMP_MGMT_SEC_LEVEL_NOAUTH ? "NoAuth, NoPriv" : conf.security_level == SNMP_MGMT_SEC_LEVEL_AUTHNOPRIV ? "Auth, NoPriv" : "Auth, Priv",
                conf.read_view_name,
                conf.write_view_name);
    }
    if (cnt) {
        CPRINTF("\nNumber of entries: %d\n", cnt);
    } else if (req->set) {
        CPRINTF("Non-existing entry\n");
    }
}
#endif /* SNMP_SUPPORT_V3 */

static void SNMP_cli_cmd_conf_show(cli_req_t *req)
{
    if (!req->set) {
        cli_header("SNMP Configuration", 1);
    }

#ifdef VTSS_SW_OPTION_IPV6
    SNMP_cli_cmd_conf(req, 1, 1, 1, 1);
#else
    SNMP_cli_cmd_conf(req, 1, 1, 1, 1);
#endif /* VTSS_SW_OPTION_IPV6 */
#ifdef SNMP_SUPPORT_V3
    CPRINTF("\n");
    SNMPV3_cli_cmd_engineid_conf(req);
    CPRINTF("\nSNMPv3 Communities Table:\n");
    SNMPV3_cli_cmd_community_lookup(req);
    CPRINTF("\nSNMPv3 Users Table:\n");
    SNMPV3_cli_cmd_user_lookup(req);
    CPRINTF("\nSNMPv3 Groups Table;\n");
    SNMPV3_cli_cmd_group_lookup(req);
    CPRINTF("\nSNMPv3 Views Table:\n");
    SNMPV3_cli_cmd_view_lookup(req);
    CPRINTF("\nSNMPv3 Accesses Table:\n");
    SNMPV3_cli_cmd_access_lookup(req);
#endif /* SNMP_SUPPORT_V3 */
}

static void _trap_cli_show_trap_entry(vtss_isid_t isid,
                                      cli_req_t *req,
                                      vtss_trap_entry_t *entry,
                                      BOOL conf_lookup,
                                      BOOL warmstart,
                                      BOOL coldstart,
                                      BOOL linkup,
                                      BOOL linkdown,
                                      BOOL lldp,
                                      BOOL auth_fail,
                                      BOOL stp,
                                      BOOL rmon)
{
    vtss_trap_conf_t    *cfg;
    vtss_trap_event_t   *evt;
#ifdef VTSS_SW_OPTION_IPV6
    char                buf[40];
#endif /* VTSS_SW_OPTION_IPV6 */
    //vtss_port_no port;

    if (!entry) {
        return;
    }

    cfg = &entry->trap_conf;
    if (cfg) {
        CPRINTF("Trap %s (ID:%u) is %s\n",
                entry->trap_conf_name,
                cfg->conf_id,
                cli_bool_txt(cfg->enable));

        if (conf_lookup) {
            CPRINTF("Community       : %s\n", cfg->trap_community);
            CPRINTF("Source Host     : %s\n", cfg->trap_src_host_name);
            if ( FALSE == cfg->dip.ipv6_flag) {
                CPRINTF("Destination Host: %s\n", cfg->dip.addr.ipv4_str);
#ifdef VTSS_SW_OPTION_IPV6
            } else {
                CPRINTF("Destination Host: %s\n", misc_ipv6_txt(&cfg->dip.addr.ipv6, buf));
#endif /* VTSS_SW_OPTION_IPV6 */
            }
            CPRINTF("UDP Port        : %u\n", cfg->trap_port);
            switch ( cfg->trap_version ) {
            case SNMP_SUPPORT_V1:
                CPRINTF("Version         : V1\n");
                break;
            case SNMP_SUPPORT_V2C:
                CPRINTF("Version         : V2C\n");
                break;
            case SNMP_SUPPORT_V3:
                CPRINTF("Version         : V3\n");
                break;
            default:
                CPRINTF("Version         : INVALID!\n");
                break;
            }

            if (cfg->trap_version == SNMP_SUPPORT_V3 || cfg->trap_version == SNMP_SUPPORT_V2C ) {
                CPRINTF("Inform Mode     : %s\n", cli_bool_txt(cfg->trap_inform_mode));
                CPRINTF("Inform Timeout  : %u\n", cfg->trap_inform_timeout);
                CPRINTF("Inform Retry    : %u\n", cfg->trap_inform_retries);
            }
#ifdef SNMP_SUPPORT_V3
            if (cfg->trap_version == SNMP_SUPPORT_V3) {
                CPRINTF("Probe Mode      : %s\n", cli_bool_txt(cfg->trap_probe_engineid));
                CPRINTF("Engine ID       : %s\n", cfg->trap_engineid);
                CPRINTF("Engine ID Length: %u\n", cfg->trap_engineid_len);
                CPRINTF("Security Name   : %s\n", cfg->trap_security_name);
            }
#endif /* SNMP_SUPPORT_V3 */
        }
    }

    evt = &entry->trap_event;
    if (evt) {
        port_iter_t     pit;
        vtss_port_no_t  iport, uport;

        if (warmstart) {
            CPRINTF("Warm Start         : %s\n", cli_bool_txt(evt->system.warm_start));
        }
        if (coldstart) {
            CPRINTF("Cold Start         : %s\n", cli_bool_txt(evt->system.cold_start));
        }
        if (linkup) {
            (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
            while (port_iter_getnext(&pit)) {
                iport = pit.iport;
                uport = iport2uport(iport);
                if ( req && (req->uport_list[uport] == 0 || port_isid_port_no_is_stack(isid, iport))) {
                    continue;
                }
                CPRINTF("Port-%-2u Link Up    : %s\n", iport2uport(iport), cli_bool_txt(evt->interface.trap_linkup[isid][iport]));
            }
        }
        if (linkdown) {
            (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
            while (port_iter_getnext(&pit)) {
                iport = pit.iport;
                uport = iport2uport(iport);
                if ( req && (req->uport_list[uport] == 0 || port_isid_port_no_is_stack(isid, iport))) {
                    continue;
                }
                CPRINTF("Port-%-2u Link Down  : %s\n", iport2uport(iport), cli_bool_txt(evt->interface.trap_linkdown[isid][iport]));
            }
        }
        if (lldp) {
            (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
            while (port_iter_getnext(&pit)) {
                iport = pit.iport;
                uport = iport2uport(iport);
                if ( req && (req->uport_list[uport] == 0 || port_isid_port_no_is_stack(isid, iport))) {
                    continue;
                }
                CPRINTF("Port-%-2u LLDP       : %s\n", iport2uport(iport), cli_bool_txt(evt->interface.trap_lldp[isid][iport]));
            }
        }
        if (auth_fail) {
            CPRINTF("Authentication Fail: %s\n", cli_bool_txt(evt->aaa.trap_authen_fail));
        }
        if (stp) {
            CPRINTF("STP                : %s\n", cli_bool_txt(evt->sw.stp));
        }
        if (rmon) {
            CPRINTF("RMON               : %s\n", cli_bool_txt(evt->sw.rmon));
        }
    }
}

BOOL is_event_set ( BOOL conf_add,
                    BOOL warmstart,
                    BOOL coldstart,
                    BOOL linkup,
                    BOOL linkdown,
                    BOOL lldp,
                    BOOL auth_fail,
                    BOOL stp,
                    BOOL rmon)
{
    return conf_add && (warmstart || coldstart || linkup || linkdown || lldp || auth_fail || stp || rmon);
}

static void trap_cli_cmd_go(cli_req_t *req,
                            BOOL global_mode,
                            BOOL conf_lookup,
                            BOOL conf_add,
                            BOOL conf_delete,
                            BOOL warmstart,
                            BOOL coldstart,
                            BOOL linkup,
                            BOOL linkdown,
                            BOOL lldp,
                            BOOL auth_fail,
                            BOOL stp,
                            BOOL rmon)
{
    i8                  buf[80], *p;
    vtss_isid_t         isid;
    vtss_usid_t         usid;
    switch_iter_t       sit;
    snmp_cli_req_t      *snmp_req = req->module_req;
    BOOL                found, flag, conf_done, event_set;
    vtss_trap_entry_t   entry;
    vtss_trap_conf_t    *cfg;
    vtss_trap_event_t   *evt;

    if (cli_cmd_switch_none(req) || cli_cmd_conf_slave(req) || !snmp_req) {
        T_E("error req");
        return;
    }

    memset(buf, 0x0, sizeof(buf));

    if (global_mode) {
        if (req->set) {
            flag = req->enable;
            (void) trap_mgmt_mode_set(flag);
        } else {
            flag = FALSE;
            (void)trap_mgmt_mode_get(&flag);
            CPRINTF("Trap Global Mode: %s\n", cli_bool_txt(flag));
        }
    }

    conf_done = FALSE;

    event_set = is_event_set( conf_add,
                              warmstart,
                              coldstart,
                              linkup,
                              linkdown,
                              lldp,
                              auth_fail,
                              stp,
                              rmon);
    (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_USID);
    while (switch_iter_getnext(&sit)) {
        usid = sit.usid;

        if ((isid = req->stack.isid[usid]) == VTSS_ISID_END) {
            continue;
        }

        flag = FALSE;   /* Specific Name Flag */
        if (strlen(snmp_req->trap_name)) {
            flag = TRUE;
        }

        p = &buf[0];
        p += sprintf(p, "Trap Config Entry Setting");
        cli_table_header(buf);

        memset(&entry, 0x0, sizeof(vtss_trap_entry_t));

        T_D("isid = %d, snmp_req->trap_name = %s, flag = %d", isid, snmp_req->trap_name, flag);
        if (flag && !event_set) {
            /* Assigned a trap configuration name */
            memcpy(entry.trap_conf_name, snmp_req->trap_name, sizeof(entry.trap_conf_name));
            cfg = &entry.trap_conf;
            found = (trap_mgmt_conf_get(&entry) == VTSS_RC_OK) ? TRUE : FALSE;
            T_D("req_set = %d, conf_done = %d, found = %d, conf_add = %d, conf_del = %d",
                req->set, conf_done, found, conf_add, conf_delete);
            if (!conf_done) {
                if (conf_add || conf_delete) {
                    if (conf_add) {
                        flag = FALSE;
                        if (found == TRUE) {
                            flag = TRUE;
                        }

                        entry.valid = TRUE;
                        cfg->enable = (snmp_req->enable_set) ? req->enable : (FALSE == flag) ? TRUE : cfg->enable;

                        if (snmp_req->dip_set) {
                            if (req->host_name[0] != 0) {
                                cfg->dip.ipv6_flag = FALSE;
                                strcpy(cfg->dip.addr.ipv4_str, req->host_name);

#ifdef VTSS_SW_OPTION_IPV6
                            } else {
                                cfg->dip.ipv6_flag = TRUE;
                                cfg->dip.addr.ipv6 = req->ipv6_addr;
#endif /* VTSS_SW_OPTION_IPV6 */
                            }
                        } else if ( FALSE == flag ) {
                            cfg->dip.ipv6_flag = FALSE;
                            strcpy(cfg->dip.addr.ipv4_str, "");
                        }

                        cfg->trap_port = (snmp_req->dport_set) ? snmp_req->trap_port : (FALSE == flag) ? 162 : cfg->trap_port;

                        cfg->trap_version = (snmp_req->version_set) ? snmp_req->version : (FALSE == flag) ? SNMP_SUPPORT_V2C : cfg->trap_version;

                        if ( snmp_req->community_set) {
                            strcpy(cfg->trap_community, req->parm);
                        } else if ( FALSE == flag ) {
                            strcpy(cfg->trap_community, "public");
                        }

                        cfg->trap_inform_mode = (snmp_req->inform_set) ? snmp_req->trap_type : (FALSE == flag) ? VTSS_TRAP_MSG_TRAP2 : cfg->trap_inform_mode;

                        cfg->trap_inform_retries = (snmp_req->retries_set) ? snmp_req->trap_inform_retries : (FALSE == flag) ? 3 : cfg->trap_inform_retries;

                        cfg->trap_inform_timeout = (snmp_req->timeout_set) ? snmp_req->trap_inform_timeout : (FALSE == flag) ? 1 : cfg->trap_inform_timeout;

                        cfg->trap_probe_engineid = (snmp_req->probe_set) ? snmp_req->probe : (FALSE == flag) ? 1 : cfg->trap_probe_engineid;

                        if ( snmp_req->engineid_set ) {
                            memcpy( cfg->trap_engineid, snmp_req->engineid, snmp_req->engineid_len);
                        } else if ( FALSE == flag ) {
                            memset(cfg->trap_engineid, 0, SNMPV3_MAX_ENGINE_ID_LEN);
                        }

                        if ( snmp_req->security_name_set ) {
                            strcpy( cfg->trap_security_name, snmp_req->security_name);
                        } else if ( FALSE == flag ) {
                            memset(cfg->trap_security_name, 0, SNMPV3_MAX_NAME_LEN);
                        }

                        (void)trap_mgmt_conf_set(&entry);
                    } else {
                        if (found != TRUE) {
                            return;
                        }
                        entry.valid = FALSE;
                        (void) trap_mgmt_conf_set(&entry);
                    }
                } else if (found) {
                    _trap_cli_show_trap_entry(isid,
                                              req,
                                              &entry,
                                              conf_lookup,
                                              warmstart,
                                              coldstart,
                                              linkup,
                                              linkdown,
                                              lldp,
                                              auth_fail,
                                              stp,
                                              rmon);
                }
                conf_done = TRUE;
            }
        } else {
            if (!conf_done) {
                if (!req->set) {
                    while (trap_mgmt_conf_get_next(&entry) == VTSS_RC_OK) {
                        _trap_cli_show_trap_entry(isid,
                                                  req,
                                                  &entry,
                                                  conf_lookup,
                                                  warmstart,
                                                  coldstart,
                                                  linkup,
                                                  linkdown,
                                                  lldp,
                                                  auth_fail,
                                                  stp,
                                                  rmon);
                    }
                    conf_done = TRUE;
                } else if ( FALSE == conf_add ) {
                    evt = &entry.trap_event;
                    memcpy(entry.trap_conf_name, snmp_req->trap_name, sizeof(entry.trap_conf_name));
                    if (trap_mgmt_conf_get(&entry) == VTSS_RC_OK) {
                        _trap_cli_show_trap_entry(isid,
                                                  req,
                                                  &entry,
                                                  0,
                                                  warmstart,
                                                  coldstart,
                                                  linkup,
                                                  linkdown,
                                                  lldp,
                                                  auth_fail,
                                                  stp,
                                                  rmon);

                    }
                    conf_done = TRUE;
                } else {
                    port_iter_t       pit;
                    vtss_port_no_t    iport, uport;
                    evt = &entry.trap_event;

                    memcpy(entry.trap_conf_name, snmp_req->trap_name, sizeof(entry.trap_conf_name));
                    if (trap_mgmt_conf_get(&entry) == VTSS_RC_OK) {
                        flag = TRUE;
                    }

                    if (warmstart) {
                        evt->system.warm_start = req->enable;
                    }
                    if (coldstart) {
                        evt->system.cold_start = req->enable;
                    }

                    (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
                    while (port_iter_getnext(&pit)) {
                        iport = pit.iport;
                        uport = iport2uport(iport);
                        if (req->uport_list[uport] == 0 || port_isid_port_no_is_stack(isid, iport)) {
                            continue;
                        }

                        if (linkup) {
                            evt->interface.trap_linkup[isid][iport] = req->enable;
                        }
                        if (linkdown) {
                            evt->interface.trap_linkdown[isid][iport] = req->enable;
                        }

                        if (lldp) {
                            evt->interface.trap_lldp[isid][iport] = req->enable;
                        }
                    }
                    if (auth_fail) {
                        evt->aaa.trap_authen_fail = req->enable;
                    }
                    if (rmon) {
                        evt->sw.rmon = req->enable;
                    }
                    if (stp) {
                        evt->sw.stp = req->enable;
                    }

                    /* GetUserInput and AssignTo entry.XXX */
                    (void) trap_mgmt_conf_set(&entry);
                }
            }
        }
    }

}

static void trap_cli_cmd_trap_global_mode(cli_req_t *req)
{
    trap_cli_cmd_go(req, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

static void trap_cli_cmd_trap_conf_lookup(cli_req_t *req)
{
    if (req->set) {
        trap_cli_cmd_go(req, 0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1);
    } else {
        trap_cli_cmd_go(req, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    }
}
static void trap_cli_cmd_trap_conf_add(cli_req_t *req)
{
    trap_cli_cmd_go(req, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

static void trap_cli_cmd_trap_conf_delete(cli_req_t *req)
{
    trap_cli_cmd_go(req, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0);
}

static void trap_cli_cmd_trap_event_lookup(cli_req_t *req)
{
    trap_cli_cmd_go(req, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1);
}

static void trap_cli_cmd_trap_event_warmstart(cli_req_t *req)
{
    snmp_cli_req_t      *snmp_req = req->module_req;
    if (snmp_req->warm_start) {
        trap_cli_cmd_go(req, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0);
    } else {
        trap_cli_cmd_go(req, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0);
    }
}

static void trap_cli_cmd_trap_event_coldstart(cli_req_t *req)
{
    snmp_cli_req_t      *snmp_req = req->module_req;
    if (snmp_req->cold_start) {
        trap_cli_cmd_go(req, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0);
    } else {
        trap_cli_cmd_go(req, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0);
    }

}

static void trap_cli_cmd_trap_event_linkup(cli_req_t *req)
{
    snmp_cli_req_t      *snmp_req = req->module_req;
    if (snmp_req->linkup) {
        trap_cli_cmd_go(req, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0);
    } else {
        trap_cli_cmd_go(req, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0);
    }
}

static void trap_cli_cmd_trap_event_linkdown(cli_req_t *req)
{
    snmp_cli_req_t      *snmp_req = req->module_req;
    if (snmp_req->linkdown) {
        trap_cli_cmd_go(req, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0);
    } else {
        trap_cli_cmd_go(req, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0);
    }

}

static void trap_cli_cmd_trap_event_lldp(cli_req_t *req)
{
    snmp_cli_req_t      *snmp_req = req->module_req;
    if (snmp_req->lldp) {
        trap_cli_cmd_go(req, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0);
    } else {
        trap_cli_cmd_go(req, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0);
    }
}

static void trap_cli_cmd_trap_event_auth_fail(cli_req_t *req)
{
    snmp_cli_req_t      *snmp_req = req->module_req;
    if (snmp_req->auth_fail) {
        trap_cli_cmd_go(req, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0);
    } else {
        trap_cli_cmd_go(req, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0);
    }
}

static void trap_cli_cmd_trap_event_stp(cli_req_t *req)
{
    snmp_cli_req_t      *snmp_req = req->module_req;
    if (snmp_req->stp) {
        trap_cli_cmd_go(req, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0);
    } else {
        trap_cli_cmd_go(req, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0);
    }
}

static void trap_cli_cmd_trap_event_rmon(cli_req_t *req)
{
    snmp_cli_req_t      *snmp_req = req->module_req;
    if (snmp_req->rmon) {
        trap_cli_cmd_go(req, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1);
    } else {
        trap_cli_cmd_go(req, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1);
    }
}

/****************************************************************************/
/*  Parameter functions                                                     */
/****************************************************************************/

/* Parse raw text engine ID string */
static int cli_parse_engieid(char *cmd, u32 *engineid_len, u8 *engineid)
{
    int     slen = strlen(cmd);
    int     idx;
    uint    pval;
    char    buf[4];

    /* The format of 'Engine ID' may not be all zeros or all 'ff'H
       and is restricted to 5 - 32 octet string */

    for (idx = 0; idx < slen; idx++) {
        if (!((cmd[idx] >= '0' && cmd[idx] <= '9') || (cmd[idx] >= 'A' && cmd[idx] <= 'F') || (cmd[idx] >= 'a' && cmd[idx] <= 'f'))) {
            CPRINTF("The format of 'Engine ID' may not be all zeros or all 'ff'H and is restricted to 5 - 32 octet string\n");
            return 1;
        }
    }

    if ((slen % 2) || (slen < SNMPV3_MIN_ENGINE_ID_LEN * 2) || (slen > SNMPV3_MAX_ENGINE_ID_LEN * 2)) {
        CPRINTF("The format of 'Engine ID' may not be all zeros or all 'ff'H and is restricted to 5 - 32 octet string\n");
        return 1;
    }

    for (idx = 0; idx < slen; idx = idx + 2) {
        memcpy(buf, cmd + idx, 2);
        buf[2] = '\0';
        sscanf(buf, "%x", &pval);
        engineid[idx / 2] = (u8)pval;
    }
    *engineid_len = slen / 2;

    if (!snmpv3_is_valid_engineid(engineid, *engineid_len)) {
        CPRINTF("The format of 'Engine ID' may not be all zeros or all 'ff'H and is restricted to 5 - 32 octet string\n");
        return 1;
    }

    return 0;
}

/* Parse raw text OID subtree string */
static int cli_parse_oid_subtree(char *str, u32 *oidSubTree, u32 *oid_len, u8 *oid_mask, u32 *oid_mask_len)
{
    char *value_char;
    int  num = 0;
    u32  i, mask = 0x80, maskpos = 0;

    value_char = str;
    *oid_len = *oid_mask_len = 0;

    //check if OID format .x.x.x
    if (value_char[0] != '.') {
        return 1;
    }
    for (i = 0; i < strlen(str); i++) {
        if (((value_char[i] != '.') && (value_char[i] != '*')) &&
            (value_char[i] < '0' || value_char[i] > '9')) {
            return 1;
        }
        if (value_char[i] == '*') {
            if (i == 0 || value_char[i - 1] != '.') {
                return 1;
            }
        }
        if (value_char[i] == '.') {
            if (i == strlen(str) - 1) {
                return 1;
            } else if (value_char[i + 1] == '.') {
                return 1;
            }
            num++;
            if (num > 128) {
                return 1;
            }
        }
    }
    *oid_mask_len = *oid_len = num;

    /* convert OID string (RFC1447)
       Each bit of this bit mask corresponds to the (8*i - 7)-th
       sub-identifier, and the least significant bit of the i-th
       octet of this octet string corresponding to the (8*i)-th
       sub-identifier, where i is in the range 1 through 16. */
    for (i = 0; i < *oid_len; i++) {
        if (!memcmp(value_char, ".*", 2)) {
            oidSubTree[i] = 0;
            oid_mask[maskpos] &= (~mask);
            value_char = value_char + 2;
        } else {
            oid_mask[maskpos] |= mask;
            sscanf(value_char++, ".%d", &oidSubTree[i]);
        }

        if (i == *oid_len - 1) {
            break; //last OID node
        }
        while (*value_char != '.') {
            value_char++;
        }

        if (mask == 1) {
            mask = 0x80;
            maskpos++;
        } else {
            mask >>= 1;
        }
    }

    return 0;
}

static int32_t SNMP_cli_comm_parse (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;
    snmp_cli_req_t *snmp_req = req->module_req;

    req->parm_parsed = 1;
    snmp_req->community_set = 1;
    error = cli_parse_text(cmd_org, req->parm, SNMP_MGMT_MAX_COMMUNITY_LEN);

    return error;
}

static int32_t SNMP_cli_trap_inform_timeout_parse (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;
    snmp_cli_req_t *snmp_req = req->module_req;

    req->parm_parsed = 1;
    snmp_req->timeout_set = 1;
    error = cli_parse_ulong(cmd, &snmp_req->trap_inform_timeout, 0, SNMP_MGMT_MAX_TRAP_INFORM_TIMEOUT);

    return error;
}

static int32_t SNMP_cli_trap_inform_retries_parse (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;
    snmp_cli_req_t *snmp_req = req->module_req;

    req->parm_parsed = 1;
    snmp_req->retries_set = 1;
    error = cli_parse_ulong(cmd, &snmp_req->trap_inform_retries, 0, SNMP_MGMT_MAX_TRAP_INFORM_RETRIES);

    return error;
}

#ifdef SNMP_SUPPORT_V3
static int32_t SNMP_cli_security_name_parse (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;
    snmp_cli_req_t *snmp_req = req->module_req;

    req->parm_parsed = 1;
    snmp_req->security_name_set = 1;
    error = cli_parse_string(cmd_org, snmp_req->security_name, SNMPV3_MIN_NAME_LEN, SNMPV3_MAX_NAME_LEN);

    return error;
}

static int32_t SNMP_cli_engine_id_parse(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;
    snmp_cli_req_t *snmp_req = req->module_req;

    req->parm_parsed = 1;
    snmp_req->engineid_set = 1;
    error = cli_parse_engieid(cmd_org, &snmp_req->engineid_len, snmp_req->engineid);

    return error;
}

static int32_t SNMP_cli_admin_str_parse(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;
    snmp_cli_req_t *snmp_req = req->module_req;

    req->parm_parsed = 1;
    error = cli_parse_string(cmd_org, snmp_req->admin_string, SNMPV3_MIN_NAME_LEN, SNMPV3_MAX_NAME_LEN);

    return error;
}

static int32_t SNMP_cli_username_parse(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;
    snmp_cli_req_t *snmp_req = req->module_req;

    req->parm_parsed = 1;
    error = cli_parse_string(cmd_org, snmp_req->username, 1, VTSS_SYS_USERNAME_LEN);

    return error;
}

static int32_t trap_cli_conf_name_parse(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;
    snmp_cli_req_t *snmp_req = req->module_req;

    req->parm_parsed = 1;
    error = cli_parse_string(cmd_org, snmp_req->trap_name, 1, TRAP_MAX_NAME_LEN);

    return error;
}

static int32_t trap_cli_conf_trap_port_parse(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;
    snmp_cli_req_t *snmp_req = req->module_req;

    req->parm_parsed = 1;
    snmp_req->dport_set = 1;
    error = cli_parse_ulong(cmd, &snmp_req->trap_port, 1, 65535);

    return error;

}

static int32_t trap_cli_conf_trap_event_warmstart_parse(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;
    snmp_cli_req_t *snmp_req = req->module_req;

    req->parm_parsed = 1;
    snmp_req->warm_start = 1;
    error = cli_parm_parse_keyword( cmd, cmd2, stx, cmd_org, req);

    return error;

}

static int32_t trap_cli_conf_trap_event_coldstart_parse(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;
    snmp_cli_req_t *snmp_req = req->module_req;

    req->parm_parsed = 1;
    snmp_req->cold_start = 1;
    error = cli_parm_parse_keyword( cmd, cmd2, stx, cmd_org, req);

    return error;

}

static int32_t trap_cli_conf_trap_event_linkup_parse(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;
    snmp_cli_req_t *snmp_req = req->module_req;

    req->parm_parsed = 1;
    snmp_req->linkup = 1;
    error = cli_parm_parse_keyword( cmd, cmd2, stx, cmd_org, req);

    return error;

}

static int32_t trap_cli_conf_trap_event_linkdown_parse(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;
    snmp_cli_req_t *snmp_req = req->module_req;

    req->parm_parsed = 1;
    snmp_req->linkdown = 1;
    error = cli_parm_parse_keyword( cmd, cmd2, stx, cmd_org, req);

    return error;

}

static int32_t trap_cli_conf_trap_event_rmon_parse(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;
    snmp_cli_req_t *snmp_req = req->module_req;

    req->parm_parsed = 1;
    snmp_req->rmon = 1;
    error = cli_parm_parse_keyword( cmd, cmd2, stx, cmd_org, req);

    return error;

}

static int32_t trap_cli_conf_trap_event_stp_parse(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;
    snmp_cli_req_t *snmp_req = req->module_req;

    req->parm_parsed = 1;
    snmp_req->stp = 1;
    error = cli_parm_parse_keyword( cmd, cmd2, stx, cmd_org, req);

    return error;

}

static int32_t trap_cli_conf_trap_event_auth_fail_parse(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;
    snmp_cli_req_t *snmp_req = req->module_req;

    req->parm_parsed = 1;
    snmp_req->auth_fail = 1;
    error = cli_parm_parse_keyword( cmd, cmd2, stx, cmd_org, req);

    return error;

}

static int32_t trap_cli_conf_trap_event_lldp_parse(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;
    snmp_cli_req_t *snmp_req = req->module_req;

    req->parm_parsed = 1;
    snmp_req->auth_fail = 1;
    error = cli_parm_parse_keyword( cmd, cmd2, stx, cmd_org, req);

    return error;

}
static int32_t SNMP_cli_auth_password_parse(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;
    snmp_cli_req_t *snmp_req = req->module_req;

    req->parm_parsed = 1;
    error = cli_parse_string(cmd_org, snmp_req->auth_password, SNMPV3_MIN_PASSWORD_LEN, SNMPV3_MAX_SHA_PASSWORD_LEN);

    return error;
}

static int32_t SNMP_cli_priv_password_parse(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;
    snmp_cli_req_t *snmp_req = req->module_req;

    req->parm_parsed = 1;
    error = cli_parse_string(cmd_org, snmp_req->priv_password, SNMPV3_MIN_PASSWORD_LEN, SNMPV3_MAX_DES_PASSWORD_LEN);

    return error;
}

static int32_t SNMP_cli_security_level_parse(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t         error = 0;
    u32             idx;
    snmp_cli_req_t  *snmp_req = req->module_req;

    req->parm_parsed = 1;
    error = cli_parse_text(cmd_org, req->parm, 13);
    for (idx = 0; idx <= strlen(req->parm); idx++) {
        req->parm[idx] = tolower(req->parm[idx]);
    }
    if (!strncmp(req->parm, "noauthnopriv", (strlen(req->parm) >= 1 && strlen(req->parm) <= 12) ? strlen(req->parm) : 12)) {
        snmp_req->security_level = SNMP_MGMT_SEC_LEVEL_NOAUTH;
    } else if (!strncmp(req->parm, "authnopriv", (strlen(req->parm) >= 5 && strlen(req->parm) <= 10) ? strlen(req->parm) : 10)) {
        snmp_req->security_level = SNMP_MGMT_SEC_LEVEL_AUTHNOPRIV;
    } else if (!strncmp(req->parm, "authpriv", (strlen(req->parm) >= 5 && strlen(req->parm) <= 8) ? strlen(req->parm) : 8)) {
        snmp_req->security_level = SNMP_MGMT_SEC_LEVEL_AUTHPRIV;
    } else {
        return 1;
    }

    return error;
}

static int32_t SNMP_cli_security_model_parse(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t         error = 0;
    u32             idx;
    snmp_cli_req_t  *snmp_req = req->module_req;

    req->parm_parsed = 1;
    error = cli_parse_text(cmd_org, req->parm, 4);
    for (idx = 0; idx <= strlen(req->parm); idx++) {
        req->parm[idx] = tolower(req->parm[idx]);
    }
    if (!strncmp(req->parm, "v1", 2)) {
        snmp_req->security_model = SNMP_MGMT_SEC_MODEL_SNMPV1;
    } else if (!strncmp(req->parm, "v2c", (strlen(req->parm) >= 2 && strlen(req->parm) <= 3) ? strlen(req->parm) : 3)) {
        snmp_req->security_model = SNMP_MGMT_SEC_MODEL_SNMPV2C;
    } else if (!strncmp(req->parm, "usm", (strlen(req->parm) >= 1 && strlen(req->parm) < 3) ? strlen(req->parm) : 3)) {
        snmp_req->security_model = SNMP_MGMT_SEC_MODEL_USM;
    } else {
        return 1;
    }

    return error;
}

static int32_t SNMP_cli_oid_subtree_parse(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;
    snmp_cli_req_t *snmp_req = req->module_req;

    req->parm_parsed = 1;
    error = cli_parse_oid_subtree(cmd_org, snmp_req->oid, &snmp_req->oid_len, snmp_req->oid_mask, &snmp_req->oid_mask_len);

    return error;
}

static int32_t SNMP_cli_access_security_model_parse(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;
    snmp_cli_req_t *snmp_req = req->module_req;

    req->parm_parsed = 1;
    error = cli_parse_text(cmd_org, req->parm, 4);
    if (!strncmp(req->parm, "any", (strlen(req->parm) >= 1 && strlen(req->parm) < 3) ? strlen(req->parm) : 3)) {
        snmp_req->security_model = SNMP_MGMT_SEC_MODEL_ANY;
    } else if (!strncmp(req->parm, "v1", 2)) {
        snmp_req->security_model = SNMP_MGMT_SEC_MODEL_SNMPV1;
    } else if (!strncmp(req->parm, "v2c", (strlen(req->parm) >= 2 && strlen(req->parm) <= 3) ? strlen(req->parm) : 3)) {
        snmp_req->security_model = SNMP_MGMT_SEC_MODEL_SNMPV2C;
    } else if (!strncmp(req->parm, "usm", (strlen(req->parm) >= 1 && strlen(req->parm) < 3) ? strlen(req->parm) : 3)) {
        snmp_req->security_model = SNMP_MGMT_SEC_MODEL_USM;
    } else {
        return 1;
    }

    return error;
}

static int32_t SNMP_cli_read_view_name_parse(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;
    snmp_cli_req_t *snmp_req = req->module_req;

    req->parm_parsed = 1;
    error = cli_parse_string(cmd_org, snmp_req->read_view_name, SNMPV3_MIN_NAME_LEN, SNMPV3_MAX_NAME_LEN);

    return error;
}

static int32_t SNMP_cli_write_view_name_parse(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;
    snmp_cli_req_t *snmp_req = req->module_req;

    req->parm_parsed = 1;
    error = cli_parse_string(cmd_org, snmp_req->write_view_name, SNMPV3_MIN_NAME_LEN, SNMPV3_MAX_NAME_LEN);

    return error;
}

static int32_t SNMP_cli_notify_view_name_parse(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;
    snmp_cli_req_t *snmp_req = req->module_req;

    req->parm_parsed = 1;
    error = cli_parse_string(cmd_org, snmp_req->notify_view_name, SNMPV3_MIN_NAME_LEN, SNMPV3_MAX_NAME_LEN);

    return error;
}

static int32_t SNMP_cli_index_parse(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t         error = 0;
    u32             value = 0;
    snmp_cli_req_t  *snmp_req = req->module_req;

    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &value, 1, SNMPV3_MAX_GROUPS);
    snmp_req->entry_idx = value;

    return error;
}
#endif /* SNMP_SUPPORT_V3 */

static int32_t SNMP_cli_ipv4_mask_parse(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;

    req->parm_parsed = 1;
    error = cli_parse_ipv4(cmd, &req->ipv4_mask, NULL, &req->ipv4_mask_spec, 1);

    return error;
}

#ifdef VTSS_SW_OPTION_IPV6
static int32_t SNMP_cli_ipv6_addr_parse(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;

    req->parm_parsed = 1;
    error = cli_parse_ipv6(cmd, &req->ipv6_addr, &req->ipv6_addr_spec);

    return error;
}
#endif /*VTSS_SW_OPTION_IPV6*/

static int32_t SNMP_cli_ipv4v6_addr_parse(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;
    snmp_cli_req_t *snmp_req = req->module_req;

    req->parm_parsed = 1;
    req->ipv6_addr_spec = CLI_SPEC_NONE;
    if (cli_parm_parse_ipaddr_str( cmd, cmd2, stx, cmd_org, req) == 0) {
        snmp_req->dip_set = 1;
        return 0;
    }
    req->host_name[0] = 0;

    error = cli_parse_ipv6(cmd, &req->ipv6_addr, &req->ipv6_addr_spec);
    if (!error) {
        snmp_req->dip_set = 1;
    }

    return error;
}




static int32_t SNMP_cli_parse_keyword (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    char *found = cli_parse_find(cmd, stx);
    snmp_cli_req_t *snmp_req = req->module_req;

    req->parm_parsed = 1;
    if (found != NULL) {
        if (!strncmp(found, "1", 1)) {
            snmp_req->version = SNMP_SUPPORT_V1;
            snmp_req->version_set = 1;
            /* 2 */
        } else if (!strncmp(found, "2c", 2)) {
            snmp_req->version = SNMP_SUPPORT_V2C;
            snmp_req->version_set = 1;
#ifdef SNMP_SUPPORT_V3
            /* 3 */
        } else if (!strncmp(found, "3", 1)) {
            snmp_req->version = SNMP_SUPPORT_V3;
            snmp_req->version_set = 1;
        } else if (!strncmp(found, "md5", 3)) {
            snmp_req->auth_protocol = SNMP_MGMT_AUTH_PROTO_MD5;
        } else if (!strncmp(found, "sha", 3)) {
            snmp_req->auth_protocol = SNMP_MGMT_AUTH_PROTO_SHA;
        } else if (!strncmp(found, "des", 3)) {
            snmp_req->priv_protocol = SNMP_MGMT_PRIV_PROTO_DES;
        } else if (!strncmp(found, "aes", 3)) {
            snmp_req->priv_protocol = SNMP_MGMT_PRIV_PROTO_AES;
        } else if (!strncmp(found, "included", 8)) {
            snmp_req->view_type = SNMPV3_MGMT_VIEW_INCLUDED;
        } else if (!strncmp(found, "excluded", 8)) {
            snmp_req->view_type = SNMPV3_MGMT_VIEW_EXCLUDED;
#endif /* SNMP_SUPPORT_V3 */
        } else if (!strncmp(found, "enable", 6)) {
            req->enable = 1;
            snmp_req->enable_set = 1;
        } else if (!strncmp(found, "disable", 6)) {
            req->enable = 0;
            snmp_req->enable_set = 1;
        } else if (!strncmp(found, "trap", 4)) {
            snmp_req->trap_type = VTSS_TRAP_MSG_TRAP2;
            snmp_req->inform_set = 1;
        } else if (!strncmp(found, "informs", 7)) {
            snmp_req->trap_type = VTSS_TRAP_MSG_INFORM;
            snmp_req->inform_set = 1;
        } else if (!strncmp(found, "probe", 5)) {
            snmp_req->probe = 1;
            snmp_req->probe_set = 1;
        } else if (!strncmp(found, "engine", 6)) {
            snmp_req->probe_set = 1;
        } else if (!strncmp(found, "dip", 3)) {
            snmp_req->dip_set = 1;
        } else if (!strncmp(found, "dport", 5)) {
            snmp_req->dport_set = 1;
        } else if (!strncmp(found, "community", 9)) {
            snmp_req->community_set = 1;
        } else if (!strncmp(found, "security", 8)) {
            snmp_req->security_set = 1;
        }
    }

    return (found == NULL ? 1 : 0);
}


/****************************************************************************/
/*  Parameter table                                                         */
/****************************************************************************/

static cli_parm_t snmp_cli_parm_table[] = {
    {
        "enable|disable",
        "enable : Enable SNMP\n"
        "disable: Disable SNMP\n"
        "(default: Show SNMP mode)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        SNMP_cli_cmd_conf_mode
    },
    {
#ifdef SNMP_SUPPORT_V3
        "1|2c|3",
#else
        "1|2c",
#endif /* SNMP_SUPPORT_V3 */
        "1 : SNMP version 1\n"
        "2c: SNMP version 2c\n"
#ifdef SNMP_SUPPORT_V3
        "3 : SNMP version 3\n"
#endif /* SNMP_SUPPORT_V3 */
        "(default: Show SNMP version)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        SNMP_cli_parse_keyword,
        SNMP_cli_cmd_conf_ver
    },
    {
        "<community>",
        "Community string. Use 'clear' or \"\" to clear the string\n"
        "Maximum length allowed is upto 256 characters.\n"
        "(default: Show SNMP read community)",
        CLI_PARM_FLAG_SET,
        SNMP_cli_comm_parse,
        SNMP_cli_cmd_conf_read_comm
    },
    {
        "<community>",
        "Community string. Use 'clear' or \"\" to clear the string\n"
        "Maximum length allowed is upto 256 characters.\n"
        "(default: Show SNMP write community)",
        CLI_PARM_FLAG_SET,
        SNMP_cli_comm_parse,
        SNMP_cli_cmd_conf_write_comm
    },
    {
        "<comm>",
        "Community string. Use 'clear' or \"\" to clear the string\n"
        "Maximum length allowed is upto 256 characters.\n"
        "(default: Show SNMP trap community)",
        CLI_PARM_FLAG_SET,
        SNMP_cli_comm_parse,
        trap_cli_cmd_trap_conf_add
    },
    {
        "enable|disable",
        "enable : Enable SNMP trap mode\n"
        "disable: Disable SNMP trap mode\n"
        "(default: Show SNMP trap mode)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        trap_cli_cmd_trap_global_mode
    },
    {
        "enable|disable",
        "enable : Enable SNMP trap configuraton(default)\n"
        "disable: Disable SNMP trap configuraton",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        SNMP_cli_parse_keyword,
        trap_cli_cmd_trap_conf_add
    },
#if 1
    {
        "1",
        "1: SNMP version 1\n"
        ,
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        SNMP_cli_parse_keyword,
        trap_cli_cmd_trap_conf_add
    },
    {
        "2c",
        "2c: SNMP version 2c\n"
        ,
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        SNMP_cli_parse_keyword,
        trap_cli_cmd_trap_conf_add
    },
#ifdef SNMP_SUPPORT_V3
    {
        "3",
        "3: SNMP version 3\n"
        ,
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        SNMP_cli_parse_keyword,
        trap_cli_cmd_trap_conf_add
    },
#endif
#endif /* SNMP_SUPPORT_V3 */
    {
        "trap",
        "trap: Send notification as trap\n"
        ,
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        SNMP_cli_parse_keyword,
        trap_cli_cmd_trap_conf_add
    },
    {
        "informs",
        "infors: Send notification as informs\n"
        ,
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        SNMP_cli_parse_keyword,
        trap_cli_cmd_trap_conf_add
    },
    {
        "security",
        "security: Send notification as trap\n"
        ,
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        SNMP_cli_parse_keyword,
        trap_cli_cmd_trap_conf_add
    },

    {
        "enable|disable",
        "enable : Enable SNMP trap Warm-start\n"
        "disable: Disable SNMP trap Warm-start\n"
        "(default: Show SNMP trap Warm-start mode)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        trap_cli_conf_trap_event_warmstart_parse,
        trap_cli_cmd_trap_event_warmstart
    },
    {
        "enable|disable",
        "enable : Enable SNMP trap Cold-start\n"
        "disable: Disable SNMP trap Cold-start\n"
        "(default: Show SNMP trap Cold-start mode)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        trap_cli_conf_trap_event_coldstart_parse,
        trap_cli_cmd_trap_event_coldstart
    },
    {
        "enable|disable",
        "enable : Enable SNMP trap link-up\n"
        "disable: Disable SNMP trap link-up\n"
        "(default: Show SNMP trap link-up mode)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        trap_cli_conf_trap_event_linkup_parse,
        trap_cli_cmd_trap_event_linkup
    },
    {
        "enable|disable",
        "enable : Enable SNMP trap link-down\n"
        "disable: Disable SNMP trap link-down\n"
        "(default: Show SNMP trap link-down mode)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        trap_cli_conf_trap_event_linkdown_parse,
        trap_cli_cmd_trap_event_linkdown
    },
    {
        "enable|disable",
        "enable : Enable SNMP trap authentication failure\n"
        "disable: Disable SNMP trap authentication failure\n"
        "(default: Show SNMP trap authentication failure mode)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        trap_cli_conf_trap_event_auth_fail_parse,
        trap_cli_cmd_trap_event_auth_fail
    },
    {
        "enable|disable",
        "enable : Enable SNMP trap LLDP\n"
        "disable: Disable SNMP trap LLDP\n"
        "(default: Show SNMP trap LLDP mode)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        trap_cli_conf_trap_event_lldp_parse,
        trap_cli_cmd_trap_event_lldp
    },
    {
        "enable|disable",
        "enable : Enable SNMP trap STP\n"
        "disable: Disable SNMP trap STP\n"
        "(default: Show SNMP trap STP mode)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        trap_cli_conf_trap_event_stp_parse,
        trap_cli_cmd_trap_event_stp
    },
    {
        "enable|disable",
        "enable : Enable SNMP trap RMON\n"
        "disable: Disable SNMP trap RMON\n"
        "(default: Show SNMP trap RMON mode)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        trap_cli_conf_trap_event_rmon_parse,
        trap_cli_cmd_trap_event_rmon
    },
    {
        "<timeout>",
        "SNMP trap inform timeout (0-2147 seconds)\n"
        "(default: Show SNMP trap inform timeout)",
        CLI_PARM_FLAG_SET,
        SNMP_cli_trap_inform_timeout_parse,
        trap_cli_cmd_trap_conf_add
    },
    {
        "<retries>",
        "SNMP trap inform retransmited times (0-255)\n"
        "(default: Show SNMP trap inform retry times)",
        CLI_PARM_FLAG_SET,
        SNMP_cli_trap_inform_retries_parse,
        trap_cli_cmd_trap_conf_add
    },
    {
        "probe",
        "probe: SNMP trap probe security engine ID mode of operation. (default)",
        CLI_PARM_FLAG_SET,
        SNMP_cli_parse_keyword,
        trap_cli_cmd_trap_conf_add
    },
    {
        "engine",
        "engine: Manually setting EngineID, the format may not be all zeros or all 'ff'H\n"
        "                 and is restricted to 5 - 32 octet string",
        CLI_PARM_FLAG_SET,
        SNMP_cli_parse_keyword,
        trap_cli_cmd_trap_conf_add
    },

    {
        "community",
        "community: Manually setting EngineID, the format may not be all zeros or all 'ff'H\n"
        "                 and is restricted to 5 - 32 octet string",
        CLI_PARM_FLAG_SET,
        SNMP_cli_parse_keyword,
        trap_cli_cmd_trap_conf_add
    },
#ifdef SNMP_SUPPORT_V3
    {
        "<security_name>",
        "A string representing the security name for a principal\n"
        "                  (default: Show SNMP trap security name).\n"
        "                  The allowed string length is (" vtss_xstr(SNMPV3_MIN_NAME_LEN) "-" vtss_xstr(SNMPV3_MAX_NAME_LEN) "),\n"
        "                  and the allowed content is ASCII characters from 33 to 126",
        CLI_PARM_FLAG_SET,
        SNMP_cli_security_name_parse,
        trap_cli_cmd_trap_conf_add
    },
    {
        "<engineid>",
        "Engine ID, the format may not be all zeros or all 'ff'H\n"
        "                 and is restricted to 5 - 32 octet string",
        CLI_PARM_FLAG_SET,
        SNMP_cli_engine_id_parse,
        trap_cli_cmd_trap_conf_add
    },

    {
        "<engineid>",
        "Engine ID, the format may not be all zeros or all 'ff'H\n"
        "                 and is restricted to 5 - 32 octet string",
        CLI_PARM_FLAG_SET,
        SNMP_cli_engine_id_parse,
        NULL
    },
    {
        "<community>",
        "Community string",
        CLI_PARM_FLAG_SET,
        SNMP_cli_admin_str_parse,
        SNMPV3_cli_cmd_community_add
    },
    {
        "<community>",
        "Community string",
        CLI_PARM_FLAG_SET,
        SNMP_cli_admin_str_parse,
        SNMPV3_cli_cmd_community_del
    },
    {
        "<user_name>",
        "A string identifying the user name that this entry should\n"
        "                 belong to. The name of " vtss_xstr(SNMPV3_NONAME) " is reserved.\n"
        "                 The allowed string length is (" vtss_xstr(SNMPV3_MIN_NAME_LEN) "-" vtss_xstr(SNMPV3_MAX_NAME_LEN) "), and the allowed content\n"
        "                 is ASCII characters from 33 to 126",
        CLI_PARM_FLAG_SET,
        SNMP_cli_admin_str_parse,
        SNMPV3_cli_cmd_user_add
    },
    {
        "md5|sha",
        "md5            : An optional flag to indicate that this user using MD5\n"
        "                 authentication protocol. The allowed length is (" vtss_xstr(SNMPV3_MIN_PASSWORD_LEN) "-" vtss_xstr(SNMPV3_MAX_MD5_PASSWORD_LEN) "),\n"
        "                 and the allowed content is ASCII characters from 33 to 126\n"
        "sha            : An optional flag to indicate that this user using SHA\n"
        "                 authentication protocol. The allowed length is (" vtss_xstr(SNMPV3_MIN_PASSWORD_LEN) "-" vtss_xstr(SNMPV3_MAX_SHA_PASSWORD_LEN) "),\n"
        "                 and the allowed content is ASCII characters from 33 to 126",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        SNMP_cli_parse_keyword,
        SNMPV3_cli_cmd_user_add
    },
    {
        "des|aes",
        "des            : An optional flag to indicate that this user using DES privacy\n"
        "                 protocol privacy protocol should belong to.\n"
        "                 The allowed string length is (" vtss_xstr(SNMPV3_MIN_PASSWORD_LEN) "-" vtss_xstr(SNMPV3_MAX_DES_PASSWORD_LEN) "), and the allowed content\n"
        "                 is ASCII characters from 33 to 126"
        "aes            : An optional flag to indicate that this user using AES privacy\n"
        "                 protocol privacy protocol should belong to.\n"
        "                 The allowed string length is (" vtss_xstr(SNMPV3_MIN_PASSWORD_LEN) "-" vtss_xstr(SNMPV3_MAX_DES_PASSWORD_LEN) "), and the allowed content\n"
        "                 is ASCII characters from 33 to 126",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        SNMP_cli_parse_keyword,
        SNMPV3_cli_cmd_user_add
    },
    {
        "<user_name>",
        "A string identifying the user name that this entry should\n"
        "                 belong to. The name of " vtss_xstr(SNMPV3_NONAME) " is reserved."
        "                 The allowed string length is (" vtss_xstr(SNMPV3_MIN_NAME_LEN) "-" vtss_xstr(SNMPV3_MAX_NAME_LEN) "), and the allowed content\n"
        "                 is ASCII characters from 33 to 126",
        CLI_PARM_FLAG_SET,
        SNMP_cli_admin_str_parse,
        SNMPV3_cli_cmd_user_change_password
    },
    {
        "<user_name>",
        "A string identifying the user name that this entry should\n"
        "                 belong to. The name of " vtss_xstr(SNMPV3_NONAME) " is reserved."
        "                 The allowed string length is (" vtss_xstr(SNMPV3_MIN_NAME_LEN) "-" vtss_xstr(SNMPV3_MAX_NAME_LEN) "), and the allowed content\n"
        "                 is ASCII characters from 33 to 126",
        CLI_PARM_FLAG_SET,
        SNMP_cli_username_parse,
        NULL
    },
    {
        "<conf_name>",
        "A string identifying the user name that this entry should\n"
        "                 belong to."
        "                 The allowed string length is (" vtss_xstr(TRAP_MIN_NAME_LEN) "-" vtss_xstr(TRAP_MAX_NAME_LEN) "), and the allowed content\n"
        "                 is ASCII characters from 33 to 126",
        CLI_PARM_FLAG_SET,
        trap_cli_conf_name_parse,
        trap_cli_cmd_trap_conf_lookup
    },
    {
        "<conf_name>",
        "A string identifying the user name that this entry should belong to.\n"
        "                 The allowed string length is (" vtss_xstr(TRAP_MIN_NAME_LEN) "-" vtss_xstr(TRAP_MAX_NAME_LEN) "), and the allowed content\n"
        "                 is ASCII characters from 33 to 126",
        CLI_PARM_FLAG_SET,
        trap_cli_conf_name_parse,
        trap_cli_cmd_trap_conf_add
    },
    {
        "<conf_name>",
        "A string identifying the user name that this entry should belong to.\n"
        "                 The allowed string length is (" vtss_xstr(TRAP_MIN_NAME_LEN) "-" vtss_xstr(TRAP_MAX_NAME_LEN) "), and the allowed content\n"
        "                 is ASCII characters from 33 to 126",
        CLI_PARM_FLAG_SET,
        trap_cli_conf_name_parse,
        trap_cli_cmd_trap_conf_delete
    },
    {
        "<conf_name>",
        "A string identifying the user name that this entry should belong to.\n"
        "                 The allowed string length is (" vtss_xstr(TRAP_MIN_NAME_LEN) "-" vtss_xstr(TRAP_MAX_NAME_LEN) "), and the allowed content\n"
        "                 is ASCII characters from 33 to 126",
        CLI_PARM_FLAG_SET,
        trap_cli_conf_name_parse,
        trap_cli_cmd_trap_event_lookup
    },
    {
        "<conf_name>",
        "A string identifying the user name that this entry should belong to.\n"
        "                 The allowed string length is (" vtss_xstr(TRAP_MIN_NAME_LEN) "-" vtss_xstr(TRAP_MAX_NAME_LEN) "), and the allowed content\n"
        "                 is ASCII characters from 33 to 126",
        CLI_PARM_FLAG_SET,
        trap_cli_conf_name_parse,
        trap_cli_cmd_trap_event_warmstart
    },
    {
        "<conf_name>",
        "A string identifying the user name that this entry should belong to.\n"
        "                 The allowed string length is (" vtss_xstr(TRAP_MIN_NAME_LEN) "-" vtss_xstr(TRAP_MAX_NAME_LEN) "), and the allowed content\n"
        "                 is ASCII characters from 33 to 126",
        CLI_PARM_FLAG_SET,
        trap_cli_conf_name_parse,
        trap_cli_cmd_trap_event_coldstart
    },
    {
        "<conf_name>",
        "A string identifying the user name that this entry should belong to.\n"
        "                 The allowed string length is (" vtss_xstr(TRAP_MIN_NAME_LEN) "-" vtss_xstr(TRAP_MAX_NAME_LEN) "), and the allowed content\n"
        "                 is ASCII characters from 33 to 126",
        CLI_PARM_FLAG_SET,
        trap_cli_conf_name_parse,
        trap_cli_cmd_trap_event_linkup
    },
    {
        "<conf_name>",
        "A string identifying the user name that this entry should belong to.\n"
        "                 The allowed string length is (" vtss_xstr(TRAP_MIN_NAME_LEN) "-" vtss_xstr(TRAP_MAX_NAME_LEN) "), and the allowed content\n"
        "                 is ASCII characters from 33 to 126",
        CLI_PARM_FLAG_SET,
        trap_cli_conf_name_parse,
        trap_cli_cmd_trap_event_linkdown
    },
    {
        "<conf_name>",
        "A string identifying the user name that this entry should belong to.\n"
        "                 The allowed string length is (" vtss_xstr(TRAP_MIN_NAME_LEN) "-" vtss_xstr(TRAP_MAX_NAME_LEN) "), and the allowed content\n"
        "                 is ASCII characters from 33 to 126",
        CLI_PARM_FLAG_SET,
        trap_cli_conf_name_parse,
        trap_cli_cmd_trap_event_lldp
    },
    {
        "<conf_name>",
        "A string identifying the user name that this entry should belong to.\n"
        "                 The allowed string length is (" vtss_xstr(TRAP_MIN_NAME_LEN) "-" vtss_xstr(TRAP_MAX_NAME_LEN) "), and the allowed content\n"
        "                 is ASCII characters from 33 to 126",
        CLI_PARM_FLAG_SET,
        trap_cli_conf_name_parse,
        trap_cli_cmd_trap_event_auth_fail
    },
    {
        "<conf_name>",
        "A string identifying the user name that this entry should belong to.\n"
        "                 The allowed string length is (" vtss_xstr(TRAP_MIN_NAME_LEN) "-" vtss_xstr(TRAP_MAX_NAME_LEN) "), and the allowed content\n"
        "                 is ASCII characters from 33 to 126",
        CLI_PARM_FLAG_SET,
        trap_cli_conf_name_parse,
        trap_cli_cmd_trap_event_stp
    },
    {
        "<conf_name>",
        "A string identifying the user name that this entry should belong to.\n"
        "                 The allowed string length is (" vtss_xstr(TRAP_MIN_NAME_LEN) "-" vtss_xstr(TRAP_MAX_NAME_LEN) "), and the allowed content\n"
        "                 is ASCII characters from 33 to 126",
        CLI_PARM_FLAG_SET,
        trap_cli_conf_name_parse,
        trap_cli_cmd_trap_event_rmon
    },
    {
        "<ipv4v6_addr>",
        "IPv6 address is in 128-bit records represented as eight fields\n"
        "               of up to four hexadecimal digits with a colon separates each\n"
        "               field (:). For example, four hexadecimal digits with a colon\n"
        "               separates each field (:). For example,\n"
        "               'fe80::215:c5ff:fe03:4dc7'. The symbol '::' is a special\n"
        "               syntax that can be used as a shorthand way of representing\n"
        "               multiple 16-bit groups of contiguous zeros; but it can only\n"
        "               appear once. It also used a following legally IPv4 address.\n"
        "               For example,'::192.1.2.34'.",
        CLI_PARM_FLAG_SET,
        SNMP_cli_ipv4v6_addr_parse,
        trap_cli_cmd_trap_conf_add
    },

    {
        "dip",
        "dip: Destinaton IP of the trap configuration\n",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        SNMP_cli_parse_keyword,
        trap_cli_cmd_trap_conf_add
    },
    {
        "dport",
        "dport: Destinaton port of the trap configuration\n",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        SNMP_cli_parse_keyword,
        trap_cli_cmd_trap_conf_add
    },

    {
        "<udp_port>",
        "UDP port for the trap destination (default: 162)\n",
        CLI_PARM_FLAG_SET,
        trap_cli_conf_trap_port_parse,
        trap_cli_cmd_trap_conf_add
    },
    {
        "<auth_password>",
        "A string identifying the authentication pass phrase",
        CLI_PARM_FLAG_SET,
        SNMP_cli_auth_password_parse,
        NULL
    },
    {
        "<priv_password>",
        "A string identifying the privacy pass phrase.\n"
        "                 The allowed string length is (" vtss_xstr(SNMPV3_MIN_PASSWORD_LEN) "-" vtss_xstr(SNMPV3_MAX_SHA_PASSWORD_LEN) "), and the allowed content\n"
        "                 is ASCII characters from 33 to 126",
        CLI_PARM_FLAG_SET,
        SNMP_cli_priv_password_parse,
        NULL
    },
    {
        "<security_level>",
        "noAuthNoPriv - None authentication and none privacy\n"
        "                   AuthNoPriv   - Authentication and none privacy\n"
        "                   AuthPriv     - Authentication and privacy",
        CLI_PARM_FLAG_SET,
        SNMP_cli_security_level_parse,
        NULL
    },
    {
        "<security_model>",
        "v1 - Reserved for SNMPv1\n"
        "                  v2c - Reserved for SNMPv2c\n"
        "                  usm - User-based Security Model (USM)",
        CLI_PARM_FLAG_SET,
        SNMP_cli_security_model_parse,
        SNMPV3_cli_cmd_group_add
    },
    {
        "<security_model>",
        "v1 - Reserved for SNMPv1\n"
        "                  v2c - Reserved for SNMPv2c\n"
        "                  usm - User-based Security Model (USM)",
        CLI_PARM_FLAG_SET,
        SNMP_cli_security_model_parse,
        SNMPV3_cli_cmd_group_del
    },
    {
        "<security_name>",
        "A string identifying the security name that this entry should\n"
        "                  belong to. The allowed string length is (" vtss_xstr(SNMPV3_MIN_NAME_LEN) "-" vtss_xstr(SNMPV3_MAX_NAME_LEN) "),\n"
        "                  and the allowed content is ASCII characters from 33 to 126",
        CLI_PARM_FLAG_SET,
        SNMP_cli_security_name_parse,
        SNMPV3_cli_cmd_group_add
    },
    {
        "<security_name>",
        "A string identifying the security name that this entry should\n"
        "                 belong to. The allowed string length is (" vtss_xstr(SNMPV3_MIN_NAME_LEN) "-" vtss_xstr(SNMPV3_MAX_NAME_LEN) "),\n"
        "                 and the allowed content is ASCII characters from 33 to 126",
        CLI_PARM_FLAG_SET,
        SNMP_cli_security_name_parse,
        SNMPV3_cli_cmd_group_del
    },
    {
        "<view_name>",
        "A string identifying the view name that this entry should\n"
        "               belong to. The allowed string length is (" vtss_xstr(SNMPV3_MIN_NAME_LEN) "-" vtss_xstr(SNMPV3_MAX_NAME_LEN) "),\n"
        "               and the allowed content is ASCII characters from 33 to 126",
        CLI_PARM_FLAG_SET,
        SNMP_cli_admin_str_parse,
        NULL
    },
    {
        "<group_name>",
        "A string identifying the group name that this entry should\n"
        "                  belong to. The allowed string length is (" vtss_xstr(SNMPV3_MIN_NAME_LEN) "-" vtss_xstr(SNMPV3_MAX_NAME_LEN) "),\n"
        "                  and the allowed content is ASCII characters from 33 to 126",
        CLI_PARM_FLAG_SET,
        SNMP_cli_admin_str_parse,
        SNMPV3_cli_cmd_group_add
    },
    {
        "<group_name>",
        "A string identifying the group name that this entry should\n"
        "                   belong to. The allowed string length is (" vtss_xstr(SNMPV3_MIN_NAME_LEN) "-" vtss_xstr(SNMPV3_MAX_NAME_LEN) "),\n"
        "                   and the allowed content is ASCII characters from 33 to 126",
        CLI_PARM_FLAG_SET,
        SNMP_cli_admin_str_parse,
        SNMPV3_cli_cmd_access_add
    },
    {
        "included|excluded",
        "included     : An optional flag to indicate that this view subtree should\n"
        "               included\n"
        "excluded     : An optional flag to indicate that this view subtree should\n"
        "               excluded",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        SNMP_cli_parse_keyword,
        SNMPV3_cli_cmd_view_add
    },
    {
        "<oid_subtree>",
        "The OID defining the root of the subtree to add to the named view",
        CLI_PARM_FLAG_SET,
        SNMP_cli_oid_subtree_parse,
        NULL
    },
    {
        "<security_model>",
        "any - Accepted any security model (v1|v2c|usm)\n"
        "                   v1  - Reserved for SNMPv1\n"
        "                   v2c - Reserved for SNMPv2c\n"
        "                   usm - User-based Security Model (USM)",
        CLI_PARM_FLAG_SET,
        SNMP_cli_access_security_model_parse,
        SNMPV3_cli_cmd_access_add
    },
    {
        "<security_model>",
        "any - accepted any security model (v1|v2c|usm)\n"
        "                   v1  - Reserved for SNMPv1\n"
        "                   v2c - Reserved for SNMPv2c\n"
        "                   usm - User-based Security Model (USM)",
        CLI_PARM_FLAG_SET,
        SNMP_cli_access_security_model_parse,
        SNMPV3_cli_cmd_access_del
    },
    {
        "<read_view_name>",
        "The name of the MIB view defining the MIB objects for which\n"
        "                   this request may request the current values.\n"
        "                   The name of " vtss_xstr(SNMPV3_NONAME) " is reserved.\n"
        "                   The allowed string length is (" vtss_xstr(SNMPV3_MIN_NAME_LEN) "-" vtss_xstr(SNMPV3_MAX_NAME_LEN) "),\n"
        "                   and the allowed content is ASCII characters from 33 to 126",
        CLI_PARM_FLAG_SET,
        SNMP_cli_read_view_name_parse,
        NULL
    },
    {
        "<write_view_name>",
        "The name of the MIB view defining the MIB objects for which\n"
        "                   this request may potentially SET new values.\n"
        "                   The name of " vtss_xstr(SNMPV3_NONAME) " is reserved.\n"
        "                   The allowed string length is (" vtss_xstr(SNMPV3_MIN_NAME_LEN) "-" vtss_xstr(SNMPV3_MAX_NAME_LEN) "),\n"
        "                   and the allowed content is ASCII characters from 33 to 126",
        CLI_PARM_FLAG_SET,
        SNMP_cli_write_view_name_parse,
        NULL
    },
    {
        "<notify_view_name>",
        "The name of the MIB view defining the MIB objects for which\n"
        "                     this request may request the current values.\n"
        "                     The name of " vtss_xstr(SNMPV3_NONAME) " is reserved",
        CLI_PARM_FLAG_SET,
        SNMP_cli_notify_view_name_parse,
        NULL
    },
    {
        "<index>",
        "entry index (1-64)",
        CLI_PARM_FLAG_SET,
        SNMP_cli_index_parse,
        NULL
    },
#endif /* SNMP_SUPPORT_V3 */
    {
        "<ip_mask>",
        "IPv4 subnet mask (a.b.c.d), default: Show IP mask",
        CLI_PARM_FLAG_SET,
        SNMP_cli_ipv4_mask_parse,
        NULL
    },
#ifdef VTSS_SW_OPTION_IPV6
    {
        "<ipv6_addr>",
        "IPv6 address is in 128-bit records represented as eight fields\n"
        "               of up to four hexadecimal digits with a colon separates each\n"
        "               field (:). For example, four hexadecimal digits with a colon\n"
        "               separates each field (:). For example,\n"
        "               'fe80::215:c5ff:fe03:4dc7'. The symbol '::' is a special\n"
        "               syntax that can be used as a shorthand way of representing\n"
        "               multiple 16-bit groups of contiguous zeros; but it can only\n"
        "               appear once. It also used a following legally IPv4 address.\n"
        "               For example,'::192.1.2.34'.",
        CLI_PARM_FLAG_SET,
        SNMP_cli_ipv6_addr_parse,
        NULL
    },
#endif /* VTSS_SW_OPTION_IPV6 */

    {NULL, NULL, 0, 0, NULL}
};

/****************************************************************************/
/*  Command table                                                           */
/****************************************************************************/

enum {
    SNMP_PRIO_CONF,
    SNMP_PRIO_MODE,
    SNMP_PRIO_VER,
    SNMP_PRIO_READ_COMM,
    SNMP_PRIO_WRITE_COMM,
    SNMP_PRIO_TRAP_MODE,
    SNMP_PRIO_TRAP_VER,
    SNMP_PRIO_TRAP_COMM,
    SNMP_PRIO_TRAP_DIP,
#ifdef VTSS_SW_OPTION_IPV6
    SNMP_PRIO_TRAP_DIPV6,
#endif /* VTSS_SW_OPTION_IPV6 */
    SNMP_PRIO_TRAP_AUTHEN_FAIL,
    SNMP_PRIO_TRAP_LINKUP_LINKDOWN,
    SNMP_PRIO_TRAP_INFORM_MODE,
    SNMP_PRIO_TRAP_INFORM_TIMEOUT,
    SNMP_PRIO_TRAP_INFORM_REETRIES,
    SNMP_PRIO_TRAP_PROBE_SECURITY_ENGINE_ID,
    SNMP_PRIO_TRAP_SECURITY_ENGINE_ID,
    SNMP_PRIO_TRAP_SECURITY_NAME,
    PRIO_SNMPV3_ENGINE_ID,
    PRIO_SNMPV3_COMMUNITY_ADD,
    PRIO_SNMPV3_COMMUNITY_DEL,
    PRIO_SNMPV3_COMMUNITY_LOOKUP,
    PRIO_SNMPV3_USER_ADD,
    PRIO_SNMPV3_USER_DEL,
    PRIO_SNMPV3_USER_CHANGE_PASSWORD,
    PRIO_SNMPV3_USER_LOOKUP,
    PRIO_SNMPV3_GROUP_ADD,
    PRIO_SNMPV3_GROUP_DEL,
    PRIO_SNMPV3_GROUP_LOOKUP,
    PRIO_SNMPV3_VIEW_ADD,
    PRIO_SNMPV3_VIEW_DEL,
    PRIO_SNMPV3_VIEW_LOOKUP,
    PRIO_SNMPV3_ACCESS_ADD,
    PRIO_SNMPV3_ACCESS_DEL,
    PRIO_SNMPV3_ACCESS_LOOKUP,
    TRAP_MODE,
    TRAP_CONF_LOOKUP,
    TRAP_CONF_ADD,
    TRAP_CONF_DEL,
    TRAP_EVENT_LOOKUP,
    TRAP_EVENT_SYSTEM_WARM_START,
    TRAP_EVENT_SYSTEM_COLD_START,
    TRAP_EVENT_INTERFACE_LINKUP,
    TRAP_EVENT_INTERFACE_LINKDOWN,
    TRAP_EVENT_INTERFACE_LLDP,
    TRAP_EVENT_AAA_AUTH_FAIL,
    TRAP_EVENT_SWITCH_STP,
    TRAP_EVENT_SWITCH_RMON
};

cli_cmd_tab_entry(
    VTSS_CLI_GRP_SEC_SWITCH_PATH "SNMP Configuration",
    NULL,
    "Show SNMP configuration",
    SNMP_PRIO_CONF,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_SNMP,
    SNMP_cli_cmd_conf_show,
    NULL,
    snmp_cli_parm_table,
    CLI_CMD_FLAG_SYS_CONF
);

cli_cmd_tab_entry(
    VTSS_CLI_GRP_SEC_SWITCH_PATH "SNMP Mode",
    VTSS_CLI_GRP_SEC_SWITCH_PATH "SNMP Mode [enable|disable]",
    "Set or show the SNMP mode",
    SNMP_PRIO_MODE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SNMP,
    SNMP_cli_cmd_conf_mode,
    NULL,
    snmp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

#ifdef SNMP_SUPPORT_V3
cli_cmd_tab_entry(
    VTSS_CLI_GRP_SEC_SWITCH_PATH "SNMP Version",
    VTSS_CLI_GRP_SEC_SWITCH_PATH "SNMP Version [1|2c|3]",
    "Set or show the SNMP protocol version",
    SNMP_PRIO_VER,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SNMP,
    SNMP_cli_cmd_conf_ver,
    NULL,
    snmp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#else
cli_cmd_tab_entry(
    VTSS_CLI_GRP_SEC_SWITCH_PATH "SNMP Version",
    VTSS_CLI_GRP_SEC_SWITCH_PATH "SNMP Version [1|2c]",
    "Set or show the SNMP protocol version",
    SNMP_PRIO_VER,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SNMP,
    SNMP_cli_cmd_conf_ver,
    NULL,
    snmp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif

cli_cmd_tab_entry(
    VTSS_CLI_GRP_SEC_SWITCH_PATH "SNMP Read Community",
    VTSS_CLI_GRP_SEC_SWITCH_PATH "SNMP Read Community [<community>]",
    "Set or show the community string for SNMP read access",
    SNMP_PRIO_READ_COMM,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SNMP,
    SNMP_cli_cmd_conf_read_comm,
    NULL,
    snmp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    VTSS_CLI_GRP_SEC_SWITCH_PATH "SNMP Write Community",
    VTSS_CLI_GRP_SEC_SWITCH_PATH "SNMP Write Community [<community>]",
    "Set or show the community string for SNMP write access",
    SNMP_PRIO_WRITE_COMM,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SNMP,
    SNMP_cli_cmd_conf_write_comm,
    NULL,
    snmp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

#ifdef SNMP_SUPPORT_V3
cli_cmd_tab_entry(
    VTSS_CLI_GRP_SEC_SWITCH_PATH "SNMP Engine ID",
    VTSS_CLI_GRP_SEC_SWITCH_PATH "SNMP Engine ID [<engineid>]",
    "Set or show SNMPv3 local engine ID",
    PRIO_SNMPV3_ENGINE_ID,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SNMP,
    SNMPV3_cli_cmd_engineid_conf,
    NULL,
    snmp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    NULL,
    VTSS_CLI_GRP_SEC_SWITCH_PATH "SNMP Community Add <community> [<ip_addr>] [<ip_mask>]",
    "Add or modify SNMPv3 community entry.\n"
    "The entry index key is <community>",
    PRIO_SNMPV3_COMMUNITY_ADD,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SNMP,
    SNMPV3_cli_cmd_community_add,
    NULL,
    snmp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    NULL,
    VTSS_CLI_GRP_SEC_SWITCH_PATH "SNMP Community Delete <index>",
    "Delete SNMPv3 community entry",
    PRIO_SNMPV3_COMMUNITY_DEL,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SNMP,
    SNMPV3_cli_cmd_community_del,
    NULL,
    snmp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    VTSS_CLI_GRP_SEC_SWITCH_PATH "SNMP Community Lookup [<index>]",
    NULL,
    "Lookup SNMPv3 community entry",
    PRIO_SNMPV3_COMMUNITY_LOOKUP,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_SNMP,
    SNMPV3_cli_cmd_community_lookup,
    NULL,
    snmp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    NULL,
    VTSS_CLI_GRP_SEC_SWITCH_PATH "SNMP User Add <engineid> <user_name> [MD5|SHA]\n"
    "        [<auth_password>] [DES|AES] [<priv_password>]",
    "Add SNMPv3 user entry.\n"
    "The entry index key are <engineid> and <user_name> and it doesn't allow modify",
    PRIO_SNMPV3_USER_ADD,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SNMP,
    SNMPV3_cli_cmd_user_add,
    NULL,
    snmp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    NULL,
    VTSS_CLI_GRP_SEC_SWITCH_PATH "SNMP User Delete <index>",
    "Delete SNMPv3 user entry",
    PRIO_SNMPV3_USER_DEL,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SNMP,
    SNMPV3_cli_cmd_user_del,
    NULL,
    snmp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    NULL,
    VTSS_CLI_GRP_SEC_SWITCH_PATH "SNMP User Changekey <engineid> <user_name>\n"
    "        <auth_password> [<priv_password>]",
    "Change SNMPv3 user password",
    PRIO_SNMPV3_USER_CHANGE_PASSWORD,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SNMP,
    SNMPV3_cli_cmd_user_change_password,
    NULL,
    snmp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    VTSS_CLI_GRP_SEC_SWITCH_PATH "SNMP User Lookup [<index>]",
    NULL,
    "Lookup SNMPv3 user entry",
    PRIO_SNMPV3_USER_LOOKUP,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_SNMP,
    SNMPV3_cli_cmd_user_lookup,
    NULL,
    snmp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    NULL,
    VTSS_CLI_GRP_SEC_SWITCH_PATH "SNMP Group Add <security_model> <security_name> <group_name>",
    "Add or modify SNMPv3 group entry.\n"
    "The entry index key are <security_model> and <security_name>",
    PRIO_SNMPV3_GROUP_ADD,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SNMP,
    SNMPV3_cli_cmd_group_add,
    NULL,
    snmp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    NULL,
    VTSS_CLI_GRP_SEC_SWITCH_PATH "SNMP Group Delete <index>",
    "Delete SNMPv3 group entry",
    PRIO_SNMPV3_GROUP_DEL,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SNMP,
    SNMPV3_cli_cmd_group_del,
    NULL,
    snmp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    VTSS_CLI_GRP_SEC_SWITCH_PATH "SNMP Group Lookup [<index>]",
    NULL,
    "Lookup SNMPv3 group entry",
    PRIO_SNMPV3_GROUP_LOOKUP,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_SNMP,
    SNMPV3_cli_cmd_group_lookup,
    NULL,
    snmp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    NULL,
    VTSS_CLI_GRP_SEC_SWITCH_PATH "SNMP View Add <view_name> [included|excluded] <oid_subtree>",
    "Add or modify SNMPv3 view entry.\n"
    "The entry index key are <view_name> and <oid_subtree>",
    PRIO_SNMPV3_VIEW_ADD,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SNMP,
    SNMPV3_cli_cmd_view_add,
    NULL,
    snmp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    NULL,
    VTSS_CLI_GRP_SEC_SWITCH_PATH "SNMP View Delete <index>",
    "Delete SNMPv3 view entry",
    PRIO_SNMPV3_VIEW_DEL,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SNMP,
    SNMPV3_cli_cmd_view_del,
    NULL,
    snmp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    VTSS_CLI_GRP_SEC_SWITCH_PATH "SNMP View Lookup [<index>]",
    NULL,
    "Lookup SNMPv3 view entry",
    PRIO_SNMPV3_VIEW_LOOKUP,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_SNMP,
    SNMPV3_cli_cmd_view_lookup,
    NULL,
    snmp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    NULL,
    VTSS_CLI_GRP_SEC_SWITCH_PATH "SNMP Access Add <group_name> <security_model> <security_level>\n"
    "        [<read_view_name>] [<write_view_name>]",
    "Add or modify SNMPv3 access entry.\n"
    "The entry index key are <group_name>, <security_model> and <security_level>",
    PRIO_SNMPV3_ACCESS_ADD,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SNMP,
    SNMPV3_cli_cmd_access_add,
    NULL,
    snmp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    NULL,
    VTSS_CLI_GRP_SEC_SWITCH_PATH "SNMP Access Delete <index>",
    "Delete SNMPv3 access entry",
    PRIO_SNMPV3_ACCESS_DEL,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SNMP,
    SNMPV3_cli_cmd_access_del,
    NULL,
    snmp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    VTSS_CLI_GRP_SEC_SWITCH_PATH "SNMP Access Lookup [<index>]",
    NULL,
    "Lookup SNMPv3 access entry",
    PRIO_SNMPV3_ACCESS_LOOKUP,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_SNMP,
    SNMPV3_cli_cmd_access_lookup,
    NULL,
    snmp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif /* SNMP_SUPPORT_V3 */

cli_cmd_tab_entry(
    VTSS_CLI_GRP_SEC_SWITCH_PATH "SNMP Trap Mode",
    VTSS_CLI_GRP_SEC_SWITCH_PATH "SNMP Trap Mode [enable|disable]",
    "Set or show the SNMP trap mode",
    TRAP_MODE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SNMP,
    trap_cli_cmd_trap_global_mode,
    NULL,
    snmp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    VTSS_CLI_GRP_SEC_SWITCH_PATH "SNMP Trap Lookup [<conf_name>]",
    NULL,
    "Show the SNMP trap configuration",
    TRAP_CONF_LOOKUP,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_SNMP,
    trap_cli_cmd_trap_conf_lookup,
    NULL,
    snmp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);


cli_cmd_tab_entry(
    VTSS_CLI_GRP_SEC_SWITCH_PATH "SNMP Trap Add <conf_name>\n"
    "        [enable|disable]\n"
    "        [(dip <ipv4v6_addr>)] [(dport <udp_port>)]\n"
    "        [((1) [(community <comm>)]) |\n"
    "         (((2c) [(community <comm>)]) [(trap) | (informs [<retries>] [<timeout>])])] |\n"
    "         ((3) [(trap) | (informs [<retries>] [<timeout>])] [(probe) | (engine <engineid>)] [(security <security_name>)])]"
    ,
    NULL,
    "Set or add the SNMP trap configuration\n\n",
    TRAP_CONF_ADD,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SNMP,
    trap_cli_cmd_trap_conf_add,
    NULL,
    snmp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    VTSS_CLI_GRP_SEC_SWITCH_PATH "SNMP Trap Delete <conf_name>",
    NULL,
    "Delete the SNMP trap configuration",
    TRAP_CONF_DEL,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SNMP,
    trap_cli_cmd_trap_conf_delete,
    NULL,
    snmp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    VTSS_CLI_GRP_SEC_SWITCH_PATH "SNMP Trap Event Lookup [<conf_name>]",
    NULL,
    "Show the SNMP trap event configuration",
    TRAP_EVENT_LOOKUP,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_SNMP,
    trap_cli_cmd_trap_event_lookup,
    NULL,
    snmp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    VTSS_CLI_GRP_SEC_SWITCH_PATH "SNMP Trap Event System Warm-start [<conf_name>] [enable|disable]",
    NULL,
    "Show and set the SNMP trap Warm-start event",
    TRAP_EVENT_SYSTEM_WARM_START,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SNMP,
    trap_cli_cmd_trap_event_warmstart,
    NULL,
    snmp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    VTSS_CLI_GRP_SEC_SWITCH_PATH "SNMP Trap Event System Cold-start [<conf_name>] [enable|disable]",
    NULL,
    "Show and set the SNMP trap Cold-start event",
    TRAP_EVENT_SYSTEM_COLD_START,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SNMP,
    trap_cli_cmd_trap_event_coldstart,
    NULL,
    snmp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    VTSS_CLI_GRP_SEC_SWITCH_PATH "SNMP Trap Event Interface Link-up [<conf_name>] [<port_list>] [enable|disable]",
    NULL,
    "Show and set the SNMP trap Link-up event",
    TRAP_EVENT_INTERFACE_LINKUP,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SNMP,
    trap_cli_cmd_trap_event_linkup,
    NULL,
    snmp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    VTSS_CLI_GRP_SEC_SWITCH_PATH "SNMP Trap Event Interface Link-down [<conf_name>] [<port_list>] [enable|disable]",
    NULL,
    "Show and set the SNMP trap Link-down event",
    TRAP_EVENT_INTERFACE_LINKDOWN,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SNMP,
    trap_cli_cmd_trap_event_linkdown,
    NULL,
    snmp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    VTSS_CLI_GRP_SEC_SWITCH_PATH "SNMP Trap Event Interface LLDP [<conf_name>] [enable|disable]",
    NULL,
    "Show and set the SNMP trap LLDP event",
    TRAP_EVENT_INTERFACE_LLDP,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SNMP,
    trap_cli_cmd_trap_event_lldp,
    NULL,
    snmp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    VTSS_CLI_GRP_SEC_SWITCH_PATH "SNMP Trap Event AAA Authentication-Failure",
    VTSS_CLI_GRP_SEC_SWITCH_PATH "SNMP Trap Event AAA Authentication-Failure [<conf_name>] [enable|disable]",
    "Show and set the SNMP trap Authentication-Failure event",
    TRAP_EVENT_AAA_AUTH_FAIL,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SNMP,
    trap_cli_cmd_trap_event_auth_fail,
    NULL,
    snmp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    VTSS_CLI_GRP_SEC_SWITCH_PATH "SNMP Trap Event Switch RMON [<conf_name>] [enable|disable]",
    NULL,
    "Show and set the SNMP trap RMON event",
    TRAP_EVENT_SWITCH_RMON,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SNMP,
    trap_cli_cmd_trap_event_rmon,
    NULL,
    snmp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    VTSS_CLI_GRP_SEC_SWITCH_PATH "SNMP Trap Event Switch STP [<conf_name>] [enable|disable]",
    NULL,
    "Show and set the SNMP trap STP event",
    TRAP_EVENT_SWITCH_STP,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SNMP,
    trap_cli_cmd_trap_event_stp,
    NULL,
    snmp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);


/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/

