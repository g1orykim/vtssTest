/*

 Vitesse Switch API software.

 Copyright (c) 2002-2009 Vitesse Semiconductor Corporation "Vitesse". All
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

#ifndef _ZL_3034X_PDV_API_H_
#define _ZL_3034X_PDV_API_H_

#include "main.h"
#include "vtss_types.h"
//#include "zl_3034x_pdv.h"


vtss_rc zl_3034x_pdv_init(vtss_init_data_t *data);

/* create the PDV filter instance */
vtss_rc zl_3034x_pdv_create(BOOL enable);


/**
 * \brief Process timestamps received in the PTP protocol.
 *
 * \param tx_ts [IN]  transmitted timestamp
 * \param rx_ts [IN]  received timestamp
 * \param corr  [IN]  received correction field
 * \param fwd_path [IN] true if forward path (Sync), false if reverse path (Delay_Req/Delat_Resp).
 * \return TRUE if success
 */
typedef struct {
    vtss_timestamp_t tx_ts;
    vtss_timestamp_t rx_ts;
    vtss_timeinterval_t corr;
    BOOL fwd_path;
} vtss_zl_3034x_process_timestamp_t;
BOOL  zl_3034x_process_timestamp(vtss_zl_3034x_process_timestamp_t *ts);

/**
 * \brief Process packet rate indications received in the PTP protocol.
 *
 * \param ptp_rate [IN]  ptp packet rate = 2**-ptp_rate packets pe sec, i.e. ptp_rate = -7 => 128 packets pr sec.
 * \param forward  [IN]  True if forward rate (Sync packets), False if reverse rate (Delay_Req)
 * \return TRUE if success
 */
BOOL  zl_3034x_packet_rate_set(i8 ptp_rate, BOOL forward);

#define ZL_3034X_PDV_INITIAL           0
#define ZL_3034X_PDV_FREQ_LOCKING      1
#define ZL_3034X_PDV_PHASE_LOCKING     2
#define ZL_3034X_PDV_FREQ_LOCKED       3
#define ZL_3034X_PDV_PHASE_LOCKED      4
#define ZL_3034X_PDV_PHASE_HOLDOVER    5
BOOL zl_3034x_pdv_status_get(u32 *pdv_clock_state, i32 *freq_offset);


#define ZL_TOP_ISR_REF_TS_ENG         0x00000001  /* DCO Phase Word and Local System Time have been sampled */

typedef u32 zl_3034x_event_t;

vtss_rc zl_3034x_event_enable_set(zl_3034x_event_t events,  BOOL enable);

vtss_rc zl_3034x_event_poll(zl_3034x_event_t *const events);


vtss_rc zl_3034x_apr_device_status_get(void);
vtss_rc zl_3034x_apr_server_config_get(void);
vtss_rc zl_3034x_apr_server_status_get(void);
vtss_rc zl_3034x_apr_force_holdover_set(BOOL enable);
vtss_rc zl_3034x_apr_force_holdover_get(BOOL *enable);
vtss_rc zl_3034x_apr_statistics_get(void);
vtss_rc zl_3034x_apr_statistics_reset(void);
vtss_rc apr_server_notify_set(BOOL enable);
vtss_rc apr_server_notify_get(BOOL *enable);
vtss_rc zl_3034x_apr_log_level_set(u8 level);
vtss_rc zl_3034x_apr_log_level_get(u8 *level);
vtss_rc zl_3034x_apr_ts_log_level_set(u8 level);
vtss_rc zl_3034x_apr_ts_log_level_get(u8 *level);
vtss_rc apr_server_one_hz_set(BOOL enable);
vtss_rc apr_server_one_hz_get(BOOL *enable);
vtss_rc apr_config_parameters_set(u32 mode);
vtss_rc apr_config_parameters_get(u32 *mode, char *zl_config);
vtss_rc apr_adj_min_set(u32 adj);
vtss_rc apr_adj_min_get(u32 *adj);
vtss_rc zl_3034x_apr_config_dump(void);


#endif // _ZL_3034X_PDV_API_H_


// ***************************************************************************
//
//  End of file.
//
// ***************************************************************************
