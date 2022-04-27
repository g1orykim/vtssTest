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

$Id$
$Revision$


This file is part of SPROUT - "Stack Protocol using ROUting Technology".
*/

#include "vtss_sprout.h"
#include <string.h>





#define VTSS_SPROUT_CRIT_THREAD_ID_UNKNOWN (0)








#if VTSS_SPROUT_MULTI_THREAD

#if VTSS_TRACE_ENABLED
static ulong vtss_sprout_crit_get_thread_id(void)
{
#if VTSS_OPSYS_ECOS
    return (ulong) cyg_thread_get_id(cyg_thread_self());
#else
    return 0;
#endif
} 



static BOOL vtss_sprout_crit_is_locked(vtss_sprout_crit_t *crit_p)
{
    int cnt = 0;

#ifdef VTSS_OS_SEM_PEEK
    VTSS_OS_SEM_PEEK(&crit_p->sem, &cnt);
#endif
    return (cnt == 0);
} 



static void vtss_sprout_crit_state_trace(
    vtss_sprout_crit_t *crit_p,
    int               trace_grp,
    int               trace_lvl,
    const char        *file,
    const int         line)
{
    int lock_file_start = 0;
    int unlock_file_start = 0;
    int i;


    
    if (crit_p->lock_file) {
        for (i = 0; i < strlen(crit_p->lock_file); i++) {
            if (crit_p->lock_file[i] == '/') {
                lock_file_start = i + 1;
            }
        }
    }
    if (crit_p->unlock_file) {
        for (i = 0; i < strlen(crit_p->unlock_file); i++) {
            if (crit_p->unlock_file[i] == '/') {
                unlock_file_start = i + 1;
            }
        }
    }

    T_EXPLICIT(VTSS_TRACE_MODULE_ID, trace_grp, trace_lvl, file, line,
               "Current state for critical region \"%s\":\n"
               "Locked=%d\n"
               "Latest lock: %u/%s#%d\n"
               "Latest unlock: %u/%s#%d",

               crit_p->name,
               vtss_sprout_crit_is_locked(crit_p),

               crit_p->lock_thread_id,
               crit_p->lock_file ? &crit_p->lock_file[lock_file_start] : crit_p->lock_file,
               crit_p->lock_line,

               crit_p->unlock_thread_id,
               crit_p->unlock_file ? &crit_p->unlock_file[unlock_file_start] : crit_p->unlock_file,
               crit_p->unlock_line);
} 
#endif 

#endif 














void vtss_sprout_crit_init(
    vtss_sprout_crit_t *crit_p,
    const char *name
#if VTSS_TRACE_ENABLED
    ,
    int        trace_grp,
    int        trace_lvl,
    const char *file,
    const int  line
#endif
)
{
#if VTSS_SPROUT_MULTI_THREAD
    T_EXPLICIT(VTSS_TRACE_MODULE_ID, trace_grp, trace_lvl, file, line,
               "Creating vtss_sprout_crit_t \"%s\"", crit_p->name);

    memset(crit_p, 0, sizeof(vtss_sprout_crit_t));

    VTSS_OS_SEM_CREATE(&crit_p->sem, 0);

    strncpy(crit_p->name, name, VTSS_SPROUT_CRIT_NAME_LEN);
    crit_p->name[VTSS_SPROUT_CRIT_NAME_LEN - 1] = 0;
#if VTSS_TRACE_ENABLED
    crit_p->current_lock_thread_id = VTSS_SPROUT_CRIT_THREAD_ID_UNKNOWN;
#endif
    crit_p->init_done = 1;
#endif 
} 



void vtss_sprout_crit_lock(
    vtss_sprout_crit_t *crit_p
#if VTSS_TRACE_ENABLED
    ,
    int        trace_grp,
    int        trace_lvl,
    const char *file,
    const int  line
#endif
)
{
#if VTSS_SPROUT_MULTI_THREAD
#if VTSS_TRACE_ENABLED
    T_EXPLICIT(VTSS_TRACE_MODULE_ID, trace_grp, trace_lvl, file, line,
               "Locking critical region \"%s\"", crit_p->name);
    vtss_sprout_crit_state_trace(crit_p, trace_grp, trace_lvl, file, line);
#endif

#if VTSS_SPROUT_CRIT_CHK && VTSS_TRACE_ENABLED
    if (!crit_p->init_done) {
        vtss_sprout_crit_state_trace(crit_p, trace_grp, VTSS_TRACE_LVL_ERROR, file, line);
        VTSS_SPROUT_ASSERT(0,  ("crit_p not initialized (init_done == 0)!"));
    }

    
    if (crit_p->current_lock_thread_id != VTSS_SPROUT_CRIT_THREAD_ID_UNKNOWN &&
        crit_p->current_lock_thread_id == vtss_sprout_crit_get_thread_id()) {
        vtss_sprout_crit_state_trace(crit_p, trace_grp, VTSS_TRACE_LVL_ERROR, file, line);
        VTSS_SPROUT_ASSERT(0,  ("Critical region already locked by this thread id!"));
    }
#endif

    VTSS_OS_SEM_WAIT(&crit_p->sem);

#if VTSS_SPROUT_CRIT_CHK && VTSS_TRACE_ENABLED
    
    crit_p->current_lock_thread_id = vtss_sprout_crit_get_thread_id();
    crit_p->lock_thread_id         = vtss_sprout_crit_get_thread_id();
    crit_p->lock_file              = file;
    crit_p->lock_line              = line;
#endif

    T_EXPLICIT(VTSS_TRACE_MODULE_ID, trace_grp, trace_lvl, file, line,
               "Locked critical region \"%s\"", crit_p->name);
#if VTSS_TRACE_ENABLED
    vtss_sprout_crit_state_trace(crit_p, trace_grp, trace_lvl, file, line);
#endif
#endif 
} 



void vtss_sprout_crit_unlock(
    vtss_sprout_crit_t *crit_p
#if VTSS_TRACE_ENABLED
    ,
    int        trace_grp,
    int        trace_lvl,
    const char *file,
    const int  line
#endif
)
{
#if VTSS_SPROUT_MULTI_THREAD
#if VTSS_TRACE_ENABLED
    T_EXPLICIT(VTSS_TRACE_MODULE_ID, trace_grp, trace_lvl, file, line,
               "Unlocking critical region \"%s\"", crit_p->name);
    vtss_sprout_crit_state_trace(crit_p, trace_grp, trace_lvl, file, line);
#endif


#if VTSS_SPROUT_CRIT_CHK && VTSS_TRACE_ENABLED
#ifdef VTSS_OS_SEM_PEEK
    
    if (!vtss_sprout_crit_is_locked(crit_p)) {
        vtss_sprout_crit_state_trace(crit_p, trace_grp, VTSS_TRACE_LVL_ERROR, file, line);
        VTSS_SPROUT_ASSERT(0, ("Unlock called, but critical region is not locked!"));
    }
#endif

    
    if (crit_p->lock_thread_id != VTSS_SPROUT_CRIT_THREAD_ID_UNKNOWN &&
        crit_p->lock_thread_id != vtss_sprout_crit_get_thread_id()) {
#if VTSS_TRACE_ENABLED
        vtss_sprout_crit_state_trace(crit_p, trace_grp, VTSS_TRACE_LVL_ERROR, file, line);
#endif
        VTSS_SPROUT_ASSERT(0, ("Unlock called, but critical region is locked by different thread id!"));
    }

    
    crit_p->current_lock_thread_id = VTSS_SPROUT_CRIT_THREAD_ID_UNKNOWN;

    
    crit_p->unlock_thread_id  = vtss_sprout_crit_get_thread_id();
    crit_p->unlock_file       = file;
    crit_p->unlock_line       = line;
#endif

    VTSS_OS_SEM_POST(&crit_p->sem);

#if VTSS_TRACE_ENABLED
    T_EXPLICIT(VTSS_TRACE_MODULE_ID, trace_grp, trace_lvl, file, line,
               "Unlocked critical region \"%s\"", crit_p->name);
#endif
#endif 
} 



void vtss_sprout_crit_assert(
    vtss_sprout_crit_t *crit_p,
    BOOL              locked
#if VTSS_TRACE_ENABLED
    ,
    int        trace_grp,
    int        trace_lvl,
    const char *file,
    const int  line
#endif
)
{
#if VTSS_SPROUT_MULTI_THREAD

#if VTSS_SPROUT_CRIT_CHK && VTSS_TRACE_ENABLED
#ifdef VTSS_OS_SEM_PEEK
    if (vtss_sprout_crit_is_locked(crit_p) != locked) {
        vtss_sprout_crit_state_trace(crit_p, trace_grp, VTSS_TRACE_LVL_ERROR, file, line);
        VTSS_SPROUT_ASSERT(0, (locked ? "Critical region not locked!" : "Critical region locked!"));
    }
#endif
#endif

#endif 
} 












