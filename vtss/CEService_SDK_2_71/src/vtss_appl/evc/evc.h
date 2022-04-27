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

 $Id$
 $Revision$

*/

#ifndef _VTSS_EVC_H_
#define _VTSS_EVC_H_

/* ================================================================= *
 *  Trace definitions
 * ================================================================= */

#include <vtss_module_id.h>
#include <vtss_trace_lvl_api.h>

#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_EVC

#define VTSS_TRACE_GRP_DEFAULT 0
#define TRACE_GRP_CRIT         1
#define TRACE_GRP_L2CP         2
#define TRACE_GRP_CNT          3

#include <vtss_trace_api.h>

/* ================================================================= *
 *  L2CP structures
 * ================================================================= */

#if defined(VTSS_ARCH_SERVAL)

/* Information for a L2CP protocol */
typedef struct {
    vtss_ece_type_t type;  /* ETYPE/LLC/SNAP */
    vtss_mac_t      dmac;  /* IEEE/Cisco DMAC */
    u16             etype; /* Valid for ETYPE */
    vtss_vcap_u16_t pid;   /* Protocol ID */
} evc_l2cp_info_t;

/* EVC ACL entry */
typedef struct {
    BOOL add;                      /* ACE must be added */
    u8   ports[VTSS_PORT_BF_SIZE]; /* Port list */
} evc_ace_entry_t;

/* EVC ACL information per ACE ID */
typedef struct {
    evc_ace_entry_t entry[ACE_MAX];
} evc_acl_info_t;

/* EVC packet entry */
typedef struct {
    BOOL add;                      /* Registration must be added */
    void *filter_id;               /* Filter ID */
    u8   ports[VTSS_PORT_BF_SIZE]; /* Port list */
} evc_packet_info_t;

#endif /* VTSS_ARCH_SERVAL */

/* ================================================================= *
 *  EVC global configuration
 * ================================================================= */

/* EVC global configuration */
typedef struct {
    BOOL port_check;
    u8   resv[31];
} evc_global_conf_t;

/* EVC port block */
#define EVC_GLOBAL_BLK_VERSION 1
typedef struct {
    u32               version; /* Block version */
    evc_global_conf_t conf;    /* Global configuration */
} evc_global_blk_t;

/* ================================================================= *
 *  EVC port configuration
 * ================================================================= */

/* EVC port flags */
#define EVC_PORT_FLAG_DEI_COLOURING 0x00000001
#define EVC_PORT_FLAG_INNER_TAG     0x00000002
#define EVC_PORT_FLAG_DMAC_DIP      0x00000004
#define EVC_PORT_FLAG_DMAC_DIP_ADV  0x00000008

/* EVC port configuration */
typedef struct {
    u32                  flags;
#if defined(VTSS_ARCH_SERVAL)
    vtss_vcap_key_type_t key_type;
    vtss_vcap_key_type_t key_type_adv;
#endif /* VTSS_ARCH_SERVAL */
    u8                   bpdu[16];
    u8                   garp[16];
} evc_port_conf_t;

/* EVC port block */
#define EVC_PORT_BLK_VERSION 1
typedef struct {
    u32             version;           /* Block version */
    evc_port_conf_t table[VTSS_PORTS]; /* Port configuration */
} evc_port_blk_t;

/* ================================================================= *
 *  EVC policer configuration
 * ================================================================= */

/* EVC policer configuration */
typedef struct {
    vtss_evc_policer_conf_t conf;
} evc_pol_conf_t;

/* EVC policer block */
#define EVC_POL_BLK_VERSION 1
typedef struct {
    u32            version;              /* Block version */
    evc_pol_conf_t table[EVC_POL_COUNT]; /* Policer table */
} evc_pol_blk_t;

/* ================================================================= *
 *  EVC configuration
 * ================================================================= */

/* EVC flags */
#define EVC_FLAG_ENABLED      0x0001
#define EVC_FLAG_LEARN        0x0002
#define EVC_FLAG_VID_TUNNEL   0x0004
#define EVC_FLAG_IT_C         0x0008
#define EVC_FLAG_IT_S         0x0010
#define EVC_FLAG_IT_S_CUSTOM  0x0020
#define EVC_FLAG_IT_PRES      0x0040
#define EVC_FLAG_IT_DEI       0x0080
#define EVC_FLAG_CHANGED      0x4000
#define EVC_FLAG_CONFLICT     0x8000

/* EVC entry */
typedef struct {
    vtss_vid_t            vid;
    vtss_vid_t            ivid;
    u16                   flags;
#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
    vtss_evc_policer_id_t policer_id;
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */
#if defined(VTSS_ARCH_CARACAL)
    vtss_vid_t            uvid;
    vtss_vid_t            it_vid;
    u8                    it_pcp;
#endif /* VTSS_ARCH_CARACAL */
#if defined(VTSS_FEATURE_MPLS)
    vtss_mpls_xc_idx_t    pw_ingress_xc;
    vtss_mpls_xc_idx_t    pw_egress_xc;
#endif /* VTSS_FEATURE_MPLS */
    u8                    ports[VTSS_PORT_BF_SIZE];
} evc_conf_t;

/* EVC block */
#define EVC_BLK_VERSION 1
typedef struct {
    u32        version;             /* Block version */
    evc_conf_t table[EVC_ID_COUNT]; /* EVC table */
} evc_blk_t;

/* ================================================================= *
 *  ECE configuration
 * ================================================================= */

/* ECE key flags */
#define ECE_FLAG_KEY_DMAC_MC_VLD     0x00000001
#define ECE_FLAG_KEY_DMAC_MC_1       0x00000002
#define ECE_FLAG_KEY_DMAC_BC_VLD     0x00000004
#define ECE_FLAG_KEY_DMAC_BC_1       0x00000008
#define ECE_FLAG_KEY_IN_TAGGED_VLD   0x00000010
#define ECE_FLAG_KEY_IN_TAGGED_1     0x00000020
#define ECE_FLAG_KEY_IN_DEI_VLD      0x00000040
#define ECE_FLAG_KEY_IN_DEI_1        0x00000080
#define ECE_FLAG_KEY_TAGGED_VLD      0x00000100
#define ECE_FLAG_KEY_TAGGED_1        0x00000200
#define ECE_FLAG_KEY_DEI_VLD         0x00000400
#define ECE_FLAG_KEY_DEI_1           0x00000800
#define ECE_FLAG_KEY_IPV4_FRAG_VLD   0x00001000
#define ECE_FLAG_KEY_IPV4_FRAG_1     0x00002000
#define ECE_FLAG_KEY_VID_RANGE       0x00004000
#define ECE_FLAG_KEY_DSCP_RANGE      0x00008000
#define ECE_FLAG_KEY_SPORT_RANGE     0x00010000
#define ECE_FLAG_KEY_DPORT_RANGE     0x00020000
#define ECE_FLAG_KEY_IN_VID_RANGE    0x00040000
#define ECE_FLAG_KEY_IN_S_TAGGED_VLD 0x00080000
#define ECE_FLAG_KEY_IN_S_TAGGED_1   0x00100000
#define ECE_FLAG_KEY_S_TAGGED_VLD    0x00200000
#define ECE_FLAG_KEY_S_TAGGED_1      0x00400000
#define ECE_FLAG_KEY_LOOKUP          0x00800000
#define ECE_FLAG_KEY_TYPE_L2CP       0x01000000

/* ECE action flags */
#define ECE_FLAG_ACT_DIR_UNI_TO_NNI    0x00000001
#define ECE_FLAG_ACT_DIR_NNI_TO_UNI    0x00000002
#define ECE_FLAG_ACT_POP_1             0x00000004
#define ECE_FLAG_ACT_POP_2             0x00000008
#define ECE_FLAG_ACT_OT_ENA            0x00000010
#define ECE_FLAG_ACT_OT_PCP_MODE_CLASS 0x00000020
#define ECE_FLAG_ACT_OT_DEI_MODE_CLASS 0x00000040
#define ECE_FLAG_ACT_OT_DEI_MODE_FIXED 0x00000080
#define ECE_FLAG_ACT_OT_DEI_MODE_DP    0x00000100
#define ECE_FLAG_ACT_OT_DEI            0x00000200
#define ECE_FLAG_ACT_IT_TYPE_C         0x00000400
#define ECE_FLAG_ACT_IT_TYPE_S         0x00000800
#define ECE_FLAG_ACT_IT_TYPE_S_CUSTOM  0x00001000
#define ECE_FLAG_ACT_IT_PCP_MODE_CLASS 0x00002000
#define ECE_FLAG_ACT_IT_DEI_MODE_CLASS 0x00004000
#define ECE_FLAG_ACT_IT_DEI_MODE_FIXED 0x00008000
#define ECE_FLAG_ACT_IT_DEI_MODE_DP    0x00010000
#define ECE_FLAG_ACT_IT_DEI            0x00020000
#define ECE_FLAG_ACT_PRIO_ENA          0x00040000
#define ECE_FLAG_ACT_DP_ENA            0x00080000
#define ECE_FLAG_ACT_OT_PCP_MODE_MAP   0x00100000
#define ECE_FLAG_ACT_IT_PCP_MODE_MAP   0x00200000
#define ECE_FLAG_ACT_RULE_RX           0x00400000
#define ECE_FLAG_ACT_RULE_TX           0x00800000
#define ECE_FLAG_ACT_TX_LOOKUP_VID     0x01000000
#define ECE_FLAG_ACT_TX_LOOKUP_VID_PCP 0x02000000
#define ECE_FLAG_ACT_L2CP_TUNNEL       0x04000000
#define ECE_FLAG_ACT_L2CP_DISCARD      0x08000000
#define ECE_FLAG_ACT_L2CP_PEER         0x10000000
#define ECE_FLAG_ACT_L2CP_DMAC_CISCO   0x20000000

/* ECE conflict flag */
#define ECE_FLAG_CONFLICT              0x80000000

/* 16-bit range */
typedef struct {
    u16 low;
    u16 high;
} evc_u16_range_t;

/* 8-bit range */
typedef struct {
    u8 low;
    u8 high;
} evc_u8_range_t;

typedef struct {
    evc_u8_range_t  dscp;
#if defined(VTSS_ARCH_CARACAL) || defined(VTSS_ARCH_SERVAL)
    vtss_vcap_u8_t  proto;
    vtss_vcap_ip_t  sip;  
#if defined(VTSS_ARCH_SERVAL)
    vtss_vcap_ip_t  dip;  
#endif /* VTSS_ARCH_SERVAL */
    evc_u16_range_t sport;
    evc_u16_range_t dport;
#endif /* VTSS_ARCH_CARACAL/SERVAL */
} evc_ipv4_data_t;

typedef struct {
    evc_u8_range_t  dscp;
#if defined(VTSS_ARCH_CARACAL) || defined(VTSS_ARCH_SERVAL)
    vtss_vcap_u8_t  proto;
    vtss_vcap_u32_t sip;  
#if defined(VTSS_ARCH_SERVAL)
    vtss_vcap_u32_t dip;  
#endif /* VTSS_ARCH_SERVAL */
    evc_u16_range_t sport;
    evc_u16_range_t dport;
#endif /* VTSS_ARCH_CARACAL/SERVAL */
} evc_ipv6_data_t;

/* ECE entry */
typedef struct {
    vtss_ece_id_t     ece_id;

    /* Key/action flags */
    u32               flags_key;
    u32               flags_act;

    /* Key fields */
    u8                ports[VTSS_PORT_BF_SIZE];
#if defined(VTSS_ARCH_CARACAL) || defined(VTSS_ARCH_SERVAL)
    vtss_vcap_u48_t   smac;
#endif /* VTSS_ARCH_CARACAL/SERVAL */
#if defined(VTSS_ARCH_SERVAL)
    vtss_vcap_u48_t   dmac;
#endif /* VTSS_ARCH_SERVAL */
    evc_u16_range_t   vid;
    vtss_vcap_u8_t    pcp;
#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
    evc_u16_range_t   in_vid;
    vtss_vcap_u8_t    in_pcp;
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */
    vtss_ece_type_t   type;
    union {
#if defined(VTSS_ARCH_SERVAL)
        vtss_ece_frame_etype_t etype;
        vtss_ece_frame_llc_t   llc;
        vtss_ece_frame_snap_t  snap;
        evc_l2cp_t             l2cp;
#endif /* VTSS_ARCH_SERVAL */
        evc_ipv4_data_t        ipv4;
        evc_ipv6_data_t        ipv6;
    } data;

    /* Action fields */
    vtss_evc_id_t         evc_id;
#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
    vtss_evc_policer_id_t policer_id;
    vtss_vid_t            ot_vid;
    vtss_vid_t            it_vid;
    u8                    it_pcp;
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */
    u8                    ot_pcp;
    u8                    policy_no;
#if defined(VTSS_ARCH_CARACAL) || defined(VTSS_ARCH_SERVAL)
    u8                    prio;
#endif /* VTSS_ARCH_CARACAL/SERVAL */
#if defined(VTSS_ARCH_SERVAL)
    u8                    dp;
    vtss_ece_type_t       frame_type;
    vtss_vcap_u40_t       frame_data; /* Frame data for ETYPE/LLC/SNAP L2CP */
    vtss_vcap_u48_t       frame_dmac;
#endif /* VTSS_ARCH_SERVAL */
} evc_ece_conf_t;

/* ECE block */
#define EVC_ECE_BLK_VERSION 1
typedef struct {
    u32            version;              /* Block version */
    u32            count;                /* Number of valid entries */
    evc_ece_conf_t table[EVC_ECE_COUNT]; /* ECE table */
} evc_ece_blk_t;

/* ================================================================= *
 *  EVC definitions
 * ================================================================= */

/* ECE entry */
typedef struct evc_ece_t {
    struct evc_ece_t *next;
    evc_ece_conf_t   conf;
} evc_ece_t;

/* Maximum number of EVC change registrations */
#define EVC_CHANGE_REG_MAX 16

/* ================================================================= *
 *  EVC global variables
 * ================================================================= */

typedef struct {
    critd_t                crit;
    evc_mgmt_global_conf_t conf;                                /* Global configuration */
    evc_port_conf_t        port_conf[VTSS_PORTS];               /* Port configuration */
    evc_pol_conf_t         pol_conf[EVC_POL_COUNT];             /* Policer configuration */
    evc_conf_t             evc_table[EVC_ID_COUNT];             /* EVC table */
    evc_ece_t              ece_table[EVC_ECE_COUNT];            /* ECE table */
    evc_ece_t              *ece_used;                           /* ECE used list */
    evc_ece_t              *ece_free;                           /* ECE free list */
    evc_change_callback_t  change_callback[EVC_CHANGE_REG_MAX]; /* EVC change registrations */
    u8                     vid_added[VTSS_BF_SIZE(VTSS_VIDS)];  /* Added VIDs */
#if defined(VTSS_ARCH_SERVAL)
    evc_l2cp_info_t        l2cp_info[EVC_L2CP_CNT];             /* L2CP info table */
    evc_acl_info_t         acl_info;                            /* ACL information */
    evc_acl_info_t         acl_info_old;                        /* Previous information */
    evc_packet_info_t      packet_info;                         /* Packet information */
    vtss_etype_t           s_custom_etype;                      /* S-custom Ethernet Type */
#endif /* VTSS_ARCH_SERVAL */
} evc_global_t;

#endif /* _VTSS_EVC_H_*/
