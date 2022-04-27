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
#include "vtss_jaguar1_cil.h"

#if defined(VTSS_ARCH_JAGUAR_1)

#if defined(VTSS_FEATURE_TIMESTAMP)

/* - CIL functions ------------------------------------------------- */

#define HW_CLK_CNT_PR_SEC 250000000L
#define HW_NS_PR_SEC 1000000000L
#define HW_NS_PR_CLK_CNT (HW_NS_PR_SEC/HW_CLK_CNT_PR_SEC)

#define HW_CLK_50_MS (HW_CLK_CNT_PR_SEC/20)
#define HW_CLK_M950_MS (-HW_CLK_CNT_PR_SEC + HW_CLK_50_MS)
#define EXT_SYNC_INPUT_LATCH_LATENCY 3  /* subtracted from EXT_SYNC_CURRENT TIME */

#define VTSS_JR_TX_TIMESTAMPS_PR_PORT 3

static vtss_rc
jr_ts_timeofday_get(vtss_state_t     *vtss_state,
                    vtss_timestamp_t *ts,
                    u32              *tc)
{
    u32 value;
    u32 sticky;
    i32 x;
    ts->seconds = vtss_state->ts.conf.sec_offset; /* sec counter is maintained in SW */
    ts->sec_msb = 0; /* to be maintained in one sec interrupt */
    JR_RD(DEVCPU_GCB, PTP_STAT_PTP_CURRENT_TIME_STAT, &value);
    x = VTSS_X_DEVCPU_GCB_PTP_STAT_PTP_CURRENT_TIME_STAT_PTP_CURRENT_TIME(value);
    /* DURING THE ADJUSTMENT PHASE THE ADJUST IS DONE IN sw */
    if (vtss_state->ts.conf.awaiting_adjustment) {
        x -= vtss_state->ts.conf.outstanding_corr;
        if (x < 0) {
            ts->seconds--;
            x += HW_CLK_CNT_PR_SEC;
        } else if (x >= HW_CLK_CNT_PR_SEC) {
            ts->seconds++;
            x -= HW_CLK_CNT_PR_SEC;
        }
    }
    /* if the timer has wrapped, and the onesec update has not been done, then increment ts->seconds */
    JR_RD(DEVCPU_GCB, PTP_STAT_PTP_EVT_STAT, &sticky);
    if (sticky & VTSS_F_DEVCPU_GCB_PTP_STAT_PTP_EVT_STAT_SYNC_STAT) {
        if (value < HW_CLK_50_MS) {
            ++ts->seconds;
        } else if (value < (HW_CLK_CNT_PR_SEC- HW_CLK_50_MS)){
            /* The counter may have wrapped after the readings, therefore a value close to 1 sec is ok */
            VTSS_E("too large interrupt latency: %d (clock cycles)", value);
        }
    }
        
    ts->nanoseconds = x*HW_NS_PR_CLK_CNT;
    *tc = VTSS_X_DEVCPU_GCB_PTP_STAT_PTP_CURRENT_TIME_STAT_PTP_CURRENT_TIME(value)*HW_NS_PR_CLK_CNT;
    VTSS_D("ts->seconds: %u, ts->nanoseconds: %u", ts->seconds, ts->nanoseconds);
    return VTSS_RC_OK;
}

static vtss_rc 
jr_ts_timeofday_next_pps_get(vtss_state_t *vtss_state, vtss_timestamp_t               *ts)
{
    ts->sec_msb = 0; /* to be maintained in one sec interrupt */
    ts->seconds = vtss_state->ts.conf.sec_offset+1; /* sec counter is maintained in SW */
    ts->nanoseconds = 0;
    VTSS_D("ts->seconds: %u, ts->nanoseconds: %u", ts->seconds, ts->nanoseconds);
    return VTSS_RC_OK;
}

/*
 * Subtract the offset from the actual time. 
 * offset is in the range [-1..+1] sec.
 * alignment algorithm:
 *  offset > 50 ms => 
 *    PTP_UPPER_LIMIT_1_TIME_ADJ = TOD_NANOSECS - ts.nanoseconds
 *    (TBD if this works, because the ADJ is < actual TOD_NANOSECS ??)
 *  50 ms > offset > -950 ms => 
 *    PTP_UPPER_LIMIT_1_TIME_ADJ = TOD_NANOSECS - ts.nanoseconds + 1 sec
 *  -950 ms > offset => 
 *    PTP_UPPER_LIMIT_1_TIME_ADJ = TOD_NANOSECS - ts.nanoseconds + 2 sec
 * The second offset is adjusted accordingly.
 *
 */
static vtss_rc
jr_ts_timeofday_offset_set(vtss_state_t *vtss_state, i32 offset)
{
    vtss_ts_conf_t *conf = &vtss_state->ts.conf;
    i32 corr;

    if (conf->awaiting_adjustment) {
        return VTSS_RC_ERROR;
    }
    conf->awaiting_adjustment = TRUE;
    conf->outstanding_adjustment = 0;
    
    corr = (offset/HW_NS_PR_CLK_CNT);
    
    if (corr > HW_CLK_50_MS) {
        --conf->sec_offset;
        conf->outstanding_adjustment = corr-1;
        conf->outstanding_corr = corr - HW_CLK_CNT_PR_SEC;
    } else if (corr < HW_CLK_M950_MS) {
        ++conf->sec_offset;
        conf->outstanding_adjustment = corr + 2*HW_CLK_CNT_PR_SEC-1;
        conf->outstanding_corr = corr + HW_CLK_CNT_PR_SEC;
    } else {
        conf->outstanding_corr = corr;
        conf->outstanding_adjustment = corr + HW_CLK_CNT_PR_SEC-1;
    }
    /* we wait until TOD_NANOSECS wraps */
    VTSS_D("offset = %d, adjust = %u, corr: %d", offset, conf->outstanding_adjustment, conf->outstanding_corr);
    return VTSS_RC_OK;
}

/*
* The actual second counter is saved in SW.
* The NS counter in hw (CURRENT_TIME) has to be aligned with the ts.nanoseconds,
* this is done by using the PTP_UPPER_LIMIT_1_TIME_ADJ:
* This register can be used to change the one sec. period once, the period can 
* be set to [0..268.435.455] periods of 4 ns = [0..1.073.741.820] ns.
* alignment algorithm:
*  (CURRENT_TIME - ts.nanoseconds) > 50 ms => 
*    PTP_UPPER_LIMIT_1_TIME_ADJ = CURRENT_TIME - ts.nanoseconds
*    (This has to be delayed until CURRENT_TIME wraps, because the ADJ is < actual CURRENT_TIME ??)
*  50 ms > (TOD_NANOSECS - ts.nanoseconds) > -950 ms => 
*    PTP_UPPER_LIMIT_1_TIME_ADJ = CURRENT_TIME - ts.nanoseconds + 1 sec
*  -950 ms > (CURRENT_TIME - ts.nanoseconds) => 
*    PTP_UPPER_LIMIT_1_TIME_ADJ = CURRENT_TIME - ts.nanoseconds + 2 sec
* The second offset is adjusted accordingly.
*
*/
static vtss_rc
    jr_ts_timeofday_set(vtss_state_t *vtss_state, const vtss_timestamp_t         *ts)
{
    vtss_ts_conf_t *conf = &vtss_state->ts.conf;
    i32 corr;
    u32 value;

    if (conf->awaiting_adjustment) {
        return VTSS_RC_ERROR;
    }
    
    conf->sec_offset = ts->seconds;
    /* corr = PTP_NS - ts.ns */
    JR_RD(DEVCPU_GCB, PTP_STAT_PTP_CURRENT_TIME_STAT, &value);
    VTSS_D("ts->seconds: %u, ts->nanoseconds: %u", ts->seconds, ts->nanoseconds);
    VTSS_D("PTP_CURRENT_TIME: %u", value);
    
    corr = (VTSS_X_DEVCPU_GCB_PTP_STAT_PTP_CURRENT_TIME_STAT_PTP_CURRENT_TIME(value));
    VTSS_RC(jr_ts_timeofday_offset_set(vtss_state, corr*HW_NS_PR_CLK_CNT - ts->nanoseconds));
    /* we wait until TOD_NANOSECS wraps */
    VTSS_D("PTP_TOD_SECS: %u, PTP_TOD_NANOSECS: %u", conf->sec_offset , value);
    return VTSS_RC_OK;
}

/*
 * When the time is set, the SW sec  counters are written. The HW ns clock is adjusted in the next 1 sec call. 
 */
static vtss_rc jr_ts_timeofday_set_delta(vtss_state_t *vtss_state,
                                         const vtss_timestamp_t *ts, BOOL negative)
{
    vtss_ts_conf_t *conf = &vtss_state->ts.conf;
    i32 corr;

    if (conf->awaiting_adjustment) {
        return VTSS_RC_ERROR;
    }
    if (ts->nanoseconds != 0) {
        if (negative) {
            corr = -(i32)ts->nanoseconds;
        } else {
            corr = ts->nanoseconds;
        }        
        VTSS_RC(jr_ts_timeofday_offset_set(vtss_state, corr));
    }
    if (negative) {
        conf->sec_offset -= ts->seconds;
    } else {
        conf->sec_offset += ts->seconds;
    }
    
    VTSS_D("ts->sec_msb: %u, ts->seconds: %u, ts->nanoseconds: %u", ts->sec_msb, ts->seconds, ts->nanoseconds);
    return VTSS_RC_OK;
}


/*
 * This function is called from interrupt, therefore the macros using vtss_state cannot be used.
 */
static u32 jr_ts_ns_cnt_get(vtss_inst_t inst)
{
    u32 value;
    (void)inst->init_conf.reg_read(0, VTSS_DEVCPU_GCB_PTP_STAT_PTP_CURRENT_TIME_STAT, &value);
    return VTSS_X_DEVCPU_GCB_PTP_STAT_PTP_CURRENT_TIME_STAT_PTP_CURRENT_TIME(value);
}

static vtss_rc
    jr_ts_timeofday_one_sec(vtss_state_t *vtss_state)
{
    vtss_ts_conf_t *conf = &vtss_state->ts.conf;
    u32 value;

    ++conf->sec_offset; /* sec counter is maintained in SW */
    if (conf->awaiting_adjustment) {
        /* For debug: read the internal NS counter */
        JR_RD(DEVCPU_GCB, PTP_STAT_PTP_CURRENT_TIME_STAT, &value);
        VTSS_D("TOD_NANOSECS %u", value);

        if (conf->outstanding_adjustment != 0) {
            if (value > conf->outstanding_adjustment) {
                VTSS_E("Too large interrupt latency to adjust %u (*4ns)", value);
            } else {
                /*PTP_UPPER_LIMIT_1_TIME_ADJ = x */
                JR_WR(DEVCPU_GCB, PTP_CFG_PTP_UPPER_LIMIT_1_TIME_ADJ_CFG, 
                      VTSS_F_DEVCPU_GCB_PTP_CFG_PTP_UPPER_LIMIT_1_TIME_ADJ_CFG_PTP_UPPER_LIMIT_1_TIME_ADJ(conf->outstanding_adjustment) |
                          VTSS_F_DEVCPU_GCB_PTP_CFG_PTP_UPPER_LIMIT_1_TIME_ADJ_CFG_PTP_UPPER_LIMIT_1_TIME_ADJ_SHOT);
                VTSS_D("onetime adjustment done %u", conf->outstanding_adjustment);
                conf->outstanding_adjustment = 0;
            }
        } else {
            conf->awaiting_adjustment = FALSE;
            VTSS_D("awaiting_adjustment cleared");
        }
    }
    return VTSS_RC_OK;
}

static vtss_rc
    jr_ts_adjtimer_set(vtss_state_t *vtss_state)
{
    /* adj in units of 0,1 pbb */
    i32 adj = vtss_state->ts.conf.adj;
    u32 clk_adj = (u64)HW_CLK_CNT_PR_SEC;
    i32 divisor = 0;
    /* CLK_ADJ = clock_rate/|adj|
    * CLK_ADJ_DIR = adj>0 ? Positive : Negative
    */
    if (adj != 0) {
        divisor = (i32) VTSS_DIV64(HW_NS_PR_SEC*10LL, vtss_state->ts.conf.adj);
        clk_adj = VTSS_LABS(divisor)-1;
    }
    vtss_state->ts.conf.adj_divisor = divisor;
    if (clk_adj > HW_CLK_CNT_PR_SEC) clk_adj = HW_CLK_CNT_PR_SEC;
    
    if ((adj != 0)) {
        clk_adj |= VTSS_F_DEVCPU_GCB_PTP_CFG_CLK_ADJ_CFG_CLK_ADJ_ENA;
        JR_WRM_SET(DEVCPU_GCB, PTP_CFG_GEN_EXT_CLK_CFG, VTSS_F_DEVCPU_GCB_PTP_CFG_GEN_EXT_CLK_CFG_GEN_EXT_CLK_ADJ_ENA);
    } else {
        JR_WRM_CLR(DEVCPU_GCB, PTP_CFG_GEN_EXT_CLK_CFG, VTSS_F_DEVCPU_GCB_PTP_CFG_GEN_EXT_CLK_CFG_GEN_EXT_CLK_ADJ_ENA);
    }
    if (adj < 0) {
        clk_adj |= VTSS_F_DEVCPU_GCB_PTP_CFG_CLK_ADJ_CFG_CLK_ADJ_DIR;
    } 
    JR_WR(DEVCPU_GCB, PTP_CFG_CLK_ADJ_CFG, clk_adj);
    
    VTSS_D("adj: %d, CLK_ACJ_CFG: %x, divisor %d", adj, clk_adj, divisor);
    return VTSS_RC_OK;
}

static vtss_rc
    jr_ts_freq_offset_get(vtss_state_t *vtss_state, i32 *const adj)
{
    i32 offset; /* frequency offset in clock counts pr sec */
    u32 cu;
    JR_RD(DEVCPU_GCB, PTP_STAT_EXT_SYNC_CURRENT_TIME_STAT,&cu);
    cu -= EXT_SYNC_INPUT_LATCH_LATENCY;
    offset = VTSS_X_DEVCPU_GCB_PTP_STAT_EXT_SYNC_CURRENT_TIME_STAT_EXT_SYNC_CURRENT_TIME(cu) - HW_CLK_CNT_PR_SEC;
    /* convert to units of 0,1 ppb */
    *adj = HW_NS_PR_CLK_CNT*10*offset;
    
    
    VTSS_D("cu: %u adj: %d", cu, *adj);
    return VTSS_RC_OK;
}




static vtss_rc
    jr_ts_external_clock_mode_set(vtss_state_t *vtss_state)
{
    vtss_ts_conf_t *conf = &vtss_state->ts.conf;
    u32 dividers;
    u32 high_div;
    u32 low_div;

    if (!conf->ext_clock_mode.enable) {
        /* disable clock output */
        JR_WRM_CLR(DEVCPU_GCB, PTP_CFG_GEN_EXT_CLK_CFG, VTSS_F_DEVCPU_GCB_PTP_CFG_GEN_EXT_CLK_CFG_GEN_EXT_CLK_ENA);
        /* disable alternate 1 for GPIO_9 (clock output). */
        (void) vtss_jr1_gpio_mode(vtss_state, 0, 9, VTSS_GPIO_IN);
    } else {
        /* enable alternate 1 for GPIO_9 (clock output). */
        (void) vtss_jr1_gpio_mode(vtss_state, 0, 9, VTSS_GPIO_ALT_0);
        
        /* set dividers; enable clock output. */
        dividers = (HW_CLK_CNT_PR_SEC/(conf->ext_clock_mode.freq));
        high_div = (dividers/2)-1;
        low_div =  ((dividers+1)/2)-1;
        JR_WR(DEVCPU_GCB, PTP_CFG_GEN_EXT_CLK_HIGH_PERIOD_CFG, high_div  &
               VTSS_M_DEVCPU_GCB_PTP_CFG_GEN_EXT_CLK_HIGH_PERIOD_CFG_GEN_EXT_CLK_HIGH_PERIOD);
        JR_WR(DEVCPU_GCB, PTP_CFG_GEN_EXT_CLK_LOW_PERIOD_CFG, low_div  &
               VTSS_M_DEVCPU_GCB_PTP_CFG_GEN_EXT_CLK_LOW_PERIOD_CFG_GEN_EXT_CLK_LOW_PERIOD);
        JR_WRM_SET(DEVCPU_GCB, PTP_CFG_GEN_EXT_CLK_CFG, VTSS_F_DEVCPU_GCB_PTP_CFG_GEN_EXT_CLK_CFG_GEN_EXT_CLK_ENA);
    }
    if ((conf->ext_clock_mode.one_pps_mode == TS_EXT_CLOCK_MODE_ONE_PPS_DISABLE)) {
        /* disable alternate 1 for GPIO_8 (clock output). */
        (void) vtss_jr1_gpio_mode(vtss_state, 0, 8, VTSS_GPIO_IN);
    } else {
        /* enable alternate 1 for GPIO_8 (clock output). */
        (void) vtss_jr1_gpio_mode(vtss_state, 0, 8, VTSS_GPIO_ALT_0);
    }
    if (conf->ext_clock_mode.one_pps_mode == TS_EXT_CLOCK_MODE_ONE_PPS_OUTPUT) {
        /* 1 pps output enabled */
        JR_WRM(DEVCPU_GCB, PTP_CFG_PTP_MISC_CFG, 
               VTSS_F_DEVCPU_GCB_PTP_CFG_PTP_MISC_CFG_EXT_SYNC_OUTP_ENA, 
               VTSS_F_DEVCPU_GCB_PTP_CFG_PTP_MISC_CFG_EXT_SYNC_OUTP_SEL |
                   VTSS_F_DEVCPU_GCB_PTP_CFG_PTP_MISC_CFG_EXT_SYNC_OUTP_INV |
                   VTSS_F_DEVCPU_GCB_PTP_CFG_PTP_MISC_CFG_EXT_SYNC_OUTP_ENA |
                   VTSS_F_DEVCPU_GCB_PTP_CFG_PTP_MISC_CFG_EXT_SYNC_INP_SEL |
                   VTSS_F_DEVCPU_GCB_PTP_CFG_PTP_MISC_CFG_EXT_SYNC_INP_INV |
                   VTSS_F_DEVCPU_GCB_PTP_CFG_PTP_MISC_CFG_EXT_SYNC_INP_ENA |
                   VTSS_F_DEVCPU_GCB_PTP_CFG_PTP_MISC_CFG_EXT_SYNC_ENA);
        
    } else if (conf->ext_clock_mode.one_pps_mode == TS_EXT_CLOCK_MODE_ONE_PPS_INPUT) {
        /* 1 pps input enabled */
        JR_WRM(DEVCPU_GCB, PTP_CFG_PTP_MISC_CFG, 
               VTSS_F_DEVCPU_GCB_PTP_CFG_PTP_MISC_CFG_EXT_SYNC_INP_ENA |
                   VTSS_F_DEVCPU_GCB_PTP_CFG_PTP_MISC_CFG_EXT_SYNC_ENA, 
               VTSS_F_DEVCPU_GCB_PTP_CFG_PTP_MISC_CFG_EXT_SYNC_OUTP_SEL |
                   VTSS_F_DEVCPU_GCB_PTP_CFG_PTP_MISC_CFG_EXT_SYNC_OUTP_INV |
                   VTSS_F_DEVCPU_GCB_PTP_CFG_PTP_MISC_CFG_EXT_SYNC_OUTP_ENA |
                   VTSS_F_DEVCPU_GCB_PTP_CFG_PTP_MISC_CFG_EXT_SYNC_INP_SEL |
                   VTSS_F_DEVCPU_GCB_PTP_CFG_PTP_MISC_CFG_EXT_SYNC_INP_INV |
                   VTSS_F_DEVCPU_GCB_PTP_CFG_PTP_MISC_CFG_EXT_SYNC_INP_ENA |
                   VTSS_F_DEVCPU_GCB_PTP_CFG_PTP_MISC_CFG_EXT_SYNC_ENA);
    } else {
        /* 1 pps disabled */
        JR_WRM(DEVCPU_GCB, PTP_CFG_PTP_MISC_CFG, 
               VTSS_F_DEVCPU_GCB_PTP_CFG_PTP_MISC_CFG_EXT_SYNC_OUTP_ENA, 
               VTSS_F_DEVCPU_GCB_PTP_CFG_PTP_MISC_CFG_EXT_SYNC_OUTP_SEL |
                   VTSS_F_DEVCPU_GCB_PTP_CFG_PTP_MISC_CFG_EXT_SYNC_OUTP_INV |
                   VTSS_F_DEVCPU_GCB_PTP_CFG_PTP_MISC_CFG_EXT_SYNC_OUTP_ENA |
                   VTSS_F_DEVCPU_GCB_PTP_CFG_PTP_MISC_CFG_EXT_SYNC_INP_SEL |
                   VTSS_F_DEVCPU_GCB_PTP_CFG_PTP_MISC_CFG_EXT_SYNC_INP_INV |
                   VTSS_F_DEVCPU_GCB_PTP_CFG_PTP_MISC_CFG_EXT_SYNC_INP_ENA |
                   VTSS_F_DEVCPU_GCB_PTP_CFG_PTP_MISC_CFG_EXT_SYNC_ENA);
    }
    return VTSS_RC_OK;
}

static vtss_rc 
    jr_ts_convert_timestamp(vtss_state_t *vtss_state, vtss_port_no_t port, u32 *ts)
{
    static const u32 slave_timer_offset[] = {
        /*  0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15 */
           13, 18, 19, 24, 25, 30, 14, 17, 20, 23, 26, 29, 15, 16, 21, 22,
        /* 16  17  18  19  20  21  22  23  24  25  26  27  28  29  30  31 */
           27, 28, 12, 11, 10,  9,  8,  7,  6,  5,  4, 34, 31, 33, 32,  3
    };
    /* chip port is signed, == -1 if unused port */
   *ts = *ts/HW_NS_PR_CLK_CNT; /* slave timers are in ns, we use clock cycles */
   /* clock adjustment compensation */
   if (vtss_state->ts.conf.adj_divisor) {
       *ts = *ts + (i32)*ts/vtss_state->ts.conf.adj_divisor;
   }
   if ((u32)VTSS_CHIP_PORT(port) < sizeof(slave_timer_offset)/sizeof(u32)) {
        *ts += slave_timer_offset[VTSS_CHIP_PORT(port)];
        if (*ts > HW_CLK_CNT_PR_SEC) {
            *ts -= HW_CLK_CNT_PR_SEC;
        }
        *ts = *ts*HW_NS_PR_CLK_CNT; /* slave timers are in ns */
        return VTSS_RC_OK;
    } else {
        return VTSS_RC_ERROR;
    }
}


static vtss_rc 
    jr_ts_timestamp_get(vtss_state_t *vtss_state)
{
    u32 value;
    u32 ts_ts;
    u32 ts_id;
    u32 chip_port;
    u32 idx;
    u32 sticky_mask;
    vtss_port_no_t tx_port;
    
    for (tx_port = 0; tx_port < VTSS_PORT_ARRAY_SIZE; tx_port++) {
        chip_port = VTSS_CHIP_PORT(tx_port);

        if (VTSS_PORT_IS_1G(chip_port)) {
            JR_RDX(DEV1G, DEV_CFG_STATUS_DEV_PTP_TX_STICKY, VTSS_TO_DEV(chip_port), &value);
        } else {
            JR_RDX(DEV10G, DEV_CFG_STATUS_DEV_PTP_TX_STICKY, VTSS_TO_DEV(chip_port), &value);
        }
        sticky_mask = 0x1;
        for (idx = 0; idx < VTSS_JR_TX_TIMESTAMPS_PR_PORT; idx++) {
            if (value & sticky_mask) {
                if (VTSS_PORT_IS_1G(chip_port)) {
                    JR_RDXY(DEV1G, DEV_CFG_STATUS_DEV_PTP_TX_TSTAMP, VTSS_TO_DEV(chip_port), idx, &ts_ts);
                    JR_RDXY(DEV1G, DEV_CFG_STATUS_DEV_PTP_TX_ID, VTSS_TO_DEV(chip_port), idx, &ts_id);
                    JR_WRX(DEV1G, DEV_CFG_STATUS_DEV_PTP_TX_STICKY, VTSS_TO_DEV(chip_port), sticky_mask);
                } else {
                    JR_RDXY(DEV10G, DEV_CFG_STATUS_DEV_PTP_TX_TSTAMP, VTSS_TO_DEV(chip_port), idx, &ts_ts);
                    JR_RDXY(DEV10G, DEV_CFG_STATUS_DEV_PTP_TX_ID, VTSS_TO_DEV(chip_port), idx, &ts_id);
                    JR_WRX(DEV10G, DEV_CFG_STATUS_DEV_PTP_TX_STICKY, VTSS_TO_DEV(chip_port), sticky_mask);
                }
                (void)jr_ts_convert_timestamp(vtss_state, tx_port, &ts_ts);
                vtss_state->ts.status[idx].tx_tc[tx_port] = ts_ts;
                vtss_state->ts.status[idx].tx_id[tx_port] = ts_id;
                vtss_state->ts.status[idx].valid_mask |= 1LL<<tx_port;
                VTSS_D("Txstamp[%d], ts = %u, id = %u, port = %u", idx, ts_ts, ts_id, tx_port);
            }
            sticky_mask = sticky_mask<<1;
        }
    }
    return VTSS_RC_OK;
}



static vtss_rc 
    jr_ts_timestamp_id_release(vtss_state_t *vtss_state, u32 ts_id)
{
    return VTSS_RC_OK;
}

/* - Debug print --------------------------------------------------- */

static vtss_rc jr_debug_ts(vtss_state_t *vtss_state,
                           const vtss_debug_printf_t pr,
                           const vtss_debug_info_t   *const info)
{
    u32  value;
    u32  port_no;
    char buf[32];
    
    /* DEVCPU_GCB: PTP_CFG */
    vtss_jr1_debug_reg_header(pr, "GCB:PTP_CFG");
    JR_DEBUG_REG_NAME(pr, DEVCPU_GCB, PTP_CFG_PTP_MISC_CFG, "PTP_MISC_CFG");
    JR_DEBUG_REG_NAME(pr, DEVCPU_GCB, PTP_CFG_PTP_UPPER_LIMIT_CFG, "PTP_UPPER_LIMIT_CFG");
    JR_DEBUG_REG_NAME(pr, DEVCPU_GCB, PTP_CFG_PTP_UPPER_LIMIT_1_TIME_ADJ_CFG, "PTP_UPPER_LIMIT_1_TIME_ADJ_CFG");
    JR_DEBUG_REG_NAME(pr, DEVCPU_GCB, PTP_CFG_PTP_SYNC_INTR_ENA_CFG, "PTP_SYNC_INTR_ENA_CFG");
    JR_DEBUG_REG_NAME(pr, DEVCPU_GCB, PTP_CFG_GEN_EXT_CLK_HIGH_PERIOD_CFG, "GEN_EXT_CLK_HIGH_PERIOD_CFG");
    JR_DEBUG_REG_NAME(pr, DEVCPU_GCB, PTP_CFG_GEN_EXT_CLK_LOW_PERIOD_CFG, "GEN_EXT_CLK_LOW_PERIOD_CFG");
    JR_DEBUG_REG_NAME(pr, DEVCPU_GCB, PTP_CFG_GEN_EXT_CLK_CFG, "GEN_EXT_CLK_CFG");
    JR_DEBUG_REG_NAME(pr, DEVCPU_GCB, PTP_CFG_CLK_ADJ_CFG, "CLK_ADJ_CFG");
    /* DEVCPU_GCB: PTP_STAT */
    vtss_jr1_debug_reg_header(pr, "GCB:PTP_STAT");
    JR_DEBUG_REG_NAME(pr, DEVCPU_GCB, PTP_STAT_PTP_CURRENT_TIME_STAT, "PTP_CURRENT_TIME_STAT");
    JR_DEBUG_REG_NAME(pr, DEVCPU_GCB, PTP_STAT_EXT_SYNC_CURRENT_TIME_STAT, "EXT_SYNC_CURRENT_TIME_STAT");
    
    JR_RD(DEVCPU_GCB, PTP_STAT_PTP_EVT_STAT, &value);
    JR_WR(DEVCPU_GCB, PTP_STAT_PTP_EVT_STAT, value);
    vtss_debug_print_sticky(pr, "CLK_ADJ_UPD_STICKY", value, VTSS_F_DEVCPU_GCB_PTP_STAT_PTP_EVT_STAT_CLK_ADJ_UPD_STICKY);
    vtss_debug_print_sticky(pr, "EXT_SYNC_CURRENT_TIME_STICKY", value, VTSS_F_DEVCPU_GCB_PTP_STAT_PTP_EVT_STAT_EXT_SYNC_CURRENT_TIME_STICKY);
    vtss_debug_print_sticky(pr, "SYNC_STAT", value, VTSS_F_DEVCPU_GCB_PTP_STAT_PTP_EVT_STAT_SYNC_STAT);
    
    /* SYS_PORT:PTP */
    for (port_no = 0; port_no < vtss_state->port_count; port_no++) {
        u32 chip_port = VTSS_CHIP_PORT(port_no);
        if (vtss_jr1_debug_port_skip(vtss_state, port_no, info))
            continue;
        
        sprintf(buf, "DEV1G:PTP, CHIP PORT %d", chip_port);
        vtss_jr1_debug_reg_header(pr, buf);
        
        JR_DEBUG_REGX_NAME(pr, ASM, CFG_ETH_CFG, chip_port, "ASM::ETH_CFG");
        if (VTSS_PORT_IS_1G(chip_port)) {
            JR_DEBUG_REGX_NAME(pr, DEV1G, DEV1G_INTR_CFG_STATUS_DEV1G_INTR_CFG, VTSS_TO_DEV(chip_port), "DEV1G_INTR_CFG");
            JR_DEBUG_REGX_NAME(pr, DEV1G, DEV1G_INTR_CFG_STATUS_DEV1G_INTR, VTSS_TO_DEV(chip_port), "DEV1G_INTR");
            JR_DEBUG_REGX_NAME(pr, DEV1G, DEV_CFG_STATUS_DEV_PTP_CFG, VTSS_TO_DEV(chip_port), "DEV_PTP_CFG");
            JR_DEBUG_REGX_NAME(pr, DEV1G, DEV_CFG_STATUS_DEV_PTP_TX_STICKY, VTSS_TO_DEV(chip_port), "DEV_PTP_TX_STICKY");
            JR_DEBUG_REGXY_NAME(pr, DEV1G, DEV_CFG_STATUS_DEV_PTP_TX_TSTAMP, VTSS_TO_DEV(chip_port), 0, "DEV_PTP_TX_TSTAMP0");
            JR_DEBUG_REGXY_NAME(pr, DEV1G, DEV_CFG_STATUS_DEV_PTP_TX_TSTAMP, VTSS_TO_DEV(chip_port), 1, "DEV_PTP_TX_TSTAMP1");
            JR_DEBUG_REGXY_NAME(pr, DEV1G, DEV_CFG_STATUS_DEV_PTP_TX_TSTAMP, VTSS_TO_DEV(chip_port), 2, "DEV_PTP_TX_TSTAMP2");
            JR_DEBUG_REGXY_NAME(pr, DEV1G, DEV_CFG_STATUS_DEV_PTP_TX_ID, VTSS_TO_DEV(chip_port), 0, "DEV_PTP_TX_ID0");
            JR_DEBUG_REGXY_NAME(pr, DEV1G, DEV_CFG_STATUS_DEV_PTP_TX_ID, VTSS_TO_DEV(chip_port), 1, "DEV_PTP_TX_ID1");
            JR_DEBUG_REGXY_NAME(pr, DEV1G, DEV_CFG_STATUS_DEV_PTP_TX_ID, VTSS_TO_DEV(chip_port), 2, "DEV_PTP_TX_ID2");
            
        } else {
            JR_DEBUG_REGX_NAME(pr, DEV10G, DEV_CFG_STATUS_DEV_PTP_CFG, VTSS_TO_DEV(chip_port), "DEV_PTP_CFG");
            JR_DEBUG_REGX_NAME(pr, DEV10G, DEV_CFG_STATUS_DEV_PTP_TX_STICKY, VTSS_TO_DEV(chip_port), "DEV_PTP_TX_STICKY");
            JR_DEBUG_REGXY_NAME(pr, DEV10G, DEV_CFG_STATUS_DEV_PTP_TX_TSTAMP, VTSS_TO_DEV(chip_port), 0, "DEV_PTP_TX_TSTAMP0");
            JR_DEBUG_REGXY_NAME(pr, DEV10G, DEV_CFG_STATUS_DEV_PTP_TX_TSTAMP, VTSS_TO_DEV(chip_port), 1, "DEV_PTP_TX_TSTAMP1");
            JR_DEBUG_REGXY_NAME(pr, DEV10G, DEV_CFG_STATUS_DEV_PTP_TX_TSTAMP, VTSS_TO_DEV(chip_port), 2, "DEV_PTP_TX_TSTAMP2");
            JR_DEBUG_REGXY_NAME(pr, DEV10G, DEV_CFG_STATUS_DEV_PTP_TX_ID, VTSS_TO_DEV(chip_port), 0, "DEV_PTP_TX_ID0");
            JR_DEBUG_REGXY_NAME(pr, DEV10G, DEV_CFG_STATUS_DEV_PTP_TX_ID, VTSS_TO_DEV(chip_port), 1, "DEV_PTP_TX_ID1");
            JR_DEBUG_REGXY_NAME(pr, DEV10G, DEV_CFG_STATUS_DEV_PTP_TX_ID, VTSS_TO_DEV(chip_port), 2, "DEV_PTP_TX_ID2");
            JR_DEBUG_REGX_NAME(pr, DEV10G, MAC_CFG_STATUS_MAC_MODE_CFG, VTSS_TO_DEV(chip_port), "MAC_MODE_CFG");
        }
    }
    
    /* ANA:: */
    vtss_jr1_debug_reg_header(pr, "ANA::PTP");
    JR_DEBUG_REG_NAME(pr, ANA_AC, PS_COMMON_MISC_CTRL, "MISC-CTRL");
    
    
    pr("\n");
    
    return VTSS_RC_OK;
}

vtss_rc vtss_jr1_ts_debug_print(vtss_state_t *vtss_state,
                                const vtss_debug_printf_t pr,
                                const vtss_debug_info_t   *const info)
{
    return vtss_debug_print_group(VTSS_DEBUG_GROUP_TS, jr_debug_ts, vtss_state, pr, info);
}

/* - Initialization ------------------------------------------------ */

static vtss_rc jr_ts_init(vtss_state_t *vtss_state)
{
    u32 port;

    /* enable ptp master counter and clock adjustment */
    JR_WR(DEVCPU_GCB, PTP_CFG_PTP_MISC_CFG, VTSS_F_DEVCPU_GCB_PTP_CFG_PTP_MISC_CFG_PTP_ENA);
    JR_WR(DEVCPU_GCB, PTP_CFG_CLK_ADJ_CFG,  HW_CLK_CNT_PR_SEC |
           VTSS_F_DEVCPU_GCB_PTP_CFG_CLK_ADJ_CFG_CLK_ADJ_UPD);
    /* configure RX timestamping config: Overwriting the frame's FCS field with the Rx timestamp value*/
    JR_WRM_SET(ANA_AC, PS_COMMON_MISC_CTRL, VTSS_F_ANA_AC_PS_COMMON_MISC_CTRL_OAM_RX_TSTAMP_IN_FCS_ENA);
    /* enable timestamping for all ports. Note: Vstax2 awareness is disabled. */
    for (port = 0; port < VTSS_CHIP_PORTS; port++) {
        JR_WRXM_SET(ASM, CFG_ETH_CFG, port, VTSS_F_ASM_CFG_ETH_CFG_ETH_PRE_MODE);
        if (VTSS_PORT_IS_1G(port)) {
            JR_WRXM_SET(DEV1G, DEV_CFG_STATUS_DEV_PTP_CFG, VTSS_TO_DEV(port), VTSS_F_DEV1G_DEV_CFG_STATUS_DEV_PTP_CFG_PTP_ENA);
        } else {
            JR_WRXM_SET(DEV10G, DEV_CFG_STATUS_DEV_PTP_CFG, VTSS_TO_DEV(port), VTSS_F_DEV10G_DEV_CFG_STATUS_DEV_PTP_CFG_PTP_CFG_ENA);
        }
    }

    return VTSS_RC_OK;
}

vtss_rc vtss_jr1_ts_init(vtss_state_t *vtss_state, vtss_init_cmd_t cmd)
{
    vtss_ts_state_t *state = &vtss_state->ts;

    switch (cmd) {
    case VTSS_INIT_CMD_CREATE:
        state->timeofday_get = jr_ts_timeofday_get;
        state->timeofday_next_pps_get = jr_ts_timeofday_next_pps_get;
        state->timeofday_set = jr_ts_timeofday_set;
        state->timeofday_set_delta = jr_ts_timeofday_set_delta;
        state->timeofday_offset_set = jr_ts_timeofday_offset_set;
        state->ns_cnt_get = jr_ts_ns_cnt_get;
        state->timeofday_one_sec = jr_ts_timeofday_one_sec;
        state->adjtimer_set = jr_ts_adjtimer_set;
        state->freq_offset_get = jr_ts_freq_offset_get;
        state->external_clock_mode_set = jr_ts_external_clock_mode_set;
        state->timestamp_get = jr_ts_timestamp_get;
        state->timestamp_convert = jr_ts_convert_timestamp;
        state->timestamp_id_release = jr_ts_timestamp_id_release;
        break;
    case VTSS_INIT_CMD_INIT:
        VTSS_RC(jr_ts_init(vtss_state));
        break;
    default:
        break;
    }
    
    return VTSS_RC_OK;
}

#endif /* VTSS_FEATURE_TIMESTAMP */

#endif /* VTSS_ARCH_JAGUAR_1 */
