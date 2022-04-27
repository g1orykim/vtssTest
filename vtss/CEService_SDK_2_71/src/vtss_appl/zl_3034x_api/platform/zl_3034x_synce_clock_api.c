/*

 Vitesse Clock API software.

 Copyright (c) 2002-2011 Vitesse Semiconductor Corporation "Vitesse". All
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

#include "synce_custom_clock_api.h"
#include "zl_3034x_synce_clock_api.h"
#include "zl_3034x_api.h"
#include "zl303xx.h"
#include "zl303xx_DpllLow.h"
#include "zl303xx_IsrLow.h"
#include "zl303xx_CustLow.h"
#include "zl303xx_RefLow.h"
#include "zl303xx_SynthLow.h"
#include "zl303xx_PllFuncs.h"
#include "main.h"
#include "vtss_os.h"
#include "critd_api.h"
#include <vtss_trace_lvl_api.h>
#include <stdio.h>
#include "port_custom_api.h"

static BOOL is_pcb106 = FALSE;
static BOOL cur_ref_failed = FALSE;
static BOOL cur_holdover  = FALSE;

static uint cur_pri[CLOCK_INPUT_MAX];
static BOOL cur_locs_failed[CLOCK_INPUT_MAX] = {FALSE, FALSE, FALSE};


static zl303xx_RefIdE clock_input2ref_id(uint clock_input)
{
    switch (clock_input) {
        case 0: return ZL303XX_REF_ID_0;
        case 1: 
            if (is_pcb106) return ZL303XX_REF_ID_4;
            return ZL303XX_REF_ID_1;
        case 2: 
            if (is_pcb106) return ZL303XX_REF_ID_6;
            return ZL303XX_REF_ID_7;
        default: return ZL303XX_REF_ID_0;
    }
}

static uint ref_id2clock_input(zl303xx_RefIdE ref_id)
{    
    switch (ref_id) {
        case ZL303XX_REF_ID_0: return 0;
        case ZL303XX_REF_ID_1:
            if (is_pcb106) return 0;
            return 1;
        case ZL303XX_REF_ID_4: 
            if (is_pcb106) return 1;
            return 0;
        case ZL303XX_REF_ID_7: 
            if (is_pcb106) return 0;
            return 2;
        case ZL303XX_REF_ID_6: 
            if (!is_pcb106) return 0;
            return 2;
        default: return 0;
    }
}

/* zl priorities: 0 = highest, 0xe = lowest, 0xf = disabled */
/* synce priorities: 0 = highest, 0x1 = lowest */
static Uint32T sync_pri2zl_pri(uint pri)
{
    return pri;
}

static uint zl_pri2sync_pri(Uint32T pri)
{
    return pri;
}

vtss_rc zl_3034x_clock_init(BOOL  cold_init)
{
    uint i;
    for (i = 0; i < CLOCK_INPUT_MAX; i++) {
        cur_pri[i] = 0xf;
    }
    vtss_rc rc=VTSS_OK;
    return(rc);
}

vtss_rc zl_3034x_clock_startup(BOOL  cold_init)
{
    zl303xx_RefConfigS ref_cfg;
    // the PHY reference input depends on the board
    int board_type = vtss_board_type();
    
    if (board_type == VTSS_BOARD_SERVAL_PCB106_REF) {
        is_pcb106 = TRUE;
    } else {
        is_pcb106 = FALSE;
    }
    T_NG(TRACE_GRP_SYNC_INTF,"is_pcb106 %d", is_pcb106);
    
    /* MODE CUSTA set to 25 MHz: val = 25000/8 = 3125*/
    ZL_3034X_CHECK(zl303xx_CustFrequencySet(zl_3034x_zl303xx_params, ZL303XX_CUST_ID_A, 3125));
    /* SCM limits: see ZL30343 data sheet section 7.10.13*/
    ZL_3034X_CHECK(zl303xx_CustScmLoLimitSet(zl_3034x_zl303xx_params, ZL303XX_CUST_ID_A, 6));
    ZL_3034X_CHECK(zl303xx_CustScmHiLimitSet(zl_3034x_zl303xx_params, ZL303XX_CUST_ID_A, 18));
    /* CFM limits: see ZL30343 data sheet section 7.10.13, fig 25 */
    ZL_3034X_CHECK(zl303xx_CustCfmLoLimitSet(zl_3034x_zl303xx_params, ZL303XX_CUST_ID_A, 1591));
    ZL_3034X_CHECK(zl303xx_CustCfmHiLimitSet(zl_3034x_zl303xx_params, ZL303XX_CUST_ID_A, 1689));
    ZL_3034X_CHECK(zl303xx_CustCfmCyclesSet(zl_3034x_zl303xx_params, ZL303XX_CUST_ID_A, 127));
    ZL_3034X_CHECK(zl303xx_CustDivideSet(zl_3034x_zl303xx_params, ZL303XX_CUST_ID_A, ZL303XX_TRUE));

    /* MODE CUSTB set to 10 MHz: val = 10000/8 = 1250*/
    ZL_3034X_CHECK(zl303xx_CustFrequencySet(zl_3034x_zl303xx_params, ZL303XX_CUST_ID_B, 1250));
    /* SCM limits: see ZL30343 data sheet section 7.10.13*/
    ZL_3034X_CHECK(zl303xx_CustScmLoLimitSet(zl_3034x_zl303xx_params, ZL303XX_CUST_ID_B, 15));
    ZL_3034X_CHECK(zl303xx_CustScmHiLimitSet(zl_3034x_zl303xx_params, ZL303XX_CUST_ID_B, 45));
    /* CFM limits: see ZL30343 data sheet section 7.10.13, fig 25 */
    /* D=1, N=256, cfm_low_limit = 1/10,3*256*80 = 2003, cfm_high_limit = 1/9,7*256*80 = 2111 */
    ZL_3034X_CHECK(zl303xx_CustCfmLoLimitSet(zl_3034x_zl303xx_params, ZL303XX_CUST_ID_B, 2003));
    ZL_3034X_CHECK(zl303xx_CustCfmHiLimitSet(zl_3034x_zl303xx_params, ZL303XX_CUST_ID_B, 2111));
    ZL_3034X_CHECK(zl303xx_CustCfmCyclesSet(zl_3034x_zl303xx_params, ZL303XX_CUST_ID_B, 255));
    ZL_3034X_CHECK(zl303xx_CustDivideSet(zl_3034x_zl303xx_params, ZL303XX_CUST_ID_B, ZL303XX_FALSE));

    /* initialize reference 0 and 1 for 125 MHZ input */
    ref_cfg.mode = ZL303XX_REF_MODE_CUSTA;  /* MODE CUSTA must be set to 25 MHz */
    ref_cfg.invert = ZL303XX_FALSE;
    ref_cfg.prescaler = ZL303XX_REF_DIV_5;
    ref_cfg.oorLimit = ZL303XX_OOR_52_67PPM;
    ref_cfg.Id = clock_input2ref_id(0);
    ZL_3034X_CHECK(zl303xx_RefConfigSet(zl_3034x_zl303xx_params, &ref_cfg));
    ref_cfg.Id = clock_input2ref_id(1);
    ZL_3034X_CHECK(zl303xx_RefConfigSet(zl_3034x_zl303xx_params, &ref_cfg));
    T_IG(TRACE_GRP_SYNC_INTF,"clock startup: Enable ref 0 and 1 for 125 MHz");
    
    /* set pullIn range to +/-12 ppm */
    ZL_3034X_CHECK(zl303xx_DpllPullinRangeSet(zl_3034x_zl303xx_params, ZL303XX_DPLL_ID_1, ZL303XX_DPLL_PULLIN_12PPM));
    /* set phase slope limit to  885 ns/s */
    ZL_3034X_CHECK(zl303xx_DpllPhaseLimitSet(zl_3034x_zl303xx_params, ZL303XX_DPLL_ID_1, ZL303XX_DPLL_PSL_P885US));
    /* enable hitless switching */
    ZL_3034X_CHECK(zl303xx_DpllHitlessSet(zl_3034x_zl303xx_params, ZL303XX_DPLL_ID_1, ZL303XX_DPLL_HITLESS));
    /* GST time to disqualify = 100 ms, time to qualify = 200 ms */
    ZL_3034X_CHECK(zl303xx_RefTime2DisqSet(zl_3034x_zl303xx_params, ZL303XX_TTDQ_100MS));
    ZL_3034X_CHECK(zl303xx_RefTime2QualifySet(zl_3034x_zl303xx_params, ZL303XX_TTQ_X2));
    
    
    //zl_3034x_clock_event_enable(CLOCK_LOSX_EV | CLOCK_LOL_EV | CLOCK_LOCS1_EV | CLOCK_LOCS2_EV | CLOCK_FOS1_EV | CLOCK_FOS2_EV);
    
    
    return(VTSS_OK);
}

static zl303xx_DpllModeE current_mode = ZL303XX_DPLL_MODE_NORM;
vtss_rc zl_3034x_clock_selection_mode_set(const clock_selection_mode_t   mode,
                                 const uint                     clock_input)
{
    uint source;
    zl303xx_DpllModeE val = -1;
    switch (mode)
    {
        case CLOCK_SELECTION_MODE_MANUEL:
            val =    ZL303XX_DPLL_MODE_NORM;
            break;
        case CLOCK_SELECTION_MODE_AUTOMATIC_NONREVERTIVE:
            val =    ZL303XX_DPLL_MODE_AUTO;
            ZL_3034X_CHECK(zl303xx_DpllRevertiveEnSet(zl_3034x_zl303xx_params,
                           ZL303XX_DPLL_ID_1,ZL303XX_FALSE));
            for (source = 0; source < clock_my_input_max; ++source) {
                ZL_3034X_CHECK(zl303xx_DpllRefRevSwitchEnSet(zl_3034x_zl303xx_params,
                               ZL303XX_DPLL_ID_1, clock_input2ref_id(source), ZL303XX_FALSE));
            }
            break;
        case CLOCK_SELECTION_MODE_AUTOMATIC_REVERTIVE:
            val =    ZL303XX_DPLL_MODE_AUTO;
            ZL_3034X_CHECK(zl303xx_DpllRevertiveEnSet(zl_3034x_zl303xx_params,
                                                 ZL303XX_DPLL_ID_1,ZL303XX_TRUE));
            for (source = 0; source < clock_my_input_max; ++source) {
                ZL_3034X_CHECK(zl303xx_DpllRefRevSwitchEnSet(zl_3034x_zl303xx_params,
                               ZL303XX_DPLL_ID_1, clock_input2ref_id(source), ZL303XX_TRUE));
            }            
            break;
        case CLOCK_SELECTION_MODE_FORCED_HOLDOVER:
            val =    ZL303XX_DPLL_MODE_HOLD;
            break;
        case CLOCK_SELECTION_MODE_FORCED_FREE_RUN:
            val =    ZL303XX_DPLL_MODE_FREE;
            break;
        default: break;
    }

    if (val != current_mode) {
        ZL_3034X_CHECK(zl303xx_DpllModeSelectSet(zl_3034x_zl303xx_params,
                                        ZL303XX_DPLL_ID_1, val));
        current_mode = val;
        T_IG(TRACE_GRP_SYNC_INTF,"DPLL mode: changed to %d", val);
    }
    if (mode == CLOCK_SELECTION_MODE_MANUEL) {
        ZL_3034X_CHECK(zl303xx_DpllRefSelectSet(zl_3034x_zl303xx_params,
                                       ZL303XX_DPLL_ID_1, clock_input2ref_id(clock_input)));
    }
    T_IG(TRACE_GRP_SYNC_INTF,"DPLL mode: %d, ref %d", val, clock_input2ref_id(clock_input));
    
    return(VTSS_OK);
}


vtss_rc zl_3034x_clock_selector_state_get(clock_selector_state_t  *const selector_state,
                                 uint                    *const clock_input)
{
    zl303xx_DpllStatusS par;
    zl303xx_DpllModeE mode;
    zl303xx_RefIdE ref_id;
    par.Id = ZL303XX_DPLL_ID_1;
    ZL_3034X_CHECK(zl303xx_DpllStatusGet(zl_3034x_zl303xx_params, &par));
    ZL_3034X_CHECK(zl303xx_DpllModeSelectGet(zl_3034x_zl303xx_params, ZL303XX_DPLL_ID_1, &mode));

    if (par.refFailed == ZL303XX_FALSE && par.holdover == ZL303XX_DPLL_HOLD_FALSE && par.locked == ZL303XX_TRUE) {
        *selector_state = CLOCK_SELECTOR_STATE_LOCKED;
    } else if (par.holdover == ZL303XX_DPLL_HOLD_TRUE) {
        *selector_state = CLOCK_SELECTOR_STATE_HOLDOVER;
    } else if (par.locked == ZL303XX_DPLL_LOCK_FALSE && par.holdover == ZL303XX_DPLL_HOLD_FALSE) {
        if (mode == ZL303XX_DPLL_MODE_FREE) {
            *selector_state = CLOCK_SELECTOR_STATE_FREERUN;
        } else {
            *selector_state = CLOCK_SELECTOR_STATE_LOCKED;
            T_DG(TRACE_GRP_SYNC_INTF,"acquiring lock, mode %d", mode);
        }
    } else {
        T_WG(TRACE_GRP_SYNC_INTF,"inconsistent selector state");
        *selector_state = CLOCK_SELECTOR_STATE_LOCKED;
    }
    T_DG(TRACE_GRP_SYNC_INTF,"refFailed: %d, holdover %d, locked %d", par.refFailed , par.holdover, par.locked);
    ZL_3034X_CHECK(zl303xx_DpllRefSelectGet(zl_3034x_zl303xx_params,
                                       ZL303XX_DPLL_ID_1, &ref_id));
    *clock_input = ref_id2clock_input(ref_id);
    T_IG(TRACE_GRP_SYNC_INTF,"selector state: %d, ref %d", *selector_state, ref_id);
    return(VTSS_OK);
}

vtss_rc zl_3034x_clock_priority_set(const uint   clock_input,
                                    const uint   priority)
{
    if (priority != cur_pri[clock_input]) {
        ZL_3034X_CHECK(zl303xx_DpllRefPrioritySet(zl_3034x_zl303xx_params,
                                         ZL303XX_DPLL_ID_1,
                                         clock_input2ref_id(clock_input),
                                         sync_pri2zl_pri(priority)));
        T_IG(TRACE_GRP_SYNC_INTF,"Priority: %d, clock_input %d", sync_pri2zl_pri(priority), clock_input2ref_id(clock_input));
        cur_pri[clock_input] = priority;
    }
    return(VTSS_OK);
}

vtss_rc zl_3034x_clock_priority_get(const uint   clock_input,
                                    uint         *const priority)
{
    Uint32T zl_pri;
    ZL_3034X_CHECK(zl303xx_DpllRefPriorityGet(zl_3034x_zl303xx_params,
                   ZL303XX_DPLL_ID_1,
                   clock_input2ref_id(clock_input),
                   &zl_pri));
    *priority = zl_pri2sync_pri(zl_pri);
    T_DG(TRACE_GRP_SYNC_INTF,"Priority: %d, clock_input %d", zl_pri, clock_input2ref_id(clock_input));
    return(VTSS_OK);
}


/*
 * time = 0 = holdoff disable
 * time =   100:  -> 0.5 ms
 *          200:  -> 1 ms
 *          300:  -> 5 ms
 *          400:  -> 10 ms
 *          500:  -> 50 ms
 *          600:  -> 100 ms
 *          700:  -> 500 ms
 *          800:  -> 1 s
 *          900:  -> 2 s
 *          1000: -> 2.5 s
 *          1100: -> 4 s
 *          1200: -> 8 s
 *          1300: -> 16 s
 *          1400: -> 32 s
 *        >=1500: -> 64 s
 */
vtss_rc   zl_3034x_clock_holdoff_time_set(const uint   clock_input,
                                 const uint   time)
{
    zl303xx_TtoDisQualE val;
    val = time/100;
    if (val > ZL303XX_TTO_DIS_QUAL_MAX) val = ZL303XX_TTO_DIS_QUAL_MAX;

    ZL_3034X_CHECK(zl303xx_RefTime2DisqSet(zl_3034x_zl303xx_params, val));
    ZL_3034X_CHECK(zl303xx_RefTime2QualifySet(zl_3034x_zl303xx_params, ZL303XX_TTQ_X2));
    return(VTSS_OK);

}

vtss_rc zl_3034x_clock_los_get(const uint   clock_input,
                               BOOL         *const los)
{
    *los = cur_locs_failed[clock_input];
    T_DG(TRACE_GRP_SYNC_INTF,"Clock_los: %d, clock_input %d", *los, clock_input2ref_id(clock_input));
    return(VTSS_OK);
}

vtss_rc zl_3034x_clock_losx_state_get(BOOL *const state)
{
    zl303xx_BooleanE val;
    ZL_3034X_CHECK(zl303xx_DpllCurRefFailStatusGet(zl_3034x_zl303xx_params, ZL303XX_DPLL_ID_1, &val));
    *state = (BOOL) val;
    T_DG(TRACE_GRP_SYNC_INTF,"Clock_losx: %d", val);
    return(VTSS_OK);
}
     
vtss_rc zl_3034x_clock_lol_state_get(BOOL         *const state)
{    
    *state = cur_ref_failed;
    T_DG(TRACE_GRP_SYNC_INTF,"Clock_lol_state: %d", cur_ref_failed);
    return(VTSS_OK);
}

vtss_rc zl_3034x_clock_dhold_state_get(BOOL         *const state)
{
    zl303xx_DpllStatusS par;
    par.Id = ZL303XX_DPLL_ID_1;
    ZL_3034X_CHECK(zl303xx_DpllStatusGet(zl_3034x_zl303xx_params, &par));
    if (par.locked == ZL303XX_DPLL_LOCK_FALSE && par.holdover == ZL303XX_DPLL_HOLD_FALSE && par.refFailed == ZL303XX_FALSE) {
        *state = FALSE;
    } else {
        *state = TRUE;
    }
    return(VTSS_OK);
}


void zl_3034x_clock_event_poll(BOOL interrupt,  clock_event_type_t *ev_mask)
{
    zl303xx_RefIsrStatusS ref_par;
    zl303xx_DpllStatusS status_par;
    status_par.Id = ZL303XX_DPLL_ID_1;
    static clock_event_type_t old_mask = 0x0;
    clock_event_type_t new_mask = 0x0;
    uint source;
    *ev_mask = 0;

    for (source = 0; source < clock_my_input_max; ++source) {
        ref_par.Id = clock_input2ref_id(source);
        ZL_3034X_CHECK(zl303xx_RefIsrStatusGet(zl_3034x_zl303xx_params, &ref_par));
        if (ref_par.refFail != cur_locs_failed[source]) {
            *ev_mask |= CLOCK_LOCS1_EV | CLOCK_LOCS2_EV;
            cur_locs_failed[source] = ref_par.refFail;
        }
        T_IG(TRACE_GRP_SYNC_INTF,"ref %d, refFail %d, scmFail %d, cfmFail %d, pfmFail %d, gstFail %d", ref_par.Id, ref_par.refFail, ref_par.scmFail, ref_par.cfmFail, ref_par.pfmFail, ref_par.gstFail);
    }            
    ZL_3034X_CHECK(zl303xx_DpllStatusGet(zl_3034x_zl303xx_params, &status_par));
    if (status_par.refFailed != cur_ref_failed || status_par.holdover != cur_holdover) {
        *ev_mask |= CLOCK_LOL_EV;
        cur_ref_failed = status_par.refFailed;
        cur_holdover = status_par.holdover;
    }
    T_IG(TRACE_GRP_SYNC_INTF,"locked %d, holdover %d, refFailed %d", status_par.locked, status_par.holdover, status_par.refFailed);
    T_NG(TRACE_GRP_SYNC_INTF,"old_ev: %x, cur_ev %x", old_mask, new_mask);
    
    T_NG(TRACE_GRP_SYNC_INTF,"interrupt: %d, ev_mask %x", interrupt, *ev_mask);
}

void zl_3034x_clock_event_enable(clock_event_type_t ev_mask)
{
    zl303xx_DpllIsrConfigS dpll_par;
    zl303xx_RefIsrConfigS ref_par;
    uint source;
    
    T_DG(TRACE_GRP_SYNC_INTF,"enable ev_mask %x", ev_mask);
    return;
    for (source = 0; source < clock_my_input_max; ++source) {
        ref_par.Id = clock_input2ref_id(source);
    /* pr reference events */
        ZL_3034X_CHECK(zl303xx_RefIsrConfigGet(zl_3034x_zl303xx_params, &ref_par));
        if (ev_mask & (CLOCK_LOCS1_EV | CLOCK_LOCS2_EV)) {
            ref_par.refIsrEn = ZL303XX_TRUE;
            ref_par.scmIsrEn = ZL303XX_TRUE;
            ref_par.cfmIsrEn = ZL303XX_TRUE;
            ref_par.pfmIsrEn = ZL303XX_TRUE;
            ref_par.gstIsrEn = ZL303XX_TRUE;
            /* Inputs to the GST */
            ref_par.gstScmIsrEn = ZL303XX_TRUE;
            ref_par.gstCfmIsrEn = ZL303XX_TRUE;
        }
        if (ev_mask & (CLOCK_FOS1_EV | CLOCK_FOS1_EV)) {
            /* TBD */
        }
        ZL_3034X_CHECK(zl303xx_RefIsrConfigSet(zl_3034x_zl303xx_params, &ref_par));
    }

    /* pr DPLL events */
    dpll_par.Id = ZL303XX_DPLL_ID_1;
    ZL_3034X_CHECK(zl303xx_DpllIsrConfigGet(zl_3034x_zl303xx_params, &dpll_par));
    if (ev_mask & CLOCK_LOSX_EV) {
        dpll_par.lockIsrEn = ZL303XX_TRUE;
    }
    if (ev_mask & CLOCK_LOL_EV) {
        dpll_par.lostLockIsrEn = ZL303XX_TRUE;
    }
    ZL_3034X_CHECK(zl303xx_DpllIsrConfigSet(zl_3034x_zl303xx_params, &dpll_par));
   
}

vtss_rc zl_3034x_station_clk_out_freq_set(const u32 freq_khz)
{    
    Uint32T reg_val;

    ZL_3034X_CHECK(zl303xx_ClosestSynthClk0FreqReg(freq_khz, &reg_val));
    ZL_3034X_CHECK(zl303xx_SynthFrequencyMultSet(zl_3034x_zl303xx_params,
                                            ZL303XX_SYNTH_ID_P0, reg_val));
    T_IG(TRACE_GRP_SYNC_INTF,"Station clock out freq: %d kHz", freq_khz);
    return(VTSS_OK);
}

vtss_rc zl_3034x_station_clk_in_freq_set(const u32 freq_khz)
{    
    zl303xx_RefConfigS ref_cfg;
    ref_cfg.Id = clock_input2ref_id(STATION_CLOCK_SOURCE_NO);
    ref_cfg.invert = ZL303XX_FALSE;
    ref_cfg.prescaler = ZL303XX_REF_DIV_1;
    ref_cfg.oorLimit = ZL303XX_OOR_52_67PPM;
    switch (freq_khz) {
        case 1544:          /* Autodetect clock input frequency */
        case 2048:          /* Autodetect clock input frequency */
            /* initialize reference 7 for autodetect */
            ref_cfg.mode = ZL303XX_REF_MODE_AUTO;
            ZL_3034X_CHECK(zl303xx_RefConfigSet(zl_3034x_zl303xx_params, &ref_cfg));
            T_IG(TRACE_GRP_SYNC_INTF,"clock input: Enable ref 7 for 10 MHz");
            break;
        case 10000:         /* use custB_mult clock input frequency */
            /* initialize reference 7 for 10 MHZ input */
            ref_cfg.mode = ZL303XX_REF_MODE_CUSTB;  /* MODE CUSTB must be set to 10 MHz (see zl_3034x_clock_startup) */
            ZL_3034X_CHECK(zl303xx_RefConfigSet(zl_3034x_zl303xx_params, &ref_cfg));
            T_IG(TRACE_GRP_SYNC_INTF,"clock input: Enable ref 7 for 10 MHz");
            break;
        default:            /* disable clock input */
            ref_cfg.mode = ZL303XX_REF_MODE_AUTO;
            ZL_3034X_CHECK(zl303xx_RefConfigSet(zl_3034x_zl303xx_params, &ref_cfg));
            T_IG(TRACE_GRP_SYNC_INTF,"clock input: disable ?");
            break;
    }
    return(VTSS_OK);
}

vtss_rc zl_3034x_eec_option_set(const clock_eec_option_t clock_eec_option)
{    

    /* set DPLL bandwidth to 0,1 HZ (EEC2) or 3.5 Hz (EEC1 option) */
    ZL_3034X_CHECK(zl303xx_DpllBandwidthSet(zl_3034x_zl303xx_params, ZL303XX_DPLL_ID_1, 
                                            clock_eec_option == CLOCK_EEC_OPTION_1 ? ZL303XX_DPLL_BW_3P5Hz : ZL303XX_DPLL_BW_P1Hz));
    T_IG(TRACE_GRP_SYNC_INTF,"clock_eec_option %d", clock_eec_option);
    return(VTSS_OK);
}
