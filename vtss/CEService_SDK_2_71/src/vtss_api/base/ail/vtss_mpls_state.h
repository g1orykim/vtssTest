/*

 Vitesse API software.

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

 $Id$
 $Revision$

*/

#ifndef _VTSS_MPLS_STATE_H_
#define _VTSS_MPLS_STATE_H_

#if defined (VTSS_FEATURE_MPLS)

// VProfile table

#define VTSS_MPLS_VPROFILE_CNT          64
#define VTSS_MPLS_VPROFILE_LSR_IDX      (VTSS_PORT_COUNT + 1)               /* +1 due to CPU port */
#define VTSS_MPLS_VPROFILE_OAM_IDX      (VTSS_PORT_COUNT + 2)
#define VTSS_MPLS_VPROFILE_RESERVED_CNT (VTSS_MPLS_VPROFILE_OAM_IDX + 1)    /* Reserved ports start at index 0 */

typedef i8 vtss_mpls_vprofile_idx_t;

typedef struct {
    vtss_port_no_t  port;
    BOOL            s1_s2_enable;
    BOOL            learn_enable;
    BOOL            recv_enable;
    BOOL            ptp_dly1_enable;
    BOOL            vlan_aware;
    BOOL            map_dscp_cos_enable;
    BOOL            map_eth_cos_enable;
    BOOL            src_mirror_enable;
} vtss_mpls_vprofile_t;

typedef struct {
    vtss_mpls_vprofile_t        pub;
    vtss_mpls_vprofile_idx_t    next_free;      /* 0 == no more free */
} vtss_mpls_vprofile_internal_t;

#define VTSS_MPLS_L2_CNT                128     /* Number of L2 LER/LSR peer/VLAN combos */

typedef struct {
    vtss_mpls_l2_t          pub;
    // Internal data:
    vtss_mpls_idxchain_t    users;              /* Chain of segments using this L2, or VTSS_MPLS_IDXCHAIN_UNDEFINED for none */
    i16                     ll_idx_upstream;    /* VTSS_MPLS_IDX_UNDEFINED == no IS0 MLL entry allocated */
    i16                     ll_idx_downstream;  /* VTSS_MPLS_IDX_UNDEFINED == no IS0 MLL entry allocated */
    vtss_mpls_l2_idx_t      next_free;          /* VTSS_MPLS_IDX_UNDEFINED == no more free in list */
} vtss_mpls_l2_internal_t;

// MPLS segments and cross-connects

#define VTSS_MPLS_LSP_CNT               512     /* Uni-directional LSPs */
#define VTSS_MPLS_BIDIR_PW_CNT          (VTSS_MPLS_VPROFILE_CNT - VTSS_MPLS_VPROFILE_RESERVED_CNT)     /* Bi-directional PWs */
#define VTSS_MPLS_SEGMENT_CNT           (2*VTSS_MPLS_LSP_CNT + 2*2*VTSS_MPLS_BIDIR_PW_CNT)
#define VTSS_MPLS_XC_CNT                (VTSS_MPLS_LSP_CNT + 2*VTSS_MPLS_BIDIR_PW_CNT)

typedef struct {
    vtss_mpls_segment_t     pub;                /* Public API struct */
    // Internal data:
    union {
        struct {
            BOOL                     has_mll;
            BOOL                     has_mlbs;     /* TRUE == IS0 MLBS entry allocated */
            vtss_mpls_vprofile_idx_t vprofile_idx; /* VProfile idx for LER (only) */
        } in;
        struct {
            BOOL                    has_encap;
            BOOL                    has_es0;
            vtss_sdx_entry_t        *esdx;
            i16                     encap_idx;
            u16                     encap_bytes;  /* Length of encap in bytes */
        } out;
    } u;
    BOOL                    need_hw_alloc;      /* TRUE if segment needs to try and allocate HW */
    vtss_mpls_idxchain_t    clients;            /* Chain of segments using this segment as server */
    vtss_mpls_segment_idx_t next_free;          /* VTSS_MPLS_IDX_UNDEFINED == no more free in list */
} vtss_mpls_segment_internal_t;

typedef struct {
    vtss_mpls_xc_t          pub;
    // Internal data:
    vtss_sdx_entry_t        *isdx;              /* Service index */
    vtss_mpls_idxchain_t    mc_chain;           /* Multicast chain: List of segments */
    vtss_mpls_xc_idx_t      next_free;          /* VTSS_MPLS_IDX_UNDEFINED == no more free in list */
} vtss_mpls_xc_internal_t;

typedef struct {
    BOOL    b_domain;                   /* TRUE == B-domain port */
    u16     l2_refcnt;                  /* Count of number of MPLS L2 entries using this port */
} vtss_mpls_port_state_t;

/* We estimate the following number of index chain entries is needed:
 *   * each MPLS segment may be part of a multicast chain
 *   * each MPLS segment may point to a L2 entry
 *   * (almost) each MPLS segment may use a server segment
 *   * one per IS0 MLL key (free-list)
 *   * one per egress MPLS encapsulation (free-list)
 *   + 512 spare entries
 */
#define VTSS_MPLS_IDXCHAIN_ENTRY_CNT    (VTSS_MPLS_SEGMENT_CNT + \
                                         VTSS_MPLS_SEGMENT_CNT + \
                                         VTSS_MPLS_SEGMENT_CNT + \
                                         VTSS_MPLS_IN_ENCAP_CNT + \
                                         VTSS_MPLS_OUT_ENCAP_CNT + \
                                         512)

#define VTSS_MPLS_IDX_CHECK(type, idx, low, cnt) \
    do { \
        if ((idx) < (low)  ||  (idx) >= (cnt)) { \
            VTSS_DG(VTSS_TRACE_GROUP_MPLS, "Invalid " type " index: %d", (idx)); \
            return VTSS_RC_ERROR; \
        } \
    } while (0)

#define VTSS_MPLS_IDX_CHECK_L2(idx)        VTSS_MPLS_IDX_CHECK("L2",       idx, 0, VTSS_MPLS_L2_CNT)
#define VTSS_MPLS_IDX_CHECK_XC(idx)        VTSS_MPLS_IDX_CHECK("XC",       idx, 0, VTSS_MPLS_XC_CNT)
#define VTSS_MPLS_IDX_CHECK_SEGMENT(idx)   VTSS_MPLS_IDX_CHECK("Segment",  idx, 0, VTSS_MPLS_SEGMENT_CNT)
#define VTSS_MPLS_IDX_CHECK_VPROFILE(idx)  VTSS_MPLS_IDX_CHECK("VProfile", idx, VTSS_MPLS_VPROFILE_RESERVED_CNT, VTSS_MPLS_VPROFILE_CNT)


typedef struct {
    /* CIL function pointers */
    vtss_rc (* tc_conf_set)(struct vtss_state_s *vtss_state,
                            const vtss_mpls_tc_conf_t * const map);

    vtss_rc (* l2_alloc)(struct vtss_state_s *vtss_state,
                         vtss_mpls_l2_idx_t * const idx);
    vtss_rc (* l2_free)(struct vtss_state_s *vtss_state,
                        const vtss_mpls_l2_idx_t idx);
    vtss_rc (* l2_set)(struct vtss_state_s *vtss_state,
                       const vtss_mpls_l2_idx_t idx, const vtss_mpls_l2_t * const l2);
    vtss_rc (* l2_segment_attach)(struct vtss_state_s *vtss_state,
                                  const vtss_mpls_l2_idx_t idx, const vtss_mpls_segment_idx_t seg_idx);
    vtss_rc (* l2_segment_detach)(struct vtss_state_s *vtss_state,
                                  const vtss_mpls_segment_idx_t seg_idx);

    vtss_rc (* xc_alloc)(struct vtss_state_s *vtss_state,
                         const vtss_mpls_xc_type_t type, vtss_mpls_xc_idx_t * const idx);
    vtss_rc (* xc_free)(struct vtss_state_s *vtss_state,
                        const vtss_mpls_xc_idx_t idx);
    vtss_rc (* xc_set)(struct vtss_state_s *vtss_state,
                       const vtss_mpls_xc_idx_t idx, const vtss_mpls_xc_t * const xc);
    
    vtss_rc (* xc_segment_attach)(struct vtss_state_s *vtss_state,
                                  const vtss_mpls_xc_idx_t xc_idx, const vtss_mpls_segment_idx_t seg_idx);
    vtss_rc (* xc_segment_detach)(struct vtss_state_s *vtss_state,
                                  const vtss_mpls_segment_idx_t seg_idx);
    
    vtss_rc (* xc_mc_segment_attach)(struct vtss_state_s *vtss_state,
                                     const vtss_mpls_xc_idx_t xc_idx, const vtss_mpls_segment_idx_t seg_idx);
    vtss_rc (* xc_mc_segment_detach)(struct vtss_state_s *vtss_state,
                                     const vtss_mpls_xc_idx_t xc_idx, const vtss_mpls_segment_idx_t seg_idx);
    
    vtss_rc (* segment_alloc)(struct vtss_state_s *vtss_state,
                              const BOOL in_seg, vtss_mpls_segment_idx_t * const idx);
    vtss_rc (* segment_free)(struct vtss_state_s *vtss_state,
                             const vtss_mpls_segment_idx_t idx);
    vtss_rc (* segment_set)(struct vtss_state_s *vtss_state,
                            const vtss_mpls_segment_idx_t idx, const vtss_mpls_segment_t * const seg);
    vtss_rc (* segment_state_get)(struct vtss_state_s *vtss_state,
                                  const vtss_mpls_segment_idx_t idx, vtss_mpls_segment_state_t * const state);
    vtss_rc (* segment_server_attach)(struct vtss_state_s *vtss_state,
                                      const vtss_mpls_segment_idx_t idx, const vtss_mpls_segment_idx_t srv_idx);
    vtss_rc (* segment_server_detach)(struct vtss_state_s *vtss_state,
                                      const vtss_mpls_segment_idx_t idx);

    /* Configuration/state */
    vtss_mpls_idxchain_entry_t    idxchain[VTSS_MPLS_IDXCHAIN_ENTRY_CNT];        /**< [0] is reserved for free-chain */

    vtss_mpls_vprofile_internal_t vprofile_conf[VTSS_MPLS_VPROFILE_CNT];
    vtss_mpls_l2_internal_t       l2_conf[VTSS_MPLS_L2_CNT];
    vtss_mpls_segment_internal_t  segment_conf[VTSS_MPLS_SEGMENT_CNT];
    vtss_mpls_xc_internal_t       xc_conf[VTSS_MPLS_XC_CNT];

    vtss_mpls_port_state_t        port_state[VTSS_PORT_ARRAY_SIZE];

    vtss_mpls_tc_conf_t           tc_conf;

    vtss_mpls_vprofile_idx_t      vprofile_free_list;
    vtss_mpls_l2_idx_t            l2_free_list;
    vtss_mpls_segment_idx_t       segment_free_list;
    vtss_mpls_xc_idx_t            xc_free_list;

    vtss_mpls_idxchain_t          is0_mll_free_chain;
    vtss_mpls_idxchain_t          encap_free_chain;
    
    // Counts of used HW entries of various types
    u32                           is0_mll_cnt;
    u32                           is0_mlbs_cnt;
    u32                           encap_cnt;
    u32                           vprofile_cnt;
} vtss_mpls_state_t;

vtss_rc vtss_mpls_inst_create(struct vtss_state_s *vtss_state);

void vtss_mpls_debug_print(struct vtss_state_s *vtss_state,
                           const vtss_debug_printf_t pr,
                           const vtss_debug_info_t   *const info);

#endif /* VTSS_FEATURE_MPLS */

#endif /* _VTSS_MPLS_STATE_H_ */
