//==========================================================================
//
//      threadload.cxx
//
//      Estimate the current per-thread load.
//
//==========================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####                                            
// -------------------------------------------                              
// This file is part of eCos, the Embedded Configurable Operating System.   
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Free Software Foundation, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under    
// the terms of the GNU General Public License as published by the Free     
// Software Foundation; either version 2 or (at your option) any later      
// version.                                                                 
//
// eCos is distributed in the hope that it will be useful, but WITHOUT      
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or    
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License    
// for more details.                                                        
//
// You should have received a copy of the GNU General Public License        
// along with eCos; if not, write to the Free Software Foundation, Inc.,    
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.            
//
// As a special exception, if other files instantiate templates or use      
// macros or inline functions from this file, or you compile this file      
// and link it with other works to produce a work based on this file,       
// this file does not by itself cause the resulting work to be covered by   
// the GNU General Public License. However the source code for this file    
// must still be made available in accordance with section (3) of the GNU   
// General Public License v2.                                               
//
// This exception does not invalidate any other reasons why a work based    
// on this file might be covered by the GNU General Public License.         
// -------------------------------------------                              
// ####ECOSGPLCOPYRIGHTEND####                                              
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    Andrew Lunn
// Contributors: Andrew Lunn
// Date:         2002-08-12
// Purpose:      
// Description:  
//    This provides a simple CPU load meter. All loads are returned
//    as a percentage, ie 0-100. This is only a rough measure. Any clever
//    power management, sleep modes etc, will cause these results to be
//    wrong.
//              
// This code is part of eCos (tm).
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/kernel/kapi.h>
#include <cyg/hal/hal_arch.h> 
#include <cyg/threadload/threadload.h>
#include <cyg/kernel/sched.hxx>
#include <string.h>

typedef struct {
  // Updated by scheduler-callback:
  cyg_uint64 last_start_time;
  cyg_uint64 total_run_time;
  cyg_bool   in_use;

  // Updated by 1-sec tick:
  cyg_uint64 last_1sec;
  cyg_uint64 last_10sec;
  cyg_uint64 last_sample;
} threadload_t;

static threadload_t volatile threadloads[CYGNUM_THREADLOAD_MAX_ID];
       cyg_bool     volatile threadload_started;
static cyg_uint16   volatile threadload_max_id_seen;
static cyg_handle_t threadload_alarm_handle;
static cyg_alarm    threadload_alarm;
static cyg_uint64   threadload_user_last_1sec[CYGNUM_THREADLOAD_MAX_ID];
static cyg_uint64   threadload_user_last_10sec[CYGNUM_THREADLOAD_MAX_ID];
static cyg_mutex_t  threadload_mutex;
static cyg_uint16   threadload_prev_thread_id;

#define MSECS_PER_TICK (CYGNUM_HAL_RTC_NUMERATOR / CYGNUM_HAL_RTC_DENOMINATOR / 1000000)
#define TICKS_PER_SEC  (1000 / (MSECS_PER_TICK))

#ifndef THREADLOAD_CURRENT_TIME_GET
#define THREADLOAD_CURRENT_TIME_GET cyg_current_time()
#endif

/******************************************************************************/
// threadload_alarm_func()
// Assumes we're in DSR context with scheduler locked.
/******************************************************************************/
static void threadload_alarm_func(cyg_handle_t alarm, cyg_addrword_t data)
{
  cyg_uint16 id;
  if (!threadload_started) {
    return;
  }

  for (id = 0; id < threadload_max_id_seen; id++) {
    volatile threadload_t *t = &threadloads[id];
    t->last_1sec   = t->total_run_time - t->last_sample;
    t->last_10sec  = t->last_1sec + (t->last_10sec * 9ULL) / 10ULL;
    t->last_sample = t->total_run_time;
  }
}

/******************************************************************************/
// threadload_update()
// Called by scheduler (.../kernel/current/src/sched/sched.cxx) upon taskswitch.
// #next_thread_id == 0 means calling DSR handler.
/******************************************************************************/
externC void threadload_update(cyg_uint16 next_thread_id)
{
  if (!threadload_started) {
    return;
  }
  
  cyg_uint64 now = THREADLOAD_CURRENT_TIME_GET;

  if (next_thread_id < CYGNUM_THREADLOAD_MAX_ID) {
    threadloads[next_thread_id].last_start_time = now;
    threadloads[next_thread_id].in_use = true;
    if (next_thread_id > threadload_max_id_seen) {
      threadload_max_id_seen = next_thread_id;
    }
  }

  if (threadload_prev_thread_id < CYGNUM_THREADLOAD_MAX_ID) {
    volatile threadload_t *prev = &threadloads[threadload_prev_thread_id];
    if (prev->in_use) {
      prev->total_run_time += now - prev->last_start_time;
    }
  }

  threadload_prev_thread_id = next_thread_id;
}

/******************************************************************************/
// cyg_threadload_start()
/******************************************************************************/
externC void cyg_threadload_start(void)
{
  cyg_handle_t counter;

  if (threadload_started) {
    return;
  }

  memset((void *)threadloads, 0, sizeof(threadloads));
  threadload_max_id_seen = 0;

  // Create a timer that fires every second.
  cyg_clock_to_counter(cyg_real_time_clock(), &counter);
  cyg_alarm_create(counter,
                   threadload_alarm_func,
                   0,
                   &threadload_alarm_handle,
                   &threadload_alarm);

  cyg_alarm_initialize(threadload_alarm_handle, cyg_current_time() + TICKS_PER_SEC , TICKS_PER_SEC);
  cyg_alarm_enable(threadload_alarm_handle);
  cyg_mutex_init(&threadload_mutex);
  threadload_started = true;
}

/******************************************************************************/
// cyg_threadload_stop()
/******************************************************************************/
externC void cyg_threadload_stop(void)
{
  if (!threadload_started) {
    return;
  }
  cyg_mutex_lock(&threadload_mutex);
  threadload_started = false;
  cyg_alarm_delete(threadload_alarm_handle);
  threadload_alarm_handle = 0;

  // Unfortunately, we cannot destroy the mutex while we're owner of it.
  cyg_scheduler_lock();
  cyg_mutex_unlock(&threadload_mutex);
  cyg_mutex_destroy(&threadload_mutex);
  cyg_scheduler_unlock();
}

/******************************************************************************/
// cyg_threadload_get()
/******************************************************************************/
externC cyg_bool cyg_threadload_get(cyg_uint16 load_1sec[CYGNUM_THREADLOAD_MAX_ID], cyg_uint16 load_10sec[CYGNUM_THREADLOAD_MAX_ID])
{
  cyg_uint64 sum_1sec = 0, sum_10sec = 0;
  cyg_uint16 id, max_id;

  if (load_1sec) {
    memset(load_1sec, 0, CYGNUM_THREADLOAD_MAX_ID * sizeof(cyg_uint16));
  }
  if (load_10sec) {
    memset(load_10sec, 0, CYGNUM_THREADLOAD_MAX_ID * sizeof(cyg_uint16));
  }

  if (!threadload_started) {
    return false;
  }

  cyg_mutex_lock(&threadload_mutex);
  memset(threadload_user_last_1sec,  0, sizeof(threadload_user_last_1sec));
  memset(threadload_user_last_10sec, 0, sizeof(threadload_user_last_10sec));
  cyg_scheduler_lock();
  max_id = threadload_max_id_seen;
  for (id = 0; id <= threadload_max_id_seen; id++) {
    threadload_user_last_1sec[id]  = threadloads[id].last_1sec;
    threadload_user_last_10sec[id] = threadloads[id].last_10sec;
  }
  cyg_scheduler_unlock();

  // Time to work on threadload_user_last_1sec/10sec[] without harming scheduling.
  for (id = 0; id <= max_id; id++) {
    sum_1sec += threadload_user_last_1sec[id];
    sum_10sec += threadload_user_last_10sec[id];
  }

  if (load_1sec && sum_1sec != 0) {
    for (id = 0; id <= max_id; id++) {
      load_1sec[id] = ((10000ULL * threadload_user_last_1sec[id]) / sum_1sec);
    }
  }

  if (load_10sec && sum_10sec != 0) {
    for (id = 0; id <= max_id; id++) {
      load_10sec[id] = ((10000ULL * threadload_user_last_10sec[id]) / sum_10sec);
    }
  }

  cyg_mutex_unlock(&threadload_mutex);
  return true;
}

