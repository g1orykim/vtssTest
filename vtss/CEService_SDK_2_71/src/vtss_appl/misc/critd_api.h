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
 
$Id$
$Revision$

*/

// Introduction
// ------------
// critd implements a semaphore wrapper, which adds checks and other debug
// facilities.
// 
// When using critd it is assumed that:
// - The semaphore is always released by the same thread, which acquired the 
//   semaphore.
// - The semaphore is never to contain more than one token.
// - Each module does not concurrently create semaphores from different threads.
// 
// For an example on how to use critd, pls. refer to topo.c.


#ifndef _CRITD_API_H_
#define _CRITD_API_H_

#include "main.h"

#define CRITD_NAME_LEN 24

#if !defined(CRITD_CHK)
#define CRITD_CHK 1
#endif

#define CRITD_LOCK_ATTEMPT_SIZE     16
#define CRITD_MAX_LOCK_TIME_DEFAULT 60

typedef enum {
  /**< Mutex
   * Used to protect critical sections. 
   * Unlock must occur from the same thread that locks it,
   * with one single exception: Since the mutex is created
   * locked (typically you would create the mutex in
   * INIT_CMD_INIT), then a critd_exit() call is needed to
   * unlock it the first time. This call may be placed in any
   * thread context, but will typically be called after you've
   * loaded the data that the mutex protects with valid data
   * - for instance loaded from flash.
   * This is supported by this Mutex wrapper.
   *
   * Unlike semaphores, mutexes support priority inheritance,
   * which avoids the priority inversion problem. So this
   * is the normal critd type you will need.
   *
   * The mutex cannot be entered recursively from the same thread.
   */
  CRITD_TYPE_MUTEX,

  /**< Semaphore
   * Use this type of critd to protect a resource, which by
   * default is available.
   * Locking may occur in one thread and unlocking in another.
   *
   * The semaphore cannot be entered recursively from the same thread.
   */
  CRITD_TYPE_SEMAPHORE,
} critd_type_t;

// Create structure with semaphore and holder information
typedef struct _critd_t {
    uint         cookie;  // Initialized to CRITD_T_COOKIE
    critd_type_t type;
    cyg_flag_t   flag;
    union {
        cyg_mutex_t mutex;
        cyg_sem_t   semaphore;
    } u;
    BOOL         init_done;

#if VTSS_TRACE_ENABLED || CRITD_CHK
    // Semaphore name (used in trace and assertion output)
    char         name[CRITD_NAME_LEN+1]; 

    int          trace_module_id;

    // Max allowed lock time in seconds. -1 => Infinite lock allowed
    int          max_lock_time;

    // Log last lock attempts
    ulong        lock_attempt_cnt;
    ulong        lock_attempt_thread_id[CRITD_LOCK_ATTEMPT_SIZE]; 
    int          lock_attempt_line[CRITD_LOCK_ATTEMPT_SIZE];
    const char*  lock_attempt_file[CRITD_LOCK_ATTEMPT_SIZE];

    // Thread ID of current locker. Set to 0 on unlock
    ulong        current_lock_thread_id;

    // Thread ID, function and line of last lock/unlock 
    // Thread ID = 0 => Unknown thread ID 
    ulong        lock_thread_id; 
    const char   *lock_file;
    int          lock_line;
    ulong        lock_time;
    ulong        lock_cnt;

    ulong        unlock_thread_id; 
    const char   *unlock_file;
    int          unlock_line;

    struct _critd_t *nxt;

    // Surveillance information
    ulong last_lock_cnt;
    uint  lock_poll_cnt;
    BOOL  cookie_illegal;
#endif

#if VTSS_TRACE_ENABLED
    // Max + total actual lock time
    cyg_tick_count_t lock_tick_cnt;       // Tick count when the mutex was last taken.
    cyg_tick_count_t total_lock_tick_cnt; // Total time it has been taken.
    cyg_tick_count_t max_lock_tick_cnt;   // Maximum time it has been taken.
    ulong            max_lock_thread_id;
    const char *     max_lock_file;
    int              max_lock_line;

    // Max + total wait time
//    cyg_tick_count_t total_wait_tick_cnt;
    
#endif

} critd_t;

// Initialize critd_t
// Semaphore is initialized without token.
void critd_init(
    critd_t*               const crit_p, 
    const char*            const name, 
    const vtss_module_id_t module_id,
    const int              trace_module_id,
    const critd_type_t     type);


// Enter critical region (i.e. lock semaphore)
void critd_enter(
    critd_t*    const crit_p
#if VTSS_TRACE_ENABLED
    ,
    const int   trace_grp,
    const int   trace_lvl,
    const char* const file,
    const int   line
#endif
    );


// Exit critical region (i.e. unlock semaphore)
void critd_exit(
    critd_t*   const crit_p
#if VTSS_TRACE_ENABLED
    ,
    const int   trace_grp,
    const int   trace_lvl,
    const char* const file,
    const int   line
#endif
    );


// Assert semaphore to be locked/unlocked
void critd_assert_locked(
    critd_t*   const crit_p
#if VTSS_TRACE_ENABLED
    ,
    const int   trace_grp,
    const char* const file,
    const int   line
#endif
    );


vtss_rc critd_module_init(vtss_init_data_t *data);

// Debug functions
// ---------------
typedef int (*critd_dbg_printf_t)(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));

void critd_dbg_list(
    const critd_dbg_printf_t dbg_printf,
    const vtss_module_id_t   module_id,
    const BOOL               detailed,
    const BOOL               header,
    const critd_t*           single_critd_p);

void critd_dbg_max_lock(
    const critd_dbg_printf_t dbg_printf,
    const vtss_module_id_t   module_id,
    const BOOL               header,
    const BOOL               clear);
#endif // _CRITD_API_H_

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
