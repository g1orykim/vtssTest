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

#ifndef _IPMC_LIB_BASE_H_
#define _IPMC_LIB_BASE_H_

#include "vtss_module_id.h"
#ifdef VTSS_SW_OPTION_PACKET
#include "packet_api.h"
#endif /* VTSS_SW_OPTION_PACKET */
#include "ipmc_lib_type.h"


#define VTSS_IPMC_VERSION_DEFAULT       0x0
#define VTSS_IPMC_VERSION1              0x1
#define VTSS_IPMC_VERSION2              0x2
#define VTSS_IPMC_VERSION3              0x3

#define VTSS_IPMC_MAX(a, b)             (((a) < (b)) ? (b) : (a))
#define VTSS_IPMC_MIN(a, b)             (((a) < (b)) ? (a) : (b))

#define IPMC_NO_OF_PKT_SRCLIST          32
#define IPMC_NO_OF_MAX_PKT_SRCLIST4     366
#define IPMC_NO_OF_MAX_PKT_SRCLIST6     89

#define IPMC_THREAD_STACK_SIZE          THREAD_DEFAULT_STACK_SIZE

#if 1 /* VTSS-AVL */
#define IPMC_LIB_DB_GET_FIRST(x, y)     (ipmc_lib_db_get_first_entry((x), &(y)))
#define IPMC_LIB_DB_GET(x, y)           (ipmc_lib_db_get_entry((x), &(y)))
#define IPMC_LIB_DB_GET_NEXT(x, y)      (ipmc_lib_db_get_next_entry((x), &(y)))
#define IPMC_LIB_DB_GET_COUNT(x)        (ipmc_lib_db_get_current_cnt((x)))
#define IPMC_LIB_DB_GET_MAX(x)          (ipmc_lib_db_get_max_cnt((x)))
#define IPMC_LIB_DB_GET_SIZE(x)         (ipmc_lib_db_get_size_of_entry((x)))
#define IPMC_LIB_DB_TAKE(v, w, x, y, z) (ipmc_lib_db_create_tree((w), (v), (x), (y), (z)))
#define IPMC_LIB_DB_GIVE(x)             (ipmc_lib_db_destroy_tree((x)))
#define IPMC_LIB_DB_ADD(x, y)           (ipmc_lib_db_set_entry((x), (y)))
#define IPMC_LIB_DB_DEL(x, y)           (ipmc_lib_db_delete_entry((x), (y)))
#define IPMC_LIB_DB_SET(x, y)           (ipmc_lib_db_set_entry((x), (y)))
#define IPMC_LIB_DB_CLR(x)              (ipmc_lib_db_delete_all_entry((x)))
#else
#include "vtss_lib_data_struct_api.h"

#define IPMC_PTR_DB_GET_FIRST(x, y)     (vtss_lib_data_struct_get_first_entry(&((x)->ctrl), &(y)) == VTSS_LIB_DATA_STRUCT_RC_OK)
#define IPMC_PTR_DB_GET(x, y)           (vtss_lib_data_struct_get_entry(&((x)->ctrl), &(y)) == VTSS_LIB_DATA_STRUCT_RC_OK)
#define IPMC_PTR_DB_GET_NEXT(x, y)      (vtss_lib_data_struct_get_next_entry(&((x)->ctrl), &(y)) == VTSS_LIB_DATA_STRUCT_RC_OK)
#define IPMC_PTR_DB_GET_COUNT(x)        vtss_lib_data_struct_get_current_cnt(&((x)->ctrl))
#define IPMC_PTR_DB_GET_MAX(x)          vtss_lib_data_struct_get_max_cnt(&((x)->ctrl))
#define IPMC_PTR_DB_GET_TYPE(x)         vtss_lib_data_struct_get_data_struct_type(&((x)->ctrl))
#define IPMC_PTR_DB_GET_SIZE(x)         vtss_lib_data_struct_get_size_of_entry(&((x)->ctrl))
#define IPMC_PTR_DB_TAKE(w, x, y, z)    (vtss_lib_data_struct_create_tree(&((w)->ctrl), (x), (y), (z), NULL, NULL, VTSS_LIB_DATA_STRUCT_TYPE_AVL_TREE) == VTSS_LIB_DATA_STRUCT_RC_OK)
#define IPMC_PTR_DB_GIVE(x)             (vtss_lib_data_struct_destroy_tree(&((x)->ctrl)) == VTSS_LIB_DATA_STRUCT_RC_OK)
#define IPMC_PTR_DB_ADD(x, y)           (vtss_lib_data_struct_set_entry(&((x)->ctrl), &(y)) == VTSS_LIB_DATA_STRUCT_RC_OK)
#define IPMC_PTR_DB_DEL(x, y)           (vtss_lib_data_struct_delete_entry(&((x)->ctrl), &(y)) == VTSS_LIB_DATA_STRUCT_RC_OK)
#define IPMC_PTR_DB_SET(x, y)           (vtss_lib_data_struct_set_entry(&((x)->ctrl), &(y)) == VTSS_LIB_DATA_STRUCT_RC_OK)
#define IPMC_PTR_DB_CLR(x)              (vtss_lib_data_struct_delete_all_entry(&((x)->ctrl)) == VTSS_LIB_DATA_STRUCT_RC_OK)

#define IPMC_BUF_DB_GET_FIRST(x, y)     (vtss_lib_data_struct_get_first_entry(&((x)->ctrl), (y)) == VTSS_LIB_DATA_STRUCT_RC_OK)
#define IPMC_BUF_DB_GET(x, y)           (vtss_lib_data_struct_get_entry(&((x)->ctrl), (y)) == VTSS_LIB_DATA_STRUCT_RC_OK)
#define IPMC_BUF_DB_GET_NEXT(x, y)      (vtss_lib_data_struct_get_next_entry(&((x)->ctrl), (y)) == VTSS_LIB_DATA_STRUCT_RC_OK)
#define IPMC_BUF_DB_GET_COUNT(x)        vtss_lib_data_struct_get_current_cnt(&((x)->ctrl))
#define IPMC_BUF_DB_GET_MAX(x)          vtss_lib_data_struct_get_max_cnt(&((x)->ctrl))
#define IPMC_BUF_DB_GET_TYPE(x)         vtss_lib_data_struct_get_data_struct_type(&((x)->ctrl))
#define IPMC_BUF_DB_GET_SIZE(x)         vtss_lib_data_struct_get_size_of_entry(&((x)->ctrl))
#define IPMC_BUF_DB_TAKE(w, x, y, z)    (vtss_lib_data_struct_create_tree(&((w)->ctrl), (x), (y), (z), malloc, free, VTSS_LIB_DATA_STRUCT_TYPE_SORT_LIST) == VTSS_LIB_DATA_STRUCT_RC_OK)
#define IPMC_BUF_DB_GIVE(x)             (vtss_lib_data_struct_destroy_tree(&((x)->ctrl)) == VTSS_LIB_DATA_STRUCT_RC_OK)
#define IPMC_BUF_DB_ADD(x, y)           (vtss_lib_data_struct_set_entry(&((x)->ctrl), (y)) == VTSS_LIB_DATA_STRUCT_RC_OK)
#define IPMC_BUF_DB_DEL(x, y)           (vtss_lib_data_struct_delete_entry(&((x)->ctrl), (y)) == VTSS_LIB_DATA_STRUCT_RC_OK)
#define IPMC_BUF_DB_SET(x, y)           (vtss_lib_data_struct_set_entry(&((x)->ctrl), (y)) == VTSS_LIB_DATA_STRUCT_RC_OK)
#define IPMC_BUF_DB_CLR(x)              (vtss_lib_data_struct_delete_all_entry(&((x)->ctrl)) == VTSS_LIB_DATA_STRUCT_RC_OK)

#define IPMC_LIB_DB_AVL(x)              ((x)->ctrl.data_struct_type == VTSS_LIB_DATA_STRUCT_TYPE_AVL_TREE)
#define IPMC_LIB_DB_GET_FIRST(x, y)     (IPMC_LIB_DB_AVL((x)) ? IPMC_PTR_DB_GET_FIRST((x), (y)) : IPMC_BUF_DB_GET_FIRST((x), (y)))
#define IPMC_LIB_DB_GET(x, y)           (IPMC_LIB_DB_AVL((x)) ? IPMC_PTR_DB_GET((x), (y)) : IPMC_BUF_DB_GET((x), (y)))
#define IPMC_LIB_DB_GET_NEXT(x, y)      (IPMC_LIB_DB_AVL((x)) ? IPMC_PTR_DB_GET_NEXT((x), (y)) : IPMC_BUF_DB_GET_NEXT((x), (y)))
#define IPMC_LIB_DB_GET_COUNT(x)        vtss_lib_data_struct_get_current_cnt(&((x)->ctrl))
#define IPMC_LIB_DB_GET_MAX(x)          vtss_lib_data_struct_get_max_cnt(&((x)->ctrl))
#define IPMC_LIB_DB_GET_TYPE(x)         vtss_lib_data_struct_get_data_struct_type(&((x)->ctrl))
#define IPMC_LIB_DB_GET_SIZE(x)         vtss_lib_data_struct_get_size_of_entry(&((x)->ctrl))
#define IPMC_LIB_DB_TAKE(v, w, x, y, z) ((v) ? IPMC_PTR_DB_TAKE((w), (x), (y), (z)) : IPMC_BUF_DB_TAKE((w), (x), (y), (z)))
#define IPMC_LIB_DB_GIVE(x)             (vtss_lib_data_struct_destroy_tree(&((x)->ctrl)) == VTSS_LIB_DATA_STRUCT_RC_OK)
#define IPMC_LIB_DB_ADD(x, y)           (IPMC_LIB_DB_AVL((x)) ? IPMC_PTR_DB_ADD((x), (y)) : IPMC_BUF_DB_ADD((x), (y)))
#define IPMC_LIB_DB_DEL(x, y)           (IPMC_LIB_DB_AVL((x)) ? IPMC_PTR_DB_DEL((x), (y)) : IPMC_BUF_DB_DEL((x), (y)))
#define IPMC_LIB_DB_SET(x, y)           (IPMC_LIB_DB_AVL((x)) ? IPMC_PTR_DB_SET((x), (y)) : IPMC_BUF_DB_SET((x), (y)))
#define IPMC_LIB_DB_CLR(x)              (vtss_lib_data_struct_delete_all_entry(&((x)->ctrl)) == VTSS_LIB_DATA_STRUCT_RC_OK)
#endif /* VTSS-AVL */

#define IPMC_LIB_GRP_PORT_DO_SFM(x, y)  (VTSS_PORT_BF_GET((x)->ipmc_sf_port_status, (y)) != VTSS_IPMC_SF_STATUS_DISABLED)
#define IPMC_LIB_GRP_PORT_SFM_IN(x, y)  (VTSS_PORT_BF_GET((x)->ipmc_sf_port_mode, (y)) == VTSS_IPMC_SF_MODE_INCLUDE)
#define IPMC_LIB_GRP_PORT_SFM_EX(x, y)  (VTSS_PORT_BF_GET((x)->ipmc_sf_port_mode, (y)) != VTSS_IPMC_SF_MODE_INCLUDE)

#define IPMC_LIB_CHK_LISTENER_SET(x, y) do {VTSS_PORT_BF_SET((x)->info->db.chk_listener_state, (y), TRUE);} while (0)
#define IPMC_LIB_CHK_LISTENER_CLR(x, y) do {VTSS_PORT_BF_SET((x)->info->db.chk_listener_state, (y), FALSE);} while (0)
#define IPMC_LIB_CHK_LISTENER_GET(x, y) (VTSS_PORT_BF_GET((x)->info->db.chk_listener_state, (y)))

#define IPMC_LIB_ADRS_CMP(x, y)         (memcmp((x), (y), sizeof(vtss_ipv6_t)))
#define IPMC_LIB_ADRS_GREATER(x, y)     (IPMC_LIB_ADRS_CMP((x), (y)) > 0)
#define IPMC_LIB_ADRS_EQUAL(x, y)       (IPMC_LIB_ADRS_CMP((x), (y)) == 0)
#define IPMC_LIB_ADRS_LESS(x, y)        (IPMC_LIB_ADRS_CMP((x), (y)) < 0)
#define IPMC_LIB_ADRS_CPY(x, y)         (memcpy((x), (y), sizeof(vtss_ipv6_t)))
#define IPMC_LIB_ADRS_SET(x, y)         (memset((x), (y), sizeof(vtss_ipv6_t)))

#define IPMC_LIB_ADRS_CMP6(x, y)        (memcmp(&(x), &(y), sizeof(vtss_ipv6_t)))
#define IPMC_LIB_ADRS_CMP4(x, y)        (memcmp(&((x).addr[12]), &((y).addr[12]), sizeof(vtss_ipv4_t)))
#define IPMC_LIB_ADRS_4TO6_SET(x, y)    (memcpy(&((y).addr[12]), (u8 *)&((x)), sizeof(ipmcv4addr)))
#define IPMC_LIB_ADRS_6TO4_SET(x, y)    (memcpy((u8 *)&((y)), &((x).addr[12]), sizeof(ipmcv4addr)))

#define IPMC_LIB_ADRS_MINUS_ONE(x)      do {    \
  if ((x)) {                                    \
    u8  i, j = 1;                               \
    for (i = 15; (i > 0) && j; i--) {           \
      if ((x)->addr[i]) {                       \
        --(x)->addr[i];                         \
        j = 0;                                  \
      } else {                                  \
        (x)->addr[i] = 0xFF;                    \
        if ((x)->addr[i - 1]) {                 \
          --(x)->addr[i - 1];                   \
          j = 0;                                \
        }                                       \
      }                                         \
    }                                           \
  }                                             \
} while (0)

#define IPMC_LIB_MC_ADR_MINUS_ONE(x, y) do {                        \
  if ((x)) {                                                        \
    u8  i, j = 1;                                                   \
    for (i = 15; (i > 0) && j; i--) {                               \
      if ((x)->addr[i]) {                                           \
        --(x)->addr[i];                                             \
        j = 0;                                                      \
      } else {                                                      \
        (x)->addr[i] = 0xFF;                                        \
        if ((x)->addr[i - 1]) {                                     \
          --(x)->addr[i - 1];                                       \
          j = 0;                                                    \
        }                                                           \
      }                                                             \
    }                                                               \
    if (ipmc_lib_grp_adrs_version((x)) == IPMC_IP_VERSION_ERR) {    \
      IPMC_LIB_ADRS_SET((x), 0x0);                                  \
      if ((y) == IPMC_IP_VERSION_IGMP) {                            \
        (x)->addr[12] = 0xE0;                                       \
      } else {                                                      \
        (x)->addr[0] = 0xFF;                                        \
      }                                                             \
    }                                                               \
  }                                                                 \
} while (0)

#define IPMC_LIB_ADRS_PLUS_ONE(x)       do {    \
  if ((x)) {                                    \
    i8  i;                                      \
    for (i = 15; i >= 0; i--) {                 \
      if ((x)->addr[i] != 0xFF) {               \
        (x)->addr[i]++;                         \
        break;                                  \
      } else {                                  \
        (x)->addr[i] = 0x0;                     \
      }                                         \
    }                                           \
  }                                             \
} while (0)

typedef enum {
    VTSS_IPMC_COMPAT_MODE_AUTO = 0,
    VTSS_IPMC_COMPAT_MODE_OLD,          /* Compatibility mode for IGMPv1 */
    VTSS_IPMC_COMPAT_MODE_GEN,          /* Compatibility mode for IGMPv2/MLDv1 */
    VTSS_IPMC_COMPAT_MODE_SFM           /* Compatibility mode for IGMPv3/MLDv2 */
} ipmc_compat_mode_t;

typedef enum {
    IPMC_OP_NO_LISTENER = 1,
    IPMC_OP_HAS_LISTENER,
    IPMC_OP_CHK_LISTENER
} ipmc_operation_states_t;

typedef enum {
    IPMC_QUERIER_INIT = 0,
    IPMC_QUERIER_IDLE,
    IPMC_QUERIER_ACTIVE
} ipmc_querier_states_t;

typedef enum {
    IPMC_V4_GEN_MXRC = 0,
    IPMC_V4_SFM_MXRC,
    IPMC_V4_SFM_QQIC,
    IPMC_V6_GEN_MXRC,
    IPMC_V6_SFM_MXRC,
    IPMC_V6_SFM_QQIC
} ipmc_pkt_exp_t;


/* Parameter Values */
#define IPMC_PARAM_VALUE_INIT           (-1)
#define IPMC_PARAM_VALUE_NULL           0xFFFFFFFF
#define IPMC_PARAM_PRIORITY_NULL        0xFF
#define IPMC_PARAM_DEF_COMPAT           VTSS_IPMC_COMPAT_MODE_AUTO
#define IPMC_PARAM_DEF_PRIORITY         0x0
#define IPMC_PARAM_MAX_PRIORITY         0x7
#define IPMC_PARAM_PRIORITY_MASK        IPMC_PARAM_MAX_PRIORITY
#define IPMC_PARAM_DEF_QUERIER_ADRS4    0x0
#define IPMC_PARAM_DEF_QI               125
#define IPMC_PARAM_DEF_RV               2
#define IPMC_PARAM_DEF_QRI              100     /* tenths of seconds */
#define IPMC_PARAM_DEF_LLQI             10      /* tenths of seconds */
#define IPMC_PARAM_DEF_URI              1

typedef BOOL (*vtss_ipmc_rx_callback_t)(void *contxt, const uchar *const frame, const vtss_packet_rx_info_t *const rx_info, BOOL next_ipmc);

typedef struct {
    u32                     sec;
    u16                     msec;
    u16                     usec;
} ipmc_time_t;

typedef struct {
    vtss_ipv6_t             src_ip_addr;
    vtss_ipv6_t             group_addr;
    u8                      msgType;
    u8                      qrv;
    u16                     max_resp_time;
    u16                     no_of_sources;
    u32                     ipmc_pkt_len;
    u32                     offset;
} ipmc_pkt_attribute_t;

typedef struct {
    BOOL                    querier_enabled;
    ipmc_querier_states_t   state;
    u16                     timeout;

    u16                     ipmc_queries_sent;
    u16                     group_queries_sent;

    u16                     proxy_query_timeout;

    /* For Querier Election */
    vtss_ipv4_t             QuerierAdrs4;
    u32                     QuerierUpTime;          /* TimeTicks */
    u32                     OtherQuerierTimeOut;    /* TimeTicks */
    /* Parameters */
    u32                     RobustVari;
    u32                     QueryIntvl;
    u32                     MaxResTime;             /* tenths of seconds */
    u32                     StartUpItv;
    u32                     StartUpCnt;
    u32                     LastQryItv;             /* tenths of seconds */
    u32                     LastQryCnt;
    u32                     UnsolicitR;
} ipmc_querier_sm_t;

typedef struct {
    ipmc_compat_mode_t      mode;

    u32                     old_present_timer;
    u32                     gen_present_timer;
    u32                     sfm_present_timer;
} ipmc_compatibility_t;

typedef struct {
    u16                     igmp_queries;
    u16                     igmp_error_pkt;
    u16                     igmp_v1_membership_join;
    u16                     igmp_v2_membership_join;
    u16                     igmp_v2_membership_leave;
    u16                     igmp_v3_membership_join;

    u16                     mld_queries;
    u16                     mld_error_pkt;
    u16                     mld_v1_membership_report;
    u16                     mld_v1_membership_done;
    u16                     mld_v2_membership_report;
} ipmc_statistics_t;

typedef struct {
    vtss_vid_t              vid;
    u32                     query_version;
    u32                     host_version;
} ipmc_intf_query_host_version_t;

typedef struct {
    BOOL                    valid;

    vtss_vid_t              vid;
    u8                      priority;
    vtss_ipv4_t             querier4_address;

    BOOL                    protocol_status;
    BOOL                    querier_status;
    u32                     compatibility;

    u32                     robustness_variable;
    u32                     query_interval;
    u32                     query_response_interval;
    u32                     last_listener_query_interval;
    u32                     unsolicited_report_interval;
} ipmc_prot_intf_basic_t;

typedef struct {
    vtss_vid_t              vid;

    u8                      mvr;
    ipmc_intf_vtag_t        vtag;
    u8                      priority;

    ipmc_querier_sm_t       querier;
    vtss_ipv6_t             active_querier;

    ipmc_compat_mode_t      cfg_compatibility;  /* Used For Adminstrative Control */

    ipmc_compatibility_t    rtr_compatibility;  /* Used as Listener(Member) */
    ipmc_compatibility_t    hst_compatibility;  /* Used as Router(Switch) */

    ipmc_statistics_t       stats;
} ipmc_prot_intf_entry_param_t;

typedef struct {
    /* INDEX */
    ipmc_ip_version_t               ipmc_version;
    /* param.vid */

    ipmc_prot_intf_entry_param_t    param;

    BOOL                            op_state;
    u8                              vlan_ports[VTSS_BF_SIZE(VTSS_PORT_ARRAY_SIZE)];

    u16                             proxy_report_timeout;
} ipmc_intf_entry_t;

typedef struct ipmc_sfm_srclist_s {
    /* INDEX */
    vtss_ipv6_t                     src_ip;

    struct ipmc_group_entry_s       *grp;
    u8                              port_mask[VTSS_PORT_BF_SIZE];

    ipmc_time_t                     min_tmr;
    union {
        struct {
            ipmc_time_t             t[VTSS_PORT_ARRAY_SIZE];    /* Use for storing into DB */
        } srct_timer;

        struct {
            ipmc_time_t             v[VTSS_PORT_ARRAY_SIZE];    /* Use for calculation */
        } delta_time;
    } tmr;

    u8                              sf_calc[VTSS_PORT_ARRAY_SIZE];  /* Use for calculation */
    BOOL                            sfm_in_hw;

    struct ipmc_sfm_srclist_s       *next;
    BOOL                            mflag;

    u8                              freid;
    u8                              alcid;
} ipmc_sfm_srclist_t;

typedef struct {
    struct ipmc_group_entry_s       *grp;
    ipmc_compatibility_t            compatibility;

    u8                              port_mask[VTSS_PORT_BF_SIZE];
    u8                              chk_listener_state[VTSS_PORT_BF_SIZE];
    u8                              ipmc_sf_port_status[VTSS_PORT_BF_SIZE];
    u8                              ipmc_sf_port_mode[VTSS_PORT_BF_SIZE];
    ipmc_time_t                     min_tmr;
    union {
        struct {
            ipmc_time_t             t[VTSS_PORT_ARRAY_SIZE];    /* Use for storing into DB */
        } fltr_timer;

        struct {
            ipmc_time_t             v[VTSS_PORT_ARRAY_SIZE];    /* Use for calculation */
        } delta_time;
    } tmr;

    ipmc_db_ctrl_hdr_t              *ipmc_sf_do_forward_srclist;
    ipmc_db_ctrl_hdr_t              *ipmc_sf_do_not_forward_srclist;

    BOOL                            asm_in_hw;
} ipmc_group_db_t;

typedef struct ipmc_group_info_s {
    BOOL                            valid;
    struct ipmc_group_entry_s       *grp;
    ipmc_intf_entry_t               *interface;

    ipmc_group_db_t                 db;

    /* For Operational State */
    ipmc_operation_states_t         state;

    ipmc_time_t                     min_tmr;
    ipmc_time_t                     rxmt_timer[VTSS_PORT_ARRAY_SIZE];
    u8                              rxmt_count[VTSS_PORT_ARRAY_SIZE];

    struct ipmc_group_info_s        *next;
    BOOL                            mflag;
} ipmc_group_info_t;

typedef struct ipmc_group_entry_s {
    /* INDEX */
    ipmc_ip_version_t               ipmc_version;
    vtss_vid_t                      vid;
    vtss_ipv6_t                     group_addr;

    ipmc_group_info_t               *info;

    struct ipmc_group_entry_s       *prev;
    struct ipmc_group_entry_s       *next;
    BOOL                            mflag;
} ipmc_group_entry_t;

typedef struct {
    ipmc_ip_version_t       ipmc_version;
    vtss_vid_t              vid;
    vtss_ipv6_t             group_addr;

    ipmc_group_db_t         db;

    BOOL                    valid;
} ipmc_prot_intf_group_entry_t;

typedef struct {
    BOOL                    type;
    u16                     cntr;
    ipmc_sfm_srclist_t      srclist;

    BOOL                    valid;
} ipmc_prot_group_srclist_t;

typedef struct {
    BOOL                    ports[VTSS_PORT_ARRAY_SIZE];
} ipmc_dynamic_router_port_t;

typedef struct {
    /* INDEX */
    ipmc_ip_version_t               ipmc_version;
    /* param.vid */

    ipmc_prot_intf_entry_param_t    param;
    union {
        struct {
            vtss_ipv4_t             adrs;
        } igmp;

        struct {
            vtss_ipv6_t             adrs;
        } mld;
    } QuerierAdrs;
    union {
        struct {
            vtss_ipv4_t             adrs;
        } igmp;

        struct {
            vtss_ipv6_t             adrs;
        } mld;
    } QuerierConf;

    BOOL                            op_state;
    u8                              vlan_ports[VTSS_BF_SIZE(VTSS_PORT_ARRAY_SIZE)];

    u16                             proxy_report_timeout;
} ipmc_intf_map_t;

vtss_rc ipmc_lib_common_init(void);
vtss_rc ipmc_lib_packet_init(void);
vtss_rc ipmc_lib_protocol_init(void);
vtss_rc ipmc_lib_forward_init(void);
vtss_rc ipmc_lib_porting_init(void);
vtss_rc ipmc_lib_profile_init(void);

void ipmc_lib_lock(void);
void ipmc_lib_unlock(void);

/* Create tree in a given structure */
BOOL ipmc_lib_db_create_tree(ipmc_db_ctrl_hdr_t         *list,
                             char                       *name,
                             u32                        max_entry_cnt,
                             size_t                     size_of_entry,
                             vtss_avl_tree_cmp_func_t   compare_func);
/* Destroy tree in a given structure */
BOOL ipmc_lib_db_destroy_tree(ipmc_db_ctrl_hdr_t *list);
/* Add or update (if existed) an entry in a given structure */
BOOL ipmc_lib_db_set_entry(ipmc_db_ctrl_hdr_t *list, void *entry);
/* Delete a designated entry from a given structure */
BOOL ipmc_lib_db_delete_entry(ipmc_db_ctrl_hdr_t *list, void *entry);
/* Delete all entries in a given structure */
BOOL ipmc_lib_db_delete_all_entry(ipmc_db_ctrl_hdr_t *list);
/* Get the max_cnt in a given structure */
u32 ipmc_lib_db_get_max_cnt(ipmc_db_ctrl_hdr_t *list);
/* Get the current_cnt in a given structure */
u32 ipmc_lib_db_get_current_cnt(ipmc_db_ctrl_hdr_t *list);
/* Get the size_of_entry in a given structure */
u32 ipmc_lib_db_get_size_of_entry(ipmc_db_ctrl_hdr_t *list);
/* Get the first entry in a given structure */
BOOL ipmc_lib_db_get_first_entry(ipmc_db_ctrl_hdr_t *header, void *entry);
/* Get the designated entry in a given structure, the specific keys of current entry should be input */
BOOL ipmc_lib_db_get_entry(ipmc_db_ctrl_hdr_t *header, void *entry);
/* Get the next entry in a given structure, the specific keys of current entry should be input */
BOOL ipmc_lib_db_get_next_entry(ipmc_db_ctrl_hdr_t *header, void *entry);

#endif /* _IPMC_LIB_BASE_H_ */
