/*

 Vitesse Switch Software.

 Copyright (c) 2002-2010 Vitesse Semiconductor Corporation "Vitesse". All
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
/* vtss_ptp_sys_timer.h */

#ifndef VTSS_PTP_SYS_TIMER_H
#define VTSS_PTP_SYS_TIMER_H

#include "vtss_ptp_types.h"

/**
 * \new timer implementation
 */

typedef struct vtss_ptp_sys_timer_s *vtss_timer_handle_t;

typedef void (*vtss_timer_callout)(vtss_timer_handle_t timer, void *context);

typedef struct vtss_timer_head_s{
	struct  vtss_timer_head_s *next;
	struct  vtss_timer_head_s *prev;
} vtss_timer_head_t;

typedef struct vtss_ptp_sys_timer_s {
	vtss_timer_head_t head;
	i32  period;
	i32  left;
	vtss_timer_callout co;
    void *context;
    BOOL periodic;
} vtss_ptp_sys_timer_t;


void vtss_ptp_timer_start(vtss_ptp_sys_timer_t *t, i32 period, BOOL repeat);
void vtss_ptp_timer_stop(vtss_ptp_sys_timer_t *t);

void vtss_ptp_timer_tick(i32 my_time);

void vtss_init_ptp_timer(vtss_ptp_sys_timer_t *t, vtss_timer_callout co, void *context);

void vtss_ptp_timer_initialize(void);



#endif

