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

#define VTSS_TRACE_GROUP VTSS_TRACE_GROUP_TS
#include "vtss_api.h"
#include "vtss_state.h"
#include "vtss_common.h"

#if defined(VTSS_FEATURE_TIMESTAMP)

/* Get the current time in a Timestamp format, and the corresponding time counter */
vtss_rc vtss_ts_timeofday_get(const vtss_inst_t             inst,
                              vtss_timestamp_t                     * const ts,
                              u32                           * const tc)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        rc = VTSS_FUNC(ts.timeofday_get,ts,tc);
    }
    VTSS_EXIT();
    return rc;
}

/* Get the time at the next 1PPS pulse edge in a Timestamp format. */
vtss_rc vtss_ts_timeofday_next_pps_get(const vtss_inst_t             inst,
                                       vtss_timestamp_t                     *const ts)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        rc = VTSS_FUNC(ts.timeofday_next_pps_get,ts);
    }
    VTSS_EXIT();
    return rc;
}

/* Set the current time in a Timestamp format */
vtss_rc vtss_ts_timeofday_set(const vtss_inst_t             inst,
                              const vtss_timestamp_t               *ts)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        rc = VTSS_FUNC_COLD(ts.timeofday_set,ts);
    }
    VTSS_EXIT();
    return rc;
}

/* Set delta from the current time in a Timestamp format */
vtss_rc vtss_ts_timeofday_set_delta(const vtss_inst_t       inst,
                              const vtss_timestamp_t         *ts,
                              BOOL                      negative)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        rc = VTSS_FUNC_COLD(ts.timeofday_set_delta,ts, negative);
    }
    VTSS_EXIT();
    return rc;
}

/* Subtract offset from the current time */
vtss_rc vtss_ts_timeofday_offset_set(const vtss_inst_t          inst,
                                     const i32                  offset)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        rc = VTSS_FUNC_COLD(ts.timeofday_offset_set,offset);
    }
    VTSS_EXIT();
    return rc;
}



/* Do the one sec administration in the Timestamp function */
vtss_rc vtss_ts_adjtimer_one_sec(const vtss_inst_t             inst,
                                 BOOL                           *const ongoing_adjustment)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        *ongoing_adjustment = vtss_state->ts.conf.awaiting_adjustment;
        rc = VTSS_FUNC_COLD_0(ts.timeofday_one_sec);
    }
    VTSS_EXIT();
    return rc;
}

/* check if clock adjustment is ongoing */
vtss_rc vtss_ts_ongoing_adjustment(const vtss_inst_t           inst,
                                   BOOL                        *const ongoing_adjustment)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        *ongoing_adjustment = vtss_state->ts.conf.awaiting_adjustment;
    }
    VTSS_EXIT();
    return rc;
}

/* Adjust the clock timer ratio */
vtss_rc vtss_ts_adjtimer_set(const vtss_inst_t              inst,
                             const i32                      adj)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        vtss_state->ts.conf.adj = adj;
        rc = VTSS_FUNC_COLD_0(ts.adjtimer_set);
    }
    VTSS_EXIT();
    return rc;
}

/* Get Adjust the clock timer ratio */
vtss_rc vtss_ts_adjtimer_get(const vtss_inst_t              inst,
                             i32                            *const adj)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        *adj = vtss_state->ts.conf.adj;
    }
    VTSS_EXIT();
    return rc;
}


/* Get get the clock internal timer frequency offset, compared to external clock input. */
vtss_rc vtss_ts_freq_offset_get(const vtss_inst_t              inst,
                                 i32                            *const adj)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        rc = VTSS_FUNC_COLD(ts.freq_offset_get, adj);
    }
    VTSS_EXIT();
    return rc;
}


/* Set the external clock mode. */
vtss_rc vtss_ts_external_clock_mode_set(
                                const vtss_inst_t           inst,
                                const vtss_ts_ext_clock_mode_t   *const ext_clock_mode)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        vtss_state->ts.conf.ext_clock_mode = *ext_clock_mode;
#if defined(VTSS_ARCH_SERVAL)
        rc = VTSS_FUNC_COLD(ts.external_clock_mode_set,1);
#else
        rc = VTSS_FUNC_COLD_0(ts.external_clock_mode_set);
#endif
    }
    VTSS_EXIT();
    return rc;
}

#if defined(VTSS_ARCH_SERVAL)
vtss_rc vtss_ts_alt_clock_saved_get(
    const vtss_inst_t           inst,
    u32    *const               saved)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        rc = VTSS_FUNC_COLD(ts.alt_clock_saved_get, saved);
    }
    VTSS_EXIT();
    return rc;
}
                                       
/* Get the alternative external clock mode. */
vtss_rc vtss_ts_alt_clock_mode_get(
    const vtss_inst_t              inst,
    vtss_ts_alt_clock_mode_t       *const alt_clock_mode)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        *alt_clock_mode = vtss_state->ts.conf.alt_clock_mode;
    }
    VTSS_EXIT();
    return rc;
}
                                       
/* Set the alternative external clock mode. */
vtss_rc vtss_ts_alt_clock_mode_set(
    const vtss_inst_t              inst,
    const vtss_ts_alt_clock_mode_t *const alt_clock_mode)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        vtss_state->ts.conf.alt_clock_mode = *alt_clock_mode;
        rc = VTSS_FUNC_COLD_0(ts.alt_clock_mode_set);
    }
    VTSS_EXIT();
    return rc;
}
                                       
/* Set the time at the next 1PPS pulse edge in a Timestamp format. */
vtss_rc vtss_ts_timeofday_next_pps_set(const vtss_inst_t       inst,
                                       const vtss_timestamp_t         *const ts)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        rc = VTSS_FUNC_COLD(ts.timeofday_next_pps_set, ts);
    }
    VTSS_EXIT();
    return rc;
}

#endif /* (VTSS_ARCH_SERVAL) */

/* Get the external clock mode. */
vtss_rc vtss_ts_external_clock_mode_get(
                                const vtss_inst_t           inst,
                                vtss_ts_ext_clock_mode_t         *const ext_clock_mode)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        *ext_clock_mode = vtss_state->ts.conf.ext_clock_mode;
    }
    VTSS_EXIT();
    return rc;
}

/* Set the ingress latency */
vtss_rc vtss_ts_ingress_latency_set(const vtss_inst_t              inst,
                                    const vtss_port_no_t           port_no,
                                    const vtss_timeinterval_t             *const ingress_latency)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        vtss_state->ts.port_conf[port_no].ingress_latency = *ingress_latency;
        rc = VTSS_FUNC_COLD(ts.ingress_latency_set,port_no);
    }
    VTSS_EXIT();
    return rc;
}

/* Get the ingress latency */
vtss_rc vtss_ts_ingress_latency_get(const vtss_inst_t              inst,
                                    const vtss_port_no_t           port_no,
                                    vtss_timeinterval_t                   *const ingress_latency)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        *ingress_latency = vtss_state->ts.port_conf[port_no].ingress_latency;
    }
    VTSS_EXIT();
    return rc;
}

/* Set the P2P delay */
vtss_rc vtss_ts_p2p_delay_set(const vtss_inst_t              inst,
                              const vtss_port_no_t           port_no,
                              const vtss_timeinterval_t             *const p2p_delay)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        vtss_state->ts.port_conf[port_no].p2p_delay = *p2p_delay;
        rc = VTSS_FUNC_COLD(ts.p2p_delay_set,port_no);
    }
    VTSS_EXIT();
    return rc;
}

/* Get the P2P delay */
vtss_rc vtss_ts_p2p_delay_get(const vtss_inst_t              inst,
                              const vtss_port_no_t           port_no,
                              vtss_timeinterval_t                   *const p2p_delay)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        *p2p_delay = vtss_state->ts.port_conf[port_no].p2p_delay;
    }
    VTSS_EXIT();
    return rc;
}

/* Set the egress latency */
vtss_rc vtss_ts_egress_latency_set(const vtss_inst_t              inst,
                                   const vtss_port_no_t           port_no,
                                   const vtss_timeinterval_t             *const egress_latency)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        vtss_state->ts.port_conf[port_no].egress_latency = *egress_latency;
        rc = VTSS_FUNC_COLD(ts.egress_latency_set,port_no);
    }
    VTSS_EXIT();
    return rc;
}

/* Get the egress latency */
vtss_rc vtss_ts_egress_latency_get(const vtss_inst_t        inst,
                                   const vtss_port_no_t     port_no,
                                   vtss_timeinterval_t             *const egress_latency)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        *egress_latency = vtss_state->ts.port_conf[port_no].egress_latency;
    }
    VTSS_EXIT();
    return rc;
}

/* Set the delay asymmetry */
vtss_rc vtss_ts_delay_asymmetry_set(const vtss_inst_t           inst,
                                   const vtss_port_no_t         port_no,
                                   const vtss_timeinterval_t    *const delay_asymmetry)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        vtss_state->ts.port_conf[port_no].delay_asymmetry = *delay_asymmetry;
        rc = VTSS_FUNC_COLD(ts.delay_asymmetry_set,port_no);
    }
    VTSS_EXIT();
    return rc;
}

/* Get the delay asymmetry */
vtss_rc vtss_ts_delay_asymmetry_get(const vtss_inst_t           inst,
                                   const vtss_port_no_t         port_no,
                                   vtss_timeinterval_t          *const delay_asymmetry)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        *delay_asymmetry = vtss_state->ts.port_conf[port_no].delay_asymmetry;
    }
    VTSS_EXIT();
    return rc;
}

/* Set the timestamping operation mode for a port */
vtss_rc vtss_ts_operation_mode_set(const vtss_inst_t              inst,
                                   const vtss_port_no_t           port_no,
                                   const vtss_ts_operation_mode_t      *const mode)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        vtss_state->ts.port_conf[port_no].mode = *mode;
        rc = VTSS_FUNC_COLD(ts.operation_mode_set,port_no);
    }
    VTSS_EXIT();
    return rc;
}

/* Get the timestamping operation mode for a port */
vtss_rc vtss_ts_operation_mode_get(const vtss_inst_t              inst,
                                   const vtss_port_no_t           port_no,
                                   vtss_ts_operation_mode_t            *const mode)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        *mode = vtss_state->ts.port_conf[port_no].mode;
    }
    VTSS_EXIT();
    return rc;
}

/* Set the timestamping internal mode */
vtss_rc vtss_ts_internal_mode_set(const vtss_inst_t              inst,
                                   const vtss_ts_internal_mode_t      *const mode)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        vtss_state->ts.int_mode = *mode;
        rc = VTSS_FUNC_COLD_0(ts.internal_mode_set);
    }
    VTSS_EXIT();
    return rc;
}

/* Get the timestamping internal mode */
vtss_rc vtss_ts_internal_mode_get(const vtss_inst_t              inst,
                                   vtss_ts_internal_mode_t            *const mode)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        *mode = vtss_state->ts.int_mode;
    }
    VTSS_EXIT();
    return rc;
}

/* Flush the timestamp FIFO  */
static void vtss_timestamp_flush(vtss_state_t *vtss_state)
{
    int     id;
    (void) VTSS_FUNC_0(ts.timestamp_get);
    VTSS_D("Flushing timestamp fifo");
    for (id = 0; id < VTSS_TS_ID_SIZE; id++) {
        vtss_state->ts.status[id].reserved_mask = 0LL;
        vtss_state->ts.status[id].valid_mask = 0LL;
        vtss_state->ts.status[id].rx_tc_valid = FALSE;
        vtss_state->ts.status[id].age = 0;
        (void) VTSS_FUNC(ts.timestamp_id_release, id);
    }
}
                                   

/* Update the internal timestamp table, from HW */
vtss_rc vtss_tx_timestamp_update(const vtss_inst_t              inst)
{
    vtss_state_t *vtss_state;
    vtss_rc rc;
    u64 port_mask;
    vtss_ts_timestamp_t ts;
    vtss_state_t *my_vtss_state;
    int port_idx = 0;
    int ts_idx = 0;
    void (*cb)(void *context, u32 port_no, vtss_ts_timestamp_t *ts);
    void *cx;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        rc = VTSS_FUNC_0(ts.timestamp_get);
        VTSS_D("rc = %d", rc);
        while (port_idx < VTSS_PORT_ARRAY_SIZE && 
               ts_idx < TS_IDS_RESERVED_FOR_SW) {
            port_mask = 1LL<<port_idx;
            if ((vtss_state->ts.status[ts_idx].valid_mask & port_mask) && 
                (vtss_state->ts.status[ts_idx].reserved_mask & port_mask)) {
                vtss_state->ts.status[ts_idx].valid_mask &= ~port_mask;
                vtss_state->ts.status[ts_idx].reserved_mask &= ~port_mask;
                ts.id = vtss_state->ts.status[ts_idx].tx_id[port_idx];
                ts.ts = vtss_state->ts.status[ts_idx].tx_tc[port_idx];
                ts.ts_valid = TRUE;
                if (vtss_state->ts.status[ts_idx].cb[port_idx] && vtss_state->ts.status[ts_idx].context[port_idx]) {
                    my_vtss_state = vtss_state; /* save context */
                    /* avoid using vtss_state while outside the API lock, as the API may be called from an other thread */
                    cb = vtss_state->ts.status[ts_idx].cb[port_idx];
                    cx = vtss_state->ts.status[ts_idx].context[port_idx];
                    VTSS_EXIT();
                    /* call out of the API */
                    cb(cx, port_idx, &ts);
                    VTSS_ENTER();
                    vtss_state = my_vtss_state; /* restore context */
                    vtss_state->ts.status[ts_idx].cb[port_idx] = NULL;
                    vtss_state->ts.status[ts_idx].context[port_idx] = NULL;
                } else {
                    VTSS_E("undefined TS callback port_idx %d, ts_idx %d", port_idx, ts_idx);
                }
                VTSS_D("port_no %d, ts_id %d, ts %d(%d)", port_idx, ts.id, ts.ts, ts.ts_valid);
            }
            if (++port_idx >= VTSS_PORT_ARRAY_SIZE) {
                port_idx = 0;
                ++ts_idx;
            }
        }
    }
    VTSS_EXIT();
    return rc;
}

/* Get the FIFO rx timestamp for a {timestampId} */
vtss_rc vtss_rx_timestamp_get(const vtss_inst_t              inst,
                              const vtss_ts_id_t             *const ts_id,
                              vtss_ts_timestamp_t                 *const ts)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        rc = VTSS_FUNC_0(ts.timestamp_get);
        if (ts_id->ts_id >= VTSS_TS_ID_SIZE) {
            /* invalid timestamp id indicates FIFO overflow */
            vtss_timestamp_flush(vtss_state);
            rc = VTSS_RC_ERROR;
        } else {
            ts->ts = vtss_state->ts.status[ts_id->ts_id].rx_tc;
            ts->ts_valid = vtss_state->ts.status[ts_id->ts_id].rx_tc_valid;
            if (ts->ts_valid) {
                vtss_state->ts.status[ts_id->ts_id].rx_tc_valid = FALSE;
                if (vtss_state->ts.status[ts_id->ts_id].reserved_mask == 0LL) {
                    vtss_state->ts.status[ts_id->ts_id].age = 0;
                    rc = VTSS_FUNC(ts.timestamp_id_release, ts_id->ts_id);
                }
            }
        }
    }
    VTSS_EXIT();
    return rc;
}

#if defined (VTSS_ARCH_SERVAL_CE)
/* brief Get oam timestamp */
vtss_rc vtss_oam_timestamp_get(const vtss_inst_t             inst,
                               const vtss_oam_ts_id_t        *const id,
                               vtss_oam_ts_timestamp_t       *const ts)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;
    i32          idx;
    u32          i;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        if (id->voe_id >= VTSS_VOE_ID_SIZE) {
            /* invalid timestamp id indicates FIFO overflow */
            rc = VTSS_RC_ERROR;
        } else {
            idx = vtss_state->ts.oam_ts_status [id->voe_id].last;
            for (i = 0; i < VTSS_SERVAL_MAX_OAM_ENTRIES; i++) {
                if (vtss_state->ts.oam_ts_status [id->voe_id].entry[idx].sq == id->voe_sq) {
                    /* an entry is found */
                    ts->ts = vtss_state->ts.oam_ts_status [id->voe_id].entry[idx].tc;
                    ts->port_no = vtss_state->ts.oam_ts_status [id->voe_id].entry[idx].port;
                    ts->ts_valid = vtss_state->ts.oam_ts_status [id->voe_id].entry[idx].valid;
                    vtss_state->ts.oam_ts_status[id->voe_id].entry[idx].valid = FALSE;
                    break;
                }
                if (--idx < 0) idx = VTSS_SERVAL_MAX_OAM_ENTRIES-1;
            }
            if (i == VTSS_SERVAL_MAX_OAM_ENTRIES) {
                /* no entries found */
                ts->ts = 0;
                ts->port_no = 0;
                ts->ts_valid = FALSE;
            }
        }
    }
    VTSS_EXIT();
    return rc;
}
#endif /* VTSS_ARCH_SERVAL_CE */



/* Release the FIFO rx timestamp id  */
vtss_rc vtss_rx_timestamp_id_release(const vtss_inst_t              inst,
                              const vtss_ts_id_t             *const ts_id)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        if (ts_id->ts_id >= VTSS_TS_ID_SIZE) {
            /* invalid timestamp id indicates FIFO overflow */
            vtss_timestamp_flush(vtss_state);
            rc = VTSS_RC_ERROR;
        } else {
            rc = VTSS_FUNC(ts.timestamp_id_release, ts_id->ts_id);
        }
    }
    VTSS_EXIT();
    return rc;
}

/* Get rx timestamp from a port (convert from slave time to the master time) */
vtss_rc vtss_rx_master_timestamp_get(const vtss_inst_t              inst,
                                     const vtss_port_no_t           port_no,
                                     vtss_ts_timestamp_t                 *const ts)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        rc = VTSS_FUNC(ts.timestamp_convert, port_no, &ts->ts);
        if (rc == VTSS_RC_OK) {
            ts->ts_valid = TRUE;
        } else {
            ts->ts_valid = TRUE;
        }

    }
    VTSS_EXIT();
    return rc;
}


/* Allocate a timestamp id for a two step transmission */
vtss_rc vtss_tx_timestamp_idx_alloc(const vtss_inst_t          inst,
                                   const vtss_ts_timestamp_alloc_t *const alloc_parm,
                                   vtss_ts_id_t               *const ts_id)
{
    vtss_state_t *vtss_state;
    int port_idx = 0;
    vtss_rc rc = VTSS_RC_ERROR;
    u32 id;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        rc = VTSS_RC_ERROR;
        /* Find a free ts_id */
        for (id = 0; id < TS_IDS_RESERVED_FOR_SW; id++) {
            if ((vtss_state->ts.status[id].reserved_mask & alloc_parm->port_mask) == 0) {
                vtss_state->ts.status[id].reserved_mask |= alloc_parm->port_mask;
                for (port_idx = 0; port_idx < VTSS_PORT_ARRAY_SIZE; port_idx++) {
                    if (alloc_parm->port_mask & (1LL<<port_idx)) {
                        vtss_state->ts.status[id].context[port_idx] = alloc_parm->context;
                        vtss_state->ts.status[id].cb[port_idx] = alloc_parm->cb;
                    }
                }
                vtss_state->ts.status[id].age = 0;
                ts_id->ts_id = id;
                VTSS_D("portmask = %" PRIx64 ", reserved_mask = %" PRIx64 " id = %u", alloc_parm->port_mask, vtss_state->ts.status[id].reserved_mask, ts_id->ts_id);
                rc = VTSS_RC_OK;
                break;
            }
        } 
    }
    VTSS_EXIT();
    return rc;
}


#define TSID_TX_MAX_TIMETICKS 3
#define TSID_RX_MAX_TIMETICKS 100
/* Age the FIFO timestamps */
vtss_rc vtss_timestamp_age(const vtss_inst_t              inst)
{
    vtss_state_t *vtss_state;
    int id;
    u64 port_mask;
    vtss_ts_timestamp_t ts;
    vtss_state_t *my_vtss_state;
    int port_idx = 0;
    vtss_rc rc = VTSS_RC_OK;
    void (*cb)(void *context, u32 port_no, vtss_ts_timestamp_t *ts);
    void *cx;

#if defined(VTSS_ARCH_LUTON26)
     /* LUTON26 does not generate tx timestamp interrupt, therefore it is first checked if there are any valid timestamps */
    rc = vtss_tx_timestamp_update(inst);
#endif
    if (rc == VTSS_RC_OK) {
        VTSS_ENTER();
        if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
            for (id = 0; id < VTSS_TS_ID_SIZE; id++) {
                if (vtss_state->ts.status[id].reserved_mask != 0LL || vtss_state->ts.status[id].rx_tc_valid) {
                    u32 max_age;
                    if (vtss_state->ts.status[id].reserved_mask != 0LL) {
                        max_age = TSID_TX_MAX_TIMETICKS;
                    } else {
                        max_age = TSID_RX_MAX_TIMETICKS;
                    }
                    if (++vtss_state->ts.status[id].age > max_age) {
                        VTSS_D("ageing timestamp ts_id = %d, reserved_mask = %" PRIx64 "", id, vtss_state->ts.status[id].reserved_mask);
                        port_idx = 0;
                        while (port_idx < VTSS_PORT_ARRAY_SIZE) {
                            port_mask = 1LL<<port_idx;
                            if ((vtss_state->ts.status[id].reserved_mask & port_mask)) {
                                vtss_state->ts.status[id].reserved_mask  &= ~port_mask;
                                ts.id = id;
                                ts.ts = 0;
                                ts.ts_valid = FALSE;
                                if (vtss_state->ts.status[id].cb[port_idx] && vtss_state->ts.status[id].context[port_idx]) {
                                    my_vtss_state = vtss_state; /* save context */
                                    /* avoid using vtss_state while outside the API lock, as the API may be called from an other thread */
                                    cb = vtss_state->ts.status[id].cb[port_idx];
                                    cx = vtss_state->ts.status[id].context[port_idx];
                                    VTSS_EXIT();
                                    /* call out of the API, to indicate timeout */
                                    cb(cx, port_idx, &ts);
                                    VTSS_ENTER();
                                    vtss_state = my_vtss_state; /* restore context */
                                    vtss_state->ts.status[id].cb[port_idx] = NULL;
                                    vtss_state->ts.status[id].context[port_idx] = NULL;
                                } else {
                                    VTSS_D("undefined TS callback port_idx %d, ts_idx %d", port_idx, id);
                                }
                                VTSS_D("port_no %d, ts_id %d, ts %d(%d)", port_idx, ts.id, ts.ts, ts.ts_valid);
                            }
                            ++ port_idx;
                        }
                        vtss_state->ts.status[id].reserved_mask = 0LL;
                        vtss_state->ts.status[id].valid_mask = 0LL;
                        vtss_state->ts.status[id].rx_tc_valid = FALSE;
                        vtss_state->ts.status[id].age = 0;
                        rc = VTSS_FUNC(ts.timestamp_id_release, id);
                    
                    }
                }
            }
        }
        VTSS_EXIT();
    }
    return rc;
}

/* set port speed in the timestamp API (used to compensate for the internal ingress ane egress latencies) */
vtss_rc vtss_ts_status_change(const vtss_inst_t      inst,
                              const vtss_port_no_t   port_no,
                              vtss_port_speed_t      speed)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;
    
    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        rc = VTSS_FUNC(ts.status_change, port_no, speed);
    }
    VTSS_EXIT();
    return rc;
}

/* - Instance create and initialization ---------------------------- */

vtss_rc vtss_ts_inst_create(vtss_state_t *vtss_state)
{
    return VTSS_RC_OK;
}


/* - Debug print --------------------------------------------------- */

static const char *one_pps_mode_disp(vtss_ts_ext_clock_one_pps_mode_t m)
{
    switch (m) {
        case TS_EXT_CLOCK_MODE_ONE_PPS_DISABLE: return "Disable";
        case TS_EXT_CLOCK_MODE_ONE_PPS_OUTPUT: return "Output";
        case TS_EXT_CLOCK_MODE_ONE_PPS_INPUT: return "Input";
        default: return "unknown";
    }
}

void vtss_ts_debug_print(vtss_state_t *vtss_state,
                         const vtss_debug_printf_t pr,
                         const vtss_debug_info_t   *const info)
{
    u32               i,j;
    vtss_ts_conf_t *ts_conf;
    vtss_ts_port_conf_t *ts_port_conf;
    vtss_ts_timestamp_status_t *status;
    BOOL first = TRUE;

    if (!vtss_debug_group_enabled(pr, info, VTSS_DEBUG_GROUP_TS))
        return;

    ts_conf = &vtss_state->ts.conf;

    pr("One-Second Timer:\n");
    pr("Adjustment rate: %d ppb, \nOne_pps mode: %s ExternalClockOut mode: %s freq %d Hz\nClock offset %d sec\n",
                ts_conf->adj,

                one_pps_mode_disp(ts_conf->ext_clock_mode.one_pps_mode),
                ts_conf->ext_clock_mode.enable ? "enable" : "disable",
                ts_conf->ext_clock_mode.freq,
                ts_conf->sec_offset);

    pr("Port timestamp parameters:\n");
    pr("Port  IngressLatency  PeerDelay  EgressLatency  OperationMode\n");
    for (i = 0; i < VTSS_PORT_ARRAY_SIZE; i++) {
        ts_port_conf = &vtss_state->ts.port_conf[i];
        pr("%-4d  %-14d  %-9d  %-13d  %-d\n",
           i,
           VTSS_INTERVAL_NS(ts_port_conf->ingress_latency),
           VTSS_INTERVAL_NS(ts_port_conf->p2p_delay),
           VTSS_INTERVAL_NS(ts_port_conf->egress_latency),
           ts_port_conf->mode.mode);
    }
    
    
    (void)VTSS_FUNC_0(ts.timestamp_get);
    pr("Timestamp fifo data:\n");
    
    for (i = 0; i < VTSS_TS_ID_SIZE; i++) {
        status = &vtss_state->ts.status[i];
        if (status->reserved_mask != 0) {
            pr("Timestamp_id : %d  Reserved_mask: %" PRIx64 ", age %d\n", i, status->reserved_mask, status->age);
        }
        if (status->valid_mask != 0) {
            pr("                    Valid_mask: %" PRIx64 "\n", status->valid_mask);
        }
        first = TRUE;
        for (j = 0; j < VTSS_PORT_ARRAY_SIZE; j++) {
            if (status->valid_mask & 1LL<<j) {
                if (first) {
                    pr("Tx Port  time counter  time id\n");
                    first = FALSE;
                }
                pr("%-9d  %-14d %-14d \n", j, status->tx_tc[j], status->tx_id[j]);
            }
        }
        if (status->rx_tc_valid) {
            pr("Rx Timestamp_id : %d  Rx timecounter: %d, \n", i, status->rx_tc);
        }
    }
    pr("\n");
#if defined(VTSS_ARCH_SERVAL_CE)
    pr("OAM Timestamp fifo data:\n");
    
    for (i = 0; i < VTSS_VOE_ID_SIZE; i++) {
        first = TRUE;
        for (j = 0; j < VTSS_SERVAL_MAX_OAM_ENTRIES; j++) {
            vtss_oam_timestamp_entry_t *entry = &vtss_state->ts.oam_ts_status[i].entry[j];
            if (entry->valid) {
                if (first) {
                    pr("VOE ID : %d  last: %u  entry: %u\n", i, vtss_state->ts.oam_ts_status[i].last, j);
                    pr("Port  time counter  time id  sequence\n");
                    first = FALSE;
                }
                pr("%-4u  %-12u  %-7u  %-8u\n", entry->port, entry->tc, entry->id, entry->sq);
            }
        }
    }
    pr("\n");
#endif /*VTSS_ARCH_SERVAL */

}

#endif /* VTSS_FEATURE_TIMESTAMP */

