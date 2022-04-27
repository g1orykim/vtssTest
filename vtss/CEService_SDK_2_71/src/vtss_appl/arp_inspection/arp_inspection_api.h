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

#ifndef _VTSS_ARP_INSPECTION_API_H_
#define _VTSS_ARP_INSPECTION_API_H_

#include "vlan_api.h"

/* ARP_INSPECTION managent enabled/disabled */
#define ARP_INSPECTION_MGMT_ENABLED         (1)     // untrust port
#define ARP_INSPECTION_MGMT_DISABLED        (0)     // trust port

#define ARP_INSPECTION_MGMT_VLAN_ENABLED    (1)
#define ARP_INSPECTION_MGMT_VLAN_DISABLED   (0)

//#define ARP_INSPECTION_MAX_STATIC_CNT       (64)
//#define ARP_INSPECTION_MAX_DYNAMIC_CNT      (1024)
#define ARP_INSPECTION_MAX_ENTRY_CNT      (256) // the default value of max entries is 256.

/**
 * \brief ARP inspection frame information maximum entry count
 */
#define ARP_INSPECTION_FRAME_INFO_MAX_CNT          1024

#define ARP_INSPECTION_PROCESS_VLAN_TAGGING_ISSUE   1

/* ARP inspection entry type */
typedef enum {
    ARP_INSPECTION_STATIC_TYPE,
    ARP_INSPECTION_DYNAMIC_TYPE
} arp_inspection_entry_type_t;

/* ARP inspection vlan mode & log type */
#define ARP_INSPECTION_VLAN_MODE            0x1
//#define ARP_INSPECTION_VLAN_LOG_NONE        0x2
#define ARP_INSPECTION_VLAN_LOG_DENY        0x2
#define ARP_INSPECTION_VLAN_LOG_PERMIT      0x4
//#define ARP_INSPECTION_VLAN_LOG_ALL         0x10

/* ARP inspection log type */
typedef enum {
    ARP_INSPECTION_LOG_NONE,
    ARP_INSPECTION_LOG_DENY,
    ARP_INSPECTION_LOG_PERMIT,
    ARP_INSPECTION_LOG_ALL
} arp_inspection_log_type_t;

/**
 * \brief API Error Return Codes (vtss_rc)
 */
enum {
    ARP_INSPECTION_ERROR_MUST_BE_MASTER = MODULE_ERROR_START(VTSS_MODULE_ID_ARP_INSPECTION),    /**< Operation is only allowed on the master switch. */
    ARP_INSPECTION_ERROR_ISID,                                                                  /**< isid parameter is invalid.                      */
    ARP_INSPECTION_ERROR_ISID_NON_EXISTING,                                                     /**< isid parameter is non-existing.                 */
    ARP_INSPECTION_ERROR_INV_PARAM,                                                             /**< Invalid parameter.                              */
    ARP_INSPECTION_ERROR_DATABASE_NOT_FOUND,                                                    /**< Databse access error.                           */
    ARP_INSPECTION_ERROR_ENTRY_EXIST_ON_DB,                                                     /**< The entry exist on the DB.                      */
    ARP_INSPECTION_ERROR_TABLE_FULL,                                                            /**< Table full.                                     */
    ARP_INSPECTION_ERROR_DATABASE_ADD,                                                          /**< The entry insert error on DB.                   */
    ARP_INSPECTION_ERROR_DATABASE_DEL,                                                          /**< The entry delete error on DB.                   */
    ARP_INSPECTION_ERROR_DATABASE_CREATE,                                                       /**< create database error.                          */
    ARP_INSPECTION_ERROR_LOAD_CONF                                                              /**< Open configuration error.                       */
};

/**
 * Default configuration
 */
#define ARP_INSPECTION_DEFAULT_MODE                 ARP_INSPECTION_MGMT_DISABLED                /**< Default global mode      */
#define ARP_INSPECTION_DEFAULT_PORT_MODE            ARP_INSPECTION_MGMT_DISABLED                /**< Default port mode        */
#define ARP_INSPECTION_DEFAULT_PORT_VLAN_MODE       ARP_INSPECTION_MGMT_VLAN_DISABLED           /**< Default port vlan mode   */
#define ARP_INSPECTION_DEFAULT_LOG_TYPE             ARP_INSPECTION_LOG_NONE                     /**< Default port vlan mode   */

typedef struct {
    u32                         isid;
    u32                         port_no;
    vtss_ipv4_t                 assigned_ip;
    vtss_vid_t                  vid;
    u8                          mac[6];
    arp_inspection_entry_type_t type;
    BOOL                        valid;
} arp_inspection_entry_t;

typedef struct {
    u8                          mode[VTSS_PORTS];
    u8                          check_VLAN[VTSS_PORTS];
    arp_inspection_log_type_t   log_type[VTSS_PORTS];
} arp_inspection_port_mode_conf_t;

typedef struct {
    u8              flags;                                                  /* mode, log_type; */
} arp_inspection_vlan_mode_conf_t;

/* ARP_INSPECTION configuration */
typedef struct {
    u32                             mode;                                                       /* ARP_INSPECTION Mode */
    arp_inspection_vlan_mode_conf_t vlan_mode_conf[VTSS_VIDS];
    arp_inspection_port_mode_conf_t port_mode_conf[VTSS_ISID_CNT];
    arp_inspection_entry_t          arp_inspection_static_entry[ARP_INSPECTION_MAX_ENTRY_CNT];
} arp_inspection_conf_t;

/* ARP_INSPECTION configuration for stacking */
typedef struct {
    u32                             mode;                                                       /* ARP_INSPECTION Mode */
    arp_inspection_port_mode_conf_t port_mode_conf[VTSS_ISID_CNT];
} arp_inspection_stacking_conf_t;

/* Set ARP_INSPECTION defaults */
void arp_inspection_default_set(arp_inspection_conf_t *conf);

/* ARP_INSPECTION error text */
char *arp_inspection_error_txt(vtss_rc rc);

/* Get ARP_INSPECTION configuration */
vtss_rc arp_inspection_mgmt_conf_get(arp_inspection_conf_t *conf);

/* Set ARP_INSPECTION configuration */
vtss_rc arp_inspection_mgmt_conf_set(arp_inspection_conf_t *conf);

/* Get ARP_INSPECTION mode */
vtss_rc arp_inspection_mgmt_conf_mode_get(u32 *mode);

/* Set ARP_INSPECTION mode */
vtss_rc arp_inspection_mgmt_conf_mode_set(u32 *mode);

/* Set ARP_INSPECTION static entry */
vtss_rc arp_inspection_mgmt_conf_static_entry_set(arp_inspection_entry_t *entry);

/* Get ARP_INSPECTION static entry */
vtss_rc arp_inspection_mgmt_conf_static_entry_get(arp_inspection_entry_t *entry, BOOL next);

/* Delete ARP_INSPECTION static entry */
vtss_rc arp_inspection_mgmt_conf_static_entry_del(arp_inspection_entry_t *entry);

/* Set ARP_INSPECTION dynamic entry */
vtss_rc arp_inspection_mgmt_conf_dynamic_entry_set(arp_inspection_entry_t *entry);

/* Get ARP_INSPECTION dynamic entry */
vtss_rc arp_inspection_mgmt_conf_dynamic_entry_get(arp_inspection_entry_t *entry, BOOL next);

/* Delete ARP_INSPECTION dynamic entry */
vtss_rc arp_inspection_mgmt_conf_dynamic_entry_del(arp_inspection_entry_t *entry);

/* Check ARP_INSPECTION dynamic entry */
vtss_rc arp_inspection_mgmt_conf_dynamic_entry_check(arp_inspection_entry_t *check_entry);

/* Get ARP_INSPECTION public key fingerprint
   type 0: RSA
   type 1: DSS */
void arp_inspection_mgmt_publickey_get(int type, unsigned char *str_buff);

/* del all ARP_INSPECTION static entry */
vtss_rc arp_inspection_mgmt_conf_all_static_entry_del(void);

/* Reset ARP_INSPECTION VLAN database */
vtss_rc arp_inspection_mgmt_conf_vlan_entry_del(void);

/* set ARP_INSPECTION port mode */
vtss_rc arp_inspection_mgmt_conf_port_mode_set(vtss_isid_t isid, arp_inspection_port_mode_conf_t *port_mode_conf);

/* get ARP_INSPECTION port mode */
vtss_rc arp_inspection_mgmt_conf_port_mode_get(vtss_isid_t isid, arp_inspection_port_mode_conf_t *port_mode_conf);

/* Set ARP_INSPECTION configuration for VLAN mode */
vtss_rc arp_inspection_mgmt_conf_vlan_mode_set(vtss_vid_t vid, arp_inspection_vlan_mode_conf_t *vlan_mode_conf);

/* Get ARP_INSPECTION configuration for VLAN mode */
vtss_rc arp_inspection_mgmt_conf_vlan_mode_get(vtss_vid_t vid, arp_inspection_vlan_mode_conf_t *vlan_mode_conf, BOOL next);

/* Save ARP_INSPECTION configuration for VLAN mode */
vtss_rc arp_inspection_mgmt_conf_vlan_mode_save(void);

/* Translate ARP_INSPECTION dynamic entries into static entries */
vtss_rc arp_inspection_mgmt_conf_translate_dynamic_into_static(void);

/* Translate ARP_INSPECTION dynamic entry into static entry */
vtss_rc arp_inspection_mgmt_conf_translate_dynamic_entry_into_static_entry(arp_inspection_entry_t *entry);

/* Initialize module */
vtss_rc arp_inspection_init(vtss_init_data_t *data);

#endif /* _VTSS_ARP_INSPECTION_API_H_ */


// ***************************************************************************
//
//  End of file.
//
// ***************************************************************************
