/*

 Vitesse Switch API software.

 Copyright (c) 2002-2011 Vitesse Semiconductor Corporation "Vitesse". All
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

*/

#include <main.h> /* For VTSS_ASSERT */
#include "vtss_ecos_mutex_api.h"

/******************************************************************************/
/******************************************************************************/
void vtss_ecos_mutex_init(vtss_ecos_mutex_t *mutex)
{
  VTSS_ASSERT(mutex);
  cyg_scheduler_lock();
  mutex->lock_cnt = 0;
  cyg_mutex_init(&mutex->cyg_mutex);
  cyg_scheduler_unlock();
}

/******************************************************************************/
/******************************************************************************/
void vtss_ecos_mutex_lock(vtss_ecos_mutex_t *mutex)
{
  VTSS_ASSERT(mutex);
  cyg_scheduler_lock();
  if(mutex->cyg_mutex.locked == 0 || mutex->cyg_mutex.owner != (cyg_thread *)cyg_thread_self()) {
    cyg_scheduler_unlock();
    cyg_mutex_lock(&mutex->cyg_mutex);
    cyg_scheduler_lock();
    mutex->lock_cnt = (mutex->cyg_mutex.locked == 0) ? 0 : 1;
    cyg_scheduler_unlock();
    return;
  }
  if(mutex->type == VTSS_ECOS_MUTEX_TYPE_RECURSIVE) {
    mutex->lock_cnt++;
    cyg_scheduler_unlock();
    return;
  }
  cyg_scheduler_unlock();
  VTSS_ASSERT(FALSE); // Deadlock, because the same thread as owns the mutex waits for the mutex, which is configured as non-recursive
}

/******************************************************************************/
/******************************************************************************/
BOOL vtss_ecos_mutex_trylock(vtss_ecos_mutex_t *mutex)
{
  VTSS_ASSERT(mutex);
  cyg_scheduler_lock();
  if(mutex->cyg_mutex.locked == 0 || mutex->cyg_mutex.owner != (cyg_thread *)cyg_thread_self()) {
    cyg_scheduler_unlock();
    if (!cyg_mutex_trylock(&mutex->cyg_mutex)) {
      return FALSE;
    }
    cyg_scheduler_lock();
    mutex->lock_cnt = (mutex->cyg_mutex.locked == 0) ? 0 : 1;
    cyg_scheduler_unlock();
    return mutex->lock_cnt ? TRUE : FALSE;
  }
  if(mutex->type == VTSS_ECOS_MUTEX_TYPE_RECURSIVE) {
    mutex->lock_cnt++;
    cyg_scheduler_unlock();
    return TRUE;
  }
  cyg_scheduler_unlock();
  VTSS_ASSERT(FALSE); // Deadlock, because the same thread as owns the mutex waits for the mutex, which is configured as non-recursive
}


/******************************************************************************/
/******************************************************************************/
void vtss_ecos_mutex_unlock(vtss_ecos_mutex_t *mutex)
{
  VTSS_ASSERT(mutex);
  cyg_scheduler_lock();
  if(mutex->cyg_mutex.locked == 0 || mutex->cyg_mutex.owner != (cyg_thread*)cyg_thread_self()) {
    cyg_scheduler_unlock();
    VTSS_ASSERT(FALSE); // Not the proper owner, or mutex not owned.
    return;
  }
  if(mutex->type == VTSS_ECOS_MUTEX_TYPE_RECURSIVE) {
    mutex->lock_cnt--;
    if(mutex->lock_cnt) {
      cyg_scheduler_unlock();
      return;
    }
  }
  else
    mutex->lock_cnt = 0;
  cyg_scheduler_unlock();
  cyg_mutex_unlock(&mutex->cyg_mutex);
}

