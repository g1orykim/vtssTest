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

/*
******************************************************************************

    Include files

******************************************************************************
*/
#include "icfg_api.h"
#include "vtss_snmp_api.h"
#include "snmp_icfg.h"
#include "misc_api.h"   //uport2iport(), iport2uport()
#include "cli.h"
/*
******************************************************************************

    Constant and Macro definition

******************************************************************************
*/

#undef  VTSS_ALLOC_MODULE_ID
#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_ICFG

/*
******************************************************************************

    Data structure type definition

******************************************************************************
*/

/*
******************************************************************************

    Static Function

******************************************************************************
*/

static void system_default_get(system_conf_t *conf)
{
    conf->sys_contact[0] = 0;
    conf->sys_name[0] = 0;
    conf->sys_location[0] = 0;
}

#ifdef SNMP_SUPPORT_V3
static BOOL snmpv3_mgmt_default_communities_check (snmpv3_communities_conf_t *conf, snmpv3_communities_conf_t *def_conf, u32 num )
{
    i32 i = 0;
    BOOL found = FALSE;
    snmpv3_communities_conf_t *ptr;

    for (i = 0, ptr = def_conf ; i < (i32) num; i++, ptr++) {
        if ( 0 == snmpv3_communities_conf_changed (ptr, conf) ) {
            found = TRUE;
            break;
        }
    }

    return found;
}

static vtss_rc SNMPV3_ICFG_community_conf(const vtss_icfg_query_request_t *req,
                                          vtss_icfg_query_result_t *result)
{
    vtss_rc             rc = VTSS_OK;
    i8                  ip_buf[16], mask_buf[16];
    snmpv3_communities_conf_t conf, *def_conf, *ptr;
    u32        num;
    i32        i = 0;

    snmpv3_default_communities_get( &num, NULL);
    if ((def_conf = VTSS_MALLOC(sizeof(conf) * num)) == NULL) {
        return VTSS_RC_ERROR;
    }
    snmpv3_default_communities_get ( NULL, def_conf);

    strcpy(conf.community, "");
    while ( snmpv3_mgmt_communities_conf_get(&conf, TRUE) == VTSS_OK ) {

        if ( TRUE == snmpv3_mgmt_default_communities_check(&conf, def_conf, num) && !req->all_defaults ) {
            continue;
        }

        /* COMMAND = snmp-server community v3 <word127> [ <ipv4_addr> <ipv4_netmask> ] */
        /* COMMAND = no snmp-server community v3 <word127>  */
        rc = vtss_icfg_printf(result, "%s%s %s %s\n",
                              "snmp-server community v3 ",
                              conf.community, misc_ipv4_txt(conf.sip, ip_buf), misc_ipv4_txt(conf.sip_mask, mask_buf));
    }

    for (i = 0, ptr = def_conf; i < (i32)num && ptr != NULL; i++, ptr++) {
        if ( VTSS_RC_OK != snmpv3_mgmt_communities_conf_get(ptr, FALSE)) {
            rc = vtss_icfg_printf( result, "no snmp-server community v3 %s\n", ptr->community);
        }
    }

    VTSS_FREE(def_conf);

    return rc;
}

static vtss_rc SNMPV3_ICFG_user_conf(const vtss_icfg_query_request_t *req,
                                     vtss_icfg_query_result_t *result)
{
    vtss_rc    rc = VTSS_OK;
    i8         cmd[256];
    snmpv3_users_conf_t conf, def_conf;

    snmpv3_default_users_get ( NULL, &def_conf);
    /* only non-authitication users can be shown in running config */
    /* command: snmp-server
        snmp-server user <username:word32> engine-id <engineID:word10-32>
        no snmp-server user <username:word32> engine-id <engineID:word10-32>
       */

    strcpy(conf.user_name, "");
    while ( snmpv3_mgmt_users_conf_get(&conf, TRUE) == VTSS_OK ) {

        if ( (0 == snmpv3_users_conf_changed( &conf, &def_conf) && !req->all_defaults) || (conf.auth_protocol != SNMP_MGMT_AUTH_PROTO_NONE) ) {
            continue;
        }

        sprintf( cmd, "snmp-server user %s engine-id %s ", conf.user_name, misc_engineid2str(conf.engineid, conf.engineid_len));

        rc = vtss_icfg_printf(result, "%s\n", cmd);
    }

    if ( VTSS_RC_OK != snmpv3_mgmt_users_conf_get(&def_conf, FALSE)) {
        sprintf( cmd, "no snmp-server user %s engine-id %s ", def_conf.user_name, misc_engineid2str(def_conf.engineid, def_conf.engineid_len));
        rc = vtss_icfg_printf(result, "%s\n", cmd);
    }

    return rc;
}

static BOOL snmpv3_mgmt_default_groups_check (snmpv3_groups_conf_t *conf, snmpv3_groups_conf_t *def_conf, u32 num )
{
    i32 i = 0;
    BOOL found = FALSE;
    snmpv3_groups_conf_t *ptr;

    for (i = 0, ptr = def_conf ; i < (i32) num; i++, ptr++) {
        if ( 0 == snmpv3_groups_conf_changed (ptr, conf) ) {
            found = TRUE;
            break;
        }
    }

    return found;
}

static vtss_rc SNMPV3_ICFG_group_conf(const vtss_icfg_query_request_t *req,
                                      vtss_icfg_query_result_t *result)
{
    vtss_rc    rc = VTSS_OK;
    i8         cmd[256];
    snmpv3_groups_conf_t conf, *def_conf, *ptr;
    u32        num;
    i32        i = 0;

    snmpv3_default_groups_get( &num, NULL);
    if ((def_conf = VTSS_MALLOC(sizeof(conf) * num)) == NULL) {
        return VTSS_RC_ERROR;
    }
    snmpv3_default_groups_get ( NULL, def_conf);
    /* command: snmp-server
        snmp-server security-to-group model { v1 | v2c | v3 } name <word32> group <word32>
        no snmp-server security-to-group model { v1 | v2c | v3 } name <word32>
       */

    conf.security_model = 0;
    strcpy(conf.security_name, "");
    while ( snmpv3_mgmt_groups_conf_get(&conf, TRUE) == VTSS_OK ) {
        sprintf( cmd, "snmp-server security-to-group model %s name %s group %s",
                 conf.security_model == SNMP_MGMT_SEC_MODEL_SNMPV1 ? "v1" : conf.security_model == SNMP_MGMT_SEC_MODEL_SNMPV2C ? "v2c" : "v3",
                 conf.security_name, conf.group_name);

        if ( TRUE == snmpv3_mgmt_default_groups_check(&conf, def_conf, num) && !req->all_defaults ) {
            continue;
        }

        rc = vtss_icfg_printf(result, "%s\n", cmd);
    }

    for (i = 0, ptr = def_conf; i < (i32)num && ptr != NULL; i++, ptr++) {
        if ( VTSS_RC_OK != snmpv3_mgmt_groups_conf_get(ptr, FALSE)) {
            sprintf( cmd, "no snmp-server security-to-group model %s name %s",
                     ptr->security_model == SNMP_MGMT_SEC_MODEL_SNMPV1 ? "v1" : ptr->security_model == SNMP_MGMT_SEC_MODEL_SNMPV2C ? "v2c" : "v3",
                     ptr->security_name);

            rc = vtss_icfg_printf(result, "%s\n", cmd);
        }
    }

    VTSS_FREE(def_conf);
    return rc;
}

static BOOL snmpv3_mgmt_default_accesses_check (snmpv3_accesses_conf_t *conf, snmpv3_accesses_conf_t *def_conf, u32 num )
{
    i32 i = 0;
    BOOL found = FALSE;
    snmpv3_accesses_conf_t *ptr;

    for (i = 0, ptr = def_conf ; i < (i32) num; i++, ptr++) {
        if ( 0 == snmpv3_accesses_conf_changed (ptr, conf) ) {
            found = TRUE;
            break;
        }
    }

    return found;
}

static BOOL snmpv3_mgmt_default_views_check (snmpv3_views_conf_t *conf, snmpv3_views_conf_t *def_conf, u32 num )
{
    i32 i = 0;
    BOOL found = FALSE;
    snmpv3_views_conf_t *ptr;

    for (i = 0, ptr = def_conf ; i < (i32) num; i++, ptr++) {
        if ( 0 == snmpv3_views_conf_changed (ptr, conf) ) {
            found = TRUE;
            break;
        }
    }

    return found;
}

static vtss_rc SNMPV3_ICFG_access_conf(const vtss_icfg_query_request_t *req,
                                       vtss_icfg_query_result_t *result)
{
    vtss_rc    rc = VTSS_OK;
    i8         cmd[256];
    snmpv3_accesses_conf_t conf, *def_conf, *ptr;
    u32        num;
    i32        i = 0;

    snmpv3_default_accesses_get( &num, NULL);
    if ((def_conf = VTSS_MALLOC(sizeof(conf) * num)) == NULL) {
        return VTSS_RC_ERROR;
    }
    snmpv3_default_accesses_get ( NULL, def_conf);
    /* command: snmp-server
        snmp-server access <word32> model { v1 | v2c | v3 | any } level { auth | noauth | priv } [ read <word255> ] [ write <word255> ]
        no snmp-server access <word32> model { v1 | v2c | v3 } level { auth | noauth | priv }
       */

    conf.security_model = 0;
    strcpy(conf.group_name, "");
    while ( snmpv3_mgmt_accesses_conf_get(&conf, TRUE) == VTSS_OK ) {
        sprintf( cmd, "snmp-server access %s model %s level %s ", conf.group_name,
                 conf.security_model == SNMP_MGMT_SEC_MODEL_ANY ? "any" : conf.security_model == SNMP_MGMT_SEC_MODEL_SNMPV1 ? "v1" : conf.security_model == SNMP_MGMT_SEC_MODEL_SNMPV2C ? "v2c" : "v3",
                 conf.security_level == SNMP_MGMT_SEC_LEVEL_AUTHNOPRIV ? "auth" : conf.security_level == SNMP_MGMT_SEC_LEVEL_NOAUTH ? "noauth" : "priv");

        if ( strcmp(conf.read_view_name, SNMPV3_NONAME) ) {
            sprintf( cmd, "%s%s%s ", cmd, "read ", conf.read_view_name);
        }

        if ( strcmp(conf.write_view_name, SNMPV3_NONAME) ) {
            sprintf( cmd, "%s%s%s ", cmd, "write ", conf.write_view_name);
        }

        if ( TRUE == snmpv3_mgmt_default_accesses_check(&conf, def_conf, num) && !req->all_defaults ) {
            continue;
        }

        rc = vtss_icfg_printf(result, "%s\n", cmd);
    }

    for (i = 0, ptr = def_conf; i < (i32)num && ptr != NULL; i++, ptr++) {
        if ( VTSS_RC_OK != snmpv3_mgmt_accesses_conf_get(ptr, FALSE)) {
            sprintf( cmd, "no snmp-server access %s model %s level %s ", ptr->group_name,
                     ptr->security_model == SNMP_MGMT_SEC_MODEL_ANY ? "any" : ptr->security_model == SNMP_MGMT_SEC_MODEL_SNMPV1 ? "v1" : ptr->security_model == SNMP_MGMT_SEC_MODEL_SNMPV2C ? "v2c" : "v3",
                     ptr->security_level == SNMP_MGMT_SEC_LEVEL_AUTHNOPRIV ? "auth" : ptr->security_level == SNMP_MGMT_SEC_LEVEL_NOAUTH ? "noauth" : "priv");

            rc = vtss_icfg_printf(result, "%s\n", cmd);
        }
    }

    VTSS_FREE(def_conf);
    return rc;
}

static vtss_rc SNMPV3_ICFG_view_conf (const vtss_icfg_query_request_t *req,
                                      vtss_icfg_query_result_t *result)
{
    vtss_rc    rc = VTSS_OK;
    i8         cmd[256];
    snmpv3_views_conf_t conf, *def_conf, *ptr;
    u32        num;
    i32        i = 0;

    snmpv3_default_views_get( &num, NULL);
    if ((def_conf = VTSS_MALLOC(sizeof(conf) * num)) == NULL) {
        return VTSS_RC_ERROR;
    }
    snmpv3_default_views_get ( NULL, def_conf);
    /* command: snmp-server
        snmp-server view <word32> <word255> { include | exclude }
        no snmp-server view <word32> <word255>
       */

    strcpy(conf.view_name, "");
    while ( snmpv3_mgmt_views_conf_get(&conf, TRUE) == VTSS_OK ) {
        sprintf( cmd, "snmp-server view %s %s %s", conf.view_name,
                 misc_oid2str(conf.subtree, conf.subtree_len, conf.subtree_mask, conf.subtree_mask_len),
                 conf.view_type == SNMPV3_MGMT_VIEW_INCLUDED ? "include" : "exclude");

        if ( TRUE == snmpv3_mgmt_default_views_check(&conf, def_conf, num) && !req->all_defaults ) {
            continue;
        }

        rc = vtss_icfg_printf(result, "%s\n", cmd);
    }

    for (i = 0, ptr = def_conf; i < (i32)num && ptr != NULL; i++, ptr++) {
        if ( VTSS_RC_OK != snmpv3_mgmt_views_conf_get(ptr, FALSE)) {
            sprintf( cmd, "no snmp-server view %s %s", ptr->view_name,
                     misc_oid2str(ptr->subtree, ptr->subtree_len, ptr->subtree_mask, ptr->subtree_mask_len));
            rc = vtss_icfg_printf(result, "%s\n", cmd);
        }
    }

    VTSS_FREE(def_conf);
    return rc;

}
#endif /* SNMP_SUPPORT_V3 */

/* ICFG callback functions */
static vtss_rc SNMP_ICFG_global_conf(const vtss_icfg_query_request_t *req,
                                     vtss_icfg_query_result_t *result)
{
    vtss_rc             rc = VTSS_OK;
    snmp_conf_t         conf, def_conf;
    system_conf_t       sysconf, def_sysconf;

    (void) snmp_mgmt_snmp_conf_get(&conf);
    system_default_get(&def_sysconf);

    (void)system_get_config(&sysconf);
    snmp_default_get(&def_conf);
    /* command: snmp-server
                snmp-server engineID local <engineID:word10-32>
                snmp-server version {v1|v2c|v3}
                snmp-server contact <line255>
                snmp-server location <line255>
       */
    if (req->all_defaults ||
        (conf.mode != def_conf.mode)) {
        rc = vtss_icfg_printf(result, "%s\n",
                              conf.mode == 1 ? "snmp-server" : "no snmp-server");
    }

#ifdef SNMP_SUPPORT_V3
    if (req->all_defaults ||
        (conf.engineid_len != def_conf.engineid_len) ||
        memcmp(conf.engineid, def_conf.engineid, def_conf.engineid_len) ) {
        rc = vtss_icfg_printf(result, "%s%s\n",
                              "snmp-server engine-id local ", misc_engineid2str(conf.engineid, conf.engineid_len));
    }
#endif /* SNMP_SUPPORT_V3 */

    if (req->all_defaults ||
        (conf.version != def_conf.version)) {
        rc = vtss_icfg_printf(result, "%s%s\n",
                              "snmp-server version ", (conf.version == SNMP_SUPPORT_V1) ? "v1" : (conf.version == SNMP_SUPPORT_V2C) ? "v2c" : "v3");
    }

    if (req->all_defaults ||
        (strcmp(sysconf.sys_contact, def_sysconf.sys_contact))) {
        rc = vtss_icfg_printf(result, "%s%s%s\n",
                              "snmp-server contact ", req->all_defaults ? "no" : "", sysconf.sys_contact);
    }

    if (req->all_defaults ||
        (strcmp(sysconf.sys_location, def_sysconf.sys_location))) {
        rc = vtss_icfg_printf(result, "%s%s%s\n",
                              "snmp-server location ", req->all_defaults ? "no" : "", sysconf.sys_location);
    }

    if (req->all_defaults ||
        (strcmp(conf.read_community, def_conf.read_community))) {
        rc = vtss_icfg_printf(result, "%s%s %s\n",
                              "snmp-server community v2c ", conf.read_community, "RO");
    }

    if (req->all_defaults ||
        (strcmp(conf.write_community, def_conf.write_community))) {
        rc = vtss_icfg_printf(result, "%s%s %s\n",
                              "snmp-server community v2c ", conf.write_community, "RW");
    }

#ifdef SNMP_SUPPORT_V3
    rc = SNMPV3_ICFG_community_conf(req, result);

    rc = SNMPV3_ICFG_user_conf(req, result);
    rc = SNMPV3_ICFG_group_conf(req, result);
    rc = SNMPV3_ICFG_view_conf(req, result);
    rc = SNMPV3_ICFG_access_conf(req, result);
#endif /* SNMP_SUPPORT_V3 */

    return rc;
}

/*
******************************************************************************

    Public functions

******************************************************************************
*/

/* Initialization function */
vtss_rc snmp_icfg_init(void)
{
    vtss_rc rc;

    /* Register callback functions to ICFG module.
       The configuration divided into two groups for this module.
       1. Global configuration
       2. Port configuration
    */
    if ((rc = vtss_icfg_query_register(VTSS_ICFG_SNMP_GLOBAL_CONF, "snmp", SNMP_ICFG_global_conf)) != VTSS_OK) {
        return rc;
    }

    return rc;
}
