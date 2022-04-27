/*

 Vitesse Switch API software.

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

#ifndef _VTSS_TOD_MOD_MAN_H_
#define _VTSS_TOD_MOD_MAN_H_
#include "vtss_types.h"
#include "vtss_phy_ts_api.h"
#include "vtss_ts_api.h"

/**
 * \brief Module manager thread trigger upon the 1 PPS from the master timecounter.
 *
 * \param ongoing_adj [IN]  TRUE if the master counter setting is ongoing
 *
 * \return Return code.
 **/
vtss_rc ptp_module_man_trig(BOOL ongoing_adj);

/**
 * \brief Module manager initialization, i.e. create task and initialize internal tables.
 *
 * \return Return code.
 **/
vtss_rc ptp_module_man_init(void);

/**
 * \brief Module manager resume, i.e. resume task.
 *
 * \return Return code.
 **/
vtss_rc ptp_module_man_resume(void);

/**
 * \brief Module manager rate adjustment, i.e. adjust the clock rate for all slave timecounters.
 * \note this function is used when all slave timecounters have the same reference clock.
 *
 * \param adj [IN]  Clock ratio frequency offset in units of scaled ppb
 *                         (parts pr billion). ratio > 0 => clock runs faster. 
 *
 * \return Return code.
 **/
vtss_rc ptp_module_man_rateadj(vtss_phy_ts_scaled_ppb_t *adj);

/* forward declaration */
typedef enum  {
    VTSS_TS_TIMECOUNTER_PHY,
    VTSS_TS_TIMECOUNTER_JR1,
    VTSS_TS_TIMECOUNTER_L26,
} vtss_ts_timecounter_type_t;

/**
 * \brief Module manager enable/disable a slave timecounter.
 *
 * \param port_no [IN]  Port number to Enable or Disable.
 * \param enable  [IN]  TRUE => enable, FALSE => disable 
 *
 * \return Return code.
 **/
vtss_rc ptp_module_man_time_slave_timecounter_enable_disable(vtss_port_no_t port_no, BOOL enable);


/**
 * \brief Module manager add an signature entry.
 *
 * \param alloc_parm [IN]  Timestamping allocation parameters.
 * \param ts_sig     [IN]  Timestamping signature 
 *
 * \return Return code.
 **/
vtss_rc vtss_module_man_tx_timestamp_sig_alloc(const vtss_ts_timestamp_alloc_t *const alloc_parm,
                                        const vtss_phy_ts_fifo_sig_t    *const ts_sig);

/*
 * in_sync state callback
 */
typedef void (*vtss_module_man_in_sync_cb_t)(vtss_port_no_t port_no, /* Port */
                                            BOOL in_sync);  /* true if port is in sync   */

/**
 * \brief Module manager add a in-sync state callback.
 *
 * \param in_sync_cb [IN]  function pointer to a callback function.
 *
 * \return Return code.
 **/
vtss_rc vtss_module_man_tx_timestamp_in_sync_cb_add(const vtss_module_man_in_sync_cb_t in_sync_cb);
vtss_rc vtss_module_man_tx_timestamp_in_sync_cb_delete(const vtss_module_man_in_sync_cb_t in_sync_cb);

/*
 * Timestamp PHY analyzer topology.
 */
typedef enum {
    VTSS_PTP_TS_PTS,       /* port has PHY timestamp feature */
    VTSS_PTP_TS_PTS_G2,    /* port has PHY Gen 2 timestamp feature */
    VTSS_PTP_TS_JTS,       /* port has Jaguar timestamp feature  */
    VTSS_PTP_TS_CTS,       /* port has Caracal (L26) timestamp feature  */
    VTSS_PTP_TS_NONE,      /* port has no timestamp feature  */
} vtss_ptp_timestamp_feature_t;

/*
 * Timestamp PHY analyzer Generation.
 */
typedef enum {
    VTSS_PTP_TS_GEN_1,     /* port has PHY timestamp feature generation 1*/
    VTSS_PTP_TS_GEN_2,     /* port has PHY timestamp feature generation 2*/
    VTSS_PTP_TS_GEN_NONE,  /* port has no timestamp feature  */
} vtss_ptp_timestamp_generation_t;

typedef struct vtss_tod_ts_phy_topo_t  {
    vtss_ptp_timestamp_feature_t ts_feature;    /* indicates pr port which timestamp feature is available */
    vtss_ptp_timestamp_generation_t ts_gen;     /* indicates which generation timestamp feature is available */
    u16 channel_id;                             /* identifies the channel id in the PHY
                                                   (needed to access the timestamping feature) */
    BOOL port_shared;                           /* port shares engine with another port */
    vtss_port_no_t shared_port_no;              /* the port this engine is shared with. */
    BOOL port_ts_in_sync;                       /* false if the port has PHY timestamper, 
                                                    and timestamper is not in sync with master timer, otherwise TRUE */
} vtss_tod_ts_phy_topo_t ;

void tod_ts_phy_topo_get(vtss_port_no_t port_no,
                         vtss_tod_ts_phy_topo_t *phy);

/**
 * \brief Module manager set 1PPS output latency.
 *
 * \param latency [IN]  The latency of the master 1PPS output compared to the start of a one sec period.
 * \return Return code.
 **/
vtss_rc vtss_module_man_master_1pps_latency(const vtss_timeinterval_t latency);































#endif // _VTSS_TOD_MOD_MAN_H_


// ***************************************************************************
//
//  End of file.
//
// ***************************************************************************
