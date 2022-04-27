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

#ifndef _ACCESS_MGMT_API_H_
#define _ACCESS_MGMT_API_H_


#include "vtss_api.h" /* For vtss_rc, vtss_vid_t, etc. */


/**
 * \file access_mgmt_api.h
 * \brief This file defines the APIs for the Access Management module
 */


#define ACCESS_MGMT_ACCESS_ID_START     (1)
#define ACCESS_MGMT_MAX_ENTRIES         (16)

/**
 * Access managent enabled/disabled
 */
#define ACCESS_MGMT_ENABLED         (1)       /**< Enable option  */
#define ACCESS_MGMT_DISABLED        (0)       /**< Disable option */

/**
 * \brief API Error Return Codes (vtss_rc)
 */
enum {
    ACCESS_MGMT_ERROR_GEN = MODULE_ERROR_START(VTSS_MODULE_ID_ACCESS_MGMT), /* Generic error code */
    ACCESS_MGMT_ERROR_PARM,                                                 /* Illegal parameter */
    ACCESS_MGMT_ERROR_STACK_STATE,                                          /* Illegal MASTER/SLAVE state */
    ACCESS_MGMT_ERROR_GET_CERT_INFO,                                        /* Illegal get certificate information */
};

/* Access management services type */
#define ACCESS_MGMT_SERVICES_TYPE_TELNET    0x1
#define ACCESS_MGMT_SERVICES_TYPE_WEB       0x2
#define ACCESS_MGMT_SERVICES_TYPE_SNMP      0x4
#define ACCESS_MGMT_SERVICES_TYPE           (ACCESS_MGMT_SERVICES_TYPE_TELNET | ACCESS_MGMT_SERVICES_TYPE_WEB | ACCESS_MGMT_SERVICES_TYPE_SNMP)

/* Access management entry type */
#define ACCESS_MGMT_ENTRY_TYPE_IPV4     0x0
#define ACCESS_MGMT_ENTRY_TYPE_IPV6     0x1

/**
 * \brief Access Management entry.
 */
typedef struct {
    BOOL        valid;
    u32         service_type;
    u32         entry_type;
    vtss_ipv4_t start_ip;
    vtss_ipv4_t end_ip;
#ifdef VTSS_SW_OPTION_IPV6
    vtss_ipv6_t start_ipv6;
    vtss_ipv6_t end_ipv6;
#endif /* VTSS_SW_OPTION_IPV6 */
    vtss_vid_t  vid;
} access_mgmt_entry_t;

/**
 * \brief Access Management configuration.
 */
typedef struct {
    u32                 mode;
    u32                 entry_num;
    access_mgmt_entry_t entry[ACCESS_MGMT_ACCESS_ID_START + ACCESS_MGMT_MAX_ENTRIES];
} access_mgmt_conf_t;

/**
 * \brief Access Management statistics.
 */
typedef struct {
    u32 http_receive_cnt;
    u32 http_discard_cnt;
    u32 https_receive_cnt;
    u32 https_discard_cnt;
    u32 snmp_receive_cnt;
    u32 snmp_discard_cnt;
    u32 telnet_receive_cnt;
    u32 telnet_discard_cnt;
    u32 ssh_receive_cnt;
    u32 ssh_discard_cnt;
} access_mgmt_stats_t;

/**
  * \brief Retrieve an error string based on a return code
  *        from one of the Access Management API functions.
  *
  * \param rc [IN]: Error code that must be in the ACCESS_MGMT_ERROR_xxx range.
  */
char *access_mgmt_error_txt(vtss_rc rc);

/* Get access management configuration */
vtss_rc access_mgmt_conf_get(access_mgmt_conf_t *conf);

/* Set access management configuration */
vtss_rc access_mgmt_conf_set(access_mgmt_conf_t *conf);

/**
  * \brief Initialize the Access Management module
  *
  * \param data [IN]: Initial data point.
  *
  * \return
  *    VTSS_OK.
  */
vtss_rc access_mgmt_init(vtss_init_data_t *data);

/* Get access management entry */
vtss_rc access_mgmt_entry_get(int access_id, access_mgmt_entry_t *entry);

/* Add access management entry
   valid access_id: ACCESS_MGMT_ACCESS_ID_START ~ ACCESS_MGMT_MAX_ENTRIES */
vtss_rc access_mgmt_entry_add(int access_id, access_mgmt_entry_t *entry);

/* Delete access management entry */
vtss_rc access_mgmt_entry_del(int access_id);

/* Clear access management entry */
vtss_rc access_mgmt_entry_clear(void);

/* Get access management statistics */
void access_mgmt_stats_get(access_mgmt_stats_t *stats);

/* Clear access management statistics */
void access_mgmt_stats_clear(void);

/**
 * \brief Callback function for access management filter reject.
 */
typedef struct {
    int service_type;
    u32 ip_addr;
#ifdef VTSS_SW_OPTION_IPV6
    u8  ip_version;
    u8  ipv6_addr[16];
#endif /* VTSS_SW_OPTION_IPV6 */
} access_mgmt_filter_reject_info_t;

typedef void (*access_mgmt_filter_reject_callback_t)(access_mgmt_filter_reject_info_t filter_reject_info);

/* Register access management filter reject callback function */
void access_mgmt_filter_reject_register(access_mgmt_filter_reject_callback_t cb);

/* Unregister access management filter reject callback function */
void access_mgmt_filter_reject_unregister(void);

/* Check if entry content is the same as others
   Retrun: 0 - no duplicated, others - duplicated access_id */
int access_mgmt_entry_content_is_duplicated(int access_id, access_mgmt_entry_t *entry);

#endif /* _ACCESS_MGMT_API_H_ */


// ***************************************************************************
//
//  End of file.
//
// ***************************************************************************
