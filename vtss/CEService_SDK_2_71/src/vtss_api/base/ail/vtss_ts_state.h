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

#ifndef _VTSS_TS_STATE_H_
#define _VTSS_TS_STATE_H_

#if defined(VTSS_FEATURE_TIMESTAMP)

#if defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_SERVAL)
#define VTSS_TS_ID_SIZE  63
#define TS_IDS_RESERVED_FOR_SW 4
#endif /* VTSS_ARCH_LUTON26 || VTSS_ARCH_SERVAL */

#if defined (VTSS_ARCH_SERVAL)
#define VTSS_VOE_ID_SIZE  VTSS_OAM_VOE_CNT
#endif /* VTSS_ARCH_SERVAL */

#if defined(VTSS_ARCH_JAGUAR_1)
#define VTSS_TS_ID_SIZE  3
#define TS_IDS_RESERVED_FOR_SW 3
#endif /* VTSS_ARCH_JAGUAR_1 */

typedef struct {
    i32 adj;
    i32 adj_divisor;
    vtss_ts_ext_clock_mode_t ext_clock_mode;
#if defined(VTSS_ARCH_SERVAL)
    vtss_ts_ext_clock_mode_t ext_clock_mode_alt;
    vtss_ts_alt_clock_mode_t alt_clock_mode;

#endif
    u32 sec_offset; /* current offset from HW timer */
    u32 outstanding_adjustment; /* set in timeofday_set, cleared in timeofday_onesec */
    i32 outstanding_corr;       /* value to be subtracted from hw time to get PTP time during the adjustment period */
    BOOL awaiting_adjustment; /* set when a clock onetime adjustment has been set, cleared after one sec */
    u32 delta_sec;              /* outstanding delta time */
    u16 delta_sec_msb;          /* outstanding delta time */
    u16 delta_sign;             /* 0 if no outstanding delta, 1 if neg, 2 if pos */
    BOOL sw_ts_id_res [TS_IDS_RESERVED_FOR_SW]; /* reservation flags for Luton 26 timestamps */
} vtss_ts_conf_t;

typedef struct {
    vtss_timeinterval_t ingress_latency;
    vtss_timeinterval_t p2p_delay;
    vtss_timeinterval_t egress_latency;
    vtss_timeinterval_t delay_asymmetry;
    vtss_ts_operation_mode_t mode;
    i32 default_igr_latency;    /* default ingress latency in ns, for automatic compensation */
    i32 default_egr_latency;    /* default egress latency in ns, for automatic compensation */
} vtss_ts_port_conf_t;

/* Serval Luton 26 and Jaguar timestamp table structure */
typedef struct {
    u64 reserved_mask;                      /* port mask indicating which ports this tx idx is reserved for */
    u64 valid_mask;                         /* indication pr. port if there is a valid timestamp in the table  */
    u32 age;                                /* ageing counter */
    u32 tx_tc [VTSS_PORT_ARRAY_SIZE];       /* actual transmit time counter for the [idx][port] */
    u32 tx_id [VTSS_PORT_ARRAY_SIZE];       /* actual transmit time stamp id read from HW [idx][port] */
    u32 tx_sq [VTSS_PORT_ARRAY_SIZE];       /* actual transmit sequence number read from HW [idx][port] (Serval)*/
    u32 rx_tc;                              /* actual receive time counter for the [id] (Luton26) */
    BOOL rx_tc_valid;                       /* actual receive time counter is valid for the [id] (Luton26) */
    void * context [VTSS_PORT_ARRAY_SIZE];  /* context aligned to the  [idx,port] */
    void (*cb  [VTSS_PORT_ARRAY_SIZE]) (void *context, u32 port_no, vtss_ts_timestamp_t *ts); /* timestamp callback functions */
} vtss_ts_timestamp_status_t;

#if defined (VTSS_ARCH_SERVAL)
/* Serval OAM timestamp table structure
 * When an OAM timestamp is registered in HW, it is saved in this table
 * For each VOE instance there is place for up to VTSS_SERVAL_MAX_OAM_ENTRIES timestamps, the sequence number is used
 * to distinguish the timestamps,
 * A ringbuffer mechanism is used to fill entries into the buffer.
 * If the buffer runs over, the oldest timestamps are thrown away.
 *
 */
typedef struct {
    u32 tc ;      /* actual time counter  read from HW */
    u32 id;       /* actual time stamp id (VOE index) read from HW*/
    u32 sq;       /* actual sequence number corrsponding to the timestamp */
    u32 port;     /* actual port number corrsponding to the timestamp */
    BOOL valid;   /* true indicates that the timestamp is valid */
} vtss_oam_timestamp_entry_t;

#define VTSS_SERVAL_MAX_OAM_ENTRIES 5

typedef struct {
    vtss_oam_timestamp_entry_t entry [VTSS_SERVAL_MAX_OAM_ENTRIES];
    i32 last;
} vtss_oam_timestamp_status_t;
#endif /* VTSS_ARCH_SERVAL */

typedef struct {
    /* CIL function pointers */
#if defined(VTSS_FEATURE_TIMESTAMP)
    vtss_rc (* timeofday_get)(struct vtss_state_s *vtss_state,
                              vtss_timestamp_t               *ts,
                              u32                            *tc);
    vtss_rc (* timeofday_next_pps_get)(struct vtss_state_s *vtss_state,
                                       vtss_timestamp_t      *ts);
    vtss_rc (* timeofday_set)(struct vtss_state_s *vtss_state,
                              const vtss_timestamp_t         *ts);
    vtss_rc (* timeofday_set_delta)(struct vtss_state_s *vtss_state,
                                    const vtss_timestamp_t   *ts,
                                    BOOL                 negative);
    vtss_rc (* timeofday_offset_set)(struct vtss_state_s *vtss_state,
                                     i32                     offset);
    u32     (* ns_cnt_get) (vtss_inst_t inst);
    vtss_rc (* timeofday_one_sec)(struct vtss_state_s *vtss_state);
    vtss_rc (* adjtimer_set)(struct vtss_state_s *vtss_state);
    vtss_rc (* freq_offset_get)(struct vtss_state_s *vtss_state, i32 *adj);
#if defined (VTSS_ARCH_SERVAL)
    vtss_rc (* external_clock_mode_set)(struct vtss_state_s *vtss_state, int idx);
    vtss_rc (* alt_clock_saved_get)(struct vtss_state_s *vtss_state, u32 *saved);
    vtss_rc (* alt_clock_mode_set)(struct vtss_state_s *vtss_state);
    vtss_rc (* timeofday_next_pps_set)(struct vtss_state_s *vtss_state,
                                       const vtss_timestamp_t *ts);
#else
    vtss_rc (* external_clock_mode_set)(struct vtss_state_s *vtss_state);
#endif
    vtss_rc (* ingress_latency_set)(struct vtss_state_s *vtss_state,
                                    vtss_port_no_t    port_no);
    vtss_rc (* p2p_delay_set)(struct vtss_state_s *vtss_state,
                              vtss_port_no_t          port_no);
    vtss_rc (* egress_latency_set)(struct vtss_state_s *vtss_state,
                                   vtss_port_no_t     port_no);
    vtss_rc (* delay_asymmetry_set)(struct vtss_state_s *vtss_state,
                                    vtss_port_no_t     port_no);
    vtss_rc (* operation_mode_set)(struct vtss_state_s *vtss_state,
                                   vtss_port_no_t     port_no);
    vtss_rc (* internal_mode_set) (struct vtss_state_s *vtss_state);
    vtss_rc (* timestamp_get)(struct vtss_state_s *vtss_state);
    vtss_rc (* timestamp_convert)(struct vtss_state_s *vtss_state,
                                  vtss_port_no_t      port_no,
                                  u32                 *ts);
    vtss_rc (* timestamp_id_release)(struct vtss_state_s *vtss_state,
                                     u32              ts_id);
    vtss_rc (* status_change)(struct vtss_state_s    *vtss_state,
                             vtss_port_no_t         port_no,
                             vtss_port_speed_t      speed);

#endif /* VTSS_FEATURE_TIMESTAMP */
    
    /* Configuration/state */
    vtss_ts_conf_t              conf;
    vtss_ts_internal_mode_t     int_mode;
    vtss_ts_port_conf_t         port_conf[VTSS_PORT_ARRAY_SIZE];
    vtss_ts_timestamp_status_t  status[VTSS_TS_ID_SIZE];
#if defined (VTSS_ARCH_SERVAL_CE)
    vtss_oam_timestamp_status_t oam_ts_status[VTSS_VOE_ID_SIZE];
#endif /* VTSS_ARCH_SERVAL_CE */
#if defined (VTSS_ARCH_SERVAL)
    BOOL                        add_sub_option;
#endif /* VTSS_ARCH_SERVAL */
} vtss_ts_state_t;

vtss_rc vtss_ts_inst_create(struct vtss_state_s *vtss_state);

void vtss_ts_debug_print(struct vtss_state_s *vtss_state,
                         const vtss_debug_printf_t pr,
                         const vtss_debug_info_t   *const info);

#endif  /* VTSS_FEATURE_TIMESTAMP */

#endif /* _VTSS_TS_STATE_H_ */
