/*

 Vitesse Switch Application software.

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

/*
==============================================================================

    Revision history
    > CP.Wang, 06/19/2013 15:43
        - create

==============================================================================
*/

/*
==============================================================================

    Include File

==============================================================================
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "icli_def.h"
#include "icli_os.h"
#include "icli_porting_trace.h"

/*
==============================================================================

    Constant and Macro

==============================================================================
*/
#ifdef ICLI_TARGET
#define ICLI_THREAD_STACK_SIZE  (16 * 1024)
#endif

/*
==============================================================================

    Type Definition

==============================================================================
*/

/*
==============================================================================

    Static Variable

==============================================================================
*/
#ifdef ICLI_TARGET

/* thread */
static cyg_handle_t g_thread_handle[ICLI_SESSION_CNT];
static cyg_thread   g_thread_block[ICLI_SESSION_CNT];
static char         g_thread_stack[ICLI_SESSION_CNT][ICLI_THREAD_STACK_SIZE];

#endif // ICLI_TARGET

/*
==============================================================================

    Static Function

==============================================================================
*/

/*
==============================================================================

    Public Function

==============================================================================
*/
#ifdef ICLI_TARGET

/*
    create thread
*/
BOOL icli_thread_create(
    IN  i32                     session_id,
    IN  char                    *name,
    IN  icli_thread_priority_t  priority,
    IN  icli_thread_entry_cb_t  *entry_cb,
    IN  i32                     entry_data
)
{
    i32     thread_priority = THREAD_DEFAULT_PRIO;

    if ( priority == ICLI_THREAD_PRIORITY_HIGH ) {
        thread_priority = THREAD_HIGHEST_PRIO;
    }

    cyg_thread_create(  thread_priority,
                        (cyg_thread_entry_t *)entry_cb,
                        (cyg_addrword_t)session_id,
                        name,
                        g_thread_stack[session_id],
                        ICLI_THREAD_STACK_SIZE,
                        &(g_thread_handle[session_id]),
                        &(g_thread_block[session_id]) );

    cyg_thread_resume( g_thread_handle[session_id] );
    return TRUE;
}

/*
    get my thread ID
*/
u32 icli_thread_id_get(
    void
)
{
    return (u32)cyg_thread_self();
}

/*
    sleep in milli-seconds
*/
void icli_sleep(
    IN u32  t
)
{
    VTSS_OS_MSLEEP( t );
}

#endif // ICLI_TARGET

/*
    get the time elapsed from system start in milli-seconds
*/
u32 icli_current_time_get(
    void
)
{
    struct timespec     tp;

    if ( clock_gettime(CLOCK_MONOTONIC, &tp) == -1 ) {
        T_E("failed to get system up time\n");
        return 0;
    }
    return tp.tv_sec * 1000 + tp.tv_nsec / 1000000;
}
