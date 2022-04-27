/*

   Vitesse Switch API software.

 Copyright (c) 2002-2012 Vitesse Semiconductor Corporation "Vitesse". All
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

#ifndef _VTSS_XXRP_TIMERS_H_
#define _VTSS_XXRP_TIMERS_H_
#include "vtss_xxrp_types.h"

#if 0
#define VTSS_XXRP_DEFAULT_JOIN_TIMER                  20
#define VTSS_XXRP_DEFAULT_LEAVE_TIMER                 60
#define VTSS_XXRP_DEFAULT_LEAVEALL_TIMER              1000
#define VTSS_XXRP_DEFAULT_PERIODIC_TIMER              100  /* This cannot be configured */
#define VTSS_XXRP_LEAVE_TIMER_MIN                     600
#define VTSS_XXRP_LEAVE_TIMER_MAX                     1000
#endif
#define VTSS_XXRP_IS_LEAVE_TIMER_RUNNING(mad, indx)   (mad->leave_timer_running[indx / 8] & (1U << (indx % 8)))
#define VTSS_XXRP_START_LEAVE_TIMER(mad, indx)        (mad->leave_timer_running[indx / 8] |= (1U << (indx % 8)))
#define VTSS_XXRP_STOP_LEAVE_TIMER(mad, indx)         (mad->leave_timer_running[indx / 8] &= ~(1U << (indx % 8)))
#define VTSS_XXRP_IS_LEAVE_TIMER_KICK(mad, indx)      (mad->leave_timer_kick[indx / 8] & (1U << (indx % 8)))
#define VTSS_XXRP_SET_LEAVE_TIMER_KICK(mad, indx)     (mad->leave_timer_kick[indx / 8] |= (1U << (indx % 8)))
#define VTSS_XXRP_CLEAR_LEAVE_TIMER_KICK(mad, indx)   (mad->leave_timer_kick[indx / 8] &= ~(1U << (indx % 8)))

void vtss_xxrp_start_join_timer(vtss_mrp_mad_t *mad, BOOL restart);
void vtss_xxrp_start_leave_timer(vtss_mrp_mad_t *mad, u16 index, BOOL restart);
void vtss_xxrp_start_leaveall_timer(vtss_mrp_mad_t *mad, BOOL restart);
void vtss_xxrp_start_periodic_timer(vtss_mrp_mad_t *mad, BOOL restart);
void vtss_xxrp_stop_join_timer(vtss_mrp_mad_t *mad);
void vtss_xxrp_stop_leave_timer(vtss_mrp_mad_t *mad, u16 index);
void vtss_xxrp_stop_leaveall_timer(vtss_mrp_mad_t *mad);
void vtss_xxrp_stop_periodic_timer(vtss_mrp_mad_t *mad);
#endif /* _VTSS_XXRP_TIMERS_H_ */
