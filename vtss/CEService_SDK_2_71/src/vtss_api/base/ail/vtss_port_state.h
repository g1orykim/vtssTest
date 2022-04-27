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

#ifndef _VTSS_PORT_STATE_H_
#define _VTSS_PORT_STATE_H_

#if defined(VTSS_FEATURE_PORT_CONTROL)
typedef struct {
    u64 value; /**< Accumulated value (64 bit) */
    u64 prev;  /**< Previous value read (32 or 40 bit) */
} vtss_chip_counter_t;

typedef struct {
    /* Rx counters */
    vtss_chip_counter_t rx_octets;
    vtss_chip_counter_t rx_drops;
    vtss_chip_counter_t rx_packets;
    vtss_chip_counter_t rx_broadcasts;
    vtss_chip_counter_t rx_multicasts;
    vtss_chip_counter_t rx_crc_align_errors;
    vtss_chip_counter_t rx_shorts;
    vtss_chip_counter_t rx_longs;
    vtss_chip_counter_t rx_fragments;
    vtss_chip_counter_t rx_jabbers;
    vtss_chip_counter_t rx_64;
    vtss_chip_counter_t rx_65_127;
    vtss_chip_counter_t rx_128_255;
    vtss_chip_counter_t rx_256_511;
    vtss_chip_counter_t rx_512_1023;
    vtss_chip_counter_t rx_1024_1526;
    vtss_chip_counter_t rx_1527_max;

    /* Tx counters */
    vtss_chip_counter_t tx_octets;
    vtss_chip_counter_t tx_drops;
    vtss_chip_counter_t tx_packets;
    vtss_chip_counter_t tx_broadcasts;
    vtss_chip_counter_t tx_multicasts;
    vtss_chip_counter_t tx_collisions;
    vtss_chip_counter_t tx_64;
    vtss_chip_counter_t tx_65_127;
    vtss_chip_counter_t tx_128_255;
    vtss_chip_counter_t tx_256_511;
    vtss_chip_counter_t tx_512_1023;
    vtss_chip_counter_t tx_1024_1526;
    vtss_chip_counter_t tx_1527_max;
    vtss_chip_counter_t tx_fifo_drops;
    vtss_chip_counter_t rx_pauses;
    vtss_chip_counter_t tx_pauses;
    vtss_chip_counter_t rx_classified_drops;
    vtss_chip_counter_t rx_class[VTSS_PRIOS];
    vtss_chip_counter_t tx_class[VTSS_PRIOS];
    vtss_chip_counter_t rx_local_drops;
    vtss_chip_counter_t rx_unicast;
    vtss_chip_counter_t tx_unicast;
    vtss_chip_counter_t tx_aging;
} vtss_port_luton28_counters_t;

typedef struct {
    /* Rx counters */
    vtss_chip_counter_t rx_octets;
    vtss_chip_counter_t rx_broadcast;
    vtss_chip_counter_t rx_multicast;
    vtss_chip_counter_t rx_unicast;
    vtss_chip_counter_t rx_shorts;
    vtss_chip_counter_t rx_fragments;
    vtss_chip_counter_t rx_jabbers;
    vtss_chip_counter_t rx_crc_align_errors;
    vtss_chip_counter_t rx_symbol_errors;
    vtss_chip_counter_t rx_64;
    vtss_chip_counter_t rx_65_127;
    vtss_chip_counter_t rx_128_255;
    vtss_chip_counter_t rx_256_511;
    vtss_chip_counter_t rx_512_1023;
    vtss_chip_counter_t rx_1024_1526;
    vtss_chip_counter_t rx_1527_max;
    vtss_chip_counter_t rx_pause;
    vtss_chip_counter_t rx_control;
    vtss_chip_counter_t rx_longs;
    vtss_chip_counter_t rx_classified_drops;
    vtss_chip_counter_t rx_red_class[VTSS_PRIOS];
    vtss_chip_counter_t rx_yellow_class[VTSS_PRIOS];
    vtss_chip_counter_t rx_green_class[VTSS_PRIOS];
    /* Drop counters */
    vtss_chip_counter_t dr_local;
    vtss_chip_counter_t dr_tail;
    vtss_chip_counter_t dr_yellow_class[VTSS_PRIOS];
    vtss_chip_counter_t dr_green_class[VTSS_PRIOS];
    /* Tx counters */
    vtss_chip_counter_t tx_octets;
    vtss_chip_counter_t tx_unicast;
    vtss_chip_counter_t tx_multicast;
    vtss_chip_counter_t tx_broadcast;
    vtss_chip_counter_t tx_collision;
    vtss_chip_counter_t tx_drops;
    vtss_chip_counter_t tx_pause;
    vtss_chip_counter_t tx_64;
    vtss_chip_counter_t tx_65_127;
    vtss_chip_counter_t tx_128_255;
    vtss_chip_counter_t tx_256_511;
    vtss_chip_counter_t tx_512_1023;
    vtss_chip_counter_t tx_1024_1526;
    vtss_chip_counter_t tx_1527_max;
    vtss_chip_counter_t tx_yellow_class[VTSS_PRIOS];
    vtss_chip_counter_t tx_green_class[VTSS_PRIOS];
    vtss_chip_counter_t tx_aging;
} vtss_port_luton26_counters_t;

typedef struct {
    /* Rx counters */
    vtss_chip_counter_t rx_in_bytes;
    vtss_chip_counter_t rx_symbol_carrier_err;
    vtss_chip_counter_t rx_pause;
    vtss_chip_counter_t rx_unsup_opcode;
    vtss_chip_counter_t rx_ok_bytes;
    vtss_chip_counter_t rx_bad_bytes;
    vtss_chip_counter_t rx_unicast;
    vtss_chip_counter_t rx_multicast;
    vtss_chip_counter_t rx_broadcast;
    vtss_chip_counter_t rx_crc_err;
    vtss_chip_counter_t rx_undersize;
    vtss_chip_counter_t rx_fragments;
    vtss_chip_counter_t rx_in_range_length_err;
    vtss_chip_counter_t rx_out_of_range_length_err;
    vtss_chip_counter_t rx_oversize;
    vtss_chip_counter_t rx_jabbers;
    vtss_chip_counter_t rx_size64;
    vtss_chip_counter_t rx_size65_127;
    vtss_chip_counter_t rx_size128_255;
    vtss_chip_counter_t rx_size256_511;
    vtss_chip_counter_t rx_size512_1023;
    vtss_chip_counter_t rx_size1024_1518;
    vtss_chip_counter_t rx_size1519_max;
    vtss_chip_counter_t rx_filter_drops;
#if defined(VTSS_FEATURE_QOS)
    vtss_chip_counter_t rx_policer_drops[VTSS_PORT_POLICERS];
#endif /* VTSS_FEATURE_QOS */
    vtss_chip_counter_t rx_class[VTSS_PRIOS];
    vtss_chip_counter_t rx_queue_drops[VTSS_PRIOS];
    vtss_chip_counter_t rx_red_drops[VTSS_PRIOS];
    vtss_chip_counter_t rx_error_drops[VTSS_PRIOS];

    /* Tx counters */
    vtss_chip_counter_t tx_out_bytes;
    vtss_chip_counter_t tx_pause;
    vtss_chip_counter_t tx_ok_bytes;
    vtss_chip_counter_t tx_unicast;
    vtss_chip_counter_t tx_multicast;
    vtss_chip_counter_t tx_broadcast;
    vtss_chip_counter_t tx_size64;
    vtss_chip_counter_t tx_size65_127;
    vtss_chip_counter_t tx_size128_255;
    vtss_chip_counter_t tx_size256_511;
    vtss_chip_counter_t tx_size512_1023;
    vtss_chip_counter_t tx_size1024_1518;
    vtss_chip_counter_t tx_size1519_max;
    vtss_chip_counter_t tx_queue_drops;
    vtss_chip_counter_t tx_red_drops;
    vtss_chip_counter_t tx_error_drops;
    vtss_chip_counter_t tx_queue;
    vtss_chip_counter_t tx_queue_bytes;

    /* These fields are only relevant for D4, for D10 these fields remain 0 */
    vtss_chip_counter_t tx_multi_coll;
    vtss_chip_counter_t tx_late_coll;
    vtss_chip_counter_t tx_xcoll;
    vtss_chip_counter_t tx_defer;
    vtss_chip_counter_t tx_xdefer;
    vtss_chip_counter_t tx_csense;
    vtss_chip_counter_t tx_backoff1;
} vtss_port_b2_counters_t;

typedef struct {
    /* Rx counters */
    vtss_chip_counter_t rx_in_bytes;
    vtss_chip_counter_t rx_symbol_err;
    vtss_chip_counter_t rx_pause;
    vtss_chip_counter_t rx_unsup_opcode;
    vtss_chip_counter_t rx_ok_bytes;
    vtss_chip_counter_t rx_bad_bytes;
    vtss_chip_counter_t rx_unicast;
    vtss_chip_counter_t rx_multicast;
    vtss_chip_counter_t rx_broadcast;
    vtss_chip_counter_t rx_crc_err;
    vtss_chip_counter_t rx_undersize;
    vtss_chip_counter_t rx_fragments;
    vtss_chip_counter_t rx_in_range_len_err;
    vtss_chip_counter_t rx_out_of_range_len_err;
    vtss_chip_counter_t rx_oversize;
    vtss_chip_counter_t rx_jabbers;
    vtss_chip_counter_t rx_size64;
    vtss_chip_counter_t rx_size65_127;
    vtss_chip_counter_t rx_size128_255;
    vtss_chip_counter_t rx_size256_511;
    vtss_chip_counter_t rx_size512_1023;
    vtss_chip_counter_t rx_size1024_1518;
    vtss_chip_counter_t rx_size1519_max;
    vtss_chip_counter_t rx_local_drops;
    vtss_chip_counter_t rx_classified_drops;
    vtss_chip_counter_t rx_red_class[VTSS_PRIOS];
    vtss_chip_counter_t rx_yellow_class[VTSS_PRIOS];
    vtss_chip_counter_t rx_green_class[VTSS_PRIOS];
    vtss_chip_counter_t rx_policer_drops[VTSS_PRIOS];
    vtss_chip_counter_t rx_queue_drops[VTSS_PRIOS];
    vtss_chip_counter_t rx_txqueue_drops[VTSS_PRIOS];

    /* Tx counters */
    vtss_chip_counter_t tx_out_bytes;
    vtss_chip_counter_t tx_pause;
    vtss_chip_counter_t tx_ok_bytes;
    vtss_chip_counter_t tx_unicast;
    vtss_chip_counter_t tx_multicast;
    vtss_chip_counter_t tx_broadcast;
    vtss_chip_counter_t tx_size64;
    vtss_chip_counter_t tx_size65_127;
    vtss_chip_counter_t tx_size128_255;
    vtss_chip_counter_t tx_size256_511;
    vtss_chip_counter_t tx_size512_1023;
    vtss_chip_counter_t tx_size1024_1518;
    vtss_chip_counter_t tx_size1519_max;
    vtss_chip_counter_t tx_yellow_class[VTSS_PRIOS];
    vtss_chip_counter_t tx_green_class[VTSS_PRIOS];
    vtss_chip_counter_t tx_queue_drops[VTSS_PRIOS];
    vtss_chip_counter_t tx_multi_coll;
    vtss_chip_counter_t tx_late_coll;
    vtss_chip_counter_t tx_xcoll;
    vtss_chip_counter_t tx_defer;
    vtss_chip_counter_t tx_xdefer;
    vtss_chip_counter_t tx_csense;
    vtss_chip_counter_t tx_backoff1;
} vtss_port_jr1_counters_t;

typedef struct {
    // JR2-TBD: Counters. (We need one at least to quiet lint.)
    /* Rx counters */
    vtss_chip_counter_t rx_in_bytes;
//    vtss_chip_counter_t rx_symbol_err;
//    vtss_chip_counter_t rx_pause;
//    vtss_chip_counter_t rx_unsup_opcode;
//    vtss_chip_counter_t rx_ok_bytes;
//    vtss_chip_counter_t rx_bad_bytes;
//    vtss_chip_counter_t rx_unicast;
//    vtss_chip_counter_t rx_multicast;
//    vtss_chip_counter_t rx_broadcast;
//    vtss_chip_counter_t rx_crc_err;
//    vtss_chip_counter_t rx_undersize;
//    vtss_chip_counter_t rx_fragments;
//    vtss_chip_counter_t rx_in_range_len_err;
//    vtss_chip_counter_t rx_out_of_range_len_err;
//    vtss_chip_counter_t rx_oversize;
//    vtss_chip_counter_t rx_jabbers;
//    vtss_chip_counter_t rx_size64;
//    vtss_chip_counter_t rx_size65_127;
//    vtss_chip_counter_t rx_size128_255;
//    vtss_chip_counter_t rx_size256_511;
//    vtss_chip_counter_t rx_size512_1023;
//    vtss_chip_counter_t rx_size1024_1518;
//    vtss_chip_counter_t rx_size1519_max;
//    vtss_chip_counter_t rx_local_drops;
//    vtss_chip_counter_t rx_classified_drops;
//    vtss_chip_counter_t rx_red_class[VTSS_PRIOS];
//    vtss_chip_counter_t rx_yellow_class[VTSS_PRIOS];
//    vtss_chip_counter_t rx_green_class[VTSS_PRIOS];
//    vtss_chip_counter_t rx_policer_drops[VTSS_PRIOS];
//    vtss_chip_counter_t rx_queue_drops[VTSS_PRIOS];
//    vtss_chip_counter_t rx_txqueue_drops[VTSS_PRIOS];
//
//    /* Tx counters */
//    vtss_chip_counter_t tx_out_bytes;
//    vtss_chip_counter_t tx_pause;
//    vtss_chip_counter_t tx_ok_bytes;
//    vtss_chip_counter_t tx_unicast;
//    vtss_chip_counter_t tx_multicast;
//    vtss_chip_counter_t tx_broadcast;
//    vtss_chip_counter_t tx_size64;
//    vtss_chip_counter_t tx_size65_127;
//    vtss_chip_counter_t tx_size128_255;
//    vtss_chip_counter_t tx_size256_511;
//    vtss_chip_counter_t tx_size512_1023;
//    vtss_chip_counter_t tx_size1024_1518;
//    vtss_chip_counter_t tx_size1519_max;
//    vtss_chip_counter_t tx_yellow_class[VTSS_PRIOS];
//    vtss_chip_counter_t tx_green_class[VTSS_PRIOS];
//    vtss_chip_counter_t tx_queue_drops[VTSS_PRIOS];
//    vtss_chip_counter_t tx_multi_coll;
//    vtss_chip_counter_t tx_late_coll;
//    vtss_chip_counter_t tx_xcoll;
//    vtss_chip_counter_t tx_defer;
//    vtss_chip_counter_t tx_xdefer;
//    vtss_chip_counter_t tx_csense;
//    vtss_chip_counter_t tx_backoff1;
} vtss_port_jr2_counters_t;

#if defined(VTSS_FEATURE_MAC10G)
typedef struct {
    /* Rx counters */
    vtss_chip_counter_t rx_in_bytes;

    vtss_chip_counter_t rx_symbol_err;
    vtss_chip_counter_t rx_pause;
    vtss_chip_counter_t rx_unsup_opcode;
    vtss_chip_counter_t rx_ok_bytes;
    vtss_chip_counter_t rx_bad_bytes;
    vtss_chip_counter_t rx_unicast;
    vtss_chip_counter_t rx_multicast;
    vtss_chip_counter_t rx_broadcast;
    vtss_chip_counter_t rx_crc_err;
    vtss_chip_counter_t rx_undersize;
    vtss_chip_counter_t rx_fragments;
    vtss_chip_counter_t rx_in_range_len_err;
    vtss_chip_counter_t rx_out_of_range_len_err;
    vtss_chip_counter_t rx_oversize;
    vtss_chip_counter_t rx_jabbers;
    vtss_chip_counter_t rx_size64;
    vtss_chip_counter_t rx_size65_127;
    vtss_chip_counter_t rx_size128_255;
    vtss_chip_counter_t rx_size256_511;
    vtss_chip_counter_t rx_size512_1023;
    vtss_chip_counter_t rx_size1024_1518;
    vtss_chip_counter_t rx_size1519_max;

    /* Tx counters */
    vtss_chip_counter_t tx_out_bytes;

    vtss_chip_counter_t tx_pause; /* DBG */
    vtss_chip_counter_t tx_ok_bytes;
    vtss_chip_counter_t tx_unicast;
    vtss_chip_counter_t tx_multicast;
    vtss_chip_counter_t tx_broadcast;
    vtss_chip_counter_t tx_size64;
    vtss_chip_counter_t tx_size65_127;
    vtss_chip_counter_t tx_size128_255;
    vtss_chip_counter_t tx_size256_511;
    vtss_chip_counter_t tx_size512_1023;
    vtss_chip_counter_t tx_size1024_1518;
    vtss_chip_counter_t tx_size1519_max;
} vtss_port_mac10g_counters_t;
#endif /* VTSS_FEATURE_MAC10G */

typedef struct {
    union {
        vtss_port_luton28_counters_t luton28;
        vtss_port_luton26_counters_t luton26;
        vtss_port_b2_counters_t      b2;
        vtss_port_jr1_counters_t     jr1;
        vtss_port_jr2_counters_t     jr2;
#if defined(VTSS_FEATURE_MAC10G)
        vtss_port_mac10g_counters_t  mac10g;
#endif /* VTSS_FEATURE_MAC10G */
    } counter;
} vtss_port_chip_counters_t;

#if defined(VTSS_FEATURE_PCS_10GBASE_R)
/** \brief pcs counters */
typedef struct vtss_pcs_10gbase_r_cnt_s {
	vtss_chip_counter_t ber_count;                 /**< counts each time BER_BAD_SH state is entered */
    vtss_chip_counter_t	rx_errored_block_count;     /**< counts once for each time RX_E state is entered. */
    vtss_chip_counter_t	tx_errored_block_count;     /**< counts once for each time TX_E state is entered. */
    vtss_chip_counter_t	test_pattern_error_count;   /**< When the receiver is in test-pattern mode, the test_pattern_error_count counts
                                                              errors as described in 802.3: 49.2.12. */
} vtss_pcs_10gbase_r_chip_counters_t;
#endif /*VTSS_FEATURE_PCS_10GBASE_R */

typedef struct
{
    BOOL link;        /**< FALSE if link has been down since last status read */
    struct
    {
        BOOL                      complete;               /**< Completion status */
        vtss_port_clause_37_adv_t partner_advertisement;  /**< Clause 37 Advertisement control data */
        vtss_port_sgmii_aneg_t    partner_adv_sgmii;      /**< SGMII Advertisement control data */
    } autoneg;                                            /**< Autoneg status */
} vtss_port_clause_37_status_t;

#if defined(VTSS_ARCH_JAGUAR_1_CE_MAC)
typedef struct
{   /* Rx counters */
    vtss_chip_counter_t rx_yellow[VTSS_PRIOS]; /**< Rx yellow frames */
    vtss_chip_counter_t rx_green[VTSS_PRIOS];  /**< Rx green frames  */
    vtss_chip_counter_t rx_drops[VTSS_PRIOS];  /**< Rx drops         */
    /* Tx counters */
    vtss_chip_counter_t tx_yellow[VTSS_PRIOS]; /**< Tx yellow frames */
    vtss_chip_counter_t tx_green[VTSS_PRIOS];  /**< Tx green frames  */
} vtss_lport_chip_counters_t;
#endif


#if defined(VTSS_ARCH_B2)
/* Internal port numbers */
#define VTSS_INT_PORT_COUNT 96

/* Port forwarding state */
#define VTSS_PORT_RX_FORWARDING(fwd_state) (fwd_state == VTSS_PORT_FORWARD_ENABLED || fwd_state == VTSS_PORT_FORWARD_INGRESS ? 1 : 0)
#define VTSS_PORT_TX_FORWARDING(fwd_state) (fwd_state == VTSS_PORT_FORWARD_ENABLED || fwd_state == VTSS_PORT_FORWARD_EGRESS ? 1 : 0)

#endif /* VTSS_ARCH_B2 */

typedef struct {
    /* CIL function pointers */
    vtss_rc (* miim_read)(struct vtss_state_s *vtss_state,
                          vtss_miim_controller_t miim_controller,
                          u8 miim_addr,
                          u8 addr,
                          u16 *value,
                          BOOL report_errors);
    vtss_rc (* miim_write)(struct vtss_state_s *vtss_state,
                           vtss_miim_controller_t miim_controller,
                           u8 miim_addr,
                           u8 addr,
                           u16 value,
                           BOOL report_errors);
#if defined(VTSS_FEATURE_10G)
    vtss_rc (* mmd_read)(struct vtss_state_s *vtss_state,
                         vtss_miim_controller_t miim_controller, u8 miim_addr, u8 mmd,
                         u16 addr, u16 *value, BOOL report_errors);
    vtss_rc (* mmd_read_inc)(struct vtss_state_s *vtss_state,
                             vtss_miim_controller_t miim_controller, u8 miim_addr, u8 mmd,
                             u16 addr, u16 *buf, u8 count, BOOL report_errors);
    vtss_rc (* mmd_write)(struct vtss_state_s *vtss_state,
                          vtss_miim_controller_t miim_controller, u8 miim_addr, u8 mmd,
                          u16 addr, u16 data, BOOL report_errors);
#endif /* VTSS_FEATURE_10G */
    vtss_rc (* map_set)(struct vtss_state_s *vtss_state);
    vtss_rc (* conf_get)(struct vtss_state_s *vtss_state,
                         const vtss_port_no_t port_no,
                         vtss_port_conf_t *const conf);
    vtss_rc (* conf_set)(struct vtss_state_s *vtss_state, const vtss_port_no_t port_no);

    vtss_rc (* ifh_set)(struct vtss_state_s *vtss_state, const vtss_port_no_t port_no);

    vtss_rc (* clause_37_status_get)(struct vtss_state_s *vtss_state,
                                     const vtss_port_no_t port_no,
                                     vtss_port_clause_37_status_t *const status);
    vtss_rc (* clause_37_control_get)(struct vtss_state_s *vtss_state,
                                      const vtss_port_no_t           port_no,
                                      vtss_port_clause_37_control_t  *const control);
    vtss_rc (* clause_37_control_set)(struct vtss_state_s *vtss_state,
                                      const vtss_port_no_t port_no);
    vtss_rc (* status_get)(struct vtss_state_s *vtss_state,
                           const vtss_port_no_t  port_no,
                           vtss_port_status_t    *const status);
    vtss_rc (* counters_update)(struct vtss_state_s *vtss_state,
                                const vtss_port_no_t port_no);
    vtss_rc (* counters_clear)(struct vtss_state_s *vtss_state,
                               const vtss_port_no_t port_no);
    vtss_rc (* counters_get)(struct vtss_state_s *vtss_state,
                             const vtss_port_no_t port_no,
                             vtss_port_counters_t *const counters);
    vtss_rc (* basic_counters_get)(struct vtss_state_s *vtss_state,
                                   const vtss_port_no_t port_no,
                                   vtss_basic_counters_t *const counters);
    vtss_rc (* forward_set)(struct vtss_state_s *vtss_state,
                                 const vtss_port_no_t port_no);
#if defined(VTSS_ARCH_JAGUAR_1_CE_MAC) || defined(VTSS_ARCH_B2)
    vtss_rc (* host_conf_set)(struct vtss_state_s *vtss_state, vtss_port_no_t port_no);
#endif
#if defined(VTSS_ARCH_JAGUAR_1_CE_MAC)
    BOOL    (* is_host)(struct vtss_state_s *vtss_state,
                        const vtss_port_no_t port_no);
    vtss_rc (* lport_counters_get)(struct vtss_state_s *vtss_state,
                                   const vtss_lport_no_t lport_no,
                                   vtss_lport_counters_t *const counters);
    vtss_rc (* lport_counters_clear)(struct vtss_state_s *vtss_state,
                                     const vtss_lport_no_t lport_no);
#endif /* VTSS_ARCH_JAGUAR_1_CE_MAC */
#if defined(VTSS_ARCH_B2)
    vtss_rc (* status_interface_set)(struct vtss_state_s *vtss_state, const u32 clock);
    vtss_rc (* status_interface_get)(struct vtss_state_s *vtss_state, u32 *clock);
    vtss_rc (* port_filter_set)(struct vtss_state_s *vtss_state,
                                const vtss_port_no_t port_no,
                                const vtss_port_filter_t * const filter);
    vtss_rc (* vid2port_set)(struct vtss_state_s *vtss_state,
                             const vtss_vid_t     vid,
                             const vtss_port_no_t port_no);
    vtss_rc (* vid2lport_set)(struct vtss_state_s *vtss_state,
                              const vtss_vid_t     vid,
                              const vtss_lport_no_t port_no);
#endif /* VTSS_ARCH_B2 */

    /* Configuration/state */
    vtss_port_map_t               map[VTSS_PORT_ARRAY_SIZE];
    vtss_port_conf_t              conf[VTSS_PORT_ARRAY_SIZE];
    vtss_serdes_mode_t            serdes_mode[VTSS_PORT_ARRAY_SIZE];
    vtss_port_clause_37_control_t clause_37[VTSS_PORT_ARRAY_SIZE];
    vtss_port_chip_counters_t     counters[VTSS_PORT_ARRAY_SIZE];
    vtss_port_chip_counters_t     cpu_counters;
#if defined(VTSS_ARCH_SERVAL)
    vtss_port_ifh_t               ifh_conf[VTSS_PORT_ARRAY_SIZE];
#endif /* VTSS_ARCH_SERVAL */
#if VTSS_OPT_INT_AGGR
    vtss_port_chip_counters_t     port_int_counters[2]; /* Internal port counters */
#endif /* VTSS_OPT_INT_AGGR */
#if defined(VTSS_ARCH_LUTON28)
    u32                           tx_packets[VTSS_PORT_ARRAY_SIZE];
#endif /* VTSS_ARCH_LUTON28 */
    vtss_port_forward_t           forward[VTSS_PORT_ARRAY_SIZE];
#if defined(VTSS_ARCH_JAGUAR_1)
    u32                           port_int_0;              /* Internal chip port 0 */
    u32                           port_int_1;              /* Internal chip port 1 */
    u32                           mask_int_ports;          /* Internal chip port mask */
    vtss_port_mux_mode_t          mux_mode[VTSS_CHIP_CNT]; /* Port mux modes */
#endif /* VTSS_ARCH_JAGUAR_1 */

#if defined(VTSS_ARCH_B2) || defined(VTSS_ARCH_JAGUAR_1_CE_MAC)
    vtss_xaui_conf_t              xaui[4];    /* XAUI Line / Host setup */
#endif /* VTSS_ARCH_B2/JAGUAR_1_CE_MAC */
#if defined(VTSS_ARCH_B2)
    vtss_port_filter_t            port_filter[VTSS_PORT_ARRAY_SIZE];   /* Filter setup */
    int                           dep_port[VTSS_INT_PORT_COUNT];     /* Departure ports */
    vtss_lport_no_t               lport_map[VTSS_PORT_ARRAY_SIZE];
    vtss_spi4_conf_t              spi4;       /* SPI-4.2 host setup */
    BOOL                          miim_addr_err[256];
#endif /* VTSS_ARCH_B2 */
#if defined(VTSS_ARCH_JAGUAR_1_CE_MAC)
    vtss_lport_chip_counters_t    lport_chip_counters[VTSS_LPORTS]; /* Lport chip counters */
    i32                           ce_mac_hmda; /* Host interface A */
    i32                           ce_mac_hmdb; /* Host interface B */
#endif /* VTSS_ARCH_JAGUAR_1_CE_MAC */
    // JR2-TBD: Additional port state?
} vtss_port_state_t;

vtss_rc vtss_port_inst_create(struct vtss_state_s *vtss_state);
vtss_rc vtss_port_restart_sync(struct vtss_state_s *vtss_state);

vtss_port_no_t vtss_cmn_first_port_no_get(struct vtss_state_s *vtss_state,
                                          const BOOL port_list[VTSS_PORT_ARRAY_SIZE],
                                          BOOL port_cpu);
vtss_port_no_t vtss_cmn_port2port_no(struct vtss_state_s *vtss_state,
                                     const vtss_debug_info_t *const info, u32 port);
vtss_rc vtss_cmn_port_clause_37_adv_get(u32 value, vtss_port_clause_37_adv_t *adv);
vtss_rc vtss_cmn_port_clause_37_adv_set(u32 *value, vtss_port_clause_37_adv_t *adv, BOOL aneg_enable);

void vtss_port_debug_print(struct vtss_state_s *vtss_state, 
                           const vtss_debug_printf_t pr,
                           const vtss_debug_info_t   *const info);

#endif /* VTSS_FEATURE_PORT_CONTROL */

#endif /* _VTSS_PORT_STATE_H_ */
