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

#ifndef _VTSS_OAM_STATE_H_
#define _VTSS_OAM_STATE_H_

#if defined(VTSS_FEATURE_OAM)
typedef struct {
    vtss_chip_counter_t rx;
    vtss_chip_counter_t tx;
} vtss_oam_voe_internal_rx_tx_counter_t;

typedef struct {
    struct {
        vtss_chip_counter_t                   rx_valid_count;
        vtss_chip_counter_t                   rx_invalid_count;
        vtss_chip_counter_t                   rx_invalid_seq_no;
    } ccm;
    struct {
        vtss_oam_voe_internal_rx_tx_counter_t lm_count[VTSS_PRIO_ARRAY_SIZE];
    } lm;
    struct {
        vtss_chip_counter_t                   rx_lbr;
        vtss_chip_counter_t                   tx_lbm;
        vtss_chip_counter_t                   rx_lbr_trans_id_err;
    } lb;
    struct {
        vtss_chip_counter_t                   rx_tst;
        vtss_chip_counter_t                   tx_tst;
        vtss_chip_counter_t                   rx_tst_trans_id_err;
    } tst;
    struct {
        vtss_oam_voe_internal_rx_tx_counter_t selected_frames;
        vtss_oam_voe_internal_rx_tx_counter_t nonselected_frames;
    } sel;
} vtss_oam_voe_internal_counters_t;

typedef struct {
    BOOL                             allocated;      /* TRUE => VOE is allocated by API (vtss_oam_voe_alloc()); FALSE => free */
    vtss_oam_voe_internal_counters_t counters;       /* Chip counters for a VOE */
} vtss_oam_voe_internal_t;

typedef struct {
    /* CIL function pointers */
    vtss_rc (* vop_cfg_set)(struct vtss_state_s *vtss_state);

    vtss_rc (* event_poll)(struct vtss_state_s *vtss_state,
                           vtss_oam_event_mask_t *const mask);
    vtss_rc (* voe_event_enable)(struct vtss_state_s *vtss_state,
                                 const vtss_oam_voe_idx_t, const vtss_oam_voe_event_mask_t mask, const BOOL enable);
    vtss_rc (* voe_event_poll)(struct vtss_state_s *vtss_state,
                               const vtss_oam_voe_idx_t, vtss_oam_voe_event_mask_t *const mask);

    vtss_rc (* voe_alloc)(struct vtss_state_s *vtss_state,
                          const vtss_oam_voe_type_t type, const vtss_oam_voe_alloc_cfg_t *data, vtss_oam_voe_idx_t *voe_idx);
    vtss_rc (* voe_free)(struct vtss_state_s *vtss_state,
                         const vtss_oam_voe_idx_t);

    vtss_rc (* voe_cfg_set)(struct vtss_state_s *vtss_state,
                            const vtss_oam_voe_idx_t);

    vtss_rc (* voe_ccm_arm_hitme)(struct vtss_state_s *vtss_state,
                                  const vtss_oam_voe_idx_t, const BOOL enable);
    vtss_rc (* voe_ccm_set_rdi_flag)(struct vtss_state_s *vtss_state,
                                     const vtss_oam_voe_idx_t, const BOOL flag);

    vtss_rc (* ccm_status_get)(struct vtss_state_s *vtss_state,
                               const vtss_oam_voe_idx_t, vtss_oam_ccm_status_t *value);

    vtss_rc (* pdu_seen_status_get)(struct vtss_state_s *vtss_state,
                                    const vtss_oam_voe_idx_t, vtss_oam_pdu_seen_status_t *value);
    vtss_rc (* proc_status_get)(struct vtss_state_s *vtss_state,
                                const vtss_oam_voe_idx_t, vtss_oam_proc_status_t *value);

    vtss_rc (* voe_counter_update)(struct vtss_state_s *vtss_state,
                                   const vtss_oam_voe_idx_t);
    vtss_rc (* voe_counter_get)(struct vtss_state_s *vtss_state,
                                const vtss_oam_voe_idx_t, vtss_oam_voe_counter_t *value);
    vtss_rc (* voe_counter_clear)(struct vtss_state_s *vtss_state,
                                  const vtss_oam_voe_idx_t, const u32 mask);
#if defined(VTSS_ARCH_SERVAL)
    vtss_rc (* voe_counter_update_serval)(struct vtss_state_s *vtss_state,
                                          const vtss_oam_voe_idx_t);
#endif
    
    vtss_rc (* ipt_conf_get)(struct vtss_state_s *vtss_state,
                             u32 isdx, vtss_oam_ipt_conf_t *cfg);
    vtss_rc (* ipt_conf_set)(struct vtss_state_s *vtss_state,
                             u32 isdx, const vtss_oam_ipt_conf_t *const cfg);

    /* Configuration/state */
    vtss_oam_vop_conf_t     vop_conf;
    vtss_oam_voe_conf_t     voe_conf[VTSS_OAM_VOE_CNT];
    vtss_oam_voe_internal_t voe_internal[VTSS_OAM_VOE_CNT];
    vtss_oam_voe_idx_t      voe_poll_idx;
} vtss_oam_state_t;

vtss_rc vtss_oam_inst_create(struct vtss_state_s *vtss_state);
void vtss_oam_debug_print(struct vtss_state_s *vtss_state,
                          const vtss_debug_printf_t pr,
                          const vtss_debug_info_t   *const info);

#endif /* VTSS_FEATURE_OAM */

#endif /* _VTSS_OAM_STATE_H_ */
