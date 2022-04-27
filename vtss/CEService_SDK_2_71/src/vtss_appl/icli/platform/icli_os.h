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
    > CP.Wang, 06/19/2013 15:58
        - create

==============================================================================
*/
#ifndef __VTSS_ICLI_OS_H__
#define __VTSS_ICLI_OS_H__
//****************************************************************************
/*
==============================================================================

    Include File

==============================================================================
*/

/*
==============================================================================

    Constant

==============================================================================
*/

/*
==============================================================================

    Type

==============================================================================
*/
typedef i32 icli_thread_entry_cb_t(
    IN i32      session_id
);

typedef enum {
    ICLI_THREAD_PRIORITY_NORMAL,
    ICLI_THREAD_PRIORITY_HIGH,
} icli_thread_priority_t;

/*
==============================================================================

    Macro Definition

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
);

/*
    get my thread ID
*/
u32 icli_thread_id_get(
    void
);

/*
    sleep in milli-seconds
*/
void icli_sleep(
    IN u32  t
);

#endif // ICLI_TARGET

/*
    get the time elapsed from system start in milli-seconds
*/
u32 icli_current_time_get(
    void
);

//****************************************************************************
#endif //__VTSS_ICLI_OS_H__

