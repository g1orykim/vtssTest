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

#include "vtss_xxrp_timers.h"
#include "vtss_xxrp_mad.h"
#include "vtss_xxrp_callout.h"
#include "vtss_xxrp_os.h"

void vtss_xxrp_start_join_timer(vtss_mrp_mad_t *mad, BOOL restart)
{
    if (mad != NULL) {
        if ((mad->join_timer_running == FALSE) || ((mad->join_timer_running == TRUE) && restart)) {
            T_D("Starting join timer");
            /* TODO: Randomize the join timer (0 to join_time) also Point2Point support */
            mad->join_timer_count = mad->join_timeout;
            mad->join_timer_running = TRUE;
            mad->join_timer_kick = TRUE;
            vtss_mrp_timer_kick();
        }
    }
}
void vtss_xxrp_start_leave_timer(vtss_mrp_mad_t *mad, u16 indx, BOOL restart)
{
    u8 temp;
    if (mad != NULL) {
        temp = VTSS_XXRP_IS_LEAVE_TIMER_RUNNING(mad, indx);
        if ((temp == 0) || ((temp == 1) && restart)) {
            T_D("Starting leave timer");
            mad->leave_timer_count[indx] = mad->leave_timeout;
            VTSS_XXRP_START_LEAVE_TIMER(mad, indx);
            mad->leave_timer_running[indx] = TRUE;
            VTSS_XXRP_SET_LEAVE_TIMER_KICK(mad, indx);
            vtss_mrp_timer_kick();
        }
    }
}
void vtss_xxrp_start_leaveall_timer(vtss_mrp_mad_t *mad, BOOL restart)
{
    if (mad != NULL) {
        if ((!mad->leaveall_timer_running) || (mad->leaveall_timer_running && restart)) {
            T_D("Starting LA timer");
            /* TODO: Randomize the LA timer 1 to 1.5 times of LA timer */
            mad->leaveall_timer_count = mad->leaveall_timeout;
            mad->leaveall_timer_running = TRUE;
            mad->leaveall_timer_kick = TRUE;
            vtss_mrp_timer_kick();
        }
    }
}
void vtss_xxrp_start_periodic_timer(vtss_mrp_mad_t *mad, BOOL restart)
{
    if (mad != NULL) {
        if ((!mad->periodic_timer_running) || (mad->periodic_timer_running && restart)) {
            T_D("Starting periodic timer");
            mad->periodic_timer_count = mad->periodic_tx_timeout;
            mad->periodic_timer_running = TRUE;
            mad->periodic_timer_kick = TRUE;
            vtss_mrp_timer_kick();
        }
    }
}
void vtss_xxrp_stop_join_timer(vtss_mrp_mad_t *mad)
{
    if (mad != NULL) {
        T_D("Stopping join timer");
        mad->join_timer_running = FALSE;
    }
}
void vtss_xxrp_stop_leave_timer(vtss_mrp_mad_t *mad, u16 indx)
{
    if (mad != NULL) {
        T_D("Stopping leave timer");
        VTSS_XXRP_STOP_LEAVE_TIMER(mad, indx);
    }
}
void vtss_xxrp_stop_leaveall_timer(vtss_mrp_mad_t *mad)
{
    if (mad != NULL) {
        T_D("Stopping leaveall timer");
        mad->leaveall_timer_running = FALSE;
    }
}
void vtss_xxrp_stop_periodic_timer(vtss_mrp_mad_t *mad)
{
    if (mad != NULL) {
        T_D("Stopping periodic timer");
        mad->periodic_timer_running = FALSE;
    }
}
