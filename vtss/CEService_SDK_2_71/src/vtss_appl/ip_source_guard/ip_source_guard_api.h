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

#ifndef _VTSS_IP_SOURCE_GUARD_API_H_
#define _VTSS_IP_SOURCE_GUARD_API_H_

#include "vtss_api.h" /* For vtss_rc, vtss_vid_t, etc. */

/* IP_SOURCE_GUARD managent enabled/disabled */
#define IP_SOURCE_GUARD_MGMT_ENABLED            1
#define IP_SOURCE_GUARD_MGMT_DISABLED           0

//#define IP_SOURCE_GUARD_MAX_STATIC_CNT          28
//#define IP_SOURCE_GUARD_MAX_DYNAMIC_CNT         84
#define IP_SOURCE_GUARD_MAX_ENTRY_CNT           (112) // reserved 16 entry for system trap

#define IP_SOURCE_GUARD_DYNAMIC_UNLIMITED       0XFFFF

/**
 * \brief API Error Return Codes (vtss_rc)
 */
enum {
    IP_SOURCE_GUARD_ERROR_MUST_BE_MASTER = MODULE_ERROR_START(VTSS_MODULE_ID_IP_SOURCE_GUARD),  /**< Operation is only allowed on the master switch. */
    IP_SOURCE_GUARD_ERROR_ISID,                                                                 /**< isid parameter is invalid.                      */
    IP_SOURCE_GUARD_ERROR_ISID_NON_EXISTING,                                                    /**< isid parameter is non-existing.                 */
    IP_SOURCE_GUARD_ERROR_INV_PARAM,                                                            /**< Invalid parameter.                              */
    IP_SOURCE_GUARD_ERROR_STATIC_TABLE_FULL,                                                    /**< IP source guard static table full.              */
    IP_SOURCE_GUARD_ERROR_DYNAMIC_TABLE_FULL,                                                   /**< IP source guard dynamic table full.             */
    IP_SOURCE_GUARD_ERROR_ACE_AUTO_ASSIGNED_FAIL,                                               /**< IP source guard ACE auto-assigned fail.         */
    IP_SOURCE_GUARD_ERROR_DATABASE_ACCESS,                                                      /**< Databse access error.                           */
    IP_SOURCE_GUARD_ERROR_DATABASE_CREATE,                                                      /**< Databse create error.                           */
    IP_SOURCE_GUARD_ERROR_ENTRY_EXIST_ON_DB,                                                    /**< The entry exist on DB.                          */
    IP_SOURCE_GUARD_ERROR_DATABASE_ADD,                                                         /**< The entry insert error on DB.                   */
    IP_SOURCE_GUARD_ERROR_DATABASE_DEL,                                                         /**< The entry delete error on DB.                   */
    IP_SOURCE_GUARD_ERROR_LOAD_CONF                                                             /**< Open configuration error.                       */
};

/**
 * Default configuration
 */
#define IP_SOURCE_GUARD_DEFAULT_MODE                IP_SOURCE_GUARD_MGMT_DISABLED       /**< Default global mode         */
#define IP_SOURCE_GUARD_DEFAULT_PORT_MODE           IP_SOURCE_GUARD_MGMT_DISABLED       /**< Default port mode           */
#define IP_SOURCE_GUARD_DEFAULT_DYNAMIC_ENTRY_CNT   IP_SOURCE_GUARD_DYNAMIC_UNLIMITED   /**< Default dynamic entry count */

/* IP source guard entry type */
typedef enum {
    IP_SOURCE_GUARD_STATIC_TYPE,
    IP_SOURCE_GUARD_DYNAMIC_TYPE
} ip_source_guard_entry_type_t;

typedef struct {
    vtss_vid_t                      vid;
    uchar                           assigned_mac[6];
    vtss_ipv4_t                     assigned_ip;
    vtss_ipv4_t                     ip_mask;
    vtss_isid_t                     isid;
    vtss_port_no_t                  port_no;
    ip_source_guard_entry_type_t    type;
    vtss_ace_id_t                   ace_id;
    ulong                           valid;
} ip_source_guard_entry_t;

typedef struct {
    ulong   mode[VTSS_PORTS];
} ip_source_guard_port_mode_conf_t;

typedef struct {
    ulong   entry_cnt[VTSS_PORTS];
} ip_source_guard_port_dynamic_entry_conf_t;

/* IP_SOURCE_GUARD configuration */
typedef struct {
    ulong                                       mode; /* IP Source Guard Global Mode */
    ip_source_guard_port_mode_conf_t            port_mode_conf[VTSS_ISID_CNT];
    ip_source_guard_port_dynamic_entry_conf_t   port_dynamic_entry_conf[VTSS_ISID_CNT];
    ip_source_guard_entry_t                     ip_source_guard_static_entry[IP_SOURCE_GUARD_MAX_ENTRY_CNT];
} ip_source_guard_conf_t;

/* Set IP_SOURCE_GUARD defaults */
void ip_source_guard_default_set(ip_source_guard_conf_t *conf);

/* IP_SOURCE_GUARD error text */
char *ip_source_guard_error_txt(vtss_rc rc);

/* Get IP_SOURCE_GUARD configuration */
vtss_rc ip_source_guard_mgmt_conf_get(ip_source_guard_conf_t *conf);

/* Set IP_SOURCE_GUARD configuration */
vtss_rc ip_source_guard_mgmt_conf_set(ip_source_guard_conf_t *conf);

/* Get IP_SOURCE_GUARD mode */
vtss_rc ip_source_guard_mgmt_conf_get_mode(ulong *mode);

/* Set IP_SOURCE_GUARD mode */
vtss_rc ip_source_guard_mgmt_conf_set_mode(ulong mode);

/* Set IP_SOURCE_GUARD static entry */
vtss_rc ip_source_guard_mgmt_conf_set_static_entry(ip_source_guard_entry_t *entry);

/* Get first IP_SOURCE_GUARD static entry */
vtss_rc ip_source_guard_mgmt_conf_get_first_static_entry(ip_source_guard_entry_t *entry);

/* Get Next IP_SOURCE_GUARD static entry */
vtss_rc ip_source_guard_mgmt_conf_get_next_static_entry(ip_source_guard_entry_t *entry);

/* Delete IP_SOURCE_GUARD static entry */
vtss_rc ip_source_guard_mgmt_conf_del_static_entry(ip_source_guard_entry_t *entry);

/* Get first IP_SOURCE_GUARD dynamic entry */
vtss_rc ip_source_guard_mgmt_conf_get_first_dynamic_entry(ip_source_guard_entry_t *entry);

/* Get Next IP_SOURCE_GUARD dynamic entry */
vtss_rc ip_source_guard_mgmt_conf_get_next_dynamic_entry(ip_source_guard_entry_t *entry);

/* del all IP_SOURCE_GUARD static entry */
vtss_rc ip_source_guard_mgmt_conf_del_all_static_entry(void);

/* set IP_SOURCE_GUARD port mode */
vtss_rc ip_source_guard_mgmt_conf_set_port_mode(vtss_isid_t isid, ip_source_guard_port_mode_conf_t *port_mode_conf);

/* get IP_SOURCE_GUARD port mode */
vtss_rc ip_source_guard_mgmt_conf_get_port_mode(vtss_isid_t isid, ip_source_guard_port_mode_conf_t *port_mode_conf);

/* set IP_SOURCE_GUARD port dynamic entry count */
vtss_rc ip_source_guard_mgmt_conf_set_port_dynamic_entry_cnt(vtss_isid_t isid, ip_source_guard_port_dynamic_entry_conf_t *port_dynamic_entry_conf);

/* get IP_SOURCE_GUARD port dynamic entry count */
vtss_rc ip_source_guard_mgmt_conf_get_port_dynamic_entry_cnt(vtss_isid_t isid, ip_source_guard_port_dynamic_entry_conf_t *port_dynamic_entry_conf);

/* Translate IP_SOURCE_GUARD dynamic entries into static entries */
vtss_rc ip_source_guard_mgmt_conf_translate_dynamic_into_static(void);

/* Initialize module */
vtss_rc ip_source_guard_init(vtss_init_data_t *data);

#endif /* _VTSS_IP_SOURCE_GUARD_API_H_ */


// ***************************************************************************
//
//  End of file.
//
// ***************************************************************************
