/*

 Vitesse Switch API software.

 Copyright (c) 2002-2014 Vitesse Semiconductor Corporation "Vitesse". All
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

#ifndef _IPMC_LIB_TYPE_H_
#define _IPMC_LIB_TYPE_H_

#include "vtss_avl_tree_api.h"
#include "vtss_free_list_api.h"
#include "ip2_api.h"

#define VTSS_IPMC_MGMT_IPIF_MAX_CNT     IP2_MAX_INTERFACES

#define VTSS_IPMC_DISABLE               0x0
#define VTSS_IPMC_ENABLE                0x1

#define VTSS_IPMC_TRUE                  0x1
#define VTSS_IPMC_FALSE                 0x0

#define VTSS_IPMC_VID_NULL              0x0
#define VTSS_IPMC_VID_MAX               0xFFF
#define VTSS_IPMC_VID_ALL               0x1FFF
#define VTSS_IPMC_VID_VOID              0xFFFF

#define VTSS_IPMC_NAME_STRING_MAX_LEN   16
#define VTSS_IPMC_DESC_STRING_MAX_LEN   64
#define VTSS_IPMC_NAME_MAX_LEN          (VTSS_IPMC_NAME_STRING_MAX_LEN + 1)
#define VTSS_IPMC_DESC_MAX_LEN          (VTSS_IPMC_DESC_STRING_MAX_LEN + 1)
#define VTSS_IPMC_MVR_NAME_MAX_LEN      (VTSS_IPMC_NAME_STRING_MAX_LEN + 1)

#define ipmc_mgmt_ipadr4                addr.ipv4.val
#define ipmc_mgmt_ipadr6                addr.ipv6.val
#define ipmc_mgmt_intf_vidx(x)          ((x)->vidx)
#define ipmc_mgmt_intf_opst(x)          ((x)->opst)
#define ipmc_mgmt_intf_live(x)          ((x)->valid)
#define ipmc_mgmt_intf_adr4(x)          ((x)->ipmc_mgmt_ipadr4)
#define ipmc_mgmt_intf_adr6(x)          ((x)->ipmc_mgmt_ipadr6)
#define ipmc_mgmt_intf_chks(x)          ((x)->chks)
#define ipmc_mgmt_intf_next(x)          ((x)->next)

#define IPMC_MGMT_SYSTEM_CHANGE(x)      ((x)->change)
#define IPMC_MGMT_SYSTEM_MAC(x)         ((x)->mac_addr)
#define IPMC_MGMT_SYSTEM_IPIF(x, y)     (&((x)->ip_addr[(y)]))

#define IPMC_MGMT_MAC_ADR_GET(x, y)     (memcpy((y), IPMC_MGMT_SYSTEM_MAC((x)), sizeof(IPMC_MGMT_SYSTEM_MAC((x)))))
#define IPMC_MGMT_MAC_ADR_SET(x, y)     (memcpy(IPMC_MGMT_SYSTEM_MAC((x)), (y), sizeof(IPMC_MGMT_SYSTEM_MAC((x)))))
#define IPMC_MGMT_MAC_ADR_CMP(x, y)     (memcmp(IPMC_MGMT_SYSTEM_MAC((x)), (y), sizeof(IPMC_MGMT_SYSTEM_MAC((x)))))

#define IPMC_MGMT_IPIF_IDVLN(x, y)      (IPMC_MGMT_SYSTEM_IPIF((x), (y))->vidx)
#define IPMC_MGMT_IPIF_VALID(x, y)      (IPMC_MGMT_SYSTEM_IPIF((x), (y))->valid)
#define IPMC_MGMT_IPIF_ADRS4(x, y)      (IPMC_MGMT_SYSTEM_IPIF((x), (y))->ipmc_mgmt_ipadr4)
#define IPMC_MGMT_IPIF_ADRS6(x, y)      (IPMC_MGMT_SYSTEM_IPIF((x), (y))->ipmc_mgmt_ipadr6)
#define IPMC_MGMT_IPIF_STATE(x, y)      (IPMC_MGMT_SYSTEM_IPIF((x), (y))->opst)
#define IPMC_MGMT_IPIF_CHKST(x, y)      (IPMC_MGMT_SYSTEM_IPIF((x), (y))->chks)


/**
 * \brief IPMC API Error Return Codes (vtss_rc)
 */
typedef enum {
    IPMC_ERROR_GEN = MODULE_ERROR_START(VTSS_MODULE_ID_IPMC_LIB),   /**< Operation is only allowed on the master switch.    */
    IPMC_ERROR_PARM,                                                /**< Illegal parameter                                  */
    IPMC_ERROR_VLAN_NOT_FOUND,                                      /**< VLAN not found                                     */
    IPMC_ERROR_VLAN_ACTIVE,                                         /**< VLAN active                                        */
    IPMC_ERROR_VLAN_NOT_ACTIVE,                                     /**< VLAN not active                                    */
    IPMC_ERROR_STACK_STATE,                                         /**< Illegal MASTER/SLAVE state                         */
    IPMC_ERROR_REQ_TIMEOUT,                                         /**< Request Timeout                                    */
    IPMC_ERROR_ENTRY_NOT_FOUND,                                     /**< Entry not found                                    */
    IPMC_ERROR_ENTRY_NOT_ACTIVE,                                    /**< Entry not active                                   */
    IPMC_ERROR_ENTRY_OVERLAPPED,                                    /**< Overlapped Entry                                   */
    IPMC_ERROR_ENTRY_INVALID,                                       /**< Invalid Entry                                      */
    IPMC_ERROR_ENTRY_EXISTED,                                       /**< Existed Entry                                      */
    IPMC_ERROR_ENTRY_NAME_DUPLICATED,                               /**< Duplicated Entry Name                              */
    IPMC_ERROR_TABLE_IS_FULL,                                       /**< Table Full                                         */
    IPMC_ERROR_MEMORY_NG,                                           /**< Something wrong in Memory Allocate or Free         */

    IPMC_ERROR_PKT_IS_QUERY,
    IPMC_ERROR_PKT_GROUP_FILTER,
    IPMC_ERROR_PKT_GROUP_NOT_FOUND,

    IPMC_ERROR_PKT_COMPATIBILITY,
    IPMC_ERROR_PKT_TOO_MUCH_QUERY,
    IPMC_ERROR_PKT_CHECKSUM,
    IPMC_ERROR_PKT_INGRESS_FILTER,
    IPMC_ERROR_PKT_CONTENT,
    IPMC_ERROR_PKT_FORMAT,
    IPMC_ERROR_PKT_ADDRESS,
    IPMC_ERROR_PKT_RESERVED,
    IPMC_ERROR_PKT_VERSION
} ipmc_error_t;

typedef enum {
    IPMC_TXT_CASE_CAPITAL = 0,
    IPMC_TXT_CASE_LOWER,
    IPMC_TXT_CASE_UPPER,
    IPMC_TXT_CASE_FREE
} ipmc_text_cap_t;

typedef enum {
    IPMC_OP_ERR = -2,
    IPMC_OP_INT,
    IPMC_OP_SET,
    IPMC_OP_ADD,
    IPMC_OP_DEL,
    IPMC_OP_UPD
} ipmc_operation_action_t;

typedef enum {
    IPMC_ACTION_DENY = 0,
    IPMC_ACTION_PERMIT,
} ipmc_action_t;

typedef enum {
    IPMC_OWNER_INIT = -2,
    IPMC_OWNER_ALL,
    IPMC_OWNER_IGMP,
    IPMC_OWNER_MLD,
    IPMC_OWNER_SNP,
    IPMC_OWNER_SNP4,
    IPMC_OWNER_SNP6,
    IPMC_OWNER_MVR,
    IPMC_OWNER_MVR4,
    IPMC_OWNER_MVR6,
    IPMC_OWNER_MAX
} ipmc_owner_t;

typedef enum {
    IPMC_IP_VERSION_ALL = 0,
    IPMC_IP_VERSION_IGMP,
    IPMC_IP_VERSION_MLD,
    IPMC_IP_VERSION_IPV4Z,
    IPMC_IP_VERSION_IPV6Z,
    IPMC_IP_VERSION_DNS,
    IPMC_IP_VERSION_INIT,
    IPMC_IP_VERSION_ERR,
    IPMC_IP_VERSION_MAX
} ipmc_ip_version_t;

typedef enum {
    IPMC_PKT_TYPE_IGMP_GQ = 0,
    IPMC_PKT_TYPE_IGMP_SQ,
    IPMC_PKT_TYPE_IGMP_SSQ,
    IPMC_PKT_TYPE_IGMP_V1JOIN,
    IPMC_PKT_TYPE_IGMP_V2JOIN,
    IPMC_PKT_TYPE_IGMP_V3JOIN,
    IPMC_PKT_TYPE_IGMP_LEAVE,
    IPMC_PKT_TYPE_MLD_GQ,
    IPMC_PKT_TYPE_MLD_SQ,
    IPMC_PKT_TYPE_MLD_SSQ,
    IPMC_PKT_TYPE_MLD_V1REPORT,
    IPMC_PKT_TYPE_MLD_V2REPORT,
    IPMC_PKT_TYPE_MLD_DONE
} ipmc_ctrl_pkt_t;

typedef enum {
    IPMC_PKT_SRC_MVR = -1,
    IPMC_PKT_SRC_MVR_INACT,
    IPMC_PKT_SRC_MVR_SOURC,
    IPMC_PKT_SRC_MVR_RECVR,
    IPMC_PKT_SRC_MVR_STACK,
    IPMC_PKT_SRC_SNP,
    IPMC_PKT_SRC_SNP_V4,
    IPMC_PKT_SRC_SNP_V6,
    IPMC_PKT_SRC_SNP_STACK
} ipmc_pkt_src_port_t;

typedef enum {
    IPMC_INTF_UNTAG = 0,
    IPMC_INTF_TAGED
} ipmc_intf_vtag_t;

typedef enum {
    IPMC_SND_HOLD = 0,
    IPMC_SND_GO_HOLD,
    IPMC_SND_GO
} ipmc_send_act_t;

typedef enum {
    MVR_INTF_MODE_INIT = -1,
    MVR_INTF_MODE_DYNA,
    MVR_INTF_MODE_COMP
} mvr_intf_mode_t;

typedef enum {
    MVR_PORT_ROLE_INACT = 0,
    MVR_PORT_ROLE_SOURC,
    MVR_PORT_ROLE_RECVR,
    MVR_PORT_ROLE_STACK
} mvr_port_role_t;

/*
    Refer to SYSLOG severity
    (Emergency, Alert, Critical, Error, Warning, Notice, Info or Debug)
*/
typedef enum {
    IPMC_SEVERITY_Normal = -1,
    IPMC_SEVERITY_InfoDebug,
    IPMC_SEVERITY_Notice,
    IPMC_SEVERITY_Warning,
    IPMC_SEVERITY_Error,
    IPMC_SEVERITY_Critical,
    IPMC_SEVERITY_Alert,
    IPMC_SEVERITY_Emergency
} ipmc_log_severity_t;

typedef struct {
    u8                  addr[4];
} ipmcv4addr;

typedef struct {
    u8                  addr[16];
} ipmcv6addr;

typedef struct {
    u8                  member_ports[VTSS_PORT_BF_SIZE];
} ipmc_port_bfs_t;

typedef struct {
    int                 max_no[VTSS_PORT_ARRAY_SIZE];
} ipmc_port_throttling_t;

typedef struct {
    u32                 profile_index[VTSS_PORT_ARRAY_SIZE];
} ipmc_port_group_filtering_t;

typedef struct {
    union {
        struct {
            vtss_ipv6_t prefix;
        } array;

        struct {
            vtss_ipv4_t reserved[3];
            vtss_ipv4_t prefix;
        } value;
    } addr;

    u32                 len;
} ipmc_prefix_t;

typedef struct ipmc_db_ctrl_hdr_s {
    vtss_avl_tree_t             ctrl;
    vtss_avl_tree_node_t        *node;
    i32                         cnt;
    size_t                      size;

    struct ipmc_db_ctrl_hdr_s   *next;
    BOOL                        mflag;
} ipmc_db_ctrl_hdr_t;

typedef struct ipmc_mgmt_ipif_s {
    vtss_vid_t                  vidx;   /* INDEX */

    BOOL                        valid;
    BOOL                        opst;

    union {
        struct {
            vtss_ipv4_t         val;
        } ipv4;

        struct {
            vtss_ipv6_t         val;
        } ipv6;
    } addr;

    i32                         chks;
    struct ipmc_mgmt_ipif_s     *next;
} ipmc_mgmt_ipif_t;

/* System Management (IP) Information */
typedef struct {
    u32                         count;
    u32                         max;

    ipmc_mgmt_ipif_t            *free;
    ipmc_mgmt_ipif_t            *used;
} ipmc_lib_mgmt_intf_t;

typedef struct {
    BOOL                        change;

    u8                          mac_addr[6];
    ipmc_mgmt_ipif_t            ip_addr[VTSS_IPMC_MGMT_IPIF_MAX_CNT];

    ipmc_lib_mgmt_intf_t        intf_list;
} ipmc_lib_mgmt_info_t;

#endif /* _IPMC_LIB_TYPE_H_ */
