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

#ifndef _IPMC_LIB_PORTING_H_
#define _IPMC_LIB_PORTING_H_

#define IPMC_LIB_TIME_REF_UNIT_VAL          1000000000L
#define IPMC_LIB_TIME_REF_MSEC_VAL          1000000L
#define IPMC_LIB_TIME_REF_USEC_VAL          1000L
#define IPMC_LIB_TIME_REF_NSEC_VAL          1L
#define IPMC_LIB_TIME_MSEC_BASE             1000L

#define IPMC_LIB_MEM_MAX_POOL               IPMC_MEM_TYPE_MAX
#define IPMC_LIB_MEM_SZ_K                   1024
#define IPMC_LIB_MEM_SZ_M                   (1024 * 1024)

#define IPMC_LIB_RX_FIFO_GROW               3
#define IPMC_LIB_RX_FIFO_SZ                 (0x300 * IPMC_LIB_RX_FIFO_GROW)
#define IPMC_LIB_BIP_BUF_SZ_B               (IPMC_LIB_RX_FIFO_SZ * 1600)

#define ROUTER_PORT_TIMEOUT                 300
#define QUERY_SUPPRESSION_TIMEOUT           2

/* Bug#8040: Customer asks not to send priority tag */
#define IPMC_LIB_TX_PRIO_TAG                0

#if defined(VTSS_SW_OPTION_IPMC)
#define SNP_NUM_OF_SUPPORTED_INTF           64      /**< Max Number of Interface that SNP can be enabled on it */
#else
#define SNP_NUM_OF_SUPPORTED_INTF           0
#endif /* defined(VTSS_SW_OPTION_IPMC) */
#if defined(VTSS_SW_OPTION_MVR)
#define MVR_NUM_OF_SUPPORTED_INTF           8       /**< Max Number of Interface that MVR can be running on it */
#else
#define MVR_NUM_OF_SUPPORTED_INTF           0
#endif /* defined(VTSS_SW_OPTION_MVR) */

#define IPMC_NUM_OF_SUPPORTED_IP_VER        2       /* V4 & V6 */
#define SNP_NUM_OF_INTF_PER_VERSION         (SNP_NUM_OF_SUPPORTED_INTF / IPMC_NUM_OF_SUPPORTED_IP_VER)
#define MVR_NUM_OF_INTF_PER_VERSION         (MVR_NUM_OF_SUPPORTED_INTF / IPMC_NUM_OF_SUPPORTED_IP_VER)

#if defined(VTSS_SW_OPTION_IPMC)
#if defined(VTSS_ARCH_LUTON28)
#define IPMC_LIB_SUPPORTED_SNP_GROUPS       256     /**< Total Group No. that SNP can support per system */
#else
#define IPMC_LIB_SUPPORTED_SNP_GROUPS       1024    /**< Total Group No. that SNP can support per system */
#endif /* defined(VTSS_ARCH_LUTON28) */
#else
#define IPMC_LIB_SUPPORTED_SNP_GROUPS       0
#endif /* defined(VTSS_SW_OPTION_IPMC) */

#if defined(VTSS_SW_OPTION_MVR)
#if defined(VTSS_ARCH_LUTON28)
#define IPMC_LIB_SUPPORTED_MVR_GROUPS       256     /**< Total Group No. that MVR can support per system */
#else
#define IPMC_LIB_SUPPORTED_MVR_GROUPS       1024    /**< Total Group No. that MVR can support per system */
#endif /* defined(VTSS_ARCH_LUTON28) */
#else
#define IPMC_LIB_SUPPORTED_MVR_GROUPS       0
#endif /* defined(VTSS_SW_OPTION_MVR) */

/**< Total Group No. that IPMC_LIB can support per system */
#define IPMC_LIB_SUPPORTED_GROUPS           (IPMC_LIB_SUPPORTED_SNP_GROUPS + IPMC_LIB_SUPPORTED_MVR_GROUPS)
/**< Total Source No. that IPMC_LIB can support per group */
#if defined(VTSS_ARCH_LUTON28)
#define IPMC_NO_OF_SUPPORTED_SRCLIST        2
#else
#define IPMC_NO_OF_SUPPORTED_SRCLIST        8
#endif /* defined(VTSS_ARCH_LUTON28) */

#define IPMC_LIB_MAX_SNP_FLTR_TMR_LIST      (VTSS_PORT_ARRAY_SIZE * IPMC_LIB_SUPPORTED_SNP_GROUPS)
#define IPMC_LIB_MAX_SNP_RXMT_TMR_LIST      (VTSS_PORT_ARRAY_SIZE * IPMC_LIB_SUPPORTED_SNP_GROUPS)
#define IPMC_LIB_MAX_SNP_SRCT_TMR_LIST      (VTSS_PORT_ARRAY_SIZE * IPMC_LIB_SUPPORTED_SNP_GROUPS * IPMC_NO_OF_SUPPORTED_SRCLIST)
#define IPMC_LIB_MAX_MVR_FLTR_TMR_LIST      (VTSS_PORT_ARRAY_SIZE * IPMC_LIB_SUPPORTED_MVR_GROUPS)
#define IPMC_LIB_MAX_MVR_RXMT_TMR_LIST      (VTSS_PORT_ARRAY_SIZE * IPMC_LIB_SUPPORTED_MVR_GROUPS)
#define IPMC_LIB_MAX_MVR_SRCT_TMR_LIST      (VTSS_PORT_ARRAY_SIZE * IPMC_LIB_SUPPORTED_MVR_GROUPS * IPMC_NO_OF_SUPPORTED_SRCLIST)

/* For VTSS-AVL Tree&Node */
/* Group Tree: One for SNP & One for MVR */
#if defined(VTSS_SW_OPTION_IPMC)
#define IPMC_LIB_AVLT_SNP_GRPS_CNT          1
#else
#define IPMC_LIB_AVLT_SNP_GRPS_CNT          0
#endif /* defined(VTSS_SW_OPTION_IPMC) */
#if defined(VTSS_SW_OPTION_MVR)
#define IPMC_LIB_AVLT_MVR_GRPS_CNT          1
#else
#define IPMC_LIB_AVLT_MVR_GRPS_CNT          0
#endif /* defined(VTSS_SW_OPTION_MVR) */
#define IPMC_LIB_AVLT_GRPS_CNT              (IPMC_LIB_AVLT_SNP_GRPS_CNT + IPMC_LIB_AVLT_MVR_GRPS_CNT)

/* Proxy Report Tree: One from SNP */
#define IPMC_LIB_AVLT_PROX_CNT              1

/* Interface Tree: One from MVR */
#if defined(VTSS_SW_OPTION_MVR)
#define IPMC_LIB_AVLT_MVR_IF_CNT            1
#else
#define IPMC_LIB_AVLT_MVR_IF_CNT            0
#endif /* defined(VTSS_SW_OPTION_MVR) */
#define IPMC_LIB_AVLT_SNP_IF_CNT            0
#define IPMC_LIB_AVLT_INTF_CNT              (IPMC_LIB_AVLT_SNP_IF_CNT + IPMC_LIB_AVLT_MVR_IF_CNT)

/* Timer List Tree: Three for SNP & Three for MVR */
#if defined(VTSS_SW_OPTION_IPMC)
#define IPMC_LIB_AVLT_SNP_TMR_CNT           3
#else
#define IPMC_LIB_AVLT_SNP_TMR_CNT           0
#endif /* defined(VTSS_SW_OPTION_IPMC) */
#if defined(VTSS_SW_OPTION_MVR)
#define IPMC_LIB_AVLT_MVR_TMR_CNT           3
#else
#define IPMC_LIB_AVLT_MVR_TMR_CNT           0
#endif /* defined(VTSS_SW_OPTION_MVR) */
#define IPMC_LIB_AVLT_TMR_CNT               (IPMC_LIB_AVLT_SNP_TMR_CNT + IPMC_LIB_AVLT_MVR_TMR_CNT)

/* IPMC Profile Tree: IPMC_LIB_FLTR_PROFILE_MAX_CNT */
#define IPMC_LIB_AVLT_PROFILE_CNT           (IPMC_LIB_FLTR_PROFILE_MAX_CNT)

/*
    FWD/BLK list per Group: 2 * #GRP
    SNP/MVR STATIC Arrary : 2 * ((2+2+1+1) + (3 * #PORT))
        FWD/BLK TMP4RCV (2)
        FWD/BLK TMP4TCK (2)
        TMP1            (1)
        TMP2            (1)
        SF_PERMIT_SL    (#PORT)
        SF_DENY_SL      (#PORT)
        SL_LWR_TMR      (#PORT)
*/
#define IPMC_LIB_AVLT_SRCLIST_CNT           ((2 * IPMC_LIB_SUPPORTED_GROUPS) + (2 * ((VTSS_PORT_ARRAY_SIZE * 3) + 6)))

#define IPMC_LIB_AVLT_TOTAL_CNT             (IPMC_LIB_AVLT_SRCLIST_CNT + IPMC_LIB_AVLT_PROFILE_CNT +    \
                                             IPMC_LIB_AVLT_TMR_CNT + IPMC_LIB_AVLT_INTF_CNT +           \
                                             IPMC_LIB_AVLT_PROX_CNT + IPMC_LIB_AVLT_GRPS_CNT)

#define IPMC_LIB_AVLTN_SRCLIST_CNT          (IPMC_LIB_AVLT_SRCLIST_CNT * IPMC_NO_OF_SUPPORTED_SRCLIST)
#define IPMC_LIB_AVLTN_PROFILE_CNT          (IPMC_LIB_AVLT_PROFILE_CNT * IPMC_LIB_FLTR_ENTRY_MAX_CNT)
#if defined(VTSS_SW_OPTION_IPMC)
#define IPMC_LIB_AVLTN_SNP_TMR_CNT          (IPMC_LIB_MAX_SNP_FLTR_TMR_LIST + IPMC_LIB_MAX_SNP_RXMT_TMR_LIST + IPMC_LIB_MAX_SNP_SRCT_TMR_LIST)
#else
#define IPMC_LIB_AVLTN_SNP_TMR_CNT          0
#endif /* defined(VTSS_SW_OPTION_IPMC) */
#if defined(VTSS_SW_OPTION_MVR)
#define IPMC_LIB_AVLTN_MVR_TMR_CNT          (IPMC_LIB_MAX_MVR_FLTR_TMR_LIST + IPMC_LIB_MAX_MVR_RXMT_TMR_LIST + IPMC_LIB_MAX_MVR_SRCT_TMR_LIST)
#else
#define IPMC_LIB_AVLTN_MVR_TMR_CNT          0
#endif /* defined(VTSS_SW_OPTION_MVR) */
#define IPMC_LIB_AVLTN_TMR_CNT              (IPMC_LIB_AVLTN_SNP_TMR_CNT + IPMC_LIB_AVLTN_MVR_TMR_CNT)
#define IPMC_LIB_AVLTN_SNP_IF_CNT           0
#define IPMC_LIB_AVLTN_MVR_IF_CNT           (IPMC_LIB_AVLT_INTF_CNT * MVR_NUM_OF_SUPPORTED_INTF)
#define IPMC_LIB_AVLTN_INTF_CNT             (IPMC_LIB_AVLTN_SNP_IF_CNT + IPMC_LIB_AVLTN_MVR_IF_CNT)
#define IPMC_LIB_AVLTN_PROX_CNT             (IPMC_LIB_AVLT_PROX_CNT * IPMC_LIB_SUPPORTED_SNP_GROUPS)
#define IPMC_LIB_AVLTN_GRPS_CNT             (IPMC_LIB_SUPPORTED_GROUPS)

/* Extra 32: Reserved */
#define IPMC_LIB_AVLTN_TOTAL_CNT            (IPMC_LIB_AVLTN_SRCLIST_CNT + IPMC_LIB_AVLTN_PROFILE_CNT +  \
                                             IPMC_LIB_AVLTN_TMR_CNT + IPMC_LIB_AVLTN_INTF_CNT +         \
                                             IPMC_LIB_AVLTN_PROX_CNT + IPMC_LIB_AVLTN_GRPS_CNT + 32)

#define IPMC_LIB_MEM_AVLTN_CNT_B            (sizeof(vtss_avl_tree_node_t) * IPMC_LIB_AVLTN_TOTAL_CNT)

/* Currently, TWO slots use IPMC_LIB_PKT_BUF_SZ for each */
#if defined(VTSS_SW_OPTION_MVR)
#define IPMC_LIB_MEM_JUMBO_MVR              1
#else
#define IPMC_LIB_MEM_JUMBO_MVR              0
#endif /* defined(VTSS_SW_OPTION_MVR) */
#if defined(VTSS_SW_OPTION_IPMC)
#define IPMC_LIB_MEM_JUMBO_SNP              1
#else
#define IPMC_LIB_MEM_JUMBO_SNP              0
#endif /* defined(VTSS_SW_OPTION_IPMC) */
/* eCos requires at least two blocks in fixed mempool: Cyg_Mempool_Fixed_Implementation */
#define IPMC_LIB_MEM_JUMBO_CNT              (IPMC_LIB_MEM_JUMBO_SNP + IPMC_LIB_MEM_JUMBO_MVR)
#define IPMC_LIB_MEM_SIZE_JUMBO             (IPMC_LIB_MEM_JUMBO_CNT * IPMC_LIB_PKT_BUF_SZ)

/* Dynamic Usage */
#if 1 /* For VTSS_MALLOC/VTSS_FREE */
#define IPMC_LIB_MEM_SIZE_SYS               0
#define IPMC_LIB_MEM_SIZE_AVLTND            0
#else
#define IPMC_LIB_MEM_SIZE_AVLTND            (sizeof(int) * ((IPMC_LIB_MEM_AVLTN_CNT_B + 3) / sizeof(int)))
#if defined(VTSS_ARCH_LUTON28)
#define IPMC_LIB_MEM_SIZE_SYS               (2 * IPMC_LIB_MEM_SZ_M)
#else
#define IPMC_LIB_MEM_SIZE_SYS               (8 * IPMC_LIB_MEM_SZ_M)
#endif /* defined(VTSS_ARCH_LUTON28) */
#endif /* For VTSS_MALLOC/VTSS_FREE */

#define IPMC_LIB_CTRL_HDR_FOR_GRPS          (2 * IPMC_LIB_SUPPORTED_GROUPS) /* 2: FWD&BLK */
#define IPMC_LIB_CTRL_HDR_FOR_MGMT          0
#define IPMC_LIB_DYN_ALLOC_CTRL_HDR_CNT     (IPMC_LIB_CTRL_HDR_FOR_MGMT + IPMC_LIB_CTRL_HDR_FOR_GRPS)

#define IPMC_LIB_TOTAL_SRC_LIST             (IPMC_LIB_AVLTN_SRCLIST_CNT)

#define IPMC_IP_INTF_IFID_TABLE_WALK(x)     while (vtss_ip2_if_id_next((x), &(x)) == VTSS_OK)
#define IPMC_IP_INTF_MAX_OPST               4   /* ~ VTSS_IF_STATUS_TYPE_IPV6 */
#define IPMC_IP_INTF_OPST_UP(x)             ((x) ? (((x)->type == VTSS_IF_STATUS_TYPE_LINK) ? (((x)->u.link.flags&VTSS_IF_LINK_FLAG_UP) && ((x)->u.link.flags&VTSS_IF_LINK_FLAG_RUNNING)) : FALSE) : FALSE)
#define IPMC_IP_INTF_OPST_VID(x)            ((x) ? (((x)->if_id.type == VTSS_ID_IF_TYPE_VLAN) ? (x)->if_id.u.vlan : 0) : 0)
#define IPMC_IP_INTF_OPST_ADR4(x)           ((x) ? (((x)->type == VTSS_IF_STATUS_TYPE_IPV4) ? ((x)->u.ipv4.net.address) : 0) : 0)
#define IPMC_IP_INTF_OPST_GET(x, y, z)      (vtss_ip2_if_status_get(VTSS_IF_STATUS_TYPE_ANY, (x), IPMC_IP_INTF_MAX_OPST, &(z), (y)) == VTSS_OK)
#define IPMC_IP_INTF_OPST_NGET_CTN(x, y, z) if (vtss_ip2_if_status_get(VTSS_IF_STATUS_TYPE_ANY, (x), IPMC_IP_INTF_MAX_OPST, &(z), (y)) != VTSS_OK) continue

/* Default IPv4 Address Used when Zero-Mgmt-Addr */
#define IPMC_LIB_DEF_IPV4_MGMT_ADDR         0xC0000201
#define IPMC_LIB_IP4_ADDR_NON_ZERO(x)       if (!(x)) (x) = IPMC_LIB_DEF_IPV4_MGMT_ADDR

#define IPMC_LIB_LOG_BUF_SIZE               256
#define IPMC_LIB_LOG(x, y)                  ((void) ipmc_lib_log((x), (y)))
#define IPMC_LIB_LOG_PROFILE(x, y)          do {(x)->type = IPMC_LOG_TYPE_PF; IPMC_LIB_LOG((x), (y));} while (0);
#define IPMC_LIB_LOG_MSG(x, y)              do {(x)->type = IPMC_LOG_TYPE_MSG; IPMC_LIB_LOG((x), (y));} while (0);

typedef enum {
    IPMC_ACTIVATE_DEP = 0,
    IPMC_ACTIVATE_RUN,
    IPMC_ACTIVATE_OFF
} ipmc_activate_t;

typedef enum {
    IPMC_MEM_SYS_MALLOC = 0,
    IPMC_MEM_PARTITIONED,
    IPMC_MEM_DYNA_POOL
} ipmc_mem_alloc_t;

/* Messages IDs */
typedef enum {
    IPMCLIB_MSG_ID_PROFILE_CLEAR_REQ,       /* Clear all profile related databases */
    IPMCLIB_MSG_ID_FLTR_STATE_SET_REQ,      /* Set Control State */
    IPMCLIB_MSG_ID_FLTR_ENTRY_SET_REQ,      /* Set Entry Object */
    IPMCLIB_MSG_ID_FLTR_PROFILE_SET_REQ,    /* Set Profile Object */
    IPMCLIB_MSG_ID_FLTR_RULE_SET_REQ        /* Set Rule Object */
} ipmc_lib_msg_id_t;

typedef enum {
    IPMC_LOG_TYPE_NONE = 0,
    IPMC_LOG_TYPE_PF,
    IPMC_LOG_TYPE_MSG
} ipmc_lib_log_type_t;

#define IPMC_LIB_MSG_REQ_BUFS               1
#define IPMC_LIB_MSG_ISID_VALID_SLV(x)      (!ipmc_lib_isid_is_local((x)) && msg_switch_exists((x)))
#define IPMC_LIB_MSG_ISID_VALID_ALL(x)      (msg_switch_exists((x)))
#define IPMC_LIB_MSG_ISID_PASS_SLV(x, y)    (IPMC_LIB_MSG_ISID_VALID_SLV((y)) ? (((x) != VTSS_ISID_GLOBAL) ? ((x) == (y)) : TRUE) : FALSE)
#define IPMC_LIB_MSG_ISID_PASS_ALL(x, y)    (IPMC_LIB_MSG_ISID_VALID_ALL((y)) ? (((x) != VTSS_ISID_GLOBAL) ? ((x) == (y)) : TRUE) : FALSE)

/* Request message */
typedef struct {
    /* Message ID */
    ipmc_lib_msg_id_t                   msg_id;

    /* Message data */
    union {
        /* IPMCLIB_MSG_ID_PROFILE_CLEAR_REQ */
        struct {
            BOOL                        clr_cfg;
        } clear_ctrl;

        /* IPMCLIB_MSG_ID_FLTR_STATE_SET_REQ */
        struct {
            BOOL                        mode;
        } ctrl_state;

        /* IPMCLIB_MSG_ID_FLTR_ENTRY_SET_REQ */
        struct {
            ipmc_operation_action_t     action;
            ipmc_lib_grp_fltr_entry_t   entry;
        } pf_entry;

        /* IPMCLIB_MSG_ID_FLTR_PROFILE_SET_REQ */
        struct {
            ipmc_operation_action_t     action;
            ipmc_lib_profile_t          data;
        } pf_data;

        /* IPMCLIB_MSG_ID_FLTR_RULE_SET_REQ */
        struct {
            ipmc_operation_action_t     action;
            u32                         pf_idx;
            ipmc_lib_rule_t             rule;
        } pf_rule;
    } req;
} ipmc_lib_msg_req_t;

typedef struct {
    ipmc_lib_log_type_t     type;

    vtss_vid_t              vid;
    u8                      port;
    ipmc_ip_version_t       version;
    vtss_ipv6_t             *dst;
    vtss_ipv6_t             *src;

    union {
        struct {
            ipmc_action_t   action;
            i8              *name;
            i8              *entry;
        } profile;

        struct {
            i8              *data;
        } message;
    } event;
} ipmc_lib_log_t;

typedef cyg_mempool_info    ipmc_lib_memory_info_t;

BOOL ipmc_lib_memory_initialize(ipmc_mem_alloc_t type,
                                u8 *pool_idx,
                                size_t total_sz,
                                size_t entry_sz);
u8 *ipmc_lib_memory_allocate(ipmc_mem_alloc_t type, u8 pool_idx, size_t size);
BOOL ipmc_lib_memory_free(ipmc_mem_alloc_t type, u8 pool_idx, u8 *ptr);
BOOL ipmc_lib_memory_info_get(ipmc_mem_alloc_t type, u8 pool_idx, ipmc_lib_memory_info_t *info);

BOOL ipmc_lib_unregistered_flood_set(ipmc_owner_t owner,
                                     ipmc_activate_t activate,
                                     BOOL member[VTSS_PORT_ARRAY_SIZE]);
BOOL ipmc_lib_mc6_ctrl_flood_set(ipmc_owner_t owner, BOOL status);
vtss_rc vtss_ipmc_lib_rx_unregister(ipmc_ip_version_t version);
vtss_rc vtss_ipmc_lib_rx_register(void *cb, ipmc_ip_version_t version);

BOOL ipmc_lib_log(const ipmc_lib_log_t *content, const ipmc_log_severity_t severity);

#endif /* _IPMC_LIB_PORTING_H_ */
