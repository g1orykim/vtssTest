/*

 Vitesse Switch Software.

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
/* Implement the 1pps synchronization feature */
/*lint -esym(766, vtss_options.h) */

#include "vtss_options.h"
#if defined(VTSS_FEATURE_PHY_TIMESTAMP)

#include "main.h"
#include "ptp.h"
#include "ptp_1pps_closed_loop.h"
#include "ptp_api.h"
#include "vtss_ptp_sys_timer.h"
#include "vtss_tod_mod_man.h"
#include "misc_api.h"

#include "critd_api.h"

#ifdef VTSS_SW_OPTION_PHY
#include "phy_api.h"
#endif
#include "vtss_phy_ts_api.h"


#define API_INST_DEFAULT PHY_INST
#define PTP_1PPS_DEFAULT_PPS_WIDTH 1000
#define PTP_1PPS_DEFAULT_PPS_OFFSET 0           /* No PPS output delay pr. default */

#define VTSS_PTP_CLOSED_LOOP_MEASURE_TIME 10    /* the number of 1 sec periods for measuring the ri_time */
#define VTSS_PTP_CLOSED_LOOP_SETTLE_TIME   4    /* the number of 1 sec periods to pause the measurement after setting the to_main */

#define VTSS_PTP_CLOSED_LOOP_TIMEINTERVAL 10    /* unit is 1/128 SEC, the polling time should be  < 200 ms */

#define PPS_GEN_CNT 200000000                   /* The pulse is delayed 0.2 sec in the slave, i.e. when the Tinp is detected,
                                                   the Tsave has been saved 0.1 sec before, and we have 0.9 sec to read it */
#define PHY_ONE_CLOCK_CYCLE 8

/*
 * Closed loop master mode.
 */
typedef enum {
    VTSS_PTP_CLOSED_LOOP_MASTER_PASSIVE,    /* Statemachine is passive */
    VTSS_PTP_CLOSED_LOOP_MASTER_MEASURE,    /* Main output mode measuring cable delay */
    VTSS_PTP_CLOSED_LOOP_MASTER_SETTLE,     /* Main output mode settlng after slave phase adjustment */
} closed_loop_master_state_t;


/* closed loop phy port configuration data
 */
typedef struct {
    vtss_1pps_closed_loop_conf_t conf;
    /* private data */
    closed_loop_master_state_t  my_state;
    vtss_port_no_t master_port;
    u32 repeat;
    u32 t_ref;
    i64 t_save;     /* accumulated Tsave value from the slave port for averaging*/
    i64 rtd;        /* accumulated round trip delay value for averaging */
    /* timer for transmission of Sync messages*/
    vtss_ptp_sys_timer_t pps_timer;
    vtss_port_no_t port;
    vtss_phy_timestamp_t t_inp;
} one_pps_closed_loop_entry_t;

static one_pps_closed_loop_entry_t one_pps_closed_loop_data[PTP_CLOCK_PORTS];

#define PTP_CLOSED_LOOP_LOCK()        critd_enter(&datamutex, VTSS_TRACE_GRP_DEFAULT, LOCK_TRACE_LEVEL, __FILE__, __LINE__)
#define PTP_CLOSED_LOOP_UNLOCK()      critd_exit (&datamutex, VTSS_TRACE_GRP_DEFAULT, LOCK_TRACE_LEVEL, __FILE__, __LINE__)
static critd_t datamutex;          /* Global data protection */

static void ptp_1pps_closed_loop_timer(vtss_timer_handle_t timer, void *t);


// Convert state to text
static char *state_txt(closed_loop_master_state_t s)
{
    switch (s)
    { 
        case VTSS_PTP_CLOSED_LOOP_MASTER_PASSIVE:       return("PASSIVE");
        case VTSS_PTP_CLOSED_LOOP_MASTER_MEASURE:             return("MEASURE");
        case VTSS_PTP_CLOSED_LOOP_MASTER_SETTLE:              return("SETTLE");
        default:                                return("Unknown state");
    }
}


/******************************************************************************/
// ptp_1pps_closed_loop_init()
// Initialize the internal data structure for handling the 1pps delay measurement feature
// 
/******************************************************************************/
vtss_rc ptp_1pps_closed_loop_init(void)
{
    vtss_rc rc = VTSS_RC_OK;
    vtss_port_no_t port_no;
    memset(one_pps_closed_loop_data, 0, sizeof(one_pps_closed_loop_entry_t)*PTP_CLOCK_PORTS);
    for (port_no = 0; port_no < PTP_CLOCK_PORTS; port_no++) {
        one_pps_closed_loop_data[port_no].conf.mode = VTSS_PTP_1PPS_CLOSED_LOOP_DISABLE;
    }
    critd_init(&datamutex, "1pps_closed_loop", VTSS_MODULE_ID_TOD, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
    PTP_CLOSED_LOOP_UNLOCK();
    return rc;
}

vtss_rc ptp_1pps_closed_loop_mode_get(vtss_port_no_t port_no, vtss_1pps_closed_loop_conf_t *conf)
{
    vtss_rc rc = VTSS_RC_OK;
    PTP_CLOSED_LOOP_LOCK();
    *conf = one_pps_closed_loop_data[port_no].conf;
    PTP_CLOSED_LOOP_UNLOCK();
    return rc;
}
/******************************************************************************/
// ptp_1pps_closed_loop_mode_set()
// Set the mode for a 1PPS delay measurement on a PHY Gen2 port
// If the feature is supported by  port, then the statemachine is enabled
// Else return an error code
// If mode == MAIN_MAN State = OUTPUT_ONLY
// If mode == MAIN_AUTO State = MEASURE
// If mode == SUB State = SLAVE
// 
/******************************************************************************/
vtss_rc ptp_1pps_closed_loop_mode_set(vtss_port_no_t port_no, const vtss_1pps_closed_loop_conf_t *conf)
{
    vtss_rc rc = VTSS_RC_OK;
    vtss_tod_ts_phy_topo_t phy_topo;
    tod_ts_phy_topo_get(port_no, &phy_topo);
    vtss_phy_ts_alt_clock_mode_t ms_phy_alt_clock_mode;
    vtss_phy_ts_alt_clock_mode_t sl_phy_alt_clock_mode;
    vtss_phy_ts_pps_conf_t ms_phy_pps_config;
    vtss_phy_ts_pps_conf_t sl_phy_pps_config;
    one_pps_closed_loop_entry_t *sync_entry;
    u16 cd;                 /* cable delay in the format the API wants it */
    PTP_CLOSED_LOOP_LOCK();
    T_IG(VTSS_TRACE_GRP_PHY_1PPS,"port_no %d, mode %d, master_port %d, cable_delay %d ", iport2uport(port_no), conf->mode, iport2uport(conf->master_port), conf->cable_delay);
    if (phy_topo.ts_gen == VTSS_PTP_TS_GEN_2) {
        if (conf->mode == VTSS_PTP_1PPS_CLOSED_LOOP_AUTO) {
            tod_ts_phy_topo_get(conf->master_port, &phy_topo);
            if (phy_topo.ts_gen != VTSS_PTP_TS_GEN_2) {
                rc = PTP_RC_MISSING_PHY_TIMESTAMP_RESOURCE;
            }
        }
        if (rc == VTSS_RC_OK) {
            sync_entry = &one_pps_closed_loop_data[port_no];
            sync_entry->conf = *conf;
            /* default loopback conf: all disabled */
            ms_phy_alt_clock_mode.pps_ls_lpbk = FALSE;
            ms_phy_alt_clock_mode.ls_lpbk = FALSE;
            ms_phy_alt_clock_mode.ls_pps_lpbk = FALSE;
            sl_phy_alt_clock_mode.pps_ls_lpbk = FALSE;
            sl_phy_alt_clock_mode.ls_lpbk = FALSE;
            sl_phy_alt_clock_mode.ls_pps_lpbk = FALSE;
            /* configure Tref (the 1pps output phase for master port) */
            ms_phy_pps_config.pps_offset = PTP_1PPS_DEFAULT_PPS_OFFSET;
            ms_phy_pps_config.pps_width_adj = ms_phy_pps_config.pps_offset + PTP_1PPS_DEFAULT_PPS_WIDTH;
            sync_entry->t_ref = PTP_1PPS_DEFAULT_PPS_OFFSET;
            /* set the PPS_GEN_CNT in the slave port to 0.1 sec */
            sl_phy_pps_config.pps_offset = PTP_1PPS_DEFAULT_PPS_OFFSET;
            sl_phy_pps_config.pps_width_adj = sl_phy_pps_config.pps_offset + PTP_1PPS_DEFAULT_PPS_WIDTH;

            sync_entry = &one_pps_closed_loop_data[port_no];
            sync_entry->port = port_no;
            /* initialize timer  */
            vtss_init_ptp_timer(&sync_entry->pps_timer, ptp_1pps_closed_loop_timer, sync_entry);
            
            switch (conf->mode) {
                case VTSS_PTP_1PPS_CLOSED_LOOP_MAN:
                case VTSS_PTP_1PPS_CLOSED_LOOP_DISABLE:
                    if(sync_entry->my_state != VTSS_PTP_CLOSED_LOOP_MASTER_PASSIVE) {
                        /* reset conf for previously used master port */
                        if (VTSS_RC_OK == rc) rc = vtss_phy_ts_pps_conf_set(API_INST_DEFAULT, sync_entry->master_port, &ms_phy_pps_config);
                        if (VTSS_RC_OK == rc) rc = vtss_phy_ts_alt_clock_mode_set(API_INST_DEFAULT, sync_entry->master_port, &ms_phy_alt_clock_mode);
                    }
                    sync_entry->my_state = VTSS_PTP_CLOSED_LOOP_MASTER_PASSIVE;
                    /* Set LOAD_PULSE_DELAY in the slave port */
                    cd = (u16)conf->cable_delay;
                    rc = vtss_phy_ts_loadpulse_delay_set(API_INST_DEFAULT, port_no, &cd);
                    break;
                case VTSS_PTP_1PPS_CLOSED_LOOP_AUTO:
                    sync_entry->master_port = conf->master_port;
                    sync_entry->my_state = VTSS_PTP_CLOSED_LOOP_MASTER_MEASURE;
                    vtss_ptp_timer_start(&sync_entry->pps_timer, VTSS_PTP_CLOSED_LOOP_TIMEINTERVAL, TRUE);
                    /* set up HW configuration */
                    ms_phy_alt_clock_mode.pps_ls_lpbk = TRUE;
                    sl_phy_alt_clock_mode.ls_lpbk = TRUE;
                    T_DG(VTSS_TRACE_GRP_PHY_1PPS,"port_no %d, master_port %d", iport2uport(port_no), iport2uport(sync_entry->master_port));
                    if (VTSS_RC_OK != (rc = vtss_phy_ts_ptptime_get(API_INST_DEFAULT, conf->master_port, &sync_entry->t_inp))) T_NG(VTSS_TRACE_GRP_PHY_1PPS,"%s", error_txt(rc));
                    PTP_RC(vtss_phy_ts_ptptime_arm(API_INST_DEFAULT, conf->master_port));
                    ms_phy_pps_config.pps_offset = sync_entry->t_ref;
                    ms_phy_pps_config.pps_width_adj = ms_phy_pps_config.pps_offset + PTP_1PPS_DEFAULT_PPS_WIDTH;
                    sl_phy_pps_config.pps_offset = PPS_GEN_CNT;
                    sl_phy_pps_config.pps_width_adj = sl_phy_pps_config.pps_offset + PTP_1PPS_DEFAULT_PPS_WIDTH;
                    sync_entry->t_save = 0LL;
                    sync_entry->rtd = 0LL;
                    break;

                default: rc = PTP_RC_UNSUPPORTED_1PPS_OPERATION_MODE;
                    break;
            }
            if (VTSS_RC_OK == rc) rc = vtss_phy_ts_pps_conf_set(API_INST_DEFAULT, port_no, &sl_phy_pps_config);
            if (conf->mode == VTSS_PTP_1PPS_CLOSED_LOOP_AUTO) {
                if (VTSS_RC_OK == rc) rc = vtss_phy_ts_pps_conf_set(API_INST_DEFAULT, sync_entry->master_port, &ms_phy_pps_config);
                if (VTSS_RC_OK == rc) rc = vtss_phy_ts_alt_clock_mode_set(API_INST_DEFAULT, sync_entry->master_port, &ms_phy_alt_clock_mode);
            }
            if (VTSS_RC_OK == rc) rc = vtss_phy_ts_alt_clock_mode_set(API_INST_DEFAULT, port_no, &sl_phy_alt_clock_mode);
            T_IG(VTSS_TRACE_GRP_PHY_1PPS,"state %s, rc %s", state_txt(sync_entry->my_state), error_txt(rc));
        }
    } else {
        rc = PTP_RC_MISSING_PHY_TIMESTAMP_RESOURCE;
    }
    PTP_CLOSED_LOOP_UNLOCK();
    return rc;
}

/******************************************************************************/
// ptp_1pps_closed_loop_timer()
// Handle the 1PPS closed loop every sec.
// Each function has a statemachine:
// State        Action
// PASSIVE      Do nothing
// MEASURE      read Tinp; calculate rount trip delay; if (repeat++ > 10) {calculate average; update slave port phase; State = SETTLE}
// SETTLE       if (repeat++ > 1) State = MEASURE
//
// 
/******************************************************************************/
/*lint -esym(459, ptp_1pps_closed_loop_timer) */
static void ptp_1pps_closed_loop_timer(vtss_timer_handle_t timer, void *t)
{
    vtss_port_no_t port_no;
    vtss_port_no_t master_port;
    i32 rtd, slave_adj, tsave;
    one_pps_closed_loop_entry_t *sync_entry = (one_pps_closed_loop_entry_t *)t;
    vtss_phy_timestamp_t t_save;
    vtss_phy_timestamp_t t_inp;
    vtss_phy_ts_todadj_status_t onga;
    u16 cd;                 /* cable delay in the format the API wants it */
    static u32 rtd_min, rtd_max;
    u32 rtd_tmp;
    static u32 t_save_min, t_save_max;
    
    PTP_CLOSED_LOOP_LOCK();
    port_no = sync_entry->port;
    master_port = sync_entry->master_port;
    if (sync_entry->my_state != VTSS_PTP_CLOSED_LOOP_MASTER_PASSIVE) {
        /* read Tinp */
        if (VTSS_RC_OK == vtss_phy_ts_ptptime_get(API_INST_DEFAULT, master_port, &t_inp)) {
            T_NG(VTSS_TRACE_GRP_PHY_1PPS,"active");
            PTP_RC(vtss_phy_ts_ptptime_arm(API_INST_DEFAULT, master_port));
            if (t_inp.seconds.low != sync_entry->t_inp.seconds.low && t_inp.nanoseconds >= PPS_GEN_CNT) {
                sync_entry->t_inp = t_inp;
                switch (sync_entry->my_state) {
                    case VTSS_PTP_CLOSED_LOOP_MASTER_MEASURE:              /* Main output mode measuring cable delay */
                        /* read Tsave  */
                        T_DG(VTSS_TRACE_GRP_PHY_1PPS,"master port %d slave port_no %d measure", iport2uport(master_port), iport2uport(port_no));
                        if (VTSS_RC_OK == vtss_phy_ts_ptptime_get(API_INST_DEFAULT, port_no, &t_save)) {
                            T_DG(VTSS_TRACE_GRP_PHY_1PPS,"t_save %d ns, t_inp %d ns", t_save.nanoseconds, t_inp.nanoseconds);
                            sync_entry->t_save += t_save.nanoseconds;
                            rtd_tmp = (sync_entry->t_inp.nanoseconds - sync_entry->t_ref - PPS_GEN_CNT + PHY_ONE_CLOCK_CYCLE);
                            if (sync_entry->repeat == 0) {
                                rtd_min = rtd_tmp;
                                rtd_max = rtd_tmp;
                                t_save_min = t_save.nanoseconds;
                                t_save_max = t_save.nanoseconds;
                            } else {
                                if (rtd_tmp > rtd_max) rtd_max = rtd_tmp;
                                if (rtd_tmp < rtd_min) rtd_min = rtd_tmp;
                                if (t_save.nanoseconds > t_save_max) t_save_min = t_save.nanoseconds;
                                if (t_save.nanoseconds < t_save_min) t_save_max = t_save.nanoseconds;
                            }
                            sync_entry->rtd += rtd_tmp;
                            PTP_RC(vtss_phy_ts_ptptime_arm(API_INST_DEFAULT, port_no));
                            T_DG(VTSS_TRACE_GRP_PHY_1PPS,"accumulated t_save %lld, rtd %lld", sync_entry->t_save, sync_entry->rtd);
                            if (++sync_entry->repeat >= VTSS_PTP_CLOSED_LOOP_MEASURE_TIME) {
                                sync_entry->my_state = VTSS_PTP_CLOSED_LOOP_MASTER_SETTLE;
                                /* calculate average turn around time and average Tsave */
                                rtd = sync_entry->rtd/VTSS_PTP_CLOSED_LOOP_MEASURE_TIME;
                                tsave = sync_entry->t_save/VTSS_PTP_CLOSED_LOOP_MEASURE_TIME;
                                cd = rtd/2;
                                PTP_RC(vtss_phy_ts_loadpulse_delay_set(API_INST_DEFAULT, port_no, &cd));
                                slave_adj = tsave/2;
                                T_IG(VTSS_TRACE_GRP_PHY_1PPS,"rtd %d, tsave %d, slave_adj %d", rtd, tsave, slave_adj);
                                T_IG(VTSS_TRACE_GRP_PHY_1PPS,"statistics: rtd_min %d, rtd_max %d, tsave_min %d, tsave_max %d", rtd_min, rtd_max, t_save_min, t_save_max);
                                PTP_RC(vtss_phy_ts_timeofday_offset_set(API_INST_DEFAULT, port_no, tsave));

                                sync_entry->repeat = 0;
                                sync_entry->my_state = VTSS_PTP_CLOSED_LOOP_MASTER_SETTLE;
                                sync_entry->t_save = 0LL;
                                sync_entry->rtd = 0LL;
                            }
                        }
                        break;
                    case VTSS_PTP_CLOSED_LOOP_MASTER_SETTLE:               /* Main output mode settlng after 1pps output phase adjustment */
                        /* */
                        T_DG(VTSS_TRACE_GRP_PHY_1PPS,"port_no %d settle", iport2uport(port_no));
                        PTP_RC(vtss_phy_ts_ongoing_adjustment(API_INST_DEFAULT, port_no, &onga));
                        if (++sync_entry->repeat >= VTSS_PTP_CLOSED_LOOP_SETTLE_TIME || onga != VTSS_PHY_TS_TODADJ_INPROGRESS) {
                            T_IG(VTSS_TRACE_GRP_PHY_1PPS,"port_no %d end of settle, onga %d, repeat %d", iport2uport(port_no), onga, sync_entry->repeat);
                            sync_entry->repeat = 0;
                            sync_entry->my_state = VTSS_PTP_CLOSED_LOOP_MASTER_MEASURE;
                        }
                        break;
                    default:
                        break;
                }
            } else {
                T_NG(VTSS_TRACE_GRP_PHY_1PPS,"ignoring tinp with  %d ns", t_inp.nanoseconds);
                
            }
        }
    }
    PTP_CLOSED_LOOP_UNLOCK();
}
#endif
