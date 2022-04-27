/*

 Vitesse API software.

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

#ifndef _VTSS_EVC_STATE_H_
#define _VTSS_EVC_STATE_H_

#if defined(VTSS_FEATURE_EVC)
/* EVC port information entry */
typedef struct {
    u16 uni_count;
    u16 nni_count;
} vtss_evc_port_info_t;

#if defined(VTSS_ARCH_JAGUAR_1)
#define VTSS_JR1_TDLB_CNT 512
#define VTSS_TDLB_ID_NONE 0xffff

/* EVC policer information */
typedef struct {
    u16 tdlb_id; /* Tunnel policer_id */
} vtss_evc_policer_info_t;
#endif /* VTSS_ARCH_JAGUAR_1 */

/* EVC entry */
typedef struct {
    BOOL                  enable;    /* EVC valid indication */
    BOOL                  learning;  /* Learning enabled */
    vtss_vid_t            vid;       /* PB VID */
    vtss_vid_t            ivid;      /* Classified VID */
#if defined(VTSS_ARCH_CARACAL)
    vtss_vid_t            uvid;      /* Classified VID */
    vtss_evc_inner_tag_t  inner_tag; /* Inner tag */
#endif /* VTSS_ARCH_CARACAL */
    u8                    ports[VTSS_PORT_BF_SIZE];
    vtss_evc_policer_id_t policer_id;
#if defined(VTSS_ARCH_SERVAL)
    u8                    voe_idx[VTSS_PORT_ARRAY_SIZE]; /* VOE index */
#endif /* VTSS_ARCH_SERVAL */
#if defined(VTSS_FEATURE_MPLS)
    vtss_mpls_xc_idx_t    pw_ingress_xc; /* XC for ingress unidirectional pseudo-wire */
    vtss_mpls_xc_idx_t    pw_egress_xc;  /* XC for egress unidirectional pseudo-wire */
#endif /* VTSS_FEATURE_MPLS */
} vtss_evc_entry_t;

/* EVC information */
typedef struct {
    u32              max_count;        /* Maximum number of rules */
    u32              count;            /* Actual number of rules */
    vtss_evc_entry_t table[VTSS_EVCS]; /* Entries */
} vtss_evc_info_t;

#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
typedef struct {
    vtss_chip_counter_t frames;
    vtss_chip_counter_t bytes;
} vtss_chip_counter_pair_t;

typedef struct {
    vtss_chip_counter_pair_t rx_green;   /**< Rx green frames/bytes */
    vtss_chip_counter_pair_t rx_yellow;  /**< Rx yellow frames/bytes */
    vtss_chip_counter_pair_t rx_red;     /**< Rx red frames/bytes */
    vtss_chip_counter_pair_t rx_discard; /**< Rx discarded frames/bytes */
    vtss_chip_counter_pair_t tx_discard; /**< Tx discarded frames/bytes */
    vtss_chip_counter_pair_t tx_green;   /**< Tx green frames/bytes */
    vtss_chip_counter_pair_t tx_yellow;  /**< Tx yellow frames/bytes */
} vtss_sdx_counters_t;

/* SDX entry */
typedef struct vtss_sdx_entry_t {
    struct vtss_sdx_entry_t *next;   /* next in list */
    u16                     sdx;     /* SDX value */
    vtss_port_no_t          port_no; /* UNI/NNI port number */
} vtss_sdx_entry_t;

/* SDX zero is reserved */
#define VTSS_JR1_SDX_CNT  4095
#define VTSS_SRVL_SDX_CNT 1022 /* SDX 1023 is reserved for CPU Tx operations */

#if defined(VTSS_ARCH_JAGUAR_1)
#define VTSS_SDX_CNT VTSS_JR1_SDX_CNT
#else
#define VTSS_SDX_CNT VTSS_SRVL_SDX_CNT
#endif

/* Table and free list for ISDX/ESDX */
typedef struct {
    u32              count;               /* Actual number of rules */
    vtss_sdx_entry_t *free;               /* List of free entries */
    vtss_sdx_entry_t table[VTSS_SDX_CNT]; /* Table of entries */
} vtss_sdx_list_t;

/* SDX information */
typedef struct {
    u32                 max_count; /* Maximum number of rules */
    u32                 poll_idx;  /* Counter polling index */
    vtss_sdx_counters_t sdx_table[VTSS_SDX_CNT];
    vtss_sdx_list_t     isdx;      /* ISDX list */
    vtss_sdx_list_t     esdx;      /* ESDX list */
} vtss_sdx_info_t;
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */

/* ECE key flags */
#define VTSS_ECE_KEY_DMAC_MC_VLD         0x00000001
#define VTSS_ECE_KEY_DMAC_MC_1           0x00000002
#define VTSS_ECE_KEY_DMAC_BC_VLD         0x00000004
#define VTSS_ECE_KEY_DMAC_BC_1           0x00000008
#define VTSS_ECE_KEY_TAG_DEI_VLD         0x00000010
#define VTSS_ECE_KEY_TAG_DEI_1           0x00000020
#define VTSS_ECE_KEY_TAG_TAGGED_VLD      0x00000040
#define VTSS_ECE_KEY_TAG_TAGGED_1        0x00000080
#define VTSS_ECE_KEY_TAG_S_TAGGED_VLD    0x00000100
#define VTSS_ECE_KEY_TAG_S_TAGGED_1      0x00000200
#define VTSS_ECE_KEY_IN_TAG_DEI_VLD      0x00000400
#define VTSS_ECE_KEY_IN_TAG_DEI_1        0x00000800
#define VTSS_ECE_KEY_IN_TAG_TAGGED_VLD   0x00001000
#define VTSS_ECE_KEY_IN_TAG_TAGGED_1     0x00002000
#define VTSS_ECE_KEY_IN_TAG_S_TAGGED_VLD 0x00004000
#define VTSS_ECE_KEY_IN_TAG_S_TAGGED_1   0x00008000
#define VTSS_ECE_KEY_LOOKUP              0x00010000
#define VTSS_ECE_KEY_PROT_ETYPE          0x00020000
#define VTSS_ECE_KEY_PROT_LLC            0x00040000
#define VTSS_ECE_KEY_PROT_SNAP           0x00080000
#define VTSS_ECE_KEY_PROT_IPV4           0x00100000
#define VTSS_ECE_KEY_PROT_IPV6           0x00200000

/* ECE action flags */
#define VTSS_ECE_ACT_DIR_UNI_TO_NNI     0x00000001 /* Flow direction, default BOTH */
#define VTSS_ECE_ACT_DIR_NNI_TO_UNI     0x00000002
#define VTSS_ECE_ACT_POP_1              0x00000004
#define VTSS_ECE_ACT_POP_2              0x00000008
#define VTSS_ECE_ACT_IT_TYPE_C          0x00000010 /* Inner tag, default NONE */
#define VTSS_ECE_ACT_IT_TYPE_S          0x00000020
#define VTSS_ECE_ACT_IT_TYPE_S_CUSTOM   0x00000040
#define VTSS_ECE_ACT_IT_PCP_MODE_FIXED  0x00000080 /* Inner PCP mode, default CLASSIFIED */
#define VTSS_ECE_ACT_IT_PCP_MODE_MAPPED 0x00000100
#define VTSS_ECE_ACT_IT_DEI             0x00000200
#define VTSS_ECE_ACT_IT_DEI_MODE_FIXED  0x00000400 /* Inner DEI mode, default CLASSIFIED */
#define VTSS_ECE_ACT_IT_DEI_MODE_DP     0x00000800
#define VTSS_ECE_ACT_OT_ENA             0x00001000 
#define VTSS_ECE_ACT_OT_PCP_MODE_FIXED  0x00002000 /* Outer PCP mode, default CLASSIFIED */
#define VTSS_ECE_ACT_OT_PCP_MODE_MAPPED 0x00004000
#define VTSS_ECE_ACT_OT_DEI             0x00008000
#define VTSS_ECE_ACT_OT_DEI_MODE_FIXED  0x00010000 /* Outer DEI mode, default CLASSIFIED */
#define VTSS_ECE_ACT_OT_DEI_MODE_DP     0x00020000
#define VTSS_ECE_ACT_PRIO_ENA           0x00040000
#define VTSS_ECE_ACT_DP_ENA             0x00080000
#define VTSS_ECE_ACT_RULE_RX            0x00100000 /* Rule direction, default BOTH */
#define VTSS_ECE_ACT_RULE_TX            0x00200000
#define VTSS_ECE_ACT_TX_LOOKUP_VID_PCP  0x00400000 /* Tx lookup, default VID */
#define VTSS_ECE_ACT_TX_LOOKUP_ISDX     0x00800000
#define VTSS_ECE_ACT_POLICY_NONE        0x01000000

#define VTSS_ECE_ACT_DIR_ONE        (VTSS_ECE_ACT_DIR_UNI_TO_NNI | VTSS_ECE_ACT_DIR_NNI_TO_UNI)
#define VTSS_ECE_ACT_IT_TYPE_USED   (VTSS_ECE_ACT_IT_TYPE_C | VTSS_ECE_ACT_IT_TYPE_S | VTSS_ECE_ACT_IT_TYPE_S_CUSTOM)

/* ECE entry */
typedef struct vtss_ece_entry_t {
    struct vtss_ece_entry_t *next;     /* Next in list */
#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
    vtss_sdx_entry_t        *isdx_list; /* ISDX list */
    vtss_sdx_entry_t        *esdx_list; /* ESDX list */
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */
    vtss_ece_id_t           ece_id;    /* ECE ID */

    /* Key/action flags */
    u32                     key_flags;
    u32                     act_flags;

    /* Action fields */
    vtss_evc_id_t           evc_id;
#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
    vtss_evc_policer_id_t   policer_id;
    vtss_vid_t              it_vid;
    vtss_vid_t              ot_vid;
    u8                      it_pcp;
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */
#if defined(VTSS_ARCH_CARACAL) | defined(VTSS_ARCH_SERVAL)
    u8                      prio;
#endif /* VTSS_ARCH_CARACAL/SERVAL */
#if defined(VTSS_ARCH_SERVAL)
    u8                      dp;
#endif /* VTSS_ARCH_SERVAL */
    u8                      ot_pcp;
    u8                      policy_no;

    /* Key fields */
    vtss_vcap_u8_t          pcp;
    vtss_vcap_vr_t          vid;
    u8                      ports[VTSS_PORT_BF_SIZE];

#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
    vtss_vcap_vr_t          in_vid;
    vtss_vcap_u8_t          in_pcp;
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */
#if defined(VTSS_ARCH_CARACAL) || defined(VTSS_ARCH_SERVAL)
    vtss_vcap_u48_t         smac;
#endif /* VTSS_ARCH_CARACAL/SERVAL */
#if defined(VTSS_ARCH_SERVAL)
    vtss_vcap_u48_t         dmac;
#endif /* VTSS_ARCH_CARACAL */
    union
    {
#if defined(VTSS_ARCH_SERVAL)
        vtss_ece_frame_etype_t etype;
        vtss_ece_frame_llc_t   llc;
        vtss_ece_frame_snap_t  snap;
#endif /* VTSS_ARCH_SERVAL */
        vtss_ece_frame_ipv4_t  ipv4;
        vtss_ece_frame_ipv6_t  ipv6;
    } frame;  /**< Frame type specific data */
} vtss_ece_entry_t;

/* Number of ECEs */
#define VTSS_ECES VTSS_EVCS

/* ECE info */
typedef struct {
    u32              max_count;        /* Maximum number of rules */
    u32              count;            /* Actual number of rules */
    vtss_ece_entry_t *used;            /* Used list */
    vtss_ece_entry_t *free;            /* Free list */
    vtss_ece_entry_t table[VTSS_ECES]; /* Entries */
} vtss_ece_info_t;

/* Resource update command */
typedef enum {
    VTSS_RES_CMD_CALC, /* Calculate resources */
    VTSS_RES_CMD_DEL,  /* Delete resources */
    VTSS_RES_CMD_ADD   /* Add resources */
} vtss_res_cmd_t;

#if defined(VTSS_ARCH_SERVAL)
/* Two MCEs pr COS-ID can map to the same VOE */
#define VTSS_MCES (2 * 8 * VTSS_OAM_PATH_SERVICE_VOE_CNT)

/* MCE entry */
typedef struct vtss_mce_entry_t {
    struct vtss_mce_entry_t *next;      /* Next in list */
    vtss_mce_t              conf;       /* Configuration */
    vtss_sdx_entry_t        *isdx_list; /* ISDX list */
    vtss_sdx_entry_t        *esdx_list; /* ESDX list */
} vtss_mce_entry_t;

/* MCE info */
typedef struct {
    u32              max_count;        /* Maximum number of rules */
    u32              count;            /* Actual number of rules */
    vtss_mce_entry_t *used;            /* Used list */
    vtss_mce_entry_t *free;            /* Free list */
    vtss_mce_entry_t table[VTSS_MCES]; /* Entries */
} vtss_mce_info_t;
#endif /* VTSS_ARCH_SERVAL */

#endif /* VTSS_FEATURE_EVC */

#if defined(VTSS_ARCH_SERVAL)
#define VTSS_EVC_VOE_IDX_NONE 0xff
#endif /* VTSS_ARCH_SERVAL */

typedef struct {
#if defined(VTSS_FEATURE_EVC)
    /* CIL function pointers */
    vtss_rc (* port_conf_set)(struct vtss_state_s *vtss_state,
                              const vtss_port_no_t port_no);
    vtss_rc (* add)(struct vtss_state_s *vtss_state,
                        const vtss_evc_id_t evc_id, const vtss_evc_conf_t *const conf);
    vtss_rc (* del)(struct vtss_state_s *vtss_state,
                        const vtss_evc_id_t evc_id);
    vtss_rc (* ece_add)(struct vtss_state_s *vtss_state,
                        const vtss_ece_id_t ece_id, const vtss_ece_t *const ece);
    vtss_rc (* ece_del)(struct vtss_state_s *vtss_state,
                        const vtss_ece_id_t ece_id);
#if defined(VTSS_ARCH_CARACAL) || defined(VTSS_ARCH_SERVAL)
    vtss_rc (* mce_add)(struct vtss_state_s *vtss_state,
                        const vtss_mce_id_t mce_id, const vtss_mce_t *const mce);
    vtss_rc (* mce_del)(struct vtss_state_s *vtss_state,
                        const vtss_mce_id_t mce_id);
#if defined(VTSS_ARCH_SERVAL)
    vtss_rc (* mce_port_get)(struct vtss_state_s *vtss_state,
                             const vtss_mce_id_t mce_id, const vtss_port_no_t port_no, 
                             vtss_mce_port_info_t *const info);
#endif /* VTSS_ARCH_SERVAL */
#endif /* VTSS_ARCH_CARACAL/SERVAL */
#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
    vtss_rc (* ece_counters_get)(struct vtss_state_s *vtss_state,
                                 const vtss_ece_id_t  ece_id,
                                 const vtss_port_no_t port_no,
                                 vtss_evc_counters_t  *const counters);
    vtss_rc (* ece_counters_clear)(struct vtss_state_s *vtss_state,
                                   const vtss_ece_id_t  ece_id,
                                   const vtss_port_no_t port_no);
    vtss_rc (* counters_get)(struct vtss_state_s *vtss_state,
                             const vtss_evc_id_t  evc_id,
                             const vtss_port_no_t port_no,
                             vtss_evc_counters_t  *const counters);
    vtss_rc (* counters_clear)(struct vtss_state_s *vtss_state,
                               const vtss_evc_id_t  evc_id,
                               const vtss_port_no_t port_no);
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */
    vtss_rc (* update)(struct vtss_state_s *vtss_state,
                       vtss_evc_id_t evc_id, vtss_res_t *res, vtss_res_cmd_t cmd);
    vtss_rc (* ece_update)(struct vtss_state_s *vtss_state,
                           vtss_ece_entry_t *ece, vtss_res_t *res, vtss_res_cmd_t cmd);
#if defined(VTSS_SDX_CNT)
    vtss_rc (* sdx_counters_update)(struct vtss_state_s *vtss_state,
                                    vtss_sdx_entry_t *isdx, vtss_sdx_entry_t *esdx,
                                    vtss_evc_counters_t *const counters, BOOL clear);
#endif /* VTSS_SDX_CNT */

    /* Configuration/state */
    vtss_evc_port_conf_t port_conf[VTSS_PORT_ARRAY_SIZE];
    vtss_evc_port_conf_t port_conf_old;
    vtss_evc_port_info_t port_info[VTSS_PORT_ARRAY_SIZE];
#if defined(VTSS_ARCH_JAGUAR_1)
    vtss_evc_policer_info_t policer_info[VTSS_EVC_POLICERS];
    BOOL                    tdlb_used[VTSS_JR1_TDLB_CNT];
#endif /* VTSS_ARCH_JAGUAR_1 */
    vtss_evc_info_t      info;
    vtss_ece_info_t      ece_info;
#if defined(VTSS_ARCH_SERVAL)
    vtss_mce_info_t      mce_info;
#endif /* VTSS_ARCH_SERVAL */
#if defined(VTSS_SDX_CNT)
    vtss_sdx_info_t      sdx_info;
#endif /* VTSS_SDX_CNT */
#endif /* VTSS_FEATURE_EVC */
    int                  dummy;
} vtss_evc_state_t;

#if defined(VTSS_FEATURE_EVC)
vtss_rc vtss_evc_inst_create(struct vtss_state_s *vtss_state);
vtss_rc vtss_evc_restart_sync(struct vtss_state_s *vtss_state);

#if defined(VTSS_SDX_CNT)
vtss_sdx_entry_t *vtss_cmn_sdx_alloc(struct vtss_state_s *vtss_state, vtss_port_no_t port_no, BOOL isdx);
void vtss_cmn_sdx_free(struct vtss_state_s *vtss_state, vtss_sdx_entry_t *sdx, BOOL isdx);
vtss_sdx_entry_t *vtss_cmn_ece_sdx_alloc(struct vtss_state_s *vtss_state,
                                         vtss_ece_entry_t *ece, vtss_port_no_t port_no, BOOL isdx);
vtss_rc vtss_cmn_ece_sdx_free(struct vtss_state_s *vtss_state,
                              vtss_ece_entry_t *ece, vtss_port_no_t port_no, BOOL isdx);
#endif /* VTSS_SDX_CNT */

BOOL vtss_cmn_evc_port_check(struct vtss_state_s *vtss_state);
BOOL vtss_cmn_ece_es0_needed(vtss_ece_entry_t *ece);
vtss_ece_dir_t vtss_cmn_ece_dir_get(vtss_ece_entry_t *ece);
#if defined(VTSS_ARCH_SERVAL)
vtss_ece_rule_t vtss_cmn_ece_rule_get(vtss_ece_entry_t *ece);
#endif /* VTSS_ARCH_SERVAL */
vtss_vcap_bit_t vtss_cmn_ece_bit_get(vtss_ece_entry_t *ece, u32 mask_vld, u32 mask_1);
vtss_rc vtss_cmn_evc_add(struct vtss_state_s *vtss_state,
                         const vtss_evc_id_t evc_id, const vtss_evc_conf_t *const conf);
vtss_rc vtss_cmn_evc_del(struct vtss_state_s *vtss_state, const vtss_evc_id_t evc_id);
vtss_rc vtss_cmn_ece_add(struct vtss_state_s *vtss_state,
                         const vtss_ece_id_t ece_id, const vtss_ece_t *const ece);
vtss_rc vtss_cmn_ece_del(struct vtss_state_s *vtss_state, const vtss_ece_id_t ece_id);
#if defined(VTSS_SDX_CNT)
vtss_rc vtss_cmn_ece_counters_get(struct vtss_state_s *vtss_state,
                                  const vtss_ece_id_t  ece_id,
                                  const vtss_port_no_t port_no,
                                  vtss_evc_counters_t  *const counters);
vtss_rc vtss_cmn_ece_counters_clear(struct vtss_state_s *vtss_state,
                                    const vtss_ece_id_t  ece_id,
                                    const vtss_port_no_t port_no);
vtss_rc vtss_cmn_evc_counters_get(struct vtss_state_s *vtss_state,
                                  const vtss_evc_id_t  evc_id,
                                  const vtss_port_no_t port_no,
                                  vtss_evc_counters_t  *const counters);
vtss_rc vtss_cmn_evc_counters_clear(struct vtss_state_s *vtss_state,
                                    const vtss_evc_id_t  evc_id,
                                    const vtss_port_no_t port_no);
#endif /* VTSS_SDX_CNT */
#if defined(VTSS_ARCH_CARACAL)
vtss_rc vtss_cmn_mce_add(struct vtss_state_s *vtss_state,
                         const vtss_mce_id_t mce_id, const vtss_mce_t *const mce);
vtss_rc vtss_cmn_mce_del(struct vtss_state_s *vtss_state, const vtss_mce_id_t mce_id);
#endif /* VTSS_ARCH_CARACAL */

void vtss_evc_debug_print(struct vtss_state_s *vtss_state,
                          const vtss_debug_printf_t pr,
                          const vtss_debug_info_t   *const info);

#endif /* VTSS_FEATURE_EVC */

#endif /* _VTSS_EVC_STATE_H_ */
