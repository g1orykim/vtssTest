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

#ifndef __EEE_H__
#define __EEE_H__

#include "port_api.h" // For port_info_t
#include "eee_api.h"  // for eee_switch_conf_t

//************************************************
// Trace definitions
//************************************************
#include <vtss_module_id.h>
#include <vtss_trace_lvl_api.h>
#include <vtss_trace_api.h>
#include "critd_api.h"

#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_EEE
#define VTSS_TRACE_GRP_DEFAULT 0
#define TRACE_GRP_CRIT         1
#define TRACE_GRP_IRQ          2
#define VTSS_TRACE_GRP_CLI     3
#define TRACE_GRP_CNT          4

// Global variables shared amongst platform code
extern critd_t            EEE_crit;
extern eee_switch_conf_t  EEE_local_conf;
extern eee_switch_global_conf_t  EEE_global_conf;
extern eee_switch_state_t EEE_local_state;

#if VTSS_TRACE_ENABLED
#define EEE_CRIT_ENTER()         critd_enter        (&EEE_crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define EEE_CRIT_EXIT()          critd_exit         (&EEE_crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define EEE_CRIT_ASSERT_LOCKED() critd_assert_locked(&EEE_crit, TRACE_GRP_CRIT,                       __FILE__, __LINE__)
#else /* !VTSS_TRACE_ENABLED */
#define EEE_CRIT_ENTER()         critd_enter        (&EEE_crit)
#define EEE_CRIT_EXIT()          critd_exit         (&EEE_crit)
#define EEE_CRIT_ASSERT_LOCKED() critd_assert_locked(&EEE_crit)
#endif /* !VTSS_TRACE_ENABLED */


// Default configuration settings
#define OPTIMIZE_FOR_POWER_DEFAULT FALSE

/*
 * Functions to be implemented by chip-specific C-file.
 * These functions are not really public to others than
 * eee.c.
 */
void EEE_platform_port_link_change_event(vtss_port_no_t iport, port_info_t *info);
void EEE_platform_thread(void);
void EEE_platform_conf_reset(eee_switch_conf_t *conf);
BOOL EEE_platform_conf_valid(eee_switch_conf_t *conf);
void EEE_platform_tx_wakeup_time_changed(vtss_port_no_t port, u16 LocResolvedTxSystemValue);
void EEE_platform_local_conf_set(BOOL *port_changes);
void EEE_platform_init(vtss_init_data_t *data);
#endif /* __EEE_H__ */

